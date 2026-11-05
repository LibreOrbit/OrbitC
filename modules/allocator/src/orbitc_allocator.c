/*
 * ================================================================================
 * File: modules/allocator/src/orbitc_allocator.c
 * ================================================================================
 */
#include "orbitc_allocator.h"

/* ========================================================================== */
/* Internal Magic Constants & Security Patterns                               */
/* ========================================================================== */

/** @brief Magic token confirming an allocator context has been initialized. */
#define ORBITC_ALLOC_INIT_MAGIC  ((ui32_t)0x4F524241UL) /* "ORBA" */

/** @brief Canary value indicating a valid, unallocated block. */
#define ORBITC_BLOCK_MAGIC_FREE  ((ui32_t)0xF0E1D2C3UL)

/** @brief Canary value indicating a valid, actively allocated block. */
#define ORBITC_BLOCK_MAGIC_USED  ((ui32_t)0xA55AA55AUL)

/** @brief Poison byte written to newly allocated memory (catches uninitialized reads). */
#define ORBITC_POISON_ALLOC_BYTE ((byte_t)0xAAU)

/** @brief Poison byte written to freed memory (catches use-after-free bugs). */
#define ORBITC_POISON_FREE_BYTE  ((byte_t)0xDDU)

/* ========================================================================== */
/* Internal Block Header Structure                                            */
/* ========================================================================== */

struct orbitc_alloc_block
{
#if (ALLOCATOR_ENABLE_CANARIES == 1)
    ui32_t                magic;    /**< State and integrity canary (FREE vs USED). */
    uptr_t                checksum; /**< XOR checksum of block address and size against SEU. */
#else
    ui32_t                is_free;  /**< Boolean flag: 1 = Free, 0 = Used. */
#endif
    usize_t               size;     /**< Usable payload size in bytes (always aligned). */
    orbitc_alloc_block_t *next;     /**< Next contiguous block in the heap, or ORBITC_NULL. */
};

/**
 * @brief Compile-time macro that rounds the block header size up to the nearest
 * multiple of ALLOCATOR_ALIGNMENT. This guarantees that (byte_t*)block + HDR_SIZE
 * is always strictly aligned.
 */
#define ORBITC_ALIGN_UP(val, align) \
    (((usize_t)(val) + ((usize_t)(align) - 1U)) & ~((usize_t)(align) - 1U))

#define ORBITC_BLOCK_HDR_SIZE \
    ORBITC_ALIGN_UP(sizeof(orbitc_alloc_block_t), ALLOCATOR_ALIGNMENT)

/* ========================================================================== */
/* Optional Built-in Static Heap Storage                                      */
/* ========================================================================== */
#if (ALLOCATOR_STATIC_HEAP == 1)

#if defined(__GNUC__) || defined(__clang__)
static byte_t g_static_heap_buffer[ALLOCATOR_STATIC_HEAP_SIZE]
    __attribute__((aligned(ALLOCATOR_ALIGNMENT)));
#else
static byte_t g_static_heap_buffer[ALLOCATOR_STATIC_HEAP_SIZE];
#endif

static orbitc_allocator_t g_static_allocator_ctx;

#endif /* ALLOCATOR_STATIC_HEAP == 1 */

/* ========================================================================== */
/* Internal Synchronization & Helper Routines                                 */
/* ========================================================================== */

static inline void orbitc_alloc_lock(const orbitc_allocator_t *alloc)
{
#if (ALLOCATOR_ENABLE_LOCKS == 1)
    if (alloc->enter_crit != ORBITC_NULL)
    {
        alloc->enter_crit(alloc->lock_arg);
    }
#else
    (void)alloc;
#endif
}

static inline void orbitc_alloc_unlock(const orbitc_allocator_t *alloc)
{
#if (ALLOCATOR_ENABLE_LOCKS == 1)
    if (alloc->exit_crit != ORBITC_NULL)
    {
        alloc->exit_crit(alloc->lock_arg);
    }
#else
    (void)alloc;
#endif
}

static inline bool_t orbitc_is_aligned(const void *ptr)
{
    return ((((uptr_t)ptr) & ((uptr_t)ALLOCATOR_ALIGNMENT - 1U)) == 0U) ? ORBITC_TRUE : ORBITC_FALSE;
}

