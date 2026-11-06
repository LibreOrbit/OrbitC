# ================================================================================
# File: modules/memory/Makefile
# ================================================================================
include $(ROOT_DIR)/config.mk
include memory-config.mk

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,$(OBJ_DIR)/memory_%.o,$(SRCS))

.PHONY: all

all: $(OBJS)

$(OBJ_DIR)/memory_%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) $(MODULES_DEFINES) -c $< -o $@