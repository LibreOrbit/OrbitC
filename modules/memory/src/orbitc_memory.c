/*
 * ================================================================================
 * File: modules/memory/src/orbitc_memory.c
 * ================================================================================
 */
#include "orbitc_memory.h"

static void orbitc_memset_portable(byte_t *dst, byte_t val, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        dst[i] = val;
    }
}

static void orbitc_memcpy_portable(byte_t *dst, const byte_t *src, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        dst[i] = src[i];
    }
}

static void orbitc_memmove_portable(byte_t *dst, const byte_t *src, usize_t len)
{
    usize_t i;
    if ((uptr_t)dst < (uptr_t)src)
    {
        for (i = 0U; i < len; ++i)
        {
            dst[i] = src[i];
        }
    }
    else if ((uptr_t)dst > (uptr_t)src)
    {
        for (i = len; i > 0U; --i)
        {
            dst[i - 1U] = src[i - 1U];
        }
    }
}

static si32_t orbitc_memcmp_portable(const byte_t *lhs, const byte_t *rhs, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return (si32_t)lhs[i] - (si32_t)rhs[i];
        }
    }
    return 0;
}

static const void *orbitc_memchr_portable(const byte_t *buf, byte_t val, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        if (buf[i] == val)
        {
            return (const void *)(&buf[i]);
        }
    }
    return ORBITC_NULL;
}

#if (ORBITC_MEM_BACKEND == ORBITC_MEM_BACKEND_GENERIC32) || \
    (ORBITC_MEM_BACKEND == ORBITC_MEM_BACKEND_CORTEX_A9)

static inline bool_t orbitc_is_aligned4(const void *ptr)
{
    return (((uptr_t)ptr & (uptr_t)3U) == 0U) ? ORBITC_TRUE : ORBITC_FALSE;
}

static inline ui32_t orbitc_repeat_byte32(byte_t val)
{
    ui32_t v = (ui32_t)val;
    return v | (v << 8U) | (v << 16U) | (v << 24U);
}

static inline ui32_t orbitc_load_u32_aligned(const byte_t *p)
{
#if (ORBITC_LITTLE_ENDIAN == 1)
    return ((ui32_t)p[0]) |
           (((ui32_t)p[1]) << 8U) |
           (((ui32_t)p[2]) << 16U) |
           (((ui32_t)p[3]) << 24U);
#else
    return (((ui32_t)p[0]) << 24U) |
           (((ui32_t)p[1]) << 16U) |
           (((ui32_t)p[2]) << 8U) |
           ((ui32_t)p[3]);
#endif
}

static inline void orbitc_store_u32_aligned(byte_t *p, ui32_t w)
{
#if (ORBITC_LITTLE_ENDIAN == 1)
    p[0] = (byte_t)(w & 0xFFU);
    p[1] = (byte_t)((w >> 8U) & 0xFFU);
    p[2] = (byte_t)((w >> 16U) & 0xFFU);
    p[3] = (byte_t)((w >> 24U) & 0xFFU);
#else
    p[0] = (byte_t)((w >> 24U) & 0xFFU);
    p[1] = (byte_t)((w >> 16U) & 0xFFU);
    p[2] = (byte_t)((w >> 8U) & 0xFFU);
    p[3] = (byte_t)(w & 0xFFU);
#endif
}

static inline bool_t orbitc_word_has_zero(ui32_t w)
{
    return (((w - 0x01010101UL) & (~w) & 0x80808080UL) != 0UL) ? ORBITC_TRUE : ORBITC_FALSE;
}

#endif

#if (ORBITC_MEM_BACKEND == ORBITC_MEM_BACKEND_GENERIC32)

#define ORBITC_G32_MIN_LEN 8U

static void orbitc_memset_backend(byte_t *dst, byte_t val, usize_t len)
{
    ui32_t w;
    if (len < ORBITC_G32_MIN_LEN)
    {
        orbitc_memset_portable(dst, val, len);
        return;
    }

    while ((len > 0U) && (orbitc_is_aligned4(dst) == ORBITC_FALSE))
    {
        *dst = val;
        ++dst;
        --len;
    }

    w = orbitc_repeat_byte32(val);
    while (len >= 16U)
    {
        orbitc_store_u32_aligned(dst + 0U,  w);
        orbitc_store_u32_aligned(dst + 4U,  w);
        orbitc_store_u32_aligned(dst + 8U,  w);
        orbitc_store_u32_aligned(dst + 12U, w);
        dst += 16U;
        len -= 16U;
    }

    while (len >= 4U)
    {
        orbitc_store_u32_aligned(dst, w);
        dst += 4U;
        len -= 4U;
    }

    if (len > 0U)
    {
        orbitc_memset_portable(dst, val, len);
    }
}

