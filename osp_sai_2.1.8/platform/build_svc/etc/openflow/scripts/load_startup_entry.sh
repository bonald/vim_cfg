#!/bin/sh

# OVS_DIR is env var of parent script (rc.sysinit.)
_OVS_DIR=$OVS_DIR
if [ ! -d $_OVS_DIR ]; then
    echo "Not have OVS_DIR env var set, reject to execute."
fi

# Block until ovs load the initial ovsdb done. 
while true; do
    if [ -f /tmp/ovs_load_initial_ovsdb_done ] ; then
        break;
    fi
    sleep 1;
done

STARTUP_FLOW_ENTRY=/mnt/flash/init_flow
STARTUP_METER_ENTRY=/mnt/flash/init_meter
STARTUP_GROUP_ENTRY=/mnt/flash/init_group

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

#load meters/groups/flows from files

# Note: OVS 2.3.1 does not support add-meters in ovs-ofctl yet...
if [ -f $STARTUP_METER_ENTRY ] ; then
    $_OVS_DIR/bin/ovs-ofctl add-meters br0 $STARTUP_METER_ENTRY $protocol > /dev/null
fi

if [ -f $STARTUP_GROUP_ENTRY ] ; then
    $_OVS_DIR/bin/ovs-ofctl add-groups br0 $STARTUP_GROUP_ENTRY $protocol > /dev/null
fi

if [ -f $STARTUP_FLOW_ENTRY ] ; then
    $_OVS_DIR/bin/ovs-ofctl add-flows br0 $STARTUP_FLOW_ENTRY $protocol > /dev/null
fi
