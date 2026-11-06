# ================================================================================
# File: modules/endian/endian-config.mk
# Description: Compile-time configuration for the OrbitC Endianness module.
# ================================================================================

# Backend selection:
# 0 = Pure portable C99 bit-shifts
# 1 = Compiler builtins (__builtin_bswap16/32/64) when available
ORBITC_ENDIAN_BACKEND ?= 1

# Enable 64-bit byte-swapping and buffer serialization (1 = Enabled, 0 = Stripped)
ENABLE_ENDIAN_64      ?= 1

MODULES_DEFINES += \
    -DORBITC_ENDIAN_BACKEND=$(ORBITC_ENDIAN_BACKEND) \
    -DENABLE_ENDIAN_64=$(ENABLE_ENDIAN_64)