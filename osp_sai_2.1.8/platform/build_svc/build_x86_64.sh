#!/bin/sh
set -o errexit

top_dir=$TOP_DIR
board=$BOARD
rootfs_dir='/opt/x86_64_rootfs'
etc_dir=$PLAT_TOP_DIR/build_svc/etc
kbuild_dir=$KDIR
version=2.17.so
bin_dir=bin
mkdir -m 777 -p /dev/shm/software
mkdir -p /dev/shm/software/$USER
tmp_file=$(mktemp -p /dev/shm/software/$USER/)
tmp_dir=$(mktemp -p /dev/shm/software/$USER/ -d)
pre_compiled_dir=${TOP_DIR}/third_party/precompiled
ovs_dir=$TOP_DIR/proto/openflow/openvswitch-2.3.0
ctc_build_dir=$BLD_DIR
enable_openflow=${ENABLE_OPENFLOW}

echo "
dir /lib 755 0 0
dir /etc 755 0 0
dir /etc/cmd 755 0 0
dir /usr 755 0 0
dir /usr/bin 755 0 0
dir /usr/sbin 755 0 0
dir /bin 755 0 0
dir /lib/security 755 0 0
dir /var 755 0 0

file    /etc/init-ns.sh     ${etc_dir}/init-ns.sh               755 0 0
file    /etc/init-veth.sh     ${etc_dir}/init-veth.sh               755 0 0
file    /etc/sysinit.sh     ${etc_dir}/${board}/rc.sysinit               755 0 0
file    /usr/sbin/restart.sh ${etc_dir}/restart.sh    755 0 0
file    /usr/bin/core_helper     ${etc_dir}/core_helper     755 0 0

file    /lib/libSUSI-4.00.so   ${top_dir}/drvs/bus/cpu/i2c/src/susi/Driver/libSUSI-4.00.so       755 0 0
slink   /lib/libSUSI-4.00.so.1   /switch/lib/libSUSI-4.00.so    755 0 0
slink   /lib/libSUSI-4.00.so.1.0.0  /switch/lib/libSUSI-4.00.so     755 0 0

" >> ${tmp_file}

##############

# syslog-ng
cp ${ctc_build_dir}/bin/syslog-ng ${tmp_dir}
cp ${ctc_build_dir}/bin/init-syslog-ng.sh ${tmp_dir}
cp ${ctc_build_dir}/etc/syslog-ng.conf ${tmp_dir}
echo "file /usr/sbin/syslog-ng ${tmp_dir}/syslog-ng 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/init-syslog-ng.sh ${tmp_dir}/init-syslog-ng.sh 755 0 0" >> ${tmp_file}
echo "file /etc/syslog-ng.conf ${tmp_dir}/syslog-ng.conf 755 0 0" >> ${tmp_file}

echo "dir /etc/datapath_profile 755 0 0" >> ${tmp_file}
profile_list=$(ls $etc_dir/datapath_profile)
for f in $profile_list; do
    echo "file /etc/datapath_profile/$f ${etc_dir}/datapath_profile/$f 755 0 0" >> ${tmp_file}
done

echo "dir /etc/spec 755 0 0" >> ${tmp_file}
profile_list=$(ls $etc_dir/spec)
for f in $profile_list; do
    echo "file /etc/spec/$f ${etc_dir}/spec/$f 755 0 0" >> ${tmp_file}
done

# Modified native commands
cp ${ctc_build_dir}/bin/ctctop ${tmp_dir}
cp ${ctc_build_dir}/bin/ctctftp ${tmp_dir}
cp ${ctc_build_dir}/bin/ctcmore ${tmp_dir}
cp ${ctc_build_dir}/bin/ctcftp  ${tmp_dir}
cp ${ctc_build_dir}/bin/chvrf   ${tmp_dir}
cp ${ctc_build_dir}/bin/ctcwatch ${tmp_dir}
echo "file /usr/bin/ctctop ${tmp_dir}/ctctop 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ctctftp ${tmp_dir}/ctctftp 755 0 0" >> ${tmp_file}
echo "file /bin/ctcmore ${tmp_dir}/ctcmore 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ctcftp ${tmp_dir}/ctcftp 755 0 0" >> ${tmp_file}
echo "file /bin/chvrf ${tmp_dir}/chvrf 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ctcwatch ${tmp_dir}/ctcwatch 755 0 0" >> ${tmp_file}

