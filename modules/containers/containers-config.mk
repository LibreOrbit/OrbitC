# ================================================================================
# File: modules/containers/containers-config.mk
# Description: Compile-time configuration for the OrbitC Deterministic Containers.
# ================================================================================

# Granular switches for container types (1 = Compiled, 0 = Stripped from ROM)
ENABLE_CONTAINER_RINGBUF ?= 1
ENABLE_CONTAINER_FIFO    ?= 1
ENABLE_CONTAINER_STACK   ?= 1
ENABLE_CONTAINER_MEMPOOL ?= 1

# Maximum number of blocks tracked by a single mempool_t instance (must be multiple of 32)
CONTAINER_MEMPOOL_MAX_BLOCKS ?= 256

MODULES_DEFINES += \
    -DENABLE_CONTAINER_RINGBUF=$(ENABLE_CONTAINER_RINGBUF) \
    -DENABLE_CONTAINER_FIFO=$(ENABLE_CONTAINER_FIFO) \
    -DENABLE_CONTAINER_STACK=$(ENABLE_CONTAINER_STACK) \
    -DENABLE_CONTAINER_MEMPOOL=$(ENABLE_CONTAINER_MEMPOOL) \
    -DCONTAINER_MEMPOOL_MAX_BLOCKS=$(CONTAINER_MEMPOOL_MAX_BLOCKS)