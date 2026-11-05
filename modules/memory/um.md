# Memory Module User Manual

## Overview

The OrbitC Memory module provides safe and deterministic low\-level memory manipulation functions for embedded and flight software.

It provides alternatives to standard memory operations while adding parameter validation, overlap detection, address overflow protection, OrbitC status reporting, and architecture\-specific optimization backends.

## Dependencies

The Memory module depends on the following OrbitC modules:

- types
- status

They must be enabled before using the module.

## Backend Selection

The module provides several compile\-time selectable implementation backends:

| Backend | Description |
| --- | --- |
| ORBITC\_MEM\_BACKEND\_PORTABLE | Fully portable byte\-by\-byte implementation. |
| ORBITC\_MEM\_BACKEND\_GENERIC32 | Optimized implementation using aligned 32\-bit accesses. |
| ORBITC\_MEM\_BACKEND\_CORTEX\_A9 | Optimized implementation for ARM Cortex\-A9 targets. |

The default backend is:

ORBITC\_MEM\_BACKEND\_GENERIC32

The backend can be selected at build time using:

ORBITC\_MEM\_BACKEND = 0

The active backend can be queried at runtime using:

orbitc\_memory\_active\_backend();

## Memory Operations

### Memory Initialization

Fill a memory region with a constant byte value:

status\_t status = orbitc\_memset(buffer, 0U, sizeof(buffer));

This is the OrbitC equivalent of memset().

### Memory Copy

Copy data between two non\-overlapping memory regions:

status\_t status = orbitc\_memcpy(dst, src, size);

orbitc\_memcpy() checks for overlapping memory regions and reports an error when an overlap is detected.

If overlap is possible, orbitc\_memmove() must be used instead.

### Memory Move

Safely copy data between potentially overlapping memory regions:

status\_t status = orbitc\_memmove(dst, src, size);

The copy direction is automatically selected to preserve data integrity.

### Memory Comparison

Compare two memory regions:

si32\_t diff;

status\_t status = orbitc\_memcmp(lhs, rhs, size, &diff);

The resulting value follows the usual comparison convention:

- diff < 0: lhs is lower than rhs;
- diff == 0: both regions are identical;
- diff > 0: lhs is greater than rhs.

### Memory Search

Search for the first occurrence of a byte:

const void \*found;

status\_t status = orbitc\_memchr(buffer, value, size, &found);

If the value is not found, the function returns MEMORY\_CODE\_NOT\_FOUND and found remains ORBITC\_NULL.

## Safety

Memory operations include runtime checks for common low\-level memory errors:

- null pointers;
- invalid memcpy() overlap;
- address arithmetic overflow;
- invalid output pointers.

Errors are reported through the OrbitC status\_t mechanism instead of relying only on returned pointers or comparison values.

## Backend Selection Guidelines

Use the **Portable** backend when maximum portability is required or when targeting an architecture without a dedicated optimized implementation.

Use **Generic32** on conventional 32\-bit targets where aligned word accesses are efficient.

Use **Cortex\-A9** when targeting ARM Cortex\-A9 platforms and the dedicated optimized implementation has been validated for the target hardware.

Optimized backends automatically fall back to portable operations when data size or alignment does not allow the optimized path.

