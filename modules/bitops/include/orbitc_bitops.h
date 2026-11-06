/**
 * @file orbitc_bitops.h
 * @brief Safe bit manipulation, bitfields, rotations, and bit-scanning for OrbitC.
 *
 * Provides compile-time macros and runtime bounds-checked functions for manipulating
 * individual bits, contiguous bitfields, bitwise rotations, population count
 * (Hamming weight), and leading/trailing zero counts without C99 undefined behavior.
 *
 * @defgroup bitops Bitops
 * @brief Low-level bit manipulation, hardware register bitfields, and bit scanning.
 * @{
 */
#ifndef ORBITC_BITOPS_H
#define ORBITC_BITOPS_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Backend Configuration                                                      */
/* ========================================================================== */

/** @brief Pure C99 portable implementation (SWAR / LUT-free). */
#define ORBITC_BITOPS_BACKEND_PORTABLE 0

/** @brief Compiler intrinsic accelerated implementation (GCC/Clang builtins). */
#define ORBITC_BITOPS_BACKEND_BUILTIN  1

#ifndef ORBITC_BITOPS_BACKEND
#define ORBITC_BITOPS_BACKEND ORBITC_BITOPS_BACKEND_BUILTIN
#endif

#ifndef ENABLE_BITOPS_64
#define ENABLE_BITOPS_64 1
#endif

/* ========================================================================== */
/* Fast Compile-Time / Constant Bitwise Macros                                */
/* ========================================================================== */

/** @brief Generates a 32-bit mask with bit @p n set (`0 <= n < 32`). */
#define ORBITC_BIT32(n) ((ui32_t)(1UL << (n)))

/** @brief Generates a 64-bit mask with bit @p n set (`0 <= n < 64`). */
#define ORBITC_BIT64(n) ((ui64_t)(1ULL << (n)))

/**
 * @brief Generates a 32-bit contiguous mask of @p width bits starting at @p offset.
 * Safe for `1 <= width <= 32` and `offset + width <= 32`.
 */
#define ORBITC_BITMASK32(offset, width) \
    ((ui32_t)((((width) >= 32U) ? 0xFFFFFFFFUL : ((1UL << (width)) - 1UL)) << (offset)))

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific diagnostic status codes for bitops operations.
 */
typedef enum
{
    BITOPS_CODE_OK          = 0x00U, /**< Operation completed successfully. */
    BITOPS_CODE_NULL_PTR    = 0x01U, /**< Output pointer argument is NULL. */
    BITOPS_CODE_INVALID_BIT = 0x02U, /**< Bit index >= register bit-width (32 or 64). */
    BITOPS_CODE_BAD_FIELD   = 0x03U, /**< Bitfield width is 0 or (offset + width) exceeds register width. */
    BITOPS_CODE_ZERO_INPUT  = 0x04U  /**< Input value is zero for an operation requiring non-zero input (CLZ/CTZ/FFS). */
} bitops_code_t;

/* ========================================================================== */
/* 32-Bit Single-Bit Operations                                               */
/* ========================================================================== */

/**
 * @brief Sets a specific bit in a 32-bit word.
 * @param[in,out] word Pointer to the 32-bit value to modify.
 * @param[in]     bit  Bit index to set (`0..31`).
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit32_set(ui32_t *word, ui8_t bit);

/**
 * @brief Clears a specific bit in a 32-bit word.
 * @param[in,out] word Pointer to the 32-bit value to modify.
 * @param[in]     bit  Bit index to clear (`0..31`).
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit32_clear(ui32_t *word, ui8_t bit);

/**
 * @brief Toggles (inverts) a specific bit in a 32-bit word.
 * @param[in,out] word Pointer to the 32-bit value to modify.
 * @param[in]     bit  Bit index to toggle (`0..31`).
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit32_toggle(ui32_t *word, ui8_t bit);

/**
 * @brief Tests the state of a specific bit in a 32-bit word.
 * @param[in]  word    32-bit value to inspect.
 * @param[in]  bit     Bit index to test (`0..31`).
 * @param[out] out_set Pointer receiving `ORBITC_TRUE` if the bit is 1, `ORBITC_FALSE` if 0.
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit32_test(ui32_t word, ui8_t bit, bool_t *out_set);

/* ========================================================================== */
/* 32-Bit Bitfield Extraction & Insertion                                     */
/* ========================================================================== */

/**
 * @brief Generates a 32-bit contiguous bitmask with runtime bounds checking.
 * @param[in]  offset   Starting bit position (`0..31`).
 * @param[in]  width    Number of bits in the mask (`1..32`, `offset + width <= 32`).
 * @param[out] out_mask Pointer receiving the generated bitmask.
 * @return Status indicating success, null pointer, or invalid field geometry.
 */
