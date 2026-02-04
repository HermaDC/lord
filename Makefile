CC = gcc
VERSION = 0.0.1

# Flags comunes
CFLAGS_COMMON = -Wall -Wextra -Werror
CFLAGS_DEBUG = -g -O0 -fsanitize=address,undefined,leak
CFLAGS_RELEASE = -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE -pie

SOURCES = main.c utils.c

# Targets
debug: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
debug: gestion_trenes_debug

release: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_RELEASE)
release: gestion_trenes_v$(VERSION)

# Compilación
gestion_trenes_debug: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o gestion_trenes_debug

gestion_trenes_v$(VERSION): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o dist/gestion_trenes_v$(VERSION)

# Ejecutar
run-debug: gestion_trenes_debug
	./gestion_trenes_debug

run-release: gestion_trenes_v$(VERSION)
	./dist/gestion_trenes_v$(VERSION)

# Limpiar
clean:
	rm -f gestion_trenes_debug dist/gestion_trenes_v$(VERSION)