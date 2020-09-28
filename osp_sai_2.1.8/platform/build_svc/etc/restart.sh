#!/bin/sh

#echo "Syn system time to hardware clock"
/bin/hwclock --systohc > /dev/null 2>&1

#echo "kill all pm if it is using /mnt/flash"
fuser -mk /mnt/flash/boot > /dev/null 2>&1   
fuser -mk /mnt/flash > /dev/null 2>&1    

#echo "Umount flash before restart"
sync
umount /mnt/flash/boot > /dev/null 2>&1
umount /mnt/flash > /dev/null 2>&1
