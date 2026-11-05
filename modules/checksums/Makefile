# ================================================================================
# File: modules/checksums/Makefile
# ================================================================================
include $(ROOT_DIR)/config.mk
include checksum-config.mk

CHECKSUMS_INC := -I$(CURDIR)/tables -I$(CURDIR)/include

SRCS := $(wildcard $(CURDIR)/src/*.c)
OBJS := $(patsubst $(CURDIR)/src/%.c,$(OBJ_DIR)/checksums_%.o,$(SRCS))

.PHONY: all
all: $(OBJS)

$(OBJ_DIR)/checksums_%.o: $(CURDIR)/src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS_PROD) $(MODULES_DEFINES) $(CHECKSUMS_INC) -c $< -o $@