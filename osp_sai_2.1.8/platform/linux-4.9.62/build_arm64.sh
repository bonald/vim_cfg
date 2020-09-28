#!/bin/sh
set -o errexit
set -o pipefail

print_usage_and_exit()
{
    echo "Usage: build_arm64.sh <obj_dir> <r|d>"
    exit 1
}

KDIR_ARM=$1
VER=$2
CPU=$CPU

if [ -z ${KDIR_ARM} ]; then
    print_usage_and_exit
fi

if [ -z ${VER} ]; then
    print_usage_and_exit
fi

if [ ! -d ${KDIR_ARM} ]; then
    mkdir -p ${KDIR_ARM}
fi

if [ -f ${KDIR_ARM}/.config ]; then
    $OSP_MAKE O=${KDIR_ARM} 
else
    $OSP_MAKE O=${KDIR_ARM} distclean
    $OSP_MAKE O=${KDIR_ARM} ctc_${CPU}_${VER}_defconfig 
    $OSP_MAKE O=${KDIR_ARM}
fi
