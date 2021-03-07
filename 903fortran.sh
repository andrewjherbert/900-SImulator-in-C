#!/bin/sh
set +x
if [ "$1" == "" ]
then
    echo demo file name missing
    exit
elif [ ! -e "demos/903fortran/$1.txt" ]
then
    echo $1 not found in demos/903fortran
    exit
fi
rm -f .reader .punch .ascii .translate
echo loading Fortran
cp bin/903fortran.fort16klg_iss5_store .store
echo convert input tape $1
./to900text demos/903fortran/$1.txt
echo read program
./emu900 -j8 >.translate
if [ $? != 0 ]
then exit $?
fi
if [ ! -s .translate ]
then
    echo signal program complete
    ./emu900 -j10
    echo
    echo run program
    ./emu900 -j11
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




