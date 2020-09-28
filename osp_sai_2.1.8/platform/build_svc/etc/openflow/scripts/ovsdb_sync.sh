#!/bin/sh
#
# sync up ovsdb from $_OVS_DIR/etc/db/conf.db to ${OPENFLOW_CONF_DIR}/conf_backup.tar.gz
#

OPENFLOW_CONF_DIR="/mnt/flash/.openflow/"

# OVS_DIR is env var of parent script (rc.sysinit.)
_OVS_DIR=$OVS_DIR
if [ ! -d $_OVS_DIR ]; then
    echo "Not have OVS_DIR env var set, reject to execute."
fi

#sync up conf.db
rm -rf /tmp/ofp_sync_db_done
if [ ! -f $_OVS_DIR/etc/db/conf_temp.db ] ; then
    touch $_OVS_DIR/etc/db/conf_temp.db
fi

conf_db_md5=`md5sum $_OVS_DIR/etc/db/conf.db`
conf_temp_db_md5=`md5sum $_OVS_DIR/etc/db/conf_temp.db`

conf_db_md5=`echo $conf_db_md5 | awk '{ print substr($conf_db_md5,1,40)}'`
conf_temp_db_md5=`echo $conf_temp_db_md5 | awk '{ print substr($conf_temp_db_md5,1,40)}'`

if [ "$conf_db_md5" != "$conf_temp_db_md5" ] ; then
    cp $_OVS_DIR/etc/db/conf.db $_OVS_DIR/etc/db/conf_temp.db
    rm -rf $_OVS_DIR/etc/db/conf_backup.db
    $_OVS_DIR/bin/ovsdb-tool compact $_OVS_DIR/etc/db/conf.db $_OVS_DIR/etc/db/conf_backup.db
    tar czf $_OVS_DIR/etc/db/conf_backup.tar.gz $_OVS_DIR/etc/db/conf_backup.db
    cp $_OVS_DIR/etc/db/conf_backup.tar.gz ${OPENFLOW_CONF_DIR}/conf_backup.tar.gz 
fi
touch /tmp/ofp_sync_db_done

exit 0
