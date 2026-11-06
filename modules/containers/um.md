# Containers Module User Manual

## Overview

The OrbitC Containers module provides deterministic, statically allocated data structures for embedded and flight software.

It provides bounded containers operating exclusively on caller-provided memory, avoiding hidden dynamic allocation and allowing memory consumption to be determined at integration time.

The module currently provides:

- byte-oriented Ring Buffer;
- fixed-size element FIFO;
- fixed-size element Stack;
- fixed-size block Memory Pool.

Individual container types can be removed at compile time when not required.

## Dependencies

The Containers module depends on the following OrbitC modules:

- `types`
- `status`

They must be enabled before using the module.

## Configuration

The module is configured at compile time through `containers-config.mk`.

Individual container types can be enabled independently:

| Option | Description |
|---|---|
| `ENABLE_CONTAINER_RINGBUF` | Enables the byte-oriented Ring Buffer. |
| `ENABLE_CONTAINER_FIFO` | Enables the fixed-size element FIFO. |
| `ENABLE_CONTAINER_STACK` | Enables the fixed-size element Stack. |
| `ENABLE_CONTAINER_MEMPOOL` | Enables the fixed-size block Memory Pool. |
| `CONTAINER_MEMPOOL_MAX_BLOCKS` | Maximum number of blocks tracked by a Memory Pool instance. |

Only required containers should be enabled in constrained builds to minimize the final binary footprint.

`CONTAINER_MEMPOOL_MAX_BLOCKS` must be greater than zero and a multiple of 32.

## Ring Buffer

The Ring Buffer provides deterministic byte-oriented circular storage suitable for streams, communication interfaces, logs, and telemetry buffering.

Initialize it over caller-provided storage:

`byte_t storage[256];`
`ringbuf_t rb;`

`orbitc_ringbuf_init(`
`    &rb, storage, sizeof(storage),`
`    RINGBUF_MODE_REJECT);`

Data can then be written and read using:

`orbitc_ringbuf_write(&rb, data, size);`

`orbitc_ringbuf_read(&rb, output, size);`

Single-byte operations are also available:

`orbitc_ringbuf_push_byte(&rb, value);`

`orbitc_ringbuf_pop_byte(&rb, &value);`

Two overflow policies are supported:

- `RINGBUF_MODE_REJECT`: rejects writes when insufficient capacity is available;
- `RINGBUF_MODE_OVERWRITE`: overwrites the oldest data when necessary.

Overwrite mode is particularly suitable for continuous logging or flight-recorder style buffers where the most recent data must be preserved.

## FIFO

The FIFO provides a bounded First-In, First-Out queue for fixed-size elements.

Example:

`my_message_t storage[16];`
`fifo_t fifo;`

`orbitc_fifo_init(`
`    &fifo, storage,`
`    sizeof(my_message_t), 16U);`

Elements can then be queued and retrieved using:

`orbitc_fifo_push(&fifo, &message);`

`orbitc_fifo_pop(&fifo, &message);`

The next element can be inspected without removing it using:

`orbitc_fifo_peek(&fifo, &message);`

The FIFO rejects new elements when its configured capacity is reached.

## Stack

The Stack provides a bounded Last-In, First-Out container for fixed-size elements.

Example:

`my_item_t storage[16];`
`stack_t stack;`

`orbitc_stack_init(`
`    &stack, storage,`
`    sizeof(my_item_t), 16U);`

Elements are manipulated using:

`orbitc_stack_push(&stack, &item);`

`orbitc_stack_pop(&stack, &item);`

The current top element can be inspected without removing it using:

`orbitc_stack_peek(&stack, &item);`

Push operations are rejected when the stack reaches its configured capacity.

## Memory Pool

The Memory Pool provides deterministic fixed-size block allocation without using the general-purpose dynamic allocator.

Storage should preferably be declared using:

`ORBITC_MEMPOOL_STORAGE_DECLARE(pool_storage, 64U, 32U);`

The pool can then be initialized using:

`mempool_t pool;`

`orbitc_mempool_init(`
`    &pool, pool_storage,`
`    64U, 32U);`

Blocks are allocated using:

`void *block;`

`orbitc_mempool_alloc(&pool, &block);`

and returned using:

`orbitc_mempool_free(&pool, block);`

Allocation and deallocation use a fixed-size block model and an embedded free list.

The pool also maintains an allocation bitmap to detect invalid blocks and double-free conditions.

Memory Pools are particularly appropriate when deterministic allocation is required and objects have a known fixed maximum size.

## State Monitoring

Containers expose functions for monitoring their current state.

FIFO and Stack provide:

`orbitc_fifo_get_count(...);`

`orbitc_stack_get_count(...);`

as well as empty and full state queries.

The Ring Buffer provides:

`orbitc_ringbuf_get_count(...);`

`orbitc_ringbuf_get_free(...);`

The Memory Pool provides:

`orbitc_mempool_get_used_count(...);`

`orbitc_mempool_get_free_count(...);`

Internal high-water marks are also maintained by the container contexts to track peak utilization.

## Reset

Each container can be returned to its initial empty state without reallocating its backing storage:

`orbitc_ringbuf_reset(&rb);`

`orbitc_fifo_reset(&fifo);`

`orbitc_stack_reset(&stack);`

`orbitc_mempool_reset(&pool);`

Data or blocks previously stored or allocated by the container must be considered invalid after a reset.

## Safety

Container operations use fixed capacities and caller-provided storage.

Runtime validation detects conditions including:

- null pointers;
- uninitialized containers;
- invalid capacities or element sizes;
- full or empty containers;
- invalid indexes;
- memory pool alignment errors;
- invalid memory pool blocks;
- memory pool double-free attempts.

Errors are reported through the OrbitC `status_t` mechanism.

No container performs hidden dynamic memory allocation.

## Usage Guidelines

Use the **Ring Buffer** for byte streams such as telemetry, UART data, logs, or flight-recorder data.

Use the **FIFO** for ordered queues of fixed-size objects such as commands, events, or messages.

Use the **Stack** when Last-In, First-Out semantics are required with a bounded maximum depth.

Use the **Memory Pool** when objects require runtime allocation but deterministic fixed-size block management is preferred over a general-purpose allocator.

For flight software, container capacities should be selected from known worst-case resource requirements rather than expected average occupancy.