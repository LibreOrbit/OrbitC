/*   
 *  ================================================================================
 *  File: modules/strings/include/orbitc_strings.h
 *  ================================================================================
 */
#ifndef ORBITC_STRINGS_H
#define ORBITC_STRINGS_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

typedef enum
{
    STRINGS_CODE_OK           = 0x00U,
    STRINGS_CODE_NULL_ARG     = 0x01U,
    STRINGS_CODE_ZERO_CAP     = 0x02U,
    STRINGS_CODE_UNTERMINATED = 0x03U,
    STRINGS_CODE_TRUNCATED    = 0x04U,
    STRINGS_CODE_OVERLAP      = 0x05U,
    STRINGS_CODE_NOT_FOUND    = 0x06U,
    STRINGS_CODE_OVERFLOW     = 0x07U
} strings_code_t;

status_t orbitc_strlen_s(const char *str, usize_t max_cap, usize_t *out_len);
status_t orbitc_strcpy_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap);
status_t orbitc_strcat_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap);
status_t orbitc_strcmp_s(
    const char *lhs, usize_t lhs_cap, const char *rhs, usize_t rhs_cap, si32_t *out_cmp);
status_t orbitc_strchr_s(const char *str, usize_t str_cap, char ch, const char **out_found);
status_t orbitc_strstr_s(
    const char *haystack, usize_t haystack_cap,
    const char *needle, usize_t needle_cap, const char **out_found);

#endif