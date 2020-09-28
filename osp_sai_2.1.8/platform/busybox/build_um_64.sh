#!/bin/sh

OUTDIR=$2

mkdir -p $OUTDIR
${OSP_MAKE} O=$OUTDIR distclean
cp config $OUTDIR/.config
${OSP_MAKE} O=$OUTDIR oldconfig >/dev/null
${OSP_MAKE} O=$OUTDIR
