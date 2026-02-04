
VERSION = 1.0

debug:
	gcc -g -o gestion_trenes main.c utils.c -Wall -Wextra
	printf "\n"
	./gestion_trenes
release:
	gcc -o gestion_trenes_v$(VERSION) main.c utils.c -Wall -Wextra -O2