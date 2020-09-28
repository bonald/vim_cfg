#!/bin/bash

chip="greatbelt goldengate usw"
feature_mode=$1

#################### Process arguments ####################
if [ $# -lt 1 ]; then
    echo "Need arguments which is a chip name!"
    exit
fi

while [ -n "$2" ]
do
    if ! [[ $chip =~ "$2" ]]; then
        echo "Chip name error!"
        echo "Usage: Chip name must be greatbelt goldengate duet2 or tsingma"
        exit
    else
        if [[ $all_chip =~ "$2" ]]; then
            echo "Too many $2 in arguments"
            exit
        fi
        all_chip="$all_chip ""$2"
    fi
    shift
done
	
	all_chip=${all_chip/duet2/usw}
		
echo "************************************************"
echo "auto generate ctc layer"
echo "************************************************"

############# Remove old ctc_$chipname_api.txt #############
for loop in $chip
do
    if [ -f "ctc_""$loop""_api.txt" ]; then
        rm "ctc_""$loop""_api.txt"
    fi
done

############## Generate ctc_$chipname_api.txt ##############
for loop in $all_chip
do
    tclsh generate_api_txt.tcl "$loop" "$feature_mode"
    sed -i 's/\s\+/ /g' "ctc_""$loop""_api.txt"
done

#################### Merge ctc_$chipname_api.txt to ctc_api.txt ####################
cat ctc_*_api.txt | sed -n -e '/##/p' | sort -u | sed -e '/##end of file##/d' > ctc_api.txt
allmodule=`cat ctc_api.txt | sed -e 's/##//g' | xargs`

for module in $allmodule
do
    cat /dev/null > tmp
    for chipname in $all_chip
    do
        cat ctc_$chipname\_api.txt | sed -n -e "/##$module##/,/##.*##/p" | sed -e '/##/d' >> tmp
    done

    sort -u tmp -o tmp
    sed -i "/##$module##/r tmp" ctc_api.txt
done

#################### Generate ctc_api.h ctc_api.c $chipname_api.c ###################
tclsh generate_api.tcl "$all_chip" "$allmodule"

################## Change no exit apis to NULL for every $chipname ##################
for loop in $all_chip
do
    cat ctc_$loop\_api.txt | awk -F"(" '{ print $1 }' | sed -e '/##/d' | sort -u -o ctc_$loop\_api.txt
	cat ctc_api.txt | awk -F"(" '{ print $1 }' | sed -e '/##/d' | sort -u -o ctc_api.txt
	comm -3 ctc_api.txt ctc_$loop\_api.txt > tmp
    sed -i "s/ctc_/ctc_$loop\_/g" tmp

	for i in `cat tmp`
	do
	    sed -i "s/$i,/NULL,/g" $loop\_api.c
	done

    rm tmp
done

#################### If API folder not exist, creat them ####################
if [ ! -d "../../core/api" ]; then
    mkdir "../../core/api"
fi

if [ ! -d "../../core/api/include" ]; then
    mkdir "../../core/api/include"
elif [ -f "../../core/api/include/ctc_api.h" ]; then
    rm ../../core/api/include/*.h
fi

if [ ! -d "../../core/api/src" ]; then
    mkdir "../../core/api/src"
fi
rm ../../core/api/src/*.c -rf

#################### Copy generated api files to API folder ####################
cp ctc_api.h ../../core/api/include/
cp ctcs_api.h ../../core/api/include/
cp ctc_api.c ../../core/api/src/
cp ctcs_api.c ../../core/api/src/
for loop in $all_chip
do
   cp $loop\_api.c ../../core/api/src/
done

#################### Remove not nessary files ####################
rm *_api.h
rm *_api.c
rm *_api.txt
