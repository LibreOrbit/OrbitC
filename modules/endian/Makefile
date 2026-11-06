# ================================================================================
# File: modules/endian/Makefile
# ================================================================================
include $(ROOT_DIR)/config.mk
include endian-config.mk

ENDIAN_INC := -I$(CURDIR)/include

SRCS := $(wildcard $(CURDIR)/src/*.c)
OBJS := $(patsubst $(CURDIR)/src/%.c,$(OBJ_DIR)/endian_%.o,$(SRCS))

.PHONY: all
all: $(OBJS)

$(OBJ_DIR)/endian_%.o: $(CURDIR)/src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) $(MODULES_DEFINES) $(ENDIAN_INC) -c $< -o $@