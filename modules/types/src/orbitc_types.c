/*
 * ================================================================================
 *  File: modules/types/src/orbitc_types.c
 * ================================================================================
 */
#include "orbitc_types.h"

ui32_t orbitc_types_arch_bits(void)
{
    return (ui32_t)(sizeof(void *) * 8U);
}

bool_t orbitc_types_is_little_endian(void)
{
    const ui32_t probe = 0x01020304UL;
    const byte_t *bytes = (const byte_t *)&probe;
    return (bytes[0] == 0x04U) ? ORBITC_TRUE : ORBITC_FALSE;
}

bool_t orbitc_types_unaligned_allowed(void)
{
#if (ORBITC_ALLOW_UNALIGNED == 1)
    return ORBITC_TRUE;
#else
    return ORBITC_FALSE;
#endif
}