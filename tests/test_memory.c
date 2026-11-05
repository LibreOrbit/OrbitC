/*
 * ================================================================================
 * File: tests/test_memory.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_memory.h"

static void test_memory_active_backend(void)
{
    ui32_t backend = orbitc_memory_active_backend();
    TEST_ASSERT_TRUE(backend <= 2U);
}

static void test_memory_memset_null_and_zero(void)
{
    byte_t buf[16];
    status_t st;

    st = orbitc_memset(ORBITC_NULL, 0xAAU, sizeof(buf));
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_CLS_NULL_PTR, (ui32_t)orbitc_status_get_class(st));

    st = orbitc_memset(buf, 0x55U, 0U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
}

static void test_memory_memset_aligned_and_unaligned(void)
{
    byte_t buf[128];
    usize_t offset;
    usize_t len;
    usize_t i;
    status_t st;

    for (offset = 0U; offset < 8U; ++offset)
    {
        for (len = 0U; len < 64U; ++len)
        {
            for (i = 0U; i < sizeof(buf); ++i)
            {
                buf[i] = 0x00U;
            }

            st = orbitc_memset(&buf[offset], 0xEEU, len);
            TEST_ASSERT_TRUE(orbitc_status_is_ok(st));

            for (i = 0U; i < offset; ++i)
            {
                TEST_ASSERT_EQUAL_HEX8(0x00U, buf[i]);
            }
            for (i = 0U; i < len; ++i)
            {
                TEST_ASSERT_EQUAL_HEX8(0xEEU, buf[offset + i]);
            }
            for (i = offset + len; i < sizeof(buf); ++i)
            {
                TEST_ASSERT_EQUAL_HEX8(0x00U, buf[i]);
            }
        }
    }
}

static void test_memory_memcpy_regular_and_errors(void)
{
    byte_t src[64];
    byte_t dst[64];
    usize_t i;
    status_t st;

    for (i = 0U; i < sizeof(src); ++i)
    {
        src[i] = (byte_t)(i + 1U);
        dst[i] = 0x00U;
    }

    st = orbitc_memcpy(ORBITC_NULL, src, sizeof(src));
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memcpy(dst, ORBITC_NULL, sizeof(src));
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memcpy(dst, src, 0U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));

    st = orbitc_memcpy(dst, src, sizeof(src));
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));

    for (i = 0U; i < sizeof(src); ++i)
    {
        TEST_ASSERT_EQUAL_HEX8(src[i], dst[i]);
    }

    st = orbitc_memcpy(&dst[0], &dst[4], 16U);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));
    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_CLS_OVERLAP, (ui32_t)orbitc_status_get_class(st));
}

static void test_memory_memmove_overlap(void)
{
    byte_t buf[32];
    usize_t i;
    status_t st;

    for (i = 0U; i < sizeof(buf); ++i)
    {
        buf[i] = (byte_t)i;
    }

    st = orbitc_memmove(&buf[4], &buf[0], 16U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    for (i = 0U; i < 16U; ++i)
    {
        TEST_ASSERT_EQUAL_HEX8((byte_t)i, buf[4U + i]);
    }

    for (i = 0U; i < sizeof(buf); ++i)
    {
        buf[i] = (byte_t)i;
    }

    st = orbitc_memmove(&buf[0], &buf[4], 16U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    for (i = 0U; i < 16U; ++i)
    {
        TEST_ASSERT_EQUAL_HEX8((byte_t)(i + 4U), buf[i]);
    }

    st = orbitc_memmove(buf, buf, 10U);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
}

static void test_memory_memcmp(void)
{
    byte_t a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    byte_t b[8] = {1, 2, 3, 4, 5, 6, 7, 9};
    byte_t c[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    si32_t diff = 0;
    status_t st;

    st = orbitc_memcmp(ORBITC_NULL, b, 8U, &diff);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memcmp(a, ORBITC_NULL, 8U, &diff);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memcmp(a, b, 8U, ORBITC_NULL);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memcmp(a, c, 8U, &diff);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    TEST_ASSERT_EQUAL_INT32(0, diff);

    st = orbitc_memcmp(a, b, 8U, &diff);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    TEST_ASSERT_TRUE(diff < 0);

    st = orbitc_memcmp(b, a, 8U, &diff);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    TEST_ASSERT_TRUE(diff > 0);

    st = orbitc_memcmp(a, b, 0U, &diff);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    TEST_ASSERT_EQUAL_INT32(0, diff);
}

static void test_memory_memchr(void)
{
    byte_t buf[16] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};
    const void *found = ORBITC_NULL;
    status_t st;

    st = orbitc_memchr(ORBITC_NULL, 50U, sizeof(buf), &found);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memchr(buf, 50U, sizeof(buf), ORBITC_NULL);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));

    st = orbitc_memchr(buf, 50U, sizeof(buf), &found);
    TEST_ASSERT_TRUE(orbitc_status_is_ok(st));
    TEST_ASSERT_EQUAL_PTR(&buf[4], found);

    st = orbitc_memchr(buf, 255U, sizeof(buf), &found);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));
    TEST_ASSERT_EQUAL_PTR(ORBITC_NULL, found);
    TEST_ASSERT_EQUAL_UINT32((ui32_t)STATUS_CLS_NOT_FOUND, (ui32_t)orbitc_status_get_class(st));

    st = orbitc_memchr(buf, 10U, 0U, &found);
    TEST_ASSERT_TRUE(orbitc_status_is_error(st));
}

void run_memory_tests(void)
{
    RUN_TEST(test_memory_active_backend);
    RUN_TEST(test_memory_memset_null_and_zero);
    RUN_TEST(test_memory_memset_aligned_and_unaligned);
    RUN_TEST(test_memory_memcpy_regular_and_errors);
    RUN_TEST(test_memory_memmove_overlap);
    RUN_TEST(test_memory_memcmp);
    RUN_TEST(test_memory_memchr);
}