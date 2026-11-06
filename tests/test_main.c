#include "unity.h"

/* Fonctions de setup/teardown obligatoires pour Unity */
void setUp(void) {}
void tearDown(void) {}

/* Prototypes des runners par module */
void run_types_tests(void);
void run_status_tests(void);
void run_memory_tests(void);
void run_strings_tests(void);
void run_checksums_tests(void);
void run_allocator_tests(void);
void run_bitops_tests(void);
void run_endian_tests(void);
void run_containers_tests(void);
int main(void)
{
    UNITY_BEGIN();

    run_types_tests();
    run_status_tests();
    run_memory_tests();
    run_strings_tests();
    run_checksums_tests();
    run_allocator_tests();
    run_bitops_tests();
    run_endian_tests();
    run_containers_tests();
    return UNITY_END();
}