#!/bin/sh

set -o errexit

SYSTEM_POINT=/mnt/flash
BOOT_POINT=/mnt/flash/boot
SYSTEM_DEV=`mount | grep $SYSTEM_POINT | grep -v $BOOT_POINT | awk '{print $1}'`
BOOT_DEV=`mount | grep $BOOT_POINT | awk '{print $1}'`

# probe if the partition is in use
echo "prepare to format partition system..."
logger -p 4 -t format "prepare to format partition system..."

if [ "$1" == "boot" ]; then
    BOOT_IN_USE="`lsof $BOOT_POINT | grep -v PID | awk '{print "the process " $2 " still use the partition boot"}'`"
    if [ "$BOOT_IN_USE" != "" ]; then
        echo "$BOOT_IN_USE"
        echo "% you can do 'cdmnt' for release occupy of partition"
        exit
    fi
fi

if [ "$1" == "system" ]; then
    BOOT_IN_USE="`lsof $BOOT_POINT | grep -v PID | awk '{print "the process " $2 " still use the partition boot"}'`"
    SYSTEM_IN_USE="`lsof $SYSTEM_POINT | grep -v PID | awk '{print "the process " $2 " still use the partition system"}'`"
    if [ "$BOOT_IN_USE" != "" ] || [ "$SYSTEM_IN_USE" != "" ]; then
        if [ "$BOOT_IN_USE" != "" ]; then
            echo "$BOOT_IN_USE"
        fi
        if [ "$SYSTEM_IN_USE" != "" ]; then
            echo "$SYSTEM_IN_USE"
        fi
        echo "% you can do 'cdmnt' for release occupy of partition"
        exit
    fi
fi

echo "prepare to format partition system done!"
logger -p 4 -t format "prepare to format partition system done!"

# backup and umount
echo "backup data and umount partition..."
logger -p 4 -t format "backup data and umount partition..."

if [ "$1" == "boot" ]; then
    cd $BOOT_POINT
    find . -print0 | cpio --null -ov --format=newc 2>/dev/null | gzip -c - > /tmp/boot_before_format.cpio.gz
    cd /
fi
umount -l $BOOT_POINT

if [ "$1" == "system" ]; then
    cd $SYSTEM_POINT
    find . -print0 | cpio --null -ov --format=newc 2>/dev/null | gzip -c - > /tmp/system_before_format.cpio.gz
    cd /
fi
umount -l $SYSTEM_POINT

echo "backup data and umount partition done!"
logger -p 4 -t format "backup data and umount partition done!"

# format partition
echo "formatting partition..."
logger -p 4 -t format "formatting partition..."

if [ "$1" == "boot" ]; then
    (echo "y") | mkfs.ext4 -F -L CNOS-BOOT $BOOT_DEV > /dev/null 2>&1
fi

if [ "$1" == "system" ]; then
    (echo "y") | mkfs.ext4 -F -L CNOS-FLASH $SYSTEM_DEV > /dev/null 2>&1
fi

echo "formatting partition done!"
logger -p 4 -t format "formatting partition done!"

# remount and restore
echo "remount and restore data..."
logger -p 4 -t format "remount and restore data..."

mount -t ext4 $SYSTEM_DEV $SYSTEM_POINT
if [ "$1" == "system" ]; then
    cd $SYSTEM_POINT
    mv /tmp/system_before_format.cpio.gz $SYSTEM_POINT/system_before_format.cpio.gz
    gzip -d $SYSTEM_POINT/system_before_format.cpio.gz
    cpio -i -F $SYSTEM_POINT/system_before_format.cpio > /dev/null 2>&1
    sync
    rm -rf $SYSTEM_POINT/system_before_format.cpio
fi

mount -t ext4 $BOOT_DEV $BOOT_POINT
if [ "$1" == "boot" ]; then
    cd $BOOT_POINT
    mv /tmp/boot_before_format.cpio.gz $BOOT_POINT/boot_before_format.cpio.gz
    gzip -d $BOOT_POINT/boot_before_format.cpio.gz
    cpio -i -F $BOOT_POINT/boot_before_format.cpio > /dev/null 2>&1
    sync
    rm -rf $BOOT_POINT/boot_before_format.cpio
fi

echo "remount and restore data done!"
logger -p 4 -t format "remount and restore data done!"

echo "done!"
