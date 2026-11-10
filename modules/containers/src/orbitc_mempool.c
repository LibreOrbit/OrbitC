/*
 * ================================================================================
 * File: modules/containers/src/orbitc_mempool.c
 * ================================================================================
 */
#include "orbitc_mempool.h"

#if (ENABLE_CONTAINER_MEMPOOL == 1)

#define ORBITC_MEMPOOL_MAGIC ((ui32_t)0x4D504F4CUL) /* "MPOL" */

typedef struct orbitc_pool_node
{
    struct orbitc_pool_node *next;
} orbitc_pool_node_t;

static void orbitc_mempool_build_freelist(mempool_t *pool)
{
    usize_t i;
    byte_t *curr = pool->pool_start;

    for (i = 0U; i < ORBITC_MEMPOOL_BITMAP_WORDS; ++i)
    {
        pool->alloc_bitmap[i] = 0UL;
    }

    pool->free_list  = (void *)curr;
    pool->used_count = 0U;

    for (i = 0U; i < (pool->block_count - 1U); ++i)
    {
        orbitc_pool_node_t *node = (orbitc_pool_node_t *)curr;
        node->next               = (orbitc_pool_node_t *)(curr + pool->block_size);
        curr                    += pool->block_size;
    }

    ((orbitc_pool_node_t *)curr)->next = ORBITC_NULL;
}

status_t orbitc_mempool_init(
    mempool_t *pool,
    void *buf,
    usize_t block_size,
    usize_t block_count)
{
    status_t      st        = STATUS_OK;
    const usize_t ptr_align = sizeof(void *);

    orbitc_assert_not_null(
        pool,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_NULL_PTR);

    orbitc_assert_not_null(
        buf,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_NULL_PTR);

    orbitc_assert_range_usize(
        block_count,
        1U,
        (usize_t)CONTAINER_MEMPOOL_MAX_BLOCKS,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_OUT_OF_RANGE);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        bool_t is_aligned;

        is_aligned =
            ((block_size >= ptr_align) &&
             ((block_size % ptr_align) == 0U) &&
             ((((uptr_t)buf) % ptr_align) == 0U))
                ? ORBITC_TRUE
                : ORBITC_FALSE;

        if (is_aligned == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_ALIGNMENT,
                    (ui8_t)CONTAINERS_CODE_ALIGNMENT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        pool->pool_start  = (byte_t *)buf;
        pool->pool_end    = pool->pool_start + (block_size * block_count);
        pool->block_size  = block_size;
        pool->block_count = block_count;
        pool->high_water  = 0U;

        orbitc_mempool_build_freelist(pool);
        pool->init_magic = ORBITC_MEMPOOL_MAGIC;
    }

    return st;
}

status_t orbitc_mempool_reset(mempool_t *pool)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        orbitc_mempool_build_freelist(pool);
    }

    return st;
}

status_t orbitc_mempool_alloc(mempool_t *pool, void **out_blk)
{
    status_t            st = STATUS_OK;
    orbitc_pool_node_t *node;
    usize_t             idx;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_blk, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_blk = ORBITC_NULL;

        if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (pool->free_list == ORBITC_NULL)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_CAPACITY,
                    (ui8_t)CONTAINERS_CODE_EMPTY));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        node            = (orbitc_pool_node_t *)pool->free_list;
        pool->free_list = (void *)node->next;

        idx = (usize_t)(((byte_t *)node - pool->pool_start) / pool->block_size);
        pool->alloc_bitmap[idx / 32U] |= ((ui32_t)1UL << (idx % 32U));

        pool->used_count += 1U;
        if (pool->used_count > pool->high_water)
        {
            pool->high_water = pool->used_count;
        }

        *out_blk = (void *)node;
    }

    return st;
}

status_t orbitc_mempool_free(mempool_t *pool, void *blk)
{
    status_t            st = STATUS_OK;
    const byte_t       *b_ptr;
    usize_t             offset;
    usize_t             idx;
    ui32_t              mask;
    orbitc_pool_node_t *node;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(blk, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        b_ptr = (const byte_t *)blk;
        if ((b_ptr < pool->pool_start) || (b_ptr >= pool->pool_end))
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_INVALID_BLOCK));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        b_ptr = (const byte_t *)blk;
        offset = (usize_t)(b_ptr - pool->pool_start);
        if ((offset % pool->block_size) != 0U)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_ALIGNMENT,
                    (ui8_t)CONTAINERS_CODE_INVALID_BLOCK));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        b_ptr = (const byte_t *)blk;
        offset = (usize_t)(b_ptr - pool->pool_start);
        idx  = offset / pool->block_size;
        mask = (ui32_t)1UL << (idx % 32U);

        if ((pool->alloc_bitmap[idx / 32U] & mask) == 0UL)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_DOUBLE_FREE));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        b_ptr = (const byte_t *)blk;
        offset = (usize_t)(b_ptr - pool->pool_start);
        idx  = offset / pool->block_size;
        mask = (ui32_t)1UL << (idx % 32U);

        pool->alloc_bitmap[idx / 32U] &= ~mask;

        node             = (orbitc_pool_node_t *)blk;
        node->next       = (orbitc_pool_node_t *)pool->free_list;
        pool->free_list  = blk;
        pool->used_count -= 1U;
    }

    return st;
}

status_t orbitc_mempool_get_free_count(const mempool_t *pool, usize_t *out_free)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_free, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_free = pool->block_count - pool->used_count;
    }

    return st;
}

status_t orbitc_mempool_get_used_count(const mempool_t *pool, usize_t *out_used)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_used, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_used = pool->used_count;
    }

    return st;
}

#else
typedef int orbitc_mempool_disabled_tu_t;
#endif /* ENABLE_CONTAINER_MEMPOOL == 1 */