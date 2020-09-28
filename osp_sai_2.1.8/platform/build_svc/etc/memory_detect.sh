#!/bin/sh
value=1024
fstype=`mount |grep "flash/boot" | awk -F " " '{print $5}'`
if [ $fstype == "ext4" ];then
        dev=`mount | grep "mnt/flash/boot" | awk -F " " '{print $1}'`
        bootsize=`fdisk $dev -l | grep Disk | awk -F ", " '{print $2}'| awk -F " " '{print $1}'`
        bootsize_k=`expr $bootsize / $value`
        bootsize_M=`expr $bootsize_k / $value`
        dev=`mount | grep "mnt/flash" | grep -v "mnt/flash/boot" | grep -v "overlay" | awk -F " " '{print $1}'`
        systemsize=`fdisk $dev -l | grep Disk | awk -F ", " '{print $2}'| awk -F " " '{print $1}'`
        systemsize_k=`expr $systemsize / $value`
        systemsize_M=`expr $systemsize_k / $value`
        size_M=`expr $bootsize_M + $systemsize_M`
fi
if [ $fstype == "ubifs" ];then
        bootsize=`ubinfo -d 1 | grep "Total amount of logical eraseblocks" | awk -F "(" '{print $2}'| awk -F " " '{print $1}'`
        systemsize=`ubinfo -d 0 | grep "Total amount of logical eraseblocks" | awk -F "(" '{print $2}'| awk -F " " '{print $1}'`
        bootsize_k=`expr $bootsize / $value`
        bootsize_M=`expr $bootsize_k / $value`
        systemsize_k=`expr $systemsize / $value`
        systemsize_M=`expr $systemsize_k / $value`
        size_M=`expr $bootsize_M + $systemsize_M`
fi
mmsize=`cat /proc/meminfo  | grep MemTotal | awk -F ":" '{print $2}' | awk -F " " '{print $1}'`
mmsize_M=`expr $mmsize / $value`

echo "flash $size_M" > /tmp/ctc_memory_info
echo "memory $mmsize_M " >> /tmp/ctc_memory_info