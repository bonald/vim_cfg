#! /bin/sh

if [ ! -f /$1 ];then
        cp $1 /
fi
cd /
dd if=$1 of=mainramfs.lzma ibs=1024 obs=1024 skip=8192
if [ -f mainramfs ];then
        rm -rf mainramfs        
fi                                    
unlzma mainramfs.lzma  

mount -t ext4 -o remount -rw /dev/boot /mnt/flash/boot/ 2>/dev/null

if [ -f /mnt/flash/boot/mainramfs ];then
        rm -rf /mnt/flash/boot/mainramfs        
fi                  
mv mainramfs /mnt/flash/boot/mainramfs             

mount -t ext4 -o remount -r /dev/boot /mnt/flash/boot/ 2>/dev/null
                                            
dd if=$1 of=minios.bin ibs=1024 obs=1024 count=8192
flashcp minios.bin /dev/mtd1

if [ -f mainramfs ];then
        rm -rf mainramfs        
fi  

if [ -f minios.bin ];then
        rm -rf minios.bin        
fi  

sync