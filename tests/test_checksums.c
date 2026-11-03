/*
 * ================================================================================
 * File: tests/test_checksums.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_checksums.h"

/* Standard ASCII test vector "123456789" */
static const byte_t k_check_vec[9] = {
    (byte_t)'1', (byte_t)'2', (byte_t)'3', (byte_t)'4', (byte_t)'5',
    (byte_t)'6', (byte_t)'7', (byte_t)'8', (byte_t)'9'
};

static void test_crc8_oneshot_and_incremental(void)
{
    ui8_t crc_one = 0U;
    ui8_t crc_inc = 0U;
    ui8_t crc_empty = 0xFFU;
    crc8_ctx_t ctx;
    status_t st;

    /* Standard CRC-8 (poly 0x07, init 0x00) over "123456789" == 0xF4 */
    st = orbitc_crc8_compute(k_check_vec, 9U, &crc_one);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX8(0xF4U, crc_one);

    /* Incremental update across multiple chunks */
    st = orbitc_crc8_init(&ctx);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc8_update(&ctx, &k_check_vec[0], 3U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc8_update(&ctx, &k_check_vec[3], 0U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc8_update(&ctx, &k_check_vec[3], 6U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc8_final(&ctx, &crc_inc);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX8(0xF4U, crc_inc);

    /* Empty buffer */
    st = orbitc_crc8_compute(k_check_vec, 0U, &crc_empty);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX8(0x00U, crc_empty);

    /* Error handling */
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_init(ORBITC_NULL)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_update(ORBITC_NULL, k_check_vec, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_update(&ctx, ORBITC_NULL, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_final(ORBITC_NULL, &crc_inc)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_final(&ctx, ORBITC_NULL)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_compute(ORBITC_NULL, 1U, &crc_one)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc8_compute(k_check_vec, 1U, ORBITC_NULL)));
}

static void test_crc16_oneshot_and_incremental(void)
{
    ui16_t crc_one = 0U;
    ui16_t crc_inc = 0U;
    ui16_t crc_empty = 0U;
    crc16_ctx_t ctx;
    status_t st;

    /* Standard CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) over "123456789" == 0x29B1 */
    st = orbitc_crc16_compute(k_check_vec, 9U, &crc_one);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX16(0x29B1U, crc_one);

    /* Incremental update */
    st = orbitc_crc16_init(&ctx);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc16_update(&ctx, &k_check_vec[0], 4U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc16_update(&ctx, &k_check_vec[4], 5U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc16_final(&ctx, &crc_inc);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX16(0x29B1U, crc_inc);

    /* Empty buffer */
    st = orbitc_crc16_compute(k_check_vec, 0U, &crc_empty);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX16(0xFFFFU, crc_empty);

    /* Error handling */
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_init(ORBITC_NULL)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_update(ORBITC_NULL, k_check_vec, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_update(&ctx, ORBITC_NULL, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_final(ORBITC_NULL, &crc_inc)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_final(&ctx, ORBITC_NULL)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_compute(ORBITC_NULL, 1U, &crc_one)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc16_compute(k_check_vec, 1U, ORBITC_NULL)));
}

static void test_crc32_oneshot_and_incremental(void)
{
    ui32_t crc_one = 0UL;
    ui32_t crc_inc = 0UL;
    ui32_t crc_empty = 1UL;
    crc32_ctx_t ctx;
    status_t st;

    /* Standard IEEE 802.3 CRC-32 over "123456789" == 0xCBF43926 */
    st = orbitc_crc32_compute(k_check_vec, 9U, &crc_one);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, crc_one);

    /* Incremental update */
    st = orbitc_crc32_init(&ctx);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc32_update(&ctx, &k_check_vec[0], 2U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc32_update(&ctx, &k_check_vec[2], 3U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc32_update(&ctx, &k_check_vec[5], 4U);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_crc32_final(&ctx, &crc_inc);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, crc_inc);

    /* Empty buffer */
    st = orbitc_crc32_compute(k_check_vec, 0U, &crc_empty);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_HEX32(0x00000000UL, crc_empty);

    /* Error handling */
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_init(ORBITC_NULL)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_update(ORBITC_NULL, k_check_vec, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_update(&ctx, ORBITC_NULL, 1U)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_final(ORBITC_NULL, &crc_inc)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_final(&ctx, ORBITC_NULL)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_compute(ORBITC_NULL, 1U, &crc_one)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_crc32_compute(k_check_vec, 1U, ORBITC_NULL)));
}

static void test_checksums_active_backend(void)
{
    ui32_t backend = orbitc_checksums_active_backend();
    TEST_ASSERT_EQUAL_UINT32((ui32_t)ORBITC_CRC_BACKEND, backend);
}

void run_checksums_tests(void)
{
    RUN_TEST(test_crc8_oneshot_and_incremental);
    RUN_TEST(test_crc16_oneshot_and_incremental);
    RUN_TEST(test_crc32_oneshot_and_incremental);
    RUN_TEST(test_checksums_active_backend);
}