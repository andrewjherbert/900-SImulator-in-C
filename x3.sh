#!/bin/sh
echo Run X3 Functional test
./emu900 -reader=bin/misc/x3_iss4
echo X3 loaded
rm -f .punch
touch .punch
time ./emu900  -j8 -v1 -a6823740 
hexdump .punch



