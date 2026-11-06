# ================================================================================
# File: modules/memory/memory-config.mk
# ================================================================================

# Memory backend: 0 = Portable byte, 1 = Generic 32-bit, 2 = ARM Cortex-A9
ORBITC_MEM_BACKEND ?= 1

MODULES_DEFINES += \
    -DORBITC_MEM_BACKEND=$(ORBITC_MEM_BACKEND)