static void orbitc_memcpy_backend(byte_t *dst, const byte_t *src, usize_t len)
{
    if (len < ORBITC_G32_MIN_LEN)
    {
        orbitc_memcpy_portable(dst, src, len);
        return;
    }

    if (((uptr_t)dst & 3U) != ((uptr_t)src & 3U))
    {
        orbitc_memcpy_portable(dst, src, len);
        return;
    }

    while ((len > 0U) && (orbitc_is_aligned4(dst) == ORBITC_FALSE))
    {
        *dst = *src;
        ++dst;
        ++src;
        --len;
    }

    while (len >= 16U)
    {
        ui32_t w0 = orbitc_load_u32_aligned(src + 0U);
        ui32_t w1 = orbitc_load_u32_aligned(src + 4U);
        ui32_t w2 = orbitc_load_u32_aligned(src + 8U);
        ui32_t w3 = orbitc_load_u32_aligned(src + 12U);
        orbitc_store_u32_aligned(dst + 0U,  w0);
        orbitc_store_u32_aligned(dst + 4U,  w1);
        orbitc_store_u32_aligned(dst + 8U,  w2);
        orbitc_store_u32_aligned(dst + 12U, w3);
        src += 16U;
        dst += 16U;
        len -= 16U;
    }

    while (len >= 4U)
    {
        orbitc_store_u32_aligned(dst, orbitc_load_u32_aligned(src));
        src += 4U;
        dst += 4U;
        len -= 4U;
    }

    if (len > 0U)
    {
        orbitc_memcpy_portable(dst, src, len);
    }
}

static void orbitc_memmove_backend(byte_t *dst, const byte_t *src, usize_t len)
{
    if (dst == src)
    {
        return;
    }

    if (orbitc_ranges_overlap(dst, len, src, len) == ORBITC_FALSE)
    {
        orbitc_memcpy_backend(dst, src, len);
        return;
    }

    if ((len < ORBITC_G32_MIN_LEN) || (((uptr_t)dst & 3U) != ((uptr_t)src & 3U)))
    {
        orbitc_memmove_portable(dst, src, len);
        return;
    }

    if ((uptr_t)dst < (uptr_t)src)
    {
        while ((len > 0U) && (orbitc_is_aligned4(dst) == ORBITC_FALSE))
        {
            *dst = *src;
            ++dst;
            ++src;
            --len;
        }
        while (len >= 4U)
        {
            orbitc_store_u32_aligned(dst, orbitc_load_u32_aligned(src));
            dst += 4U;
            src += 4U;
            len -= 4U;
        }
        if (len > 0U)
        {
            orbitc_memmove_portable(dst, src, len);
        }
    }
    else
    {
        byte_t       *d_end = dst + len;
        const byte_t *s_end = src + len;

        while ((len > 0U) && (orbitc_is_aligned4(d_end) == ORBITC_FALSE))
        {
            --d_end;
            --s_end;
            *d_end = *s_end;
            --len;
        }
        while (len >= 4U)
        {
            d_end -= 4U;
            s_end -= 4U;
            orbitc_store_u32_aligned(d_end, orbitc_load_u32_aligned(s_end));
            len -= 4U;
        }
        if (len > 0U)
        {
            orbitc_memmove_portable(dst, src, len);
        }
    }
}

static si32_t orbitc_memcmp_backend(const byte_t *lhs, const byte_t *rhs, usize_t len)
{
    if ((len < ORBITC_G32_MIN_LEN) || (((uptr_t)lhs & 3U) != ((uptr_t)rhs & 3U)))
    {
        return orbitc_memcmp_portable(lhs, rhs, len);
    }

    while ((len > 0U) && (orbitc_is_aligned4(lhs) == ORBITC_FALSE))
    {
        if (*lhs != *rhs)
        {
            return (si32_t)(*lhs) - (si32_t)(*rhs);
        }
        ++lhs;
        ++rhs;
        --len;
    }

    while (len >= 4U)
    {
        ui32_t wl = orbitc_load_u32_aligned(lhs);
        ui32_t wr = orbitc_load_u32_aligned(rhs);
        if (wl != wr)
        {
            return orbitc_memcmp_portable(lhs, rhs, 4U);
        }
        lhs += 4U;
        rhs += 4U;
        len -= 4U;
    }

    return orbitc_memcmp_portable(lhs, rhs, len);
}

