/*   
 *  ================================================================================
 *  File: modules/status/include/orbitc_assert.h
 *  ================================================================================
 */

#ifndef ORBITC_ASSERT_H
#define ORBITC_ASSERT_H

#include "orbitc_status.h"

#ifndef ORBITC_ENABLE_ASSERT
#define ORBITC_ENABLE_ASSERT 1
#endif

static inline bool_t orbitc_ranges_overlap(
    const void *ptr_a, usize_t len_a, const void *ptr_b, usize_t len_b)
{
    uptr_t start_a;
    uptr_t start_b;
    uptr_t end_a;
    uptr_t end_b;

    if ((ptr_a == ORBITC_NULL) || (ptr_b == ORBITC_NULL) || (len_a == 0U) || (len_b == 0U))
    {
        return ORBITC_FALSE;
    }

    start_a = (uptr_t)ptr_a;
    start_b = (uptr_t)ptr_b;
    end_a   = start_a + (uptr_t)len_a;
    end_b   = start_b + (uptr_t)len_b;

    if ((end_a < start_a) || (end_b < start_b))
    {
        return ORBITC_TRUE;
    }

    return ((start_a < end_b) && (start_b < end_a)) ? ORBITC_TRUE : ORBITC_FALSE;
}

#if (ORBITC_ENABLE_ASSERT == 1)

static inline void orbitc_assert_not_null(
    const void *ptr, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (ptr == ORBITC_NULL)
        {
            *st = STATUS_MAKE(STATUS_SEV_ERROR, mod, STATUS_CLS_NULL_PTR, code);
        }
    }
}

static inline void orbitc_assert_param(
    bool_t cond, status_t *st, status_sev_t sev, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (cond == ORBITC_FALSE)
        {
            *st = STATUS_MAKE(sev, mod, STATUS_CLS_INVALID_PARAM, code);
        }
    }
}

static inline void orbitc_assert_range_u32(
    ui32_t val, ui32_t min_val, ui32_t max_val, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if ((val < min_val) || (val > max_val))
        {
            *st = STATUS_MAKE(STATUS_SEV_ERROR, mod, STATUS_CLS_OUT_OF_RANGE, code);
        }
    }
}

static inline void orbitc_assert_capacity(
    usize_t required, usize_t capacity, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (required > capacity)
        {
            *st = STATUS_MAKE(STATUS_SEV_ERROR, mod, STATUS_CLS_CAPACITY, code);
        }
    }
}

static inline void orbitc_assert_no_overlap(
    const void *ptr_a, usize_t len_a, const void *ptr_b, usize_t len_b,
    status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (orbitc_ranges_overlap(ptr_a, len_a, ptr_b, len_b) == ORBITC_TRUE)
        {
            *st = STATUS_MAKE(STATUS_SEV_ERROR, mod, STATUS_CLS_OVERLAP, code);
        }
    }
}

static inline void orbitc_assert_no_overflow_size_add(
    usize_t lhs, usize_t rhs, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (lhs > (ORBITC_USIZE_MAX - rhs))
        {
            *st = STATUS_MAKE(STATUS_SEV_ERROR, mod, STATUS_CLS_OVERFLOW, code);
        }
    }
}

#else

static inline void orbitc_assert_not_null(
    const void *ptr, status_t *st, status_mod_t mod, ui8_t code)
{
    (void)ptr;
    (void)st;
    (void)mod;
    (void)code;
}

static inline void orbitc_assert_param(
    bool_t cond, status_t *st, status_sev_t sev, status_mod_t mod, ui8_t code)
{
    (void)cond;
    (void)st;
    (void)sev;
    (void)mod;
    (void)code;
}

static inline void orbitc_assert_range_u32(
    ui32_t val, ui32_t min_val, ui32_t max_val, status_t *st, status_mod_t mod, ui8_t code)
{
    (void)val;
    (void)min_val;
    (void)max_val;
    (void)st;
    (void)mod;
    (void)code;
}

static inline void orbitc_assert_capacity(
    usize_t required, usize_t capacity, status_t *st, status_mod_t mod, ui8_t code)
{
    (void)required;
    (void)capacity;
    (void)st;
    (void)mod;
    (void)code;
}

static inline void orbitc_assert_no_overlap(
    const void *ptr_a, usize_t len_a, const void *ptr_b, usize_t len_b,
    status_t *st, status_mod_t mod, ui8_t code)
{
    (void)ptr_a;
    (void)len_a;
    (void)ptr_b;
    (void)len_b;
    (void)st;
    (void)mod;
    (void)code;
}

static inline void orbitc_assert_no_overflow_size_add(
    usize_t lhs, usize_t rhs, status_t *st, status_mod_t mod, ui8_t code)
{
    (void)lhs;
    (void)rhs;
    (void)st;
    (void)mod;
    (void)code;
}

#endif

#endif