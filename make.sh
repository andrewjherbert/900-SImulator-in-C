#!/bin/sh
set -x
gcc -Wall -Wno-main -o emu900 src/emu900.c $(pkg-config libpng --cflags --libs)
gcc src/from900text.c -o from900text
gcc src/to900text.c -o to900text
gcc src/reverse.c -o reverse
