/*
 * ================================================================================
 * File: modules/memory/include/orbitc_memory.h
 * ================================================================================
 */
#ifndef ORBITC_MEMORY_H
#define ORBITC_MEMORY_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

#define ORBITC_MEM_BACKEND_PORTABLE  0
#define ORBITC_MEM_BACKEND_GENERIC32 1
#define ORBITC_MEM_BACKEND_CORTEX_A9 2

#ifndef ORBITC_MEM_BACKEND
#define ORBITC_MEM_BACKEND ORBITC_MEM_BACKEND_GENERIC32
#endif

typedef enum
{
    MEMORY_CODE_OK        = 0x00U,
    MEMORY_CODE_NULL_DST  = 0x01U,
    MEMORY_CODE_NULL_SRC  = 0x02U,
    MEMORY_CODE_NULL_OUT  = 0x03U,
    MEMORY_CODE_OVERLAP   = 0x04U,
    MEMORY_CODE_OVERFLOW  = 0x05U,
    MEMORY_CODE_NOT_FOUND = 0x06U
} memory_code_t;

status_t orbitc_memset(void *dst, byte_t val, usize_t len);
status_t orbitc_memcpy(void *dst, const void *src, usize_t len);
status_t orbitc_memmove(void *dst, const void *src, usize_t len);
status_t orbitc_memcmp(const void *lhs, const void *rhs, usize_t len, si32_t *out_diff);
status_t orbitc_memchr(const void *buf, byte_t val, usize_t len, const void **out_found);
ui32_t   orbitc_memory_active_backend(void);

#endif