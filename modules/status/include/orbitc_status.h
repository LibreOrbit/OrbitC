/**
 * @file orbitc_status.h
 * @brief Status reporting, error classification, and code encoding for OrbitC.
 *
 * Defines the unified 32-bit status representation encoding severity, module,
 * classification, and module-specific error codes into a single word.
 */

/**
 * @defgroup status Status
 * @brief Status code management, error severity classification, and assertion guards.
 * @{
 */

#ifndef ORBITC_STATUS_H
#define ORBITC_STATUS_H

#include "orbitc_types.h"

/* ========================================================================== */
/* Type Definitions and Constants                                             */
/* ========================================================================== */

/**
 * @brief Composite 32-bit status code type.
 *
 * Encodes four distinct 8-bit components:
 * - Bits 31..24: Severity (@ref status_sev_t)
 * - Bits 23..16: Originating Module (@ref status_mod_t)
 * - Bits 15..8:  Error Classification (@ref status_cls_t)
 * - Bits 7..0:   Module-specific Code (@ref status_code_t or custom enum)
 */
typedef ui32_t status_t;

/**
 * @brief Canonical success status value indicating zero errors.
 */
#define STATUS_OK ((status_t)0x00000000UL)

/* ========================================================================== */
/* Status Enumerations                                                        */
/* ========================================================================== */

/**
 * @brief Severity level of a status code.
 */
typedef enum
{
    STATUS_SEV_OK      = 0x00U, /**< Success condition; no error occurred. */
    STATUS_SEV_INFO    = 0x01U, /**< Informational status; execution succeeded normally. */
    STATUS_SEV_WARNING = 0x02U, /**< Non-fatal warning; operation completed with caveats. */
    STATUS_SEV_ERROR   = 0x03U, /**< Recoverable error condition; operation failed. */
    STATUS_SEV_FATAL   = 0x04U  /**< Unrecoverable or critical system failure. */
} status_sev_t;

/**
 * @brief Identifier of the subsystem generating the status.
 */
typedef enum
{
    STATUS_MOD_NONE      = 0x00U, /**< Unspecified module. */
    STATUS_MOD_TYPES     = 0x01U, /**< Core types subsystem. */
    STATUS_MOD_STATUS    = 0x02U, /**< Status and diagnostic subsystem. */
    STATUS_MOD_MEMORY    = 0x03U, /**< Memory management subsystem. */
    STATUS_MOD_STRINGS   = 0x04U, /**< String processing subsystem. */
    STATUS_MOD_CHECKSUMS = 0x05U, /**< Checksums and cryptographic hashing subsystem. */
    STATUS_MOD_ALLOCATOR = 0x06U, /**< Dynamic memory allocator subsystem. */
    STATUS_MOD_BITOPS    = 0x07U, /**< Bit manipulation and bitfield subsystem. */
    STATUS_MOD_ENDIAN    = 0x08U, /**< Endianness conversion and serialization subsystem. */
    STATUS_MOD_CONTAINERS = 0x09U  /**< Deterministic static containers subsystem. */
} status_mod_t;

/**
 * @brief Generic classification category of the status or error condition.
 */
typedef enum
{
    STATUS_CLS_NONE          = 0x00U, /**< No specific error classification. */
    STATUS_CLS_NULL_PTR      = 0x01U, /**< Null pointer dereference or argument violation. */
    STATUS_CLS_INVALID_PARAM = 0x02U, /**< General invalid argument or configuration. */
    STATUS_CLS_OUT_OF_RANGE  = 0x03U, /**< Numerical or indexing value out of allowed range. */
    STATUS_CLS_CAPACITY      = 0x04U, /**< Insufficient buffer size or storage capacity. */
    STATUS_CLS_OVERLAP       = 0x05U, /**< Disallowed memory region overlap detected. */
    STATUS_CLS_OVERFLOW      = 0x06U, /**< Arithmetic or buffer boundary overflow. */
    STATUS_CLS_NOT_FOUND     = 0x07U, /**< Target resource or element not located. */
    STATUS_CLS_UNTERMINATED  = 0x08U, /**< Missing termination delimiter (e.g., null character). */
    STATUS_CLS_ALIGNMENT     = 0x09U, /**< Hardware or type memory alignment violation. */
    STATUS_CLS_INTERNAL      = 0x0AU  /**< Internal assertion or consistency failure. */
} status_cls_t;

/**
 * @brief Common status codes across OrbitC modules.
 */
typedef enum
{
    STATUS_CODE_OK        = 0x00U, /**< Operation succeeded. */
    STATUS_CODE_NULL_ARG  = 0x01U, /**< A required pointer parameter was NULL. */
    STATUS_CODE_BAD_PARAM = 0x02U, /**< Parameter failed validation checks. */
    STATUS_CODE_RANGE     = 0x03U, /**< Value falls outside valid boundaries. */
    STATUS_CODE_CAPACITY  = 0x04U, /**< Buffer capacity is insufficient. */
    STATUS_CODE_OVERLAP   = 0x05U, /**< Memory buffers overlap when distinct regions are required. */
    STATUS_CODE_OVERFLOW  = 0x06U  /**< Value would overflow target container. */
} status_code_t;

