/*
 * ================================================================================
 * File: modules/bitops/src/orbitc_bitops.c
 * ================================================================================
 */
#include "orbitc_bitops.h"

/* ========================================================================== */
/* Internal Portable Algorithms (SWAR / De Bruijn-free)                       */
/* ========================================================================== */

static inline ui8_t orbitc_popcount32_swar(ui32_t x)
{
    x = x - ((x >> 1U) & 0x55555555UL);
    x = (x & 0x33333333UL) + ((x >> 2U) & 0x33333333UL);
    x = (x + (x >> 4U)) & 0x0F0F0F0FUL;
    return (ui8_t)((x * 0x01010101UL) >> 24U);
}

static inline ui8_t orbitc_clz32_portable(ui32_t x)
{
    ui8_t n = 0U;
    if ((x & 0xFFFF0000UL) == 0UL) { n = (ui8_t)(n + 16U); x <<= 16U; }
    if ((x & 0xFF000000UL) == 0UL) { n = (ui8_t)(n + 8U);  x <<= 8U;  }
    if ((x & 0xF0000000UL) == 0UL) { n = (ui8_t)(n + 4U);  x <<= 4U;  }
    if ((x & 0xC0000000UL) == 0UL) { n = (ui8_t)(n + 2U);  x <<= 2U;  }
    if ((x & 0x80000000UL) == 0UL) { n = (ui8_t)(n + 1U); }
    return n;
}

static inline ui8_t orbitc_ctz32_portable(ui32_t x)
{
    ui8_t n = 0U;
    if ((x & 0x0000FFFFUL) == 0UL) { n = (ui8_t)(n + 16U); x >>= 16U; }
    if ((x & 0x000000FFUL) == 0UL) { n = (ui8_t)(n + 8U);  x >>= 8U;  }
    if ((x & 0x0000000FUL) == 0UL) { n = (ui8_t)(n + 4U);  x >>= 4U;  }
    if ((x & 0x00000003UL) == 0UL) { n = (ui8_t)(n + 2U);  x >>= 2U;  }
    if ((x & 0x00000001UL) == 0UL) { n = (ui8_t)(n + 1U); }
    return n;
}

static inline ui32_t orbitc_make_mask32(ui8_t offset, ui8_t width)
{
    ui32_t base_mask = (width == 32U) ? 0xFFFFFFFFUL : (((ui32_t)1UL << width) - 1UL);
    return base_mask << offset;
}

/* ========================================================================== */
/* 32-Bit Single-Bit Operations                                               */
/* ========================================================================== */

status_t orbitc_bit32_set(ui32_t *word, ui8_t bit)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 31U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *word |= ((ui32_t)1UL << bit);
    }

    return st;
}

status_t orbitc_bit32_clear(ui32_t *word, ui8_t bit)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 31U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *word &= ~((ui32_t)1UL << bit);
    }

    return st;
}

status_t orbitc_bit32_toggle(ui32_t *word, ui8_t bit)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 31U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *word ^= ((ui32_t)1UL << bit);
    }

    return st;
}

status_t orbitc_bit32_test(ui32_t word, ui8_t bit, bool_t *out_set)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_set, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 31U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_set = ((word & ((ui32_t)1UL << bit)) != 0UL) ? ORBITC_TRUE : ORBITC_FALSE;
    }

    return st;
}

/* ========================================================================== */
/* 32-Bit Bitfield Operations                                                 */
/* ========================================================================== */

status_t orbitc_bitfield32_mask(ui8_t offset, ui8_t width, ui32_t *out_mask)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_mask, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        bool_t valid = ((width != 0U) && (offset < 32U) && (((ui32_t)offset + (ui32_t)width) <= 32U)) ? ORBITC_TRUE : ORBITC_FALSE;
        if (valid == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_BITOPS,
                    STATUS_CLS_OUT_OF_RANGE,
                    (ui8_t)BITOPS_CODE_BAD_FIELD));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_mask = orbitc_make_mask32(offset, width);
    }

    return st;
}

status_t orbitc_bitfield32_extract(ui32_t word, ui8_t offset, ui8_t width, ui32_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        bool_t valid = ((width != 0U) && (offset < 32U) && (((ui32_t)offset + (ui32_t)width) <= 32U)) ? ORBITC_TRUE : ORBITC_FALSE;
        if (valid == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_BITOPS,
                    STATUS_CLS_OUT_OF_RANGE,
                    (ui8_t)BITOPS_CODE_BAD_FIELD));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui32_t mask = orbitc_make_mask32(offset, width);
        *out_val = (word & mask) >> offset;
    }

    return st;
}

