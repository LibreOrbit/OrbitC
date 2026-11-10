/**
 * @file orbitc_assert.h
 * @brief Defensive assertion routines and address-range validation for OrbitC.
 *
 * Provides non-terminating validation routines that report structured errors
 * through an in-flight status variable. The first detected non-OK status is
 * preserved.
 */

/**
 * @addtogroup status
 * @{
 */

#ifndef ORBITC_ASSERT_H
#define ORBITC_ASSERT_H

#include "orbitc_status.h"


/* ========================================================================== */
/* Memory Range Validation                                                    */
/* ========================================================================== */

/**
 * @brief Checks if two memory buffers overlap in the address space.
 *
 * Accurately accounts for pointer wraparound at address space limits.
 *
 * @param[in] ptr_a Starting pointer of the first memory block.
 * @param[in] len_a Length in bytes of the first memory block.
 * @param[in] ptr_b Starting pointer of the second memory block.
 * @param[in] len_b Length in bytes of the second memory block.
 *
 * @retval ORBITC_TRUE  If the buffers overlap or an arithmetic wrap is detected.
 * @retval ORBITC_FALSE If the memory blocks are fully disjoint or either pointer is NULL/length is zero.
 */
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

/* ========================================================================== */
/* Active Assertion Implementations                                           */
/* ========================================================================== */

/**
 * @brief Asserts that a required pointer argument is not null.
 *
 * If @p ptr is NULL and @p st is in an OK state, updates @p st to report
 * @ref STATUS_CLS_NULL_PTR with severity @ref STATUS_SEV_ERROR.
 *
 * @param[in]     ptr  Pointer address to validate.
 * @param[in,out] st   Status container to update upon violation.
 * @param[in]     mod  Module reporting the violation (@ref status_mod_t).
 * @param[in]     code Subsystem error code identifier.
 */
static inline void orbitc_assert_not_null(
    const void *ptr, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (ptr == ORBITC_NULL)
        {
            orbitc_status_latch(
            st,
            STATUS_MAKE(
                STATUS_SEV_ERROR,
                mod,
                STATUS_CLS_NULL_PTR,
                code));
        }
    }
}

/**
 * @brief Asserts that a boolean condition holds true.
 *
 * If @p cond is false and @p st is in an OK state, updates @p st with
 * @ref STATUS_CLS_INVALID_PARAM at the requested severity level.
 *
 * @param[in]     cond Boolean condition required to evaluate to @ref ORBITC_TRUE.
 * @param[in,out] st   Status container to update upon violation.
 * @param[in]     sev  Severity level to report on failure.
 * @param[in]     mod  Module reporting the violation (@ref status_mod_t).
 * @param[in]     code Subsystem error code identifier.
 */
static inline void orbitc_assert_param(
    bool_t cond, status_t *st, status_sev_t sev, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (cond == ORBITC_FALSE)
        {
            orbitc_status_latch(
            st,
            STATUS_MAKE(
                sev,
                mod,
                STATUS_CLS_INVALID_PARAM,
                code));
        }
    }
}

/**
 * @brief Asserts that an unsigned 32-bit integer is within an inclusive range [min, max].
 *
 * If @p val falls outside boundaries and @p st is OK, updates @p st with
 * @ref STATUS_CLS_OUT_OF_RANGE.
 *
 * @param[in]     val     Value to test.
 * @param[in]     min_val Lower inclusive bound.
 * @param[in]     max_val Upper inclusive bound.
 * @param[in,out] st      Status container to update upon violation.
 * @param[in]     mod     Module reporting the violation (@ref status_mod_t).
 * @param[in]     code    Subsystem error code identifier.
 */
static inline void orbitc_assert_range_u32(
    ui32_t val, ui32_t min_val, ui32_t max_val, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if ((val < min_val) || (val > max_val))
        {
            orbitc_status_latch(
            st,
            STATUS_MAKE(
                STATUS_SEV_ERROR,
                mod,
                STATUS_CLS_OUT_OF_RANGE,
                code));
        }
    }
}

/**
 * @brief Asserts that a required size does not exceed the available capacity.
 *
 * If @p required exceeds @p capacity and @p st is OK, updates @p st with
 * @ref STATUS_CLS_CAPACITY.
 *
 * @param[in]     required Requested capacity/size in bytes.
 * @param[in]     capacity Total available buffer capacity in bytes.
 * @param[in,out] st       Status container to update upon violation.
 * @param[in]     mod      Module reporting the violation (@ref status_mod_t).
 * @param[in]     code     Subsystem error code identifier.
 */
