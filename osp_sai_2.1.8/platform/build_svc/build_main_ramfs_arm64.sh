#!/bin/sh
#set -o errexit

product=$PRODUCT
board=$BOARD
rootfs_dir=$BUILD_ROOTFS_DIR
rootfs_lib_dir=aarch64-linux-gnu/libc
CROSS_COMPILER=$CROSS_COMPILE
etc_dir=$PLAT_TOP_DIR/build_svc/etc
ctc_build_dir=$BLD_DIR
kbuild_dir=$KDIR
top_dir=$TOP_DIR
klish_dir=$TOP_DIR/klish
ver=$VER
version=2.23.so
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
vender=$VENDER
cpu=${CPU}

if [ $ver == "d" ] || [ $ver == "r" ]; then
    mkdir -p ${tmp_dir}/usr
    mkdir -p ${tmp_dir}/aarch64-linux-gnu
    mkdir -p ${tmp_dir}/aarch64-linux-gnu/libc
    cp ${rootfs_dir}/usr/bin ${tmp_dir}/usr/bin -rf
    cp ${rootfs_dir}/aarch64-linux-gnu/libc/lib ${tmp_dir}/aarch64-linux-gnu/libc/lib -rf
    cp ${rootfs_dir}/usr/lib ${tmp_dir}/usr/lib -rf
    cp ${rootfs_dir}/aarch64-linux-gnu/lib64 ${tmp_dir}/aarch64-linux-gnu/lib64 -rf
    chmod 777 -R ${tmp_dir}
    find ${tmp_dir} | xargs ${CROSS_COMPILER}strip 1>/dev/null 2>&1
    rootfs_dir=${tmp_dir}
fi

#package init config
package_init_config=""
if [ "${enable_debian}" == "discard" ]; then
    cboot_tmp_file=$(mktemp)
    package_init_config="
