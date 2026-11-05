/**
 * @file crc32_tables.h
 * @brief Hardcoded 4-bit nibble lookup tables for CRC-32 algorithms.
 */
#ifndef ORBITC_CRC32_TABLES_H
#define ORBITC_CRC32_TABLES_H

#include "orbitc_types.h"

#if (ENABLE_CRC32 == 1) && (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

#if (ENABLE_CRC32_IEEE == 1)
/**
 * @brief Nibble lookup table for CRC-32 IEEE 802.3 (Poly: 0xEDB88320, Reflected).
 */
static const ui32_t orbitc_crc32_ieee_nibble_table[16] = {
    0x00000000UL, 0x1DB71064UL, 0x3B6E20C8UL, 0x26D930ACUL,
    0x76DC4190UL, 0x6B6B51F4UL, 0x4DB26158UL, 0x5005713CUL,
    0xEDB88320UL, 0xF00F9344UL, 0xD6D6A3E8UL, 0xCB61B38CUL,
    0x9B64C2B0UL, 0x86D3D2D4UL, 0xA00AE278UL, 0xBDBDF21CUL
};
#endif /* ENABLE_CRC32_IEEE */

#if (ENABLE_CRC32_CASTAGNOLI == 1)
/**
 * @brief Nibble lookup table for CRC-32C Castagnoli (Poly: 0x82F63B78, Reflected).
 */
static const ui32_t orbitc_crc32_castagnoli_nibble_table[16] = {
    0x00000000UL, 0x105EC76FUL, 0x20BD8EDEUL, 0x30E349B1UL,
    0x417B1DBCUL, 0x5125DAD3UL, 0x61C69362UL, 0x7198540DUL,
    0x82F63B78UL, 0x92A8FC17UL, 0xA24BB5A6UL, 0xB21572C9UL,
    0xC38D26C4UL, 0xD3D3E1ABUL, 0xE330A81AUL, 0xF36E6F75UL
};
#endif /* ENABLE_CRC32_CASTAGNOLI */

#endif /* (ENABLE_CRC32 == 1) && (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE) */

#endif /* ORBITC_CRC32_TABLES_H */