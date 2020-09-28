#!/bin/sh

# OVS_DIR is env var of parent script (rc.sysinit.)
_OVS_DIR=$OVS_DIR
if [ ! -d $_OVS_DIR ]; then
    echo "Not have OVS_DIR env var set, reject to execute."
fi

STARTUP_METER_ENTRY=/mnt/flash/init_meter

OPENFLOW_PROTOCOLS=`$_OVS_DIR/bin/ovs-vsctl list bridge br0 |grep protocols`
use_openflow14=`echo "$OPENFLOW_PROTOCOLS" |grep "OpenFlow14"`
use_openflow13=`echo "$OPENFLOW_PROTOCOLS" |grep "OpenFlow13"`
use_openflow12=`echo "$OPENFLOW_PROTOCOLS" |grep "OpenFlow12"`
use_openflow10=`echo "$OPENFLOW_PROTOCOLS" |grep "OpenFlow10"`

#choose the highest protocol
if [ -n "$use_openflow14" ] ; then
    protocol="-O openflow14"
elif [ -n "$use_openflow13" ] ; then
    protocol="-O openflow13"
elif [ -n "$use_openflow12" ] ; then
    protocol="-O openflow12"
elif [ -n "$use_openflow10" ] ; then
    protocol=""
else
    protocol="-O openflow13"
fi

if [ -f $STARTUP_METER_ENTRY ]; then
    echo "Openflow startup meter file exists, will override it!"
fi

$_OVS_DIR/bin/ovs-ofctl dump-meters br0 $protocol > $STARTUP_METER_ENTRY

# Remove the heading OpenFlow message exchange line.
# Remove the possible OFPMP_MORE present int the output.
sed -i -e "/xid/d" $STARTUP_METER_ENTRY
sync
echo "Write openflow startup meter file successfully!"
