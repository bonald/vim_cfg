#!/bin/sh
set -o errexit

product=$PRODUCT
board=$BOARD
rootfs_dir=$BUILD_ROOTFS_DIR
CROSS_COMPILER=$CROSS_COMPILE
etc_dir=$PLAT_TOP_DIR/build_svc/etc
ctc_build_dir=$BLD_DIR
kbuild_dir=$KDIR
top_dir=$TOP_DIR
klish_dir=$TOP_DIR/klish
ver=$VER
version=2.11.1.so
bin_dir=bin
mkdir -m 777 -p /dev/shm/software
mkdir -p /dev/shm/software/$USER
tmp_file=$(mktemp -p /dev/shm/software/$USER/)
tmp_dir=$(mktemp -p /dev/shm/software/$USER/ -d)
pre_compiled_dir=${TOP_DIR}/third_party/precompiled
enable_rpc_server=${ENABLE_RPC_SERVER}
enable_debian=${ENABLE_DEBIAN_LINUX}
enable_openflow=${ENABLE_OPENFLOW}
ovs_dir=$TOP_DIR/proto/openflow/openvswitch-2.3.0
language=$LANGUAGE

#package init config
package_init_config=""
if [ "${enable_debian}" == "y" ]; then
    cboot_tmp_file=$(mktemp)
    package_init_config="
