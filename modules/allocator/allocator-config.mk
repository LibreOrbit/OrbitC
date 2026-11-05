# ================================================================================
# File: modules/allocator/allocator-config.mk
# Description: Compile-time configuration for the OrbitC Ultra-Safe Allocator.
# ================================================================================

# Memory alignment in bytes (must be a power of 2: 4, 8, 16, 32, 64)
ALLOCATOR_ALIGNMENT          ?= 8

# Maximum allowed size for a single allocation request in bytes (security bound)
ALLOCATOR_MAX_ALLOC_SIZE     ?= 65536

# Allocation strategy:
# 0 = First-Fit (fast, standard fragmentation)
# 1 = Best-Fit  (slower, minimizes fragmentation)
# 2 = Bump/Arena (init-only allocation, O(1), orbitc_free is forbidden/disabled)
ALLOCATOR_POLICY             ?= 0

# Safety & Concurrency features (1 = Enabled, 0 = Disabled)
# CANARIES: Places magic numbers & checksums in headers to detect overflows / SEU
ALLOCATOR_ENABLE_CANARIES    ?= 1

# POISONING: Fills allocated memory with 0xAA and freed memory with 0xDD
ALLOCATOR_ENABLE_POISONING   ?= 1

# FREE_ENABLED: Set to 0 for strict flight profiles where deallocation is forbidden
ALLOCATOR_ENABLE_FREE        ?= 1

# LOCKS: Enables thread-safety critical section hooks (mutex / IRQ masking)
ALLOCATOR_ENABLE_LOCKS       ?= 1

# Built-in static heap (1 = Library instantiates a static buffer, 0 = User-provided buffer only)
ALLOCATOR_STATIC_HEAP        ?= 0
ALLOCATOR_STATIC_HEAP_SIZE   ?= 32768
ALLOCATOR_HEAP_SECTION       ?= ""

# Append definitions to the global MODULES_DEFINES variable
MODULES_DEFINES += \
    -DALLOCATOR_ALIGNMENT=$(ALLOCATOR_ALIGNMENT) \
    -DALLOCATOR_MAX_ALLOC_SIZE=$(ALLOCATOR_MAX_ALLOC_SIZE) \
    -DALLOCATOR_POLICY=$(ALLOCATOR_POLICY) \
    -DALLOCATOR_ENABLE_CANARIES=$(ALLOCATOR_ENABLE_CANARIES) \
    -DALLOCATOR_ENABLE_POISONING=$(ALLOCATOR_ENABLE_POISONING) \
    -DALLOCATOR_ENABLE_FREE=$(ALLOCATOR_ENABLE_FREE) \
    -DALLOCATOR_ENABLE_LOCKS=$(ALLOCATOR_ENABLE_LOCKS) \
    -DALLOCATOR_STATIC_HEAP=$(ALLOCATOR_STATIC_HEAP) \
    -DALLOCATOR_STATIC_HEAP_SIZE=$(ALLOCATOR_STATIC_HEAP_SIZE) \
    -DALLOCATOR_HEAP_SECTION=\"$(ALLOCATOR_HEAP_SECTION)\"