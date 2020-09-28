#!/bin/sh
#
# do system log daily rotate
#

#
# don't modify under this line
#

cleanup()
{
    rm -fr /mnt/flash/syslog
    touch /mnt/flash/syslog
    return
}

trap 'cleanup' EXIT
trap 'echo "Command interrupted by user"; exit 1;' SIGINT SIGKILL SIGHUP

while true; do
    # check logsync
    line=`ps -ax 2>/dev/null|grep logsync.sh | grep -v grep`
    if [ "$line" != "" ]; then
        echo "Waiting logsync.sh..."
        echo "$line"
        sleep 1;
        continue;
    fi

    break;
done

if [ -f /var/log/messages ]; then
    filesize=`du -ck /var/log/messages | grep "total$" | grep -v grep | awk '{print $1}'`
    if [ $filesize -gt 0 ]; then
        # running logsync to sync old messages
        logsync.sh yes
    fi 
fi

filesize=`du -ck /mnt/flash/syslog | grep "total$" | grep -v grep | awk '{print $1}'`
if [ $filesize -eq 0  ]; then
    exit 0
fi

do_rotate()
{
    echo "Do log file rotate" 
        
    # delete the oldest one
    if [ -f "/mnt/flash/syslog.14.gz" ]; then
        echo "Delete syslog.14.gz"
        logger -t LOG -p warn "Daliy log file rotate and delete flash:/syslog.14.gz"
        rm -fr /mnt/flash/syslog.14.gz
    fi

    # do roate
    i=13
    while [ $i -gt 0 ]; do
        if [ -f "/mnt/flash/syslog.$i.gz" ]; then
            echo "Move syslog.$i.gz syslog.`expr $i + 1`.gz"
            mv /mnt/flash/syslog.$i.gz /mnt/flash/syslog.`expr $i + 1`.gz
        fi
        i=`expr $i - 1`
    done

    return;
}

do_rotate
if [ -f /mnt/flash/syslog ]; then
    mv /mnt/flash/syslog /mnt/flash/syslog.1
    gzip /mnt/flash/syslog.1
    echo "Gzip syslog to syslog.1.gz"
fi

logger -t LOG -p warn "Daily log file rotate success..."
echo "Done"
ls -lh /mnt/flash
exit 0
