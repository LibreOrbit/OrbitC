/**
 * @file orbitc_endian.h
 * @brief Byte-order conversion and unaligned-safe telemetry serialization for OrbitC.
 *
 * Provides byte-swapping primitives (`bswap16`, `bswap32`, `bswap64`), host-to-endian
 * conversions (`htobe*`, `htole*`), and alignment-agnostic memory buffer readers/writers
 * (`read_be*`, `write_be*`, `read_le*`, `write_le*`) critical for CCSDS/PUS space
 * packet headers, hardware registers, and cross-platform telemetry streams.
 *
 * @defgroup endian Endian
 * @brief Byte-order swapping, host/network conversions, and unaligned buffer I/O.
 * @{
 */
#ifndef ORBITC_ENDIAN_H
#define ORBITC_ENDIAN_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Backend Configuration                                                      */
/* ========================================================================== */

/** @brief Pure C99 bitwise shift and mask byte-swap backend. */
#define ORBITC_ENDIAN_BACKEND_PORTABLE 0

/** @brief Compiler intrinsic byte-swap backend (`__builtin_bswap*`). */
#define ORBITC_ENDIAN_BACKEND_BUILTIN  1

#ifndef ORBITC_ENDIAN_BACKEND
#define ORBITC_ENDIAN_BACKEND ORBITC_ENDIAN_BACKEND_BUILTIN
#endif

#ifndef ENABLE_ENDIAN_64
#define ENABLE_ENDIAN_64 1
#endif

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific diagnostic status codes for endian and serialization operations.
 */
typedef enum
{
    ENDIAN_CODE_OK       = 0x00U, /**< Operation completed successfully. */
    ENDIAN_CODE_NULL_PTR = 0x01U, /**< Buffer or output pointer argument is NULL. */
    ENDIAN_CODE_CAPACITY = 0x02U  /**< Provided buffer capacity is smaller than the data type size. */
} endian_code_t;

/* ========================================================================== */
/* Fast Inline Byte-Swap Primitives (Pure Value)                              */
/* ========================================================================== */

/**
 * @brief Unconditionally swaps the byte order of a 16-bit unsigned integer.
 * @param[in] val 16-bit unsigned integer value to swap.
 * @return The byte-swapped 16-bit unsigned integer.
 */
static inline ui16_t orbitc_bswap16(ui16_t val)
{
#if (ORBITC_ENDIAN_BACKEND == ORBITC_ENDIAN_BACKEND_BUILTIN) && (defined(__GNUC__) || defined(__clang__))
    return __builtin_bswap16(val);
#else
    return (ui16_t)(((val & 0x00FFU) << 8U) | ((val & 0xFF00U) >> 8U));
#endif
}

/**
 * @brief Unconditionally swaps the byte order of a 32-bit unsigned integer.
 * @param[in] val 32-bit unsigned integer value to swap.
 * @return The byte-swapped 32-bit unsigned integer.
 */
static inline ui32_t orbitc_bswap32(ui32_t val)
{
#if (ORBITC_ENDIAN_BACKEND == ORBITC_ENDIAN_BACKEND_BUILTIN) && (defined(__GNUC__) || defined(__clang__))
    return __builtin_bswap32(val);
#else
    return ((val & 0x000000FFUL) << 24U) |
           ((val & 0x0000FF00UL) << 8U)  |
           ((val & 0x00FF0000UL) >> 8U)  |
           ((val & 0xFF000000UL) >> 24U);
#endif
}

#if (ENABLE_ENDIAN_64 == 1)
/**
 * @brief Unconditionally swaps the byte order of a 64-bit unsigned integer.
 * @param[in] val 64-bit unsigned integer value to swap.
 * @return The byte-swapped 64-bit unsigned integer.
 */
static inline ui64_t orbitc_bswap64(ui64_t val)
{
#if (ORBITC_ENDIAN_BACKEND == ORBITC_ENDIAN_BACKEND_BUILTIN) && (defined(__GNUC__) || defined(__clang__))
    return __builtin_bswap64(val);
#else
    return ((val & 0x00000000000000FFULL) << 56U) |
           ((val & 0x000000000000FF00ULL) << 40U) |
           ((val & 0x0000000000FF0000ULL) << 24U) |
           ((val & 0x00000000FF000000ULL) << 8U)  |
           ((val & 0x000000FF00000000ULL) >> 8U)  |
           ((val & 0x0000FF0000000000ULL) >> 24U) |
           ((val & 0x00FF000000000000ULL) >> 40U) |
           ((val & 0xFF00000000000000ULL) >> 56U);
#endif
}
#endif /* ENABLE_ENDIAN_64 == 1 */

/* ========================================================================== */
/* Host <-> Big-Endian (Network / CCSDS) & Little-Endian Conversions          */
/* ========================================================================== */

#if (ORBITC_LITTLE_ENDIAN == 1)
    #define orbitc_htobe16(x) orbitc_bswap16(x)
    #define orbitc_be16toh(x) orbitc_bswap16(x)
    #define orbitc_htole16(x) ((ui16_t)(x))
    #define orbitc_le16toh(x) ((ui16_t)(x))

    #define orbitc_htobe32(x) orbitc_bswap32(x)
    #define orbitc_be32toh(x) orbitc_bswap32(x)
    #define orbitc_htole32(x) ((ui32_t)(x))
    #define orbitc_le32toh(x) ((ui32_t)(x))

    #if (ENABLE_ENDIAN_64 == 1)
        #define orbitc_htobe64(x) orbitc_bswap64(x)
        #define orbitc_be64toh(x) orbitc_bswap64(x)
        #define orbitc_htole64(x) ((ui64_t)(x))
        #define orbitc_le64toh(x) ((ui64_t)(x))
    #endif
