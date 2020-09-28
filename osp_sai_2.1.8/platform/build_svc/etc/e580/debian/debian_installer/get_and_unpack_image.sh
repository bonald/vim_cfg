#!/bin/bash

# powerpc image format: size is big endian:
# /-----------------------------------------------------------------------------------------
# |                 |                   |                   |                   |             |
# |  64bytes header | 4bytes part1 size | 4bytes part2 size | 4bytes part3 size | 4bytes null |
# |                 |  (2bytes swap)    |  (2bytes swap)    |  (2bytes swap)    |             |
# /_________________|___________________|___________________|___________________|_____________|___
# 
# -----------------------------------------------------------------------------/
#    |        |               |        |               |        |               |
#    | part 1 | pad to 4bytes | part 2 | pad to 4bytes | part 3 | pad to 4bytes |
#    |        |               |        |               |        |               |
#    |   _____|_______________|________|_______________|________|_______________/


# get image
ipaddr=`fw_printenv ipaddr 2>/dev/null | awk -F '=' '{print $2}'`
netmask=`fw_printenv netmask 2>/dev/null | awk -F '=' '{print $2}'`
gateway=`fw_printenv gateway 2>/dev/null | awk -F '=' '{print $2}'`
serverip=`fw_printenv serverip 2>/dev/null | awk -F '=' '{print $2}'`

if [ "`cat /proc/cmdline 2>/dev/null | grep TftpFile`" != "" ]; then
    imagename=`cat /proc/cmdline | sed -e 's/ /\n/g' | grep bootimage | awk -F ':' '{print $3}'`
    if [ "$ipaddr" != "" ]; then
        ifconfig eth0 $ipaddr 1>/dev/null 2>&1
    fi
    if [ "$netmask" != "" ]; then
        ifconfig eth0 netmask $netmask 1>/dev/null 2>&1
    fi
    if [ "$gateway" != "" ]; then
        route add default gw $gateway 1>/dev/null 2>&1
    fi
    ifconfig eth0 up 1>/dev/null 2>&1

    network_reachable="`ping -c3 $serverip | awk '/[1-3] received/{print}'`"
    if [ "$network_reachable" == "" ]; then
        ifconfig eth0 down
        udhcpc -n -t 15 -T 3 -i eth0 -s /bin/udhcpc_def.script >/dev/ttyS0 2>/dev/null
        udhcpc_pid="`ps aux | grep udhcpc | grep -v grep | awk '{ print $2}'`"
        if [ "$udhcpc_pid" != "" ]; then
            kill -9 $udhcpc_pid
        fi
        network_reachable="`ping -c3 $serverip | awk '/[1-3] received/{print}'`"
        if [ "$network_reachable" == "" ]; then
            echo "Network Unreachable!"
            exit 0
        fi
    fi

    cd /tmp
    (echo "get $imagename") | tftp $serverip 1>/dev/null 2>&1
    cd - 1>/dev/null 2>&1    
    bootimage_name=/tmp/$imagename
else
    bootimage_name=`readlink -f /mnt/flash/boot/uImage`
fi


# unpack image
declare -a size_array

for i in $(seq 0 2)
do
    ((skip=64+4*$i))
    high=`hexdump -x -n 4 -s $skip $bootimage_name | sed -n '1, 1p' | awk '{print $2}'`
    low=`hexdump -x -n 4 -s $skip $bootimage_name | sed -n '1, 1p' | awk '{print $3}'`
    # size=0x${high:2:2}${high:0:2}${low:2:2}${low:0:2}
    size=0x${high:0:2}${high:2:2}${low:0:2}${low:2:2}
    size_array[$i]=$size
done

((initramfs_baseaddr=(80+(${size_array[0]}+3)/4*4)))
((initramfs_size=${size_array[1]}))
((initramfs_tailaddr=$initramfs_baseaddr+$initramfs_size))
# dd if=$bootimage_name of=/switch/initramfs.lzma bs=1 skip=$initramfs_baseaddr count=$initramfs_size 1>/dev/null 2>&1
cat $bootimage_name | head -c $initramfs_tailaddr | tail -c $initramfs_size >/switch/initramfs.lzma
cd /switch
lzma -d initramfs.lzma 1>/dev/null 2>&1
cpio -i -d -u --no-absolute-filenames -F initramfs 1>/dev/null 2>&1

if [ -f /tmp/$imagename ]; then
    rm /tmp/$imagename -rf
fi
if [ -f /switch/initramfs ]; then
    rm /switch/initramfs -rf
fi
