#!/bin/sh
#
# do system log daily rotate
#

#
# don't modify under this line
#

cleanup()
{
    rm -fr /mnt/flash/info/diaglog
    touch /mnt/flash/info/diaglog
    return
}

trap 'cleanup' EXIT
trap 'echo "Command interrupted by user"; exit 1;' SIGINT SIGKILL SIGHUP

while true; do
    # check logsync
    line=`ps -ax 2>/dev/null|grep diaglogsync.sh | grep -v grep`
    if [ "$line" != "" ]; then
        echo "Waiting diaglogsync.sh..."
        echo "$line"
        sleep 1;
        continue;
    fi

    break;
done

if [ -f /var/log/diag_messages ]; then
    filesize=`du -ck /var/log/diag_messages | grep "total$" | grep -v grep | awk '{print $1}'`
    if [ $filesize -gt 0 ]; then
        # running logsync to sync old diag_messages
        diaglogsync.sh yes
    fi 
fi

filesize=`du -ck /mnt/flash/info/diaglog | grep "total$" | grep -v grep | awk '{print $1}'`
if [ $filesize -eq 0  ]; then
    exit 0
fi

do_rotate()
{
    echo "Do log file rotate" 

    # delete the oldest one
	  if [ $(ls /mnt/flash/info | awk 'END{print NR}') -gt 256 ]; then
				rm -r /mnt/flash/info/$(ls -rtl /mnt/flash/info/*diaglog* | awk '{ if (NR == 2) { print $9 } }')
		fi

    return;
}

do_rotate
if [ -f /mnt/flash/info/diaglog ]; then
		filename = diaglog-`date +%Y-%m-%d-%H-%M-%S`.log
    mv /mnt/flash/info/diaglog /mnt/flash/info/$filename
    gzip /mnt/flash/info/$filename
    echo "Gzip diaglog to diaglog.gz"
fi

logger -t LOG -p warn "Daily log file rotate success..."
echo "Done"
ls -lh /mnt/flash
exit 0
