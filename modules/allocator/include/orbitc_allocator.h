/**
 * @file orbitc_allocator.h
 * @brief Ultra-safe, deterministic memory allocator for flight software.
 *
 * Provides a bounded, corruption-resistant memory allocator supporting custom
 * user memory pools, linker section placement, header canaries, memory poisoning,
 * RTOS critical section hooks, and background FDIR heap integrity verification.
 *
 * @defgroup allocator Allocator
 * @brief Safe dynamic/arena memory management with corruption detection.
 * @{
 */
#ifndef ORBITC_ALLOCATOR_H
#define ORBITC_ALLOCATOR_H

#include "orbitc_types.h"
#include "orbitc_status.h"
#include "orbitc_assert.h"

/* ========================================================================== */
/* Allocation Policies                                                        */
/* ========================================================================== */

/** @brief First-Fit allocation policy (fast traversal, standard splitting). */
#define ORBITC_ALLOC_POLICY_FIRST_FIT 0

/** @brief Best-Fit allocation policy (minimizes memory fragmentation). */
#define ORBITC_ALLOC_POLICY_BEST_FIT  1

/** @brief Bump/Arena allocation policy (O(1) deterministic, free() is disabled). */
#define ORBITC_ALLOC_POLICY_BUMP      2

/* ========================================================================== */
/* Default Compile-Time Configuration Fallbacks                               */
/* ========================================================================== */

#ifndef ALLOCATOR_ALIGNMENT
#define ALLOCATOR_ALIGNMENT 8U
#endif

#ifndef ALLOCATOR_MAX_ALLOC_SIZE
#define ALLOCATOR_MAX_ALLOC_SIZE 65536U
#endif

#ifndef ALLOCATOR_POLICY
#define ALLOCATOR_POLICY ORBITC_ALLOC_POLICY_FIRST_FIT
#endif

#ifndef ALLOCATOR_ENABLE_CANARIES
#define ALLOCATOR_ENABLE_CANARIES 1
#endif

#ifndef ALLOCATOR_ENABLE_POISONING
#define ALLOCATOR_ENABLE_POISONING 1
#endif

#ifndef ALLOCATOR_ENABLE_FREE
#define ALLOCATOR_ENABLE_FREE 1
#endif

#ifndef ALLOCATOR_ENABLE_LOCKS
#define ALLOCATOR_ENABLE_LOCKS 1
#endif

#ifndef ALLOCATOR_STATIC_HEAP
#define ALLOCATOR_STATIC_HEAP 0
#endif

#ifndef ALLOCATOR_STATIC_HEAP_SIZE
#define ALLOCATOR_STATIC_HEAP_SIZE 32768U
#endif

#ifndef ALLOCATOR_HEAP_SECTION
#define ALLOCATOR_HEAP_SECTION ""
#endif

/* Compile-time validation of alignment configuration */
ORBITC_STATIC_ASSERT((ALLOCATOR_ALIGNMENT >= 4U), allocator_alignment_min_4_bytes);
ORBITC_STATIC_ASSERT(((ALLOCATOR_ALIGNMENT & (ALLOCATOR_ALIGNMENT - 1U)) == 0U), allocator_alignment_must_be_power_of_two);

/* ========================================================================== */
/* Linker Section & Buffer Declaration Helpers                                */
/* ========================================================================== */

#if defined(__GNUC__) || defined(__clang__)
/** @brief Forces alignment of a static heap buffer. */
#define ORBITC_ALIGNED_HEAP __attribute__((aligned(ALLOCATOR_ALIGNMENT)))
/** @brief Places a static heap buffer into a specific linker script section. */
#define ORBITC_SECTION_HEAP(sec_name) __attribute__((section(sec_name), aligned(ALLOCATOR_ALIGNMENT)))
#else
#define ORBITC_ALIGNED_HEAP
#define ORBITC_SECTION_HEAP(sec_name)
#endif

/**
 * @brief Helper macro to declare a properly aligned memory buffer in default RAM (.bss).
 * @param name Variable identifier for the byte array.
 * @param size Size of the memory pool in bytes.
 */
#define ORBITC_HEAP_STORAGE_DECLARE(name, size) \
    static byte_t name[(size)] ORBITC_ALIGNED_HEAP

/**
 * @brief Helper macro to declare an aligned memory buffer in a dedicated Linker Section.
 * Example: `ORBITC_HEAP_STORAGE_DECLARE_SECTION(g_sram_pool, 16384, ".sram2");`
 *
 * @param name     Variable identifier for the byte array.
 * @param size     Size of the memory pool in bytes.
 * @param sec_name Linker section string literal (e.g., ".dtcm", ".sdram").
 */
