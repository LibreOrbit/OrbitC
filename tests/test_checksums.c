/*
 * ================================================================================
 * File: tests/test_checksums.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_checksums.h"

#if (ENABLE_CRC8 == 1) || (ENABLE_CRC16 == 1) || (ENABLE_CRC32 == 1)
static const byte_t k_check_vec[9] = {
    (byte_t)'1', (byte_t)'2', (byte_t)'3', (byte_t)'4', (byte_t)'5',
    (byte_t)'6', (byte_t)'7', (byte_t)'8', (byte_t)'9'
};
#endif

#if (ENABLE_CRC8 == 1)
static void test_crc8_variants_and_errors(void)
{
    ui8_t      crc_out = 0U;
    crc8_ctx_t ctx;
    status_t   st;

#if (ENABLE_CRC8_STANDARD == 1)
    st = orbitc_crc8_compute(CRC8_TYPE_STANDARD, k_check_vec, 9U, &crc_out);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX8(0xF4U, crc_out);
#else
    st = orbitc_crc8_compute(CRC8_TYPE_STANDARD, k_check_vec, 9U, &crc_out);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_INVALID_PARAM, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(CHECKSUMS_CODE_NOT_COMPILED, orbitc_status_get_code(st));
#endif

#if (ENABLE_CRC8_AUTOSAR == 1)
    st = orbitc_crc8_compute(CRC8_TYPE_AUTOSAR, k_check_vec, 9U, &crc_out);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX8(0xDFU, crc_out);
#else
    st = orbitc_crc8_init(&ctx, CRC8_TYPE_AUTOSAR);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_INVALID_PARAM, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(CHECKSUMS_CODE_NOT_COMPILED, orbitc_status_get_code(st));
#endif

    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_init(ORBITC_NULL, CRC8_TYPE_STANDARD)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_update(ORBITC_NULL, k_check_vec, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_final(ORBITC_NULL, &crc_out)));
}
#endif /* ENABLE_CRC8 == 1 */

#if (ENABLE_CRC16 == 1)
static void test_crc16_variants_and_errors(void)
{
    ui16_t      crc_one = 0U;
    ui16_t      crc_inc = 0U;
    crc16_ctx_t ctx;
    status_t    st;

#if (ENABLE_CRC16_CCITT == 1)
    st = orbitc_crc16_compute(CRC16_TYPE_CCITT, k_check_vec, 9U, &crc_one);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX16(0x29B1U, crc_one);

    st = orbitc_crc16_init(&ctx, CRC16_TYPE_CCITT);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc16_update(&ctx, &k_check_vec[0], 4U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc16_update(&ctx, &k_check_vec[4], 5U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc16_final(&ctx, &crc_inc);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX16(0x29B1U, crc_inc);
#else
    st = orbitc_crc16_compute(CRC16_TYPE_CCITT, k_check_vec, 9U, &crc_one);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_INVALID_PARAM, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(CHECKSUMS_CODE_NOT_COMPILED, orbitc_status_get_code(st));
#endif

#if (ENABLE_CRC16_IBM == 1)
    st = orbitc_crc16_compute(CRC16_TYPE_IBM, k_check_vec, 9U, &crc_one);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX16(0x4B37U, crc_one);
#else
    st = orbitc_crc16_init(&ctx, CRC16_TYPE_IBM);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_INVALID_PARAM, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(CHECKSUMS_CODE_NOT_COMPILED, orbitc_status_get_code(st));
#endif

    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_init(ORBITC_NULL, CRC16_TYPE_CCITT)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_update(ORBITC_NULL, k_check_vec, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_final(ORBITC_NULL, &crc_inc)));
}
#endif /* ENABLE_CRC16 == 1 */

#if (ENABLE_CRC32 == 1)
static void test_crc32_variants_and_errors(void)
{
    ui32_t      crc_out = 0UL;
    crc32_ctx_t ctx;
    status_t    st;

#if (ENABLE_CRC32_IEEE == 1)
    st = orbitc_crc32_compute(CRC32_TYPE_IEEE, k_check_vec, 9U, &crc_out);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, crc_out);
#else
    st = orbitc_crc32_compute(CRC32_TYPE_IEEE, k_check_vec, 9U, &crc_out);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_INVALID_PARAM, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(CHECKSUMS_CODE_NOT_COMPILED, orbitc_status_get_code(st));
#endif

#if (ENABLE_CRC32_CASTAGNOLI == 1)
    st = orbitc_crc32_compute(CRC32_TYPE_CASTAGNOLI, k_check_vec, 9U, &crc_out);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX32(0xE3069283UL, crc_out);
#else
    st = orbitc_crc32_init(&ctx, CRC32_TYPE_CASTAGNOLI);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_INVALID_PARAM, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(CHECKSUMS_CODE_NOT_COMPILED, orbitc_status_get_code(st));
#endif

    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_init(ORBITC_NULL, CRC32_TYPE_IEEE)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_update(ORBITC_NULL, k_check_vec, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_final(ORBITC_NULL, &crc_out)));
}
#endif /* ENABLE_CRC32 == 1 */

static void test_checksums_active_backend(void)
{
    ui32_t backend = orbitc_checksums_active_backend();
    TEST_ASSERT_EQUAL_UINT32((ui32_t)ORBITC_CRC_BACKEND, backend);
}

void run_checksums_tests(void)
{
#if (ENABLE_CRC8 == 1)
    RUN_TEST(test_crc8_variants_and_errors);
#endif
#if (ENABLE_CRC16 == 1)
    RUN_TEST(test_crc16_variants_and_errors);
#endif
#if (ENABLE_CRC32 == 1)
    RUN_TEST(test_crc32_variants_and_errors);
#endif
    RUN_TEST(test_checksums_active_backend);
}