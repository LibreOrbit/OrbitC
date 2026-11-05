/**
 * @file crc16_tables.h
 * @brief Hardcoded 4-bit nibble lookup tables for CRC-16 algorithms.
 */
#ifndef ORBITC_CRC16_TABLES_H
#define ORBITC_CRC16_TABLES_H

#include "orbitc_types.h"

#if (ENABLE_CRC16 == 1) && (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

#if (ENABLE_CRC16_CCITT == 1)
/**
 * @brief Nibble lookup table for CRC-16-CCITT / CCSDS (Poly: 0x1021, Normal).
 */
static const ui16_t orbitc_crc16_ccitt_nibble_table[16] = {
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U,
    0x8108U, 0x9129U, 0xA14AU, 0xB16BU, 0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU
};
#endif /* ENABLE_CRC16_CCITT */

#if (ENABLE_CRC16_IBM == 1)
/**
 * @brief Nibble lookup table for CRC-16-IBM / Modbus / ARC (Poly: 0xA001, Reflected).
 */
static const ui16_t orbitc_crc16_ibm_nibble_table[16] = {
    0x0000U, 0xCC01U, 0xD801U, 0x1400U, 0xF001U, 0x3C00U, 0x2800U, 0xE401U,
    0xA001U, 0x6C00U, 0x7800U, 0xB401U, 0x5000U, 0x9C01U, 0x8801U, 0x4400U
};
#endif /* ENABLE_CRC16_IBM */

#endif /* (ENABLE_CRC16 == 1) && (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE) */

#endif /* ORBITC_CRC16_TABLES_H */