/**
 * @file orbitc_memory.h
 * @brief Low-level memory manipulation interface and backend selectors for OrbitC.
 *
 * This header defines bounded and checked memory utilities (set, copy, move,
 * compare, and search) alongside architecture-optimized backend selectors.
 * All functions integrate with OrbitC status reporting and compile-time/runtime
 * safety invariants.
 */

/**
 * @defgroup memory Memory
 * @brief Low-level memory manipulation routines, safety checks, and backend selection.
 * @{
 */

#ifndef ORBITC_MEMORY_H
#define ORBITC_MEMORY_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Hardware and Architecture Backend Identifiers                             */
/* ========================================================================== */

/** @brief Fully portable, architecture-agnostic C implementation backend. */
#define ORBITC_MEM_BACKEND_PORTABLE  0

/** @brief Generic 32-bit word-aligned optimized backend. */
#define ORBITC_MEM_BACKEND_GENERIC32 1

/** @brief ARM Cortex-A9 specific optimized backend (e.g., NEON/unaligned SIMD). */
#define ORBITC_MEM_BACKEND_CORTEX_A9 2

#ifndef ORBITC_MEM_BACKEND
/**
 * @brief Active memory backend selector macro.
 *
 * Defaults to @ref ORBITC_MEM_BACKEND_GENERIC32. Can be overridden via build
 * configuration flags (e.g., `-DORBITC_MEM_BACKEND=ORBITC_MEM_BACKEND_PORTABLE`).
 */
#define ORBITC_MEM_BACKEND ORBITC_MEM_BACKEND_GENERIC32
#endif

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific diagnostic status codes for memory operations.
 */
typedef enum
{
    MEMORY_CODE_OK        = 0x00U, /**< Memory operation completed successfully. */
    MEMORY_CODE_NULL_DST  = 0x01U, /**< Destination pointer argument is NULL. */
    MEMORY_CODE_NULL_SRC  = 0x02U, /**< Source pointer argument is NULL. */
    MEMORY_CODE_NULL_OUT  = 0x03U, /**< Output result pointer argument is NULL. */
    MEMORY_CODE_OVERLAP   = 0x04U, /**< Unexpected or illegal memory region overlap detected. */
    MEMORY_CODE_OVERFLOW  = 0x05U, /**< Address calculation wraps around or exceeds bounds. */
    MEMORY_CODE_NOT_FOUND = 0x06U  /**< Target byte pattern was not found in the search range. */
} memory_code_t;

/* ========================================================================== */
/* Memory Manipulation Routines                                               */
/* ========================================================================== */

/**
 * @brief Fills a destination memory block with a constant byte value.
 *
 * Writes @p len consecutive bytes initialized to @p val starting at address @p dst.
 *
 * @param[out] dst Destination buffer pointer to fill.
 * @param[in]  val Byte value to write.
 * @param[in]  len Number of bytes to set.
 *
 * @return Status indicating success or a null-pointer error code.
 */
status_t orbitc_memset(void *dst, byte_t val, usize_t len);

/**
 * @brief Copies bytes between non-overlapping memory buffers.
 *
 * Transfers @p len bytes from @p src to @p dst. Source and destination buffers
 * must not overlap; if overlap is possible, use @ref orbitc_memmove instead.
 *
 * @param[out] dst Destination buffer pointer.
 * @param[in]  src Source buffer pointer.
 * @param[in]  len Number of bytes to copy.
 *
 * @return Status indicating success, invalid/null parameters, or illegal overlap.
 */
status_t orbitc_memcpy(void *dst, const void *src, usize_t len);

/**
 * @brief Copies bytes between memory buffers with overlap tolerance.
 *
 * Safely transfers @p len bytes from @p src to @p dst even if both memory
 * regions overlap, ensuring data integrity across directional copies.
 *
 * @param[out] dst Destination buffer pointer.
 * @param[in]  src Source buffer pointer.
 * @param[in]  len Number of bytes to move.
 *
 * @return Status indicating success or null-pointer violation.
 */
status_t orbitc_memmove(void *dst, const void *src, usize_t len);

/**
 * @brief Compares two memory blocks byte-by-byte.
 *
 * Evaluates the first @p len bytes of @p lhs and @p rhs.
 *
 * @param[in]  lhs      Pointer to the first memory buffer.
 * @param[in]  rhs      Pointer to the second memory buffer.
 * @param[in]  len      Number of bytes to compare.
 * @param[out] out_diff Pointer receiving the comparison delta:
 *                      - `< 0` if the first differing byte in @p lhs is less than @p rhs
 *                      - `0` if all @p len bytes match
 *                      - `> 0` if the first differing byte in @p lhs is greater than @p rhs
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_memcmp(const void *lhs, const void *rhs, usize_t len, si32_t *out_diff);

/**
 * @brief Searches a memory block for the first occurrence of a specific byte.
 *
 * Scans @p len bytes of buffer @p buf for the first instance of @p val.
 *
 * @param[in]  buf       Pointer to the buffer to search.
 * @param[in]  val       Byte value to search for.
 * @param[in]  len       Maximum number of bytes to inspect.
 * @param[out] out_found Pointer receiving the address of the located byte,
 *                       or NULL if the byte was not found.
 *
 * @return Status indicating success, byte not found, or null-pointer error.
 */
status_t orbitc_memchr(const void *buf, byte_t val, usize_t len, const void **out_found);

/**
 * @brief Queries the runtime or compiled memory acceleration backend.
 *
 * @return Identifier corresponding to the active memory backend:
 *         - @ref ORBITC_MEM_BACKEND_PORTABLE
 *         - @ref ORBITC_MEM_BACKEND_GENERIC32
 *         - @ref ORBITC_MEM_BACKEND_CORTEX_A9
 */
ui32_t orbitc_memory_active_backend(void);

#endif /* ORBITC_MEMORY_H */

/** @} */ /* End of group memory */