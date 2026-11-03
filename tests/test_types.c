/*
 * ================================================================================
 * File: tests/test_types.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_types.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_types_sizes(void)
{
    TEST_ASSERT_EQUAL_UINT32(1U, (ui32_t)sizeof(ui8_t));
    TEST_ASSERT_EQUAL_UINT32(1U, (ui32_t)sizeof(si8_t));
    TEST_ASSERT_EQUAL_UINT32(1U, (ui32_t)sizeof(byte_t));
    TEST_ASSERT_EQUAL_UINT32(2U, (ui32_t)sizeof(ui16_t));
    TEST_ASSERT_EQUAL_UINT32(2U, (ui32_t)sizeof(si16_t));
    TEST_ASSERT_EQUAL_UINT32(4U, (ui32_t)sizeof(ui32_t));
    TEST_ASSERT_EQUAL_UINT32(4U, (ui32_t)sizeof(si32_t));
    TEST_ASSERT_EQUAL_UINT32(8U, (ui32_t)sizeof(ui64_t));
    TEST_ASSERT_EQUAL_UINT32(8U, (ui32_t)sizeof(si64_t));
    TEST_ASSERT_EQUAL_UINT32(4U, (ui32_t)sizeof(f32_t));
    TEST_ASSERT_EQUAL_UINT32(8U, (ui32_t)sizeof(f64_t));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)sizeof(void *), (ui32_t)sizeof(usize_t));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)sizeof(void *), (ui32_t)sizeof(uptr_t));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)sizeof(void *), (ui32_t)sizeof(sptr_t));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)sizeof(void *), (ui32_t)sizeof(addr_t));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)sizeof(const void *), (ui32_t)sizeof(caddr_t));
}

void test_types_runtime_helpers(void)
{
    ui32_t bits = orbitc_types_arch_bits();
    TEST_ASSERT_TRUE((bits == 32U) || (bits == 64U));

    (void)orbitc_types_is_little_endian();
    (void)orbitc_types_unaligned_allowed();
    TEST_ASSERT_TRUE(ORBITC_TRUE == 1);
    TEST_ASSERT_TRUE(ORBITC_FALSE == 0);
}