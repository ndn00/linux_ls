

CFLAGS = -Wall -g -std=c11

all: build
build: main.c
	gcc $(CFLAGS) main.c -o myls

clean:
	rm -f myls