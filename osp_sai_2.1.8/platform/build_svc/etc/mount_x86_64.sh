#!/bin/sh

DEV_RIGHT=`echo $1 | grep 'sd[a-z][1-9]' | wc -L`
if [ $DEV_RIGHT == "0" ]; then
    echo "DEV NOT RIGHT!"
    exit
fi

MOUNTPOINT=/mnt/udisk
if [ ! -d $MOUNTPOINT ]; then
    mkdir -p $MOUNTPOINT
fi

EXIST=`ls /dev/$1 2>/dev/null | wc -L`
if [ $EXIST == "0" ]; then
    echo "NO SUCH DEVICE!"
    exit
fi

NOT_LINUX_PARTITION=`e2label /dev/$1 2>&1 | grep 'Bad magic number' | wc -L`

if [ $NOT_LINUX_PARTITION == "0" ]; then
    echo "IS LINUX PARTITION!"
    CTC_LABEL=`e2label /dev/$1`
    NONE_LABEL=`e2label /dev/$1 | wc -L`
    if [ $NONE_LABEL == "0" ]; then
        echo "NOT CENTEC SYSTEM PARTITION!"
        mount -t ext4 /dev/$1 $MOUNTPOINT
        if [ "$?" != "0" ];then
            echo "MOUNT USB DISK FAILED!"
            logger -p crit "MOUNT USB DISK FAILED!"
        fi
        exit
    fi
    if [ $CTC_LABEL == "ONIE-BOOT" ] || [ $CTC_LABEL == "FLASH" ]; then
        echo "CENTEC SYSTEM PARTITION!"
    else
        echo "NOT CENTEC SYSTEM PARTITION!"
        mount -t ext4 /dev/$1 $MOUNTPOINT
        if [ "$?" != "0" ];then
            echo "MOUNT USB DISK FAILED!"
            logger -p crit "MOUNT USB DISK FAILED!"
        fi
        exit
    fi
else
    echo "NOT LINUX PARTITION!"
    EFI_PARTITION=`e2label /dev/$1 2>&1 | grep 'EFI' | wc -L`
    if [ $EFI_PARTITION == '0' ]; then
        echo "NOT CENTEC SYSTEM PARTITION!"
        mount -t vfat /dev/$1 $MOUNTPOINT
        if [ "$?" != "0" ];then
            echo "MOUNT USB DISK FAILED!"
            logger -p crit "MOUNT USB DISK FAILED!"
        fi
        exit
    else
        echo "CENTEC SYSTEM PARTITION!"
    fi
fi
