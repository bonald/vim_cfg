#!/bin/sh

#for bug 51943, change partition for e550 board

if [ ! -b /dev/mmcblk0 ];then
	echo "/dev/mmcblk0 is not present"
	exit 1
fi

if [ ! -b /dev/mmcblk0p1 ] || [ ! -b /dev/mmcblk0p2 ];then
echo "n
p
1
1
50000


n
p
2
50001
108800

w

" | fdisk /dev/mmcblk0
fi
sleep 2s
bootdev=$(blkid | grep 'LABEL="CTC-BOOT"'| awk -F "/dev/|:" '{print $2}')
sysdev=$(blkid | grep 'LABEL="CTC-SYSTEM"'| awk -F "/dev/|:" '{print $2}')

if [ "$sysdev" != "mmcblk0p1" ];then
	mkfs.ext4 -L CTC-SYSTEM /dev/mmcblk0p1
fi

if [ "$bootdev" != "mmcblk0p2" ];then
	mkfs.ext4 -L CTC-BOOT /dev/mmcblk0p2
fi
mount /dev/mmcblk0p1 /mnt/flash/ 1>/dev/NULL 2>&1
mkdir /mnt/flash/boot/ 1>/dev/NULL 2>&1
mount /dev/mmcblk0p2 /mnt/flash/boot/ 1>/dev/NULL 2>&1

