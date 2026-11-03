/*
 * ================================================================================
 * File: tests/test_status.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

void test_status_make_and_decode(void)
{
    status_t st = orbitc_status_make(STATUS_SEV_ERROR, STATUS_MOD_MEMORY, STATUS_CLS_OVERLAP, 0x42U);

    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_SEV_ERROR, (ui32_t)orbitc_status_get_severity(st));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_MOD_MEMORY, (ui32_t)orbitc_status_get_module(st));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_CLS_OVERLAP, (ui32_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(0x42U, orbitc_status_get_code(st));

    TEST_ASSERT_FALSE(orbitc_status_is_ok(st));
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    TEST_ASSERT_TRUE(orbitc_status_is_ok(STATUS_OK));
    TEST_ASSERT_FALSE(orbitc_status_is_error(STATUS_OK));
}

void test_status_latching(void)
{
    status_t target = STATUS_OK;
    status_t first_err = orbitc_status_make(STATUS_SEV_ERROR, STATUS_MOD_STRINGS, STATUS_CLS_CAPACITY, 1U);
    status_t second_err = orbitc_status_make(STATUS_SEV_FATAL, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, 2U);

    orbitc_status_latch(&target, first_err);
    TEST_ASSERT_EQUAL_UINT32(first_err, target);

    orbitc_status_latch(&target, second_err);
    TEST_ASSERT_EQUAL_UINT32(first_err, target);
}

void test_assert_helpers(void)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null((void *)0x1234, &st, STATUS_MOD_STATUS, 10U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));

    orbitc_assert_not_null(ORBITC_NULL, &st, STATUS_MOD_STATUS, 10U);
    TEST_ASSERT_FALSE(orbitc_status_is_ok(st));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_CLS_NULL_PTR, (ui32_t)orbitc_status_get_class(st));

    st = STATUS_OK;
    orbitc_assert_param(ORBITC_TRUE, &st, STATUS_SEV_ERROR, STATUS_MOD_STATUS, 11U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_STATUS, 11U);
    TEST_ASSERT_FALSE(orbitc_status_is_ok(st));

    st = STATUS_OK;
    orbitc_assert_range_u32(50U, 10U, 100U, &st, STATUS_MOD_STATUS, 12U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    orbitc_assert_range_u32(5U, 10U, 100U, &st, STATUS_MOD_STATUS, 12U);
    TEST_ASSERT_FALSE(orbitc_status_is_ok(st));

    st = STATUS_OK;
    orbitc_assert_capacity(10U, 20U, &st, STATUS_MOD_STATUS, 13U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    orbitc_assert_capacity(30U, 20U, &st, STATUS_MOD_STATUS, 13U);
    TEST_ASSERT_FALSE(orbitc_status_is_ok(st));

    st = STATUS_OK;
    orbitc_assert_no_overflow_size_add(100U, 200U, &st, STATUS_MOD_STATUS, 14U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    orbitc_assert_no_overflow_size_add(ORBITC_USIZE_MAX, 1U, &st, STATUS_MOD_STATUS, 14U);
    TEST_ASSERT_FALSE(orbitc_status_is_ok(st));
}