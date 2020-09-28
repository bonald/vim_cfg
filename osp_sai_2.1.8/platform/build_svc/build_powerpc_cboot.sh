#!/bin/sh
set -o errexit

product=$PRODUCT
vender=$VENDER
board=$BOARD
rootfs_dir=$BUILD_ROOTFS_DIR
CROSS_COMPILER=$CROSS_COMPILE
etc_dir=$PLAT_TOP_DIR/build_svc/etc
ctc_build_dir=$BLD_DIR
kbuild_dir=$KDIR
top_dir=$TOP_DIR
klish_dir=$TOP_DIR/klish
ver=$VER
version=2.15.so
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
export TMP_FILE=$tmp_file
export TMP_DIR=$tmp_dir
#package init config
package_init_config=""
cp -rf ${etc_dir}/${board}/rc.sysinit ${tmp_dir}
if [ "${enable_debian}" == "y" ]; then
    cboot_tmp_file=$(mktemp)
    package_init_config="
slink   /bin/init /bin/busybox 755 0 0
file    /sbin/init          ${etc_dir}/${board}/debian/cboot.sysinit     755 0 0
file    /switch/etc/rc.sysinit     ${etc_dir}/${board}/debian/cboot_debug.sysinit        755 0 0
file    /etc/inittab        ${etc_dir}/${board}/debian/cboot.inittab     755 0 0
file    /etc/rc.sh          ${tmp_dir}/rc.sysinit               755 0 0"
fi

echo "
dir     /bin        755 0 0
dir     /dev        755 0 0
dir     /etc        755 0 0
dir     /switch     755 0 0
dir     /switch/etc 755 0 0
dir     /switch/etc/cmd 755 0 0
dir     /cmd        755 0 0
dir     /lib        755 0 0
dir     /mnt        755 0 0
dir     /mnt/flash  755 0 0
dir     /proc       755 0 0
dir     /root       755 0 0
dir     /sbin       755 0 0
dir     /sys        755 0 0
dir     /tftpboot   777 0 0
dir     /tmp        755 0 0
dir     /tmp/cmd    755 0 0
dir     /usr        755 0 0
dir     /usr/bin    755 0 0
dir     /usr/sbin   755 0 0
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
dir     /etc/udev   755 0 0
dir     /etc/udev/rules.d   755 0 0
#dir     /mnt/udisk   755 0 0
dir     /mnt/data   755 0 0
#comment by jcao for bug 19380,2012/05/23
#open by Alexander for bug 33970, use crontab to clear file per 10 minutes
slink   /var/log/lastlog /mnt/flash/log/lastlog 755 0 0
slink   /var/log/wtmp /mnt/flash/log/wtmp 755 0 0

slink   /boot /mnt/flash/boot   755 0 0
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
file    /etc/securetty ${pre_compiled_dir}/p1010/etc/securetty 600 0 0
file    /etc/pam.d/other ${pre_compiled_dir}/p1010/etc/pam.d/other 644 0 0
file    /etc/pam.d/clish ${ctc_build_dir}/etc/pam_ctc_conf 644 0 0
slink   /etc/pam.d/sshd /etc/pam.d/clish 644 0 0