#else /* Big-Endian target (e.g., SPARC LEON3/4) */
    #define orbitc_htobe16(x) ((ui16_t)(x))
    #define orbitc_be16toh(x) ((ui16_t)(x))
    #define orbitc_htole16(x) orbitc_bswap16(x)
    #define orbitc_le16toh(x) orbitc_bswap16(x)

    #define orbitc_htobe32(x) ((ui32_t)(x))
    #define orbitc_be32toh(x) ((ui32_t)(x))
    #define orbitc_htole32(x) orbitc_bswap32(x)
    #define orbitc_le32toh(x) orbitc_bswap32(x)

    #if (ENABLE_ENDIAN_64 == 1)
        #define orbitc_htobe64(x) ((ui64_t)(x))
        #define orbitc_be64toh(x) ((ui64_t)(x))
        #define orbitc_htole64(x) orbitc_bswap64(x)
        #define orbitc_le64toh(x) orbitc_bswap64(x)
    #endif
#endif

/* ========================================================================== */
/* Unaligned-Safe Memory Buffer Serialization (Big-Endian & Little-Endian)    */
/* ========================================================================== */

/**
 * @brief Safely reads a 16-bit Big-Endian integer from a potentially unaligned buffer.
 * @param[in]  buf     Pointer to the source byte buffer.
 * @param[in]  cap     Available capacity in @p buf (must be >= 2).
 * @param[out] out_val Pointer receiving the decoded host-order 16-bit value.
 */
status_t orbitc_read_be16(const void *buf, usize_t cap, ui16_t *out_val);

/**
 * @brief Safely reads a 32-bit Big-Endian integer from a potentially unaligned buffer.
 * @param[in]  buf     Pointer to the source byte buffer.
 * @param[in]  cap     Available capacity in @p buf (must be >= 4).
 * @param[out] out_val Pointer receiving the decoded host-order 32-bit value.
 */
status_t orbitc_read_be32(const void *buf, usize_t cap, ui32_t *out_val);

/**
 * @brief Safely writes a 16-bit integer in Big-Endian order to a buffer.
 * @param[out] buf Destination byte buffer.
 * @param[in]  cap Available capacity in @p buf (must be >= 2).
 * @param[in]  val Host-order 16-bit value to serialize.
 */
status_t orbitc_write_be16(void *buf, usize_t cap, ui16_t val);

/**
 * @brief Safely writes a 32-bit integer in Big-Endian order to a buffer.
 * @param[out] buf Destination byte buffer.
 * @param[in]  cap Available capacity in @p buf (must be >= 4).
 * @param[in]  val Host-order 32-bit value to serialize.
 */
status_t orbitc_write_be32(void *buf, usize_t cap, ui32_t val);

/**
 * @brief Safely reads a 16-bit Little-Endian integer from a potentially unaligned buffer.
 */
status_t orbitc_read_le16(const void *buf, usize_t cap, ui16_t *out_val);

/**
 * @brief Safely reads a 32-bit Little-Endian integer from a potentially unaligned buffer.
 */
status_t orbitc_read_le32(const void *buf, usize_t cap, ui32_t *out_val);

/**
 * @brief Safely writes a 16-bit integer in Little-Endian order to a buffer.
 */
status_t orbitc_write_le16(void *buf, usize_t cap, ui16_t val);

/**
 * @brief Safely writes a 32-bit integer in Little-Endian order to a buffer.
 */
status_t orbitc_write_le32(void *buf, usize_t cap, ui32_t val);

#if (ENABLE_ENDIAN_64 == 1)
/**
 * @brief Safely reads a 64-bit Big-Endian integer from a potentially unaligned buffer.
 * @param[in]  buf     Pointer to the source byte buffer.
 * @param[in]  cap     Available capacity in @p buf (must be >= 8).
 * @param[out] out_val Pointer receiving the decoded host-order 64-bit value.
 * @return Status indicating success, null pointer, or capacity error.
 */
status_t orbitc_read_be64(const void *buf, usize_t cap, ui64_t *out_val);

/**
 * @brief Safely writes a 64-bit integer in Big-Endian order to a buffer.
 * @param[out] buf Destination byte buffer.
 * @param[in]  cap Available capacity in @p buf (must be >= 8).
 * @param[in]  val Host-order 64-bit value to serialize.
 * @return Status indicating success, null pointer, or capacity error.
 */
status_t orbitc_write_be64(void *buf, usize_t cap, ui64_t val);

/**
 * @brief Safely reads a 64-bit Little-Endian integer from a potentially unaligned buffer.
 * @param[in]  buf     Pointer to the source byte buffer.
 * @param[in]  cap     Available capacity in @p buf (must be >= 8).
 * @param[out] out_val Pointer receiving the decoded host-order 64-bit value.
 * @return Status indicating success, null pointer, or capacity error.
 */
status_t orbitc_read_le64(const void *buf, usize_t cap, ui64_t *out_val);

/**
 * @brief Safely writes a 64-bit integer in Little-Endian order to a buffer.
 * @param[out] buf Destination byte buffer.
 * @param[in]  cap Available capacity in @p buf (must be >= 8).
 * @param[in]  val Host-order 64-bit value to serialize.
 * @return Status indicating success, null pointer, or capacity error.
 */
status_t orbitc_write_le64(void *buf, usize_t cap, ui64_t val);
#endif /* ENABLE_ENDIAN_64 == 1 */

/**
 * @brief Queries the active endianness implementation backend.
 */
ui32_t orbitc_endian_active_backend(void);

#endif /* ORBITC_ENDIAN_H */
/** @} */ /* End of group endian */