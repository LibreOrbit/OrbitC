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
    bool_t   terminated = ORBITC_FALSE;

    orbitc_assert_not_null(str, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_len, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_non_zero_capacity(max_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_len = 0U;

        for (i = 0U; i < max_cap; ++i)
        {
            if (str[i] == '\0')
            {
                *out_len = i;
                terminated = ORBITC_TRUE;
                break;
            }
        }

        if (terminated == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_STRINGS,
                    STATUS_CLS_UNTERMINATED,
                    (ui8_t)STRINGS_CODE_UNTERMINATED));
        }
    }

    return st;
}

status_t orbitc_strcpy_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap)
{
    status_t st = STATUS_OK;
    usize_t  src_len = 0U;
    usize_t  i;

    orbitc_assert_not_null(dst, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(src, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_non_zero_capacity(dst_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);
    orbitc_assert_non_zero_capacity(src_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        status_t len_st = orbitc_strlen_s(src, src_cap, &src_len);
        if (orbitc_status_is_ok(len_st) == ORBITC_FALSE)
        {
            dst[0] = '\0';
            orbitc_status_latch(&st, len_st);
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        orbitc_assert_no_overlap(
            dst,
            dst_cap,
            src,
            src_len + 1U,
            &st,
            STATUS_MOD_STRINGS,
            (ui8_t)STRINGS_CODE_OVERLAP);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if ((src_len + 1U) > dst_cap)
        {
            dst[0] = '\0';
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_STRINGS,
                    STATUS_CLS_CAPACITY,
                    (ui8_t)STRINGS_CODE_TRUNCATED));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        for (i = 0U; i <= src_len; ++i)
        {
            dst[i] = src[i];
        }
    }

    return st;
}

status_t orbitc_strcat_s(char *dst, usize_t dst_cap, const char *src, usize_t src_cap)
{
    status_t st = STATUS_OK;
    usize_t  dst_len = 0U;
    usize_t  src_len = 0U;
    usize_t  i;

    orbitc_assert_not_null(dst, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(src, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_non_zero_capacity(dst_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);
    orbitc_assert_non_zero_capacity(src_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        status_t dst_len_st = orbitc_strlen_s(dst, dst_cap, &dst_len);
        orbitc_status_latch(&st, dst_len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        status_t src_len_st = orbitc_strlen_s(src, src_cap, &src_len);
        orbitc_status_latch(&st, src_len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        orbitc_assert_no_overlap(
            dst,
            dst_cap,
            src,
            src_len + 1U,
            &st,
            STATUS_MOD_STRINGS,
            (ui8_t)STRINGS_CODE_OVERLAP);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        orbitc_assert_no_overflow_size_add(
            dst_len,
            src_len + 1U,
            &st,
            STATUS_MOD_STRINGS,
            (ui8_t)STRINGS_CODE_OVERFLOW);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if ((dst_len + src_len + 1U) > dst_cap)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_STRINGS,
                    STATUS_CLS_CAPACITY,
                    (ui8_t)STRINGS_CODE_TRUNCATED));
        }
    }
    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        for (i = 0U; i <= src_len; ++i)
        {
            dst[dst_len + i] = src[i];
        }
    }

    return st;
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
    orbitc_assert_non_zero_capacity(lhs_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);
    orbitc_assert_non_zero_capacity(rhs_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_cmp = 0;

        status_t lhs_len_st = orbitc_strlen_s(lhs, lhs_cap, &lhs_len);
        orbitc_status_latch(&st, lhs_len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        status_t rhs_len_st = orbitc_strlen_s(rhs, rhs_cap, &rhs_len);
        orbitc_status_latch(&st, rhs_len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        while ((i <= lhs_len) && (i <= rhs_len))
        {
            byte_t cl = (byte_t)lhs[i];
            byte_t cr = (byte_t)rhs[i];
            if (cl != cr)
            {
                *out_cmp = (si32_t)cl - (si32_t)cr;
                break;
            }
            if (cl == 0U)
            {
                break;
            }
            ++i;
        }
    }

    return st;
}

status_t orbitc_strchr_s(const char *str, usize_t str_cap, char ch, const char **out_found)
{
    status_t st = STATUS_OK;
    usize_t  str_len = 0U;
    usize_t  i;
    bool_t   found = ORBITC_FALSE;

    orbitc_assert_not_null(str, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_found, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_non_zero_capacity(str_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_found = ORBITC_NULL;

        status_t len_st = orbitc_strlen_s(str, str_cap, &str_len);
        orbitc_status_latch(&st, len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        for (i = 0U; i <= str_len; ++i)
        {
            if (str[i] == ch)
            {
                *out_found = &str[i];
                found = ORBITC_TRUE;
                break;
            }
        }

        if (found == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_STRINGS,
                    STATUS_CLS_NOT_FOUND,
                    (ui8_t)STRINGS_CODE_NOT_FOUND));
        }
    }

    return st;
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
    bool_t   found = ORBITC_FALSE;

    orbitc_assert_not_null(haystack, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(needle, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_not_null(out_found, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_NULL_ARG);
    orbitc_assert_non_zero_capacity(haystack_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);
    orbitc_assert_non_zero_capacity(needle_cap, &st, STATUS_MOD_STRINGS, (ui8_t)STRINGS_CODE_ZERO_CAP);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_found = ORBITC_NULL;

        status_t h_len_st = orbitc_strlen_s(haystack, haystack_cap, &h_len);
        orbitc_status_latch(&st, h_len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        status_t n_len_st = orbitc_strlen_s(needle, needle_cap, &n_len);
        orbitc_status_latch(&st, n_len_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (n_len == 0U)
        {
            *out_found = haystack;
        }
        else if (n_len > h_len)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_STRINGS,
                    STATUS_CLS_NOT_FOUND,
                    (ui8_t)STRINGS_CODE_NOT_FOUND));
        }
        else
        {
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
                    found = ORBITC_TRUE;
                    break;
                }
            }

            if (found == ORBITC_FALSE)
            {
                orbitc_status_latch(
                    &st,
                    STATUS_MAKE(
                        STATUS_SEV_ERROR,
                        STATUS_MOD_STRINGS,
                        STATUS_CLS_NOT_FOUND,
                        (ui8_t)STRINGS_CODE_NOT_FOUND));
            }
        }
    }

    return st;
}