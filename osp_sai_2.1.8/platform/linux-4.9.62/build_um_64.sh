#!/bin/sh
set -o errexit
set -o pipefail

print_usage_and_exit()
{
    echo "Usage: build_uml.sh <obj_dir>"
    exit 1
}

if [ -z $1 ]; then
    print_usage_and_exit
fi

if [ ! -d $1 ]; then
    mkdir -p $1
fi

KDIR_UML=$1

if [ ! -f Makefile ]; then
    touch Makefile
fi

msg=`comm -3 Makefile.r Makefile`
if [ "$msg" != "" ]; then
    cp -f Makefile.r Makefile
fi

if [ "$2" == "clean" ]; then
   $CTC_MAKE  O=$1 distclean
else
    if [ -f $KDIR_UML/.config ]; then
        cd $KDIR_UML
        $CTC_MAKE ARCH=um CROSS_COMPILE= linux
    else
        $CTC_MAKE O=$KDIR_UML ARCH=um distclean
        $CTC_MAKE O=$KDIR_UML ARCH=um x86_64_defconfig >/dev/null
        $CTC_MAKE O=$KDIR_UML ARCH=um linux
        $CTC_MAKE O=$KDIR_UML ARCH=um modules
    fi
fi
