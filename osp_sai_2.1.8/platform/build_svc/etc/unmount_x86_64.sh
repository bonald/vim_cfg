#!/bin/sh
sync
HAVE_MOUNTED=`mount 2>&1 | grep 'udisk' | wc -L`
if [ ! $HAVE_MOUNTED == "0" ]; then
    umount /mnt/udisk 2>&1
fi
