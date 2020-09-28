#!/bin/sh
#
# sync system log from /var/log/messages to /mnt/flash/syslog
#

# unit is kbytes
minleftsize=1024
mincompresssize=2048

#
# don't modify under this line
#

cleanup()
{
    rm -fr /var/log/messages_sync
    return
}

if ! [ -f /var/log/messages ]; then
    exit 0
fi

if [ -f /var/log/messages_sync ]; then
    echo "Already running..."
    exit 0
fi

mv -f /var/log/messages /var/log/messages_sync                                             

trap 'cleanup' EXIT
trap 'echo "Command interrupted by user"; exit 1;' SIGINT SIGKILL SIGHUP

if ! [ -f /mnt/flash/syslog ]; then
    touch /mnt/flash/syslog
fi

filedeleted=0
havespace=1
compressed=0
outofdisk=0

echo "Initialize..."
# removed by kcao for bug 51372
#ls -lh /mnt/flash /var/log/messages*

do_rotate()
{
    echo "Do log file rotate" 
        
    # create syslogfile or not exist
    mkdir -p /mnt/flash/syslogfile
        
    # delete the oldest one
	  if [ $(ls /mnt/flash/syslogfile | awk 'END{print NR}') -gt 255 ]; then
				rm -r /mnt/flash/syslogfile/$(ls -rtl /mnt/flash/syslogfile | awk '{ if (NR == 2) { print $9 } }')
		fi

    return;
}

# check free space on flash for merge system log from memory to flash
check_freespace()
{
    totalsize=`du -ck /mnt/flash/syslog /var/log/messages_sync |grep "total$" | grep -v grep | awk '{print $1}'`
    freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
    leftsize=`expr $freesize - $totalsize`
    echo "Free space on flash: $freesize kbytes, file total size $totalsize kbytes"
    if [ $leftsize -lt $minleftsize ]; then
        havespace=0
        echo "Free space too small on flash"
        #ls -lh /mnt/flash /var/log/messages*
        return
    fi
    
    havespace=1
    return
}

# delete the oldest system log file when no enough space 
delete_oldest_logfile()
{
    if [! -d "/mnt/flash/syslogfile" ]; then
	    echo "No older system log file for delete now"
	    return
    fi

    # check total size for syslog.*.gz
    totalsize=`du -ck /mnt/flash/syslog /var/log/messages_sync |grep "total$" | grep -v grep | awk '{print $1}'`
    freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
    oldlogsize=`du -ck /mnt/flash/syslogfile*.gz |grep "total$" | grep -v grep | awk '{print $1}'`
    freesize=`expr $oldlogsize + $freesize`
    leftsize=`expr $freesize - $totalsize`
    if [ $leftsize -lt $minleftsize ]; then
        logger -t LOG -p warn "No space left on flash:/ even if delete all old log files."
		outofdisk=1
        return
    fi
        
    # create syslogfile or not exist
    mkdir -p /mnt/flash/syslogfile
        
    # delete the oldest one
	  if [ $(ls /mnt/flash/syslogfile | awk 'END{print NR}') -gt 0 ]; then
				rm -r /mnt/flash/syslogfile/$(ls -rtl /mnt/flash/syslogfile | awk '{ if (NR == 2) { print $9 } }')
		fi
}


bfirst=1
while true; do
    check_freespace
	
	if [ $outofdisk -eq 1 ]; then
		break;
	fi

    if [ $havespace -eq 0 ]; then
        if [ $bfirst -eq 1 ]; then
            bfirst=0
            echo "Try to compress the newest logs"
            cat /mnt/flash/syslog /var/log/messages_sync | gzip > /var/log/messages_sync.gz
            rm -fr /var/log/messages_sync
            rm -fr /mnt/flash/syslog
            touch /mnt/flash/syslog
            mv -f /var/log/messages_sync.gz /var/log/messages_sync
            compressed=1
            #ls -lh /mnt/flash /var/log/messages*
            continue;
        fi

        delete_oldest_logfile
        continue
    fi

    break;
done


if [ $havespace -eq 0 ]; then
    echo "No space left on /mnt/flash. drop all logs"
    #ls -lh /mnt/flash /var/log/messages*
    freesize=`df -k /mnt/flash |grep flash | grep -v grep| awk '{ print $4}'`
    logger -t LOG -p warn "No space left on flash:/ (< $freesize kbytes). Drop updated log messages."
    rm -fr /var/log/messages_sync
    exit 1
fi

if [ $compressed -eq 1 ]; then
    do_rotate
    filename=syslogfile-`date +%Y-%m-%d-%H-%M-%S`.log.gz
    mv -f /var/log/messages_sync /mnt/flash/syslogfile/$filename
    touch /mnt/flash/syslog
else
    totalsize=`du -ck /mnt/flash/syslog /var/log/messages_sync |grep "total$" | grep -v grep | awk '{print $1}'`
    if [ $totalsize -gt $mincompresssize ]; then
        echo "Compress system log file for size too large"
        #ls -lh /mnt/flash /var/log/messages*
        cat /mnt/flash/syslog /var/log/messages_sync | gzip > /var/log/messages_sync.gz
        rm -fr /var/log/messages_sync
        rm -fr /mnt/flash/syslog
        touch /mnt/flash/syslog
        mv -f /var/log/messages_sync.gz /var/log/messages_sync
        compressed=1
        echo "Compress done..."
        #ls -lh /var/log/message* /mnt/flash
        do_rotate
        filename=syslogfile-`date +%Y-%m-%d-%H-%M-%S`.log.gz
        mv -f /var/log/messages_sync /mnt/flash/syslogfile/$filename
        if [ $? -eq 1 ]; then
            logger -t LOG -p warn "Sync up log messages to flash:/ faild for no disk space."
            touch /mnt/flash
            exit 1
        fi
        touch /mnt/flash/syslog
    else
        cat /var/log/messages_sync >> /mnt/flash/syslog
        if [ $? -eq 1 ]; then
            logger -t LOG -p warn "Sync up log messages to flash:/ faild for no disk space."
            rm -fr /var/log/messages_sync
            exit 1
        fi
        rm -fr /var/log/messages_sync
    fi
fi

echo "Sync up success..."
#ls -lh /mnt/flash /var/log/messages*
exit 0
