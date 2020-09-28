#!/bin/sh

######################################################################
# Copyright (C) 2004-2017 Centec Networks. All rights reserved.
# OSP fast init.  
######################################################################

#echo "Starting CROND..."
mkdir -p /var/spool/cron/crontabs
mkdir -p /etc/crontabs
#add crontab
# Openflow conf syncup scripts. 
echo "*/30 * * * * $OVS_DIR/bin/ovsdb_sync.sh >/dev/null 2>&1 " >>  /var/spool/cron/crontabs/root
echo "*/30 * * * * sync && echo 3 > /proc/sys/vm/drop_caches" >> /var/spool/cron/crontabs/root
echo "*/10 * * * * cat /dev/null > /var/log/lastlog && cat /dev/null > /var/log/wtmp " >>  /var/spool/cron/crontabs/root
# Modified by zhangdy on 20150411, the new busybox (v1.22.1) 's -c options should be the directory
# which must contains the crond tabs
crond -b -c /var/spool/cron/crontabs -L /dev/null

echo "Starting NTPD..."
# initialize drift file for ntp
echo "0.000" > /etc/ntp/drift >/dev/null 2>&1 
ntpd -U 30 -c /etc/ntp.conf -p /var/run/ntpd.pid -g -l /dev/null &
echo "Ntpd init done" >> $STARTUP_LOG && date >> $STARTUP_LOG

OVS_DIR=$SWITCH_DIR/usr/local/openvswitch
TAP_DIR=$SWITCH_DIR/usr/local/tap
# startup snmp trap agent
export PYTHONHOME='/usr'
export PYTHONPATH='/usr/lib/python2.7:/usr/lib/python2.7/site-packages:/usr/lib/python2.7/site-packages/rpc_server/lib/'
# TAP also use python agent
if [ -d $TAP_DIR ]; then
    /usr/sbin/trap_agent -d >/dev/null 2>&1
fi

# startup web flask
tar xzf /var/web.bin -C /var/ >/dev/null 2>&1
tar xzf /var/rpcapi.bin -C /var/ >/dev/null 2>&1

if [ ! -d /conf/ssh ]; then
    mkdir -p /conf/ssh
fi

STARTUP_LOG=/mnt/flash/cold/log/startup_log
SWITCH_DIR=""
if [ "$IS_DEBIAN" == "y" ]; then
    SWITCH_DIR=/switch
fi

# export this env var to be used in child scripts.
export OVS_DIR
# Append ovs utilities path to PATH.
export PATH=$PATH:$OVS_DIR/bin

DEFAULT_NS=default
MGMT_NS=mgmt

insmod $SWITCH_DIR/lib/dal.ko
insmod $SWITCH_DIR/lib/ctc_hw.ko
insmod $SWITCH_DIR/lib/ctc_gpio.ko
insmod $SWITCH_DIR/lib/ctc_phy.ko
insmod $SWITCH_DIR/lib/ctc_power.ko
insmod $SWITCH_DIR/lib/ctc_reset.ko
insmod $SWITCH_DIR/lib/ctc_watchdog.ko
#insmod $SWITCH_DIR/lib/kernel_monitor.ko

board_type_detect
boardseries=`cat /tmp/ctcos_board_info | grep product_series | awk '{ print $2}'`
boardtype=`cat /tmp/ctcos_board_info | grep board_type | awk '{ print $2}'`

