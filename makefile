# makefile for Elliott 900 emulator
CC = gcc
SRC = ./src
PNG = `pkg-config libpng --cflags --libs`

emu900: $(SRC)/emu900.c
	$(CC) -Wall -Wno-main -o emu900 $(SRC)/emu900.c $(PNG)

from900text: $(SRC)/from900text.c
	$(CC) $(SRC)/from900text.c -o from900text

to900text: $(SRC)/to900text.c
	$(CC) $(SRC)/to900text.c -o to900text

reverse: $(SRC)/reverse.c
	$(CC) $(SRC)/reverse.c -o reverse

.PHONY: all

all: emu900 from900text to900text reverse

.PHONY: clean

clean:
	rm -f $(SRC)/*~

