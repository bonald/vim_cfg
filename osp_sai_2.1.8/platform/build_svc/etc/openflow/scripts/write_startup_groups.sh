#!/bin/sh

# OVS_DIR is env var of parent script (rc.sysinit.)
_OVS_DIR=$OVS_DIR
if [ ! -d $_OVS_DIR ]; then
    echo "Not have OVS_DIR env var set, reject to execute."
fi

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

if [ -f $STARTUP_GROUP_ENTRY ]; then
    echo "Openflow startup group file exists, will override it!"
fi

$_OVS_DIR/bin/ovs-ofctl dump-groups br0 $protocol > /tmp/temp_groups

# Remove the heading OpenFlow message exchange line.
# Remove the possible OFPMP_MORE present int the output.
sed -i -e "/xid/d" /tmp/temp_groups
# Group should be reorder to select->all/ff->indirect order to
# make them being successfully installed when loading them at
# startup.
sed -n -e "/lsp_aps/p" /tmp/temp_groups > /tmp/temp_groups_lsp_aps
sed -i -e "/lsp_aps/d" /tmp/temp_groups
sed -n -e "/pw_aps/p" /tmp/temp_groups > /tmp/temp_groups_pw_aps
sed -i -e "/pw_aps/d" /tmp/temp_groups
sed -n -e "/select/p" /tmp/temp_groups > /tmp/temp_groups_select
sed -n -e "/[all\|ff].*group:/p" /tmp/temp_groups > /tmp/temp_groups_all
sed -i -e "/[all\|ff].*group:/d" /tmp/temp_groups
sed -n -e '/\(ff\|all\)/p' /tmp/temp_groups > /tmp/temp_groups_all_ff_nested
sed -n -e "/indirect/p" /tmp/temp_groups > /tmp/temp_groups_indirect

# Merge the reordered result.
cat /tmp/temp_groups_select > $STARTUP_GROUP_ENTRY
cat /tmp/temp_groups_all_ff_nested >> $STARTUP_GROUP_ENTRY
cat /tmp/temp_groups_all >> $STARTUP_GROUP_ENTRY
cat /tmp/temp_groups_indirect >> $STARTUP_GROUP_ENTRY
cat /tmp/temp_groups_lsp_aps >> $STARTUP_GROUP_ENTRY
cat /tmp/temp_groups_pw_aps >> $STARTUP_GROUP_ENTRY

rm /tmp/temp_groups
rm /tmp/temp_groups_select
rm /tmp/temp_groups_all_ff_nested
rm /tmp/temp_groups_all
rm /tmp/temp_groups_indirect
rm /tmp/temp_groups_lsp_aps
rm /tmp/temp_groups_pw_aps

echo "Write openflow startup group file successfully!"
