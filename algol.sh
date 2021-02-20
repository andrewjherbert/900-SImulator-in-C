#!/bin/sh
set +x
rm -f .reader .punch .ascii
#echo loading Algol
./emu900 alg16klg_masd
#echo convert input tape
./to900text src/algol/$1
#echo run translator
./emu900 -j8
if [ $? != 0 ]
then exit $?
fi
echo 
echo
#echo run interpreter
./emu900 -j10
touch .punch
./from900text
if  [ ! -s .ascii ]
then
    echo "*** No punch output ***"
    echo
else
    echo "*** Punch output ***"
    echo
    cat .ascii
fi






