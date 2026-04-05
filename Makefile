# Compilador y flags
CC = gcc
VERSION = 0.0.1

CFLAGS_ALL = -Wall -Werror -Wextra
CFLAGS_DEBUG = $(CFLAGS_ALL) -g -Ilib -DDEBUG
CFLAGS_RELEASE = $(CFLAGS_ALL) -O2 -Ilib -DVERSION=\"$(VERSION)\"
LDFLAGS = -Llib


# Archivos fuente
SRC = main.c lib/parser.c lib/types.c lib/utils.c lib/cli.c

# Carpeta para objetos
BUILD_DIR = build

# Objetos en build/ (manteniendo subcarpetas)
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

DATE = $(shell date +%Y-%m-%d--%H-%M-%S)
# Ejecutables
DEBUG = debug-$(DATE)
RELEASE = dist/gestion_trenes_v$(VERSION)

# Objetivo por defecto
all: $(DEBUG) $(RELEASE)

debug: $(DEBUG)

# Crear carpetas necesarias para los objetos
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

# Debug
$(DEBUG): $(OBJ)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(OBJ) $(LDFLAGS)

# Release
$(RELEASE): $(OBJ)
	$(CC) $(CFLAGS_RELEASE) -o $@ $(OBJ) $(LDFLAGS)

# Limpiar todo
clean:
	rm -rf $(BUILD_DIR)  
