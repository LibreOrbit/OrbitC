/*
 * ================================================================================
 * File: tests/test_strings.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_strings.h"

static void test_strlen_s_all_cases(void)
{
    const char s[] = "OrbitC";
    const char unterm[4] = { 'A', 'B', 'C', 'D' };
    usize_t len = 99U;
    status_t st;

    st = orbitc_strlen_s(s, sizeof(s), &len);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_UINT32(6U, (ui32_t)len);

    st = orbitc_strlen_s("", 1U, &len);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_UINT32(0U, (ui32_t)len);

    /* Exact capacity equal to string length (missing room for null terminator) */
    st = orbitc_strlen_s(s, 6U, &len);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_UNTERMINATED, orbitc_status_get_code(st));

    st = orbitc_strlen_s(unterm, sizeof(unterm), &len);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strlen_s(s, 0U, &len);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_CAPACITY, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strlen_s(ORBITC_NULL, 5U, &len);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strlen_s(s, 5U, ORBITC_NULL);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));
}

static void test_strcpy_s_all_cases(void)
{
    char dst[16];
    const char src[] = "FlightSW";
    const char unterm[3] = { 'X', 'Y', 'Z' };
    status_t st;

    st = orbitc_strcpy_s(dst, sizeof(dst), src, sizeof(src));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_MEMORY(src, dst, sizeof(src));

    /* Truncation rejected and never hidden */
    dst[0] = 'A';
    st = orbitc_strcpy_s(dst, 5U, src, sizeof(src));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_CAPACITY, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_TRUNCATED, orbitc_status_get_code(st));
    TEST_ASSERT_EQUAL_INT('\0', dst[0]);

    /* Unterminated source rejected */
    st = orbitc_strcpy_s(dst, sizeof(dst), unterm, sizeof(unterm));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_INT('\0', dst[0]);

    /* Overlapping buffers rejected */
    st = orbitc_strcpy_s(dst, sizeof(dst), src, sizeof(src));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_strcpy_s(dst + 1U, 10U, dst, 9U);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_OVERLAP, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_OVERLAP, orbitc_status_get_code(st));

    /* Zero capacities and NULL pointers */
    st = orbitc_strcpy_s(dst, 0U, src, sizeof(src));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strcpy_s(dst, sizeof(dst), src, 0U);
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strcpy_s(ORBITC_NULL, sizeof(dst), src, sizeof(src));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strcpy_s(dst, sizeof(dst), ORBITC_NULL, sizeof(src));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));
}

static void test_strcat_s_all_cases(void)
{
    char dst[16];
    const char init_str[] = "Orbit";
    const char suffix[] = "-C99";
    const char unterm[3] = { '1', '2', '3' };
    status_t st;

    st = orbitc_strcpy_s(dst, sizeof(dst), init_str, sizeof(init_str));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);

    st = orbitc_strcat_s(dst, sizeof(dst), suffix, sizeof(suffix));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_MEMORY("Orbit-C99", dst, 10U);

    /* Exact boundary fit */
    st = orbitc_strcpy_s(dst, sizeof(dst), init_str, sizeof(init_str));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_strcat_s(dst, 10U, suffix, sizeof(suffix));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);

    /* Truncation by 1 byte rejected */
    st = orbitc_strcpy_s(dst, sizeof(dst), init_str, sizeof(init_str));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_strcat_s(dst, 9U, suffix, sizeof(suffix));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_CAPACITY, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_TRUNCATED, orbitc_status_get_code(st));

    /* Unterminated dst or src rejected */
    dst[0] = 'X';
    dst[1] = 'Y';
    st = orbitc_strcat_s(dst, 2U, suffix, sizeof(suffix));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strcpy_s(dst, sizeof(dst), init_str, sizeof(init_str));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_strcat_s(dst, sizeof(dst), unterm, sizeof(unterm));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    /* Overlap rejected */
    st = orbitc_strcat_s(dst, sizeof(dst), dst, 6U);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_OVERLAP, (ui8_t)orbitc_status_get_class(st));

    /* Zero cap and NULL */
    st = orbitc_strcat_s(dst, 0U, suffix, sizeof(suffix));
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strcat_s(ORBITC_NULL, sizeof(dst), suffix, sizeof(suffix));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strcat_s(dst, sizeof(dst), ORBITC_NULL, sizeof(suffix));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));
}