# package web binary image
cp ${top_dir}/apps/web/ ${tmp_dir}/web/ -rf
if [ "$product" == "tap" ]; then
    cp ${top_dir}/apps/web/static/switch/templates/nav_tap.html ${tmp_dir}/web/static/switch/templates/nav.html
    if [ "$board" == "e580" ]; then
        sed -i "s/CNOS/Tap-580/g" ${tmp_dir}/web/static/switch/js/config.js
    else
        sed -i "s/CNOS/Tap-350/g" ${tmp_dir}/web/static/switch/js/config.js
    fi
elif [ "$product" == "of" ]; then
    cp ${top_dir}/apps/web/static/switch/templates/nav_hybrid.html ${tmp_dir}/web/static/switch/templates/nav.html
    if [ "$board" == "e580" ]; then
        sed -i "s/CNOS/CNOS/g" ${tmp_dir}/web/static/switch/js/config.js
    else
        sed -i "s/CNOS/CNOS-350/g" ${tmp_dir}/web/static/switch/js/config.js
    fi
else
    if [ "$board" == "e580" ]; then
        sed -i "s/CNOS/CNOS/g" ${tmp_dir}/web/static/switch/js/config.js
    else
        sed -i "s/CNOS/CNOS-350/g" ${tmp_dir}/web/static/switch/js/config.js
    fi
fi

ROOT_DIR=`pwd`
SCRIPT_DIR=$ROOT_DIR/../../system/oem

if [ -d $SCRIPT_DIR ] ; then
        $SCRIPT_DIR/vender-script.sh web;
fi;

org_pwd=$(pwd)
cd ${tmp_dir}
${top_dir}/apps/web/pkg_web.sh web ${tmp_dir}/web.bin
echo "file /var/web.bin ${tmp_dir}/web.bin  755 0 0" >> ${tmp_file}

rm ${tmp_dir}/web -rf
cd ${org_pwd}


# package rpcapi binary image
cp ${top_dir}/apps/rpcapi/ ${tmp_dir}/rpcapi/ -rf

org_pwd=$(pwd)
cd ${tmp_dir}
${top_dir}/apps/rpcapi/pkg_rpcapi.sh rpcapi ${tmp_dir}/rpcapi.bin
echo "file /var/rpcapi.bin ${tmp_dir}/rpcapi.bin  755 0 0" >> ${tmp_file}

rm ${tmp_dir}/rpcapi -rf
cd ${org_pwd}


cp ${ctc_build_dir}/bin/ntpd  ${tmp_dir}
cp ${ctc_build_dir}/bin/ntpdc ${tmp_dir}
cp ${ctc_build_dir}/bin/ntpq  ${tmp_dir}
echo "file /usr/sbin/ntpd ${tmp_dir}/ntpd 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpdc ${tmp_dir}/ntpdc 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpq ${tmp_dir}/ntpq 755 0 0" >> ${tmp_file}

osp_lib_list="libcdbclient.so libctclib.so libgenlog.so libsal.so libproto.so libcdb.so libsdb.so libprotopbc.so libfei.so libsock.so libtask.so libcjson.so libdbclient.so"
for osp_lib in ${osp_lib_list}; do
    cp ${ctc_build_dir}/lib/${osp_lib} ${tmp_dir}/
    echo "file /lib/${osp_lib} ${tmp_dir}/${osp_lib} 755 0 0" >> ${tmp_file}
done

cp ${ctc_build_dir}/lib/pam_ctc.so ${tmp_dir}/
echo "file /lib/security/pam_ctc.so ${tmp_dir}/pam_ctc.so 755 0 0" >> ${tmp_file}

cp ${ctc_build_dir}/lib/libnss_cdb.so.2 ${tmp_dir}/
echo "file /lib/libnss_cdb.so.2 ${tmp_dir}/libnss_cdb.so.2 755 0 0" >> ${tmp_file}
echo "slink /lib/libnss_cdb.so /switch/lib/libnss_cdb.so.2 755 0 0" >> ${tmp_file}

cmd_xml_list=$(ls ${ctc_build_dir}/etc/cmd)
for f in $cmd_xml_list; do
    echo "file /etc/cmd/$f ${ctc_build_dir}/etc/cmd/$f 755 0 0" >> ${tmp_file}
done

pre_compiled_bin_files="lrz lsz"
for bin_file in ${pre_compiled_bin_files}; do
    cp ${pre_compiled_dir}/x86_64/bin/${bin_file} ${tmp_dir} 
    echo "file /usr/sbin/${bin_file} ${tmp_dir}/${bin_file} 755 0 0" >> ${tmp_file}
done

