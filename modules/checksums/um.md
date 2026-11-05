# Checksums Module User Manual

## Overview

The OrbitC Checksums module provides configurable CRC algorithms for embedded and flight software.

It supports CRC\-8, CRC\-16, and CRC\-32 algorithms with both one\-shot and streaming computation interfaces. Algorithms can be individually enabled or removed at compile time to minimize ROM and Flash usage.

## Dependencies

The Checksums module depends on the following OrbitC modules:

- types
- status

They must be enabled before using the module.

## Configuration

The module is configured at compile time through checksum\-config.mk.

CRC families can be enabled independently:

| Option | Description |
| --- | --- |
| ENABLE\_CRC8 | Enables CRC\-8 support. |
| ENABLE\_CRC16 | Enables CRC\-16 support. |
| ENABLE\_CRC32 | Enables CRC\-32 support. |

Individual algorithms can then be selected:

| Option | Algorithm |
| --- | --- |
| ENABLE\_CRC8\_STANDARD | CRC\-8 Standard / SMBus |
| ENABLE\_CRC8\_AUTOSAR | CRC\-8 AUTOSAR |
| ENABLE\_CRC16\_CCITT | CRC\-16\-CCITT\-FALSE / CCSDS |
| ENABLE\_CRC16\_IBM | CRC\-16 IBM / Modbus |
| ENABLE\_CRC32\_IEEE | CRC\-32 IEEE 802.3 |
| ENABLE\_CRC32\_CASTAGNOLI | CRC\-32C Castagnoli |

Only required algorithms should be enabled in embedded builds to minimize the final binary footprint.

## Backend Selection

Two implementation backends are available:

- ORBITC\_CRC\_BACKEND\_TABLE: lookup\-table implementation providing higher throughput at the cost of additional ROM.
- ORBITC\_CRC\_BACKEND\_BITWISE: bitwise implementation minimizing ROM usage at the cost of additional CPU cycles.

The active backend can be queried at runtime using:

orbitc\_checksums\_active\_backend();

## One\-Shot Computation

For a complete buffer available in memory, the simplest interface is the one\-shot API.

Example with CRC\-16 CCITT:

ui16\_t crc;

status\_t status = orbitc\_crc16\_compute( CRC16\_TYPE\_CCITT, data, data\_size, &crc);

Equivalent functions are available for CRC\-8 and CRC\-32:

orbitc\_crc8\_compute(...);

orbitc\_crc16\_compute(...);

orbitc\_crc32\_compute(...);

## Streaming Computation

For packets, files, communication frames, or data received incrementally, the streaming API avoids requiring the complete buffer in memory.

Example:

crc16\_ctx\_t ctx;ui16\_t crc;

orbitc\_crc16\_init(&ctx, CRC16\_TYPE\_CCITT);

orbitc\_crc16\_update(&ctx, header, header\_size);orbitc\_crc16\_update(&ctx, payload, payload\_size);

orbitc\_crc16\_final(&ctx, &crc);

Any number of update() calls can be performed between initialization and finalization.

This interface is particularly suitable for constrained systems where data is processed progressively.

## Error Handling

All operations return an OrbitC status\_t.

Typical errors include:

- null context;
- null input buffer;
- null output pointer;
- request for an algorithm that was not compiled into the binary.

The returned status should therefore always be checked before using the resulting CRC.

## Typical Flight Configuration

For a system requiring only the CCSDS CRC\-16 variant:

ENABLE\_CRC8 = 0ENABLE\_CRC16 = 1ENABLE\_CRC32 = 0

ENABLE\_CRC16\_CCITT = 1ENABLE\_CRC16\_IBM = 0

For memory\-constrained targets, the **bitwise backend** can be selected to reduce ROM usage.

For CPU\-constrained targets or frequently computed CRCs, the **table backend** should generally be preferred.