status_t orbitc_bitfield32_insert(ui32_t *word, ui8_t offset, ui8_t width, ui32_t field_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        bool_t valid = ((width != 0U) && (offset < 32U) && (((ui32_t)offset + (ui32_t)width) <= 32U)) ? ORBITC_TRUE : ORBITC_FALSE;
        if (valid == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_BITOPS,
                    STATUS_CLS_OUT_OF_RANGE,
                    (ui8_t)BITOPS_CODE_BAD_FIELD));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui32_t mask = orbitc_make_mask32(offset, width);
        *word = (*word & ~mask) | ((field_val << offset) & mask);
    }

    return st;
}

/* ========================================================================== */
/* 32-Bit Rotations, Reversal & Bit Scanning                                  */
/* ========================================================================== */

status_t orbitc_rotl32(ui32_t word, ui8_t shift, ui32_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui8_t s = (ui8_t)(shift & 31U);
        if (s == 0U)
        {
            *out_val = word;
        }
        else
        {
            *out_val = (word << s) | (word >> (32U - s));
        }
    }

    return st;
}

status_t orbitc_rotr32(ui32_t word, ui8_t shift, ui32_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui8_t s = (ui8_t)(shift & 31U);
        if (s == 0U)
        {
            *out_val = word;
        }
        else
        {
            *out_val = (word >> s) | (word << (32U - s));
        }
    }

    return st;
}

status_t orbitc_bitrev32(ui32_t word, ui32_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui32_t x = word;
        x = ((x >> 1U)  & 0x55555555UL) | ((x & 0x55555555UL) << 1U);
        x = ((x >> 2U)  & 0x33333333UL) | ((x & 0x33333333UL) << 2U);
        x = ((x >> 4U)  & 0x0F0F0F0FUL) | ((x & 0x0F0F0F0FUL) << 4U);
        x = ((x >> 8U)  & 0x00FF00FFUL) | ((x & 0x00FF00FFUL) << 8U);
        x = (x >> 16U) | (x << 16U);
        *out_val = x;
    }

    return st;
}

status_t orbitc_popcount32(ui32_t word, ui8_t *out_count)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_count, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
#if (ORBITC_BITOPS_BACKEND == ORBITC_BITOPS_BACKEND_BUILTIN) && (defined(__GNUC__) || defined(__clang__))
        *out_count = (ui8_t)__builtin_popcount((unsigned int)word);
#else
        *out_count = orbitc_popcount32_swar(word);
#endif
    }

    return st;
}

status_t orbitc_clz32(ui32_t word, ui8_t *out_count)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_count, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_param(
        (word != 0UL) ? ORBITC_TRUE : ORBITC_FALSE,
        &st,
        STATUS_SEV_ERROR,
        STATUS_MOD_BITOPS,
        (ui8_t)BITOPS_CODE_ZERO_INPUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
#if (ORBITC_BITOPS_BACKEND == ORBITC_BITOPS_BACKEND_BUILTIN) && (defined(__GNUC__) || defined(__clang__))
        *out_count = (ui8_t)__builtin_clz((unsigned int)word);
#else
        *out_count = orbitc_clz32_portable(word);
#endif
    }

    return st;
}

status_t orbitc_ctz32(ui32_t word, ui8_t *out_count)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_count, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_param(
        (word != 0UL) ? ORBITC_TRUE : ORBITC_FALSE,
        &st,
        STATUS_SEV_ERROR,
        STATUS_MOD_BITOPS,
        (ui8_t)BITOPS_CODE_ZERO_INPUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
#if (ORBITC_BITOPS_BACKEND == ORBITC_BITOPS_BACKEND_BUILTIN) && (defined(__GNUC__) || defined(__clang__))
        *out_count = (ui8_t)__builtin_ctz((unsigned int)word);
#else
        *out_count = orbitc_ctz32_portable(word);
#endif
    }

    return st;
}

status_t orbitc_parity32(ui32_t word, ui8_t *out_parity)
{
    status_t st = STATUS_OK;
    ui8_t    cnt = 0U;

    st = orbitc_popcount32(word, &cnt);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_parity = (ui8_t)(cnt & 1U);
    }

    return st;
}

/* ========================================================================== */
/* 64-Bit Operations                                                          */
/* ========================================================================== */
#if (ENABLE_BITOPS_64 == 1)

static inline ui64_t orbitc_make_mask64(ui8_t offset, ui8_t width)
{
    ui64_t base_mask = (width == 64U) ? 0xFFFFFFFFFFFFFFFFULL : (((ui64_t)1ULL << width) - 1ULL);
    return base_mask << offset;
}

status_t orbitc_bit64_set(ui64_t *word, ui8_t bit)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 63U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *word |= ((ui64_t)1ULL << bit);
    }

    return st;
}

status_t orbitc_bit64_clear(ui64_t *word, ui8_t bit)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 63U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *word &= ~((ui64_t)1ULL << bit);
    }

    return st;
}

