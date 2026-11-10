/*
 * ================================================================================
 * File: modules/checksums/src/orbitc_checksums.c
 * ================================================================================
 */
#include "orbitc_checksums.h"

#if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
#include "crc8_tables.h"
#include "crc16_tables.h"
#include "crc32_tables.h"
#endif

/* ========================================================================== */
/* Polynomial, Seed, and XOR Constants                                        */
/* ========================================================================== */

/* CRC-8 Constants */
#define ORBITC_CRC8_STD_INIT         ((ui8_t)0x00U)
#define ORBITC_CRC8_STD_POLY         ((ui8_t)0x07U)
#define ORBITC_CRC8_STD_XOROUT       ((ui8_t)0x00U)

#define ORBITC_CRC8_AUTOSAR_INIT     ((ui8_t)0xFFU)
#define ORBITC_CRC8_AUTOSAR_POLY     ((ui8_t)0x2FU)
#define ORBITC_CRC8_AUTOSAR_XOROUT   ((ui8_t)0xFFU)

/* CRC-16 Constants */
#define ORBITC_CRC16_CCITT_INIT      ((ui16_t)0xFFFFU)
#define ORBITC_CRC16_CCITT_POLY      ((ui16_t)0x1021U)
#define ORBITC_CRC16_CCITT_XOROUT    ((ui16_t)0x0000U)

#define ORBITC_CRC16_IBM_INIT        ((ui16_t)0xFFFFU)
#define ORBITC_CRC16_IBM_POLY_REF    ((ui16_t)0xA001U)
#define ORBITC_CRC16_IBM_XOROUT      ((ui16_t)0x0000U)

/* CRC-32 Constants */
#define ORBITC_CRC32_IEEE_INIT       ((ui32_t)0xFFFFFFFFUL)
#define ORBITC_CRC32_IEEE_POLY_REF   ((ui32_t)0xEDB88320UL)
#define ORBITC_CRC32_IEEE_XOROUT     ((ui32_t)0xFFFFFFFFUL)

#define ORBITC_CRC32_CAST_INIT       ((ui32_t)0xFFFFFFFFUL)
#define ORBITC_CRC32_CAST_POLY_REF   ((ui32_t)0x82F63B78UL)
#define ORBITC_CRC32_CAST_XOROUT     ((ui32_t)0xFFFFFFFFUL)

/* ========================================================================== */
/* CRC-8 Implementation                                                       */
/* ========================================================================== */
#if (ENABLE_CRC8 == 1)

#if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

#if (ENABLE_CRC8_STANDARD == 1) || (ENABLE_CRC8_AUTOSAR == 1)
static ui8_t orbitc_crc8_step_table_normal(ui8_t crc, const byte_t *buf, usize_t len, const ui8_t *table)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b = (byte_t)(crc ^ buf[i]);
        crc = (ui8_t)((ui8_t)(b << 4U) ^ table[b >> 4U]);
        crc = (ui8_t)((ui8_t)(crc << 4U) ^ table[crc >> 4U]);
    }
    return crc;
}
#endif

#else /* ORBITC_CRC_BACKEND_BITWISE */

#if (ENABLE_CRC8_STANDARD == 1) || (ENABLE_CRC8_AUTOSAR == 1)
static ui8_t orbitc_crc8_step_bitwise_normal(ui8_t crc, const byte_t *buf, usize_t len, ui8_t poly)
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
                crc = (ui8_t)((ui8_t)(crc << 1U) ^ poly);
            }
            else
            {
                crc = (ui8_t)(crc << 1U);
            }
        }
    }
    return crc;
}
#endif

#endif /* ORBITC_CRC_BACKEND */

status_t orbitc_crc8_init(crc8_ctx_t *ctx, crc8_type_t type)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    
    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {

        ctx->type = type;

        switch (type)
        {
    #if (ENABLE_CRC8_STANDARD == 1)
            case CRC8_TYPE_STANDARD:
                ctx->state = ORBITC_CRC8_STD_INIT;
                break;
    #endif
    #if (ENABLE_CRC8_AUTOSAR == 1)
            case CRC8_TYPE_AUTOSAR:
                ctx->state = ORBITC_CRC8_AUTOSAR_INIT;
                break;
    #endif
            default:
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }
    return st;
}

