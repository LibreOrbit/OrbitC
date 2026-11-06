# ================================================================================
# File: config.mk
# ================================================================================
CC ?= gcc
AR ?= ar
ARFLAGS ?= rcs
RM := rm -rf

# Target processor & architecture flags
CPU ?= generic
CPU_FLAGS ?=
ORBITC_ARCH_BITS ?= 64
ORBITC_LITTLE_ENDIAN ?= 1
ORBITC_ALLOW_UNALIGNED ?= 0

# Assertion configuration (1 = Enabled, 0 = Disabled/inert)
ORBITC_ENABLE_ASSERT ?= 1

# Module switches (1 = Enabled, 0 = Disabled)
ENABLE_TYPES ?= 1
ENABLE_STATUS ?= 1
ENABLE_MEMORY ?= 1
ENABLE_STRINGS ?= 1
ENABLE_CHECKSUMS ?= 1
ENABLE_ALLOCATOR ?= 1
ENABLE_BITOPS ?= 1
ENABLE_ENDIAN ?= 1
ENABLE_CONTAINERS ?= 1

BUILD_DIR ?= $(ROOT_DIR)/build
OBJ_DIR ?= $(BUILD_DIR)/obj
BIN_DIR ?= $(BUILD_DIR)/bin

# Unit testing configuration
# Override locally with: make test UNITY_ROOT=/path/to/unity
UNITY_ROOT ?= /opt/unity
UNITY_INC  := -I$(UNITY_ROOT)/src
UNITY_SRC  := $(UNITY_ROOT)/src/unity.c

ORBITC_INCLUDES := \
    -I$(ROOT_DIR)/modules/types/include \
    -I$(ROOT_DIR)/modules/status/include \
    -I$(ROOT_DIR)/modules/memory/include \
    -I$(ROOT_DIR)/modules/strings/include \
    -I$(ROOT_DIR)/modules/checksums/include \
    -I$(ROOT_DIR)/modules/allocator/include \
    -I$(ROOT_DIR)/modules/bitops/include \
    -I$(ROOT_DIR)/modules/endian/include \
    -I$(ROOT_DIR)/modules/containers/include

ORBITC_DEFINES := \
    -DORBITC_ARCH_BITS=$(ORBITC_ARCH_BITS) \
    -DORBITC_LITTLE_ENDIAN=$(ORBITC_LITTLE_ENDIAN) \
    -DORBITC_ALLOW_UNALIGNED=$(ORBITC_ALLOW_UNALIGNED) \
    -DORBITC_ENABLE_ASSERT=$(ORBITC_ENABLE_ASSERT) \
    -DENABLE_TYPES=$(ENABLE_TYPES) \
    -DENABLE_STATUS=$(ENABLE_STATUS) \
    -DENABLE_MEMORY=$(ENABLE_MEMORY) \
    -DENABLE_STRINGS=$(ENABLE_STRINGS) \
    -DENABLE_CHECKSUMS=$(ENABLE_CHECKSUMS) \
    -DENABLE_ALLOCATOR=$(ENABLE_ALLOCATOR) \
    -DENABLE_BITOPS=$(ENABLE_BITOPS) \
    -DENABLE_ENDIAN=$(ENABLE_ENDIAN) \
    -DENABLE_CONTAINERS=$(ENABLE_CONTAINERS)

CFLAGS_PROD := -std=c99 -ffreestanding -nostdinc -fno-builtin \
    -Wall -Wextra -Werror -O2 $(CPU_FLAGS) \
    $(ORBITC_DEFINES) $(ORBITC_INCLUDES)

CFLAGS_TEST := -std=c99 -Wall -Wextra -Werror -O2 $(CPU_FLAGS) \
    $(ORBITC_DEFINES) $(ORBITC_INCLUDES) $(UNITY_INC)