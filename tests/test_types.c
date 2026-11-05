/*
 * ================================================================================
 * File: tests/test_types.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_types.h"

static void test_types_sizes(void)
{
    TEST_ASSERT_EQUAL_UINT(1U, sizeof(ui8_t));
    TEST_ASSERT_EQUAL_UINT(1U, sizeof(si8_t));
    TEST_ASSERT_EQUAL_UINT(1U, sizeof(byte_t));
    TEST_ASSERT_EQUAL_UINT(2U, sizeof(ui16_t));
    TEST_ASSERT_EQUAL_UINT(2U, sizeof(si16_t));
    TEST_ASSERT_EQUAL_UINT(4U, sizeof(ui32_t));
    TEST_ASSERT_EQUAL_UINT(4U, sizeof(si32_t));
    TEST_ASSERT_EQUAL_UINT(8U, sizeof(ui64_t));
    TEST_ASSERT_EQUAL_UINT(8U, sizeof(si64_t));
    TEST_ASSERT_EQUAL_UINT(4U, sizeof(f32_t));
    TEST_ASSERT_EQUAL_UINT(8U, sizeof(f64_t));
    TEST_ASSERT_EQUAL_UINT(sizeof(void *), sizeof(usize_t));
    TEST_ASSERT_EQUAL_UINT(sizeof(void *), sizeof(uptr_t));
    TEST_ASSERT_EQUAL_UINT(sizeof(void *), sizeof(sptr_t));
    TEST_ASSERT_EQUAL_UINT(sizeof(void *), sizeof(addr_t));
    TEST_ASSERT_EQUAL_UINT(sizeof(const void *), sizeof(caddr_t));
}

static void test_types_runtime_helpers(void)
{
    ui32_t bits = orbitc_types_arch_bits();
    TEST_ASSERT_TRUE((bits == 32U) || (bits == 64U));
    (void)orbitc_types_is_little_endian();
    (void)orbitc_types_unaligned_allowed();
    TEST_ASSERT_TRUE(ORBITC_TRUE == 1);
    TEST_ASSERT_TRUE(ORBITC_FALSE == 0);
}

void run_types_tests(void)
{
    RUN_TEST(test_types_sizes);
    RUN_TEST(test_types_runtime_helpers);
}