static void test_strcmp_s_all_cases(void)
{
    const char a[] = "Alpha";
    const char b[] = "Alpha";
    const char c[] = "Alpho";
    const char d[] = "AlphaBeta";
    const char unterm[3] = { 'A', 'l', 'p' };
    si32_t cmp = 99;
    status_t st;

    st = orbitc_strcmp_s(a, sizeof(a), b, sizeof(b), &cmp);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_INT(0, cmp);

    st = orbitc_strcmp_s(a, sizeof(a), c, sizeof(c), &cmp);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_TRUE(cmp < 0);

    st = orbitc_strcmp_s(c, sizeof(c), a, sizeof(a), &cmp);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_TRUE(cmp > 0);

    st = orbitc_strcmp_s(a, sizeof(a), d, sizeof(d), &cmp);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_TRUE(cmp < 0);

    /* Unterminated lhs or rhs */
    st = orbitc_strcmp_s(unterm, sizeof(unterm), a, sizeof(a), &cmp);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strcmp_s(a, sizeof(a), unterm, sizeof(unterm), &cmp);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    /* Zero cap and NULL */
    st = orbitc_strcmp_s(a, 0U, b, sizeof(b), &cmp);
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strcmp_s(ORBITC_NULL, sizeof(a), b, sizeof(b), &cmp);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strcmp_s(a, sizeof(a), ORBITC_NULL, sizeof(b), &cmp);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strcmp_s(a, sizeof(a), b, sizeof(b), ORBITC_NULL);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));
}

static void test_strchr_s_all_cases(void)
{
    const char s[] = "Telemetry";
    const char unterm[3] = { 'T', 'e', 'l' };
    const char *found = ORBITC_NULL;
    status_t st;

    st = orbitc_strchr_s(s, sizeof(s), 'm', &found);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_PTR(&s[4], found);

    /* Search for null terminator */
    st = orbitc_strchr_s(s, sizeof(s), '\0', &found);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_PTR(&s[9], found);

    /* Not found */
    st = orbitc_strchr_s(s, sizeof(s), 'z', &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NOT_FOUND, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_NULL(found);

    /* Unterminated */
    st = orbitc_strchr_s(unterm, sizeof(unterm), 'T', &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    /* Zero cap and NULL */
    st = orbitc_strchr_s(s, 0U, 'T', &found);
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strchr_s(ORBITC_NULL, sizeof(s), 'T', &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strchr_s(s, sizeof(s), 'T', ORBITC_NULL);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));
}

static void test_strstr_s_all_cases(void)
{
    const char hay[] = "CCSDS_123_Packet_Header";
    const char needle[] = "Packet";
    const char empty[] = "";
    const char missing[] = "Payload";
    const char unterm[4] = { 'C', 'C', 'S', 'D' };
    const char *found = ORBITC_NULL;
    status_t st;

    st = orbitc_strstr_s(hay, sizeof(hay), needle, sizeof(needle), &found);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_PTR(&hay[10], found);

    /* Empty needle returns start of haystack */
    st = orbitc_strstr_s(hay, sizeof(hay), empty, sizeof(empty), &found);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_PTR(hay, found);

    /* Needle not found */
    st = orbitc_strstr_s(hay, sizeof(hay), missing, sizeof(missing), &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NOT_FOUND, (ui8_t)orbitc_status_get_class(st));
    TEST_ASSERT_NULL(found);

    /* Needle longer than haystack */
    st = orbitc_strstr_s(needle, sizeof(needle), hay, sizeof(hay), &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NOT_FOUND, (ui8_t)orbitc_status_get_class(st));

    /* Unterminated haystack or needle */
    st = orbitc_strstr_s(unterm, sizeof(unterm), needle, sizeof(needle), &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strstr_s(hay, sizeof(hay), unterm, sizeof(unterm), &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_UNTERMINATED, (ui8_t)orbitc_status_get_class(st));

    /* Zero cap and NULL */
    st = orbitc_strstr_s(hay, 0U, needle, sizeof(needle), &found);
    TEST_ASSERT_EQUAL_UINT8(STRINGS_CODE_ZERO_CAP, orbitc_status_get_code(st));

    st = orbitc_strstr_s(ORBITC_NULL, sizeof(hay), needle, sizeof(needle), &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strstr_s(hay, sizeof(hay), ORBITC_NULL, sizeof(needle), &found);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));

    st = orbitc_strstr_s(hay, sizeof(hay), needle, sizeof(needle), ORBITC_NULL);
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(st));
}

void run_strings_tests(void)
{
    RUN_TEST(test_strlen_s_all_cases);
    RUN_TEST(test_strcpy_s_all_cases);
    RUN_TEST(test_strcat_s_all_cases);
    RUN_TEST(test_strcmp_s_all_cases);
    RUN_TEST(test_strchr_s_all_cases);
    RUN_TEST(test_strstr_s_all_cases);
}