slink   /bin/init /bin/busybox 755 0 0
file    /sbin/init          ${etc_dir}/${board}/debian/cboot.sysinit     755 0 0
file    /switch/etc/rc.sysinit     ${etc_dir}/${board}/debian/cboot_debug.sysinit        755 0 0
file    /etc/inittab        ${etc_dir}/${board}/debian/cboot.inittab     755 0 0
file    /etc/rc.sh          ${etc_dir}/${board}/rc.sysinit.${cpu}               755 0 0"
# append debian related command xml files into image
cp -rf ${klish_dir}/xml-examples/debian_cmd/*.xml ${ctc_build_dir}/etc/cmd
else
    package_init_config="
file    /switch/etc/rc.sysinit     ${etc_dir}/${board}/rc.sysinit.${cpu}              755 0 0"
fi

echo "
dir     /switch     755 0 0
dir     /switch/etc 755 0 0
dir     /switch/etc/cmd 755 0 0
dir     /switch/etc/stunnel 755 0 0
dir     /cmd        755 0 0
dir     /tftpboot   777 0 0
dir     /tmp/cmd    755 0 0
dir     /usr/share  755 0 0
dir     /usr/share/zoneinfo 755 0 0
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

file    /switch/etc/start_klish_for_console.sh     ${etc_dir}/${board}/start_klish_for_console.sh              755 0 0

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

file    /lib/libpam.so.0.84.2 ${rootfs_dir}/usr/lib/libpam.so.0.84.2 755 0 0
slink   /lib/libpam.so libpam.so.0.84.2 755 0 0
slink   /lib/libpam.so.0 libpam.so.0.84.2 755 0 0

file    /lib/libpam_misc.so.0.82.1 ${rootfs_dir}/usr/lib/libpam_misc.so.0.82.1 755 0 0
slink   /lib/libpam_misc.so libpam_misc.so.0.82.1 755 0 0
slink   /lib/libpam_misc.so.0 libpam_misc.so.0.82.1 755 0 0

file    /lib/libpamc.so.0.82.1 ${rootfs_dir}/usr/lib/libpamc.so.0.82.1 755 0 0
slink   /lib/libpamc.so libpamc.so.0.82.1 755 0 0
slink   /lib/libpamc.so.0 libpamc.so.0.82.1 755 0 0

file    /lib/security/pam_unix.so ${rootfs_dir}/usr/lib/security/pam_unix.so 755 0 0
file    /lib/security/pam_deny.so ${rootfs_dir}/usr/lib/security/pam_deny.so 755 0 0
#file    /etc/securetty ${pre_compiled_dir}/arm64/etc/securetty 600 0 0
file    /etc/pam.d/other ${pre_compiled_dir}/arm64/etc/pam.d/other 644 0 0
file    /etc/pam.d/clish ${ctc_build_dir}/etc/pam_ctc_conf 644 0 0
slink   /etc/pam.d/sshd clish 644 0 0

#file    /lib/libproc-3.2.8.so ${rootfs_dir}/usr/lib/libproc-3.2.8.so 755 0 0

file    /lib/libcrypto.so.1.0.0 ${rootfs_dir}/usr/lib/libcrypto.so.1.0.0 755 0 0
slink   /lib/libcrypto.so libcrypto.so.1.0.0 755 0 0
file    /lib/libssl.so.1.0.0 ${rootfs_dir}/usr/lib/libssl.so.1.0.0 755 0 0
slink   /lib/libssl.so libssl.so.1.0.0 755 0 0

file    /lib/libstdc++.so.6 ${rootfs_dir}/aarch64-linux-gnu/lib64/libstdc++.so.6.0.22 755 0 0
slink   /lib/libstdc++.so libstdc++.so.6 755 0 0
file    /lib/libgcc_s.so.1 ${rootfs_dir}/aarch64-linux-gnu/lib64/libgcc_s.so.1 755 0 0
slink   /lib/libgcc_s.so libgcc_s.so.1 755 0 0

file    /lib/libz.so.1.2.8 ${rootfs_dir}/usr/lib/libz.so.1.2.8 755 0 0
slink   /lib/libz.so.1 libz.so.1.2.8  755 0 0
slink   /lib/libz.so libz.so.1.2.8  755 0 0

file    /lib/libncurses.so.6.0 ${rootfs_dir}/usr/lib/libncurses.so.6.0 755 0 0
slink   /lib/libncurses.so.6 libncurses.so.6.0 755 0 0
slink   /lib/libncurses.so libncurses.so.6 755 0 0

file    /lib/libpcap.so.1.7.4 ${rootfs_dir}/usr/lib/libpcap.so.1.7.4 755 0 0
slink   /lib/libpcap.so.1 libpcap.so.1.7.4 755 0 0
slink   /lib/libpcap.so libpcap.so.1 755 0 0

file    /lib/libglib-2.0.so.0.4400.1 ${rootfs_dir}/usr/lib/libglib-2.0.so.0.4400.1 755 0 0
slink   /lib/libglib-2.0.so libglib-2.0.so.0.4400.1 755 0 0
slink   /lib/libglib-2.0.so.0 libglib-2.0.so.0.4400.1 755 0 0

file    /lib/libgobject-2.0.so.0.4400.1 ${rootfs_dir}/usr/lib/libgobject-2.0.so.0.4400.1 755 0 0
slink   /lib/libgobject-2.0.so libgobject-2.0.so.0.4400.1 755 0 0
slink   /lib/libgobject-2.0.so.0 libgobject-2.0.so.0.4400.1 755 0 0

file    /lib/libgthread-2.0.so.0.4400.1 ${rootfs_dir}/usr/lib/libgthread-2.0.so.0.4400.1 755 0 0
slink   /lib/libgthread-2.0.so libgthread-2.0.so.0.4400.1 755 0 0
slink   /lib/libgthread-2.0.so.0 libgthread-2.0.so.0.4400.1 755 0 0

file    /lib/libffi.so.6.0.4 ${rootfs_dir}/usr/lib/libffi.so.6.0.4 755 0 0
slink   /lib/libffi.so libffi.so.6.0.4 755 0 0
slink   /lib/libffi.so.6 libffi.so.6.0.4 755 0 0
file    /lib/libpcre.so.1.2.10 ${rootfs_dir}/usr/lib/libpcre.so.1.2.10 755 0 0 
slink   /lib/libpcre.so.1 libpcre.so.1.2.10 755 0 0

#file    /lib/libip4tc.so.0.1.0 ${pre_compiled_dir}/arm64/lib/libip4tc.so.0.1.0 755 0 0 
#slink   /lib/libip4tc.so.0  libip4tc.so.0.1.0 755 0 0
#slink   /lib/libip4tc.so  libip4tc.so.0.1.0 755 0 0
#file    /lib/libiptc.so.0.0.0 ${pre_compiled_dir}/arm64/lib/libiptc.so.0.0.0 755 0 0 
#slink   /lib/libiptc.so.0  libiptc.so.0.0.0 755 0 0
#slink   /lib/libiptc.so  libiptc.so.0.0.0 755 0 0
#file    /lib/libip6tc.so.0.1.0 ${pre_compiled_dir}/arm64/lib/libip6tc.so.0.1.0 755 0 0 
#slink   /lib/libip6tc.so.0  libip6tc.so.0.1.0 755 0 0
#slink   /lib/libip6tc.so  libip6tc.so.0.1.0 755 0 0
#file    /lib/libxtables.so.10.0.0 ${pre_compiled_dir}/arm64/lib/libxtables.so.10.0.0 755 0 0 
#slink   /lib/libxtables.so.10  libxtables.so.10.0.0 755 0 0
#slink   /lib/libxtables.so  libxtables.so.10.0.0 755 0 0
#file    /lib/xtables/libipt_MASQUERADE.so ${pre_compiled_dir}/arm64/lib/xtables/libipt_MASQUERADE.so 755 0 0 

file    /lib/libnetfilter_conntrack.so.3.7.0 ${rootfs_dir}/usr/lib/libnetfilter_conntrack.so.3.7.0 755 0 0 
slink   /lib/libnetfilter_conntrack.so.3  libnetfilter_conntrack.so.3.7.0 755 0 0

file    /lib/libnfnetlink.so.0.2.0 ${rootfs_dir}/usr/lib/libnfnetlink.so.0.2.0 755 0 0 
slink   /lib/libnfnetlink.so.0  libnfnetlink.so.0.2.0 755 0 0

file    /lib/libmnl.so.0.2.0 ${rootfs_dir}/usr/lib/libmnl.so.0.2.0 755 0 0 
slink   /lib/libmnl.so.0   libmnl.so.0.2.0 755 0 0

file    /lib/libnftables.so.0.0.0 ${rootfs_dir}/usr/lib/libnftables.so.0.0.0 755 0 0 
slink   /lib/libnftables.so.0   libnftables.so.0.0.0 755 0 0

file    /lib/libnftnl.so.7.3.0 ${rootfs_dir}/usr/lib/libnftnl.so.7.3.0 755 0 0 
slink   /lib/libnftnl.so.7   libnftnl.so.7.3.0 755 0 0

${package_init_config}

file    /lib/liblzo2.so.2.0.0 ${rootfs_dir}/usr/lib/liblzo2.so.2.0.0 755 0 0
slink   /lib/liblzo2.so.2 liblzo2.so.2.0.0  755 0 0
slink   /lib/liblzo2.so liblzo2.so.2.0.08  755 0 0
file    /lib/libuuid.so.1.0.0 ${rootfs_dir}/usr/lib/libuuid.so.1.0.0 755 0 0
slink   /lib/libuuid.so.1 libuuid.so.1.0.0  755 0 0
slink   /lib/libuuid.so libuuid.so.1.0.0  755 0 0
#added by xgu for bug 13955
#file    /sbin/mii-tool ${rootfs_dir}/usr/${bin_dir}/mii-tool 755 0 0
file    /sbin/ethtool ${rootfs_dir}/usr/${bin_dir}/ethtool 755 0 0
#file    /sbin/tcpdump ${rootfs_dir}/usr/${bin_dir}/tcpdump 755 0 0
file    /sbin/strace ${rootfs_dir}/usr/${bin_dir}/strace 755 0 0
file    /usr/bin/ss ${rootfs_dir}/usr/${bin_dir}/ss 755 0 0
#file    /bin/ls ${rootfs_dir}/usr/${bin_dir}/ls_arm 755 0 0
file    /bin/ps ${rootfs_dir}/usr/${bin_dir}/ps 755 0 0
file     /usr/sbin/ip ${rootfs_dir}/usr/${bin_dir}/ip 755 0 0
file    /bin/netstat ${rootfs_dir}/usr/${bin_dir}/netstat 755 0 0
file    /usr/bin/taskset ${rootfs_dir}/usr/bin/taskset 755 0 0
file    /bin/ping ${rootfs_dir}/usr/${bin_dir}/ping 755 0 0
file    /bin/ping6 ${rootfs_dir}/usr/${bin_dir}/ping6 755 0 0
#file    /usr/bin/traceroute6 ${rootfs_dir}/usr/${bin_dir}/traceroute6 755 0 0
#file    /usr/bin/conntrack ${rootfs_dir}/usr/${bin_dir}/conntrack 755 0 0
file    /usr/bin/iozone ${rootfs_dir}/usr/${bin_dir}/iozone 755 0 0
file    /usr/bin/badblocks ${rootfs_dir}/usr/${bin_dir}/badblocks 755 0 0
file    /usr/bin/fsck ${rootfs_dir}/usr/${bin_dir}/fsck 755 0 0
file    /usr/bin/fsck.ext4 ${rootfs_dir}/usr/${bin_dir}/fsck.ext4 755 0 0
file    /usr/bin/mmc ${rootfs_dir}/usr/${bin_dir}/mmc 755 0 0
file    /etc/format_flash.sh     ${etc_dir}/format_flash_arm64.sh              755 0 0
file    /etc/start_fdisk.sh      ${etc_dir}/start_fdisk.sh              755 0 0
file    /etc/fsck_flash.sh     ${etc_dir}/fsck_flash_arm64.sh              755 0 0
file    /usr/bin/gdbserver ${rootfs_dir}/usr/${bin_dir}/gdbserver 755 0 0
file    /usr/bin/gdb ${rootfs_dir}/usr/${bin_dir}/gdb 755 0 0

file    /usr/bin/core_helper     ${etc_dir}/core_helper     755 0 0
#file    /usr/sbin/lspci ${rootfs_dir}/usr/${bin_dir}/lspci 755 0 0
#file    /usr/sbin/setpci ${rootfs_dir}/usr/${bin_dir}/setpci 755 0 0
file    /usr/sbin/startup_monitor ${etc_dir}/${board}/startup_monitor 755 0 0
#file    /usr/sbin/tftpd ${rootfs_dir}/usr/${bin_dir}/tftpd 755 0 0

file    /usr/sbin/mtd_debug ${rootfs_dir}/usr/${bin_dir}/mtd_debug 755 0 0

file    /etc/termcap        ${etc_dir}/termcap                  644 0 0
file    /etc/profile        ${etc_dir}/profile                  644 0 0
file    /etc/services       ${etc_dir}/services                 644 0 0
file    /etc/protocols       ${etc_dir}/protocols               644 0 0
file    /switch/etc/resolv.conf    ${etc_dir}/resolv.conf              644 0 0
file    /etc/nsswitch.conf  ${etc_dir}/nsswitch.conf            644 0 0
file    /etc/cf_hotplug     ${etc_dir}/cf_hotplug               755 0 0
file    /etc/memory_detect.sh ${etc_dir}/memory_detect.sh    755 0 0
file    /usr/sbin/restart.sh ${etc_dir}/restart.sh    755 0 0
file    /switch/etc/init-ns.sh ${etc_dir}/init-ns.sh    755 0 0
file    /switch/etc/init-veth.sh ${etc_dir}/init-veth.sh    755 0 0
file    /etc/login.sh ${etc_dir}/login.sh    755 0 0
file    /usr/share/zoneinfo/zone.tab ${PLAT_TOP_DIR}/build_svc/zone.tab 644 0 0
file    /usr/share/zoneinfo/iso3166.tab ${PLAT_TOP_DIR}/build_svc/iso3166.tab 644 0 0
#add by zhw for multicast
file    /lib/libifaddrs.so ${rootfs_dir}/usr/lib/libifaddrs.so 755 0 0
" > ${tmp_file}

# for debian doesn't have libreadline.so.6, copied from build_mini_ramfs_arm64.sh
if [ "${enable_debian}" == "y" ]; then
echo "
#file    /lib/ld-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/ld-${version} 755 0 0
#slink   /lib/ld-linux-aarch64.so.1 ld-2.23.so 755 0 0
#file    /lib/libc-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libc-${version} 755 0 0
#slink   /lib/libc.so.6 libc-${version} 755 0 0
#file    /lib/libm-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libm-${version} 755 0 0
#slink   /lib/libm.so.6 libm-${version} 755 0 0
#file    /lib/librt-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/librt-${version} 755 0 0
#slink   /lib/librt.so.1 librt-${version} 755 0 0
#file    /lib/libcrypt-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libcrypt-${version} 755 0 0
#slink   /lib/libcrypt.so.1 libcrypt-${version} 755 0 0
#file    /lib/libutil-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libutil-${version} 755 0 0
#slink   /lib/libutil.so.1 libutil-${version} 755 0 0
#file    /lib/libnsl-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libnsl-${version} 755 0 0
#slink   /lib/libnsl.so.1 libnsl-${version} 755 0 0
#file    /lib/libnss_files-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libnss_files-${version} 755 0 0
#slink   /lib/libnss_files.so.2 libnss_files-${version} 755 0 0
#file    /lib/libnss_dns-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libnss_dns-${version} 755 0 0
#slink   /lib/libnss_dns.so.2 libnss_dns-${version} 755 0 0
#file    /lib/libresolv-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libresolv-${version} 755 0 0
#slink   /lib/libresolv.so.2 libresolv-${version} 755 0 0
file    /lib/libreadline.so.6.3 ${rootfs_dir}/usr/lib/libreadline.so.6.3 755 0 0
slink   /lib/libreadline.so.6 libreadline.so.6.3 755 0 0
#file    /lib/libpthread-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libpthread-${version} 755 0 0
#slink   /lib/libpthread.so.0 libpthread-${version} 755 0 0
#file    /lib/libthread_db-1.0.so ${rootfs_dir}/${rootfs_lib_dir}/lib/libthread_db-1.0.so 755 0 0
#slink   /lib/libthread_db.so.1 libthread_db-1.0.so 755 0 0
#file    /lib/libdl-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libdl-${version} 755 0 0
#slink   /lib/libdl.so.2 libdl-${version} 755 0 0
" >> ${tmp_file}
fi
cp ${pre_compiled_dir}/p1010/bin/mgmt-ip.sh ${tmp_dir}/mgmt-ip.sh
echo "file /usr/bin/mgmt-ip.sh ${tmp_dir}/mgmt-ip.sh 755 0 0" >> ${tmp_file}
# For emulation board 
if [ "$board" == "48t4x" ]; then
echo "
file    /tmp/ShowIpeDiscardType.txt ${ctc_build_dir}/etc/ShowIpeDiscardType.txt 644 0 0
file    /tmp/ShowEpeDiscardType.txt ${ctc_build_dir}/etc/ShowEpeDiscardType.txt 644 0 0
" >> ${tmp_file}
fi

# stunnel for HTTPS
echo "file    /usr/sbin/stunnel ${pre_compiled_dir}/arm64/sbin/stunnel 755 0 0" >> ${tmp_file}
echo "file    /switch/etc/stunnel/stunnel.conf ${pre_compiled_dir}/etc/stunnel/stunnel.conf 644 0 0" >> ${tmp_file}
echo "file    /switch/etc/stunnel/stunnel.pem ${pre_compiled_dir}/etc/stunnel/stunnel.pem 600 0 0" >> ${tmp_file}

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
    if [ "$board" == "e550" ]; then
        sed -i "s/CNOS/Tap-550/g" ${tmp_dir}/web/static/switch/js/config.js
    elif [ "$board" == "e530" ]; then
        sed -i "s/CNOS/Tap-530/g" ${tmp_dir}/web/static/switch/js/config.js
    fi
elif [ "$product" == "of" ]; then
cp ${top_dir}/apps/web/static/switch/templates/nav_hybrid.html ${tmp_dir}/web/static/switch/templates/nav.html
fi

if [ "$language" == "en" ]; then
cp ${top_dir}/apps/web/static/switch/templates/login_en.html ${tmp_dir}/web/static/switch/templates/login.html
cp ${top_dir}/apps/web/static/switch/js/header_tap_def_en.js ${tmp_dir}/web/static/switch/js/header.js
fi

# for tap en web
if [ "luxar" = "$vender" ] ; then
    cp -fr ${top_dir}/oem/script/luxar/web/. ${tmp_dir}/web/
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
if [ "${enable_debian}" == "discard" ]; then
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
echo "file /usr/sbin/ntp-iptables.sh ${ctc_build_dir}/etc/ntp-iptables.sh 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpd ${tmp_dir}/ntpd 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpdc ${tmp_dir}/ntpdc 755 0 0" >> ${tmp_file}
echo "file /usr/sbin/ntpq ${tmp_dir}/ntpq 755 0 0" >> ${tmp_file}
echo "file /etc/ntp.conf ${ctc_build_dir}/etc/ntp.conf 644 0 0" >> ${tmp_file}
echo "dir /etc/ntp 755 0 0" >> ${tmp_file}
echo "file /etc/ntp/keys ${ctc_build_dir}/etc/ntpkeys.conf 644 0 0" >> ${tmp_file}

osp_lib_list="libcdbclient.so libctclib.so libgenlog.so libsal.so libproto.so libcdb.so libsdb.so libprotopbc.so libfei.so libsock.so libtask.so libcjson.so libdbclient.so libmnl.so libnetfilter_conntrack.so libnfnetlink.so libnftnl.so libreadline.so"
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
echo "slink /lib/libnss_cdb.so libnss_cdb.so.2 755 0 0" >> ${tmp_file}

cmd_xml_list=$(ls ${ctc_build_dir}/etc/cmd)
for f in $cmd_xml_list; do
    echo "file /switch/etc/cmd/$f ${ctc_build_dir}/etc/cmd/$f 755 0 0" >> ${tmp_file}
done

pre_compiled_bin_files="sort df lrz lsz scp"
for bin_file in ${pre_compiled_bin_files}; do
    cp ${pre_compiled_dir}/arm64/bin/${bin_file} ${tmp_dir} 
    echo "file /usr/bin/${bin_file} ${tmp_dir}/${bin_file} 755 0 0" >> ${tmp_file}
done

# embed conntrack tool for manage conntrack entry.
#pre_compiled_bin_files="conntrack"
#for bin_file in ${pre_compiled_bin_files}; do
#    cp ${pre_compiled_dir}/arm64/sbin/${bin_file} ${tmp_dir} 
#    echo "file /usr/bin/${bin_file} ${tmp_dir}/${bin_file} 755 0 0" >> ${tmp_file}
#done

if [ "$product" != "tap" ]; then
quagga_conf_list="zebra.conf ospfd.conf ripd.conf bgpd.conf ospf6d.conf ripngd.conf"
for conf in ${quagga_conf_list}; do
    cp ${ctc_build_dir}/etc/${conf} ${tmp_dir} 
    echo "file /switch/etc/${conf} ${tmp_dir}/${conf} 755 0 0" >> ${tmp_file}
done
fi

#copy redis.conf
#echo "file /switch/etc/redis.conf ${ctc_build_dir}/etc/redis.conf 644 0 0" >> ${tmp_file}
# LKMs
lkm_list="ctc_hw.ko dal.ko ctc_boards.ko ctc_dpll.ko ctc_gpio.ko ctc_gpio_spi.ko ctc_phy.ko ctc_power.ko ctc_reset.ko ctc_restart.ko ctc_watchdog.ko"
for lkm in ${lkm_list}; do
    cp ${ctc_build_dir}/bin/${lkm} ${tmp_dir} 
if [ $ver == "r" ]; then
    ${CROSS_COMPILER}strip -g ${tmp_dir}/${lkm}
fi
    echo "file /lib/${lkm} ${tmp_dir}/${lkm} 644 0 0" >> ${tmp_file}
done

# flash_env
cp ${ctc_build_dir}/bin/flash_env ${tmp_dir} 
${CROSS_COMPILER}strip ${tmp_dir}/flash_env
echo "file /usr/sbin/flash_env ${tmp_dir}/flash_env 755 0 0" >> ${tmp_file}

# check_fit_image
cp ${ctc_build_dir}/bin/check_fit_image ${tmp_dir} 
${CROSS_COMPILER}strip ${tmp_dir}/check_fit_image
echo "file /usr/sbin/check_fit_image ${tmp_dir}/check_fit_image 755 0 0" >> ${tmp_file}

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
quagga_list="vtysh zebra bgpd ospfd ripd ospf6d ripngd "
#quagga_list="vtysh zebra ospfd pimd "
#add by zhw for multicast
if [ "$product" == "tap" ]; then
pm_list="ccs cds chsm switch fea appcfg bhm ctcmon snmpagent ncs chvrf ctc_shell lcsh reactorctl getripi pimdd pimd_sm"
prog_list=$cli_list$pm_list
else
pm_list="ccs cds chsm switch opm routed fea appcfg ptp bhm ctcmon snmpagent ssm ncs chvrf ctc_shell lcsh reactorctl dhcrelay dhcsnooping dhclient getrip dhclient6 nft pimdd pimd_sm"
prog_list=$cli_list$quagga_list$pm_list
fi

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
profile_list="mem_profile_default.cfg mem_profile_hybrid_default.cfg mem_profile_tap_default.cfg mem_profile_IPv6.cfg"
for profile in ${profile_list}; do
    echo "file /switch/etc/${profile} ${ctc_build_dir}/etc/${profile} 644 0 0" >> ${tmp_file}
done

# LKMs
# lkm_list="ctc_hw.ko dal.ko"
# for lkm in ${lkm_list}; do
#     cp ${ctc_build_dir}/bin/${lkm} ${tmp_dir} 
# if [ $ver == "r" ]; then
#     ${CROSS_COMPILER}strip -g ${tmp_dir}/${lkm}
# fi
#     echo "file /lib/${lkm} ${tmp_dir}/${lkm} 644 0 0" >> ${tmp_file}
# done

# IPTABLES
#cp ${PLAT_TOP_DIR}/build_svc/etc/iptables-ls1023a tmp/iptables
#${CROSS_COMPILER}strip tmp/iptables
#echo "file /usr/bin/iptables ${PLAT_TOP_DIR}/build_svc/tmp/iptables 755 0 0" >> ${tmp_file}
# IPTABLES
cp ${pre_compiled_dir}/arm64/sbin/xtables-multi ${tmp_dir}/xtables-multi
${CROSS_COMPILER}strip ${tmp_dir}/xtables-multi
echo "file /usr/bin/xtables-multi ${tmp_dir}/xtables-multi 755 0 0" >> ${tmp_file}

cp ${PLAT_TOP_DIR}/build_svc/etc/dhclient-mgmt.arm64 ${tmp_dir}/dhclient-mgmt
${CROSS_COMPILER}strip ${tmp_dir}/dhclient-mgmt
echo "file /usr/bin/dhclient-mgmt ${tmp_dir}/dhclient-mgmt 755 0 0" >> ${tmp_file}

cp ${PLAT_TOP_DIR}/build_svc/etc/dhclient-script-mgmt ${tmp_dir}/dhclient-script
echo "file /usr/bin/dhclient-script ${tmp_dir}/dhclient-script 755 0 0" >> ${tmp_file}

cp ${PLAT_TOP_DIR}/build_svc/etc/dhclient.leases ${tmp_dir}/dhclient.leases
echo "file /mnt/flash/dhclient.leases ${tmp_dir}/dhclient.leases 755 0 0" >> ${tmp_file}

cp ${PLAT_TOP_DIR}/build_svc/etc/dhclient-mgmt.arm64 ${tmp_dir}/dhclient6-mgmt
${CROSS_COMPILER}strip ${tmp_dir}/dhclient6-mgmt
echo "file /usr/bin/dhclient6-mgmt ${tmp_dir}/dhclient6-mgmt 755 0 0" >> ${tmp_file}

cp ${PLAT_TOP_DIR}/build_svc/etc/dhclient.leases ${tmp_dir}/dhclient6.leases
echo "file /usr/bin/dhclient6.leases ${tmp_dir}/dhclient6.leases 755 0 0" >> ${tmp_file}

cp ${PLAT_TOP_DIR}/build_svc/etc/dhclient-mgmt-dhcpv6.conf ${tmp_dir}/dhclient-dhcpv6.conf
echo "file /usr/bin/dhclient-dhcpv6.conf ${tmp_dir}/dhclient-dhcpv6.conf 755 0 0" >> ${tmp_file}

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
cp ${pre_compiled_dir}/arm64/sbin/sshd ${tmp_dir}/sshd
${CROSS_COMPILER}strip ${tmp_dir}/sshd 
cp ${pre_compiled_dir}/arm64/bin/ssh ${tmp_dir}/ssh
${CROSS_COMPILER}strip ${tmp_dir}/ssh 
cp ${pre_compiled_dir}/arm64/bin/ssh ${tmp_dir}/ssh-keygen
${CROSS_COMPILER}strip ${tmp_dir}/ssh-keygen
echo "file /usr/sbin/sshd ${tmp_dir}/sshd 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ssh ${tmp_dir}/ssh 755 0 0" >> ${tmp_file}
echo "file /usr/bin/ssh-keygen ${tmp_dir}/ssh-keygen 755 0 0" >> ${tmp_file}
ssh_cfg_list="moduli ssh_config ssh_host_dsa_key ssh_host_dsa_key.pub"
ssh_cfg_list=${ssh_cfg_list}" ssh_host_key ssh_host_key.pub ssh_host_rsa_key ssh_host_rsa_key.pub sshd_config"
ssh_cfg_list=${ssh_cfg_list}" ssh_host_ed25519_key ssh_host_ed25519_key.pub"
ssh_cfg_list=${ssh_cfg_list}" ssh_host_ecdsa_key ssh_host_ecdsa_key.pub"
for cfg in ${ssh_cfg_list}; do
    cp ${pre_compiled_dir}/arm64/etc/${cfg} ${tmp_dir}
    echo "file /etc/ssh/${cfg} ${tmp_dir}/${cfg} 600 0 0" >> ${tmp_file}
done

# ssh
#cp ${pre_compiled_dir}/arm64/sbin/ip ${tmp_dir}/ip
#echo "file /usr/sbin/ip ${tmp_dir}/ip 755 0 0" >> ${tmp_file}

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
#if [ "y" == "y" ]; then
if [ "${enable_debian}" != "y" ]; then
# 1. create dir for python
echo "dir /usr/include                       755 0 0" >> ${tmp_file}
echo "dir /usr/include/python2.7             755 0 0" >> ${tmp_file}
echo "dir /usr/lib                           755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7                 755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7/site-packages   755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7/lib-dynload     755 0 0" >> ${tmp_file}
echo "dir /usr/lib/python2.7/config          755 0 0" >> ${tmp_file}

# 1. python include files
python_dir=$pre_compiled_dir/arm64/python
file_list=$(ls $python_dir/usr/include/python2.7)
for f in $file_list; do
    echo "file /usr/include/python2.7/$f ${python_dir}/usr/include/python2.7/$f 644 0 0" >> ${tmp_file}
done

# 2. package bin files
file_list=$(ls $python_dir/usr/bin)
for f in $file_list; do
    echo "file /usr/bin/$f ${python_dir}/usr/bin/$f 755 0 0" >> ${tmp_file}
done

### # 3. package lib file
### #echo "file  /usr/lib/python27.zip ${python_dir}/usr/lib/python27.zip 644 0 0" >> ${tmp_file}
### 
### # 4. package other config files
### file_list=$(ls $python_dir/usr/lib/python2.7/lib-dynload)
### for f in $file_list; do
###     echo "file /usr/lib/python2.7/lib-dynload/$f ${python_dir}/usr/lib/python2.7/lib-dynload/$f 755 0 0" >> ${tmp_file}
### done
### 
### file_list=$(ls $python_dir/usr/lib/python2.7/config)
### for f in $file_list; do
###     echo "file /usr/lib/python2.7/config/$f ${python_dir}/usr/lib/python2.7/config/$f 755 0 0" >> ${tmp_file}
### done
### 
### # 5. package site-packages
### pushd $python_dir/usr/lib/python2.7/site-packages
### 
### # 5.1 create dir for site-packages
### dir_list=$(find -type d -not -iwholename '*.svn*' -not -iwholename '*.settings*')
### for f in $dir_list; do
###     echo "dir /usr/lib/python2.7/site-packages/$f  755 0 0" >> ${tmp_file}
### done
### 
### # 5.2 package py files
### file_list=$(find -type f -path '*' -not -iwholename '*.svn*' -not -iwholename '*.settings*')
### for f in $file_list; do
###     echo "file /usr/lib/python2.7/site-packages/$f $python_dir/usr/lib/python2.7/site-packages/$f 755 0 0" >> ${tmp_file}
### done
### 
### popd

# 3. package lib
pushd $python_dir/usr/lib/

# 3.1 create dir for lib
dir_list=$(find -type d -not -iwholename '*.svn*' -not -iwholename '*.settings*')
for f in $dir_list; do
    echo "dir /usr/lib/$f  755 0 0" >> ${tmp_file}
done

# 3.2 package files
file_list=$(find -type f -path '*' -not -iwholename '*.svn*' -not -iwholename '*.settings*')
for f in $file_list; do
    echo "file /usr/lib/$f $python_dir/usr/lib/$f 755 0 0" >> ${tmp_file}
done

# 3.3 package links
file_list=$(find -type l -path '*' -not -iwholename '*.svn*' -not -iwholename '*.settings*')
for f in $file_list; do
    echo "file /usr/lib/$f $python_dir/usr/lib/$f 755 0 0" >> ${tmp_file}
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
echo "file /usr/local/openvswitch/bin/write_startup_meters.sh ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/write_startup_meters.sh 755 0 0" >> ${tmp_file}
echo "file /usr/local/openvswitch/bin/openflow_controller_op.py ${PLAT_TOP_DIR}/build_svc/etc/openflow/scripts/openflow_controller_op.py 755 0 0" >> ${tmp_file}

fi

# TAP etc
if [ "$product" == "tap" ]; then
echo "
dir     /usr/local     755 0 0
dir     /usr/local/tap 755 0 0
" >> ${tmp_file}
fi

echo "slink /usr/lib/python2.7/site-packages/neo_cgi.so ../../../../lib/neo_cgi.so 755 0 0" >> ${tmp_file}

# add by zhw for multicast
echo "file /usr/bin/sendmsg ${ctc_build_dir}/bin/sendmsg 755 0 0" >> ${tmp_file}

org_pwd=$(pwd)
cd ${org_pwd}
##############

echo "Create main filesystem..."
#cat ${tmp_file}

# build main_initramfs initramfs(cpio.lzma) file that contains full Centec switching system software
if [ "${enable_debian}" == "discard" ]; then
    ${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/main_initramfs.lzma
    cp -rf ${cboot_tmp_file} ${tmp_file}
    rm -rf ${cboot_tmp_file}
fi

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/main_initramfs.lzma
rm ${tmp_file} &
rm -rf ${tmp_dir} &
