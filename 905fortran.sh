#!/bin/sh
set +x
rm -rf .reader .punch .reverse .save .ascii .linker
#echo load compiler
cp 905fortran_iss6_store .store
#echo convert input tape $1
to900text src/905fortran/$1
#echo compile program
./emu900 -j16 .reader .punch src/905fortran/O0R
echo
# save paper tape in case contains data
mv .reader .save
#echo reverse output
./reverse
#echo load loader
cp loader_iss3_store .store
#echo load program binary
./emu900 -j16 .reverse .punch src/905fortran/O20L >.linker
grep --silent "*LDR 000000" .linker
if [ $? != 0 ]
then
    echo
    ./emu900 -j16 905fortlib .punch src/905fortran/O3L
    #echo complete load and run
    echo
    # clear punch
    rm .punch
    touch .punch
    ./emu900 -j16 .save .punch src/905fortran/MM
    echo
    #echo check for punch output
    touch .ascii
    ./from900text
    echo
    if  [ ! -s .ascii ]
    then
        echo ***" No punch output ***"

    else
        echo "*** Punch output ***"
        cat .ascii
    fi
    echo
fi



