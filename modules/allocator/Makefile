# ================================================================================
# File: modules/allocator/Makefile
# ================================================================================
include $(ROOT_DIR)/config.mk
include allocator-config.mk

ALLOCATOR_INC := -I$(CURDIR)/include

SRCS := $(wildcard $(CURDIR)/src/*.c)
OBJS := $(patsubst $(CURDIR)/src/%.c,$(OBJ_DIR)/allocator_%.o,$(SRCS))

.PHONY: all
all: $(OBJS)

$(OBJ_DIR)/allocator_%.o: $(CURDIR)/src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) $(MODULES_DEFINES) $(ALLOCATOR_INC) -c $< -o $@