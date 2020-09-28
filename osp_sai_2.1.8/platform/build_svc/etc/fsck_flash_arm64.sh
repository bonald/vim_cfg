#!/bin/sh

set -o errexit

SYSTEM_POINT=/mnt/flash
BOOT_POINT=/mnt/flash/boot
SYSTEM_DEV=`blkid | grep CTC-SYSTEM | awk '{print $1}' | sed 's/.$//'`
BOOT_DEV=`blkid | grep CTC-BOOT | awk '{print $1}' | sed 's/.$//'`
MOUNT_SYSTEM_DEV=`mount | grep $SYSTEM_POINT | grep -v $BOOT_POINT | awk '{print $1}'`
MOUNT_BOOT_DEV=`mount | grep $BOOT_POINT | awk '{print $1}'`

# probe if the partition is in use
echo "prepare to fsck partition..."
logger -p 4 -t format "prepare to fsck partition..."

if [ "$1" == "boot" ]; then
    BOOT_IN_USE="`fuser $BOOT_POINT | xargs -r -n 1 | awk '{print "the process " $1 " still use the partition boot"}'`"
    if [ "$BOOT_IN_USE" != "" ]; then
        echo "$BOOT_IN_USE"
        echo "% you can do 'cdmnt' for release occupy of partition"
        exit
    fi
fi

if [ "$1" == "system" ]; then
    BOOT_IN_USE="`fuser $BOOT_POINT | xargs -r -n 1 | awk '{print "the process " $1 " still use the partition boot"}'`"
    SYSTEM_IN_USE="`fuser $SYSTEM_POINT | xargs -r -n 1 | awk '{print "the process " $1 " still use the partition system"}'`"
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

echo "prepare to fsck partition done!"
logger -p 4 -t format "prepare to fsck partition done!"

# umount
echo "umount partition..."
logger -p 4 -t format "umount partition..."

if [ "$MOUNT_BOOT_DEV" != "" ]; then
    umount -l $BOOT_POINT
fi

if [ "$MOUNT_SYSTEM_DEV" != "" ]; then
    umount -l $SYSTEM_POINT
fi

echo "umount partition done!"
logger -p 4 -t format "umount partition done!"

# format partition
echo "fsck partition..."
logger -p 4 -t format "fsck partition..."

if [ "$1" == "boot" ]; then
    fsck.ext4 -y $BOOT_DEV > /dev/null 2>&1
fi

if [ "$1" == "system" ]; then
    fsck.ext4 -y $SYSTEM_DEV > /dev/null 2>&1
fi

echo "fsck partition done!"
logger -p 4 -t format "fsck partition done!"

# remount
echo "remount..."
logger -p 4 -t format "remount..."

mount -t ext4 $SYSTEM_DEV $SYSTEM_POINT
mount -t ext4 -o ro $BOOT_DEV $BOOT_POINT

echo "remount done!"
logger -p 4 -t format "remount done!"

echo "done!"
