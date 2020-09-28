#!/bin/sh
#mount -t vfat /dev/$1 /mnt/udisk
mntpoint=/mnt/udisk
if [ "`mount | grep $mntpoint`" == "" ]; then
    if [ ! -d $mntpoint ];then
        mkdir $mntpoint
    fi
    mount -t vfat /dev/$1 $mntpoint
    if [ "$?" != "0" ];then
        echo "Mount USB disk failed!"
        logger -p crit "Mount USB disk failed!"
        rmdir $mntpoint
        exit 1
    fi
    echo "Mount USB disk done!"
    logger -p crit "Mount USB disk done!"
    touch -c $mntpoint
    sync
fi
