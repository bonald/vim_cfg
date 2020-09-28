#!/bin/sh

#set -o errexit

SYSTEM_POINT=/mnt/flash
BOOT_POINT=/mnt/flash/boot
SYSTEM_DEV=`blkid | grep CTC-SYSTEM | awk '{print $1}' | sed 's/.$//'`
BOOT_DEV=`blkid | grep CTC-BOOT | awk '{print $1}' | sed 's/.$//'`
MOUNT_SYSTEM_DEV=`mount | grep $SYSTEM_POINT | grep -v $BOOT_POINT | awk '{print $1}'`
MOUNT_BOOT_DEV=`mount | grep $BOOT_POINT | awk '{print $1}'`

# probe if the partition is in use
echo "prepare to format partition..."
logger -p 4 -t format "prepare to format partition..."

if [ "$1" == "boot" ]; then
    BOOT_IN_USE="`fuser $BOOT_POINT | xargs -r -n 1 | awk '{print "the process " $1 " still use the partition boot"}'`"
    if [ "$BOOT_IN_USE" != "" ]; then
        echo "$BOOT_IN_USE"
        # for bug 54226, modified by tongzb for change log 
        echo "% some one still occupy of boot partition, please release"
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
        # for bug 54226, modified by tongzb for change log 
        echo "% some one still occupy of boot partition, please release"
        exit
    fi
fi

echo "prepare to format partition done!"
logger -p 4 -t format "prepare to format partition done!"

# backup and umount
echo "backup data and umount partition..."
logger -p 4 -t format "backup data and umount partition..."

if [ "$1" == "boot" ]; then
    if [ "$MOUNT_BOOT_DEV" != "" ]; then
        cd $BOOT_POINT
        tar czf /tmp/boot_before_format.tar.gz .factory-config.conf 2>/dev/null
        cd /
    fi
fi
if [ "$MOUNT_BOOT_DEV" != "" ]; then
    umount -l $BOOT_POINT
fi

if [ "$1" == "system" ]; then
    if [ "$MOUNT_SYSTEM_DEV" != "" ]; then
        cd $SYSTEM_POINT
        tar czf /tmp/system_before_format.tar.gz init_group init_meter init_flow startup-config.conf post_init_hook.sh reboot-info syslog boot 2>/dev/null
        cd /
    fi
fi
if [ "$MOUNT_SYSTEM_DEV" != "" ]; then
    umount -l $SYSTEM_POINT
fi

echo "backup data and umount partition done!"
logger -p 4 -t format "backup data and umount partition done!"

# format partition
echo "formatting partition..."
logger -p 4 -t format "formatting partition..."

if [ "$1" == "boot" ]; then
    mkfs.ext4 -q -F -L CTC-BOOT $BOOT_DEV > /dev/null 2>&1
fi

if [ "$1" == "system" ]; then
    mkfs.ext4 -q -F -L CTC-SYSTEM $SYSTEM_DEV > /dev/null 2>&1
fi

echo "formatting partition done!"
logger -p 4 -t format "formatting partition done!"

# remount and restore
echo "remount and restore data..."
logger -p 4 -t format "remount and restore data..."

mount -t ext4 $SYSTEM_DEV $SYSTEM_POINT
if [ "$1" == "system" ]; then
    cd $SYSTEM_POINT
    if [ -f /tmp/system_before_format.tar.gz ]; then
        mv /tmp/system_before_format.tar.gz $SYSTEM_POINT/system_before_format.tar.gz
        tar -xzf system_before_format.tar.gz -C .
        sync
        rm -rf system_before_format.tar.gz
    else
        mkdir -p boot
    fi
fi

mount -t ext4 $BOOT_DEV $BOOT_POINT
if [ "$1" == "boot" ]; then
    cd $BOOT_POINT
    if [ -f /tmp/boot_before_format.tar.gz ]; then
        mv /tmp/boot_before_format.tar.gz $BOOT_POINT/boot_before_format.tar.gz
        tar -xzf boot_before_format.tar.gz -C .
        sync
        rm -rf boot_before_format.tar.gz
    fi
fi
mount -t ext4 -o ro,remount $BOOT_DEV $BOOT_POINT

echo "remount and restore data done!"
logger -p 4 -t format "remount and restore data done!"

echo "done!"
