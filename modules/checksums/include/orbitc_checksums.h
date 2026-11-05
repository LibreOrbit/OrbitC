/**
 * @file orbitc_checksums.h
 * @brief Cyclic Redundancy Check (CRC) calculation interfaces for OrbitC.
 *
 * This header provides streaming (init/update/final) and one-shot calculation
 * routines for CRC-8, CRC-16, and CRC-32 checksums. It supports selectable
 * calculation backends (table-driven for throughput vs. bitwise for minimal memory footprint).
 */

/**
 * @defgroup checksums Checksums
 * @brief Cyclic Redundancy Check (CRC) algorithms, contexts, and backend configurations.
 * @{
 */

#ifndef ORBITC_CHECKSUMS_H
#define ORBITC_CHECKSUMS_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Algorithm Implementation Backends                                          */
/* ========================================================================== */

/** @brief Bit-by-bit shift implementation backend (minimal footprint, lower throughput). */
#define ORBITC_CRC_BACKEND_BITWISE 0

/** @brief Precomputed lookup-table backend (higher throughput, uses ROM/RAM tables). */
#define ORBITC_CRC_BACKEND_TABLE   1

#ifndef ORBITC_CRC_BACKEND
/**
 * @brief Active CRC backend selector macro.
 *
 * Defaults to @ref ORBITC_CRC_BACKEND_TABLE. Can be overridden at compile time
 * (e.g., `-DORBITC_CRC_BACKEND=ORBITC_CRC_BACKEND_BITWISE`).
 */
#define ORBITC_CRC_BACKEND ORBITC_CRC_BACKEND_TABLE
#endif

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific status and error codes for checksum operations.
 */
typedef enum
{
    CHECKSUMS_CODE_OK       = 0x00U, /**< Operation completed successfully. */
    CHECKSUMS_CODE_NULL_CTX = 0x01U, /**< Context structure pointer argument is NULL. */
    CHECKSUMS_CODE_NULL_BUF = 0x02U, /**< Input data buffer pointer argument is NULL. */
    CHECKSUMS_CODE_NULL_OUT = 0x03U  /**< Output checksum destination pointer is NULL. */
} checksums_code_t;

/* ========================================================================== */
/* Context Structures                                                         */
/* ========================================================================== */

/**
 * @brief Running state context for incremental CRC-8 computation.
 */
typedef struct
{
    ui8_t state; /**< Intermediate 8-bit CRC accumulator state. */
} crc8_ctx_t;

/**
 * @brief Running state context for incremental CRC-16 computation.
 */
typedef struct
{
    ui16_t state; /**< Intermediate 16-bit CRC accumulator state. */
} crc16_ctx_t;

/**
 * @brief Running state context for incremental CRC-32 computation.
 */
typedef struct
{
    ui32_t state; /**< Intermediate 32-bit CRC accumulator state. */
} crc32_ctx_t;

/* ========================================================================== */
/* CRC-8 Interface                                                            */
/* ========================================================================== */

/**
 * @brief Initializes a CRC-8 context with default seed values.
 *
 * @param[out] ctx Pointer to the CRC-8 context structure to initialize.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc8_init(crc8_ctx_t *ctx);

/**
 * @brief Updates an ongoing CRC-8 calculation with a chunk of input data.
 *
 * @param[in,out] ctx  Pointer to the active CRC-8 context.
 * @param[in]     data Pointer to the input data buffer.
 * @param[in]     len  Number of bytes to process.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc8_update(crc8_ctx_t *ctx, const void *data, usize_t len);

/**
 * @brief Finalizes the CRC-8 calculation and retrieves the resulting checksum.
 *
 * Performs post-processing (e.g., final XOR if applicable) and outputs the checksum.
 *
 * @param[in]  ctx     Pointer to the active CRC-8 context.
 * @param[out] out_crc Pointer to store the computed 8-bit checksum value.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc8_final(const crc8_ctx_t *ctx, ui8_t *out_crc);

/**
 * @brief Computes a CRC-8 checksum over a contiguous memory buffer in a single pass.
 *
 * Equivalent to executing @ref orbitc_crc8_init, followed by @ref orbitc_crc8_update,
 * and @ref orbitc_crc8_final.
 *
 * @param[in]  data    Pointer to the input data buffer.
 * @param[in]  len     Number of bytes to process.
 * @param[out] out_crc Pointer to store the computed 8-bit checksum value.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc8_compute(const void *data, usize_t len, ui8_t *out_crc);

/* ========================================================================== */
/* CRC-16 Interface                                                           */
/* ========================================================================== */

