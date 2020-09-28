#!/bin/sh

OUTDIR=$2

mkdir -p $OUTDIR
${OSP_MAKE} O=$OUTDIR distclean ARCH=arm64
cp config $OUTDIR/.config
${OSP_MAKE} O=$OUTDIR oldconfig ARCH=arm64
${OSP_MAKE} O=$OUTDIR ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
