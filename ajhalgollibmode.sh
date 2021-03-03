#!/bin/sh
set +x
rm -f .reader .punch .ascii .save .paper
echo loading Algol
./emu900 -v1 alg16klg_ajh
echo convert input tape
./to900text src/algol/$1
echo run translator in library mode
./emu900 -v1 -j12
cp .reader .save
if [ $? != 0 ]
then exit $?
fi
echo scan library
./emu900 -v1 -j9 algol_tape3_iss7_plotting .reader 
echo 
echo
if [ $? != 0 ]
then exit $?
fi
#echo run interpreter
./emu900 -v1 -j10 .save
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
if [ -r .paper ]
then
    open -a preview .paper
fi






