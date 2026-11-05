# Types Module User Manual

## Overview

The OrbitC Types module provides the core primitive types and architecture abstractions used throughout the OrbitC library.

It defines fixed\-width integers, size and pointer types, boolean and byte abstractions, numeric limits, and target architecture properties.

Compile\-time assertions ensure that fundamental type sizes remain consistent with the assumptions made by OrbitC.

## Dependencies

The Types module has no dependency on other OrbitC modules.

It is a core module and is required by most other OrbitC components.

## Integer Types

OrbitC provides explicit\-width integer types:

| Type | Description |
| --- | --- |
| ui8\_t / si8\_t | Unsigned / signed 8\-bit integer |
| ui16\_t / si16\_t | Unsigned / signed 16\-bit integer |
| ui32\_t / si32\_t | Unsigned / signed 32\-bit integer |
| ui64\_t / si64\_t | Unsigned / signed 64\-bit integer |
| usize\_t | Unsigned type used for sizes and capacities |
| uptr\_t | Unsigned integer capable of representing a pointer |
| sptr\_t | Signed integer capable of representing a pointer |

Additional common types are available:

- byte\_t for raw bytes;
- f32\_t and f64\_t for floating\-point values;
- bool\_t with ORBITC\_TRUE and ORBITC\_FALSE;
- addr\_t and caddr\_t for generic memory addresses.

ORBITC\_NULL provides the OrbitC null pointer representation.

## Architecture Configuration

The target platform can be configured using:

ORBITC\_ARCH\_BITS = 32 or 64

ORBITC\_LITTLE\_ENDIAN = 1 or 0

ORBITC\_ALLOW\_UNALIGNED = 1 or 0

These settings describe the architecture expected by other OrbitC modules and should match the actual target platform.

## Compile\-Time Validation

OrbitC validates fundamental assumptions during compilation using:

ORBITC\_STATIC\_ASSERT(condition, identifier);

The Types module automatically verifies the size of integer, floating\-point, size, and pointer types.

For example, ui32\_t must always occupy exactly 4 bytes and uptr\_t must have the same size as a pointer.

An incompatible target or compiler configuration therefore results in a build failure rather than an undetected runtime issue.

## Numeric Limits

Common maximum values are provided directly by OrbitC:

ORBITC\_UI8\_MAX

ORBITC\_UI16\_MAX

ORBITC\_UI32\_MAX

ORBITC\_UI64\_MAX

ORBITC\_USIZE\_MAX

ORBITC\_UPTR\_MAX

These constants can be used for range checking and overflow protection without relying on the standard C library.

## Runtime Introspection

The active platform characteristics can be queried at runtime:

orbitc\_types\_arch\_bits();

returns the actual pointer width of the compiled target.

orbitc\_types\_is\_little\_endian();

detects the runtime byte order.

orbitc\_types\_unaligned\_allowed();

returns whether unaligned accesses are enabled by the OrbitC build configuration.

## Usage Guidelines

OrbitC modules should use the provided types instead of relying directly on compiler\-specific C primitive sizes.

For example:

ui32\_t counter;

usize\_t buffer\_size;

byte\_t data[32];

bool\_t valid = ORBITC\_FALSE;

Architecture configuration must always match the target hardware and compiler ABI, especially when building flight software for different 32\-bit and 64\-bit targets.