status_t orbitc_bitfield32_mask(ui8_t offset, ui8_t width, ui32_t *out_mask);

/**
 * @brief Extracts an unsigned bitfield from a 32-bit word (shifted right to LSB).
 * @param[in]  word    32-bit source register/word.
 * @param[in]  offset  Starting bit position (`0..31`).
 * @param[in]  width   Width of the bitfield in bits (`1..32`, `offset + width <= 32`).
 * @param[out] out_val Pointer receiving the right-aligned extracted field value.
 * @return Status indicating success, null pointer, or invalid field geometry.
 */
status_t orbitc_bitfield32_extract(ui32_t word, ui8_t offset, ui8_t width, ui32_t *out_val);

/**
 * @brief Inserts a value into a specific bitfield within a 32-bit word.
 * @param[in,out] word      Pointer to the 32-bit target word to modify.
 * @param[in]     offset    Starting bit position (`0..31`).
 * @param[in]     width     Width of the bitfield in bits (`1..32`, `offset + width <= 32`).
 * @param[in]     field_val Value to insert (automatically masked to @p width bits).
 * @return Status indicating success, null pointer, or invalid field geometry.
 */
status_t orbitc_bitfield32_insert(ui32_t *word, ui8_t offset, ui8_t width, ui32_t field_val);

/* ========================================================================== */
/* 32-Bit Rotations, Reversal & Bit Scanning                                  */
/* ========================================================================== */

/**
 * @brief Rotates a 32-bit word left by @p shift bits (UB-safe for any shift value).
 * @param[in]  word    Value to rotate.
 * @param[in]  shift   Rotation count (automatically reduced modulo 32).
 * @param[out] out_val Pointer receiving the rotated value.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_rotl32(ui32_t word, ui8_t shift, ui32_t *out_val);

/**
 * @brief Rotates a 32-bit word right by @p shift bits (UB-safe for any shift value).
 * @param[in]  word    Value to rotate.
 * @param[in]  shift   Rotation count (automatically reduced modulo 32).
 * @param[out] out_val Pointer receiving the rotated value.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_rotr32(ui32_t word, ui8_t shift, ui32_t *out_val);

/**
 * @brief Reverses the bit order of a 32-bit word (MSB becomes LSB).
 * @param[in]  word    Value to bit-reverse.
 * @param[out] out_val Pointer receiving the bit-reversed 32-bit word.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_bitrev32(ui32_t word, ui32_t *out_val);

/**
 * @brief Counts the number of set bits (Hamming weight / popcount) in a 32-bit word.
 * @param[in]  word      32-bit value to evaluate.
 * @param[out] out_count Pointer receiving the number of 1-bits (`0..32`).
 * @return Status indicating success or null pointer.
 */
status_t orbitc_popcount32(ui32_t word, ui8_t *out_count);

/**
 * @brief Counts leading zeros (from MSB downwards) in a non-zero 32-bit word.
 * @param[in]  word      Non-zero 32-bit value to scan.
 * @param[out] out_count Pointer receiving the leading zero count (`0..31`).
 * @return Status indicating success, null pointer, or `BITOPS_CODE_ZERO_INPUT` if `word == 0`.
 */
status_t orbitc_clz32(ui32_t word, ui8_t *out_count);

/**
 * @brief Counts trailing zeros (from LSB upwards) in a non-zero 32-bit word.
 * @param[in]  word      Non-zero 32-bit value to scan.
 * @param[out] out_count Pointer receiving the trailing zero count (`0..31`).
 * @return Status indicating success, null pointer, or `BITOPS_CODE_ZERO_INPUT` if `word == 0`.
 */
status_t orbitc_ctz32(ui32_t word, ui8_t *out_count);

/**
 * @brief computes odd parity of a 32-bit word.
 * @param[in]  word       32-bit value to inspect.
 * @param[out] out_parity Pointer receiving `1U` if an odd number of bits are set, `0U` if even.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_parity32(ui32_t word, ui8_t *out_parity);

/* ========================================================================== */
/* 64-Bit Operations (Optional)                                               */
/* ========================================================================== */
#if (ENABLE_BITOPS_64 == 1)