cp -fr /etc/*.conf /centec_switch/etc/
rm -fr /tmp/fea_done

echo "Start PMs..."
ccs &
cds &
ncs &
sleep 2
switch &

if [ -f $SWITCH_DIR/usr/sbin/routed ]; then
    routed &
fi

if [ ! -d $TAP_DIR ]; then
    opm & 
fi

appcfg &
ptp &
bhm &
ssm &
authd &

if [ ! -d $TAP_DIR ]; then
#	bgpd &
	zebra -d
	ospfd -d
#	ripd -d
	dhcrelay -4 -d -q
	dhcsnooping -d
    echo "" > /switch/etc/dhclient.conf
    dhclient &
fi

chsm &
ctctop &

# start openflow
if [ -d $OVS_DIR ]; then
    # Setup backup conf dir.
    OPENFLOW_CONF_DIR="/mnt/flash/.openflow/"
    mkdir -p $OPENFLOW_CONF_DIR

    # Recover the backuped conf tar.gz
    if [ -f $OPENFLOW_CONF_DIR/conf_backup.tar.gz ] ; then
        cp -rf $OPENFLOW_CONF_DIR/conf_backup.tar.gz $OVS_DIR/etc/db/conf_backup.tar.gz
        line=`tar xzvf $OVS_DIR/etc/db/conf_backup.tar.gz -C / 2>&1 | grep conf_backup`
        if [ "$line" != "" ] ; then
            #echo "Success to decompress ovsdb backup file: conf_backup.tar.gz"
            cp -rf $OVS_DIR/etc/db/conf_backup.db $OVS_DIR/etc/db/conf.db
	    echo "Recover user saved ovs conf db successfully."
        fi
    fi
# added by wangl for clear db
	if [ -f $OVS_DIR/etc/db/conf_org.db ] ; then
	    cp -f $OVS_DIR/etc/db/conf_org.db $OVS_DIR/etc/db/conf.db
	fi
	
    echo "Start OpenFlow..."
    $OVS_DIR/bin/ovsdb-server $OVS_DIR/etc/db/conf.db --remote=punix:$OVS_DIR/var/run/db.sock --remote=db:Open_vSwitch,Open_vSwitch,manager_options --pidfile=$OVS_DIR/var/run/ovsdb-server.pid --unixctl=$OVS_DIR/var/run/ovsdb-server.ctl --verbose=ANY:syslog:off --detach > /dev/null 2>&1
    $OVS_DIR/bin/ovs-vswitchd unix:$OVS_DIR/var/run/db.sock --pidfile=$OVS_DIR/var/run/ovs-vswitchd.pid --verbose=ANY:syslog:err > /dev/null 2>&1 &
fi

sleep 2
fea &

echo "Waiting the system initialize..."
# waiting fea done
let wait_count=0
while true; do          
    if [ -f /tmp/fea_done ] ; then
        break;           
    fi
    echo -n "."
    sleep 1;
    let wait_count=wait_count+1
    
    if [ $wait_count -ge 120 ] ; then
        echo "system can't be started in 120 seconds, return to bash..."
        break;
    fi
done

cdbctl update/cdb/sys/sys_global/init_done/1
cdbctl update/cdb/sys/sys_global/hostname/Switch

if grep nopass /proc/cmdline 2>&1 >/dev/null; then
    nopassword /mnt/flash/startup-config.conf
fi

if [ ! -d $TAP_DIR ]; then
    cdbctl update/cdb/sys/manage_if/addr/192.168.0.1/24
fi

echo "Loading startup configuration file"
klish -q -s /mnt/flash/startup-config.conf

# added by kcao for insure startup_done is called
cdbctl update/cdb/sys/sys_global/startup_done/1

cdbctl create/cdb/sys/ns_port_forwarding/0#0#23#0.0.0.0/ip/0.0.0.0
cdbctl create/cdb/sys/ns_port_forwarding/0#0#80#0.0.0.0/ip/0.0.0.0
cdbctl create/cdb/sys/ns_port_forwarding/0#0#443#0.0.0.0/ip/0.0.0.0

# If this is OVS enabled image, try to load startup entries.
if [ -d $OVS_DIR ]; then
    $OVS_DIR/bin/load_startup_entry.sh &
fi

ret="`grep http /mnt/flash/startup-config.conf | grep service | grep -v banner | grep -v password | grep -v description |awk '{print $3}'`"
if [ "$ret" == "disable" ]; then
    echo "Http service disabled..."
else
    python /var/web/run_server.py start 2>&1 >/dev/null
    #ip netns exec mgmt python /var/web/index.py & 
fi

# Load user-defined hook for customer of OpenFlow.
if [ -d $OVS_DIR ]; then    
    $OVS_DIR/bin/load_post_init_hook.sh &
fi

if [ -f /mnt/flash/logo.gif ] ; then
    cp -f /mnt/flash/logo.gif /var/web/static/switch/images/logo_show.gif
else
    grep "Centec Networks" /tmp/oem_info 2>&1 > /dev/null
    if [ "$?" == "0" ]; then
        if [ -f /var/web/static/switch/images/logo.gif ] ; then
            cp -f /var/web/static/switch/images/logo.gif /var/web/static/switch/images/logo_show.gif
        fi
    fi
fi

ctcmon &

echo -n "Ready to service!"
