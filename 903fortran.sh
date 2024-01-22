#!/bin/sh
set +x
if [ "$1" = "" ]
then
    echo "Usage 903fortran demo [options]"
    exit
elif [ ! -e "demos/903fortran/$1.txt" ]
then
    echo $1 not found in demos/903fortran
    exit
fi
rm -f .reader .punch .ascii .translate
echo loading Fortran
cp bin/903fortran/fort16klg_iss5_store .store
echo convert input tape $1
./to900text demos/903fortran/$1.txt
echo read program
./emu900 -j=8 $2 >.translate
cp .save .data
if [ $? == 2 ]
then cat .translate
     echo -en "\n*** Translator ran off end of source tape ***"
     exit $?
fi
if [ ! -s .translate ]
then
    echo finalize program
    ./emu900 -j=10
    echo
    echo run program
    ./emu900 -j=11 -reader=.data
    if [ $? == 2 ]
    then echo -en "\n*** Program ran off end of data tape ***"
    fi
    echo
    echo
    touch .punch
    ./from900text
    if  [ ! -s .ascii ]
    then
        echo "*** No punch output ***"
    else
        echo "*** Punch output ***"
        cat .ascii
	echo
    fi
else
    cat .translate
    echo
    echo abandoned after translator errors
    echo
fi




