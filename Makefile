# 工程配置
PROJECT = zwlia
INCLUDE_SCPI = scpi
INCLUDE_TIRPC = tirpc
INCLUDE_HISLIP = hislip
BUILD_DIR = build

ifeq ($(ARCH),arm)
    LIB_DIR = lib/32
else ifeq ($(ARCH),arm64)
    LIB_DIR = lib/64
endif

OBJ_DIR = $(BUILD_DIR)/$(ARCH)/obj
BIN_DIR = $(BUILD_DIR)/$(ARCH)/bin
TARGET = $(BIN_DIR)/$(PROJECT)

CFLAGS = -Wall -Wextra -I$(INCLUDE_SCPI) -I$(INCLUDE_TIRPC) -I$(INCLUDE_HISLIP) -MMD $(ARCH_CFLAGS)
LDFLAGS = $(ARCH_LDFLAGS) -lm -pthread -L$(LIB_DIR) -lscpi -ltirpc -Wl,--as-needed

SRCS = $(shell find . -name '*.c')
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@printf "\033[1;32mBuild succeeded: executable generated -> %s\033[0m\n" "$@"

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
