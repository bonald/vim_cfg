#!/bin/sh
#
# do system log daily rotate
#

#
# don't modify under this line
#

tmpfile=""

cleanup()
{
	if [ "$tmpfile" != "" ]; then
		rm -fr $tmpfile
	fi
    return
}

trap 'cleanup' EXIT
trap 'echo "Command interrupted by user"; exit 1;' SIGINT SIGKILL SIGHUP

usage()
{
    echo "subagent.sh path [ stop ]"
    exit 1
}

if [ $# -eq 1 ]; then
    sapath=$1
    action=""
elif [ $# -eq 2 ]; then
    sapath=$1;
    action=$2;
    if [ "$action" != "stop" ]; then
        echo "Invalid action. only accept stop"
        exit 1
    fi
else
    usage;
fi

if ! [ -d $sapath ]; then
    echo "Please specify scripts path..."
    exit 1
fi

cd $sapath

#ps -fe | grep snmp_agent_main.py | grep -v grep
if [ -z "$action" ]; then
    ps -fe | grep snmp_agent_main.py | grep -v grep
    if [ $? -ne 0 ]; then
        snmp_agent_main.py -d
    fi
fi
if [ "$action" == "stop" ]; then
    ps -fe | grep snmp_agent_main.py | grep -v grep
    if ! [ $? -ne 0 ]; then
        ps -fe | grep snmp_agent_main.py | grep -v grep | awk '{print $2}' | xargs kill -9 $2
    fi
fi


exit 0   
