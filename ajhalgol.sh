#!/bin/sh
set +x
rm -f .reader .punch .ascii
echo loading Algol
./emu900 -v1 alg16klg_ajh
echo convert input tape
./to900text src/algol/$1
echo run translator 
./emu900 -v1 -j8
if [ $? != 0 ]
then exit $?
fi
echo 
echo
if [ $? != 0 ]
then exit $?
fi
#echo run interpreter
./emu900 -v1 -j10
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





