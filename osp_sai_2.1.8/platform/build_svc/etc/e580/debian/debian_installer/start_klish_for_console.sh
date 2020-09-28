#! /bin/bash

SWITCH_DIR=/switch

#export PYTHONHOME=$SWITCH_DIR/usr
#export PYTHONPATH=$SWITCH_DIR/usr/lib/python2.7:$SWITCH_DIR/usr/lib/python2.7/site-packages:$SWITCH_DIR/usr/lib/python2.7/lib-dynload/:$SWITCH_DIR/usr/lib/python2.7/site-packages/rpc_server/lib/

OVS_DIR=$SWITCH_DIR/usr/local/openvswitch
TAP_DIR=$SWITCH_DIR/usr/local/tap
export OVS_DIR

export PATH=$SWITCH_DIR/sbin:$SWITCH_DIR/usr/sbin:$SWITCH_DIR/bin:$SWITCH_DIR/usr/bin:$OVS_DIR/bin:$PATH
export LD_LIBRARY_PATH=/lib/powerpc-linux-gnu:/usr/lib/powerpc-linux-gnu:$SWITCH_DIR/lib:$SWITCH_DIR/usr/lib

# translate carriage return to newline
stty icrnl

if [ -f /switch/usr/sbin/klish ]; then
    /switch/usr/sbin/klish
else
    su - switch
fi