static inline usize_t orbitc_align_size(usize_t size)
{
    return ORBITC_ALIGN_UP(size, ALLOCATOR_ALIGNMENT);
}

#if (ALLOCATOR_ENABLE_POISONING == 1)
static void orbitc_mem_fill(byte_t *dst, byte_t val, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        dst[i] = val;
    }
}
#endif

#if (ALLOCATOR_ENABLE_CANARIES == 1)
static inline uptr_t orbitc_compute_block_checksum(const orbitc_alloc_block_t *blk)
{
    return ((uptr_t)blk) ^ ((uptr_t)blk->size) ^ ((uptr_t)blk->next) ^ (uptr_t)blk->magic;
}

static inline void orbitc_seal_block(orbitc_alloc_block_t *blk, bool_t is_free)
{
    blk->magic    = (is_free == ORBITC_TRUE) ? ORBITC_BLOCK_MAGIC_FREE : ORBITC_BLOCK_MAGIC_USED;
    blk->checksum = orbitc_compute_block_checksum(blk);
}
#else
static inline void orbitc_seal_block(orbitc_alloc_block_t *blk, bool_t is_free)
{
    blk->is_free = (is_free == ORBITC_TRUE) ? 1U : 0U;
}
#endif

static inline bool_t orbitc_is_block_free(const orbitc_alloc_block_t *blk)
{
#if (ALLOCATOR_ENABLE_CANARIES == 1)
    return (blk->magic == ORBITC_BLOCK_MAGIC_FREE) ? ORBITC_TRUE : ORBITC_FALSE;
#else
    return (blk->is_free != 0U) ? ORBITC_TRUE : ORBITC_FALSE;
#endif
}

/**
 * @brief Validates that a block header is within bounds, aligned, and uncorrupted.
 */
static bool_t orbitc_validate_block(const orbitc_allocator_t *alloc, const orbitc_alloc_block_t *blk)
{
    const byte_t *blk_addr = (const byte_t *)blk;

    if ((blk_addr < alloc->heap_start) || ((blk_addr + ORBITC_BLOCK_HDR_SIZE) > alloc->heap_end))
    {
        return ORBITC_FALSE;
    }

    if (orbitc_is_aligned(blk) == ORBITC_FALSE)
    {
        return ORBITC_FALSE;
    }

    if ((blk->size == 0U) || ((blk->size & (ALLOCATOR_ALIGNMENT - 1U)) != 0U))
    {
        return ORBITC_FALSE;
    }

    if ((blk_addr + ORBITC_BLOCK_HDR_SIZE + blk->size) > alloc->heap_end)
    {
        return ORBITC_FALSE;
    }

    if (blk->next != ORBITC_NULL)
    {
        /* Next block must strictly follow this block in memory to prevent cycle attacks */
        const byte_t *expected_next = blk_addr + ORBITC_BLOCK_HDR_SIZE + blk->size;
        if ((const byte_t *)blk->next != expected_next)
        {
            return ORBITC_FALSE;
        }
    }

#if (ALLOCATOR_ENABLE_CANARIES == 1)
    if ((blk->magic != ORBITC_BLOCK_MAGIC_FREE) && (blk->magic != ORBITC_BLOCK_MAGIC_USED))
    {
        return ORBITC_FALSE;
    }
    if (blk->checksum != orbitc_compute_block_checksum(blk))
    {
        return ORBITC_FALSE;
    }
#endif

    return ORBITC_TRUE;
}

/* ========================================================================== */
/* Public API Implementation                                                  */
/* ========================================================================== */

