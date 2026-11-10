/* ================================================================================
 * File: modules/status/src/orbitc_status.c
 * ================================================================================
 */

 #include "orbitc_status.h"

status_t orbitc_status_make(status_sev_t sev, status_mod_t mod, status_cls_t cls, ui8_t code)
{
    return STATUS_MAKE(sev, mod, cls, code);
}

bool_t orbitc_status_is_ok(status_t st)
{
    status_sev_t severity = STATUS_GET_SEV(st);

    return ((severity == STATUS_SEV_OK) ||
            (severity == STATUS_SEV_INFO))
               ? ORBITC_TRUE
               : ORBITC_FALSE;
}

bool_t orbitc_status_is_error(status_t st)
{
    status_sev_t sev = STATUS_GET_SEV(st);
    return ((sev >= STATUS_SEV_ERROR) || (st != STATUS_OK)) ? ORBITC_TRUE : ORBITC_FALSE;
}

void orbitc_status_latch(status_t *target, status_t candidate)
{
    if (target != ORBITC_NULL)
    {
        if (orbitc_status_is_ok(*target) == ORBITC_TRUE)
        {
            *target = candidate;
        }
    }
}

status_sev_t orbitc_status_get_severity(status_t st)
{
    return STATUS_GET_SEV(st);
}

status_mod_t orbitc_status_get_module(status_t st)
{
    return STATUS_GET_MOD(st);
}

status_cls_t orbitc_status_get_class(status_t st)
{
    return STATUS_GET_CLS(st);
}

ui8_t orbitc_status_get_code(status_t st)
{
    return STATUS_GET_CODE(st);
}