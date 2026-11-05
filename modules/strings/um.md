# Strings Module User Manual

## Overview

The OrbitC Strings module provides safe and bounded string manipulation functions for embedded and flight software.

It provides alternatives to common C string operations while enforcing explicit buffer capacities, null-termination checks, overlap detection, and OrbitC status reporting.

## Dependencies

The Strings module depends on the following OrbitC modules:

- `types`
- `status`

They must be enabled before using the module.

## String Length

Determine the length of a bounded null-terminated string:

`usize_t length;`

`status_t status = orbitc_strlen_s(str, sizeof(str), &length);`

Unlike the standard `strlen()`, the function never reads beyond the specified capacity.

If no null terminator is found within the provided capacity, `STRINGS_CODE_UNTERMINATED` is returned.

## String Copy

Copy a string into a bounded destination buffer:

`status_t status = orbitc_strcpy_s(`
`    dst, sizeof(dst),`
`    src, sizeof(src));`

The function verifies:

- source null termination;
- destination capacity;
- source and destination overlap.

The destination remains null-terminated after a successful operation.

If the destination is too small, `STRINGS_CODE_TRUNCATED` is returned and the destination is set to an empty string.

## String Concatenation

Append a string to an existing bounded string:

`status_t status = orbitc_strcat_s(`
`    dst, sizeof(dst),`
`    src, sizeof(src));`

Both strings must be null-terminated within their respective capacities.

The operation fails if the resulting string cannot fit entirely inside the destination buffer.

## String Comparison

Compare two bounded strings:

`si32_t result;`

`status_t status = orbitc_strcmp_s(`
`    lhs, sizeof(lhs),`
`    rhs, sizeof(rhs),`
`    &result);`

The comparison result follows the usual convention:

- `result < 0`: `lhs` is lower than `rhs`;
- `result == 0`: both strings are equal;
- `result > 0`: `lhs` is greater than `rhs`.

## String Search

Search for the first occurrence of a character:

`const char *found;`

`status_t status = orbitc_strchr_s(`
`    str, sizeof(str), 'A', &found);`

Search for a substring:

`status_t status = orbitc_strstr_s(`
`    str, sizeof(str),`
`    "OrbitC", sizeof("OrbitC"),`
`    &found);`

If no match is found, `STRINGS_CODE_NOT_FOUND` is returned and the output pointer remains `ORBITC_NULL`.

## Safety

All string operations use explicit buffer capacities to prevent unbounded memory accesses.

The module detects common string handling errors including:

- null pointers;
- zero-capacity buffers;
- unterminated strings;
- insufficient destination capacity;
- overlapping source and destination regions;
- arithmetic overflow;
- unsuccessful searches.

Errors are reported through the OrbitC `status_t` mechanism.

## Usage Guidelines

Always provide the actual capacity of the underlying buffer, not the expected string length.

For example:

`char buffer[32];`

Operations on `buffer` should normally use:

`sizeof(buffer)`

rather than a manually calculated string length.

The `_s` functions are intended to replace common unbounded operations such as `strlen()`, `strcpy()`, `strcat()`, `strcmp()`, `strchr()`, and `strstr()` when explicit memory bounds and deterministic error reporting are required.