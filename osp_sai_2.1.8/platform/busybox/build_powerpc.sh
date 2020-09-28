#!/bin/sh

OUTDIR=$2

mkdir -p $OUTDIR
${OSP_MAKE} O=$OUTDIR distclean ARCH=ppc
cp config $OUTDIR/.config
${OSP_MAKE} O=$OUTDIR oldconfig ARCH=ppc >/dev/null
${OSP_MAKE} O=$OUTDIR ARCH=ppc 
