# Compilador
CC ?= gcc

# Versión
VERSION ?= dev

# Flags
CFLAGS_COMMON = -Wall -Wextra -Werror -Ilib
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2 -DVERSION=\"$(VERSION)\"

LDFLAGS = -Llib

# Nombre del binario
BIN_NAME = lord

# Fuentes
SRC = main.c \
      lib/parser.c lib/types.c lib/utils.c \
      lib/cli.c lib/interactive.c
LIB_SRC = lib/linenoise-lib/linenoise.c
ALL_SRC = $(SRC) $(LIB_SRC)

# Directorios
BUILD_DIR = build
DIST_DIR = dist

BUILD_DEBUG = $(BUILD_DIR)/debug
BUILD_RELEASE = $(BUILD_DIR)/release

# Objetos
OBJ_DEBUG = $(patsubst %.c,$(BUILD_DEBUG)/%.o,$(ALL_SRC))
OBJ_RELEASE = $(patsubst %.c,$(BUILD_RELEASE)/%.o,$(ALL_SRC))

# Binarios
DEBUG_BIN = $(BUILD_DEBUG)/$(BIN_NAME)-debug
RELEASE_BIN = $(DIST_DIR)/$(BIN_NAME)

# Instalación
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

.PHONY: all debug release clean install uninstall

all: release

# Debug
debug: $(DEBUG_BIN)

$(BUILD_DEBUG)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

$(DEBUG_BIN): $(OBJ_DEBUG)
	$(CC) $(CFLAGS_DEBUG) -o $@ $^ $(LDFLAGS)

# Release
release: $(RELEASE_BIN)

$(BUILD_RELEASE)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

$(RELEASE_BIN): $(OBJ_RELEASE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_RELEASE) -o $@ $^ $(LDFLAGS)

# Install
install: release
	install -d $(BINDIR)
	install -m 755 $(RELEASE_BIN) $(BINDIR)/$(BIN_NAME)

uninstall:
	rm -f $(BINDIR)/$(BIN_NAME)

format:
	clang-format -i $(SRC)
# Clean
clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)
