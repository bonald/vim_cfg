#!/usr/bin/env bash

echo "---------------------------------------"
echo "Gen makefile configuration:"
echo "---------------------------------------"

### system ###
echo "Please select operation system: 0-linux, 1-vxworks"
echo -n "your choice:"
read line
case $line in
0) targetbase="linux" board="linux-board";;
1) targetbase="vxworks" board="vxworks-board";;
*) targetbase="linux" board="linux-board";;
esac
echo "your have selected system: $targetbase"

### chipname ###
chip_name=(greatbelt goldengate duet2 tsingma)
echo "Select your want to compile API types"
echo "chipname: 0-greatbelt, 1-goldengate, 2-duet2, 3-tsingma"
echo -n "please input the number, such as: 0 or 0-1 or 0-1 2, your choice: "
read line
num=${#line}
for str in $line
do
    FALSE=`expr index $str "-"`
if [ $FALSE != "0" ]
    then
        len=$FALSE-1
        chip=${str:0:$len}
        nextChip=${str:$FALSE}
    var=$chip
        while [ "$var" -le "$nextChip" ]
        do
            chipname="$chipname${chip_name[$var]} "
        ((var++))
        done
else
    chipname="$chipname${chip_name[$str]} "
fi
done

if [[ $num == "1" ]]
then
    chipname=${chipname%% *}
fi
echo "your have selected CHIPNAME: $chipname"

### arch ###
echo "Please selects the types of CPU"
echo "arch: 0-powerpc, 1-mips, 2-arm, 3-x86"
echo -n "your choice:"
read line
case $line in
0) arch="powerpc";;
1) arch="mips";;
2) arch="arm";;
3) arch="x86";;
*) arch="x86";;
esac
echo "your have selected ARCH: $arch"

### cpu ###
read -p "Please input your cpu:" cpu
echo "your have inputed CPU: $cpu"

### 64bit system select ###
is_64bits="no"
echo "Please select 64 bits or 32 bits sdk"
echo "cpu is 64bits: 0-no, 1-yes"
echo -n "your choice:"
read line
case $line in
0) is_64bits="FALSE";;
1) is_64bits="TRUE";;
*) is_64bits="FALSE";;
esac
echo "your have inputed M64: $is_64bits"

### version ###
echo "Please selected your version release or debug"
echo "ver: 0-release, 1-debug"
echo -n "your choice:"
read line
case $line in
0) ver="r";;
1) ver="d";;
*) ver="d";;
esac
echo "your have selected VER: $ver"

### CROSS_COMPILE ###
read -p "Please input your cross_complie:" cross_compile

### linux kernel select ###
is_kernel="no"
if [[ "linux"  == $targetbase ]];then
    echo "Please select sdk mode, user mode or kernel mode"
    echo "sdk in kernel: 0-no, 1-yes"
    echo -n "your choice:"
    read line
    case $line in
    0) is_kernel="FLASE";;
    1) is_kernel="TRUE";;
    *) is_kernel="FALSE";;
    esac
fi
echo "your have selected LINUX_LK: $is_kernel"

### linux kernel kbuild ###
read -rp "Please input your kbuild path:" kbuild
echo "your have inputed KDIR: $kbuild"

### linux M path ###
if [[ "TRUE"  == $is_kernel ]];then
    read -rp "Please input your sdk path:" m
    echo "your have inputed M: $is_kernel"
fi


### one lib system select ###
one_lib="no"
echo "Please select compile to one lib or multi small lib"
echo "Compile sdk to one lib: 0-no, 1-yes"
echo -n "your choice:"
read line
case $line in
0) one_lib="no";;
1) one_lib="yes";;
*) one_lib="no";;
esac
echo "your have inputed ONE_LIB: $one_lib"

### static lib or dynamic lib select ###
so_lib="no"
echo "Please select sdk compile to staic lib or dynamic lib"
echo "Static or dynamic lib: 0-static, 1-dynamic"
echo -n "your choice:"
read line
case $line in
0) so_lib="no";;
1) so_lib="yes";;
*) so_lib="no";;
esac
echo "your have inputed SO_LIB: $one_lib"

sed -i "s/targetbase =.*/targetbase = $targetbase/" Makefile.in
sed -i "s/BOARD =.*/BOARD = $board/" Makefile.in
sed -i "s/CHIPNAME =.*/CHIPNAME = $chipname/" Makefile.in
sed -i "s/ARCH =.*/ARCH = $arch/" Makefile.in
sed -i "s/CPU =.*/CPU = $cpu/" Makefile.in
sed -i "s/M64 =.*/M64 = $is_64bits/" Makefile.in
sed -i "s/VER =.*/VER = $ver/" Makefile.in
sed -i "s/CROSS_COMPILE =.*/CROSS_COMPILE = $cross_compile/" Makefile.in
sed -i "s/LINUX_LK =.*/LINUX_LK = $is_kernel/" Makefile.in
sed -i "s!KDIR =.*!KDIR = $kbuild!" Makefile.in
sed -i "s!M =.*!M = $m!" Makefile.in
sed -i "s/ONE_LIB =.*/ONE_LIB = $one_lib/" Makefile.in
sed -i "s/SO_LIB =.*/SO_LIB = $so_lib/" Makefile.in