static const void *orbitc_memchr_backend(const byte_t *buf, byte_t val, usize_t len)
{
    ui32_t mask;
    if (len < ORBITC_G32_MIN_LEN)
    {
        return orbitc_memchr_portable(buf, val, len);
    }

    while ((len > 0U) && (orbitc_is_aligned4(buf) == ORBITC_FALSE))
    {
        if (*buf == val)
        {
            return (const void *)buf;
        }
        ++buf;
        --len;
    }

    mask = orbitc_repeat_byte32(val);
    while (len >= 4U)
    {
        ui32_t w = orbitc_load_u32_aligned(buf) ^ mask;
        if (orbitc_word_has_zero(w) == ORBITC_TRUE)
        {
            return orbitc_memchr_portable(buf, val, 4U);
        }
        buf += 4U;
        len -= 4U;
    }

    return orbitc_memchr_portable(buf, val, len);
}

#elif (ORBITC_MEM_BACKEND == ORBITC_MEM_BACKEND_CORTEX_A9)

#define ORBITC_CA9_CACHELINE 32U

static void orbitc_memset_backend(byte_t *dst, byte_t val, usize_t len)
{
    ui32_t w;
    if (len < ORBITC_CA9_CACHELINE)
    {
        orbitc_memset_portable(dst, val, len);
        return;
    }

    while ((len > 0U) && (orbitc_is_aligned4(dst) == ORBITC_FALSE))
    {
        *dst = val;
        ++dst;
        --len;
    }

    w = orbitc_repeat_byte32(val);

    while ((len >= 4U) && (((uptr_t)dst & (uptr_t)(ORBITC_CA9_CACHELINE - 1U)) != 0U))
    {
        orbitc_store_u32_aligned(dst, w);
        dst += 4U;
        len -= 4U;
    }

    while (len >= ORBITC_CA9_CACHELINE)
    {
        orbitc_store_u32_aligned(dst + 0U,  w);
        orbitc_store_u32_aligned(dst + 4U,  w);
        orbitc_store_u32_aligned(dst + 8U,  w);
        orbitc_store_u32_aligned(dst + 12U, w);
        orbitc_store_u32_aligned(dst + 16U, w);
        orbitc_store_u32_aligned(dst + 20U, w);
        orbitc_store_u32_aligned(dst + 24U, w);
        orbitc_store_u32_aligned(dst + 28U, w);
        dst += ORBITC_CA9_CACHELINE;
        len -= ORBITC_CA9_CACHELINE;
    }

    while (len >= 4U)
    {
        orbitc_store_u32_aligned(dst, w);
        dst += 4U;
        len -= 4U;
    }

    if (len > 0U)
    {
        orbitc_memset_portable(dst, val, len);
    }
}

static void orbitc_memcpy_backend(byte_t *dst, const byte_t *src, usize_t len)
{
    if (len < ORBITC_CA9_CACHELINE)
    {
        orbitc_memcpy_portable(dst, src, len);
        return;
    }

    if (((uptr_t)dst & 3U) != ((uptr_t)src & 3U))
    {
        orbitc_memcpy_portable(dst, src, len);
        return;
    }

    while ((len > 0U) && (orbitc_is_aligned4(dst) == ORBITC_FALSE))
    {
        *dst = *src;
        ++dst;
        ++src;
        --len;
    }

    while (len >= ORBITC_CA9_CACHELINE)
    {
        ui32_t r0 = orbitc_load_u32_aligned(src + 0U);
        ui32_t r1 = orbitc_load_u32_aligned(src + 4U);
        ui32_t r2 = orbitc_load_u32_aligned(src + 8U);
        ui32_t r3 = orbitc_load_u32_aligned(src + 12U);
        ui32_t r4 = orbitc_load_u32_aligned(src + 16U);
        ui32_t r5 = orbitc_load_u32_aligned(src + 20U);
        ui32_t r6 = orbitc_load_u32_aligned(src + 24U);
        ui32_t r7 = orbitc_load_u32_aligned(src + 28U);

        orbitc_store_u32_aligned(dst + 0U,  r0);
        orbitc_store_u32_aligned(dst + 4U,  r1);
        orbitc_store_u32_aligned(dst + 8U,  r2);
        orbitc_store_u32_aligned(dst + 12U, r3);
        orbitc_store_u32_aligned(dst + 16U, r4);
        orbitc_store_u32_aligned(dst + 20U, r5);
        orbitc_store_u32_aligned(dst + 24U, r6);
        orbitc_store_u32_aligned(dst + 28U, r7);

        src += ORBITC_CA9_CACHELINE;
        dst += ORBITC_CA9_CACHELINE;
        len -= ORBITC_CA9_CACHELINE;
    }

    while (len >= 4U)
    {
        orbitc_store_u32_aligned(dst, orbitc_load_u32_aligned(src));
        src += 4U;
        dst += 4U;
        len -= 4U;
    }

    if (len > 0U)
    {
        orbitc_memcpy_portable(dst, src, len);
    }
}

