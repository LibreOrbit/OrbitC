/**
 * @file orbitc_types.h
 * @brief Core primitive type definitions and architectural configurations for OrbitC.
 *
 * This header defines fixed-width integer types, floating-point aliases,
 * boolean abstractions, pointer representations, and platform architecture
 * constants. It guarantees standard type sizes across supported compilers
 * and validates them at compile time using static assertions.
 */

/**
 * @defgroup types Types
 * @brief Core primitive data types, memory abstractions, and architectural macros.
 * @{
 */

#ifndef ORBITC_TYPES_H
#define ORBITC_TYPES_H

/* ========================================================================== */
/* Architecture and Platform Defaults                                         */
/* ========================================================================== */

#ifndef ORBITC_ARCH_BITS
/**
 * @brief Target machine word size in bits (defaults to 64-bit).
 *
 * Can be overridden before inclusion via compiler flags (e.g., -DORBITC_ARCH_BITS=32).
 */
#define ORBITC_ARCH_BITS 64
#endif

#ifndef ORBITC_LITTLE_ENDIAN
/**
 * @brief Byte-order configuration flag.
 *
 * Set to 1 if the target architecture is little-endian, or 0 if big-endian.
 */
#define ORBITC_LITTLE_ENDIAN 1
#endif

#ifndef ORBITC_ALLOW_UNALIGNED
/**
 * @brief Flag indicating whether unaligned memory access is permitted.
 *
 * Set to 1 if unaligned reads/writes are supported and allowed, 0 otherwise.
 */
#define ORBITC_ALLOW_UNALIGNED 0
#endif

/* ========================================================================== */
/* Compile-time Assertions                                                    */
/* ========================================================================== */

/**
 * @brief Evaluates a compile-time condition and triggers a compilation error on failure.
 *
 * Implemented via an invalid negative-size array typedef if the condition is false.
 *
 * @param cond Boolean condition to test at compile time.
 * @param msg  Identifier token to uniquely name the diagnostic typedef.
 */
#define ORBITC_STATIC_ASSERT(cond, msg) typedef char orbitc_static_assert_##msg[(cond) ? 1 : -1]

/* ========================================================================== */
/* Fixed-width Integer Types                                                  */
/* ========================================================================== */

#if defined(__GNUC__) || defined(__clang__)

/** @brief Unsigned 8-bit integer. */
typedef __UINT8_TYPE__   ui8_t;

/** @brief Signed 8-bit integer. */
typedef __INT8_TYPE__    si8_t;

/** @brief Unsigned 16-bit integer. */
typedef __UINT16_TYPE__  ui16_t;

/** @brief Signed 16-bit integer. */
typedef __INT16_TYPE__   si16_t;

/** @brief Unsigned 32-bit integer. */
typedef __UINT32_TYPE__  ui32_t;

/** @brief Signed 32-bit integer. */
typedef __INT32_TYPE__   si32_t;

/** @brief Unsigned 64-bit integer. */
typedef __UINT64_TYPE__  ui64_t;

/** @brief Signed 64-bit integer. */
typedef __INT64_TYPE__   si64_t;

/** @brief Unsigned integer type representing object/buffer sizes. */
typedef __SIZE_TYPE__    usize_t;

/** @brief Unsigned integer capable of holding a pointer value. */
typedef __UINTPTR_TYPE__ uptr_t;

/** @brief Signed integer capable of holding a pointer difference or value. */
typedef __INTPTR_TYPE__  sptr_t;

#else

/* Fallback type definitions for non-GCC/Clang compilers */

#ifndef ORBITC_FALLBACK_UI8
/** @brief Fallback underlying type for unsigned 8-bit integers. */
#define ORBITC_FALLBACK_UI8 unsigned char
#endif

#ifndef ORBITC_FALLBACK_SI8
/** @brief Fallback underlying type for signed 8-bit integers. */
#define ORBITC_FALLBACK_SI8 signed char
#endif

#ifndef ORBITC_FALLBACK_UI16
/** @brief Fallback underlying type for unsigned 16-bit integers. */
#define ORBITC_FALLBACK_UI16 unsigned short
#endif

#ifndef ORBITC_FALLBACK_SI16
/** @brief Fallback underlying type for signed 16-bit integers. */
#define ORBITC_FALLBACK_SI16 signed short
#endif

#ifndef ORBITC_FALLBACK_UI32
/** @brief Fallback underlying type for unsigned 32-bit integers. */
#define ORBITC_FALLBACK_UI32 unsigned int
#endif

#ifndef ORBITC_FALLBACK_SI32
/** @brief Fallback underlying type for signed 32-bit integers. */
#define ORBITC_FALLBACK_SI32 signed int
#endif

#ifndef ORBITC_FALLBACK_UI64
/** @brief Fallback underlying type for unsigned 64-bit integers. */
#define ORBITC_FALLBACK_UI64 unsigned long long
#endif

#ifndef ORBITC_FALLBACK_SI64
/** @brief Fallback underlying type for signed 64-bit integers. */
#define ORBITC_FALLBACK_SI64 signed long long
#endif

