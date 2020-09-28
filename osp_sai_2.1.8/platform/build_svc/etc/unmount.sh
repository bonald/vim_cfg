#!/bin/sh
sync
#umount /mnt/udisk
mntpoint=/mnt/udisk
tryumount=`umount -l $mntpoint 2>&1`
ret=$?
if [ "`echo $tryumount |grep 'Invalid argument'`" ]; then
    rm -rf $mntpoint
    exit 0
fi

if [ "`echo $tryumount |grep 'not mounted'`" ]; then
    logger -p info "%s is not mounted" "$mntpoint"
    rm -rf $mntpoint
    exit 1
fi

if [ "`echo $tryumount |grep 'not found'`" ]; then
    logger -p info "%s not fount" "$mntpoint"
    rm -rf $mntpoint
    exit 1
fi

# did the umount succeed?
if [ $ret -eq 0 ]; then
    #success
    echo "$mntpoint umounted"
    logger -p info "$mntpoint umounted"
    rm -rf $mntpoint
    exit 0
else
    # if the filesystem is busy, kill all processes accessing it
    if [ "`echo $tryumount |grep 'busy'`" ]; then
        echo "busy... killing all processes accessing the filesystem"
        logger -p crit "busy... killing all processes accessing the filesystem"
        while [ ! -z "`fuser -m $mntpoint`" ]; do
           # for bug 53335, modified by tongzb
		   fuser -m $mntpoint | awk '{print $1}' | xargs kill -9
           sleep 1
        done
    else
        #logger -p crit "%s failed... umount returned %d" "$SCRIPT" "$ret"
        rm -rf $mntpoint
        exit 1
    fi
fi

# try to umount again
tryumount=`umount -l $mntpoint 2>&1`
ret=$?
if [ $ret -eq 0 ]; then
    #success
    echo "$mntpoint umounted"
    logger -p info "$mntpoint umounted"
    rm -rf $mntpoint
    exit 0
else
    logger -p alert "ALARM!!! OPERATE USB mass storage device FAILED!!!"
fi