static void orbitc_memmove_backend(byte_t *dst, const byte_t *src, usize_t len)
{
    if (dst == src)
    {
        return;
    }

    if (orbitc_ranges_overlap(dst, len, src, len) == ORBITC_FALSE)
    {
        orbitc_memcpy_backend(dst, src, len);
        return;
    }

    if ((len < ORBITC_CA9_CACHELINE) || (((uptr_t)dst & 3U) != ((uptr_t)src & 3U)))
    {
        orbitc_memmove_portable(dst, src, len);
        return;
    }

    if ((uptr_t)dst < (uptr_t)src)
    {
        while ((len > 0U) && (orbitc_is_aligned4(dst) == ORBITC_FALSE))
        {
            *dst = *src;
            ++dst;
            ++src;
            --len;
        }
        while (len >= ORBITC_CA9_CACHELINE)
        {
            ui32_t r0 = orbitc_load_u32_aligned(src + 0U);
            ui32_t r1 = orbitc_load_u32_aligned(src + 4U);
            ui32_t r2 = orbitc_load_u32_aligned(src + 8U);
            ui32_t r3 = orbitc_load_u32_aligned(src + 12U);
            ui32_t r4 = orbitc_load_u32_aligned(src + 16U);
            ui32_t r5 = orbitc_load_u32_aligned(src + 20U);
            ui32_t r6 = orbitc_load_u32_aligned(src + 24U);
            ui32_t r7 = orbitc_load_u32_aligned(src + 28U);

            orbitc_store_u32_aligned(dst + 0U,  r0);
            orbitc_store_u32_aligned(dst + 4U,  r1);
            orbitc_store_u32_aligned(dst + 8U,  r2);
            orbitc_store_u32_aligned(dst + 12U, r3);
            orbitc_store_u32_aligned(dst + 16U, r4);
            orbitc_store_u32_aligned(dst + 20U, r5);
            orbitc_store_u32_aligned(dst + 24U, r6);
            orbitc_store_u32_aligned(dst + 28U, r7);

            src += ORBITC_CA9_CACHELINE;
            dst += ORBITC_CA9_CACHELINE;
            len -= ORBITC_CA9_CACHELINE;
        }
        while (len >= 4U)
        {
            orbitc_store_u32_aligned(dst, orbitc_load_u32_aligned(src));
            src += 4U;
            dst += 4U;
            len -= 4U;
        }
        if (len > 0U)
        {
            orbitc_memmove_portable(dst, src, len);
        }
    }
    else
    {
        byte_t       *d_end = dst + len;
        const byte_t *s_end = src + len;

        while ((len > 0U) && (orbitc_is_aligned4(d_end) == ORBITC_FALSE))
        {
            --d_end;
            --s_end;
            *d_end = *s_end;
            --len;
        }
        while (len >= ORBITC_CA9_CACHELINE)
        {
            ui32_t r0, r1, r2, r3, r4, r5, r6, r7;
            d_end -= ORBITC_CA9_CACHELINE;
            s_end -= ORBITC_CA9_CACHELINE;

            r0 = orbitc_load_u32_aligned(s_end + 0U);
            r1 = orbitc_load_u32_aligned(s_end + 4U);
            r2 = orbitc_load_u32_aligned(s_end + 8U);
            r3 = orbitc_load_u32_aligned(s_end + 12U);
            r4 = orbitc_load_u32_aligned(s_end + 16U);
            r5 = orbitc_load_u32_aligned(s_end + 20U);
            r6 = orbitc_load_u32_aligned(s_end + 24U);
            r7 = orbitc_load_u32_aligned(s_end + 28U);

            orbitc_store_u32_aligned(d_end + 28U, r7);
            orbitc_store_u32_aligned(d_end + 24U, r6);
            orbitc_store_u32_aligned(d_end + 20U, r5);
            orbitc_store_u32_aligned(d_end + 16U, r4);
            orbitc_store_u32_aligned(d_end + 12U, r3);
            orbitc_store_u32_aligned(d_end + 8U,  r2);
            orbitc_store_u32_aligned(d_end + 4U,  r1);
            orbitc_store_u32_aligned(d_end + 0U,  r0);

            len -= ORBITC_CA9_CACHELINE;
        }
        while (len >= 4U)
        {
            d_end -= 4U;
            s_end -= 4U;
            orbitc_store_u32_aligned(d_end, orbitc_load_u32_aligned(s_end));
            len -= 4U;
        }
        if (len > 0U)
        {
            orbitc_memmove_portable(dst, src, len);
        }
    }
}