status_t orbitc_crc8_update(crc8_ctx_t *ctx, const void *data, usize_t len)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_BUF);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        switch (ctx->type)
        {
    #if (ENABLE_CRC8_STANDARD == 1)
            case CRC8_TYPE_STANDARD:
                if (len > 0U)
                {
    #if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
                    ctx->state = orbitc_crc8_step_table_normal(ctx->state, (const byte_t *)data, len, orbitc_crc8_standard_nibble_table);
    #else
                    ctx->state = orbitc_crc8_step_bitwise_normal(ctx->state, (const byte_t *)data, len, ORBITC_CRC8_STD_POLY);
    #endif
                }
                break;
    #endif
    #if (ENABLE_CRC8_AUTOSAR == 1)
            case CRC8_TYPE_AUTOSAR:
                if (len > 0U)
                {
    #if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
                    ctx->state = orbitc_crc8_step_table_normal(ctx->state, (const byte_t *)data, len, orbitc_crc8_autosar_nibble_table);
    #else
                    ctx->state = orbitc_crc8_step_bitwise_normal(ctx->state, (const byte_t *)data, len, ORBITC_CRC8_AUTOSAR_POLY);
    #endif
                }
                break;
    #endif
            default:
                (void)len;
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }
    return st;
}

status_t orbitc_crc8_final(const crc8_ctx_t *ctx, ui8_t *out_crc)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(out_crc, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_OUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {

        switch (ctx->type)
        {
    #if (ENABLE_CRC8_STANDARD == 1)
            case CRC8_TYPE_STANDARD:
                *out_crc = (ui8_t)(ctx->state ^ ORBITC_CRC8_STD_XOROUT);
                break;
    #endif
    #if (ENABLE_CRC8_AUTOSAR == 1)
            case CRC8_TYPE_AUTOSAR:
                *out_crc = (ui8_t)(ctx->state ^ ORBITC_CRC8_AUTOSAR_XOROUT);
                break;
    #endif
            default:
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;  
        }
    }
    return st;
}

status_t orbitc_crc8_compute(crc8_type_t type, const void *data, usize_t len, ui8_t *out_crc)
{
    crc8_ctx_t ctx;
    status_t   st;
    status_t upd_st;
    status_t fin_st;

    st = orbitc_crc8_init(&ctx, type);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        upd_st = orbitc_crc8_update(&ctx, data, len);
        orbitc_status_latch(&st, upd_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        fin_st = orbitc_crc8_final(&ctx, out_crc);
        orbitc_status_latch(&st, fin_st);
    }

    return st;
}

#endif /* ENABLE_CRC8 == 1 */

/* ========================================================================== */
/* CRC-16 Implementation                                                      */
/* ========================================================================== */
#if (ENABLE_CRC16 == 1)

#if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

#if (ENABLE_CRC16_CCITT == 1)
static ui16_t orbitc_crc16_step_table_normal(ui16_t crc, const byte_t *buf, usize_t len, const ui16_t *table)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b  = buf[i];
        ui8_t  hi = (ui8_t)(((crc >> 12U) ^ (ui16_t)(b >> 4U)) & 0x0FU);
        crc = (ui16_t)((ui16_t)(crc << 4U) ^ table[hi]);
        ui8_t  lo = (ui8_t)(((crc >> 12U) ^ (ui16_t)(b & 0x0FU)) & 0x0FU);
        crc = (ui16_t)((ui16_t)(crc << 4U) ^ table[lo]);
    }
    return crc;
}
#endif

#if (ENABLE_CRC16_IBM == 1)
static ui16_t orbitc_crc16_step_table_reflected(ui16_t crc, const byte_t *buf, usize_t len, const ui16_t *table)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b = buf[i];
        crc = (ui16_t)((crc >> 4U) ^ table[(crc ^ (ui16_t)(b & 0x0FU)) & 0x0FU]);
        crc = (ui16_t)((crc >> 4U) ^ table[(crc ^ (ui16_t)(b >> 4U)) & 0x0FU]);
    }
    return crc;
}
#endif

#else /* ORBITC_CRC_BACKEND_BITWISE */

#if (ENABLE_CRC16_CCITT == 1)
static ui16_t orbitc_crc16_step_bitwise_normal(ui16_t crc, const byte_t *buf, usize_t len, ui16_t poly)
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
                crc = (ui16_t)((ui16_t)(crc << 1U) ^ poly);
            }
            else
            {
                crc = (ui16_t)(crc << 1U);
            }
        }
    }
    return crc;
}
#endif

