/*
 * ================================================================================
 * File: tests/test_bitops.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_bitops.h"

static void test_bitops_single_bits(void)
{
    ui32_t reg = 0U;
    bool_t is_set = ORBITC_FALSE;

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bit32_set(&reg, 0U));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bit32_set(&reg, 31U));
    TEST_ASSERT_EQUAL_HEX32(0x80000001UL, reg);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bit32_test(reg, 31U, &is_set));
    TEST_ASSERT_EQUAL_INT(ORBITC_TRUE, is_set);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bit32_toggle(&reg, 0U));
    TEST_ASSERT_EQUAL_HEX32(0x80000000UL, reg);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bit32_clear(&reg, 31U));
    TEST_ASSERT_EQUAL_HEX32(0x00000000UL, reg);

    /* Out of range bit index (32) must fail safely without UB */
    TEST_ASSERT_EQUAL_UINT8(BITOPS_CODE_INVALID_BIT, orbitc_status_get_code(orbitc_bit32_set(&reg, 32U)));
}

static void test_bitops_bitfields_ccsds(void)
{
    /* Simulate packing a 16-bit CCSDS Packet Primary Header ID word:
     * Bits 0..10 (11 bits): APID = 0x2A5
     * Bit  11    (1 bit)  : SecHdrFlag = 1
     * Bit  12    (1 bit)  : PacketType (TM=0, TC=1) = 1
     * Bits 13..15(3 bits) : Version = 0
     */
    ui32_t hdr = 0U;
    ui32_t apid_out = 0U;

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bitfield32_insert(&hdr, 0U, 11U, 0x2A5UL));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bitfield32_insert(&hdr, 11U, 1U, 1UL));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bitfield32_insert(&hdr, 12U, 1U, 1UL));
    TEST_ASSERT_EQUAL_HEX32(0x1AA5UL, hdr);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_bitfield32_extract(hdr, 0U, 11U, &apid_out));
    TEST_ASSERT_EQUAL_HEX32(0x2A5UL, apid_out);

    /* Invalid bitfield geometry (offset + width > 32) */
    TEST_ASSERT_EQUAL_UINT8(BITOPS_CODE_BAD_FIELD, orbitc_status_get_code(orbitc_bitfield32_extract(hdr, 28U, 8U, &apid_out)));
}

static void test_bitops_rotations_and_scans(void)
{
    ui32_t rot = 0U;
    ui8_t  cnt = 0U;

    /* Shift by 0 and 32 must not trigger C99 undefined behavior */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_rotl32(0x12345678UL, 0U, &rot));
    TEST_ASSERT_EQUAL_HEX32(0x12345678UL, rot);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_rotl32(0x12345678UL, 4U, &rot));
    TEST_ASSERT_EQUAL_HEX32(0x23456781UL, rot);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_rotr32(0x23456781UL, 4U, &rot));
    TEST_ASSERT_EQUAL_HEX32(0x12345678UL, rot);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_popcount32(0xF0F00F0FUL, &cnt));
    TEST_ASSERT_EQUAL_UINT8(16U, cnt);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_clz32(0x00010000UL, &cnt));
    TEST_ASSERT_EQUAL_UINT8(15U, cnt);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ctz32(0x00010000UL, &cnt));
    TEST_ASSERT_EQUAL_UINT8(16U, cnt);

    /* CLZ/CTZ on 0 must return BITOPS_CODE_ZERO_INPUT */
    TEST_ASSERT_EQUAL_UINT8(BITOPS_CODE_ZERO_INPUT, orbitc_status_get_code(orbitc_clz32(0UL, &cnt)));
    TEST_ASSERT_EQUAL_UINT8(BITOPS_CODE_ZERO_INPUT, orbitc_status_get_code(orbitc_ctz32(0UL, &cnt)));
}

void run_bitops_tests(void)
{
    RUN_TEST(test_bitops_single_bits);
    RUN_TEST(test_bitops_bitfields_ccsds);
    RUN_TEST(test_bitops_rotations_and_scans);
}