static si32_t orbitc_memcmp_backend(const byte_t *lhs, const byte_t *rhs, usize_t len)
{
    if ((len < ORBITC_CA9_CACHELINE) || (((uptr_t)lhs & 3U) != ((uptr_t)rhs & 3U)))
    {
        return orbitc_memcmp_portable(lhs, rhs, len);
    }

    while ((len > 0U) && (orbitc_is_aligned4(lhs) == ORBITC_FALSE))
    {
        if (*lhs != *rhs)
        {
            return (si32_t)(*lhs) - (si32_t)(*rhs);
        }
        ++lhs;
        ++rhs;
        --len;
    }

    while (len >= 16U)
    {
        ui32_t l0 = orbitc_load_u32_aligned(lhs + 0U);
        ui32_t r0 = orbitc_load_u32_aligned(rhs + 0U);
        ui32_t l1 = orbitc_load_u32_aligned(lhs + 4U);
        ui32_t r1 = orbitc_load_u32_aligned(rhs + 4U);
        ui32_t l2 = orbitc_load_u32_aligned(lhs + 8U);
        ui32_t r2 = orbitc_load_u32_aligned(rhs + 8U);
        ui32_t l3 = orbitc_load_u32_aligned(lhs + 12U);
        ui32_t r3 = orbitc_load_u32_aligned(rhs + 12U);

        if (((l0 ^ r0) | (l1 ^ r1) | (l2 ^ r2) | (l3 ^ r3)) != 0U)
        {
            return orbitc_memcmp_portable(lhs, rhs, 16U);
        }
        lhs += 16U;
        rhs += 16U;
        len -= 16U;
    }

    while (len >= 4U)
    {
        if (orbitc_load_u32_aligned(lhs) != orbitc_load_u32_aligned(rhs))
        {
            return orbitc_memcmp_portable(lhs, rhs, 4U);
        }
        lhs += 4U;
        rhs += 4U;
        len -= 4U;
    }

    return orbitc_memcmp_portable(lhs, rhs, len);
}

static const void *orbitc_memchr_backend(const byte_t *buf, byte_t val, usize_t len)
{
    ui32_t mask;
    if (len < ORBITC_CA9_CACHELINE)
    {
        return orbitc_memchr_portable(buf, val, len);
    }

    while ((len > 0U) && (orbitc_is_aligned4(buf) == ORBITC_FALSE))
    {
        if (*buf == val)
        {
            return (const void *)buf;
        }
        ++buf;
        --len;
    }

    mask = orbitc_repeat_byte32(val);
    while (len >= 16U)
    {
        ui32_t w0 = orbitc_load_u32_aligned(buf + 0U) ^ mask;
        ui32_t w1 = orbitc_load_u32_aligned(buf + 4U) ^ mask;
        ui32_t w2 = orbitc_load_u32_aligned(buf + 8U) ^ mask;
        ui32_t w3 = orbitc_load_u32_aligned(buf + 12U) ^ mask;

        if ((orbitc_word_has_zero(w0) == ORBITC_TRUE) ||
            (orbitc_word_has_zero(w1) == ORBITC_TRUE) ||
            (orbitc_word_has_zero(w2) == ORBITC_TRUE) ||
            (orbitc_word_has_zero(w3) == ORBITC_TRUE))
        {
            return orbitc_memchr_portable(buf, val, 16U);
        }
        buf += 16U;
        len -= 16U;
    }

    return orbitc_memchr_portable(buf, val, len);
}

