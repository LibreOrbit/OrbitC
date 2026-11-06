# ================================================================================
# File: modules/bitops/Makefile
# ================================================================================
include $(ROOT_DIR)/config.mk
include bitops-config.mk

BITOPS_INC := -I$(CURDIR)/include

SRCS := $(wildcard $(CURDIR)/src/*.c)
OBJS := $(patsubst $(CURDIR)/src/%.c,$(OBJ_DIR)/bitops_%.o,$(SRCS))

.PHONY: all
all: $(OBJS)

$(OBJ_DIR)/bitops_%.o: $(CURDIR)/src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) $(MODULES_DEFINES) $(BITOPS_INC) -c $< -o $@