#if (ENABLE_CRC16_IBM == 1)
static ui16_t orbitc_crc16_step_bitwise_reflected(ui16_t crc, const byte_t *buf, usize_t len, ui16_t poly_ref)
{
    usize_t i;
    ui8_t   bit;
    for (i = 0U; i < len; ++i)
    {
        crc ^= (ui16_t)buf[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (ui16_t)((crc >> 1U) ^ poly_ref);
            }
            else
            {
                crc = (ui16_t)(crc >> 1U);
            }
        }
    }
    return crc;
}
#endif

#endif /* ORBITC_CRC_BACKEND */

status_t orbitc_crc16_init(crc16_ctx_t *ctx, crc16_type_t type)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {

        ctx->type = type;

        switch (type)
        {
    #if (ENABLE_CRC16_CCITT == 1)
            case CRC16_TYPE_CCITT:
                ctx->state = ORBITC_CRC16_CCITT_INIT;
                break;
    #endif
    #if (ENABLE_CRC16_IBM == 1)
            case CRC16_TYPE_IBM:
                ctx->state = ORBITC_CRC16_IBM_INIT;
                break;
    #endif
            default:
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }
    return st;
}

status_t orbitc_crc16_update(crc16_ctx_t *ctx, const void *data, usize_t len)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_BUF);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        switch (ctx->type)
        {
    #if (ENABLE_CRC16_CCITT == 1)
            case CRC16_TYPE_CCITT:
                if (len > 0U)
                {
    #if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
                    ctx->state = orbitc_crc16_step_table_normal(ctx->state, (const byte_t *)data, len, orbitc_crc16_ccitt_nibble_table);
    #else
                    ctx->state = orbitc_crc16_step_bitwise_normal(ctx->state, (const byte_t *)data, len, ORBITC_CRC16_CCITT_POLY);
    #endif
                }
                break;
    #endif
    #if (ENABLE_CRC16_IBM == 1)
            case CRC16_TYPE_IBM:
                if (len > 0U)
                {
    #if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
                    ctx->state = orbitc_crc16_step_table_reflected(ctx->state, (const byte_t *)data, len, orbitc_crc16_ibm_nibble_table);
    #else
                    ctx->state = orbitc_crc16_step_bitwise_reflected(ctx->state, (const byte_t *)data, len, ORBITC_CRC16_IBM_POLY_REF);
    #endif
                }
                break;
    #endif
            default:
                (void)len;
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }
    return st;
}

status_t orbitc_crc16_final(const crc16_ctx_t *ctx, ui16_t *out_crc)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(out_crc, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_OUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        switch (ctx->type)
        {
    #if (ENABLE_CRC16_CCITT == 1)
            case CRC16_TYPE_CCITT:
                *out_crc = (ui16_t)(ctx->state ^ ORBITC_CRC16_CCITT_XOROUT);
                break;
    #endif
    #if (ENABLE_CRC16_IBM == 1)
            case CRC16_TYPE_IBM:
                *out_crc = (ui16_t)(ctx->state ^ ORBITC_CRC16_IBM_XOROUT);
                break;
    #endif
            default:
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }

    return st;
}

status_t orbitc_crc16_compute(crc16_type_t type, const void *data, usize_t len, ui16_t *out_crc)
{
    crc16_ctx_t ctx;
    status_t    st, upd_st, fin_st;

    st = orbitc_crc16_init(&ctx, type);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        upd_st = orbitc_crc16_update(&ctx, data, len);
        orbitc_status_latch(&st, upd_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        fin_st = orbitc_crc16_final(&ctx, out_crc);
        orbitc_status_latch(&st, fin_st);
    }

    return st;
}

#endif /* ENABLE_CRC16 == 1 */

/* ========================================================================== */
/* CRC-32 Implementation                                                      */
/* ========================================================================== */
#if (ENABLE_CRC32 == 1)

#if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

#if (ENABLE_CRC32_IEEE == 1) || (ENABLE_CRC32_CASTAGNOLI == 1)
static ui32_t orbitc_crc32_step_table_reflected(ui32_t crc, const byte_t *buf, usize_t len, const ui32_t *table)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        byte_t b = buf[i];
        crc = (crc >> 4U) ^ table[(crc ^ (ui32_t)(b & 0x0FU)) & 0x0FUL];
        crc = (crc >> 4U) ^ table[(crc ^ (ui32_t)(b >> 4U)) & 0x0FUL];
    }
    return crc;
}
#endif

