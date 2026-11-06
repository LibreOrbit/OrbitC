/*
 * ================================================================================
 * File: modules/endian/src/orbitc_endian.c
 * ================================================================================
 */
#include "orbitc_endian.h"

/* ========================================================================== */
/* 16-Bit & 32-Bit Big-Endian Serialization                                   */
/* ========================================================================== */

status_t orbitc_read_be16(const void *buf, usize_t cap, ui16_t *out_val)
{
    status_t      st = STATUS_OK;
    const byte_t *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if ((buf == ORBITC_NULL) || (out_val == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(2U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 2U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p        = (const byte_t *)buf;
    *out_val = (ui16_t)(((ui16_t)p[0] << 8U) | ((ui16_t)p[1]));
    return st;
}

status_t orbitc_read_be32(const void *buf, usize_t cap, ui32_t *out_val)
{
    status_t      st = STATUS_OK;
    const byte_t *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if ((buf == ORBITC_NULL) || (out_val == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(4U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 4U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p        = (const byte_t *)buf;
    *out_val = ((ui32_t)p[0] << 24U) |
               ((ui32_t)p[1] << 16U) |
               ((ui32_t)p[2] << 8U)  |
               ((ui32_t)p[3]);
    return st;
}

status_t orbitc_write_be16(void *buf, usize_t cap, ui16_t val)
{
    status_t st = STATUS_OK;
    byte_t  *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if (buf == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(2U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 2U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p    = (byte_t *)buf;
    p[0] = (byte_t)((val >> 8U) & 0xFFU);
    p[1] = (byte_t)(val & 0xFFU);
    return st;
}

status_t orbitc_write_be32(void *buf, usize_t cap, ui32_t val)
{
    status_t st = STATUS_OK;
    byte_t  *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if (buf == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(4U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 4U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p    = (byte_t *)buf;
    p[0] = (byte_t)((val >> 24U) & 0xFFUL);
    p[1] = (byte_t)((val >> 16U) & 0xFFUL);
    p[2] = (byte_t)((val >> 8U)  & 0xFFUL);
    p[3] = (byte_t)(val & 0xFFUL);
    return st;
}

/* ========================================================================== */
/* 16-Bit & 32-Bit Little-Endian Serialization                                */
/* ========================================================================== */

status_t orbitc_read_le16(const void *buf, usize_t cap, ui16_t *out_val)
{
    status_t      st = STATUS_OK;
    const byte_t *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if ((buf == ORBITC_NULL) || (out_val == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(2U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 2U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p        = (const byte_t *)buf;
    *out_val = (ui16_t)(((ui16_t)p[0]) | ((ui16_t)p[1] << 8U));
    return st;
}

status_t orbitc_read_le32(const void *buf, usize_t cap, ui32_t *out_val)
{
    status_t      st = STATUS_OK;
    const byte_t *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if ((buf == ORBITC_NULL) || (out_val == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(4U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 4U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p        = (const byte_t *)buf;
    *out_val = ((ui32_t)p[0])        |
               ((ui32_t)p[1] << 8U)  |
               ((ui32_t)p[2] << 16U) |
               ((ui32_t)p[3] << 24U);
    return st;
}

status_t orbitc_write_le16(void *buf, usize_t cap, ui16_t val)
{
    status_t st = STATUS_OK;
    byte_t  *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if (buf == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(2U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 2U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p    = (byte_t *)buf;
    p[0] = (byte_t)(val & 0xFFU);
    p[1] = (byte_t)((val >> 8U) & 0xFFU);
    return st;
}

status_t orbitc_write_le32(void *buf, usize_t cap, ui32_t val)
{
    status_t st = STATUS_OK;
    byte_t  *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if (buf == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }

    orbitc_assert_capacity(4U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 4U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p    = (byte_t *)buf;
    p[0] = (byte_t)(val & 0xFFUL);
    p[1] = (byte_t)((val >> 8U)  & 0xFFUL);
    p[2] = (byte_t)((val >> 16U) & 0xFFUL);
    p[3] = (byte_t)((val >> 24U) & 0xFFUL);
    return st;
}

/* ========================================================================== */
/* 64-Bit Serialization (Optional)                                            */
/* ========================================================================== */
#if (ENABLE_ENDIAN_64 == 1)

status_t orbitc_read_be64(const void *buf, usize_t cap, ui64_t *out_val)
{
    status_t      st = STATUS_OK;
    const byte_t *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if ((buf == ORBITC_NULL) || (out_val == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }
    orbitc_assert_capacity(8U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 8U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p        = (const byte_t *)buf;
    *out_val = ((ui64_t)p[0] << 56U) |
               ((ui64_t)p[1] << 48U) |
               ((ui64_t)p[2] << 40U) |
               ((ui64_t)p[3] << 32U) |
               ((ui64_t)p[4] << 24U) |
               ((ui64_t)p[5] << 16U) |
               ((ui64_t)p[6] << 8U)  |
               ((ui64_t)p[7]);
    return st;
}

status_t orbitc_write_be64(void *buf, usize_t cap, ui64_t val)
{
    status_t st = STATUS_OK;
    byte_t  *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if (buf == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }
    orbitc_assert_capacity(8U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 8U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p    = (byte_t *)buf;
    p[0] = (byte_t)((val >> 56U) & 0xFFULL);
    p[1] = (byte_t)((val >> 48U) & 0xFFULL);
    p[2] = (byte_t)((val >> 40U) & 0xFFULL);
    p[3] = (byte_t)((val >> 32U) & 0xFFULL);
    p[4] = (byte_t)((val >> 24U) & 0xFFULL);
    p[5] = (byte_t)((val >> 16U) & 0xFFULL);
    p[6] = (byte_t)((val >> 8U)  & 0xFFULL);
    p[7] = (byte_t)(val & 0xFFULL);
    return st;
}

status_t orbitc_read_le64(const void *buf, usize_t cap, ui64_t *out_val)
{
    status_t      st = STATUS_OK;
    const byte_t *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if ((buf == ORBITC_NULL) || (out_val == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }
    orbitc_assert_capacity(8U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 8U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p        = (const byte_t *)buf;
    *out_val = ((ui64_t)p[0])        |
               ((ui64_t)p[1] << 8U)  |
               ((ui64_t)p[2] << 16U) |
               ((ui64_t)p[3] << 24U) |
               ((ui64_t)p[4] << 32U) |
               ((ui64_t)p[5] << 40U) |
               ((ui64_t)p[6] << 48U) |
               ((ui64_t)p[7] << 56U);
    return st;
}

status_t orbitc_write_le64(void *buf, usize_t cap, ui64_t val)
{
    status_t st = STATUS_OK;
    byte_t  *p;

    orbitc_assert_not_null(buf, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_NULL_PTR);
    if (buf == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_NULL_PTR, ENDIAN_CODE_NULL_PTR);
    }
    orbitc_assert_capacity(8U, cap, &st, STATUS_MOD_ENDIAN, (ui8_t)ENDIAN_CODE_CAPACITY);
    if (cap < 8U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_ENDIAN, STATUS_CLS_CAPACITY, ENDIAN_CODE_CAPACITY);
    }

    p    = (byte_t *)buf;
    p[0] = (byte_t)(val & 0xFFULL);
    p[1] = (byte_t)((val >> 8U)  & 0xFFULL);
    p[2] = (byte_t)((val >> 16U) & 0xFFULL);
    p[3] = (byte_t)((val >> 24U) & 0xFFULL);
    p[4] = (byte_t)((val >> 32U) & 0xFFULL);
    p[5] = (byte_t)((val >> 40U) & 0xFFULL);
    p[6] = (byte_t)((val >> 48U) & 0xFFULL);
    p[7] = (byte_t)((val >> 56U) & 0xFFULL);
    return st;
}

#endif /* ENABLE_ENDIAN_64 == 1 */

ui32_t orbitc_endian_active_backend(void)
{
    return (ui32_t)ORBITC_ENDIAN_BACKEND;
}