if [ "$product" != "tap" ]; then
quagga_conf_list="zebra.conf ospfd.conf pimd.conf "
for conf in ${quagga_conf_list}; do
    cp ${ctc_build_dir}/etc/${conf} ${tmp_dir}
    echo "file /etc/${conf} ${tmp_dir}/${conf} 755 0 0" >> ${tmp_file}
done
fi

#copy redis.conf
#echo "file /etc/redis.conf ${ctc_build_dir}/etc/redis.conf 644 0 0" >> ${tmp_file}
# LKMs
lkm_list="dal.ko"
for lkm in ${lkm_list}; do
    cp ${ctc_build_dir}/bin/${lkm} ${tmp_dir}
    echo "file /lib/${lkm} ${tmp_dir}/${lkm} 644 0 0" >> ${tmp_file}
done

# board_type_detect
cp ${ctc_build_dir}/bin/board_type_detect ${tmp_dir}
echo "file /usr/sbin/board_type_detect ${tmp_dir}/board_type_detect 755 0 0" >> ${tmp_file}

# boot console no password
cp ${ctc_build_dir}/bin/nopassword ${tmp_dir}
echo "file /usr/sbin/nopassword ${tmp_dir}/nopassword 755 0 0" >> ${tmp_file}

# reboot_record
cp ${ctc_build_dir}/bin/reboot_record ${tmp_dir}
echo "file /usr/sbin/reboot_record ${tmp_dir}/reboot_record 755 0 0" >> ${tmp_file}

# reg_ops
cp ${ctc_build_dir}/bin/reg_ops ${tmp_dir}
echo "file /usr/sbin/reg_ops ${tmp_dir}/reg_ops 755 0 0" >> ${tmp_file}

# PMs
cli_list="klish cdbctl viewctl fnconvert "
quagga_list="vtysh zebra ospfd pimd "
if [ "$product" == "tap" ]; then
pm_list="ccs cds ncs chsm switch fea appcfg bhm ctcmon snmpagent chvrf ctc_shell lcsh reactorctl getrip"
prog_list=$cli_list$pm_list
else
pm_list="ccs cds ncs chsm switch opm routed fea appcfg ptp bhm ctcmon snmpagent ssm chvrf ctc_shell lcsh reactorctl dhcrelay dhcsnooping dhclient getrip"
prog_list=$cli_list$quagga_list$pm_list
fi

prog_list=$prog_list" snmpget snmpset snmpwalk"
prog_list=$prog_list" angel authd"

for prog in ${prog_list}; do
    cp ${ctc_build_dir}/bin/${prog} ${tmp_dir}
    echo "file /usr/sbin/${prog} ${tmp_dir}/${prog} 755 0 0" >> ${tmp_file}
done

# SNMP
cp ${ctc_build_dir}/lib/libnetsnmpagent.so ${tmp_dir}/
echo "file /lib/libnetsnmpagent.so ${tmp_dir}/libnetsnmpagent.so 755 0 0" >> ${tmp_file}
cp ${ctc_build_dir}/bin/snmpd ${tmp_dir}
echo "file /usr/sbin/snmpd ${tmp_dir}/snmpd 755 0 0" >> ${tmp_file}
echo "file /etc/snmpd.conf ${ctc_build_dir}/etc/snmpd.conf 644 0 0" >> ${tmp_file}

# Profiles
profile_list="mem_profile_default.cfg mem_profile_hybrid_default.cfg mem_profile_tap_default.cfg"
for profile in ${profile_list}; do
    echo "file /etc/${profile} ${ctc_build_dir}/etc/${profile} 644 0 0" >> ${tmp_file}
done

# reboot fast
cp ${ctc_build_dir}/bin/reboot_fast.sh ${tmp_dir}
echo "file /usr/sbin/reboot_fast.sh ${tmp_dir}/reboot_fast.sh 755 0 0" >> ${tmp_file}

# logrotate
echo "file /usr/sbin/logsync.sh ${ctc_build_dir}/bin/logsync.sh 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/diaglogsync.sh ${ctc_build_dir}/bin/diaglogsync.sh 755 0 0" >> ${tmp_file}

