# Allocator Module User Manual

## Overview

The OrbitC Allocator provides bounded and deterministic dynamic memory management for embedded and flight software.

It operates on a fixed memory pool and provides optional safety mechanisms such as memory corruption detection, poisoning, allocation limits, critical-section hooks, and heap integrity monitoring.

## Dependencies

The allocator depends on the following OrbitC modules:

- `types`
- `status`

They must be enabled before using the allocator.

## Configuration

The module is configured at compile time through `allocator-config.mk`.

The main configuration options are:

| Option | Description |
|---|---|
| `ALLOCATOR_ALIGNMENT` | Required memory alignment. |
| `ALLOCATOR_MAX_ALLOC_SIZE` | Maximum size of a single allocation. |
| `ALLOCATOR_POLICY` | Allocation strategy: First-Fit, Best-Fit, or Bump/Arena. |
| `ALLOCATOR_ENABLE_CANARIES` | Enables block corruption detection. |
| `ALLOCATOR_ENABLE_POISONING` | Fills allocated/freed memory with known patterns. |
| `ALLOCATOR_ENABLE_FREE` | Enables or disables memory deallocation. |
| `ALLOCATOR_ENABLE_LOCKS` | Enables critical-section hooks for concurrent access. |
| `ALLOCATOR_STATIC_HEAP` | Uses the allocator-provided static memory pool. |
| `ALLOCATOR_STATIC_HEAP_SIZE` | Size of the built-in static heap. |

The default policy is **First-Fit** with an alignment of **8 bytes**.

## User-Provided Heap

The recommended approach is to provide a statically allocated memory pool:

`ORBITC_HEAP_STORAGE_DECLARE(heap, 4096);`

The allocator is then initialized over this memory region:

`orbitc_allocator_t allocator;`

`allocator_config_t config = {`
`    .mem_start = heap,`
`    .mem_size  = sizeof(heap)`
`};`

`orbitc_allocator_init(&allocator, &config);`

For specific memory regions such as SRAM, SDRAM, or DTCM, a dedicated linker section can also be used:

`ORBITC_HEAP_STORAGE_DECLARE_SECTION(heap, 4096, ".sram2");`

## Allocation

Memory is allocated through:

`void *ptr = ORBITC_NULL;`

`status_t status = orbitc_alloc(&allocator, 128U, &ptr);`

The returned pointer is guaranteed to respect `ALLOCATOR_ALIGNMENT`.

Allocation failures are reported through the OrbitC `status` module rather than through the returned pointer alone.

## Deallocation

When enabled:

`orbitc_free(&allocator, ptr);`

Free blocks are automatically coalesced to reduce fragmentation.

Deallocation is unavailable when:

- `ALLOCATOR_ENABLE_FREE=0`
- the **Bump/Arena** allocation policy is selected.

## Heap Monitoring

The allocator can expose runtime memory statistics:

`allocator_stats_t stats;`

`orbitc_allocator_get_stats(&allocator, &stats);`

Available information includes:

- used and free memory;
- largest contiguous free block;
- allocation count;
- fragmentation information;
- allocator overhead;
- high-water mark.

For FDIR or periodic health monitoring, the complete heap can be verified using:

`orbitc_allocator_check_integrity(&allocator);`

This detects invalid block metadata, corrupted canaries, inconsistent links, and memory accounting errors.

## Reset

The complete heap can be restored to its initial empty state with:

`orbitc_allocator_reset(&allocator);`

All existing allocations become invalid after a reset.

## Typical Flight Configuration

For flight software where runtime deallocation is undesirable, a deterministic configuration can use the **Bump/Arena** policy:

`ALLOCATOR_POLICY = 2`
`ALLOCATOR_ENABLE_FREE = 0`
`ALLOCATOR_ENABLE_CANARIES = 1`
`ALLOCATOR_ENABLE_POISONING = 1`

This provides simple bounded allocation while preventing runtime heap fragmentation.