file    /lib/ld-${version} ${rootfs_dir}/lib/ld-${version} 755 0 0
slink   /lib/ld.so.1 ld-${version} 755 0 0
file    /lib/libc-${version} ${rootfs_dir}/lib/libc-${version} 755 0 0
slink   /lib/libc.so.6 libc-${version} 755 0 0
file    /lib/libm-${version} ${rootfs_dir}/lib/libm-${version} 755 0 0
slink   /lib/libm.so.6 libm-${version} 755 0 0
file    /lib/librt-${version} ${rootfs_dir}/lib/librt-${version} 755 0 0
slink   /lib/librt.so.1 librt-${version} 755 0 0
file    /lib/libcrypt-${version} ${rootfs_dir}/lib/libcrypt-${version} 755 0 0
slink   /lib/libcrypt.so.1 libcrypt-${version} 755 0 0
file    /lib/libutil-${version} ${rootfs_dir}/lib/libutil-${version} 755 0 0
slink   /lib/libutil.so.1 libutil-${version} 755 0 0
file    /lib/libnsl-${version} ${rootfs_dir}/lib/libnsl-${version} 755 0 0
slink   /lib/libnsl.so.1 libnsl-${version} 755 0 0
file    /lib/libnss_files-${version} ${rootfs_dir}/lib/libnss_files-${version} 755 0 0
slink   /lib/libnss_files.so.2 libnss_files-${version} 755 0 0
file    /lib/libnss_dns-${version} ${rootfs_dir}/lib/libnss_dns-${version} 755 0 0
slink   /lib/libnss_dns.so.2 libnss_dns-${version} 755 0 0
file    /lib/libresolv-${version} ${rootfs_dir}/lib/libresolv-${version} 755 0 0
slink   /lib/libresolv.so.2 libresolv-${version} 755 0 0
file    /lib/libreadline.so.5.2 ${rootfs_dir}/usr/lib/libreadline.so.5.2 755 0 0
slink   /lib/libreadline.so.5 libreadline.so.5.2 755 0 0
file    /lib/libpthread-${version} ${rootfs_dir}/lib/libpthread-${version} 755 0 0
slink   /lib/libpthread.so.0 libpthread-${version} 755 0 0
file    /lib/libthread_db-1.0.so ${rootfs_dir}/lib/libthread_db-1.0.so 755 0 0
slink   /lib/libthread_db.so.1 libthread_db-1.0.so 755 0 0
file    /lib/libdl-${version} ${rootfs_dir}/lib/libdl-${version} 755 0 0
slink   /lib/libdl.so.2 libdl-${version} 755 0 0

file    /lib/libproc-3.2.8.so ${rootfs_dir}/usr/lib/libproc-3.2.8.so 755 0 0

file    /lib/libcrypto.so.1.0.0 ${rootfs_dir}/usr/lib/libcrypto.so.1.0.0 755 0 0
slink   /lib/libcrypto.so libcrypto.so.1.0.0 755 0 0
file    /lib/libssl.so.1.0.0 ${rootfs_dir}/usr/lib/libssl.so.1.0.0 755 0 0
slink   /lib/libssl.so libssl.so.1.0.0 755 0 0

file    /lib/libstdc++.so.6 ${rootfs_dir}/usr/lib/libstdc++.so.6.0.17 755 0 0
slink   /lib/libstdc++.so libstdc++.so.6 755 0 0
file    /lib/libgcc_s.so.1 ${rootfs_dir}/lib/libgcc_s.so.1 755 0 0
slink   /lib/libgcc_s.so libgcc_s.so.1 755 0 0

file    /lib/libz.so.1.2.7 ${rootfs_dir}/usr/lib/libz.so.1.2.7 755 0 0
slink   /lib/libz.so.1 libz.so.1.2.7  755 0 0
slink   /lib/libz.so libz.so.1.2.7  755 0 0

file    /lib/libncurses.so.5.9 ${rootfs_dir}/usr/lib/libncurses.so.5.9 755 0 0
slink   /lib/libncurses.so.5 libncurses.so.5.9 755 0 0
slink   /lib/libncurses.so libncurses.so.5 755 0 0

file    /lib/libpcap.so.1.5.1 ${rootfs_dir}/usr/lib/libpcap.so.1.5.1 755 0 0
slink   /lib/libpcap.so.1 libpcap.so.1.5.1 755 0 0
slink   /lib/libpcap.so libpcap.so.1 755 0 0

file    /lib/libglib-2.0.so.0.3400.3 ${rootfs_dir}/usr/lib/libglib-2.0.so.0.3400.3 755 0 0
slink   /lib/libglib-2.0.so libglib-2.0.so.0.3400.3 755 0 0
slink   /lib/libglib-2.0.so.0 libglib-2.0.so.0.3400.3 755 0 0

