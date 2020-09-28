#!/bin/sh

dev=$1
systempoint=/mnt/flash
bootpoint=/mnt/flash/boot
udiskpoint=/mnt/udisk

bootdev=$(blkid | grep 'LABEL="CTC-BOOT"'| awk -F "/dev/|:" '{print $2}')
sysdev=$(blkid | grep 'LABEL="CTC-SYSTEM"'| awk -F "/dev/|:" '{print $2}')

if [ "$dev" == "$sysdev" ];then
    mount -t ext4 /dev/${sysdev} $systempoint
    if [ ! -d $bootpoint ]; then
    	mkdir $bootpoint
    fi
    sync
    mount -t ext4 -o ro /dev/${bootdev} $bootpoint
	touch /tmp/.fs_mount_done
	cd /dev
	ln -s ${bootdev} boot
elif [ "$dev" != "$bootdev" ];then
    if [ ! -d $udiskpoint ]; then
        mkdir $udiskpoint
    fi
    mount -t vfat /dev/$dev $udiskpoint
    if [ "$?" != "0" ];then
        echo "Mount USB disk failed!"
        logger -p crit "Mount USB disk failed!"
        rmdir $udiskpoint
        exit 1
    fi
    echo "Mount USB disk done!"
    logger -p crit "Mount USB disk done!"
fi