##############
# package mib subagent *py
#
cp ${ctc_build_dir}/mibd/*.py ${tmp_dir}/
cp ${ctc_build_dir}/mibd/subagent.sh ${tmp_dir}/
cp ${ctc_build_dir}/mibd/trap_agent ${tmp_dir}/
cp ${ctc_build_dir}/mibd/mib_modules/common/*.py ${tmp_dir}/
if [ "$product" == "tap" ]; then
    cp ${ctc_build_dir}/mibd/mib_modules/tap/*.py ${tmp_dir}/
elif [ "$product" == "of" ]; then
    #cp ${ctc_build_dir}/mibd/mib_modules/openflow/*.py ${tmp_dir}/
    echo "snmp py copy openflow"
else
    #cp ${ctc_build_dir}/mibd/mib_modules/cnos/*.py ${tmp_dir}/
    echo "snmp py copy cnos"
fi
rm -fr ${tmp_dir}/threading_agent.py
file_list=$(ls ${tmp_dir}/*.py ${tmp_dir}/subagent.sh ${tmp_dir}/trap_agent)
for f in $file_list; do
    echo "file /usr/sbin/`basename $f` ${tmp_dir}/`basename $f` 755 0 0" >> ${tmp_file}
done
#
##############

# openflow etc
if [ "${enable_openflow}" == "y" ]; then
echo "
dir     /usr/local    755 0 0
dir     /usr/local/openvswitch  755 0 0
dir     /usr/local/openvswitch/bin  755 0 0
dir     /usr/local/openvswitch/var  755 0 0
dir     /usr/local/openvswitch/var/run  755 0 0
dir     /usr/local/openvswitch/var/log  755 0 0
dir     /usr/local/openvswitch/etc  755 0 0
dir     /usr/local/openvswitch/etc/db  755 0 0
dir     /usr/local/openvswitch/data  755 0 0

file    /usr/local/openvswitch/etc/db/conf.db ${etc_dir}/openflow/${board}/conf.db 755 0 0
file    /usr/local/openvswitch/etc/db/conf_org.db ${etc_dir}/openflow/${board}/conf.db 755 0 0
file    /usr/local/openvswitch/data/vswitch.ovsschema ${ovs_dir}/vswitchd/vswitch.ovsschema 755 0 0
" >> ${tmp_file}

prog_list="ovs-vswitchd ovsdb-server ovsdb-tool ovs-vsctl ovs-ofctl ovs-appctl"
for prog in ${prog_list}; do
    cp ${ctc_build_dir}/bin/${prog} ${tmp_dir}
    if [ "$ver" == "r" ]; then
        ${CROSS_COMPILER}strip ${tmp_dir}/${prog}
    fi 
    echo "file /usr/local/openvswitch/bin/${prog} ${tmp_dir}/${prog} 755 0 0" >> ${tmp_file}
done

# openflow
echo "file /usr/local/openvswitch/bin/ovsdb_sync.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/ovsdb_sync.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/load_startup_entry.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/load_startup_entry.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/load_post_init_hook.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/load_post_init_hook.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/write_startup_flows.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/write_startup_flows.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/write_startup_groups.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/write_startup_groups.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/write_startup_meters.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/write_startup_meters.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/openflow_controller_op.py ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/openflow_controller_op.py 755 0 0" >> ${tmp_file}

fi

echo "Create cnos package..."

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/cnos.cpio.lzma

rm ${tmp_file}
rm -rf ${tmp_dir}

touch ${tmp_file}
mkdir ${tmp_dir}

##########################################
# package etc files

echo "
dir /etc 755 0 0
dir /etc/pam.d 755 0 0

file    /etc/profile    ${etc_dir}/${board}/profile              755 0 0
file    /etc/services       ${etc_dir}/services                 644 0 0
file    /etc/protocols       ${etc_dir}/protocols               644 0 0
file    /etc/resolv.conf    ${etc_dir}/resolv.conf              644 0 0
file    /etc/nsswitch.conf  ${etc_dir}/nsswitch.conf            644 0 0
file    /etc/cf_hotplug     ${etc_dir}/cf_hotplug               755 0 0
file    /etc/login.sh ${etc_dir}/${board}/login.sh    755 0 0
file    /etc/securetty ${pre_compiled_dir}/x86_64/etc/securetty 600 0 0
file    /etc/fw_env.config     ${etc_dir}/fw_env.config               755 0 0
file    /etc/pam.d/other ${pre_compiled_dir}/x86_64/etc/pam.d/other 644 0 0
file    /etc/pam.d/clish ${pre_compiled_dir}/x86_64/etc/pam_ctc_conf 644 0 0
slink   /etc/pam.d/sshd /etc/pam.d/clish 755 0 0
file    /etc/start_klish_for_console.sh ${etc_dir}/${board}/start_klish_for_console.sh 744 0 0
file    /etc/format_flash.sh     ${etc_dir}/${board}/format_flash_debian.sh              755 0 0

" > ${tmp_file}

echo "file /etc/ntp.conf ${ctc_build_dir}/etc/ntp.conf 644 0 0" >> ${tmp_file}
echo "dir /etc/ntp 755 0 0" >> ${tmp_file}
echo "file /etc/ntp/keys ${ctc_build_dir}/etc/ntpkeys.conf 644 0 0" >> ${tmp_file}
echo "file /etc/ntp/ntpd.debug ${ctc_build_dir}/etc/ntpd.debug 644 0 0" >> ${tmp_file}

echo "Create cnos etc package..."

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/etc.cpio.lzma

rm ${tmp_file}
rm -rf ${tmp_dir}

touch ${tmp_file}
mkdir ${tmp_dir}

##########################################

cp -rf ${etc_dir}/${board}/debian ${tmp_dir}
cp ${kbuild_dir}/cnos.cpio.lzma ${tmp_dir}/debian/cnos_installer/
cp ${kbuild_dir}/etc.cpio.lzma ${tmp_dir}/debian/cnos_installer/
cp ${kbuild_dir}/arch/x86/boot/bzImage ${tmp_dir}/debian/debian_installer/vmlinuz-3.10.28-amd64
cp ${BUSYBOX_DIR}/busybox ${tmp_dir}/debian/debian_installer/busybox
cp ${etc_dir}/${board}/start_klish_for_console.sh ${tmp_dir}/debian/debian_installer/start_klish_for_console.sh
rm -f ${kbuild_dir}/modules/lib/modules/3.10.28/build ${kbuild_dir}/modules/lib/modules/3.10.28/source
cp -rf ${kbuild_dir}/modules/lib ${tmp_dir}/debian/debian_installer/lib
cd ${tmp_dir}/debian/debian_installer/ > /dev/null 2>&1
tar -czf modules.tar.gz lib
cd - > /dev/null 2>&1

cd ${tmp_dir}/debian/cnos_installer/
chmod +x make_cnos_installer.sh
./make_cnos_installer.sh
cp cnos-installer ${tmp_dir}/debian/cnos_installer/deb/boot/cnos/
echo Date: `date +"%Y-%m-%d %H:%M:%S"` >> ${tmp_dir}/debian/cnos_installer/deb/DEBIAN/control
#echo Checksum: `cksum cnos-installer | awk {'print $1'}` >> ${tmp_dir}/debian/cnos_installer/deb/DEBIAN/control

if [ "$PRODUCT" == "sai" ]; then
    OSP_VERSION=`grep -E '#define[[:space:]]+SAI_VERSION' ${TOP_DIR}/proto/include/version.h | awk -F [\"] {'print $2'}`
elif [ "$PRODUCT" == "tap" ]; then
    OSP_VERSION=`grep -E '#define[[:space:]]+TAP_VERSION' ${TOP_DIR}/proto/include/version.h | awk -F [\"] {'print $2'}`
elif [ "$PRODUCT" == "of" ]; then
    OSP_VERSION=`grep -E '#define[[:space:]]+OF_VERSION' ${TOP_DIR}/proto/include/version.h | awk -F [\"] {'print $2'}`
elif [ "$PRODUCT" == "ofdpa" ]; then
    OSP_VERSION=`grep -E '#define[[:space:]]+OFDPA_VERSION' ${TOP_DIR}/proto/include/version.h | awk -F [\"] {'print $2'}`
else
    OSP_VERSION=`grep -E '#define[[:space:]]+L2PLUS_VERSION' ${TOP_DIR}/proto/include/version.h | awk -F [\"] {'print $2'}`
fi
echo "Version: $OSP_VERSION" >> ${tmp_dir}/debian/cnos_installer/deb/DEBIAN/control

cd deb
md5sum `find etc -type f` > DEBIAN/md5sums
md5sum `find boot -type f` >> DEBIAN/md5sums
cd - >/dev/null 2>&1
dpkg -b deb centec_cnos.deb
cp centec_cnos.deb ${PLAT_TOP_DIR}/build_svc


cp centec_cnos.deb ${tmp_dir}/debian/debian_installer
touch ${tmp_dir}/debian/debian_installer/version
DEBIAN_VERSION=`grep -E '#define[[:space:]]+DEBIAN_VERSION' ${TOP_DIR}/proto/include/version.h | awk -F [\"] {'print $2'}`
echo "$DEBIAN_VERSION" > ${tmp_dir}/debian/debian_installer/version

cd ${tmp_dir}/debian/debian_installer/
chmod +x make_onie_installer.sh
./make_onie_installer.sh
cp onie-installer ${PLAT_TOP_DIR}/build_svc

cd ${PLAT_TOP_DIR}/build_svc
rm ${tmp_file}
rm -rf ${tmp_dir}
