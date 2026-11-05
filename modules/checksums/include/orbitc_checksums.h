/**
 * @file orbitc_checksums.h
 * @brief Configurable Cyclic Redundancy Check (CRC) interfaces for OrbitC.
 *
 * This header provides streaming (init/update/final) and one-shot calculation
 * routines for CRC-8, CRC-16, and CRC-32 checksums. Both the CRC bit-widths
 * and individual polynomial lookup tables can be selectively enabled or stripped
 * at compile time via `checksum-config.mk`.
 *
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

/** @brief Precomputed lookup-table backend (higher throughput, uses ROM tables). */
#define ORBITC_CRC_BACKEND_TABLE   1

#ifndef ORBITC_CRC_BACKEND
/**
 * @brief Active CRC backend selector macro.
 * Defaults to @ref ORBITC_CRC_BACKEND_TABLE.
 */
#define ORBITC_CRC_BACKEND ORBITC_CRC_BACKEND_TABLE
#endif

/* ========================================================================== */
/* Default Compile-Time Feature Fallbacks                                     */
/* ========================================================================== */

#ifndef ENABLE_CRC8
#define ENABLE_CRC8 1
#endif

#ifndef ENABLE_CRC16
#define ENABLE_CRC16 1
#endif

#ifndef ENABLE_CRC32
#define ENABLE_CRC32 1
#endif

#ifndef ENABLE_CRC8_STANDARD
#define ENABLE_CRC8_STANDARD 1
#endif

#ifndef ENABLE_CRC8_AUTOSAR
#define ENABLE_CRC8_AUTOSAR 0
#endif

#ifndef ENABLE_CRC16_CCITT
#define ENABLE_CRC16_CCITT 1
#endif

#ifndef ENABLE_CRC16_IBM
#define ENABLE_CRC16_IBM 0
#endif

#ifndef ENABLE_CRC32_IEEE
#define ENABLE_CRC32_IEEE 1
#endif

#ifndef ENABLE_CRC32_CASTAGNOLI
#define ENABLE_CRC32_CASTAGNOLI 0
#endif

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific status and error codes for checksum operations.
 */
typedef enum
{
    CHECKSUMS_CODE_OK           = 0x00U, /**< Operation completed successfully. */
    CHECKSUMS_CODE_NULL_CTX     = 0x01U, /**< Context structure pointer argument is NULL. */
    CHECKSUMS_CODE_NULL_BUF     = 0x02U, /**< Input data buffer pointer argument is NULL. */
    CHECKSUMS_CODE_NULL_OUT     = 0x03U, /**< Output checksum destination pointer is NULL. */
    CHECKSUMS_CODE_NOT_COMPILED = 0x04U  /**< Requested CRC polynomial/table is not compiled in or invalid. */
} checksums_code_t;

/* ========================================================================== */
/* CRC-8 Interface                                                            */
/* ========================================================================== */
#if (ENABLE_CRC8 == 1)

/**
 * @brief Supported CRC-8 polynomial and algorithm variants.
 */
typedef enum
{
    CRC8_TYPE_STANDARD = 0, /**< CRC-8 Standard / SMBus (Poly: 0x07, Init: 0x00, XorOut: 0x00, Normal) */
    CRC8_TYPE_AUTOSAR  = 1  /**< CRC-8 AUTOSAR (Poly: 0x2F, Init: 0xFF, XorOut: 0xFF, Normal) */
} crc8_type_t;

/**
 * @brief Running state context for incremental CRC-8 computation.
 */
typedef struct
{
    ui8_t       state; /**< Intermediate 8-bit CRC accumulator state. */
    crc8_type_t type;  /**< Selected CRC-8 algorithm variant. */
} crc8_ctx_t;

/**
 * @brief Initializes a CRC-8 context for a specific polynomial variant.
 *
 * @param[out] ctx  Pointer to the CRC-8 context structure to initialize.
 * @param[in]  type CRC-8 variant identifier (@ref crc8_type_t).
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc8_init(crc8_ctx_t *ctx, crc8_type_t type);

/**
 * @brief Updates an ongoing CRC-8 calculation with a chunk of input data.
 *
 * @param[in,out] ctx  Pointer to the active CRC-8 context.
 * @param[in]     data Pointer to the input data buffer.
 * @param[in]     len  Number of bytes to process.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc8_update(crc8_ctx_t *ctx, const void *data, usize_t len);

/**
 * @brief Finalizes the CRC-8 calculation and retrieves the resulting checksum.
 *
 * @param[in]  ctx     Pointer to the active CRC-8 context.
 * @param[out] out_crc Pointer to store the computed 8-bit checksum value.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc8_final(const crc8_ctx_t *ctx, ui8_t *out_crc);

/**
 * @brief Computes a CRC-8 checksum over a contiguous memory buffer in a single pass.
 *
 * @param[in]  type    CRC-8 variant identifier (@ref crc8_type_t).
 * @param[in]  data    Pointer to the input data buffer.
 * @param[in]  len     Number of bytes to process.
 * @param[out] out_crc Pointer to store the computed 8-bit checksum value.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc8_compute(crc8_type_t type, const void *data, usize_t len, ui8_t *out_crc);

#endif /* ENABLE_CRC8 == 1 */

/* ========================================================================== */
/* CRC-16 Interface                                                           */
/* ========================================================================== */
#if (ENABLE_CRC16 == 1)

/**
 * @brief Supported CRC-16 polynomial and algorithm variants.
 */