/**
 * @brief Initializes a CRC-16 context with default seed values.
 *
 * @param[out] ctx Pointer to the CRC-16 context structure to initialize.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc16_init(crc16_ctx_t *ctx);

/**
 * @brief Updates an ongoing CRC-16 calculation with a chunk of input data.
 *
 * @param[in,out] ctx  Pointer to the active CRC-16 context.
 * @param[in]     data Pointer to the input data buffer.
 * @param[in]     len  Number of bytes to process.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc16_update(crc16_ctx_t *ctx, const void *data, usize_t len);

/**
 * @brief Finalizes the CRC-16 calculation and retrieves the resulting checksum.
 *
 * Performs post-processing and outputs the checksum.
 *
 * @param[in]  ctx     Pointer to the active CRC-16 context.
 * @param[out] out_crc Pointer to store the computed 16-bit checksum value.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc16_final(const crc16_ctx_t *ctx, ui16_t *out_crc);

/**
 * @brief Computes a CRC-16 checksum over a contiguous memory buffer in a single pass.
 *
 * Equivalent to executing @ref orbitc_crc16_init, followed by @ref orbitc_crc16_update,
 * and @ref orbitc_crc16_final.
 *
 * @param[in]  data    Pointer to the input data buffer.
 * @param[in]  len     Number of bytes to process.
 * @param[out] out_crc Pointer to store the computed 16-bit checksum value.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc16_compute(const void *data, usize_t len, ui16_t *out_crc);

/* ========================================================================== */
/* CRC-32 Interface                                                           */
/* ========================================================================== */

/**
 * @brief Initializes a CRC-32 context with default seed values.
 *
 * @param[out] ctx Pointer to the CRC-32 context structure to initialize.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc32_init(crc32_ctx_t *ctx);

/**
 * @brief Updates an ongoing CRC-32 calculation with a chunk of input data.
 *
 * @param[in,out] ctx  Pointer to the active CRC-32 context.
 * @param[in]     data Pointer to the input data buffer.
 * @param[in]     len  Number of bytes to process.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc32_update(crc32_ctx_t *ctx, const void *data, usize_t len);

/**
 * @brief Finalizes the CRC-32 calculation and retrieves the resulting checksum.
 *
 * Performs post-processing and outputs the checksum.
 *
 * @param[in]  ctx     Pointer to the active CRC-32 context.
 * @param[out] out_crc Pointer to store the computed 32-bit checksum value.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc32_final(const crc32_ctx_t *ctx, ui32_t *out_crc);

/**
 * @brief Computes a CRC-32 checksum over a contiguous memory buffer in a single pass.
 *
 * Equivalent to executing @ref orbitc_crc32_init, followed by @ref orbitc_crc32_update,
 * and @ref orbitc_crc32_final.
 *
 * @param[in]  data    Pointer to the input data buffer.
 * @param[in]  len     Number of bytes to process.
 * @param[out] out_crc Pointer to store the computed 32-bit checksum value.
 *
 * @return Status indicating success or null-pointer error.
 */
status_t orbitc_crc32_compute(const void *data, usize_t len, ui32_t *out_crc);

/* ========================================================================== */
/* Runtime Introspection                                                      */
/* ========================================================================== */

/**
 * @brief Queries the runtime or compiled CRC implementation backend.
 *
 * @return Identifier corresponding to the active backend:
 *         - @ref ORBITC_CRC_BACKEND_BITWISE
 *         - @ref ORBITC_CRC_BACKEND_TABLE
 */
ui32_t orbitc_checksums_active_backend(void);

#endif /* ORBITC_CHECKSUMS_H */

/** @} */ /* End of group checksums */