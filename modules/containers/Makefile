# ================================================================================
# File: modules/containers/Makefile
# ================================================================================
include $(ROOT_DIR)/config.mk
include containers-config.mk

CONTAINERS_INC := -I$(CURDIR)/include

SRCS := $(wildcard $(CURDIR)/src/*.c)
OBJS := $(patsubst $(CURDIR)/src/%.c,$(OBJ_DIR)/containers_%.o,$(SRCS))

.PHONY: all
all: $(OBJS)

$(OBJ_DIR)/containers_%.o: $(CURDIR)/src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) $(MODULES_DEFINES) $(CONTAINERS_INC) -c $< -o $@