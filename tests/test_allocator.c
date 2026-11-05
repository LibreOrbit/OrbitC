/*
 * ================================================================================
 * File: tests/test_allocator.c
 * ================================================================================
 */
#include "unity.h"
#include "orbitc_allocator.h"

#define TEST_HEAP_SIZE 4096U

/* Uses the helper macro to declare a properly aligned static pool */
ORBITC_HEAP_STORAGE_DECLARE(g_test_pool, TEST_HEAP_SIZE);

#if (ALLOCATOR_ENABLE_LOCKS == 1)
typedef struct
{
    ui32_t enter_count;
    ui32_t exit_count;
    si32_t lock_depth;
} mock_mutex_t;

static void mock_enter_critical(void *arg)
{
    mock_mutex_t *m = (mock_mutex_t *)arg;
    if (m != ORBITC_NULL)
    {
        m->enter_count += 1U;
        m->lock_depth  += 1;
    }
}

static void mock_exit_critical(void *arg)
{
    mock_mutex_t *m = (mock_mutex_t *)arg;
    if (m != ORBITC_NULL)
    {
        m->exit_count += 1U;
        m->lock_depth -= 1;
    }
}
#endif

static void test_allocator_init_and_validation(void)
{
    orbitc_allocator_t alloc = {0};
    allocator_config_t cfg   = {0};
    status_t           st;

    /* Null checks */
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_allocator_init(ORBITC_NULL, &cfg)));
    TEST_ASSERT_EQUAL_UINT8(STATUS_CLS_NULL_PTR, (ui8_t)orbitc_status_get_class(orbitc_allocator_init(&alloc, ORBITC_NULL)));

    /* Misaligned start pointer */
    cfg.mem_start = (void *)(&g_test_pool[1]);
    cfg.mem_size  = TEST_HEAP_SIZE - 1U;
    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_BAD_ALIGNMENT, orbitc_status_get_code(st));

    /* Pool too small */
    cfg.mem_start = (void *)g_test_pool;
    cfg.mem_size  = 16U;
    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_INVALID_SIZE, orbitc_status_get_code(st));

    /* Valid initialization */
    cfg.mem_start = (void *)g_test_pool;
    cfg.mem_size  = TEST_HEAP_SIZE;
    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);

    /* Double initialization warning */
    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_ALREADY_INIT, orbitc_status_get_code(st));
}

static void test_allocator_alloc_free_and_coalescing(void)
{
    orbitc_allocator_t alloc = {0};
    allocator_config_t cfg   = {0};
    allocator_stats_t  stats;
    void              *p1 = ORBITC_NULL;
    void              *p2 = ORBITC_NULL;
    void              *p3 = ORBITC_NULL;
    status_t           st;

    cfg.mem_start = (void *)g_test_pool;
    cfg.mem_size  = TEST_HEAP_SIZE;

    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);

    /* Allocate 3 blocks */
    st = orbitc_alloc(&alloc, 100U, &p1);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_EQUAL_UINT(0U, ((uptr_t)p1) % ALLOCATOR_ALIGNMENT);

#if (ALLOCATOR_ENABLE_POISONING == 1)
    TEST_ASSERT_EQUAL_HEX8(0xAAU, ((byte_t *)p1)[0]);
#endif

    st = orbitc_alloc(&alloc, 200U, &p2);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    st = orbitc_alloc(&alloc, 300U, &p3);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);

    st = orbitc_allocator_get_stats(&alloc, &stats);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_UINT32(3U, stats.alloc_count);

#if (ALLOCATOR_ENABLE_FREE == 1) && (ALLOCATOR_POLICY != ORBITC_ALLOC_POLICY_BUMP)
    /* Invalid pointer free (offset inside block p2) */
    st = orbitc_free(&alloc, (byte_t *)p2 + ALLOCATOR_ALIGNMENT);
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_INVALID_PTR, orbitc_status_get_code(st));

    /* Free middle block p2, then p1, then p3 -> should coalesce back into a single block */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_free(&alloc, p2));

    /* Double free detection */
    st = orbitc_free(&alloc, p2);
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_DOUBLE_FREE, orbitc_status_get_code(st));

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_free(&alloc, p1));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_free(&alloc, p3));

    /* Verify full coalescing */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_allocator_get_stats(&alloc, &stats));
    TEST_ASSERT_EQUAL_UINT32(0U, stats.alloc_count);
    TEST_ASSERT_EQUAL_UINT32(0U, stats.used_bytes);
    TEST_ASSERT_EQUAL_UINT32(1U, stats.free_block_count);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_allocator_check_integrity(&alloc));
