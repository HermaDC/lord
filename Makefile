# Compilador
CC = gcc
VERSION = 0.0.2

# Flags
CFLAGS_ALL = -Wall -Werror -Wextra -Ilib
CFLAGS_DEBUG = $(CFLAGS_ALL) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS_ALL) -O2 -DVERSION=\"$(VERSION)\"

LDFLAGS = -Llib

# Fuentes
SRC = main.c lib/parser.c lib/types.c lib/utils.c lib/cli.c lib/interactive.c lib/linenoise-lib/linenoise.c

# Carpetas de build separadas
BUILD_DEBUG = build/debug
BUILD_RELEASE = build/release

# Objetos
OBJ_DEBUG = $(patsubst %.c,$(BUILD_DEBUG)/%.o,$(SRC))
OBJ_RELEASE = $(patsubst %.c,$(BUILD_RELEASE)/%.o,$(SRC))

# Binarios
DATE = $(shell date +%Y-%m-%d--%H-%M-%S)
DEBUG_BIN = $(BUILD_DEBUG)/debug-$(DATE)
RELEASE_BIN = dist/lord_v$(VERSION)

# Targets principales
.PHONY: all debug release clean run-debug run-release

all: debug release

debug: $(DEBUG_BIN)

release: $(RELEASE_BIN)

# ========================
# Reglas de compilación
# ========================

# Objetos debug
$(BUILD_DEBUG)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

# Objetos release
$(BUILD_RELEASE)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

# ========================
# Linkado
# ========================

# Debug
$(DEBUG_BIN): $(OBJ_DEBUG)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(OBJ_DEBUG) $(LDFLAGS)

# Release
$(RELEASE_BIN): $(OBJ_RELEASE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_RELEASE) -o $@ $(OBJ_RELEASE) $(LDFLAGS)

# ========================
# Ejecutar
# ========================

run-debug: debug
	./$(DEBUG_BIN)

run-release: release
	./$(RELEASE_BIN)

# ========================
# Limpieza
# ========================

clean:
	rm -rf build
