#!/bin/sh
set +x
rm -f .reader .punch .ascii
#echo loading Fortran
./emu900 fort16klg_iss5
#echo convert input tape $1
./to900text src/903fortran/$1
#echo read program
./emu900 -j8
if [ $? != 0 ]
then exit $?
fi
#echo signal program complete
./emu900 -j10
echo
#echo run program
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
fi




