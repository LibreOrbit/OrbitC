/*
 * ================================================================================
 * File: modules/status/include/orbitc_status.h
 * ================================================================================
 */
#ifndef ORBITC_STATUS_H
#define ORBITC_STATUS_H

#include "orbitc_types.h"

typedef ui32_t status_t;

#define STATUS_OK ((status_t)0x00000000UL)

typedef enum
{
    STATUS_SEV_OK      = 0x00U,
    STATUS_SEV_INFO    = 0x01U,
    STATUS_SEV_WARNING = 0x02U,
    STATUS_SEV_ERROR   = 0x03U,
    STATUS_SEV_FATAL   = 0x04U
} status_sev_t;

typedef enum
{
    STATUS_MOD_NONE      = 0x00U,
    STATUS_MOD_TYPES     = 0x01U,
    STATUS_MOD_STATUS    = 0x02U,
    STATUS_MOD_MEMORY    = 0x03U,
    STATUS_MOD_STRINGS   = 0x04U,
    STATUS_MOD_CHECKSUMS = 0x05U
} status_mod_t;

typedef enum
{
    STATUS_CLS_NONE          = 0x00U,
    STATUS_CLS_NULL_PTR      = 0x01U,
    STATUS_CLS_INVALID_PARAM = 0x02U,
    STATUS_CLS_OUT_OF_RANGE  = 0x03U,
    STATUS_CLS_CAPACITY      = 0x04U,
    STATUS_CLS_OVERLAP       = 0x05U,
    STATUS_CLS_OVERFLOW      = 0x06U,
    STATUS_CLS_NOT_FOUND     = 0x07U,
    STATUS_CLS_UNTERMINATED  = 0x08U,
    STATUS_CLS_ALIGNMENT     = 0x09U,
    STATUS_CLS_INTERNAL      = 0x0AU
} status_cls_t;

typedef enum
{
    STATUS_CODE_OK        = 0x00U,
    STATUS_CODE_NULL_ARG  = 0x01U,
    STATUS_CODE_BAD_PARAM = 0x02U,
    STATUS_CODE_RANGE     = 0x03U,
    STATUS_CODE_CAPACITY  = 0x04U,
    STATUS_CODE_OVERLAP   = 0x05U,
    STATUS_CODE_OVERFLOW  = 0x06U
} status_code_t;

#define STATUS_MAKE(sev, mod, cls, code) \
    ((status_t)(((((status_t)(sev)) & 0xFFUL) << 24U) | \
                ((((status_t)(mod)) & 0xFFUL) << 16U) | \
                ((((status_t)(cls)) & 0xFFUL) << 8U)  | \
                (((status_t)(code)) & 0xFFUL)))

#define STATUS_GET_SEV(st)  ((status_sev_t)(((st) >> 24U) & 0xFFUL))
#define STATUS_GET_MOD(st)  ((status_mod_t)(((st) >> 16U) & 0xFFUL))
#define STATUS_GET_CLS(st)  ((status_cls_t)(((st) >> 8U) & 0xFFUL))
#define STATUS_GET_CODE(st) ((ui8_t)((st) & 0xFFUL))

status_t     orbitc_status_make(status_sev_t sev, status_mod_t mod, status_cls_t cls, ui8_t code);
bool_t       orbitc_status_is_ok(status_t st);
bool_t       orbitc_status_is_error(status_t st);
void         orbitc_status_latch(status_t *target, status_t candidate);
status_sev_t orbitc_status_get_severity(status_t st);
status_mod_t orbitc_status_get_module(status_t st);
status_cls_t orbitc_status_get_class(status_t st);
ui8_t        orbitc_status_get_code(status_t st);

#endif