#endif
}

static void test_allocator_corruption_detection(void)
{
#if (ALLOCATOR_ENABLE_CANARIES == 1)
    orbitc_allocator_t alloc = {0};
    allocator_config_t cfg   = {0};
    void              *p1    = ORBITC_NULL;
    void              *p2    = ORBITC_NULL;
    byte_t            *overflow_ptr;
    status_t           st;

    cfg.mem_start = (void *)g_test_pool;
    cfg.mem_size  = TEST_HEAP_SIZE;

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_allocator_init(&alloc, &cfg));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_alloc(&alloc, 64U, &p1));
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_alloc(&alloc, 64U, &p2));

    /* Simulate a buffer overflow from p1 corrupting p2's block header canary */
    overflow_ptr = (byte_t *)p1;
    overflow_ptr[64] ^= 0xFFU;

    /* FDIR integrity check must immediately catch the corruption */
    st = orbitc_allocator_check_integrity(&alloc);
    TEST_ASSERT_EQUAL_UINT8(STATUS_SEV_FATAL, (ui8_t)orbitc_status_get_severity(st));
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_CORRUPTED, orbitc_status_get_code(st));
#endif
}

static void test_allocator_critical_section_hooks(void)
{
#if (ALLOCATOR_ENABLE_LOCKS == 1)
    orbitc_allocator_t alloc = {0};
    allocator_config_t cfg   = {0};
    mock_mutex_t       mtx   = {0U, 0U, 0};
    void              *ptr   = ORBITC_NULL;
    status_t           st;

    cfg.mem_start  = (void *)g_test_pool;
    cfg.mem_size   = TEST_HEAP_SIZE;
    cfg.enter_crit = mock_enter_critical;
    cfg.exit_crit  = ORBITC_NULL; /* Asymmetric: should fail! */
    cfg.lock_arg   = &mtx;

    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_UINT8(ALLOCATOR_CODE_BAD_LOCK_CFG, orbitc_status_get_code(st));

    /* Proper symmetric lock hooks */
    cfg.exit_crit = mock_exit_critical;
    st = orbitc_allocator_init(&alloc, &cfg);
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, st);
    TEST_ASSERT_EQUAL_UINT32(1U, mtx.enter_count);
    TEST_ASSERT_EQUAL_UINT32(1U, mtx.exit_count);
    TEST_ASSERT_EQUAL_INT32(0, mtx.lock_depth);

    /* Allocation must lock and unlock cleanly */
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_alloc(&alloc, 128U, &ptr));
    TEST_ASSERT_EQUAL_UINT32(2U, mtx.enter_count);
    TEST_ASSERT_EQUAL_UINT32(2U, mtx.exit_count);
    TEST_ASSERT_EQUAL_INT32(0, mtx.lock_depth);

#if (ALLOCATOR_ENABLE_FREE == 1) && (ALLOCATOR_POLICY != ORBITC_ALLOC_POLICY_BUMP)
    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_free(&alloc, ptr));
    TEST_ASSERT_EQUAL_UINT32(3U, mtx.enter_count);
    TEST_ASSERT_EQUAL_UINT32(3U, mtx.exit_count);
    TEST_ASSERT_EQUAL_INT32(0, mtx.lock_depth);
#endif

    TEST_ASSERT_EQUAL_HEX32(STATUS_OK, orbitc_allocator_check_integrity(&alloc));
    TEST_ASSERT_EQUAL_INT32(0, mtx.lock_depth);
#endif
}

void run_allocator_tests(void)
{
    RUN_TEST(test_allocator_init_and_validation);
    RUN_TEST(test_allocator_alloc_free_and_coalescing);
    RUN_TEST(test_allocator_corruption_detection);
    RUN_TEST(test_allocator_critical_section_hooks);
}