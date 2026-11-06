/**
 * @file orbitc_containers.h
 * @brief Common status codes, configurations, and umbrella header for OrbitC Containers.
 *
 * @defgroup containers Containers
 * @brief Deterministic static containers (Ring Buffer, FIFO, Stack, Memory Pool).
 * @{
 */
#ifndef ORBITC_CONTAINERS_H
#define ORBITC_CONTAINERS_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Compile-Time Feature Fallbacks                                             */
/* ========================================================================== */

#ifndef ENABLE_CONTAINER_RINGBUF
#define ENABLE_CONTAINER_RINGBUF 1
#endif

#ifndef ENABLE_CONTAINER_FIFO
#define ENABLE_CONTAINER_FIFO 1
#endif

#ifndef ENABLE_CONTAINER_STACK
#define ENABLE_CONTAINER_STACK 1
#endif

#ifndef ENABLE_CONTAINER_MEMPOOL
#define ENABLE_CONTAINER_MEMPOOL 1
#endif

#ifndef CONTAINER_MEMPOOL_MAX_BLOCKS
#define CONTAINER_MEMPOOL_MAX_BLOCKS 256U
#endif

/* ========================================================================== */
/* Shared Status and Error Codes                                              */
/* ========================================================================== */

/**
 * @brief Specific diagnostic status codes for all container operations.
 */
typedef enum
{
    CONTAINERS_CODE_OK            = 0x00U, /**< Operation completed successfully. */
    CONTAINERS_CODE_NULL_PTR      = 0x01U, /**< Required pointer argument is NULL. */
    CONTAINERS_CODE_NOT_INIT      = 0x02U, /**< Container instance has not been initialized. */
    CONTAINERS_CODE_INVALID_PARAM = 0x03U, /**< Zero capacity, zero element size, or invalid mode. */
    CONTAINERS_CODE_FULL          = 0x04U, /**< Container is full; cannot push/write requested items. */
    CONTAINERS_CODE_EMPTY         = 0x05U, /**< Container is empty; cannot pop/read/peek. */
    CONTAINERS_CODE_ALIGNMENT     = 0x06U, /**< Buffer or block size violates alignment requirements. */
    CONTAINERS_CODE_INVALID_BLOCK = 0x07U, /**< Pointer does not belong to pool or is misaligned. */
    CONTAINERS_CODE_DOUBLE_FREE   = 0x08U, /**< Block was already released back to the memory pool. */
    CONTAINERS_CODE_OUT_OF_RANGE  = 0x09U  /**< Index or block count exceeds allowed limits. */
} containers_code_t;

/* ========================================================================== */
/* Sub-Module Headers                                                         */
/* ========================================================================== */

#if (ENABLE_CONTAINER_RINGBUF == 1)
#include "orbitc_ringbuf.h"
#endif

#if (ENABLE_CONTAINER_FIFO == 1)
#include "orbitc_fifo.h"
#endif

#if (ENABLE_CONTAINER_STACK == 1)
#include "orbitc_stack.h"
#endif

#if (ENABLE_CONTAINER_MEMPOOL == 1)
#include "orbitc_mempool.h"
#endif

#endif /* ORBITC_CONTAINERS_H */
/** @} */ /* End of group containers */