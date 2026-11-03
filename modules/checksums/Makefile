# ================================================================================
# File: modules/checksums/Makefile
# ================================================================================

include $(ROOT_DIR)/config.mk

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,$(OBJ_DIR)/checksums_%.o,$(SRCS))

.PHONY: all

all: $(OBJS)

$(OBJ_DIR)/checksums_%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) -c $< -o $@