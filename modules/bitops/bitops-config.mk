# ================================================================================
# File: modules/bitops/bitops-config.mk
# Description: Compile-time configuration for the OrbitC Bitops module.
# ================================================================================

# Backend selection:
# 0 = Pure portable C99 (SWAR / bitwise algorithms, no compiler intrinsics)
# 1 = Compiler builtins (__builtin_popcount, __builtin_clz, __builtin_ctz) when available
ORBITC_BITOPS_BACKEND ?= 1

# Enable 64-bit bit manipulation routines (1 = Enabled, 0 = Stripped to save ROM)
ENABLE_BITOPS_64      ?= 1

MODULES_DEFINES += \
    -DORBITC_BITOPS_BACKEND=$(ORBITC_BITOPS_BACKEND) \
    -DENABLE_BITOPS_64=$(ENABLE_BITOPS_64)