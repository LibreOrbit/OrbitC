/**
 * @file orbitc_ringbuf.h
 * @brief Deterministic byte-oriented circular buffer interface.
 *
 * @addtogroup containers
 * @{
 */
#ifndef ORBITC_RINGBUF_H
#define ORBITC_RINGBUF_H

#include "orbitc_containers.h"

#if (ENABLE_CONTAINER_RINGBUF == 1)

/**
 * @brief Overflow handling policy for the byte ring buffer.
 */
typedef enum
{
    RINGBUF_MODE_REJECT    = 0, /**< Rejects writes if available free space is insufficient. */
    RINGBUF_MODE_OVERWRITE = 1  /**< Overwrites oldest bytes when full (flight recorder / blackbox mode). */
} ringbuf_mode_t;

/**
 * @brief Deterministic byte-oriented circular buffer context.
 */
typedef struct
{
    ui32_t         init_magic; /**< Initialization validation marker. */
    byte_t        *buf;        /**< Caller-supplied storage buffer. */
    usize_t        capacity;   /**< Total capacity in bytes. */
    usize_t        head;       /**< Write index. */
    usize_t        tail;       /**< Read index. */
    usize_t        count;      /**< Number of bytes currently stored. */
    usize_t        high_water; /**< Peak byte occupancy recorded. */
    ringbuf_mode_t mode;       /**< Behavior when buffer is full. */
} ringbuf_t;

/**
 * @brief Initializes a deterministic byte-oriented circular buffer.
 * @param[out] rb       Pointer to the ring buffer context to initialize.
 * @param[in]  buf      Caller-supplied memory buffer for storage.
 * @param[in]  capacity Total capacity of @p buf in bytes.
 * @param[in]  mode     Overflow handling policy (`RINGBUF_MODE_REJECT` or `RINGBUF_MODE_OVERWRITE`).
 * @return Status indicating success, null pointer, or invalid parameters.
 */
status_t orbitc_ringbuf_init(ringbuf_t *rb, void *buf, usize_t capacity, ringbuf_mode_t mode);

/**
 * @brief Resets a ring buffer to its initial empty state.
 * @param[in,out] rb Pointer to the ring buffer context to reset.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_ringbuf_reset(ringbuf_t *rb);

/**
 * @brief Pushes a single byte into the ring buffer.
 * @param[in,out] rb  Pointer to the ring buffer context.
 * @param[in]     val Byte value to push.
 * @return Status indicating success, null pointer, or rejection due to full buffer.
 */
status_t orbitc_ringbuf_push_byte(ringbuf_t *rb, byte_t val);

/**
 * @brief Pops a single byte from the ring buffer.
 * @param[in,out] rb      Pointer to the ring buffer context.
 * @param[out]    out_val Pointer receiving the popped byte.
 * @return Status indicating success, null pointer, or underflow (empty buffer).
 */
status_t orbitc_ringbuf_pop_byte(ringbuf_t *rb, byte_t *out_val);

/**
 * @brief Peeks at a byte in the ring buffer at a specific offset without removing it.
 * @param[in]  rb      Pointer to the ring buffer context.
 * @param[in]  offset  Index offset relative to the current read position.
 * @param[out] out_val Pointer receiving the peeked byte.
 * @return Status indicating success, null pointer, or out-of-bounds offset.
 */
status_t orbitc_ringbuf_peek_byte(const ringbuf_t *rb, usize_t offset, byte_t *out_val);

/**
 * @brief Writes a block of data into the ring buffer.
 * @param[in,out] rb   Pointer to the ring buffer context.
 * @param[in]     data Pointer to the source data buffer to write.
 * @param[in]     len  Number of bytes to write.
 * @return Status indicating success, null pointer, or capacity error depending on mode.
 */
status_t orbitc_ringbuf_write(ringbuf_t *rb, const void *data, usize_t len);

/**
 * @brief Reads a block of data from the ring buffer.
 * @param[in,out] rb       Pointer to the ring buffer context.
 * @param[out]    out_data Pointer to the destination buffer to receive the data.
 * @param[in]     len      Number of bytes to read.
 * @return Status indicating success, null pointer, or insufficient available data.
 */
status_t orbitc_ringbuf_read(ringbuf_t *rb, void *out_data, usize_t len);

/**
 * @brief Queries the number of bytes currently stored in the ring buffer.
 * @param[in]  rb        Pointer to the ring buffer context.
 * @param[out] out_count Pointer receiving the current occupancy count in bytes.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_ringbuf_get_count(const ringbuf_t *rb, usize_t *out_count);

/**
 * @brief Queries the available free space in bytes in the ring buffer.
 * @param[in]  rb       Pointer to the ring buffer context.
 * @param[out] out_free Pointer receiving the available free space count in bytes.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_ringbuf_get_free(const ringbuf_t *rb, usize_t *out_free);

#endif /* ENABLE_CONTAINER_RINGBUF == 1 */

#endif /* ORBITC_RINGBUF_H */
/** @} */