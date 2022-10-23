CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Wextra -Werror

.PHONY: all
all: nyush

nyush: nyush.o nyushFunctions.o

nyush.o: nyush.c nyushFunctions.h

nyushFunctions.o: nyushFunctions.c nyushFunctions.h

.PHONY: clean
clean:
	rm -f *.o nyush