/* ========================================================================== */
/* Bit Manipulation Macros                                                    */
/* ========================================================================== */

/**
 * @brief Packs severity, module, class, and code fields into a single status word.
 *
 * @param sev  Severity level (@ref status_sev_t).
 * @param mod  Originating module (@ref status_mod_t).
 * @param cls  Error classification (@ref status_cls_t).
 * @param code Specific error code (8-bit value).
 *
 * @return 32-bit packed @ref status_t value.
 */
#define STATUS_MAKE(sev, mod, cls, code) \
    ((status_t)(((((status_t)(sev)) & 0xFFUL) << 24U) | \
                ((((status_t)(mod)) & 0xFFUL) << 16U) | \
                ((((status_t)(cls)) & 0xFFUL) << 8U)  | \
                (((status_t)(code)) & 0xFFUL)))

/**
 * @brief Extracts the severity field from a status value.
 *
 * @param st Status code to inspect.
 * @return Extracted @ref status_sev_t value.
 */
#define STATUS_GET_SEV(st)  ((status_sev_t)(((st) >> 24U) & 0xFFUL))

/**
 * @brief Extracts the module identifier field from a status value.
 *
 * @param st Status code to inspect.
 * @return Extracted @ref status_mod_t value.
 */
#define STATUS_GET_MOD(st)  ((status_mod_t)(((st) >> 16U) & 0xFFUL))

/**
 * @brief Extracts the error classification field from a status value.
 *
 * @param st Status code to inspect.
 * @return Extracted @ref status_cls_t value.
 */
#define STATUS_GET_CLS(st)  ((status_cls_t)(((st) >> 8U) & 0xFFUL))

/**
 * @brief Extracts the module-specific code field from a status value.
 *
 * @param st Status code to inspect.
 * @return Extracted 8-bit code value.
 */
#define STATUS_GET_CODE(st) ((ui8_t)((st) & 0xFFUL))

/* ========================================================================== */
/* Status Helper Functions                                                    */
/* ========================================================================== */

/**
 * @brief Constructs a packed 32-bit status code from component fields.
 *
 * @param sev  Severity level.
 * @param mod  Originating module identifier.
 * @param cls  Error category classification.
 * @param code Subsystem-specific error code.
 *
 * @return Packed @ref status_t representation.
 */
status_t orbitc_status_make(status_sev_t sev, status_mod_t mod, status_cls_t cls, ui8_t code);

/**
 * @brief Determines whether a status indicates success.
 *
 * @param st Status code to evaluate.
 *
 * @retval ORBITC_TRUE  If severity corresponds to OK or INFO.
 * @retval ORBITC_FALSE If severity indicates a warning, error, or fatal state.
 */
bool_t orbitc_status_is_ok(status_t st);

/**
 * @brief Determines whether a status represents an error or fatal state.
 *
 * @param st Status code to evaluate.
 *
 * @retval ORBITC_TRUE  If severity is @ref STATUS_SEV_ERROR or @ref STATUS_SEV_FATAL.
 * @retval ORBITC_FALSE If severity is OK, INFO, or WARNING.
 */
bool_t orbitc_status_is_error(status_t st);

/**
 * @brief Latches a candidate status if the target is still in an OK state.
 *
 * Preserves the first non-OK status. If @p target is null, no operation is
 * performed. If the target already contains a warning, error, or fatal status,
 * it remains unchanged.
 *
 * @param[in,out] target    Status variable receiving the candidate.
 * @param[in]     candidate Candidate status to latch.
 */
void orbitc_status_latch(status_t *target, status_t candidate);

/**
 * @brief Retrieves the severity level from a status code.
 *
 * @param st Status code to inspect.
 * @return Decoded @ref status_sev_t value.
 */
status_sev_t orbitc_status_get_severity(status_t st);

/**
 * @brief Retrieves the originating module identifier from a status code.
 *
 * @param st Status code to inspect.
 * @return Decoded @ref status_mod_t value.
 */
status_mod_t orbitc_status_get_module(status_t st);

/**
 * @brief Retrieves the error category classification from a status code.
 *
 * @param st Status code to inspect.
 * @return Decoded @ref status_cls_t value.
 */
status_cls_t orbitc_status_get_class(status_t st);

/**
 * @brief Retrieves the lower 8-bit code value from a status code.
 *
 * @param st Status code to inspect.
 * @return Decoded 8-bit code.
 */
ui8_t orbitc_status_get_code(status_t st);

#endif /* ORBITC_STATUS_H */

/** @} */ /* End of group status */