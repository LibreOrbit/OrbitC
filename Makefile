# ================================================================================
# File: Makefile
# ================================================================================
ROOT_DIR := $(CURDIR)

# Include library configuration
include $(ROOT_DIR)/config.mk

# Global variable aggregating all module-specific definitions
MODULES_DEFINES :=

# Inclusions of modular configurations (these populate MODULES_DEFINES)
ifeq ($(ENABLE_CHECKSUMS),1)
    include $(ROOT_DIR)/modules/checksums/checksum-config.mk
endif
ifeq ($(ENABLE_MEMORY),1)
    include $(ROOT_DIR)/modules/memory/memory-config.mk
endif
ifeq ($(ENABLE_ALLOCATOR),1)
    include $(ROOT_DIR)/modules/allocator/allocator-config.mk
endif
ifeq ($(ENABLE_BITOPS),1)
    include $(ROOT_DIR)/modules/bitops/bitops-config.mk
endif


MODULES :=
ifeq ($(ENABLE_TYPES),1)
    MODULES += modules/types
endif
ifeq ($(ENABLE_STATUS),1)
    MODULES += modules/status
endif
ifeq ($(ENABLE_MEMORY),1)
    MODULES += modules/memory
endif
ifeq ($(ENABLE_STRINGS),1)
    MODULES += modules/strings
endif
ifeq ($(ENABLE_CHECKSUMS),1)
    MODULES += modules/checksums
endif
ifeq ($(ENABLE_ALLOCATOR),1)
    MODULES += modules/allocator
endif
ifeq ($(ENABLE_BITOPS),1)
    MODULES += modules/bitops
endif

MODULE_TARGETS := $(patsubst %,build_%,$(MODULES))

TEST_SRCS := tests/test_main.c

ifeq ($(ENABLE_TYPES),1)
    TEST_SRCS += tests/test_types.c
endif
ifeq ($(ENABLE_STATUS),1)
    TEST_SRCS += tests/test_status.c
endif
ifeq ($(ENABLE_MEMORY),1)
    TEST_SRCS += tests/test_memory.c
endif
ifeq ($(ENABLE_STRINGS),1)
    TEST_SRCS += tests/test_strings.c
endif
ifeq ($(ENABLE_CHECKSUMS),1)
    TEST_SRCS += tests/test_checksums.c
endif
ifeq ($(ENABLE_ALLOCATOR),1)
    TEST_SRCS += tests/test_allocator.c
endif
ifeq ($(ENABLE_BITOPS),1)
    TEST_SRCS += tests/test_bitops.c
endif

.PHONY: all clean directories $(MODULE_TARGETS) lib test run_tests

all: lib

directories:
	@mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR)

$(MODULE_TARGETS): directories
	@$(MAKE) -C $(patsubst build_%,%,$@) ROOT_DIR=$(ROOT_DIR) MODULES_DEFINES="$(MODULES_DEFINES)"

lib: $(MODULE_TARGETS)
	@$(AR) $(ARFLAGS) $(BUILD_DIR)/liborbitc.a $(wildcard $(OBJ_DIR)/*.o)

test: lib directories
	@$(CC) $(CFLAGS_TEST) $(MODULES_DEFINES) $(TEST_SRCS) $(UNITY_SRC) -L$(BUILD_DIR) -lorbitc -o $(BIN_DIR)/orbitc_tests

run_tests: test
	@$(BIN_DIR)/orbitc_tests

clean:
	@$(RM) $(BUILD_DIR)