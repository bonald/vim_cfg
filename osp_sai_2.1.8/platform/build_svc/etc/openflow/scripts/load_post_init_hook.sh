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

# If the user-defined hook present, load it in background.
if [ -f /mnt/flash/post_init_hook.sh ]; then
    /mnt/flash/post_init_hook.sh &
fi
