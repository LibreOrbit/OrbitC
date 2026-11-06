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

status_t orbitc_mempool_init(mempool_t *pool, void *buf, usize_t block_size, usize_t block_count)
{
    status_t      st        = STATUS_OK;
    const usize_t ptr_align = sizeof(void *);

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(buf, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((pool == ORBITC_NULL) || (buf == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }

    if ((block_count == 0U) || (block_count > (usize_t)CONTAINER_MEMPOOL_MAX_BLOCKS))
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_OUT_OF_RANGE);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_OUT_OF_RANGE, CONTAINERS_CODE_OUT_OF_RANGE);
    }

    if ((block_size < ptr_align) || ((block_size % ptr_align) != 0U) || ((((uptr_t)buf) % ptr_align) != 0U))
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_ALIGNMENT);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_ALIGNMENT, CONTAINERS_CODE_ALIGNMENT);
    }

    pool->pool_start  = (byte_t *)buf;
    pool->pool_end    = pool->pool_start + (block_size * block_count);
    pool->block_size  = block_size;
    pool->block_count = block_count;
    pool->high_water  = 0U;

    orbitc_mempool_build_freelist(pool);
    pool->init_magic = ORBITC_MEMPOOL_MAGIC;

    return st;
}

status_t orbitc_mempool_reset(mempool_t *pool)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if (pool == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }

    orbitc_mempool_build_freelist(pool);
    return st;
}

status_t orbitc_mempool_alloc(mempool_t *pool, void **out_blk)
{
    status_t            st = STATUS_OK;
    orbitc_pool_node_t *node;
    usize_t             idx;

    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_blk, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((pool == ORBITC_NULL) || (out_blk == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }

    *out_blk = ORBITC_NULL;

    if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }

    if (pool->free_list == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_CAPACITY, CONTAINERS_CODE_EMPTY);
    }

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
    if ((pool == ORBITC_NULL) || (blk == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }

    if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }

    b_ptr = (const byte_t *)blk;
    if ((b_ptr < pool->pool_start) || (b_ptr >= pool->pool_end))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_INVALID_BLOCK);
    }

    offset = (usize_t)(b_ptr - pool->pool_start);
    if ((offset % pool->block_size) != 0U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_ALIGNMENT, CONTAINERS_CODE_INVALID_BLOCK);
    }

    idx  = offset / pool->block_size;
    mask = (ui32_t)1UL << (idx % 32U);

    if ((pool->alloc_bitmap[idx / 32U] & mask) == 0UL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_DOUBLE_FREE);
    }

    pool->alloc_bitmap[idx / 32U] &= ~mask;

    node             = (orbitc_pool_node_t *)blk;
    node->next       = (orbitc_pool_node_t *)pool->free_list;
    pool->free_list  = blk;
    pool->used_count -= 1U;

    return st;
}

status_t orbitc_mempool_get_free_count(const mempool_t *pool, usize_t *out_free)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_free, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((pool == ORBITC_NULL) || (out_free == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    *out_free = pool->block_count - pool->used_count;
    return st;
}

status_t orbitc_mempool_get_used_count(const mempool_t *pool, usize_t *out_used)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(pool, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_used, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((pool == ORBITC_NULL) || (out_used == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (pool->init_magic != ORBITC_MEMPOOL_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    *out_used = pool->used_count;
    return st;
}

#else
typedef int orbitc_mempool_disabled_tu_t;
#endif /* ENABLE_CONTAINER_MEMPOOL == 1 */