status_t orbitc_allocator_init(orbitc_allocator_t *alloc, const allocator_config_t *config)
{
    status_t st = STATUS_OK;
    usize_t  usable_total;
    usize_t  initial_payload;

    orbitc_assert_not_null(alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    orbitc_assert_not_null(config, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);

    if ((alloc == ORBITC_NULL) || (config == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    orbitc_assert_not_null(config->mem_start, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    if (config->mem_start == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

#if (ALLOCATOR_ENABLE_LOCKS == 1)
    /* Reject asymmetric lock hooks (both must be NULL or both must be valid function pointers) */
    if (((config->enter_crit != ORBITC_NULL) && (config->exit_crit == ORBITC_NULL)) ||
        ((config->enter_crit == ORBITC_NULL) && (config->exit_crit != ORBITC_NULL)))
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_BAD_LOCK_CFG);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_BAD_LOCK_CFG);
    }
#endif

    if (alloc->init_magic == ORBITC_ALLOC_INIT_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_WARNING, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_ALREADY_INIT);
    }

    if (orbitc_is_aligned(config->mem_start) == ORBITC_FALSE)
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_BAD_ALIGNMENT);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_ALIGNMENT, ALLOCATOR_CODE_BAD_ALIGNMENT);
    }

    /* Truncate total size down to a multiple of ALLOCATOR_ALIGNMENT */
    usable_total = config->mem_size & ~((usize_t)ALLOCATOR_ALIGNMENT - 1U);

    if (usable_total <= (ORBITC_BLOCK_HDR_SIZE + ALLOCATOR_ALIGNMENT))
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_INVALID_SIZE);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_CAPACITY, ALLOCATOR_CODE_INVALID_SIZE);
    }

    /* Check pointer wrap-around */
    if (((uptr_t)config->mem_start + (uptr_t)usable_total) < (uptr_t)config->mem_start)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_OVERFLOW, ALLOCATOR_CODE_INVALID_SIZE);
    }

#if (ALLOCATOR_ENABLE_LOCKS == 1)
    alloc->enter_crit     = config->enter_crit;
    alloc->exit_crit      = config->exit_crit;
    alloc->lock_arg       = config->lock_arg;
#endif

    orbitc_alloc_lock(alloc);

    alloc->heap_start     = (byte_t *)config->mem_start;
    alloc->heap_end       = alloc->heap_start + usable_total;
    alloc->total_size     = usable_total;
    alloc->used_bytes     = 0U;
    alloc->high_watermark = 0U;
    alloc->alloc_count    = 0U;

    initial_payload       = usable_total - ORBITC_BLOCK_HDR_SIZE;
    alloc->head           = (orbitc_alloc_block_t *)alloc->heap_start;
    alloc->head->size     = initial_payload;
    alloc->head->next     = ORBITC_NULL;
    orbitc_seal_block(alloc->head, ORBITC_TRUE);

#if (ALLOCATOR_ENABLE_POISONING == 1)
    orbitc_mem_fill(alloc->heap_start + ORBITC_BLOCK_HDR_SIZE, ORBITC_POISON_FREE_BYTE, initial_payload);
#endif

    alloc->init_magic     = ORBITC_ALLOC_INIT_MAGIC;

    orbitc_alloc_unlock(alloc);

    return st;
}

