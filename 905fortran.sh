#!/bin/sh
set +x
if [ "$1" = "" ]
then
    echo "Usage: 903algol.sh demo"
    exit
elif [ ! -e "demos/905fortran/$1.txt" ]
then
    echo $1 not found in demos/903fortran
    exit
fi
rm -rf .reader .punch .reverse .save .ascii .linker
#echo load compiler
cp bin/905fortran/905fortran_iss6_store .store
#echo convert input tape $1
./to900text demos/905fortran/$1.txt
#echo compile program
./emu900 -j16 .reader .punch bin/905fortran/O0R
echo
# save paper tape in case contains data
mv .reader .save
#echo reverse output
./reverse
#echo load loader
cp bin/905fortran/loader_iss3_store .store
#echo load program binary
./emu900 -j16 .reverse .punch bin/905fortran/O20L >.linker
grep --silent "*LDR 000000" .linker
if [ $? != 0 ]
then
    echo
    ./emu900 -j16 bin/905fortran/905fortlib .punch bin/905fortran/O3L
    #echo complete load and run
    echo
    # clear punch
    rm .punch
    touch .punch
    ./emu900 -j16 .save .punch bin/905fortran/MM
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



