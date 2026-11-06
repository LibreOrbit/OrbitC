/**
 * @file orbitc_stack.h
 * @brief Deterministic fixed-size element LIFO stack interface.
 *
 * @addtogroup containers
 * @{
 */
#ifndef ORBITC_STACK_H
#define ORBITC_STACK_H

#include "orbitc_containers.h"

#if (ENABLE_CONTAINER_STACK == 1)

/**
 * @brief Generic, bounded LIFO stack context.
 */
typedef struct
{
    ui32_t  init_magic; /**< Initialization validation marker. */
    byte_t *buf;        /**< Backing array of size `capacity * elem_size`. */
    usize_t elem_size;  /**< Size of a single element in bytes. */
    usize_t capacity;   /**< Maximum number of elements in the stack. */
    usize_t top;        /**< Current number of elements on the stack. */
    usize_t high_water; /**< Peak stack depth recorded. */
} stack_t;

/**
 * @brief Initializes a deterministic fixed-size element LIFO stack.
 * @param[out] stack     Pointer to the stack context to initialize.
 * @param[in]  buf       Caller-supplied backing memory buffer of at least `elem_size * capacity` bytes.
 * @param[in]  elem_size Size of a single element in bytes.
 * @param[in]  capacity  Maximum number of elements the stack can hold.
 * @return Status indicating success, null pointer, or invalid parameters.
 */
status_t orbitc_stack_init(stack_t *stack, void *buf, usize_t elem_size, usize_t capacity);

/**
 * @brief Resets a stack to its initial empty state.
 * @param[in,out] stack Pointer to the stack context to reset.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_stack_reset(stack_t *stack);

/**
 * @brief Pushes a new element onto the top of the stack.
 * @param[in,out] stack Pointer to the stack context.
 * @param[in]     elem  Pointer to the source element to copy onto the stack.
 * @return Status indicating success, null pointer, or stack overflow (full).
 */
status_t orbitc_stack_push(stack_t *stack, const void *elem);

/**
 * @brief Pops (removes and retrieves) the top element from the stack.
 * @param[in,out] stack    Pointer to the stack context.
 * @param[out]    out_elem Pointer receiving the popped element.
 * @return Status indicating success, null pointer, or stack underflow (empty).
 */
status_t orbitc_stack_pop(stack_t *stack, void *out_elem);

/**
 * @brief Peeks at the top element of the stack without removing it.
 * @param[in]  stack    Pointer to the stack context.
 * @param[out] out_elem Pointer receiving a copy of the top element.
 * @return Status indicating success, null pointer, or empty stack.
 */
status_t orbitc_stack_peek(const stack_t *stack, void *out_elem);

/**
 * @brief Checks whether the stack is currently empty.
 * @param[in]  stack     Pointer to the stack context.
 * @param[out] out_empty Pointer receiving `ORBITC_TRUE` if the stack is empty, `ORBITC_FALSE` otherwise.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_stack_is_empty(const stack_t *stack, bool_t *out_empty);

/**
 * @brief Checks whether the stack is currently full.
 * @param[in]  stack    Pointer to the stack context.
 * @param[out] out_full Pointer receiving `ORBITC_TRUE` if the stack is full, `ORBITC_FALSE` otherwise.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_stack_is_full(const stack_t *stack, bool_t *out_full);

/**
 * @brief Queries the number of elements currently stored in the stack.
 * @param[in]  stack     Pointer to the stack context.
 * @param[out] out_count Pointer receiving the current element count.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_stack_get_count(const stack_t *stack, usize_t *out_count);

#endif /* ENABLE_CONTAINER_STACK == 1 */

#endif /* ORBITC_STACK_H */
/** @} */