file    /lib/libgobject-2.0.so.0.3400.3 ${rootfs_dir}/usr/lib/libgobject-2.0.so.0.3400.3 755 0 0
slink   /lib/libgobject-2.0.so libgobject-2.0.so.0.3400.3 755 0 0
slink   /lib/libgobject-2.0.so.0 libgobject-2.0.so.0.3400.3 755 0 0

file    /lib/libgthread-2.0.so.0.3400.3 ${rootfs_dir}/usr/lib/libgthread-2.0.so.0.3400.3 755 0 0
slink   /lib/libgthread-2.0.so libgthread-2.0.so.0.3400.3 755 0 0
slink   /lib/libgthread-2.0.so.0 libgthread-2.0.so.0.3400.3 755 0 0

file    /lib/libffi.so.6.0.0 ${rootfs_dir}/usr/lib/libffi.so.6.0.0 755 0 0
slink   /lib/libffi.so libffi.so.6.0.0 755 0 0
slink   /lib/libffi.so.6 libffi.so.6.0.0 755 0 0

file    /lib/libcnoslic.a ${pre_compiled_dir}/p1010/lib/libcnoslic.a 755 0 0

file    /bin/busybox $BUSYBOX_DIR/busybox 755 0 0
${package_init_config}
slink   /bin/sh /bin/busybox 755 0 0
slink   /bin/ash /bin/busybox 755 0 0
slink   /bin/login /bin/busybox 755 0 0
slink   /init /sbin/init 755 0 0
slink   /sbin/mount /bin/busybox 755 0 0
slink   /sbin/getty /bin/busybox 755 0 0
slink   /etc/mtab /proc/mounts 755 0 0

#added by xgu for bug 13955
file    /sbin/mii-tool ${rootfs_dir}/${bin_dir}/mii-tool 755 0 0
file    /sbin/ethtool ${rootfs_dir}/${bin_dir}/ethtool 755 0 0
file    /sbin/tcpdump ${rootfs_dir}/${bin_dir}/tcpdump 755 0 0
file    /sbin/strace ${rootfs_dir}/${bin_dir}/strace 755 0 0
#file    /bin/fsck ${rootfs_dir}/bin/fsck 755 0 0
#file    /bin/fsck.msdos ${rootfs_dir}/bin/fsck.msdos 755 0 0
#file    /bin/mkfs ${rootfs_dir}/bin/mkfs 755 0 0
#file    /bin/mkfs.msdos ${rootfs_dir}/bin/mkfs.msdos 755 0 0
file    /bin/ls ${rootfs_dir}/${bin_dir}/ls 755 0 0
file    /bin/ps ${rootfs_dir}/${bin_dir}/ps 755 0 0
file    /bin/netstat ${rootfs_dir}/${bin_dir}/netstat 755 0 0
file    /bin/ping ${rootfs_dir}/${bin_dir}/ping 755 0 0
file    /bin/ping6 ${rootfs_dir}/${bin_dir}/ping6 755 0 0
#file    /usr/bin/traceroute6 ${rootfs_dir}/${bin_dir}/traceroute6 755 0 0

file    /usr/bin/gdbserver ${rootfs_dir}/usr/${bin_dir}/gdbserver 755 0 0
file    /usr/bin/gdb ${rootfs_dir}/usr/${bin_dir}/gdb 755 0 0

