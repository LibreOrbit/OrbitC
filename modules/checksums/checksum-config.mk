# ================================================================================
# File: modules/checksums/checksum-config.mk
# Description: Compile-time configuration for the OrbitC Checksums module.
#              Allows granular enabling/disabling of CRC widths and specific
#              polynomial tables to minimize ROM/Flash footprint in flight builds.
# ================================================================================

# --------------------------------------------------------------------------------
# CRC Family Master Switches (1 = Compiled, 0 = Stripped from binary)
# --------------------------------------------------------------------------------
ENABLE_CRC8                 ?= 0
ENABLE_CRC16                ?= 1
ENABLE_CRC32                ?= 0

# --------------------------------------------------------------------------------
# CRC-8 Algorithm / Table Switches
# --------------------------------------------------------------------------------
ENABLE_CRC8_STANDARD        ?= 1
ENABLE_CRC8_AUTOSAR         ?= 0

# --------------------------------------------------------------------------------
# CRC-16 Algorithm / Table Switches
# --------------------------------------------------------------------------------
ENABLE_CRC16_CCITT          ?= 1
ENABLE_CRC16_IBM            ?= 0

# --------------------------------------------------------------------------------
# CRC-32 Algorithm / Table Switches
# --------------------------------------------------------------------------------
ENABLE_CRC32_IEEE           ?= 1
ENABLE_CRC32_CASTAGNOLI     ?= 0

# --------------------------------------------------------------------------------
# Preprocessor Definitions Export
# --------------------------------------------------------------------------------
MODULES_DEFINES += \
    -DENABLE_CRC8=$(ENABLE_CRC8) \
    -DENABLE_CRC16=$(ENABLE_CRC16) \
    -DENABLE_CRC32=$(ENABLE_CRC32) \
    -DENABLE_CRC8_STANDARD=$(ENABLE_CRC8_STANDARD) \
    -DENABLE_CRC8_AUTOSAR=$(ENABLE_CRC8_AUTOSAR) \
    -DENABLE_CRC16_CCITT=$(ENABLE_CRC16_CCITT) \
    -DENABLE_CRC16_IBM=$(ENABLE_CRC16_IBM) \
    -DENABLE_CRC32_IEEE=$(ENABLE_CRC32_IEEE) \
    -DENABLE_CRC32_CASTAGNOLI=$(ENABLE_CRC32_CASTAGNOLI)