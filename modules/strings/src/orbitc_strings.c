/*   
 *  ================================================================================
 *  File: modules/strings/src/orbitc_strings.c
 *  ================================================================================
 */
#include "orbitc_strings.h"

status_t orbitc_strlen_s(const char *str, usize_t max_cap, usize_t *out_len)
{
    status_t st = STATUS_OK;
    usize_t  i;

    orbitc_assert_not_null(str, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_len, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);

    if ((str == ORBITC_NULL) || (out_len == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NULL_PTR, STRINGS_CODE_NULL_ARG);
    }

    *out_len = 0U;
    if (max_cap == 0U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_ZERO_CAP);
    }

    for (i = 0U; i < max_cap; ++i)
    {
        if (str[i] == '\0')
        {
            *out_len = i;
            return STATUS_OK;
        }
    }

    return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_UNTERMINATED, STRINGS_CODE_UNTERMINATED);
}

status_t orbitc_strcpy_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap)
{
    status_t st = STATUS_OK;
    usize_t  src_len = 0U;
    usize_t  i;

    orbitc_assert_not_null(dst, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(src, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);

    if ((dst == ORBITC_NULL) || (src == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NULL_PTR, STRINGS_CODE_NULL_ARG);
    }

    if ((dst_cap == 0U) || (src_cap == 0U))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_ZERO_CAP);
    }

    st = orbitc_strlen_s(src, src_cap, &src_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        dst[0] = '\0';
        return st;
    }

    if (orbitc_ranges_overlap(dst, dst_cap, src, src_len + 1U) == ORBITC_TRUE)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_OVERLAP, STRINGS_CODE_OVERLAP);
    }

    if ((src_len + 1U) > dst_cap)
    {
        dst[0] = '\0';
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_TRUNCATED);
    }

    for (i = 0U; i <= src_len; ++i)
    {
        dst[i] = src[i];
    }

    return STATUS_OK;
}

status_t orbitc_strcat_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap)
{
    status_t st = STATUS_OK;
    usize_t  dst_len = 0U;
    usize_t  src_len = 0U;
    usize_t  i;

    orbitc_assert_not_null(dst, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(src, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);

    if ((dst == ORBITC_NULL) || (src == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NULL_PTR, STRINGS_CODE_NULL_ARG);
    }

    if ((dst_cap == 0U) || (src_cap == 0U))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_ZERO_CAP);
    }

    st = orbitc_strlen_s(dst, dst_cap, &dst_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    st = orbitc_strlen_s(src, src_cap, &src_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    if (orbitc_ranges_overlap(dst, dst_cap, src, src_len + 1U) == ORBITC_TRUE)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_OVERLAP, STRINGS_CODE_OVERLAP);
    }

    if (dst_len > (ORBITC_USIZE_MAX - (src_len + 1U)))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_OVERFLOW, STRINGS_CODE_OVERFLOW);
    }

    if ((dst_len + src_len + 1U) > dst_cap)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_TRUNCATED);
    }

    for (i = 0U; i <= src_len; ++i)
    {
        dst[dst_len + i] = src[i];
    }

    return STATUS_OK;
}

status_t orbitc_strcmp_s(
    const char *lhs, usize_t lhs_cap, const char *rhs, usize_t rhs_cap, si32_t *out_cmp)
{
    status_t st = STATUS_OK;
    usize_t  lhs_len = 0U;
    usize_t  rhs_len = 0U;
    usize_t  i = 0U;

    orbitc_assert_not_null(lhs, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(rhs, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_cmp, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);

    if ((lhs == ORBITC_NULL) || (rhs == ORBITC_NULL) || (out_cmp == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NULL_PTR, STRINGS_CODE_NULL_ARG);
    }

    *out_cmp = 0;
    if ((lhs_cap == 0U) || (rhs_cap == 0U))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_ZERO_CAP);
    }

    st = orbitc_strlen_s(lhs, lhs_cap, &lhs_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    st = orbitc_strlen_s(rhs, rhs_cap, &rhs_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    while ((i <= lhs_len) && (i <= rhs_len))
    {
        byte_t cl = (byte_t)lhs[i];
        byte_t cr = (byte_t)rhs[i];
        if (cl != cr)
        {
            *out_cmp = (si32_t)cl - (si32_t)cr;
            return STATUS_OK;
        }
        if (cl == 0U)
        {
            break;
        }
        ++i;
    }

    *out_cmp = 0;
    return STATUS_OK;
}

status_t orbitc_strchr_s(const char *str, usize_t str_cap, char ch, const char **out_found)
{
    status_t st = STATUS_OK;
    usize_t  str_len = 0U;
    usize_t  i;

    orbitc_assert_not_null(str, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_found, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);

    if ((str == ORBITC_NULL) || (out_found == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NULL_PTR, STRINGS_CODE_NULL_ARG);
    }

    *out_found = ORBITC_NULL;
    if (str_cap == 0U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_ZERO_CAP);
    }

    st = orbitc_strlen_s(str, str_cap, &str_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    for (i = 0U; i <= str_len; ++i)
    {
        if (str[i] == ch)
        {
            *out_found = &str[i];
            return STATUS_OK;
        }
    }

    return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NOT_FOUND, STRINGS_CODE_NOT_FOUND);
}

status_t orbitc_strstr_s(
    const char *haystack, usize_t haystack_cap,
    const char *needle, usize_t needle_cap, const char **out_found)
{
    status_t st = STATUS_OK;
    usize_t  h_len = 0U;
    usize_t  n_len = 0U;
    usize_t  i;
    usize_t  j;

    orbitc_assert_not_null(haystack, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(needle, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_found, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);

    if ((haystack == ORBITC_NULL) || (needle == ORBITC_NULL) || (out_found == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NULL_PTR, STRINGS_CODE_NULL_ARG);
    }

    *out_found = ORBITC_NULL;
    if ((haystack_cap == 0U) || (needle_cap == 0U))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, STRINGS_CODE_ZERO_CAP);
    }

    st = orbitc_strlen_s(haystack, haystack_cap, &h_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    st = orbitc_strlen_s(needle, needle_cap, &n_len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }

    if (n_len == 0U)
    {
        *out_found = haystack;
        return STATUS_OK;
    }

    if (n_len > h_len)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NOT_FOUND, STRINGS_CODE_NOT_FOUND);
    }

    for (i = 0U; i <= (h_len - n_len); ++i)
    {
        bool_t match = ORBITC_TRUE;
        for (j = 0U; j < n_len; ++j)
        {
            if (haystack[i + j] != needle[j])
            {
                match = ORBITC_FALSE;
                break;
            }
        }
        if (match == ORBITC_TRUE)
        {
            *out_found = &haystack[i];
            return STATUS_OK;
        }
    }

    return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_NOT_FOUND, STRINGS_CODE_NOT_FOUND);
}