typedef enum
{
    CRC16_TYPE_CCITT = 0, /**< CRC-16-CCITT-FALSE / CCSDS (Poly: 0x1021, Init: 0xFFFF, XorOut: 0x0000, Normal) */
    CRC16_TYPE_IBM   = 1  /**< CRC-16-IBM / Modbus (Poly: 0x8005 / Ref: 0xA001, Init: 0xFFFF, XorOut: 0x0000, Reflected) */
} crc16_type_t;

/**
 * @brief Running state context for incremental CRC-16 computation.
 */
typedef struct
{
    ui16_t       state; /**< Intermediate 16-bit CRC accumulator state. */
    crc16_type_t type;  /**< Selected CRC-16 algorithm variant. */
} crc16_ctx_t;

/**
 * @brief Initializes a CRC-16 context for a specific polynomial variant.
 *
 * @param[out] ctx  Pointer to the CRC-16 context structure to initialize.
 * @param[in]  type CRC-16 variant identifier (@ref crc16_type_t).
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc16_init(crc16_ctx_t *ctx, crc16_type_t type);

/**
 * @brief Updates an ongoing CRC-16 calculation with a chunk of input data.
 *
 * @param[in,out] ctx  Pointer to the active CRC-16 context.
 * @param[in]     data Pointer to the input data buffer.
 * @param[in]     len  Number of bytes to process.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc16_update(crc16_ctx_t *ctx, const void *data, usize_t len);

/**
 * @brief Finalizes the CRC-16 calculation and retrieves the resulting checksum.
 *
 * @param[in]  ctx     Pointer to the active CRC-16 context.
 * @param[out] out_crc Pointer to store the computed 16-bit checksum value.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc16_final(const crc16_ctx_t *ctx, ui16_t *out_crc);

/**
 * @brief Computes a CRC-16 checksum over a contiguous memory buffer in a single pass.
 *
 * @param[in]  type    CRC-16 variant identifier (@ref crc16_type_t).
 * @param[in]  data    Pointer to the input data buffer.
 * @param[in]  len     Number of bytes to process.
 * @param[out] out_crc Pointer to store the computed 16-bit checksum value.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc16_compute(crc16_type_t type, const void *data, usize_t len, ui16_t *out_crc);

#endif /* ENABLE_CRC16 == 1 */

/* ========================================================================== */
/* CRC-32 Interface                                                           */
/* ========================================================================== */
#if (ENABLE_CRC32 == 1)

/**
 * @brief Supported CRC-32 polynomial and algorithm variants.
 */
typedef enum
{
    CRC32_TYPE_IEEE       = 0, /**< CRC-32 IEEE 802.3 (Poly: 0x04C11DB7 / Ref: 0xEDB88320, Init: 0xFFFFFFFF, XorOut: 0xFFFFFFFF, Reflected) */
    CRC32_TYPE_CASTAGNOLI = 1  /**< CRC-32C Castagnoli (Poly: 0x1EDC6F41 / Ref: 0x82F63B78, Init: 0xFFFFFFFF, XorOut: 0xFFFFFFFF, Reflected) */
} crc32_type_t;

/**
 * @brief Running state context for incremental CRC-32 computation.
 */
typedef struct
{
    ui32_t       state; /**< Intermediate 32-bit CRC accumulator state. */
    crc32_type_t type;  /**< Selected CRC-32 algorithm variant. */
} crc32_ctx_t;

/**
 * @brief Initializes a CRC-32 context for a specific polynomial variant.
 *
 * @param[out] ctx  Pointer to the CRC-32 context structure to initialize.
 * @param[in]  type CRC-32 variant identifier (@ref crc32_type_t).
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc32_init(crc32_ctx_t *ctx, crc32_type_t type);

/**
 * @brief Updates an ongoing CRC-32 calculation with a chunk of input data.
 *
 * @param[in,out] ctx  Pointer to the active CRC-32 context.
 * @param[in]     data Pointer to the input data buffer.
 * @param[in]     len  Number of bytes to process.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc32_update(crc32_ctx_t *ctx, const void *data, usize_t len);

/**
 * @brief Finalizes the CRC-32 calculation and retrieves the resulting checksum.
 *
 * @param[in]  ctx     Pointer to the active CRC-32 context.
 * @param[out] out_crc Pointer to store the computed 32-bit checksum value.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc32_final(const crc32_ctx_t *ctx, ui32_t *out_crc);

/**
 * @brief Computes a CRC-32 checksum over a contiguous memory buffer in a single pass.
 *
 * @param[in]  type    CRC-32 variant identifier (@ref crc32_type_t).
 * @param[in]  data    Pointer to the input data buffer.
 * @param[in]  len     Number of bytes to process.
 * @param[out] out_crc Pointer to store the computed 32-bit checksum value.
 * @return Status indicating success, null-pointer error, or @ref CHECKSUMS_CODE_NOT_COMPILED.
 */
status_t orbitc_crc32_compute(crc32_type_t type, const void *data, usize_t len, ui32_t *out_crc);

#endif /* ENABLE_CRC32 == 1 */

/* ========================================================================== */
/* Runtime Introspection                                                      */
/* ========================================================================== */

/**
 * @brief Queries the compiled CRC implementation backend.
 *
 * @return Identifier corresponding to the active backend:
 *         - @ref ORBITC_CRC_BACKEND_BITWISE
 *         - @ref ORBITC_CRC_BACKEND_TABLE
 */
ui32_t orbitc_checksums_active_backend(void);

#endif /* ORBITC_CHECKSUMS_H */
/** @} */ /* End of group checksums */