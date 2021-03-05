#!/bin/sh
set +x
rm -f .reader .punch .ascii .save .paper .translate
echo loading Algol
cp alg16klg_ajh_store .store
echo convert input tape
./to900text src/algol/$1
echo run translator in library mode
./emu900 -j12 >.translate
cp .reader .save
if [ $? != 0 ]
then exit $?
fi
cat .translate
grep --silent "^FAIL$" .translate
if [ $? != 0 ]
then
    echo
    echo scan library
    ./emu900 -j9 algol_tape3_iss7_plotting .reader
    echo 
    echo
    if [ $? != 0 ]
    then exit $?
    fi
    echo run interpreter
    ./emu900 -j10 .save
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
    if [ -e .paper ]
    then
    open -a preview .paper
    fi
else
    echo
    echo Abandoned after translation errors
    echo
 fi






