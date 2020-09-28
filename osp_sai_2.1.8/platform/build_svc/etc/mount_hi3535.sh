#!/bin/sh

cd /sys
dev=$1
systempoint=/mnt/flash
bootpoint=/mnt/flash/boot
udiskpoint=/mnt/udisk
dev_sg_no=$(find -name sd* | grep $dev | grep "host" | awk -F "host|/target" '{print $2}')
dev_isusb=$(cat /proc/scsi/usb-storage/$dev_sg_no | grep USB)

if [ ! -z "${dev_isusb}" ]; then
    if [ ! -d $udiskpoint ]; then
        mkdir $udiskpoint
    fi
    mount -t vfat /dev/$dev $udiskpoint
    if [ "$?" != "0" ];then
        echo "Mount USB disk failed!"
        logger -p crit "Mount USB disk failed!"
        rm -rf $udiskpoint
        exit 1
    fi
    echo "Mount USB disk done!"
    logger -p crit "Mount USB disk done!"
else
    #pre=${dev:0:3}
    #devid=${dev:3}
	pre=$(echo $dev | cut -c1-3)
	devid=$(echo $dev | cut -c4-4)
    if [ "${devid}" == "1" ]; then
        mount -t ext4 /dev/${pre}1 $systempoint
        if [ ! -d $bootpoint ]; then
                mkdir $bootpoint
        fi
        sync
        mount -t ext4 -o ro /dev/${pre}2 $bootpoint
		touch /tmp/.fs_mount_done
		cd /dev
		ln -s ${pre}2 boot
    fi
fi