/**
 * @brief Sets a specific bit in a 64-bit word.
 * @param[in,out] word Pointer to the 64-bit value to modify.
 * @param[in]     bit  Bit index to set (`0..63`).
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit64_set(ui64_t *word, ui8_t bit);

/**
 * @brief Clears a specific bit in a 64-bit word.
 * @param[in,out] word Pointer to the 64-bit value to modify.
 * @param[in]     bit  Bit index to clear (`0..63`).
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit64_clear(ui64_t *word, ui8_t bit);

/**
 * @brief Toggles (inverts) a specific bit in a 64-bit word.
 * @param[in,out] word Pointer to the 64-bit value to modify.
 * @param[in]     bit  Bit index to toggle (`0..63`).
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit64_toggle(ui64_t *word, ui8_t bit);

/**
 * @brief Tests the state of a specific bit in a 64-bit word.
 * @param[in]  word    64-bit value to inspect.
 * @param[in]  bit     Bit index to test (`0..63`).
 * @param[out] out_set Pointer receiving `ORBITC_TRUE` if the bit is 1, `ORBITC_FALSE` if 0.
 * @return Status indicating success, null pointer, or out-of-range bit index.
 */
status_t orbitc_bit64_test(ui64_t word, ui8_t bit, bool_t *out_set);

/**
 * @brief Extracts an unsigned bitfield from a 64-bit word (shifted right to LSB).
 * @param[in]  word   64-bit source register/word.
 * @param[in]  offset Starting bit position (`0..63`).
 * @param[in]  width  Width of the bitfield in bits (`1..64`, `offset + width <= 64`).
 * @param[out] out_val Pointer receiving the right-aligned extracted field value.
 * @return Status indicating success, null pointer, or invalid field geometry.
 */
status_t orbitc_bitfield64_extract(ui64_t word, ui8_t offset, ui8_t width, ui64_t *out_val);

/**
 * @brief Inserts a value into a specific bitfield within a 64-bit word.
 * @param[in,out] word      Pointer to the 64-bit target word to modify.
 * @param[in]     offset    Starting bit position (`0..63`).
 * @param[in]     width     Width of the bitfield in bits (`1..64`, `offset + width <= 64`).
 * @param[in]     field_val Value to insert (automatically masked to @p width bits).
 * @return Status indicating success, null pointer, or invalid field geometry.
 */
status_t orbitc_bitfield64_insert(ui64_t *word, ui8_t offset, ui8_t width, ui64_t field_val);

/**
 * @brief Rotates a 64-bit word left by @p shift bits (UB-safe for any shift value).
 * @param[in]  word    Value to rotate.
 * @param[in]  shift   Rotation count (automatically reduced modulo 64).
 * @param[out] out_val Pointer receiving the rotated value.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_rotl64(ui64_t word, ui8_t shift, ui64_t *out_val);

/**
 * @brief Rotates a 64-bit word right by @p shift bits (UB-safe for any shift value).
 * @param[in]  word    Value to rotate.
 * @param[in]  shift   Rotation count (automatically reduced modulo 64).
 * @param[out] out_val Pointer receiving the rotated value.
 * @return Status indicating success or null pointer.
 */
status_t orbitc_rotr64(ui64_t word, ui8_t shift, ui64_t *out_val);

/**
 * @brief Counts the number of set bits (Hamming weight / popcount) in a 64-bit word.
 * @param[in]  word      64-bit value to evaluate.
 * @param[out] out_count Pointer receiving the number of 1-bits (`0..64`).
 * @return Status indicating success or null pointer.
 */
status_t orbitc_popcount64(ui64_t word, ui8_t *out_count);

/**
 * @brief Counts leading zeros (from MSB downwards) in a non-zero 64-bit word.
 * @param[in]  word      Non-zero 64-bit value to scan.
 * @param[out] out_count Pointer receiving the leading zero count (`0..63`).
 * @return Status indicating success, null pointer, or `BITOPS_CODE_ZERO_INPUT` if `word == 0`.
 */
status_t orbitc_clz64(ui64_t word, ui8_t *out_count);

/**
 * @brief Counts trailing zeros (from LSB upwards) in a non-zero 64-bit word.
 * @param[in]  word      Non-zero 64-bit value to scan.
 * @param[out] out_count Pointer receiving the trailing zero count (`0..63`).
 * @return Status indicating success, null pointer, or `BITOPS_CODE_ZERO_INPUT` if `word == 0`.
 */
status_t orbitc_ctz64(ui64_t word, ui8_t *out_count);

#endif /* ENABLE_BITOPS_64 == 1 */

/**
 * @brief Queries the active bitops implementation backend.
 */
ui32_t orbitc_bitops_active_backend(void);

#endif /* ORBITC_BITOPS_H */
/** @} */ /* End of group bitops */