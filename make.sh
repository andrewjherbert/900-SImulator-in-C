#!/bin/sh
set -x
gcc src/emu900.c -o emu900
gcc src/from900text.c -o from900text
gcc src/to900text.c -o to900text
gcc src/reverse.c -o reverse
