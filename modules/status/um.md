# Status Module User Manual

## Overview

The OrbitC Status module provides a unified error reporting mechanism for the entire library.

Errors are represented by a 32\-bit status\_t value containing the severity, originating module, generic error class, and module\-specific error code.

The module also provides optional defensive assertions for validating parameters, capacities, memory ranges, and arithmetic operations without terminating the application.

## Dependencies

The Status module depends on:

- types

Other OrbitC modules use status as their common error reporting interface.

## Status Format

A status\_t is organized as follows:

| Bits | Field | Description |
| --- | --- | --- |
| 31..24 | Severity | Error severity level. |
| 23..16 | Module | Module reporting the status. |
| 15..8 | Class | Generic error category. |
| 7..0 | Code | Module\-specific error code. |

A status can be constructed using:

status\_t status = STATUS\_MAKE(STATUS\_SEV\_ERROR, STATUS\_MOD\_MEMORY, STATUS\_CLS\_NULL\_PTR, 1U);

STATUS\_OK represents successful execution.

## Status Handling

The result of an OrbitC operation should normally be checked using:

status\_t status = some\_orbitc\_function();

if (orbitc\_status\_is\_ok(status) == ORBITC\_FALSE)\{ /\* Handle error \*/\}

Status information can be decoded using:

orbitc\_status\_get\_severity(status);

orbitc\_status\_get\_module(status);

orbitc\_status\_get\_class(status);

orbitc\_status\_get\_code(status);

Equivalent STATUS\_GET\_\* macros are also available.

## Status Latching

orbitc\_status\_latch() can be used to preserve the first detected failure:

status\_t status = STATUS\_OK;

orbitc\_status\_latch(&status, candidate\_status);

Once a non\-OK status has been latched, subsequent candidates do not overwrite it.

This is useful when several validation steps contribute to a single operation status.

## Defensive Assertions

OrbitC assertions are non\-terminating.

Unlike the standard C assert(), they do not abort or reset the application. Instead, they report validation failures through an existing status\_t.

Available helpers include:

- orbitc\_assert\_not\_null() for pointer validation;
- orbitc\_assert\_param() for general conditions;
- orbitc\_assert\_range\_u32() for numerical bounds;
- orbitc\_assert\_capacity() for buffer capacities;
- orbitc\_assert\_no\_overlap() for memory\-region overlap;
- orbitc\_assert\_no\_overflow\_size\_add() for size arithmetic overflow.

Example:

status\_t status = STATUS\_OK;

orbitc\_assert\_not\_null(ptr, &status, STATUS\_MOD\_MEMORY, MEMORY\_CODE\_NULL\_SRC);

Assertions only update the status while it is still STATUS\_OK, preserving the first detected failure.

## Memory Range Validation

The helper:

orbitc\_ranges\_overlap(ptr\_a, len\_a, ptr\_b, len\_b);

checks whether two memory regions overlap.

It also detects address wraparound when computing the end of a memory region.

Unlike the assertion helpers, this function remains available independently of ORBITC\_ENABLE\_ASSERT.

## Assertion Configuration

Runtime assertions can be enabled or removed at compile time:

ORBITC\_ENABLE\_ASSERT = 1

When enabled, defensive validation updates the provided status\_t when an invariant is violated.

When disabled:

ORBITC\_ENABLE\_ASSERT = 0

assertion helpers become no\-op inline functions and introduce no runtime validation behavior.

## Usage Guidelines

Use status\_t as the standard error propagation mechanism between OrbitC modules.

Use defensive assertions to detect programmer errors and invalid API usage, but do not rely exclusively on assertions for safety\-critical validation when assertions may be disabled by the build configuration.

Critical runtime checks should remain explicitly implemented when required for correct or safe execution.

