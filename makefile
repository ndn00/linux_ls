

CFLAGS = -Wall -g -std=c11

all: build
build: main.c
	gcc $(CFLAGS) main.c -o myls

valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all -s ./myls -l / ./ main.c ../ ../test ../test/assn3Link ../assn3/main.c

clean:
	rm -f myls