/** @brief Unsigned 8-bit integer. */
typedef ORBITC_FALLBACK_UI8  ui8_t;

/** @brief Signed 8-bit integer. */
typedef ORBITC_FALLBACK_SI8  si8_t;

/** @brief Unsigned 16-bit integer. */
typedef ORBITC_FALLBACK_UI16 ui16_t;

/** @brief Signed 16-bit integer. */
typedef ORBITC_FALLBACK_SI16 si16_t;

/** @brief Unsigned 32-bit integer. */
typedef ORBITC_FALLBACK_UI32 ui32_t;

/** @brief Signed 32-bit integer. */
typedef ORBITC_FALLBACK_SI32 si32_t;

/** @brief Unsigned 64-bit integer. */
typedef ORBITC_FALLBACK_UI64 ui64_t;

/** @brief Signed 64-bit integer. */
typedef ORBITC_FALLBACK_SI64 si64_t;

#if (ORBITC_ARCH_BITS == 64)
/** @brief Unsigned integer representing object/buffer sizes on 64-bit architectures. */
typedef ui64_t usize_t;

/** @brief Unsigned integer type holding pointer addresses on 64-bit architectures. */
typedef ui64_t uptr_t;

/** @brief Signed integer type holding pointer values on 64-bit architectures. */
typedef si64_t sptr_t;
#elif (ORBITC_ARCH_BITS == 32)
/** @brief Unsigned integer representing object/buffer sizes on 32-bit architectures. */
typedef ui32_t usize_t;

/** @brief Unsigned integer type holding pointer addresses on 32-bit architectures. */
typedef ui32_t uptr_t;

/** @brief Signed integer type holding pointer values on 32-bit architectures. */
typedef si32_t sptr_t;
#else
#error "ORBITC_ARCH_BITS must be set to 32 or 64"
#endif

#endif

/* ========================================================================== */
/* Floating-point, Byte, and Boolean Types                                    */
/* ========================================================================== */

/** @brief Single-precision 32-bit IEEE 754 floating-point type. */
typedef float  f32_t;

/** @brief Double-precision 64-bit IEEE 754 floating-point type. */
typedef double f64_t;

/** @brief Generic raw byte alias (equivalent to 8-bit unsigned integer). */
typedef ui8_t  byte_t;

/**
 * @brief Standard boolean enumeration for OrbitC.
 */
typedef enum
{
    ORBITC_FALSE = 0, /**< Logical false value (0). */
    ORBITC_TRUE  = 1  /**< Logical true value (1). */
} bool_t;

/* ========================================================================== */
/* Pointer and Memory Address Aliases                                         */
/* ========================================================================== */

/** @brief Generic mutable memory pointer alias. */
typedef void       *addr_t;

/** @brief Generic read-only (constant) memory pointer alias. */
typedef const void *caddr_t;

/** @brief Null pointer constant. */
#define ORBITC_NULL ((void *)0)

#ifndef NULL
/** @brief Fallback definition for the standard NULL pointer constant. */
#define NULL ORBITC_NULL
#endif

/* ========================================================================== */
/* Numeric Limits                                                             */
/* ========================================================================== */

/** @brief Maximum value representable by a ui8_t (0xFF). */
#define ORBITC_UI8_MAX   ((ui8_t)0xFFU)

/** @brief Maximum value representable by a ui16_t (0xFFFF). */
#define ORBITC_UI16_MAX  ((ui16_t)0xFFFFU)

/** @brief Maximum value representable by a ui32_t (0xFFFFFFFF). */
#define ORBITC_UI32_MAX  ((ui32_t)0xFFFFFFFFUL)

/** @brief Maximum value representable by a ui64_t (0xFFFFFFFFFFFFFFFF). */
#define ORBITC_UI64_MAX  ((ui64_t)0xFFFFFFFFFFFFFFFFULL)

/** @brief Maximum value representable by a usize_t. */
#define ORBITC_USIZE_MAX ((usize_t)(~(usize_t)0U))

/** @brief Maximum value representable by a uptr_t. */
#define ORBITC_UPTR_MAX  ((uptr_t)(~(uptr_t)0U))

/* ========================================================================== */
/* Compile-Time Type Validation Checks                                        */
/* ========================================================================== */

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

/* ========================================================================== */
/* Runtime Introspection Functions                                            */
/* ========================================================================== */

/**
 * @brief Retrieves the target architecture bit-width configured for the build.
 *
 * @return Architecture width in bits (typically 32 or 64).
 */
ui32_t orbitc_types_arch_bits(void);

/**
 * @brief Checks if the target platform is configured as little-endian.
 *
 * @retval ORBITC_TRUE  If the target system is little-endian.
 * @retval ORBITC_FALSE If the target system is big-endian.
 */
bool_t orbitc_types_is_little_endian(void);

/**
 * @brief Checks whether unaligned memory access is allowed on this build.
 *
 * @retval ORBITC_TRUE  If unaligned accesses are safe and enabled.
 * @retval ORBITC_FALSE If strict alignment is required.
 */
bool_t orbitc_types_unaligned_allowed(void);

#endif /* ORBITC_TYPES_H */

/** @} */ /* End of group types */