/**
 * @file orbitc_fifo.h
 * @brief Deterministic fixed-size element FIFO queue interface.
 *
 * @addtogroup containers
 * @{
 */
#ifndef ORBITC_FIFO_H
#define ORBITC_FIFO_H

#include "orbitc_containers.h"

#if (ENABLE_CONTAINER_FIFO == 1)

/**
 * @brief Generic, type-safe fixed-size element FIFO queue context.
 */
typedef struct
{
    ui32_t  init_magic; /**< Initialization validation marker. */
    byte_t *buf;        /**< Backing array of size `capacity * elem_size`. */
    usize_t elem_size;  /**< Size of a single element in bytes. */
    usize_t capacity;   /**< Maximum number of elements in the queue. */
    usize_t head;       /**< Write element index (`0 .. capacity - 1`). */
    usize_t tail;       /**< Read element index (`0 .. capacity - 1`). */
    usize_t count;      /**< Number of elements currently queued. */
    usize_t high_water; /**< Maximum number of elements queued simultaneously. */
} fifo_t;

/**
 * @brief Initializes a deterministic fixed-size element FIFO queue.
 * @param[out] fifo      Pointer to the FIFO queue context to initialize.
 * @param[in]  buf       Caller-supplied backing memory buffer of at least `elem_size * capacity` bytes.
 * @param[in]  elem_size Size of a single element in bytes.
 * @param[in]  capacity  Maximum number of elements the queue can hold.
 * @return Status indicating success, null pointer, or invalid parameters.
 */
status_t orbitc_fifo_init(fifo_t *fifo, void *buf, usize_t elem_size, usize_t capacity);

/**
 * @brief Resets a FIFO queue to its initial empty state.
 * @param[in,out] fifo Pointer to the FIFO queue context to reset.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_fifo_reset(fifo_t *fifo);

/**
 * @brief Pushes a new element into the FIFO queue.
 * @param[in,out] fifo Pointer to the FIFO queue context.
 * @param[in]     elem Pointer to the source element to copy into the queue.
 * @return Status indicating success, null pointer, or queue overflow (full).
 */
status_t orbitc_fifo_push(fifo_t *fifo, const void *elem);

/**
 * @brief Pops (removes and retrieves) the oldest element from the FIFO queue.
 * @param[in,out] fifo     Pointer to the FIFO queue context.
 * @param[out]    out_elem Pointer receiving the popped element.
 * @return Status indicating success, null pointer, or queue underflow (empty).
 */
status_t orbitc_fifo_pop(fifo_t *fifo, void *out_elem);

/**
 * @brief Peeks at the next element in the FIFO queue without removing it.
 * @param[in]  fifo     Pointer to the FIFO queue context.
 * @param[out] out_elem Pointer receiving a copy of the front element.
 * @return Status indicating success, null pointer, or empty queue.
 */
status_t orbitc_fifo_peek(const fifo_t *fifo, void *out_elem);

/**
 * @brief Checks whether the FIFO queue is currently empty.
 * @param[in]  fifo      Pointer to the FIFO queue context.
 * @param[out] out_empty Pointer receiving `ORBITC_TRUE` if the queue is empty, `ORBITC_FALSE` otherwise.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_fifo_is_empty(const fifo_t *fifo, bool_t *out_empty);

/**
 * @brief Checks whether the FIFO queue is currently full.
 * @param[in]  fifo     Pointer to the FIFO queue context.
 * @param[out] out_full Pointer receiving `ORBITC_TRUE` if the queue is full, `ORBITC_FALSE` otherwise.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_fifo_is_full(const fifo_t *fifo, bool_t *out_full);

/**
 * @brief Queries the number of elements currently stored in the FIFO queue.
 * @param[in]  fifo      Pointer to the FIFO queue context.
 * @param[out] out_count Pointer receiving the current element count.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_fifo_get_count(const fifo_t *fifo, usize_t *out_count);

#endif /* ENABLE_CONTAINER_FIFO == 1 */

#endif /* ORBITC_FIFO_H */
/** @} */