# Endian Module User Manual

## Overview

The OrbitC Endian module provides portable byte\-order conversion and serialization utilities for embedded and flight software.

It supports byte swapping, host\-to\-endian conversions, and alignment\-independent reading and writing of integer values from raw byte buffers.

These operations are particularly useful for communication protocols, telemetry packets, hardware interfaces, and binary formats such as CCSDS/PUS.

## Dependencies

The Endian module depends on the following OrbitC modules:

- types
- status

They must be enabled before using the module.

## Configuration

The module is configured at compile time through endian\-config.mk.

The main configuration options are:

| Option | Description |
| --- | --- |
| ORBITC\_ENDIAN\_BACKEND | Selects the byte\-swap implementation backend. |
| ENABLE\_ENDIAN\_64 | Enables or removes 64\-bit operations. |

64\-bit support can be disabled when not required:

ENABLE\_ENDIAN\_64 = 0

This removes 64\-bit byte swapping and serialization functions from the build.

## Backend Selection

Two implementation backends are available:

- ORBITC\_ENDIAN\_BACKEND\_PORTABLE: pure C99 implementation using shifts and masks.
- ORBITC\_ENDIAN\_BACKEND\_BUILTIN: uses GCC/Clang byte\-swap intrinsics when available.

The active backend can be queried using:

orbitc\_endian\_active\_backend();

The portable backend should be preferred when compiler independence is required.

The builtin backend allows the compiler to use target\-specific byte\-swap instructions when available.

## Byte Swapping

Values can be byte\-swapped directly using:

ui16\_t swapped16 = orbitc\_bswap16(value16);

ui32\_t swapped32 = orbitc\_bswap32(value32);

When 64\-bit support is enabled:

ui64\_t swapped64 = orbitc\_bswap64(value64);

These functions always reverse the byte order regardless of the target architecture.

## Host Byte\-Order Conversion

OrbitC provides conversion helpers between host, Big\-Endian, and Little\-Endian representations:

orbitc\_htobe16(value);

orbitc\_be16toh(value);

orbitc\_htole16(value);

orbitc\_le16toh(value);

Equivalent helpers are available for 32\-bit and, when enabled, 64\-bit values.

The conversion is automatically selected according to the ORBITC\_LITTLE\_ENDIAN target configuration.

## Buffer Deserialization

Integer values can be safely decoded directly from raw byte buffers without requiring aligned accesses.

Example with a Big\-Endian 16\-bit field:

ui16\_t value;

status\_t status = orbitc\_read\_be16(buffer, buffer\_size, &value);

Available Big\-Endian readers include:

orbitc\_read\_be16();

orbitc\_read\_be32();

orbitc\_read\_be64();

Little\-Endian equivalents are provided through:

orbitc\_read\_le16();

orbitc\_read\_le32();

orbitc\_read\_le64();

The 64\-bit variants are only available when ENABLE\_ENDIAN\_64=1.

## Buffer Serialization

Host\-order integers can be serialized directly into raw byte buffers.

Example:

byte\_t buffer[4];

status\_t status = orbitc\_write\_be32( buffer, sizeof(buffer), value);

Big\-Endian writers:

orbitc\_write\_be16();

orbitc\_write\_be32();

orbitc\_write\_be64();

Little\-Endian writers:

orbitc\_write\_le16();

orbitc\_write\_le32();

orbitc\_write\_le64();

All serialization functions verify the supplied buffer capacity before accessing memory.

## Safety

Buffer serialization and deserialization functions operate byte\-by\-byte and do not require the input or output buffer to be naturally aligned.

The runtime API detects:

- null buffer pointers;
- null output pointers;
- insufficient buffer capacity.

Errors are reported through the OrbitC status\_t mechanism.

No direct cast from a byte buffer to ui16\_t \*, ui32\_t \*, or ui64\_t \* is required.

## Usage Guidelines

Use byte\-swap functions when the operation explicitly requires reversing the byte order of a value:

orbitc\_bswap32(value);

Use host conversion helpers when converting an integer representation between the CPU and an external byte order:

orbitc\_htobe32(value);

Use the buffer read/write API when parsing or generating binary protocols:

orbitc\_read\_be16(buffer, size, &value);

orbitc\_write\_be16(buffer, size, value);

For CCSDS or PUS packet serialization, prefer the Big\-Endian buffer API rather than pointer casting or architecture\-dependent structure packing.

This keeps packet encoding independent from host endianness, memory alignment, and compiler structure layout.

