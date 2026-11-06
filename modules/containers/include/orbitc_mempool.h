/**
 * @file orbitc_mempool.h
 * @brief Deterministic O(1) fixed-size block memory pool interface.
 *
 * @addtogroup containers
 * @{
 */
#ifndef ORBITC_MEMPOOL_H
#define ORBITC_MEMPOOL_H

#include "orbitc_containers.h"

#if (ENABLE_CONTAINER_MEMPOOL == 1)

ORBITC_STATIC_ASSERT((CONTAINER_MEMPOOL_MAX_BLOCKS > 0U) && ((CONTAINER_MEMPOOL_MAX_BLOCKS % 32U) == 0U),
                     mempool_max_blocks_must_be_multiple_of_32);

#define ORBITC_MEMPOOL_BITMAP_WORDS ((CONTAINER_MEMPOOL_MAX_BLOCKS) / 32U)

/**
 * @brief Helper macro to declare an aligned static buffer for a memory pool.
 */
#if defined(__GNUC__) || defined(__clang__)
#define ORBITC_MEMPOOL_STORAGE_DECLARE(name, block_size, block_count) \
    static byte_t name[(block_size) * (block_count)] __attribute__((aligned(sizeof(void *))))
#else
#define ORBITC_MEMPOOL_STORAGE_DECLARE(name, block_size, block_count) \
    static uptr_t name[((block_size) * (block_count) + sizeof(uptr_t) - 1U) / sizeof(uptr_t)]
#endif

/**
 * @brief Deterministic O(1) fixed-size block allocator with bitmap double-free protection.
 */
typedef struct
{
    ui32_t  init_magic;                                /**< Initialization validation marker. */
    byte_t *pool_start;                                /**< Base address of the memory buffer. */
    byte_t *pool_end;                                  /**< End address (exclusive) of the buffer. */
    usize_t block_size;                                /**< Size of each block (aligned to pointer size). */
    usize_t block_count;                               /**< Total number of blocks in the pool. */
    usize_t used_count;                                /**< Number of currently allocated blocks. */
    usize_t high_water;                                /**< Peak number of simultaneously allocated blocks. */
    void   *free_list;                                 /**< Head pointer of the embedded O(1) free list. */
    ui32_t  alloc_bitmap[ORBITC_MEMPOOL_BITMAP_WORDS]; /**< Bitmask tracking allocated blocks (1 = Used, 0 = Free). */
} mempool_t;

/**
 * @brief Initializes a deterministic O(1) fixed-size block memory pool.
 * @param[out] pool        Pointer to the memory pool context to initialize.
 * @param[in]  buf         Caller-supplied backing memory buffer of at least `block_size * block_count` bytes.
 * @param[in]  block_size  Size of each individual block in bytes (aligned to pointer size).
 * @param[in]  block_count Total number of blocks in the pool (must be a multiple of 32).
 * @return Status indicating success, null pointer, or invalid parameters.
 */
status_t orbitc_mempool_init(mempool_t *pool, void *buf, usize_t block_size, usize_t block_count);

/**
 * @brief Resets a memory pool to its initial empty state.
 * @param[in,out] pool Pointer to the memory pool context to reset.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_mempool_reset(mempool_t *pool);

/**
 * @brief Allocates a fixed-size memory block from the pool in O(1) time.
 * @param[in,out] pool    Pointer to the memory pool context.
 * @param[out]    out_blk Pointer receiving the address of the allocated memory block.
 * @return Status indicating success, null pointer, or pool exhaustion (out of blocks).
 */
status_t orbitc_mempool_alloc(mempool_t *pool, void **out_blk);

/**
 * @brief Frees a previously allocated memory block back to the pool.
 * @param[in,out] pool Pointer to the memory pool context.
 * @param[in]     blk  Pointer to the memory block to free.
 * @return Status indicating success, null pointer, invalid block address, or double-free error.
 */
status_t orbitc_mempool_free(mempool_t *pool, void *blk);

/**
 * @brief Queries the number of available free blocks in the memory pool.
 * @param[in]  pool     Pointer to the memory pool context.
 * @param[out] out_free Pointer receiving the count of currently free blocks.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_mempool_get_free_count(const mempool_t *pool, usize_t *out_free);

/**
 * @brief Queries the number of currently allocated blocks in the memory pool.
 * @param[in]  pool     Pointer to the memory pool context.
 * @param[out] out_used Pointer receiving the count of currently allocated blocks.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_mempool_get_used_count(const mempool_t *pool, usize_t *out_used);

#endif /* ENABLE_CONTAINER_MEMPOOL == 1 */

#endif /* ORBITC_MEMPOOL_H */
/** @} */