static inline void orbitc_assert_capacity(
    usize_t required, usize_t capacity, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (required > capacity)
        {
            orbitc_status_latch(
            st,
            STATUS_MAKE(
                STATUS_SEV_ERROR,
                mod,
                STATUS_CLS_CAPACITY,
                code));
        }
    }
}

/**
 * @brief Asserts that two memory buffers do not overlap in address space.
 *
 * Uses @ref orbitc_ranges_overlap to check intersections. On overlap and if @p st
 * is OK, updates @p st with @ref STATUS_CLS_OVERLAP.
 *
 * @param[in]     ptr_a Starting pointer of buffer A.
 * @param[in]     len_a Length in bytes of buffer A.
 * @param[in]     ptr_b Starting pointer of buffer B.
 * @param[in]     len_b Length in bytes of buffer B.
 * @param[in,out] st    Status container to update upon violation.
 * @param[in]     mod   Module reporting the violation (@ref status_mod_t).
 * @param[in]     code  Subsystem error code identifier.
 */
static inline void orbitc_assert_no_overlap(
    const void *ptr_a, usize_t len_a, const void *ptr_b, usize_t len_b,
    status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (orbitc_ranges_overlap(ptr_a, len_a, ptr_b, len_b) == ORBITC_TRUE)
        {
            orbitc_status_latch(
            st,
            STATUS_MAKE(
                STATUS_SEV_ERROR,
                mod,
                STATUS_CLS_OVERLAP,
                code));
        }
    }
}

/**
 * @brief Asserts that adding two sizes will not wrap around @ref ORBITC_USIZE_MAX.
 *
 * If the sum overflows and @p st is OK, updates @p st with @ref STATUS_CLS_OVERFLOW.
 *
 * @param[in]     lhs  Left-hand size operand.
 * @param[in]     rhs  Right-hand size operand.
 * @param[in,out] st   Status container to update upon violation.
 * @param[in]     mod  Module reporting the violation (@ref status_mod_t).
 * @param[in]     code Subsystem error code identifier.
 */
static inline void orbitc_assert_no_overflow_size_add(
    usize_t lhs, usize_t rhs, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (lhs > (ORBITC_USIZE_MAX - rhs))
        {
            orbitc_status_latch(
            st,
            STATUS_MAKE(
                STATUS_SEV_ERROR,
                mod,
                STATUS_CLS_OVERFLOW,
                code));
        }
    }
}

/**
 * @brief Asserts that a pointer range does not wrap around the address space.
 *
 * A null pointer is not reported by this helper and must be validated separately
 * with @ref orbitc_assert_not_null.
 *
 * @param[in]     ptr  Start address of the memory range.
 * @param[in]     len  Length of the memory range in bytes.
 * @param[in,out] st   Status receiving the first detected error.
 * @param[in]     mod  Module reporting the error.
 * @param[in]     code Module-specific error code.
 */
static inline void orbitc_assert_address_range(
    const void *ptr,
    usize_t len,
    status_t *st,
    status_mod_t mod,
    ui8_t code)
{
    uptr_t start;

    if (ptr != ORBITC_NULL)
    {
        start = (uptr_t)ptr;

        if (((uptr_t)len) > (ORBITC_UPTR_MAX - start))
        {
            orbitc_status_latch(
                st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    mod,
                    STATUS_CLS_OVERFLOW,
                    code));
        }
    }
}

/**
 * @brief Asserts that a buffer capacity is strictly non-zero.
 *
 * If @p cap is 0 and @p st is OK, updates @p st with @ref STATUS_CLS_CAPACITY.
 *
 * @param[in]     cap  Capacity to validate.
 * @param[in,out] st   Status container to update upon violation.
 * @param[in]     mod  Module reporting the violation (@ref status_mod_t).
 * @param[in]     code Subsystem error code identifier.
 */
static inline void orbitc_assert_non_zero_capacity(
    usize_t cap, status_t *st, status_mod_t mod, ui8_t code)
{
    if ((st != ORBITC_NULL) && (orbitc_status_is_ok(*st) == ORBITC_TRUE))
    {
        if (cap == 0U)
        {
            orbitc_status_latch(
                st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    mod,
                    STATUS_CLS_CAPACITY,
                    code));
        }
    }
}
#endif /* ORBITC_ASSERT_H */

/** @} */ /* End of group status */