file    /usr/bin/core_helper     ${etc_dir}/core_helper     755 0 0
file    /usr/sbin/flash_erase ${rootfs_dir}/${bin_dir}/flash_erase 755 0 0
file    /usr/sbin/flashcp ${rootfs_dir}/${bin_dir}/flashcp 755 0 0
file    /usr/sbin/flash_eraseall ${rootfs_dir}/${bin_dir}/flash_eraseall 755 0 0
file    /usr/sbin/flash_lock ${rootfs_dir}/${bin_dir}/flash_lock 755 0 0
file    /usr/sbin/flash_unlock ${rootfs_dir}/${bin_dir}/flash_unlock 755 0 0
file    /usr/sbin/ubiattach ${rootfs_dir}/${bin_dir}/ubiattach 755 0 0
file    /usr/sbin/ubimkvol  ${rootfs_dir}/${bin_dir}/ubimkvol  755 0 0
file    /usr/sbin/ubiformat ${rootfs_dir}/${bin_dir}/ubiformat 755 0 0
file    /usr/sbin/ubinfo ${rootfs_dir}/${bin_dir}/ubinfo 755 0 0
file    /usr/sbin/ubidetach ${rootfs_dir}/${bin_dir}/ubidetach 755 0 0
#file    /usr/sbin/stressapptest  ${rootfs_dir}/${bin_dir}/stressapptest  755 0 0
file    /usr/sbin/udevd ${rootfs_dir}/${bin_dir}/udevd 755 0 0
file    /usr/sbin/udevstart ${rootfs_dir}/${bin_dir}/udevstart 755 0 0
file    /usr/sbin/lspci ${rootfs_dir}/${bin_dir}/lspci 755 0 0
file    /usr/sbin/setpci ${rootfs_dir}/${bin_dir}/setpci 755 0 0
file    /usr/sbin/startup_monitor ${etc_dir}/${board}/startup_monitor 755 0 0
file    /usr/sbin/tftpd ${rootfs_dir}/${bin_dir}/tftpd 755 0 0

file    /etc/localtime      ${etc_dir}/UTC                      644 0 0
file    /etc/termcap        ${etc_dir}/termcap                  644 0 0
file    /etc/passwd         ${etc_dir}/passwd                   644 0 0
file    /etc/group          ${etc_dir}/group                    644 0 0
file    /etc/profile        ${etc_dir}/profile                  644 0 0
file    /etc/services       ${etc_dir}/services                 644 0 0
file    /etc/protocols       ${etc_dir}/protocols               644 0 0
file    /etc/hosts          ${etc_dir}/hosts                    644 0 0
file    /switch/etc/resolv.conf    ${etc_dir}/resolv.conf              644 0 0
file    /etc/nsswitch.conf  ${etc_dir}/nsswitch.conf            644 0 0
file    /etc/cf_hotplug     ${etc_dir}/cf_hotplug               755 0 0
file    /etc/memory_detect.sh ${etc_dir}/memory_detect.sh    755 0 0
file    /etc/udev/udev.conf      ${etc_dir}/udev/udev.conf                755 0 0
file    /etc/udev/rules.d/udev.conf      ${etc_dir}/udev/rules.d/udev.conf                755 0 0
file    /etc/udev/rules.d/mount.rules      ${etc_dir}/udev/rules.d/mount.rules                755 0 0
file    /etc/udev/rules.d/unmount.rules      ${etc_dir}/udev/rules.d/unmount.rules                755 0 0
file    /etc/mount.sh     ${etc_dir}/mount.sh               755 0 0
file    /etc/unmount.sh     ${etc_dir}/unmount.sh               755 0 0
file    /etc/format_flash.sh     ${etc_dir}/format_flash.sh              755 0 0
file    /usr/sbin/restart.sh ${etc_dir}/restart.sh    755 0 0
file    /switch/etc/init-ns.sh ${etc_dir}/init-ns.sh    755 0 0
file    /switch/etc/init-veth.sh ${etc_dir}/init-veth.sh    755 0 0
file    /etc/login.sh ${etc_dir}/login.sh    755 0 0
file    /etc/fw_env.config     ${etc_dir}/fw_env.config               755 0 0
file    /usr/share/zoneinfo/zone.tab ${PLAT_TOP_DIR}/build_svc/zone.tab 644 0 0
file    /usr/share/zoneinfo/iso3166.tab ${PLAT_TOP_DIR}/build_svc/iso3166.tab 644 0 0
" > ${tmp_file}

echo "Create CBOOT System Image..."

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/initramfs.lzma
rm ${tmp_file} &
rm -rf ${tmp_dir} &