#define ORBITC_HEAP_STORAGE_DECLARE_SECTION(name, size, sec_name) \
    static byte_t name[(size)] ORBITC_SECTION_HEAP(sec_name)

/* ========================================================================== */
/* Status and Error Codes                                                     */
/* ========================================================================== */

/**
 * @brief Specific diagnostic status codes for allocator operations.
 */
typedef enum
{
    ALLOCATOR_CODE_OK             = 0x00U, /**< Operation completed successfully. */
    ALLOCATOR_CODE_NULL_PTR       = 0x01U, /**< A required pointer argument is NULL. */
    ALLOCATOR_CODE_NOT_INIT       = 0x02U, /**< Allocator instance has not been initialized. */
    ALLOCATOR_CODE_ALREADY_INIT   = 0x03U, /**< Allocator instance is already initialized. */
    ALLOCATOR_CODE_BAD_ALIGNMENT  = 0x04U, /**< Memory pool base address is not aligned to ALLOCATOR_ALIGNMENT. */
    ALLOCATOR_CODE_INVALID_SIZE   = 0x05U, /**< Requested size is zero, too small, or exceeds ALLOCATOR_MAX_ALLOC_SIZE. */
    ALLOCATOR_CODE_OUT_OF_MEMORY  = 0x06U, /**< Insufficient contiguous free memory to satisfy request. */
    ALLOCATOR_CODE_INVALID_PTR    = 0x07U, /**< Pointer is out of heap bounds or does not point to a valid block. */
    ALLOCATOR_CODE_DOUBLE_FREE    = 0x08U, /**< Attempted to free a block that is already marked as free. */
    ALLOCATOR_CODE_CORRUPTED      = 0x09U, /**< Block header canary or linkage corruption detected (SEU / overflow). */
    ALLOCATOR_CODE_FREE_DISABLED  = 0x0AU, /**< Deallocation is disabled by compile-time configuration or policy. */
    ALLOCATOR_CODE_BAD_LOCK_CFG   = 0x0BU  /**< Asymmetric lock configuration (only one of lock/unlock was provided). */
} allocator_code_t;

/* ========================================================================== */
/* Critical Section / Synchronization Hook Types                              */
/* ========================================================================== */

/**
 * @brief Callback function signature for entering or exiting a critical section.
 *
 * Can be used to wrap an RTOS mutex (e.g., `xSemaphoreTake` / `rtems_semaphore_obtain`)
 * or hardware interrupt masking.
 *
 * @param[in,out] lock_arg User-supplied opaque context pointer (e.g., mutex handle).
 */
typedef void (*allocator_lock_fn_t)(void *lock_arg);

/* ========================================================================== */
/* Data Structures                                                            */
/* ========================================================================== */

/**
 * @brief Forward declaration of internal block header structure.
 */
typedef struct orbitc_alloc_block orbitc_alloc_block_t;

/**
 * @brief Configuration structure for initializing an allocator instance.
 */
typedef struct
{
    void               *mem_start;  /**< Base address of the memory region (must be aligned to ALLOCATOR_ALIGNMENT). */
    usize_t             mem_size;   /**< Total size of the memory region in bytes. */
#if (ALLOCATOR_ENABLE_LOCKS == 1)
    allocator_lock_fn_t enter_crit; /**< Optional hook to acquire lock / enter critical section (or ORBITC_NULL). */
    allocator_lock_fn_t exit_crit;  /**< Optional hook to release lock / exit critical section (or ORBITC_NULL). */
    void               *lock_arg;   /**< Optional opaque argument passed to `enter_crit` and `exit_crit`. */
#endif
} allocator_config_t;

/**
 * @brief Runtime statistics and telemetry for an allocator instance.
 */
typedef struct
{
    usize_t total_size;       /**< Total raw size of the memory pool in bytes. */
    usize_t free_bytes;       /**< Currently available payload bytes across all free blocks. */
    usize_t used_bytes;       /**< Currently allocated payload bytes across all used blocks. */
    usize_t overhead_bytes;   /**< Bytes consumed by internal block headers. */
    usize_t largest_free_blk; /**< Size of the largest contiguous free block available. */
    usize_t high_watermark;   /**< Peak allocated payload bytes recorded since initialization. */
    ui32_t  alloc_count;      /**< Number of currently active allocations. */
    ui32_t  free_block_count; /**< Number of fragmented free blocks. */
} allocator_stats_t;

/**
 * @brief Instance context structure representing an independent memory heap.
 */