status_t orbitc_bit64_toggle(ui64_t *word, ui8_t bit)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 63U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *word ^= ((ui64_t)1ULL << bit);
    }

    return st;
}

status_t orbitc_bit64_test(ui64_t word, ui8_t bit, bool_t *out_set)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_set, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_range_u32((ui32_t)bit, 0U, 63U, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_INVALID_BIT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_set = ((word & ((ui64_t)1ULL << bit)) != 0ULL) ? ORBITC_TRUE : ORBITC_FALSE;
    }

    return st;
}

status_t orbitc_bitfield64_extract(ui64_t word, ui8_t offset, ui8_t width, ui64_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        bool_t valid = ((width != 0U) && (offset < 64U) && (((ui32_t)offset + (ui32_t)width) <= 64U)) ? ORBITC_TRUE : ORBITC_FALSE;
        if (valid == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_BITOPS,
                    STATUS_CLS_OUT_OF_RANGE,
                    (ui8_t)BITOPS_CODE_BAD_FIELD));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui64_t mask = orbitc_make_mask64(offset, width);
        *out_val = (word & mask) >> offset;
    }

    return st;
}

status_t orbitc_bitfield64_insert(ui64_t *word, ui8_t offset, ui8_t width, ui64_t field_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(word, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        bool_t valid = ((width != 0U) && (offset < 64U) && (((ui32_t)offset + (ui32_t)width) <= 64U)) ? ORBITC_TRUE : ORBITC_FALSE;
        if (valid == ORBITC_FALSE)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_BITOPS,
                    STATUS_CLS_OUT_OF_RANGE,
                    (ui8_t)BITOPS_CODE_BAD_FIELD));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui64_t mask = orbitc_make_mask64(offset, width);
        *word = (*word & ~mask) | ((field_val << offset) & mask);
    }

    return st;
}

status_t orbitc_rotl64(ui64_t word, ui8_t shift, ui64_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui8_t s = (ui8_t)(shift & 63U);
        *out_val = (s == 0U) ? word : ((word << s) | (word >> (64U - s)));
    }

    return st;
}

status_t orbitc_rotr64(ui64_t word, ui8_t shift, ui64_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_val, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui8_t s = (ui8_t)(shift & 63U);
        *out_val = (s == 0U) ? word : ((word >> s) | (word << (64U - s)));
    }

    return st;
}

status_t orbitc_popcount64(ui64_t word, ui8_t *out_count)
{
    status_t st = STATUS_OK;
    ui8_t    lo = 0U;
    ui8_t    hi = 0U;

    orbitc_assert_not_null(out_count, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        (void)orbitc_popcount32((ui32_t)(word & 0xFFFFFFFFUL), &lo);
        (void)orbitc_popcount32((ui32_t)(word >> 32U), &hi);
        *out_count = (ui8_t)(lo + hi);
    }

    return st;
}

status_t orbitc_clz64(ui64_t word, ui8_t *out_count)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_count, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_param(
        (word != 0ULL) ? ORBITC_TRUE : ORBITC_FALSE,
        &st,
        STATUS_SEV_ERROR,
        STATUS_MOD_BITOPS,
        (ui8_t)BITOPS_CODE_ZERO_INPUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui32_t hi = (ui32_t)(word >> 32U);
        ui8_t  c  = 0U;

        if (hi != 0UL)
        {
            (void)orbitc_clz32(hi, &c);
            *out_count = c;
        }
        else
        {
            (void)orbitc_clz32((ui32_t)(word & 0xFFFFFFFFUL), &c);
            *out_count = (ui8_t)(32U + c);
        }
    }

    return st;
}

status_t orbitc_ctz64(ui64_t word, ui8_t *out_count)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(out_count, &st, STATUS_MOD_BITOPS, (ui8_t)BITOPS_CODE_NULL_PTR);
    orbitc_assert_param(
        (word != 0ULL) ? ORBITC_TRUE : ORBITC_FALSE,
        &st,
        STATUS_SEV_ERROR,
        STATUS_MOD_BITOPS,
        (ui8_t)BITOPS_CODE_ZERO_INPUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        ui32_t lo = (ui32_t)(word & 0xFFFFFFFFUL);
        ui8_t  c  = 0U;

        if (lo != 0UL)
        {
            (void)orbitc_ctz32(lo, &c);
            *out_count = c;
        }
        else
        {
            (void)orbitc_ctz32((ui32_t)(word >> 32U), &c);
            *out_count = (ui8_t)(32U + c);
        }
    }

    return st;
}

#endif /* ENABLE_BITOPS_64 == 1 */

ui32_t orbitc_bitops_active_backend(void)
{
    return (ui32_t)ORBITC_BITOPS_BACKEND;
}