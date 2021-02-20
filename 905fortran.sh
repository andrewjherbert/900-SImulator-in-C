#!/bin/sh
set +x
rm -rf .reader .punch .reverse .save .ascii
#echo
echo
echo "***"
echo "*** Loading 905 FORTRAN compiler and reading source code tape."
echo "*** Outputs a relocatable binary tape and halts."
echo "***"
./emu900 905fortran_iss6
#echo convert input tape $1
to900text src/905fortran/$1
#echo compile program
./emu900 -j16 .reader .punch src/905fortran/O0R
echo
echo "***"
echo "*** Compilation complete - now loading binary using \"900 LINKER\"."
echo "***"
# save paper tape in case contains data
mv .reader .save
#echo reverse output
./reverse
#echo load loader
./emu900 loader_iss3
#echo load program binary
./emu900 -j16 .reverse .punch src/905fortran/O20L
echo
echo "***"
echo "*** Now loading FORTRAN library routines."
echo "***"
./emu900 -j16 905fortlib .punch src/905fortran/O3L
#echo complete load and run
echo
echo "***"
echo "*** Now running compiled program"
echo "***"
# clear punch
rm .punch
touch .punch
./emu900 -j16 .save .punch src/905fortran/MM
echo
echo "***"
echo "*** Program run complete."
echo "***"
#echo check for punch output
touch .ascii
./from900text
echo
echo "***"
if  [ ! -s .ascii ]
then
    echo ***" No punch output ***"
    echo "***"
else
    echo "*** Punch output ***"
    echo "***"
    cat .ascii
fi
echo