typedef struct
{
    ui32_t                init_magic;     /**< Magic marker validating initialized state. */
    byte_t               *heap_start;     /**< Start address of the managed memory pool. */
    byte_t               *heap_end;       /**< End address (exclusive) of the managed memory pool. */
    usize_t               total_size;     /**< Total managed size in bytes. */
    orbitc_alloc_block_t *head;           /**< Pointer to the first block in the heap. */
    usize_t               used_bytes;     /**< Currently allocated payload bytes. */
    usize_t               high_watermark; /**< Historical peak of allocated payload bytes. */
    ui32_t                alloc_count;    /**< Active allocation count. */
#if (ALLOCATOR_ENABLE_LOCKS == 1)
    allocator_lock_fn_t   enter_crit;     /**< Critical section entry callback. */
    allocator_lock_fn_t   exit_crit;      /**< Critical section exit callback. */
    void                 *lock_arg;       /**< User lock context argument. */
#endif
} orbitc_allocator_t;

/* ========================================================================== */
/* Core Allocator Interface                                                   */
/* ========================================================================== */

/**
 * @brief Initializes an allocator instance over a user-provided memory buffer.
 *
 * @param[out] alloc  Pointer to the allocator context to initialize.
 * @param[in]  config Pointer to the configuration descriptor (start address, size, and lock hooks).
 * @return Status indicating success, alignment error, capacity error, or bad lock configuration.
 */
status_t orbitc_allocator_init(orbitc_allocator_t *alloc, const allocator_config_t *config);

#if (ALLOCATOR_STATIC_HEAP == 1)
/**
 * @brief Initializes and retrieves the built-in static heap allocator instance.
 *
 * Only available when `ALLOCATOR_STATIC_HEAP=1` in `allocator-config.mk`.
 *
 * @param[out] out_alloc Pointer receiving the address of the internal static allocator instance.
 * @return Status indicating success or initialization failure.
 */
status_t orbitc_allocator_init_static(orbitc_allocator_t **out_alloc);
#endif

/**
 * @brief Allocates an aligned block of memory from the specified heap.
 *
 * Thread-safe when critical section hooks are configured. Guarantees that `*out_ptr`
 * is aligned to `ALLOCATOR_ALIGNMENT` bytes.
 *
 * @param[in,out] alloc   Pointer to an initialized allocator instance.
 * @param[in]     size    Number of bytes to allocate (must be > 0 and <= ALLOCATOR_MAX_ALLOC_SIZE).
 * @param[out]    out_ptr Pointer to store the address of the allocated memory block.
 * @return Status indicating success, out-of-memory, corruption, or invalid parameters.
 */
status_t orbitc_alloc(orbitc_allocator_t *alloc, usize_t size, void **out_ptr);

/**
 * @brief Frees a previously allocated memory block and coalesces adjacent free blocks.
 *
 * Thread-safe when critical section hooks are configured. Verifies pointer bounds,
 * exact block alignment, double-free conditions, and header/neighbor canaries.
 *
 * @param[in,out] alloc Pointer to the allocator instance owning the block.
 * @param[in]     ptr   Pointer to the memory payload to release.
 * @return Status indicating success, double-free, invalid pointer, corruption, or disabled free.
 */
status_t orbitc_free(orbitc_allocator_t *alloc, void *ptr);

/**
 * @brief Scans the entire heap to verify structural and canary integrity (FDIR / Watchdog).
 *
 * Thread-safe when critical section hooks are configured. Walks all blocks checking
 * magic numbers, pointer bounds, alignment, and size conservation.
 *
 * @param[in] alloc Pointer to the allocator instance to inspect.
 * @return `STATUS_OK` if the heap is intact, or `ALLOCATOR_CODE_CORRUPTED` upon anomaly detection.
 */
status_t orbitc_allocator_check_integrity(const orbitc_allocator_t *alloc);

/**
 * @brief Retrieves comprehensive telemetry and fragmentation statistics for the heap.
 *
 * Thread-safe when critical section hooks are configured.
 *
 * @param[in]  alloc     Pointer to the allocator instance to query.
 * @param[out] out_stats Pointer to the structure receiving the telemetry data.
 * @return Status indicating success, corruption, or invalid parameters.
 */
status_t orbitc_allocator_get_stats(const orbitc_allocator_t *alloc, allocator_stats_t *out_stats);

/**
 * @brief Resets the allocator to its initial empty state, wiping all allocations.
 *
 * Thread-safe when critical section hooks are configured.
 *
 * @param[in,out] alloc Pointer to an initialized allocator instance.
 * @return Status indicating success or invalid parameters.
 */
status_t orbitc_allocator_reset(orbitc_allocator_t *alloc);

#endif /* ORBITC_ALLOCATOR_H */
/** @} */ /* End of group allocator */