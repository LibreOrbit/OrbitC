/*
 * ================================================================================
 * File: tests/test_endian.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_endian.h"

static void test_endian_bswap_primitives(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x3412U, orbitc_bswap16(0x1234U));
    TEST_ASSERT_EQUAL_HEX32(0x78563412UL, orbitc_bswap32(0x12345678UL));
#if (ENABLE_ENDIAN_64 == 1)
    TEST_ASSERT_EQUAL_HEX64(0xEFCDAB8967452301ULL, orbitc_bswap64(0x0123456789ABCDEFULL));
#endif
}

static void test_endian_unaligned_serialization(void)
{
    byte_t buf[16] = {0};
    ui16_t v16     = 0U;
    ui32_t v32     = 0UL;

    /* Deliberately write at odd/unaligned address &buf[1] */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_write_be32(&buf[1], 8U, 0xCAFEBABEUL));
    TEST_ASSERT_EQUAL_HEX8(0xCAU, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0xFEU, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0xBAU, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0xBEU, buf[4]);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_read_be32(&buf[1], 8U, &v32));
    TEST_ASSERT_EQUAL_HEX32(0xCAFEBABEUL, v32);

    /* Little-Endian unaligned test at &buf[3] */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_write_le16(&buf[3], 4U, 0xBEEFU));
    TEST_ASSERT_EQUAL_HEX8(0xEFU, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0xBEU, buf[4]);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_read_le16(&buf[3], 4U, &v16));
    TEST_ASSERT_EQUAL_HEX16(0xBEEFU, v16);

    /* Capacity violation check */
    TEST_ASSERT_EQUAL_UINT8(ENDIAN_CODE_CAPACITY, orbitc_status_get_code(orbitc_read_be32(buf, 3U, &v32)));
}

void run_endian_tests(void)
{
    RUN_TEST(test_endian_bswap_primitives);
    RUN_TEST(test_endian_unaligned_serialization);
}