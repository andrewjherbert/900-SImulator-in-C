#!/bin/sh
echo Run X3 Functional test
cp x3_iss4 .reader
./emu900 
echo X3 loaded
rm -f .punch
touch .punch
time ./emu900  -j8 -v1 -a6823740
hexdump .reader



