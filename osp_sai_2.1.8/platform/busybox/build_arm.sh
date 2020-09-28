#!/bin/sh

OUTDIR=$2

mkdir -p $OUTDIR
${OSP_MAKE} O=$OUTDIR distclean ARCH=arm
cp config $OUTDIR/.config
${OSP_MAKE} O=$OUTDIR oldconfig ARCH=arm 
${OSP_MAKE} O=$OUTDIR ARCH=arm CROSS_COMPILE=arm-hisiv200-linux-
