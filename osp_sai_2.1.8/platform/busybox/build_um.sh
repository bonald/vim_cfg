#!/bin/sh

OUTDIR=$2

mkdir -p $OUTDIR
${OSP_MAKE} O=$OUTDIR distclean ARCH=i386
cp config $OUTDIR/.config
${OSP_MAKE} O=$OUTDIR oldconfig ARCH=i386 >/dev/null
${OSP_MAKE} O=$OUTDIR ARCH=i386 
