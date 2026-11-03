/*   
 *  ================================================================================
 *  File: modules/checksums/src/orbitc_checksums.c
 *  ================================================================================
 */
#include "orbitc_checksums.h"

#define ORBITC_CRC8_INIT  ((ui8_t)0x00U)
#define ORBITC_CRC8_POLY  ((ui8_t)0x07U)

#define ORBITC_CRC16_INIT ((ui16_t)0xFFFFU)
#define ORBITC_CRC16_POLY ((ui16_t)0x1021U)

#define ORBITC_CRC32_INIT ((ui32_t)0xFFFFFFFFUL)
#define ORBITC_CRC32_POLY ((ui32_t)0xEDB88320UL)

#if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

/*
 * Compact 16-entry nibble (4-bit) lookup tables for CRC-8 (0x07),
 * CRC-16-CCITT (0x1021), and IEEE 802.3 CRC-32 (reflected 0xEDB88320).
 * Processes two nibbles per byte for table-accelerated throughput in minimal ROM.
 */
static const ui8_t orbitc_crc8_nibble_table[16] =
{
    0x00U, 0x07U, 0x0EU, 0x09U, 0x1CU, 0x1BU, 0x12U, 0x15U,
    0x38U, 0x3FU, 0x36U, 0x31U, 0x24U, 0x23U, 0x2AU, 0x2DU
};

static const ui16_t orbitc_crc16_nibble_table[16] =
{
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U,
    0x8108U, 0x9129U, 0xA14AU, 0xB16BU, 0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU
};

static const ui32_t orbitc_crc32_nibble_table[16] =
{
    0x00000000UL, 0x1DB71064UL, 0x3B6E20C8UL, 0x26D930ACUL,
    0x76DC4190UL, 0x6B6B51F4UL, 0x4DB26158UL, 0x5005713CUL,
    0xEDB88320UL, 0xF00F9344UL, 0xD6D6A3E8UL, 0xCB61B38CUL,
    0x9B64C2B0UL, 0x86D3D2D4UL, 0xA00AE278UL, 0xBDBDF21CUL
};

static ui8_t orbitc_crc8_step(ui8_t crc, const byte_t *buf, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b = (byte_t)(crc ^ buf[i]);
        crc = (ui8_t)((ui8_t)(b << 4U) ^ orbitc_crc8_nibble_table[b >> 4U]);
        crc = (ui8_t)((ui8_t)(crc << 4U) ^ orbitc_crc8_nibble_table[crc >> 4U]);
    }
    return crc;
}

static ui16_t orbitc_crc16_step(ui16_t crc, const byte_t *buf, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b = buf[i];
        ui8_t hi = (ui8_t)(((crc >> 12U) ^ (ui16_t)(b >> 4U)) & 0x0FU);
        crc = (ui16_t)((ui16_t)(crc << 4U) ^ orbitc_crc16_nibble_table[hi]);
        ui8_t lo = (ui8_t)(((crc >> 12U) ^ (ui16_t)(b & 0x0FU)) & 0x0FU);
        crc = (ui16_t)((ui16_t)(crc << 4U) ^ orbitc_crc16_nibble_table[lo]);
    }
    return crc;
}

static ui32_t orbitc_crc32_step(ui32_t crc, const byte_t *buf, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b = buf[i];
        crc = (crc >> 4U) ^ orbitc_crc32_nibble_table[(crc ^ (ui32_t)(b & 0x0FU)) & 0x0FUL];
        crc = (crc >> 4U) ^ orbitc_crc32_nibble_table[(crc ^ (ui32_t)(b >> 4U)) & 0x0FUL];
    }
    return crc;
}

#else

static ui8_t orbitc_crc8_step(ui8_t crc, const byte_t *buf, usize_t len)
{
    usize_t i;
    ui8_t   bit;
    for (i = 0U; i < len; ++i)
    {
        crc ^= buf[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (ui8_t)((ui8_t)(crc << 1U) ^ ORBITC_CRC8_POLY);
            }
            else
            {
                crc = (ui8_t)(crc << 1U);
            }
        }
    }
    return crc;
}

