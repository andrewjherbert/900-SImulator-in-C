#!/bin/sh
set +x
if [ "$1" = "" ]
then
    echo "Usage 903algolmasd demo [options]"
    exit
elif [ ! -e "demos/903algol/$1.txt" ]
then
    echo $1 not found in demos/903algol
    exit
fi
rm -f .reader .punch .ascii .save .data .plot.png .translate
#echo loading Algol
cp bin/903algol/alg16klg_masd_store .store
#echo convert input tape
./to900text demos/903algol/$1.txt
#echo run translator in library mode
./emu900 -j=13 $2 >.translate
if [ $?  == 2 ]
     then cat .translate # display translator output
          echo "\n*** Translator ran off end of input tape ***"
          exit 2
fi
cat .translate # display translator output
cp .save .data
# check to see if translation failed
grep --silent "^FAIL$" .translate
if [ $? != 0 ]
then
    # check to see if need to scan the library tape or not
    grep --silent "^FIRST  NEXT" .translate
    if [ $? != 0 ]
    then
	#echo Library scan
        ./emu900 -j=9 $2 -reader=bin/903algol/algol_tape3_iss5_plotting
        if [ $? != 0 ]
        then exit $?
        fi
    fi
    #echo run interpreter
    ./emu900 -j=10 $2 -reader=.data
    if [ $?  == 2 ]
    then echo "\n*** Program ran off input data tape ***"
    fi
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






