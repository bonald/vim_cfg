#!/bin/sh
#
# sync system log from /var/log/diag_messages to /mnt/flash/info/diaglog
#

# unit is kbytes
minleftsize=1024
mincompresssize=2048

#
# don't modify under this line
#

cleanup()
{
    rm -fr /var/log/diag_messages_sync
    return
}

checkdailyrotate=1
if [ $# == 1 ]; then
    checkdailyrotate=0
fi

if ! [ -f /var/log/diag_messages ]; then
    exit 0
fi

if [ -f /var/log/diag_messages_sync ]; then
    echo "Already running..."
    exit 0
fi

mv -f /var/log/diag_messages /var/log/diag_messages_sync

while [ $checkdailyrotate -eq 1 ] ; do
    # check diagdailyrotate.sh
    line=`ps -ax 2>/dev/null |grep diagdailyrotate.sh | grep -v grep`
    if [ "$line" != "" ]; then
        echo "Waiting diagdailyrotate.sh..."
        echo "$line"
        sleep 1;
        continue;
    fi
    
    break;
done                                                

trap 'cleanup' EXIT
trap 'echo "Command interrupted by user"; exit 1;' SIGINT SIGKILL SIGHUP

if ! [ -f /mnt/flash/info/diaglog ]; then
    touch /mnt/flash/info/diaglog
fi

filedeleted=0
havespace=1
compressed=0

echo "Initialize..."
ls -lh /mnt/flash /var/log/diag_messages*
echo "=============================="

do_rotate()
{
    echo "Do log file rotate" 
        
    if [ -f "/mnt/flash/info/diaglog.14.gz" ]; then
        echo "Delete /mnt/flash/info/diaglog.14.gz"
        rm -fr /mnt/flash/info/diaglog.14.gz
    fi

    # do roate
    i=13
    while [ $i -gt 0 ]; do
        if [ -f "/mnt/flash/info/diaglog.$i.gz" ]; then
            echo "mv /mnt/flash/info/diaglog.$i.gz /mnt/flash/info/diaglog.`expr $i + 1`.gz"
            mv /mnt/flash/info/diaglog.$i.gz /mnt/flash/info/diaglog.`expr $i + 1`.gz
        fi

        i=`expr $i - 1`
    done

    return;
}

# check free space on flash for merge system log from memory to flash
check_freespace()
{
    totalsize=`du -ck /mnt/flash/info/diaglog /var/log/diag_messages_sync |grep "total$" | grep -v grep | awk '{print $1}'`
    freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
    leftsize=`expr $freesize - $totalsize`
    echo "Free space on flash: $freesize kbytes, file total size $totalsize kbytes"
    if [ $leftsize -lt $minleftsize ]; then
        havespace=0
        echo "Free space too small on flash"
        ls -lh /mnt/flash /var/log/diag_messages*
        return
    fi
    
    havespace=1
    return
}

# delete the oldest system log file when no enough space 
delete_oldest_logfile()
{
    filedeleted=0

    # check total size for diaglog.*.gz
    totalsize=`du -ck /mnt/flash/info/diaglog /var/log/diag_messages_sync |grep "total$" | grep -v grep | awk '{print $1}'`
    freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
    oldlogsize=`du -ck /mnt/flash/info/diaglog.*.gz |grep "total$" | grep -v grep | awk '{print $1}'`
    freesize=`expr $oldlogsize + $freesize`
    leftsize=`expr $freesize - $totalsize`
    if [ $leftsize -lt $minleftsize ]; then
        logger -t LOG -p warn "No space left on flash:/ even if delete all old log files."
        return
    fi
        
    i=14
    while [ $i -gt 0 ]; do
        logf="/mnt/flash/info/diaglog.$i.gz"
        if [ -f $logf ]; then
            freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
            logger -t LOG -p warn "Delete oldest system log flash:/diaglog.$i.gz for no space (< $freesize kbytes)."
            filedeleted=1
            rm -fr $logf
            echo "Delete oldest system log file: $logfile"
            ls -lh /mnt/flash /var/log/diag_messages*
            return
        fi

        i=`expr $i - 1`
    done
    
    echo "No older system log file for delete now"
    return
}

bfirst=1
while true; do
    check_freespace

    if [ $havespace -eq 0 ]; then
        if [ $bfirst -eq 1 ]; then
            bfirst=0
            echo "Try to compress the newest logs"
            cat /mnt/flash/info/diaglog /var/log/diag_messages_sync | gzip > /var/log/diag_messages_sync.gz
            rm -fr /var/log/diag_messages_sync
            rm -fr /mnt/flash/info/diaglog
            touch /mnt/flash/info/diaglog
            mv -f /var/log/diag_messages_sync.gz /var/log/diag_messages_sync
            compressed=1
            ls -lh /mnt/flash /var/log/diag_messages*
            continue;
        fi

        delete_oldest_logfile
        if [ $filedeleted -eq 0 ]; then
            echo "No space for system log on /mnt/flash"
            break;
        fi
        continue
    fi

    break;
done

if [ $havespace -eq 0 ]; then
    echo "No space left on /mnt/flash. drop all logs"
    ls -lh /mnt/flash /var/log/diag_messages*
    freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
    logger -t LOG -p warn "No space left on flash:/ (< $freesize kbytes). Drop updated log diag_messages."
    rm -fr /var/log/diag_messages_sync
    exit 1
fi

if [ $compressed -eq 1 ]; then
    do_rotate
    mv -f /var/log/diag_messages_sync /mnt/flash/info/diaglog.1.gz
    touch /mnt/flash/info/diaglog
else
    totalsize=`du -ck /mnt/flash/info/diaglog /var/log/diag_messages_sync |grep "total$" | grep -v grep | awk '{print $1}'`
    if [ $totalsize -gt $mincompresssize ]; then
        echo "Compress system log file for size too large"
        ls -lh /mnt/flash /var/log/diag_messages*
        cat /mnt/flash/info/diaglog /var/log/diag_messages_sync | gzip > /var/log/diag_messages_sync.gz
        rm -fr /var/log/diag_messages_sync
        rm -fr /mnt/flash/info/diaglog
        touch /mnt/flash/info/diaglog
        mv -f /var/log/diag_messages_sync.gz /var/log/diag_messages_sync
        compressed=1
        echo "Compress done..."
        ls -lh /var/log/message* /mnt/flash
        do_rotate
        mv -f /var/log/diag_messages_sync /mnt/flash/info/diaglog.1.gz
        if [ $? -eq 1 ]; then
            logger -t LOG -p warn "Sync up log diag_messages to flash:/ faild for no disk space."
            touch /mnt/flash
            exit 1
        fi
        touch /mnt/flash/info/diaglog
    else
        cat /var/log/diag_messages_sync >> /mnt/flash/info/diaglog
        if [ $? -eq 1 ]; then
            logger -t LOG -p warn "Sync up log diag_messages to flash:/ faild for no disk space."
            rm -fr /var/log/diag_messages_sync
            exit 1
        fi
        rm -fr /var/log/diag_messages_sync
    fi
fi

echo "Sync up success..."
ls -lh /mnt/flash /var/log/diag_messages*
exit 0