#if (ALLOCATOR_STATIC_HEAP == 1)
status_t orbitc_allocator_init_static(orbitc_allocator_t **out_alloc)
{
    status_t           st = STATUS_OK;
    allocator_config_t cfg;

    orbitc_assert_not_null(out_alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    if (out_alloc == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    *out_alloc = ORBITC_NULL;

    if (g_static_allocator_ctx.init_magic != ORBITC_ALLOC_INIT_MAGIC)
    {
        cfg.mem_start  = (void *)g_static_heap_buffer;
        cfg.mem_size   = (usize_t)ALLOCATOR_STATIC_HEAP_SIZE;
#if (ALLOCATOR_ENABLE_LOCKS == 1)
        cfg.enter_crit = ORBITC_NULL;
        cfg.exit_crit  = ORBITC_NULL;
        cfg.lock_arg   = ORBITC_NULL;
#endif
        st = orbitc_allocator_init(&g_static_allocator_ctx, &cfg);
        if (orbitc_status_is_ok(st) == ORBITC_FALSE)
        {
            return st;
        }
    }

    *out_alloc = &g_static_allocator_ctx;
    return STATUS_OK;
}
#endif

status_t orbitc_allocator_reset(orbitc_allocator_t *alloc)
{
    status_t st = STATUS_OK;
    usize_t  initial_payload;

    orbitc_assert_not_null(alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    if (alloc == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    if (alloc->init_magic != ORBITC_ALLOC_INIT_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_NOT_INIT);
    }

    orbitc_alloc_lock(alloc);

    initial_payload       = alloc->total_size - ORBITC_BLOCK_HDR_SIZE;
    alloc->head           = (orbitc_alloc_block_t *)alloc->heap_start;
    alloc->head->size     = initial_payload;
    alloc->head->next     = ORBITC_NULL;
    alloc->used_bytes     = 0U;
    alloc->alloc_count    = 0U;
    orbitc_seal_block(alloc->head, ORBITC_TRUE);

#if (ALLOCATOR_ENABLE_POISONING == 1)
    orbitc_mem_fill(alloc->heap_start + ORBITC_BLOCK_HDR_SIZE, ORBITC_POISON_FREE_BYTE, initial_payload);
#endif

    orbitc_alloc_unlock(alloc);

    return st;
}

status_t orbitc_alloc(orbitc_allocator_t *alloc, usize_t size, void **out_ptr)
{
    status_t              st = STATUS_OK;
    usize_t               aligned_size;
    orbitc_alloc_block_t *curr;
    orbitc_alloc_block_t *chosen      = ORBITC_NULL;
    orbitc_alloc_block_t *prev        = ORBITC_NULL;
    orbitc_alloc_block_t *chosen_prev = ORBITC_NULL;

    orbitc_assert_not_null(alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    orbitc_assert_not_null(out_ptr, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);

    if ((alloc == ORBITC_NULL) || (out_ptr == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    *out_ptr = ORBITC_NULL;

    if (alloc->init_magic != ORBITC_ALLOC_INIT_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_NOT_INIT);
    }

    if ((size == 0U) || (size > (usize_t)ALLOCATOR_MAX_ALLOC_SIZE))
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_INVALID_SIZE);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_OUT_OF_RANGE, ALLOCATOR_CODE_INVALID_SIZE);
    }

    aligned_size = orbitc_align_size(size);
    if ((aligned_size < size) || (aligned_size > (alloc->total_size - ORBITC_BLOCK_HDR_SIZE)))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_CAPACITY, ALLOCATOR_CODE_OUT_OF_MEMORY);
    }

    orbitc_alloc_lock(alloc);

    curr = alloc->head;
    while (curr != ORBITC_NULL)
    {
        if (orbitc_validate_block(alloc, curr) == ORBITC_FALSE)
        {
            orbitc_alloc_unlock(alloc);
            return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
        }

        if ((orbitc_is_block_free(curr) == ORBITC_TRUE) && (curr->size >= aligned_size))
        {
#if (ALLOCATOR_POLICY == ORBITC_ALLOC_POLICY_BEST_FIT)
            if ((chosen == ORBITC_NULL) || (curr->size < chosen->size))
            {
                chosen      = curr;
                chosen_prev = prev;
                if (curr->size == aligned_size)
                {
                    break; /* Exact fit found */
                }
            }
#else /* FIRST_FIT or BUMP */
            chosen      = curr;
            chosen_prev = prev;
            break;
#endif
        }

        prev = curr;
        curr = curr->next;
    }

    if (chosen == ORBITC_NULL)
    {
        orbitc_alloc_unlock(alloc);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_CAPACITY, ALLOCATOR_CODE_OUT_OF_MEMORY);
    }

    /* Split the block if the remainder can hold at least a header + 1 alignment unit */
    if (chosen->size >= (aligned_size + ORBITC_BLOCK_HDR_SIZE + ALLOCATOR_ALIGNMENT))
    {
        byte_t               *split_addr = (byte_t *)chosen + ORBITC_BLOCK_HDR_SIZE + aligned_size;
        orbitc_alloc_block_t *remainder  = (orbitc_alloc_block_t *)split_addr;

        remainder->size = chosen->size - aligned_size - ORBITC_BLOCK_HDR_SIZE;
        remainder->next = chosen->next;
        orbitc_seal_block(remainder, ORBITC_TRUE);

        chosen->size = aligned_size;
        chosen->next = remainder;
    }

    orbitc_seal_block(chosen, ORBITC_FALSE);
    (void)chosen_prev;

    alloc->used_bytes  += chosen->size;
    alloc->alloc_count += 1U;
    if (alloc->used_bytes > alloc->high_watermark)
    {
        alloc->high_watermark = alloc->used_bytes;
    }

    *out_ptr = (void *)((byte_t *)chosen + ORBITC_BLOCK_HDR_SIZE);

