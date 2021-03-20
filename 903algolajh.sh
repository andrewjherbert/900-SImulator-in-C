#!/bin/sh
set -x
if [ "$1" = "" ]
then
    echo "Usage 903algolajh demo [options]"
    exit
elif [ ! -e "demos/903algol/$1.txt" ]
then
    echo $1 not found in demos/903algol
    exit
fi
rm -f .reader .punch .ascii .save .plot .translate
#echo loading Algol
cp bin/903algol/alg16klg_ajh_store .store
#echo convert input tape
./to900text demos/903algol/$1.txt
#echo run translator in library mode
./emu900 -j=12 $2 >.translate
cp .reader .save
if [ $? != 0 ]
then exit $?
fi
cat .translate
grep --silent "^FAIL$" .translate
if [ $? != 0 ]
then
    echo
    #echo scan library
    ./emu900 -j=9 $2 -reader=bin/903algol/algol_tape3_iss7_plotting
    echo 
    echo
    if [ $? != 0 ]
    then exit $?
    fi
    #echo run interpreter
    ./emu900 -j=10 $2 -reader=.save
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
    if [ -e .plot.png ]
    then
	OS=`uname`
        if   [ "$OS" = "Linux" ]
	then gpicview .plot.png &
	elif [ "$OS" = "Darwin" ]
	then open -a preview .plot.png
	else echo Cannot open plotter file
	fi
    fi
else
    echo
    echo Abandoned after translation errors
    echo
 fi






