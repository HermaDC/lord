CC = gcc
VERSION = 0.0.1

# Flags comunes
CFLAGS_COMMON = -Wall -Wextra -Werror
CFLAGS_DEBUG = -g -O0 -fsanitize=address,undefined,leak -DDEBUG
CFLAGS_RELEASE = -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE -pie

# Carpetas
BUILD_DIR = build
DIST_DIR = dist

# Fuentes y objetos
SOURCES = main.c lib/utils.c lib/parser.c lib/types.c lib/cli.c
OBJ_DEBUG = $(patsubst %.c,$(BUILD_DIR)/debug_%.o,$(SOURCES))
OBJ_RELEASE = $(patsubst %.c,$(BUILD_DIR)/release_%.o,$(SOURCES))

# Binaries
TARGET_DEBUG = $(BUILD_DIR)/gestion_trenes_debug
TARGET_RELEASE = $(DIST_DIR)/gestion_trenes_v$(VERSION)

# --------------------------
# Targets principales
# --------------------------
debug: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
debug: $(TARGET_DEBUG)

release: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_RELEASE)
release: $(TARGET_RELEASE)

# --------------------------
# Compilación
# --------------------------
$(BUILD_DIR)/debug_%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/release_%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_RELEASE): $(OBJ_RELEASE)
	mkdir -p $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -DVERSION=\"$(VERSION)\"

# --------------------------
# Ejecutar
# --------------------------
run-debug: debug
	./$(TARGET_DEBUG)

run-release: release
	./$(TARGET_RELEASE)

# --------------------------
# Limpiar
# --------------------------
clean:
	rm -rf $(BUILD_DIR)
