#!/bin/sh

OUTDIR=$2

mkdir -p $OUTDIR
${OSP_MAKE} O=$OUTDIR distclean ARCH=x86_64
cp config $OUTDIR/.config
${OSP_MAKE} O=$OUTDIR oldconfig ARCH=x86_64 > /dev/null
${OSP_MAKE} O=$OUTDIR ARCH=x86_64
