/*   
 *  ================================================================================
 *  File: modules/checksums/include/orbitc_checksums.h
 *  ================================================================================
 */
#ifndef ORBITC_CHECKSUMS_H
#define ORBITC_CHECKSUMS_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

#define ORBITC_CRC_BACKEND_BITWISE 0
#define ORBITC_CRC_BACKEND_TABLE   1

#ifndef ORBITC_CRC_BACKEND
#define ORBITC_CRC_BACKEND ORBITC_CRC_BACKEND_TABLE
#endif

typedef enum
{
    CHECKSUMS_CODE_OK       = 0x00U,
    CHECKSUMS_CODE_NULL_CTX = 0x01U,
    CHECKSUMS_CODE_NULL_BUF = 0x02U,
    CHECKSUMS_CODE_NULL_OUT = 0x03U
} checksums_code_t;

typedef struct
{
    ui8_t state;
} crc8_ctx_t;

typedef struct
{
    ui16_t state;
} crc16_ctx_t;

typedef struct
{
    ui32_t state;
} crc32_ctx_t;

status_t orbitc_crc8_init(crc8_ctx_t *ctx);
status_t orbitc_crc8_update(crc8_ctx_t *ctx, const void *data, usize_t len);
status_t orbitc_crc8_final(const crc8_ctx_t *ctx, ui8_t *out_crc);
status_t orbitc_crc8_compute(const void *data, usize_t len, ui8_t *out_crc);

status_t orbitc_crc16_init(crc16_ctx_t *ctx);
status_t orbitc_crc16_update(crc16_ctx_t *ctx, const void *data, usize_t len);
status_t orbitc_crc16_final(const crc16_ctx_t *ctx, ui16_t *out_crc);
status_t orbitc_crc16_compute(const void *data, usize_t len, ui16_t *out_crc);

status_t orbitc_crc32_init(crc32_ctx_t *ctx);
status_t orbitc_crc32_update(crc32_ctx_t *ctx, const void *data, usize_t len);
status_t orbitc_crc32_final(const crc32_ctx_t *ctx, ui32_t *out_crc);
status_t orbitc_crc32_compute(const void *data, usize_t len, ui32_t *out_crc);

ui32_t orbitc_checksums_active_backend(void);

#endif