#else /* ORBITC_CRC_BACKEND_BITWISE */

#if (ENABLE_CRC32_IEEE == 1) || (ENABLE_CRC32_CASTAGNOLI == 1)
static ui32_t orbitc_crc32_step_bitwise_reflected(ui32_t crc, const byte_t *buf, usize_t len, ui32_t poly_ref)
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
                crc = (crc >> 1U) ^ poly_ref;
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

#endif /* ORBITC_CRC_BACKEND */

status_t orbitc_crc32_init(crc32_ctx_t *ctx, crc32_type_t type)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {

        ctx->type = type;

        switch (type)
        {
    #if (ENABLE_CRC32_IEEE == 1)
            case CRC32_TYPE_IEEE:
                ctx->state = ORBITC_CRC32_IEEE_INIT;
                break;
    #endif
    #if (ENABLE_CRC32_CASTAGNOLI == 1)
            case CRC32_TYPE_CASTAGNOLI:
                ctx->state = ORBITC_CRC32_CAST_INIT;
                break;
    #endif
            default:
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }

    return st;
}

status_t orbitc_crc32_update(crc32_ctx_t *ctx, const void *data, usize_t len)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_BUF);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {

        switch (ctx->type)
        {
    #if (ENABLE_CRC32_IEEE == 1)
            case CRC32_TYPE_IEEE:
                if (len > 0U)
                {
    #if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
                    ctx->state = orbitc_crc32_step_table_reflected(ctx->state, (const byte_t *)data, len, orbitc_crc32_ieee_nibble_table);
    #else
                    ctx->state = orbitc_crc32_step_bitwise_reflected(ctx->state, (const byte_t *)data, len, ORBITC_CRC32_IEEE_POLY_REF);
    #endif
                }
                break;
    #endif
    #if (ENABLE_CRC32_CASTAGNOLI == 1)
            case CRC32_TYPE_CASTAGNOLI:
                if (len > 0U)
                {
    #if (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)
                    ctx->state = orbitc_crc32_step_table_reflected(ctx->state, (const byte_t *)data, len, orbitc_crc32_castagnoli_nibble_table);
    #else
                    ctx->state = orbitc_crc32_step_bitwise_reflected(ctx->state, (const byte_t *)data, len, ORBITC_CRC32_CAST_POLY_REF);
    #endif
                }
                break;
    #endif
            default:
                (void)len;
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }
    return st;
}

status_t orbitc_crc32_final(const crc32_ctx_t *ctx, ui32_t *out_crc)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(ctx, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_CTX);
    orbitc_assert_not_null(out_crc, &st, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NULL_OUT);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        switch (ctx->type)
        {
    #if (ENABLE_CRC32_IEEE == 1)
            case CRC32_TYPE_IEEE:
                *out_crc = ctx->state ^ ORBITC_CRC32_IEEE_XOROUT;
                break;
    #endif
    #if (ENABLE_CRC32_CASTAGNOLI == 1)
            case CRC32_TYPE_CASTAGNOLI:
                *out_crc = ctx->state ^ ORBITC_CRC32_CAST_XOROUT;
                break;
    #endif
            default:
                orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CHECKSUMS, (ui8_t)CHECKSUMS_CODE_NOT_COMPILED);
                break;
        }
    }

    return st;
}

status_t orbitc_crc32_compute(crc32_type_t type, const void *data, usize_t len, ui32_t *out_crc)
{
    crc32_ctx_t ctx;
    status_t    st, upd_st, fin_st;

    st = orbitc_crc32_init(&ctx, type);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        upd_st = orbitc_crc32_update(&ctx, data, len);
        orbitc_status_latch(&st, upd_st);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        fin_st = orbitc_crc32_final(&ctx, out_crc);
        orbitc_status_latch(&st, fin_st);
    }

    return st;
}

#endif /* ENABLE_CRC32 == 1 */

/* ========================================================================== */
/* Runtime Introspection                                                      */
/* ========================================================================== */

ui32_t orbitc_checksums_active_backend(void)
{
    return (ui32_t)ORBITC_CRC_BACKEND;
}