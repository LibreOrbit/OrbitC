/**
 * @file orbitc_strings.h
 * @brief Safe and bounded string manipulation interface for OrbitC.
 *
 * This header provides length-bounded, error-checked implementations of
 * standard C string manipulation operations (length calculation, copying,
 * concatenation, comparison, and searching). All functions enforce explicit
 * buffer capacities and prevent buffer overflows and unterminated reads.
 */

/**
 * @defgroup strings Strings
 * @brief Safe string manipulation utilities with explicit bounds checking.
 * @{
 */

#ifndef ORBITC_STRINGS_H
#define ORBITC_STRINGS_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific status and error codes for string operations.
 */
typedef enum
{
    STRINGS_CODE_OK           = 0x00U, /**< Operation completed successfully. */
    STRINGS_CODE_NULL_ARG     = 0x01U, /**< One or more required pointer arguments are NULL. */
    STRINGS_CODE_ZERO_CAP     = 0x02U, /**< Provided buffer capacity is zero. */
    STRINGS_CODE_UNTERMINATED = 0x03U, /**< String is not null-terminated within the given capacity. */
    STRINGS_CODE_TRUNCATED    = 0x04U, /**< Destination capacity was insufficient; content was truncated. */
    STRINGS_CODE_OVERLAP      = 0x05U, /**< Source and destination memory regions overlap unexpectedly. */
    STRINGS_CODE_NOT_FOUND    = 0x06U, /**< Target character or substring was not found. */
    STRINGS_CODE_OVERFLOW     = 0x07U, /**< Operation would exceed storage capacity or numeric bounds. */
} strings_code_t;

/* ========================================================================== */
/* Safe String Functions                                                      */
/* ========================================================================== */

/**
 * @brief Computes the length of a null-terminated string up to a maximum capacity.
 *
 * Scans @p str for a null-terminator without reading past @p max_cap bytes.
 *
 * @param[in]  str      Pointer to the null-terminated string to inspect.
 * @param[in]  max_cap  Maximum number of bytes to inspect in @p str.
 * @param[out] out_len  Pointer to store the calculated length (excluding the null terminator).
 *
 * @return Status indicating success or the specific failure code.
 */
status_t orbitc_strlen_s(const char *str, usize_t max_cap, usize_t *out_len);

/**
 * @brief Copies a bounded source string to a destination buffer.
 *
 * Copies characters from @p src into @p dst ensuring destination bounds are respected
 * and the resulting string is safely null-terminated.
 *
 * @param[out] dst      Pointer to the destination buffer.
 * @param[in]  dst_cap  Total capacity of the destination buffer in bytes.
 * @param[in]  src      Pointer to the source string.
 * @param[in]  src_cap  Maximum number of bytes to read from the source string.
 *
 * @return Status indicating success, truncation, overlap, or invalid parameters.
 */
status_t orbitc_strcpy_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap);

/**
 * @brief Appends a bounded source string to a destination buffer.
 *
 * Concatenates characters from @p src to the end of the existing string in @p dst,
 * ensuring the destination buffer is not overrun and remains null-terminated.
 *
 * @param[in,out] dst      Pointer to the destination buffer containing an existing string.
 * @param[in]     dst_cap  Total capacity of the destination buffer in bytes.
 * @param[in]     src      Pointer to the source string to append.
 * @param[in]     src_cap  Maximum number of bytes to read from the source string.
 *
 * @return Status indicating success, truncation, overlap, or invalid parameters.
 */
status_t orbitc_strcat_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap);

/**
 * @brief Compares two bounded strings lexicographically.
 *
 * Evaluates characters from @p lhs and @p rhs within their respective capacity limits.
 *
 * @param[in]  lhs      Pointer to the left-hand side string.
 * @param[in]  lhs_cap  Maximum capacity/bytes to inspect in @p lhs.
 * @param[in]  rhs      Pointer to the right-hand side string.
 * @param[in]  rhs_cap  Maximum capacity/bytes to inspect in @p rhs.
 * @param[out] out_cmp  Pointer to store the comparison result:
 *                      - `< 0` if @p lhs is lexicographically less than @p rhs
 *                      - `0` if @p lhs is equal to @p rhs
 *                      - `> 0` if @p lhs is lexicographically greater than @p rhs
 *
 * @return Status indicating success or invalid parameters.
 */
status_t orbitc_strcmp_s(
    const char *lhs, usize_t lhs_cap, const char *rhs, usize_t rhs_cap, si32_t *out_cmp);

/**
 * @brief Locates the first occurrence of a character in a bounded string.
 *
 * Searches @p str up to @p str_cap bytes for the first occurrence of @p ch.
 *
 * @param[in]  str       Pointer to the string to search.
 * @param[in]  str_cap   Maximum number of bytes to search within @p str.
 * @param[in]  ch        Character to find.
 * @param[out] out_found Pointer to store the address of the located character,
 *                       or NULL if not found.
 *
 * @return Status indicating success, not found, or invalid parameters.
 */
status_t orbitc_strchr_s(const char *str, usize_t str_cap, char ch, const char **out_found);

/**
 * @brief Locates the first occurrence of a substring in a bounded string.
 *
 * Searches for @p needle within @p haystack, respecting both buffers' maximum capacities.
 *
 * @param[in]  haystack     Pointer to the string to search within.
 * @param[in]  haystack_cap Maximum number of bytes to search in @p haystack.
 * @param[in]  needle       Pointer to the substring to find.
 * @param[in]  needle_cap   Maximum number of bytes to read from @p needle.
 * @param[out] out_found    Pointer to store the address of the beginning of the substring,
 *                          or NULL if not found.
 *
 * @return Status indicating success, not found, or invalid parameters.
 */
status_t orbitc_strstr_s(
    const char *haystack, usize_t haystack_cap,
    const char *needle, usize_t needle_cap, const char **out_found);

#endif /* ORBITC_STRINGS_H */

/** @} */ /* End of group strings */