#if (ALLOCATOR_ENABLE_POISONING == 1)
    orbitc_mem_fill((byte_t *)(*out_ptr), ORBITC_POISON_ALLOC_BYTE, chosen->size);
#endif

    orbitc_alloc_unlock(alloc);

    return st;
}

status_t orbitc_free(orbitc_allocator_t *alloc, void *ptr)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    orbitc_assert_not_null(ptr, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);

    if ((alloc == ORBITC_NULL) || (ptr == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    if (alloc->init_magic != ORBITC_ALLOC_INIT_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_NOT_INIT);
    }

#if (ALLOCATOR_ENABLE_FREE == 0) || (ALLOCATOR_POLICY == ORBITC_ALLOC_POLICY_BUMP)
    (void)ptr;
    return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_FREE_DISABLED);
#else
    {
        orbitc_alloc_block_t *curr;
        orbitc_alloc_block_t *prev  = ORBITC_NULL;
        orbitc_alloc_block_t *found = ORBITC_NULL;
        const byte_t         *p_raw = (const byte_t *)ptr;

        /* Quick range and alignment rejection before acquiring lock */
        if ((p_raw < (alloc->heap_start + ORBITC_BLOCK_HDR_SIZE)) ||
            (p_raw >= alloc->heap_end) ||
            (orbitc_is_aligned(ptr) == ORBITC_FALSE))
        {
            return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_INVALID_PTR);
        }

        orbitc_alloc_lock(alloc);

        curr = alloc->head;
        while (curr != ORBITC_NULL)
        {
            if (orbitc_validate_block(alloc, curr) == ORBITC_FALSE)
            {
                orbitc_alloc_unlock(alloc);
                return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
            }

            if (((const byte_t *)curr + ORBITC_BLOCK_HDR_SIZE) == p_raw)
            {
                found = curr;
                break;
            }

            prev = curr;
            curr = curr->next;
        }

        if (found == ORBITC_NULL)
        {
            orbitc_alloc_unlock(alloc);
            return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NOT_FOUND, ALLOCATOR_CODE_INVALID_PTR);
        }

        if (orbitc_is_block_free(found) == ORBITC_TRUE)
        {
            orbitc_alloc_unlock(alloc);
            return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_DOUBLE_FREE);
        }

        /* Validate next neighbor before coalescing */
        if (found->next != ORBITC_NULL)
        {
            if (orbitc_validate_block(alloc, found->next) == ORBITC_FALSE)
            {
                orbitc_alloc_unlock(alloc);
                return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
            }
        }

        alloc->used_bytes  -= found->size;
        alloc->alloc_count -= 1U;

#if (ALLOCATOR_ENABLE_POISONING == 1)
        orbitc_mem_fill((byte_t *)found + ORBITC_BLOCK_HDR_SIZE, ORBITC_POISON_FREE_BYTE, found->size);
#endif

        /* Mark as free */
        orbitc_seal_block(found, ORBITC_TRUE);

        /* Coalesce with next block if free */
        if ((found->next != ORBITC_NULL) && (orbitc_is_block_free(found->next) == ORBITC_TRUE))
        {
            orbitc_alloc_block_t *next_blk = found->next;
            found->size += ORBITC_BLOCK_HDR_SIZE + next_blk->size;
            found->next  = next_blk->next;

#if (ALLOCATOR_ENABLE_POISONING == 1)
            orbitc_mem_fill((byte_t *)next_blk, ORBITC_POISON_FREE_BYTE, ORBITC_BLOCK_HDR_SIZE);
#endif
            orbitc_seal_block(found, ORBITC_TRUE);
        }

        /* Coalesce with previous block if free */
        if ((prev != ORBITC_NULL) && (orbitc_is_block_free(prev) == ORBITC_TRUE))
        {
            prev->size += ORBITC_BLOCK_HDR_SIZE + found->size;
            prev->next  = found->next;

#if (ALLOCATOR_ENABLE_POISONING == 1)
            orbitc_mem_fill((byte_t *)found, ORBITC_POISON_FREE_BYTE, ORBITC_BLOCK_HDR_SIZE);
#endif
            orbitc_seal_block(prev, ORBITC_TRUE);
        }

        orbitc_alloc_unlock(alloc);

        return st;
    }
