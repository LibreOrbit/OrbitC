/*
 * ================================================================================
 * File: tests/test_containers.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_containers.h"

#if (ENABLE_CONTAINER_RINGBUF == 1)
static void test_ringbuf_reject_and_overwrite(void)
{
    byte_t    storage[4];
    ringbuf_t rb = {0};
    byte_t    out[4] = {0};
    byte_t    b = 0U;

    /* 1. Reject mode */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_init(&rb, storage, 4U, RINGBUF_MODE_REJECT));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_write(&rb, "ABCD", 4U));
    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_FULL, orbitc_status_get_code(orbitc_ringbuf_push_byte(&rb, (byte_t)'E')));

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_peek_byte(&rb, 2U, &b));
    TEST_ASSERT_EQUAL_HEX8((byte_t)'C', b);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_read(&rb, out, 4U));
    TEST_ASSERT_EQUAL_MEMORY("ABCD", out, 4U);
    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_EMPTY, orbitc_status_get_code(orbitc_ringbuf_pop_byte(&rb, &b)));

    /* 2. Overwrite mode (Blackbox flight recorder behavior) */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_init(&rb, storage, 4U, RINGBUF_MODE_OVERWRITE));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_write(&rb, "1234", 4U));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_push_byte(&rb, (byte_t)'5')); /* Drops '1' */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_push_byte(&rb, (byte_t)'6')); /* Drops '2' */

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_ringbuf_read(&rb, out, 4U));
    TEST_ASSERT_EQUAL_MEMORY("3456", out, 4U);
}
#endif

#if (ENABLE_CONTAINER_FIFO == 1)
typedef struct
{
    ui16_t apid;
    ui32_t timestamp;
} test_tc_msg_t;

static void test_fifo_struct_elements(void)
{
    test_tc_msg_t storage[3];
    fifo_t        q = {0};
    test_tc_msg_t m1 = { 0x101U, 1000UL };
    test_tc_msg_t m2 = { 0x102U, 2000UL };
    test_tc_msg_t out = { 0U, 0UL };

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_fifo_init(&q, storage, sizeof(test_tc_msg_t), 3U));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_fifo_push(&q, &m1));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_fifo_push(&q, &m2));

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_fifo_peek(&q, &out));
    TEST_ASSERT_EQUAL_HEX16(0x101U, out.apid);

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_fifo_pop(&q, &out));
    TEST_ASSERT_EQUAL_HEX16(0x101U, out.apid);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_fifo_pop(&q, &out));
    TEST_ASSERT_EQUAL_HEX16(0x102U, out.apid);

    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_EMPTY, orbitc_status_get_code(orbitc_fifo_pop(&q, &out)));
}
#endif

#if (ENABLE_CONTAINER_STACK == 1)
static void test_stack_lifo_order(void)
{
    ui32_t  storage[4];
    stack_t st = {0};
    ui32_t  v1 = 10UL, v2 = 20UL, v3 = 30UL, out = 0UL;

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_init(&st, storage, sizeof(ui32_t), 4U));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_push(&st, &v1));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_push(&st, &v2));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_push(&st, &v3));

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_pop(&st, &out));
    TEST_ASSERT_EQUAL_UINT32(30UL, out);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_pop(&st, &out));
    TEST_ASSERT_EQUAL_UINT32(20UL, out);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_stack_pop(&st, &out));
    TEST_ASSERT_EQUAL_UINT32(10UL, out);
    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_EMPTY, orbitc_status_get_code(orbitc_stack_pop(&st, &out)));
}
#endif

#if (ENABLE_CONTAINER_MEMPOOL == 1)
ORBITC_MEMPOOL_STORAGE_DECLARE(g_pool_buf, 32U, 4U);

static void test_mempool_o1_and_safety(void)
{
    mempool_t pool = {0};
    void     *b1 = ORBITC_NULL, *b2 = ORBITC_NULL, *b3 = ORBITC_NULL, *b4 = ORBITC_NULL, *b5 = ORBITC_NULL;
    usize_t   free_cnt = 0U;

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_init(&pool, g_pool_buf, 32U, 4U));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_alloc(&pool, &b1));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_alloc(&pool, &b2));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_alloc(&pool, &b3));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_alloc(&pool, &b4));

    /* Pool exhausted */
    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_EMPTY, orbitc_status_get_code(orbitc_mempool_alloc(&pool, &b5)));

    /* Misaligned pointer inside pool must be rejected */
    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_INVALID_BLOCK,
                            orbitc_status_get_code(orbitc_mempool_free(&pool, (byte_t *)b2 + 8U)));

    /* Valid free & Double-free detection */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_free(&pool, b2));
    TEST_ASSERT_EQUAL_UINT8(CONTAINERS_CODE_DOUBLE_FREE, orbitc_status_get_code(orbitc_mempool_free(&pool, b2)));

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_mempool_get_free_count(&pool, &free_cnt));
    TEST_ASSERT_EQUAL_UINT(1U, free_cnt);
}
#endif

void run_containers_tests(void)
{
#if (ENABLE_CONTAINER_RINGBUF == 1)
    RUN_TEST(test_ringbuf_reject_and_overwrite);
#endif
#if (ENABLE_CONTAINER_FIFO == 1)
    RUN_TEST(test_fifo_struct_elements);
#endif
#if (ENABLE_CONTAINER_STACK == 1)
    RUN_TEST(test_stack_lifo_order);
#endif
#if (ENABLE_CONTAINER_MEMPOOL == 1)
    RUN_TEST(test_mempool_o1_and_safety);
#endif
}