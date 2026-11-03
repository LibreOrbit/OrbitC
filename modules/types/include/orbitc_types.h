/*
 * ================================================================================
 * File: modules/types/include/orbitc_types.h
 * ================================================================================
 */
#ifndef ORBITC_TYPES_H
#define ORBITC_TYPES_H

#ifndef ORBITC_ARCH_BITS
#define ORBITC_ARCH_BITS 64
#endif

#ifndef ORBITC_LITTLE_ENDIAN
#define ORBITC_LITTLE_ENDIAN 1
#endif

#ifndef ORBITC_ALLOW_UNALIGNED
#define ORBITC_ALLOW_UNALIGNED 0
#endif

#define ORBITC_STATIC_ASSERT(cond, msg) typedef char orbitc_static_assert_##msg[(cond) ? 1 : -1]

#if defined(__GNUC__) || defined(__clang__)

typedef __UINT8_TYPE__   ui8_t;
typedef __INT8_TYPE__    si8_t;
typedef __UINT16_TYPE__  ui16_t;
typedef __INT16_TYPE__   si16_t;
typedef __UINT32_TYPE__  ui32_t;
typedef __INT32_TYPE__   si32_t;
typedef __UINT64_TYPE__  ui64_t;
typedef __INT64_TYPE__   si64_t;
typedef __SIZE_TYPE__    usize_t;
typedef __UINTPTR_TYPE__ uptr_t;
typedef __INTPTR_TYPE__  sptr_t;

#else

#ifndef ORBITC_FALLBACK_UI8
#define ORBITC_FALLBACK_UI8 unsigned char
#endif
#ifndef ORBITC_FALLBACK_SI8
#define ORBITC_FALLBACK_SI8 signed char
#endif
#ifndef ORBITC_FALLBACK_UI16
#define ORBITC_FALLBACK_UI16 unsigned short
#endif
#ifndef ORBITC_FALLBACK_SI16
#define ORBITC_FALLBACK_SI16 signed short
#endif
#ifndef ORBITC_FALLBACK_UI32
#define ORBITC_FALLBACK_UI32 unsigned int
#endif
#ifndef ORBITC_FALLBACK_SI32
#define ORBITC_FALLBACK_SI32 signed int
#endif
#ifndef ORBITC_FALLBACK_UI64
#define ORBITC_FALLBACK_UI64 unsigned long long
#endif
#ifndef ORBITC_FALLBACK_SI64
#define ORBITC_FALLBACK_SI64 signed long long
#endif

typedef ORBITC_FALLBACK_UI8  ui8_t;
typedef ORBITC_FALLBACK_SI8  si8_t;
typedef ORBITC_FALLBACK_UI16 ui16_t;
typedef ORBITC_FALLBACK_SI16 si16_t;
typedef ORBITC_FALLBACK_UI32 ui32_t;
typedef ORBITC_FALLBACK_SI32 si32_t;
typedef ORBITC_FALLBACK_UI64 ui64_t;
typedef ORBITC_FALLBACK_SI64 si64_t;

#if (ORBITC_ARCH_BITS == 64)
typedef ui64_t usize_t;
typedef ui64_t uptr_t;
typedef si64_t sptr_t;
#elif (ORBITC_ARCH_BITS == 32)
typedef ui32_t usize_t;
typedef ui32_t uptr_t;
typedef si32_t sptr_t;
#else
#error "ORBITC_ARCH_BITS must be set to 32 or 64"
#endif

#endif

typedef float  f32_t;
typedef double f64_t;

typedef ui8_t byte_t;

typedef enum
{
    ORBITC_FALSE = 0,
    ORBITC_TRUE  = 1
} bool_t;

typedef void       *addr_t;
typedef const void *caddr_t;

#define ORBITC_NULL ((void *)0)

#ifndef NULL
#define NULL ORBITC_NULL
#endif

#define ORBITC_UI8_MAX   ((ui8_t)0xFFU)
#define ORBITC_UI16_MAX  ((ui16_t)0xFFFFU)
#define ORBITC_UI32_MAX  ((ui32_t)0xFFFFFFFFUL)
#define ORBITC_UI64_MAX  ((ui64_t)0xFFFFFFFFFFFFFFFFULL)
#define ORBITC_USIZE_MAX ((usize_t)(~(usize_t)0U))
#define ORBITC_UPTR_MAX  ((uptr_t)(~(uptr_t)0U))

ORBITC_STATIC_ASSERT(sizeof(ui8_t) == 1U, ui8_must_be_1_byte);
ORBITC_STATIC_ASSERT(sizeof(si8_t) == 1U, si8_must_be_1_byte);
ORBITC_STATIC_ASSERT(sizeof(byte_t) == 1U, byte_must_be_1_byte);
ORBITC_STATIC_ASSERT(sizeof(ui16_t) == 2U, ui16_must_be_2_bytes);
ORBITC_STATIC_ASSERT(sizeof(si16_t) == 2U, si16_must_be_2_bytes);
ORBITC_STATIC_ASSERT(sizeof(ui32_t) == 4U, ui32_must_be_4_bytes);
ORBITC_STATIC_ASSERT(sizeof(si32_t) == 4U, si32_must_be_4_bytes);
ORBITC_STATIC_ASSERT(sizeof(ui64_t) == 8U, ui64_must_be_8_bytes);
ORBITC_STATIC_ASSERT(sizeof(si64_t) == 8U, si64_must_be_8_bytes);
ORBITC_STATIC_ASSERT(sizeof(f32_t) == 4U, f32_must_be_4_bytes);
ORBITC_STATIC_ASSERT(sizeof(f64_t) == 8U, f64_must_be_8_bytes);
ORBITC_STATIC_ASSERT(sizeof(usize_t) == sizeof(void *), usize_must_match_ptr_size);
ORBITC_STATIC_ASSERT(sizeof(uptr_t) == sizeof(void *), uptr_must_match_ptr_size);
ORBITC_STATIC_ASSERT(sizeof(sptr_t) == sizeof(void *), sptr_must_match_ptr_size);
ORBITC_STATIC_ASSERT(sizeof(addr_t) == sizeof(void *), addr_must_match_ptr_size);
ORBITC_STATIC_ASSERT(sizeof(caddr_t) == sizeof(const void *), caddr_must_match_ptr_size);

ui32_t orbitc_types_arch_bits(void);
bool_t orbitc_types_is_little_endian(void);
bool_t orbitc_types_unaligned_allowed(void);

#endif