#endif
}

status_t orbitc_allocator_check_integrity(const orbitc_allocator_t *alloc)
{
    status_t                    st = STATUS_OK;
    const orbitc_alloc_block_t *curr;
    usize_t                     accounted_bytes = 0U;
    usize_t                     counted_used    = 0U;
    ui32_t                      counted_allocs  = 0U;

    orbitc_assert_not_null(alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    if (alloc == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    if (alloc->init_magic != ORBITC_ALLOC_INIT_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_NOT_INIT);
    }

    orbitc_alloc_lock(alloc);

    curr = alloc->head;
    if ((const byte_t *)curr != alloc->heap_start)
    {
        orbitc_alloc_unlock(alloc);
        return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
    }

    while (curr != ORBITC_NULL)
    {
        if (orbitc_validate_block(alloc, curr) == ORBITC_FALSE)
        {
            orbitc_alloc_unlock(alloc);
            return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
        }

        accounted_bytes += ORBITC_BLOCK_HDR_SIZE + curr->size;
        if (orbitc_is_block_free(curr) == ORBITC_FALSE)
        {
            counted_used   += curr->size;
            counted_allocs += 1U;
        }

        curr = curr->next;
    }

    /* Verify exact conservation of total memory bytes and metadata counters */
    if ((accounted_bytes != alloc->total_size) ||
        (counted_used != alloc->used_bytes) ||
        (counted_allocs != alloc->alloc_count))
    {
        orbitc_alloc_unlock(alloc);
        return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
    }

    orbitc_alloc_unlock(alloc);

    return STATUS_OK;
}

status_t orbitc_allocator_get_stats(const orbitc_allocator_t *alloc, allocator_stats_t *out_stats)
{
    status_t                    st = STATUS_OK;
    const orbitc_alloc_block_t *curr;

    orbitc_assert_not_null(alloc, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);
    orbitc_assert_not_null(out_stats, &st, STATUS_MOD_ALLOCATOR, (ui8_t)ALLOCATOR_CODE_NULL_PTR);

    if ((alloc == ORBITC_NULL) || (out_stats == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_NULL_PTR, ALLOCATOR_CODE_NULL_PTR);
    }

    if (alloc->init_magic != ORBITC_ALLOC_INIT_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ALLOCATOR, STATUS_CLS_INVALID_PARAM, ALLOCATOR_CODE_NOT_INIT);
    }

    orbitc_alloc_lock(alloc);

    out_stats->total_size       = alloc->total_size;
    out_stats->free_bytes       = 0U;
    out_stats->used_bytes       = alloc->used_bytes;
    out_stats->overhead_bytes   = 0U;
    out_stats->largest_free_blk = 0U;
    out_stats->high_watermark   = alloc->high_watermark;
    out_stats->alloc_count      = alloc->alloc_count;
    out_stats->free_block_count = 0U;

    curr = alloc->head;
    while (curr != ORBITC_NULL)
    {
        if (orbitc_validate_block(alloc, curr) == ORBITC_FALSE)
        {
            orbitc_alloc_unlock(alloc);
            return STATUS_MAKE(STATUS_SEV_FATAL, STATUS_MOD_ALLOCATOR, STATUS_CLS_INTERNAL, ALLOCATOR_CODE_CORRUPTED);
        }

        out_stats->overhead_bytes += ORBITC_BLOCK_HDR_SIZE;

        if (orbitc_is_block_free(curr) == ORBITC_TRUE)
        {
            out_stats->free_bytes       += curr->size;
            out_stats->free_block_count += 1U;
            if (curr->size > out_stats->largest_free_blk)
            {
                out_stats->largest_free_blk = curr->size;
            }
        }

        curr = curr->next;
    }

    orbitc_alloc_unlock(alloc);

    return STATUS_OK;
}