slink   /bin/init /bin/busybox 755 0 0
file    /sbin/init          ${etc_dir}/${board}/debian/cboot.sysinit     755 0 0
file    /switch/etc/rc.sysinit     ${etc_dir}/${board}/debian/cboot_debug.sysinit        755 0 0
file    /etc/inittab        ${etc_dir}/${board}/debian/cboot.inittab     755 0 0
file    /etc/rc.sh          ${etc_dir}/${board}/rc.sysinit.hi3535               755 0 0"
# append debian related command xml files into image
cp -rf ${klish_dir}/xml-examples/debian_cmd/*.xml ${ctc_build_dir}/etc/cmd
else
    package_init_config="
file    /switch/etc/rc.sysinit     ${etc_dir}/${board}/rc.sysinit.hi3535               755 0 0"
fi

echo "
dir     /switch     755 0 0
dir     /switch/etc 755 0 0
dir     /switch/etc/cmd 755 0 0
dir     /cmd        755 0 0
dir     /tftpboot   777 0 0
dir     /tmp/cmd    755 0 0
dir     /usr/share  755 0 0
dir     /usr/share/zoneinfo 755 0 0
dir     /var            755 0 0
dir     /var/www    755 0 0
dir     /var/www/images    755 0 0
dir     /var/www/pub    755 0 0
dir     /var/lib        755 0 0
dir     /var/lib/ntp    755 0 0
dir     /var/log        755 0 0
dir     /var/run        755 0 0
dir     /var/empty      755 0 0
dir     /etc/ssh        755 0 0
dir     /etc/ssh/keys   700 0 0
dir     /data        755 0 0
dir     /mnt/data   755 0 0
#comment by jcao for bug 19380,2012/05/23
#open by Alexander for bug 33970, use crontab to clear file per 10 minutes
slink   /var/log/lastlog /mnt/flash/log/lastlog 755 0 0
slink   /var/log/wtmp /mnt/flash/log/wtmp 755 0 0

slink   /conf /mnt/flash/conf   755 0 0

dir /etc/pam.d 755 0 0
dir /lib/security 755 0 0

dir /centec_switch 755 0 0
slink /centec_switch/etc /mnt/flash/conf 755 0 0

nod     /dev/console 600 0 0 c 5 1
nod     /dev/ttyS0 600 0 0 c 4 64
nod     /dev/hda  660 0 0 b 3 0
nod     /dev/hda1  660 0 0 b 3 1
nod     /dev/hda2  660 0 0 b 3 2
nod     /dev/hda3  660 0 0 b 3 3
nod     /dev/hda4  660 0 0 b 3 4
nod     /dev/hda5  660 0 0 b 3 5
nod     /dev/ctc_monitor 660 0 0 c 200 0

file    /lib/libpam.so.0.83.1 ${rootfs_dir}/usr/lib/libpam.so.0.83.1 755 0 0
slink   /lib/libpam.so libpam.so.0.83.1 755 0 0
slink   /lib/libpam.so.0 libpam.so.0.83.1 755 0 0

file    /lib/libpam_misc.so.0.82.0 ${rootfs_dir}/usr/lib/libpam_misc.so.0.82.0 755 0 0
slink   /lib/libpam_misc.so libpam_misc.so.0.82.0 755 0 0
slink   /lib/libpam_misc.so.0 libpam_misc.so.0.82.0 755 0 0

file    /lib/libpamc.so.0.82.1 ${rootfs_dir}/usr/lib/libpamc.so.0.82.1 755 0 0
slink   /lib/libpamc.so libpamc.so.0.82.1 755 0 0
slink   /lib/libpamc.so.0 libpamc.so.0.82.1 755 0 0

file    /lib/security/pam_unix.so ${rootfs_dir}/usr/lib/security/pam_unix.so 755 0 0
file    /lib/security/pam_deny.so ${rootfs_dir}/usr/lib/security/pam_deny.so 755 0 0
#file    /etc/securetty ${pre_compiled_dir}/hi3535/etc/securetty 600 0 0
#file    /etc/pam.d/other ${pre_compiled_dir}/hi3535/etc/pam.d/other 644 0 0
file    /etc/pam.d/clish ${ctc_build_dir}/etc/pam_ctc_conf 644 0 0
slink   /etc/pam.d/sshd /etc/pam.d/clish 644 0 0

file    /lib/libproc-3.2.8.so ${rootfs_dir}/usr/lib/libproc-3.2.8.so 755 0 0

file    /lib/libcrypto.so.1.0.0 ${rootfs_dir}/usr/lib/libcrypto.so.1.0.0 755 0 0
slink   /lib/libcrypto.so libcrypto.so.1.0.0 755 0 0
file    /lib/libssl.so.1.0.0 ${rootfs_dir}/usr/lib/libssl.so.1.0.0 755 0 0
slink   /lib/libssl.so libssl.so.1.0.0 755 0 0

file    /lib/libstdc++.so.6 ${rootfs_dir}/usr/lib/libstdc++.so.6.0.12 755 0 0
slink   /lib/libstdc++.so libstdc++.so.6 755 0 0
file    /lib/libgcc_s.so.1 ${rootfs_dir}/usr/lib/libgcc_s.so.1 755 0 0
slink   /lib/libgcc_s.so libgcc_s.so.1 755 0 0

file    /lib/libz.so.1.2.8 ${rootfs_dir}/usr/lib/libz.so.1.2.8 755 0 0
slink   /lib/libz.so.1 libz.so.1.2.8  755 0 0
slink   /lib/libz.so libz.so.1.2.7  755 0 0

file    /lib/libncurses.so.5.9 ${rootfs_dir}/usr/lib/libncurses.so.5.9 755 0 0
slink   /lib/libncurses.so.5 libncurses.so.5.9 755 0 0
slink   /lib/libncurses.so libncurses.so.5 755 0 0

file    /lib/libpcap.so.1.7.4  ${rootfs_dir}/usr/lib/libpcap.so.1.7.4  755 0 0
slink   /lib/libpcap.so.1 libpcap.so.1.7.4  755 0 0
slink   /lib/libpcap.so libpcap.so.1 755 0 0

file    /lib/libglib-2.0.so.0.4800.1 ${rootfs_dir}/usr/lib/libglib-2.0.so.0.4800.1 755 0 0
slink   /lib/libglib-2.0.so libglib-2.0.so.0.4800.1 755 0 0
slink   /lib/libglib-2.0.so.0 libglib-2.0.so.0.4800.1 755 0 0

file    /lib/libgobject-2.0.so.0.4800.1 ${rootfs_dir}/usr/lib/libgobject-2.0.so.0.4800.1 755 0 0
slink   /lib/libgobject-2.0.so libgobject-2.0.so.0.4800.1 755 0 0
slink   /lib/libgobject-2.0.so.0 libgobject-2.0.so.0.4800.1 755 0 0

file    /lib/libgthread-2.0.so.0.4800.1 ${rootfs_dir}/usr/lib/libgthread-2.0.so.0.4800.1 755 0 0
slink   /lib/libgthread-2.0.so libgthread-2.0.so.0.4800.1 755 0 0
slink   /lib/libgthread-2.0.so.0 libgthread-2.0.so.0.4800.1 755 0 0

file    /lib/libffi.so.6.0.4 ${rootfs_dir}/usr/lib/libffi.so.6.0.4 755 0 0
slink   /lib/libffi.so libffi.so.6.0.4 755 0 0
slink   /lib/libffi.so.6 libffi.so.6.0.4 755 0 0
file    /lib/libpcre.so.1.2.6 ${rootfs_dir}/usr/lib/libpcre.so.1.2.6 755 0 0 
slink   /lib/libpcre.so.1 libpcre.so.1.2.6 755 0 0
file    /lib/libip4tc.so.0.1.0 ${rootfs_dir}/usr/lib/libip4tc.so.0.1.0 755 0 0 
slink   /lib/libip4tc.so.0  libip4tc.so.0.1.0 755 0 0
file    /lib/libip6tc.so.0.1.0 ${rootfs_dir}/usr/lib/libip6tc.so.0.1.0 755 0 0 
slink   /lib/libip6tc.so.0  libip6tc.so.0.1.0 755 0 0
file    /lib/libxtables.so.12.0.0 ${rootfs_dir}/usr/lib/libxtables.so.12.0.0 755 0 0 
slink   /lib/libxtables.so.12  libxtables.so.12.0.0 755 0 0

file    /lib/libnetfilter_conntrack.so.3.6.0 ${rootfs_dir}/usr/lib/libnetfilter_conntrack.so.3.6.0 755 0 0 
slink   /lib/libnetfilter_conntrack.so.3  libnetfilter_conntrack.so.3.6.0 755 0 0

file    /lib/libnfnetlink.so.0.2.0 ${rootfs_dir}/usr/lib/libnfnetlink.so.0.2.0 755 0 0 
slink   /lib/libnfnetlink.so.0  libnfnetlink.so.0.2.0 755 0 0

file    /lib/libmnl.so.0.2.0 ${rootfs_dir}/usr/lib/libmnl.so.0.2.0 755 0 0 
slink   /lib/libmnl.so.0   libmnl.so.0.2.0 755 0 0

${package_init_config}

#added by xgu for bug 13955
file    /sbin/mii-tool ${rootfs_dir}/usr/${bin_dir}/mii-tool 755 0 0
file    /sbin/ethtool ${rootfs_dir}/usr/${bin_dir}/ethtool 755 0 0
file    /sbin/tcpdump ${rootfs_dir}/usr/${bin_dir}/tcpdump 755 0 0
file    /sbin/strace ${rootfs_dir}/usr/${bin_dir}/strace 755 0 0
file    /bin/ls ${rootfs_dir}/usr/${bin_dir}/ls_arm 755 0 0
file    /bin/ps ${rootfs_dir}/usr/${bin_dir}/ps 755 0 0
file    /bin/netstat ${rootfs_dir}/usr/${bin_dir}/netstat 755 0 0
file    /bin/ping ${rootfs_dir}/usr/${bin_dir}/ping 755 0 0
file    /bin/ping6 ${rootfs_dir}/usr/${bin_dir}/ping6 755 0 0
#file    /usr/bin/traceroute6 ${rootfs_dir}/usr/${bin_dir}/traceroute6 755 0 0

file    /usr/bin/gdbserver ${rootfs_dir}/usr/${bin_dir}/gdbserver 755 0 0
file    /usr/bin/gdb ${rootfs_dir}/usr/${bin_dir}/gdb 755 0 0

file    /usr/bin/core_helper     ${etc_dir}/core_helper     755 0 0
file    /usr/sbin/lspci ${rootfs_dir}/usr/${bin_dir}/lspci 755 0 0
file    /usr/sbin/setpci ${rootfs_dir}/usr/${bin_dir}/setpci 755 0 0
file    /usr/sbin/startup_monitor ${etc_dir}/${board}/startup_monitor 755 0 0
file    /usr/sbin/tftpd ${rootfs_dir}/usr/${bin_dir}/tftpd 755 0 0

file    /etc/termcap        ${etc_dir}/termcap                  644 0 0
file    /etc/profile        ${etc_dir}/profile                  644 0 0
file    /etc/services       ${etc_dir}/services                 644 0 0
file    /etc/protocols       ${etc_dir}/protocols               644 0 0
file    /switch/etc/resolv.conf    ${etc_dir}/resolv.conf              644 0 0
file    /etc/nsswitch.conf  ${etc_dir}/nsswitch.conf            644 0 0
file    /etc/cf_hotplug     ${etc_dir}/cf_hotplug               755 0 0
file    /usr/sbin/restart.sh ${etc_dir}/restart.sh    755 0 0
file    /switch/etc/init-ns.sh ${etc_dir}/init-ns.sh    755 0 0
file    /switch/etc/init-veth.sh ${etc_dir}/init-veth.sh    755 0 0
file    /etc/login.sh ${etc_dir}/login.sh    755 0 0
file    /etc/fw_env.config     ${etc_dir}/fw_env.config               755 0 0
file    /usr/share/zoneinfo/zone.tab ${PLAT_TOP_DIR}/build_svc/zone.tab 644 0 0
file    /usr/share/zoneinfo/iso3166.tab ${PLAT_TOP_DIR}/build_svc/iso3166.tab 644 0 0
" > ${tmp_file}

# For emulation board
if [ "$board" == "48t4x" ]; then
echo "
file    /tmp/ShowIpeDiscardType.txt ${ctc_build_dir}/etc/ShowIpeDiscardType.txt 644 0 0
file    /tmp/ShowEpeDiscardType.txt ${ctc_build_dir}/etc/ShowEpeDiscardType.txt 644 0 0
" >> ${tmp_file}
fi

echo "dir /switch/etc/datapath_profile 755 0 0" >> ${tmp_file}
profile_list=$(ls $etc_dir/datapath_profile)
for f in $profile_list; do
    echo "file /switch/etc/datapath_profile/$f ${etc_dir}/datapath_profile/$f 755 0 0" >> ${tmp_file}
done

echo "dir /switch/etc/spec 755 0 0" >> ${tmp_file}
profile_list=$(ls $etc_dir/spec)
for f in $profile_list; do  
    echo "file /switch/etc/spec/$f ${etc_dir}/spec/$f 755 0 0" >> ${tmp_file}
done 

# For emulation board
if [ "$board" == "hw_emu" ] || [ "$board" == "emu_svc" ]; then
echo "
dir /etc/emu_init 755 0 0
file /etc/emu_init/hwinit.txt ${SRCROOT}/TheGrandCanal/emu_init/hwinit.txt 644 0 0
file /etc/emu_init/disable_phy ${SRCROOT}/TheGrandCanal/emu_init/disable_phy 644 0 0
file /etc/emu_init/enable_phy ${SRCROOT}/TheGrandCanal/emu_init/enable_phy 644 0 0
file /etc/emu_init/mac_init ${SRCROOT}/TheGrandCanal/emu_init/mac_init 644 0 0
file /etc/emu_init/patch ${SRCROOT}/TheGrandCanal/emu_init/patch 644 0 0
file /etc/emu_init/InitBayDataPath_24GNet_16GFabric.pl ${SRCROOT}/TheGrandCanal/emu_init/InitBayDataPath_24GNet_16GFabric.pl 644 0 0
" >> ${tmp_file}
fi


# Modified native commands
cp ${ctc_build_dir}/bin/ctctop ${tmp_dir}
cp ${ctc_build_dir}/bin/ctctftp ${tmp_dir}
cp ${ctc_build_dir}/bin/ctcmore ${tmp_dir}
cp ${ctc_build_dir}/bin/ctcftp  ${tmp_dir}
cp ${ctc_build_dir}/bin/chvrf   ${tmp_dir}
cp ${ctc_build_dir}/bin/ctcwatch ${tmp_dir}
${CROSS_COMPILER}strip ${tmp_dir}/ctctop
${CROSS_COMPILER}strip ${tmp_dir}/ctctftp
${CROSS_COMPILER}strip ${tmp_dir}/ctcmore
${CROSS_COMPILER}strip ${tmp_dir}/ctcftp
${CROSS_COMPILER}strip ${tmp_dir}/chvrf
${CROSS_COMPILER}strip ${tmp_dir}/ctcwatch
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
elif [ "$product" == "of" ]; then
cp ${top_dir}/apps/web/static/switch/templates/nav_hybrid.html ${tmp_dir}/web/static/switch/templates/nav.html
fi

if [ "$language" == "en" ]; then
cp ${top_dir}/apps/web/static/switch/templates/login_en.html ${tmp_dir}/web/static/switch/templates/login.html
cp ${top_dir}/apps/web/static/switch/js/header_tap_def_en.js ${tmp_dir}/web/static/switch/js/header.js
fi

org_pwd=$(pwd)
cd ${tmp_dir}
${top_dir}/apps/web/pkg_web.sh web ${tmp_dir}/web.bin
echo "file /var/web.bin ${tmp_dir}/web.bin  755 0 0" >> ${tmp_file}
rm ${tmp_dir}/web -rf
cd ${org_pwd}

#ls -l webImage.bin
# for debugging purpose

# package rpcapi binary image
cp ${top_dir}/apps/rpcapi/ ${tmp_dir}/rpcapi/ -rf

org_pwd=$(pwd)
cd ${tmp_dir}
${top_dir}/apps/rpcapi/pkg_rpcapi.sh rpcapi ${tmp_dir}/rpcapi.bin
echo "file /var/rpcapi.bin ${tmp_dir}/rpcapi.bin  755 0 0" >> ${tmp_file}

rm ${tmp_dir}/rpcapi -rf
cd ${org_pwd}

# create cpio_list file for debian cboot image
# Modified by zhangdy, 050413, add ctcftp/ctctftp tool for user use in cboot image.
# all files need be packed into the cboot image MUST be listed above in the file: ${tmp_file}
if [ "${enable_debian}" == "y" ]; then
    cp ${tmp_file} ${cboot_tmp_file}
fi

cp ${ctc_build_dir}/bin/ntpd  ${tmp_dir}
cp ${ctc_build_dir}/bin/ntpdc ${tmp_dir} 
cp ${ctc_build_dir}/bin/ntpq  ${tmp_dir}
if [ $ver == "r" ]; then
    ${CROSS_COMPILER}strip ${tmp_dir}/ntpd
    ${CROSS_COMPILER}strip ${tmp_dir}/ntpdc
    ${CROSS_COMPILER}strip ${tmp_dir}/ntpq
fi
echo "file /usr/sbin/ntpd ${tmp_dir}/ntpd 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpdc ${tmp_dir}/ntpdc 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpq ${tmp_dir}/ntpq 755 0 0" >> ${tmp_file}
echo "file /etc/ntp.conf ${ctc_build_dir}/etc/ntp.conf 644 0 0" >> ${tmp_file}
echo "dir /etc/ntp 755 0 0" >> ${tmp_file}
echo "file /etc/ntp/keys ${ctc_build_dir}/etc/ntpkeys.conf 644 0 0" >> ${tmp_file}

osp_lib_list="libcdbclient.so libctclib.so libgenlog.so libsal.so libproto.so libcdb.so libsdb.so libprotopbc.so libfei.so libsock.so libtask.so libcjson.so libdbclient.so"
for osp_lib in ${osp_lib_list}; do
    cp ${ctc_build_dir}/lib/${osp_lib} ${tmp_dir}/ 

    #Do not strip prog currently for happy debuging
    if [ "${enable_openflow}" == "n" ]; then
        if [ $ver == "r" ]; then
            ${CROSS_COMPILER}strip ${tmp_dir}/${osp_lib}
        fi
    fi
    echo "file /lib/${osp_lib} ${tmp_dir}/${osp_lib} 755 0 0" >> ${tmp_file}
done

cp ${ctc_build_dir}/lib/pam_ctc.so ${tmp_dir}/ && ${CROSS_COMPILER}strip ${tmp_dir}/pam_ctc.so
echo "file /lib/security/pam_ctc.so ${tmp_dir}/pam_ctc.so 755 0 0" >> ${tmp_file}

cp ${ctc_build_dir}/lib/libnss_cdb.so.2 ${tmp_dir}/ && ${CROSS_COMPILER}strip ${tmp_dir}/libnss_cdb.so.2
echo "file /lib/libnss_cdb.so.2 ${tmp_dir}/libnss_cdb.so.2 755 0 0" >> ${tmp_file}
echo "slink /lib/libnss_cdb.so /lib/libnss_cdb.so.2 755 0 0" >> ${tmp_file}

cmd_xml_list=$(ls ${ctc_build_dir}/etc/cmd)
for f in $cmd_xml_list; do
    echo "file /switch/etc/cmd/$f ${ctc_build_dir}/etc/cmd/$f 755 0 0" >> ${tmp_file}
done

pre_compiled_bin_files="sort df ss lrz lsz scp"
for bin_file in ${pre_compiled_bin_files}; do
    cp ${pre_compiled_dir}/hi3535/bin/${bin_file} ${tmp_dir} 
    echo "file /usr/bin/${bin_file} ${tmp_dir}/${bin_file} 755 0 0" >> ${tmp_file}
done

# embed conntrack tool for manage conntrack entry.
pre_compiled_bin_files="conntrack strace"
for bin_file in ${pre_compiled_bin_files}; do
    cp ${pre_compiled_dir}/hi3535/sbin/${bin_file} ${tmp_dir} 
    echo "file /usr/bin/${bin_file} ${tmp_dir}/${bin_file} 755 0 0" >> ${tmp_file}
done

quagga_conf_list="zebra.conf ospfd.conf pimd.conf "
for conf in ${quagga_conf_list}; do
    cp ${ctc_build_dir}/etc/${conf} ${tmp_dir} 
    echo "file /switch/etc/${conf} ${tmp_dir}/${conf} 755 0 0" >> ${tmp_file}
done
#copy redis.conf
#echo "file /switch/etc/redis.conf ${ctc_build_dir}/etc/redis.conf 644 0 0" >> ${tmp_file}
# LKMs
lkm_list="ctc_hw.ko dal.ko"
for lkm in ${lkm_list}; do
    cp ${ctc_build_dir}/bin/${lkm} ${tmp_dir} 
if [ $ver == "r" ]; then
    ${CROSS_COMPILER}strip -g ${tmp_dir}/${lkm}
fi
    echo "file /lib/${lkm} ${tmp_dir}/${lkm} 644 0 0" >> ${tmp_file}
done

# board_type_detect
cp ${ctc_build_dir}/bin/board_type_detect ${tmp_dir} 
${CROSS_COMPILER}strip ${tmp_dir}/board_type_detect
echo "file /usr/sbin/board_type_detect ${tmp_dir}/board_type_detect 755 0 0" >> ${tmp_file}

# boot console no password
cp ${ctc_build_dir}/bin/nopassword ${tmp_dir}
${CROSS_COMPILER}strip ${tmp_dir}/nopassword
echo "file /usr/sbin/nopassword ${tmp_dir}/nopassword 755 0 0" >> ${tmp_file}

# reboot_record
cp ${ctc_build_dir}/bin/reboot_record ${tmp_dir}
${CROSS_COMPILER}strip ${tmp_dir}/reboot_record
echo "file /usr/sbin/reboot_record ${tmp_dir}/reboot_record 755 0 0" >> ${tmp_file}

# reg_ops
cp ${ctc_build_dir}/bin/reg_ops ${tmp_dir}
echo "file /usr/sbin/reg_ops ${tmp_dir}/reg_ops 755 0 0" >> ${tmp_file}

# PMs
cli_list="klish cdbctl viewctl fnconvert "
#quagga_list="vtysh zebra bgpd ospfd ripd "
quagga_list="vtysh zebra ospfd pimd "
if [ "$product" == "tap" ]; then
pm_list="ccs cds chsm switch fea routed appcfg ptp bhm ctcmon snmpagent ssm ncs chvrf ctc_shell lcsh reactorctl getrip"
else
pm_list="ccs cds chsm switch opm routed fea appcfg ptp bhm ctcmon snmpagent ssm ncs chvrf ctc_shell lcsh reactorctl dhcrelay dhcsnooping dhclient getrip"
fi

prog_list=$cli_list$quagga_list$pm_list
prog_list=$prog_list" snmpget snmpset snmpwalk"
prog_list=$prog_list" angel authd"

for prog in ${prog_list}; do
    cp ${ctc_build_dir}/bin/${prog} ${tmp_dir} 
    if [ $ver == "r" ]; then
        ${CROSS_COMPILER}strip ${tmp_dir}/${prog}
    fi
    echo "file /usr/sbin/${prog} ${tmp_dir}/${prog} 755 0 0" >> ${tmp_file}
done

# SNMP
cp ${ctc_build_dir}/lib/libnetsnmpagent.so ${tmp_dir}/ && ${CROSS_COMPILER}strip ${tmp_dir}/libnetsnmpagent.so
echo "file /lib/libnetsnmpagent.so ${tmp_dir}/libnetsnmpagent.so 755 0 0" >> ${tmp_file}
cp ${ctc_build_dir}/bin/snmpd ${tmp_dir} 
${CROSS_COMPILER}strip ${tmp_dir}/snmpd
echo "file /usr/sbin/snmpd ${tmp_dir}/snmpd 755 0 0" >> ${tmp_file}
echo "file /switch/etc/snmpd.conf ${ctc_build_dir}/etc/snmpd.conf 644 0 0" >> ${tmp_file}

# Profiles
profile_list="mem_profile_default.cfg mem_profile_hybrid_default.cfg mem_profile_tap_default.cfg"
for profile in ${profile_list}; do
    echo "file /switch/etc/${profile} ${ctc_build_dir}/etc/${profile} 644 0 0" >> ${tmp_file}
done

# LKMs
lkm_list="ctc_hw.ko dal.ko"
for lkm in ${lkm_list}; do
    cp ${ctc_build_dir}/bin/${lkm} ${tmp_dir} 
if [ $ver == "r" ]; then
    ${CROSS_COMPILER}strip -g ${tmp_dir}/${lkm}
fi
    echo "file /lib/${lkm} ${tmp_dir}/${lkm} 644 0 0" >> ${tmp_file}
done

# IPTABLES
#cp ${PLAT_TOP_DIR}/build_svc/etc/iptables-hi3535 tmp/iptables
#${CROSS_COMPILER}strip tmp/iptables
#echo "file /usr/bin/iptables ${PLAT_TOP_DIR}/build_svc/tmp/iptables 755 0 0" >> ${tmp_file}
# IPTABLES
cp ${PLAT_TOP_DIR}/build_svc/etc/xtables-multi-hi3535 ${tmp_dir}/xtables-multi
${CROSS_COMPILER}strip ${tmp_dir}/xtables-multi
echo "file /usr/bin/xtables-multi ${tmp_dir}/xtables-multi 755 0 0" >> ${tmp_file}

# syslog-ng
cp ${ctc_build_dir}/bin/syslog-ng ${tmp_dir}
${CROSS_COMPILER}strip ${tmp_dir}/syslog-ng
cp ${ctc_build_dir}/bin/init-syslog-ng.sh ${tmp_dir} 
echo "file /usr/sbin/syslog-ng ${tmp_dir}/syslog-ng 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/init-syslog-ng.sh ${tmp_dir}/init-syslog-ng.sh 755 0 0" >> ${tmp_file}

# logrotate
echo "file /usr/sbin/logsync.sh ${ctc_build_dir}/bin/logsync.sh 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/diaglogsync.sh ${ctc_build_dir}/bin/diaglogsync.sh 755 0 0" >> ${tmp_file}

# ssh
cp ${pre_compiled_dir}/hi3535/sbin/sshd ${tmp_dir}/sshd
${CROSS_COMPILER}strip ${tmp_dir}/sshd 
cp ${pre_compiled_dir}/hi3535/bin/ssh ${tmp_dir}/ssh
${CROSS_COMPILER}strip ${tmp_dir}/ssh 
cp ${pre_compiled_dir}/hi3535/bin/ssh ${tmp_dir}/ssh-keygen
${CROSS_COMPILER}strip ${tmp_dir}/ssh-keygen
echo "file /usr/sbin/sshd ${tmp_dir}/sshd 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ssh ${tmp_dir}/ssh 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ssh-keygen ${tmp_dir}/ssh-keygen 755 0 0" >> ${tmp_file}
ssh_cfg_list="moduli ssh_config ssh_host_dsa_key ssh_host_dsa_key.pub"
ssh_cfg_list=${ssh_cfg_list}" ssh_host_key ssh_host_key.pub ssh_host_rsa_key ssh_host_rsa_key.pub sshd_config"
ssh_cfg_list=${ssh_cfg_list}" ssh_host_ed25519_key ssh_host_ed25519_key.pub"
ssh_cfg_list=${ssh_cfg_list}" ssh_host_ecdsa_key ssh_host_ecdsa_key.pub"
for cfg in ${ssh_cfg_list}; do
    cp ${pre_compiled_dir}/hi3535/etc/${cfg} ${tmp_dir}
    echo "file /etc/ssh/${cfg} ${tmp_dir}/${cfg} 600 0 0" >> ${tmp_file}
done

# ssh
cp ${pre_compiled_dir}/hi3535/sbin/ip ${tmp_dir}/ip
echo "file /usr/sbin/ip ${tmp_dir}/ip 755 0 0" >> ${tmp_file}

# 3. package bin files
#file_list=$(ls $pre_compiled_dir/usr/bin)
#for f in $file_list; do
#    echo "file /usr/bin/$f ${pre_compiled_dir}/usr/bin/$f 755 0 0" >> ${tmp_file}
#done

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

##############
# package python
#if [ "${enable_python}" == "y" ]; then
if [ "y" == "y" ]; then
# 1. create dir for python
echo "dir /usr/include                       755 0 0" >> ${tmp_file}
echo "dir /usr/include/python2.7             755 0 0" >> ${tmp_file}
echo "dir /usr/lib                           755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7                 755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7/site-packages   755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7/lib-dynload     755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7/config          755 0 0" >> ${tmp_file}

# 1. python include files
python_dir=$pre_compiled_dir/hi3535/python
file_list=$(ls $python_dir/usr/include/python2.7)
for f in $file_list; do
    echo "file /usr/include/python2.7/$f ${python_dir}/usr/include/python2.7/$f 644 0 0" >> ${tmp_file}
done

# 2. package bin files
file_list=$(ls $python_dir/usr/bin)
for f in $file_list; do
    echo "file /usr/bin/$f ${python_dir}/usr/bin/$f 755 0 0" >> ${tmp_file}
done

# 3. package lib file
#echo "file  /usr/lib/python27.zip ${python_dir}/usr/lib/python27.zip 644 0 0" >> ${tmp_file}

# 4. package other config files
file_list=$(ls $python_dir/usr/lib/python2.7/lib-dynload)
for f in $file_list; do
    echo "file /usr/lib/python2.7/lib-dynload/$f ${python_dir}/usr/lib/python2.7/lib-dynload/$f 755 0 0" >> ${tmp_file}
done

file_list=$(ls $python_dir/usr/lib/python2.7/config)
for f in $file_list; do
    echo "file /usr/lib/python2.7/config/$f ${python_dir}/usr/lib/python2.7/config/$f 755 0 0" >> ${tmp_file}
done

# 5. package site-packages
pushd $python_dir/usr/lib/python2.7/site-packages

# 5.1 create dir for site-packages
dir_list=$(find -type d -not -iwholename '*.svn*' -not -iwholename '*.settings*')
for f in $dir_list; do
    echo "dir /usr/lib/python2.7/site-packages/$f  755 0 0" >> ${tmp_file}
done

# 5.2 package py files
file_list=$(find -type f -path '*' -not -iwholename '*.svn*' -not -iwholename '*.settings*')
for f in $file_list; do
    echo "file /usr/lib/python2.7/site-packages/$f $python_dir/usr/lib/python2.7/site-packages/$f 755 0 0" >> ${tmp_file}
done

popd

fi
# package python end

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
    if [ $ver == "r" ]; then
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

fi

# TAP etc
if [ "$product" == "tap" ]; then
echo "
dir     /usr/local     755 0 0
dir     /usr/local/tap 755 0 0
" >> ${tmp_file}
fi

echo "slink /usr/lib/python2.7/site-packages/neo_cgi.so /lib/neo_cgi.so 755 0 0" >> ${tmp_file}

org_pwd=$(pwd)
cd ${org_pwd}
##############

echo "Create main filesystem..."
#cat ${tmp_file}

# build main_initramfs initramfs(cpio.lzma) file that contains full Centec switching system software
if [ "${enable_debian}" == "y" ]; then
    ${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/main_initramfs.lzma
    cp -rf ${cboot_tmp_file} ${tmp_file}
    rm -rf ${cboot_tmp_file}
fi

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/main_initramfs.lzma
rm ${tmp_file} &
rm -rf ${tmp_dir} &