static ui16_t orbitc_crc16_step(ui16_t crc, const byte_t *buf, usize_t len)
{
    usize_t i;
    ui8_t   bit;
    for (i = 0U; i < len; ++i)
    {
        crc ^= (ui16_t)((ui16_t)buf[i] << 8U);
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (ui16_t)((ui16_t)(crc << 1U) ^ ORBITC_CRC16_POLY);
            }
            else
            {
                crc = (ui16_t)(crc << 1U);
            }
        }
    }
    return crc;
}

static ui32_t orbitc_crc32_step(ui32_t crc, const byte_t *buf, usize_t len)
{
    usize_t i;
    ui8_t   bit;
    for (i = 0U; i < len; ++i)
    {
        crc ^= (ui32_t)buf[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 1UL) != 0UL)
            {
                crc = (crc >> 1U) ^ ORBITC_CRC32_POLY;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }
    return crc;
}

#endif

status_t orbitc_crc8_init(crc8_ctx_t *ctx)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    ctx->state = ORBITC_CRC8_INIT;
    return st;
}

status_t orbitc_crc8_update(crc8_ctx_t *ctx, const void *data, usize_t len)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_BUF);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    if (data == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_BUF);
    }
    if (len > 0U)
    {
        ctx->state = orbitc_crc8_step(ctx->state, (const byte_t *)data, len);
    }
    return st;
}

status_t orbitc_crc8_final(const crc8_ctx_t *ctx, ui8_t *out_crc)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(out_crc, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_OUT);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    if (out_crc == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_OUT);
    }
    *out_crc = ctx->state;
    return st;
}

status_t orbitc_crc8_compute(const void *data, usize_t len, ui8_t *out_crc)
{
    crc8_ctx_t ctx;
    status_t   st;

    st = orbitc_crc8_init(&ctx);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }
    st = orbitc_crc8_update(&ctx, data, len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }
    return orbitc_crc8_final(&ctx, out_crc);
}

status_t orbitc_crc16_init(crc16_ctx_t *ctx)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    ctx->state = ORBITC_CRC16_INIT;
    return st;
}

status_t orbitc_crc16_update(crc16_ctx_t *ctx, const void *data, usize_t len)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_BUF);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    if (data == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_BUF);
    }
    if (len > 0U)
    {
        ctx->state = orbitc_crc16_step(ctx->state, (const byte_t *)data, len);
    }
    return st;
}

status_t orbitc_crc16_final(const crc16_ctx_t *ctx, ui16_t *out_crc)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(out_crc, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_OUT);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    if (out_crc == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_OUT);
    }
    *out_crc = ctx->state;
    return st;
}

status_t orbitc_crc16_compute(const void *data, usize_t len, ui16_t *out_crc)
{
    crc16_ctx_t ctx;
    status_t    st;

    st = orbitc_crc16_init(&ctx);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }
    st = orbitc_crc16_update(&ctx, data, len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }
    return orbitc_crc16_final(&ctx, out_crc);
}

status_t orbitc_crc32_init(crc32_ctx_t *ctx)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    ctx->state = ORBITC_CRC32_INIT;
    return st;
}

status_t orbitc_crc32_update(crc32_ctx_t *ctx, const void *data, usize_t len)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_BUF);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    if (data == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_BUF);
    }
    if (len > 0U)
    {
        ctx->state = orbitc_crc32_step(ctx->state, (const byte_t *)data, len);
    }
    return st;
}

status_t orbitc_crc32_final(const crc32_ctx_t *ctx, ui32_t *out_crc)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(out_crc, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_OUT);
    if (ctx == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_CTX);
    }
    if (out_crc == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, STATUS_CLS_NULL_PTR, CHECKSUMS_CODE_NULL_OUT);
    }
    *out_crc = ctx->state ^ 0xFFFFFFFFUL;
    return st;
}

status_t orbitc_crc32_compute(const void *data, usize_t len, ui32_t *out_crc)
{
    crc32_ctx_t ctx;
    status_t    st;

    st = orbitc_crc32_init(&ctx);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }
    st = orbitc_crc32_update(&ctx, data, len);
    if (orbitc_status_is_ok(st) == ORBITC_FALSE)
    {
        return st;
    }
    return orbitc_crc32_final(&ctx, out_crc);
}

ui32_t orbitc_checksums_active_backend(void)
{
    return (ui32_t)ORBITC_CRC_BACKEND;
}