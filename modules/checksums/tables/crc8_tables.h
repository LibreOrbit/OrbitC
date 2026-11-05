/**
 * @file crc8_tables.h
 * @brief Hardcoded 4-bit nibble lookup tables for CRC-8 algorithms.
 */
#ifndef ORBITC_CRC8_TABLES_H
#define ORBITC_CRC8_TABLES_H

#include "orbitc_types.h"

#if (ENABLE_CRC8 == 1) && (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE)

#if (ENABLE_CRC8_STANDARD == 1)
/**
 * @brief Nibble lookup table for CRC-8 Standard / SMBus (Poly: 0x07, Normal).
 */
static const ui8_t orbitc_crc8_standard_nibble_table[16] = {
    0x00U, 0x07U, 0x0EU, 0x09U, 0x1CU, 0x1BU, 0x12U, 0x15U,
    0x38U, 0x3FU, 0x36U, 0x31U, 0x24U, 0x23U, 0x2AU, 0x2DU
};
#endif /* ENABLE_CRC8_STANDARD */

#if (ENABLE_CRC8_AUTOSAR == 1)
/**
 * @brief Nibble lookup table for CRC-8 AUTOSAR (Poly: 0x2F, Normal).
 */
static const ui8_t orbitc_crc8_autosar_nibble_table[16] = {
    0x00U, 0x2FU, 0x5EU, 0x71U, 0xBCU, 0x93U, 0xE2U, 0xCDU,
    0x57U, 0x78U, 0x09U, 0x26U, 0xEBU, 0xC4U, 0xB5U, 0x9AU
};
#endif /* ENABLE_CRC8_AUTOSAR */

#endif /* (ENABLE_CRC8 == 1) && (ORBITC_CRC_BACKEND == ORBITC_CRC_BACKEND_TABLE) */

#endif /* ORBITC_CRC8_TABLES_H */