#else

static void orbitc_memset_backend(byte_t *dst, byte_t val, usize_t len)
{
    orbitc_memset_portable(dst, val, len);
}

static void orbitc_memcpy_backend(byte_t *dst, const byte_t *src, usize_t len)
{
    orbitc_memcpy_portable(dst, src, len);
}

static void orbitc_memmove_backend(byte_t *dst, const byte_t *src, usize_t len)
{
    orbitc_memmove_portable(dst, src, len);
}

static si32_t orbitc_memcmp_backend(const byte_t *lhs, const byte_t *rhs, usize_t len)
{
    return orbitc_memcmp_portable(lhs, rhs, len);
}

static const void *orbitc_memchr_backend(const byte_t *buf, byte_t val, usize_t len)
{
    return orbitc_memchr_portable(buf, val, len);
}

#endif

status_t orbitc_memset(void *dst, byte_t val, usize_t len)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(
        dst,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_DST);

    orbitc_assert_address_range(
        dst,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    if ((orbitc_status_is_ok(st) == ORBITC_TRUE) &&
        (len > 0U))
    {
        orbitc_memset_backend((byte_t *)dst, val, len);
    }

    return st;
}

status_t orbitc_memcpy(void *dst, const void *src, usize_t len)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(
        dst,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_DST);

    orbitc_assert_not_null(
        src,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_SRC);

    orbitc_assert_address_range(
        dst,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    orbitc_assert_address_range(
        src,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    orbitc_assert_no_overlap(
        dst,
        len,
        src,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERLAP);

    if ((orbitc_status_is_ok(st) == ORBITC_TRUE) &&
        (len > 0U))
    {
        orbitc_memcpy_backend(
            (byte_t *)dst,
            (const byte_t *)src,
            len);
    }

    return st;
}


status_t orbitc_memmove(void *dst, const void *src, usize_t len)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(
        dst,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_DST);

    orbitc_assert_not_null(
        src,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_SRC);

    orbitc_assert_address_range(
        dst,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    orbitc_assert_address_range(
        src,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    if ((orbitc_status_is_ok(st) == ORBITC_TRUE) &&
        (len > 0U))
    {
        orbitc_memmove_backend(
            (byte_t *)dst,
            (const byte_t *)src,
            len);
    }

    return st;
}


status_t orbitc_memcmp(
    const void *lhs,
    const void *rhs,
    usize_t len,
    si32_t *out_diff)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(
        lhs,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_DST);

    orbitc_assert_not_null(
        rhs,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_SRC);

    orbitc_assert_not_null(
        out_diff,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_OUT);

    orbitc_assert_address_range(
        lhs,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    orbitc_assert_address_range(
        rhs,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (len == 0U)
        {
            *out_diff = 0;
        }
        else
        {
            *out_diff = orbitc_memcmp_backend(
                (const byte_t *)lhs,
                (const byte_t *)rhs,
                len);
        }
    }

    return st;
}

status_t orbitc_memchr(
    const void *buf,
    byte_t val,
    usize_t len,
    const void **out_found)
{
    status_t    st = STATUS_OK;
    const void *found = ORBITC_NULL;

    orbitc_assert_not_null(
        buf,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_SRC);

    orbitc_assert_not_null(
        out_found,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_NULL_OUT);

    orbitc_assert_address_range(
        buf,
        len,
        &st,
        STATUS_MOD_MEMORY,
        (ui8_t)MEMORY_CODE_OVERFLOW);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_found = ORBITC_NULL;

        if (len > 0U)
        {
            found = orbitc_memchr_backend(
                (const byte_t *)buf,
                val,
                len);
        }

        if (found == ORBITC_NULL)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_MEMORY,
                    STATUS_CLS_NOT_FOUND,
                    MEMORY_CODE_NOT_FOUND));
        }
        else
        {
            *out_found = found;
        }
    }

    return st;
}

ui32_t orbitc_memory_active_backend(void)
{
    return (ui32_t)ORBITC_MEM_BACKEND;
}