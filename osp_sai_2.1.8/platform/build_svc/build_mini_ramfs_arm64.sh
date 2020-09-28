#!/bin/sh
#set -o errexit

board=$BOARD
rootfs_dir=$BUILD_ROOTFS_DIR
rootfs_lib_dir=aarch64-linux-gnu/libc
CROSS_COMPILER=$CROSS_COMPILE
etc_dir=$PLAT_TOP_DIR/build_svc/etc
kbuild_dir=$KDIR
top_dir=$TOP_DIR
ver=$VER
version=2.23.so
bin_dir=bin
mkdir -m 777 -p /dev/shm/software
mkdir -p /dev/shm/software/$USER
tmp_file=$(mktemp -p /dev/shm/software/$USER/)
tmp_dir=$(mktemp -p /dev/shm/software/$USER/ -d)
enable_debian=${ENABLE_DEBIAN_LINUX}
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

echo "
dir /root 755 0 0
dir /boot 755 0 0
dir /lib 755 0 0
dir /dev 755 0 0
dir /etc 755 0 0
dir /etc/init.d 755 0 0
dir /etc/udev 755 0 0
dir /etc/udev/rules.d 755 0 0
dir /bin 755 0 0
dir /sbin 755 0 0
dir /proc 755 0 0
dir /sys  755 0 0
dir /tmp  755 0 0
dir     /usr        755 0 0
dir     /usr/bin    755 0 0
dir     /usr/sbin   755 0 0
dir     /run   755 0 0

dir     /mnt        755 0 0
dir     /mnt/flash  755 0 0
dir     /mnt/flash/boot  755 0 0
dir     /mnt/udisk  755 0 0

slink   /boot /mnt/flash/boot   755 0 0

file    /bin/busybox $BUSYBOX_DIR/busybox 755 0 0
slink   /bin/sh /bin/busybox 755 0 0
slink   /bin/ash /bin/busybox 755 0 0
slink   /bin/login /bin/busybox 755 0 0
slink   /sbin/mount /bin/busybox 755 0 0
slink   /sbin/getty /bin/busybox 755 0 0
slink   /etc/mtab /proc/mounts 755 0 0

slink 	/usr/bin/[ /bin/busybox 755 0 0
slink 	/usr/bin/[[ /bin/busybox 755 0 0
slink 	/bin/addgroup /bin/busybox 755 0 0
slink 	/bin/adduser /bin/busybox 755 0 0
slink 	/sbin/adjtimex /bin/busybox 755 0 0
slink 	/sbin/arp /bin/busybox 755 0 0
slink 	/usr/bin/arping /bin/busybox 755 0 0
slink 	/bin/ash /bin/busybox 755 0 0
slink 	/usr/bin/awk /bin/busybox 755 0 0
slink 	/sbin/blkid /bin/busybox 755 0 0
slink 	/usr/bin/basename /bin/busybox 755 0 0
slink 	/bin/cat /bin/busybox 755 0 0
slink 	/bin/chgrp /bin/busybox 755 0 0
slink 	/bin/chmod /bin/busybox 755 0 0
slink 	/bin/chown /bin/busybox 755 0 0
slink 	/usr/sbin/chroot /bin/busybox 755 0 0
slink 	/bin/cp /bin/busybox 755 0 0
slink 	/bin/cpio /bin/busybox 755 0 0
slink 	/usr/sbin/crond /bin/busybox 755 0 0
slink 	/usr/bin/crontab /bin/busybox 755 0 0
slink 	/usr/bin/cryptpw /bin/busybox 755 0 0
slink 	/usr/bin/cut /bin/busybox 755 0 0
slink 	/bin/date /bin/busybox 755 0 0
slink 	/bin/dd /bin/busybox 755 0 0
slink 	/bin/delgroup /bin/busybox 755 0 0
slink 	/bin/deluser /bin/busybox 755 0 0
slink 	/usr/bin/diff /bin/busybox 755 0 0
slink 	/usr/bin/dirname /bin/busybox 755 0 0
slink 	/bin/dmesg /bin/busybox 755 0 0
slink 	/usr/bin/du /bin/busybox 755 0 0
slink 	/bin/echo /bin/busybox 755 0 0
slink 	/bin/egrep /bin/busybox 755 0 0
slink 	/usr/bin/env /bin/busybox 755 0 0
slink 	/usr/bin/expr /bin/busybox 755 0 0
slink 	/bin/false /bin/busybox 755 0 0
slink 	/sbin/fdisk /bin/busybox 755 0 0
slink 	/bin/fgrep /bin/busybox 755 0 0
slink 	/usr/bin/find /bin/busybox 755 0 0
slink 	/usr/bin/free /bin/busybox 755 0 0
slink 	/usr/bin/fuser /bin/busybox 755 0 0
#slink 	/usr/sbin/fw_printenv /bin/busybox 755 0 0
#slink 	/usr/sbin/fw_setenv /bin/busybox 755 0 0
slink 	/bin/getopt /bin/busybox 755 0 0
slink 	/sbin/getty /bin/busybox 755 0 0
slink 	/bin/grep /bin/busybox 755 0 0
slink 	/bin/gunzip /bin/busybox 755 0 0
slink 	/bin/gzip /bin/busybox 755 0 0
slink 	/sbin/halt /bin/busybox 755 0 0
slink 	/sbin/hdparm /bin/busybox 755 0 0
slink 	/usr/bin/hexdump /bin/busybox 755 0 0
slink 	/usr/bin/hostid /bin/busybox 755 0 0
slink 	/bin/hostname /bin/busybox 755 0 0
slink 	/sbin/hwclock /bin/busybox 755 0 0
slink 	/usr/bin/id /bin/busybox 755 0 0
slink 	/sbin/ifconfig /bin/busybox 755 0 0
slink 	/sbin/insmod /bin/busybox 755 0 0
slink 	/bin/ipaddr /bin/busybox 755 0 0
slink 	/usr/bin/ipcrm /bin/busybox 755 0 0
slink 	/usr/bin/ipcs /bin/busybox 755 0 0
slink 	/bin/iplink /bin/busybox 755 0 0
slink 	/bin/iproute /bin/busybox 755 0 0
slink 	/bin/iprule /bin/busybox 755 0 0
slink 	/bin/iptunnel /bin/busybox 755 0 0
slink 	/bin/kill /bin/busybox 755 0 0
slink 	/usr/bin/killall /bin/busybox 755 0 0
slink 	/usr/bin/killall5 /bin/busybox 755 0 0
slink 	/sbin/klogd /bin/busybox 755 0 0
slink 	/usr/bin/last /bin/busybox 755 0 0
slink 	/usr/bin/length /bin/busybox 755 0 0
slink 	/usr/bin/less /bin/busybox 755 0 0
slink 	/bin/ln /bin/busybox 755 0 0
slink 	/usr/bin/logger /bin/busybox 755 0 0
slink 	/bin/login /bin/busybox 755 0 0
slink 	/usr/bin/logname /bin/busybox 755 0 0
slink 	/sbin/logread /bin/busybox 755 0 0
slink 	/sbin/lsmod /bin/busybox 755 0 0
slink 	/sbin/lsusb /bin/busybox 755 0 0
slink 	/sbin/lspci /bin/busybox 755 0 0
slink 	/usr/bin/lzmacat /bin/busybox 755 0 0
slink 	/usr/bin/md5sum /bin/busybox 755 0 0
slink 	/sbin/mdev /bin/busybox 755 0 0
slink 	/usr/bin/mesg /bin/busybox 755 0 0
slink 	/bin/mkdir /bin/busybox 755 0 0
slink 	/usr/bin/mkfifo /bin/busybox 755 0 0
slink 	/bin/mknod /bin/busybox 755 0 0
slink 	/bin/more /bin/busybox 755 0 0
slink 	/bin/mount /bin/busybox 755 0 0
slink 	/bin/mv /bin/busybox 755 0 0
slink 	/bin/pgrep /bin/busybox 755 0 0
slink 	/bin/netstat /bin/busybox 755 0 0
slink 	/bin/nice /bin/busybox 755 0 0
slink 	/usr/bin/nohup /bin/busybox 755 0 0
slink 	/usr/bin/nslookup /bin/busybox 755 0 0
slink 	/usr/bin/onie-syseeprom /bin/busybox 755 0 0
slink 	/usr/bin/passwd /bin/busybox 755 0 0
slink 	/bin/pidof /bin/busybox 755 0 0
#slink 	/bin/ping /bin/busybox 755 0 0
#slink 	/bin/ping6 /bin/busybox 755 0 0
slink 	/sbin/poweroff /bin/busybox 755 0 0
slink 	/bin/printenv /bin/busybox 755 0 0
slink 	/usr/bin/printf /bin/busybox 755 0 0
slink 	/bin/ps /bin/busybox 755 0 0
slink 	/bin/pwd /bin/busybox 755 0 0
slink 	/sbin/reboot /bin/busybox 755 0 0
slink 	/usr/bin/renice /bin/busybox 755 0 0
slink 	/bin/rm /bin/busybox 755 0 0
slink 	/bin/rmdir /bin/busybox 755 0 0
slink 	/sbin/rmmod /bin/busybox 755 0 0
slink 	/sbin/route /bin/busybox 755 0 0
slink 	/usr/bin/rx /bin/busybox 755 0 0
slink 	/bin/sed /bin/busybox 755 0 0
slink 	/bin/sh /bin/busybox 755 0 0
slink 	/bin/sleep /bin/busybox 755 0 0
slink 	/bin/stat /bin/busybox 755 0 0
slink 	/bin/stty /bin/busybox 755 0 0
slink 	/bin/su /bin/busybox 755 0 0
slink 	/sbin/sulogin /bin/busybox 755 0 0
slink 	/sbin/switch_root /bin/busybox 755 0 0
slink 	/bin/sync /bin/busybox 755 0 0
slink 	/sbin/sysctl /bin/busybox 755 0 0
slink 	/sbin/syslogd /bin/busybox 755 0 0
slink 	/bin/tar /bin/busybox 755 0 0
slink 	/usr/bin/tee /bin/busybox 755 0 0
slink 	/usr/bin/telnet /bin/busybox 755 0 0
slink 	/usr/sbin/telnetd /bin/busybox 755 0 0
slink 	/usr/bin/test /bin/busybox 755 0 0
slink 	/usr/bin/tftp /bin/busybox 755 0 0
slink 	/usr/bin/top /bin/busybox 755 0 0
slink 	/bin/touch /bin/busybox 755 0 0
slink 	/usr/bin/traceroute /bin/busybox 755 0 0
slink 	/bin/true /bin/busybox 755 0 0
slink 	/usr/bin/tty /bin/busybox 755 0 0
slink 	/bin/umount /bin/busybox 755 0 0
slink 	/bin/uname /bin/busybox 755 0 0
slink 	/usr/bin/unlzma /bin/busybox 755 0 0
slink 	/usr/bin/uptime /bin/busybox 755 0 0
slink 	/bin/vi /bin/busybox 755 0 0
slink 	/usr/bin/wget /bin/busybox 755 0 0
slink 	/usr/bin/who /bin/busybox 755 0 0
slink 	/usr/bin/whoami /bin/busybox 755 0 0
slink 	/usr/bin/xargs /bin/busybox 755 0 0
slink 	/usr/bin/yes /bin/busybox 755 0 0
slink 	/bin/zcat /bin/busybox 755 0 0



#file    /usr/sbin/flash_erase ${rootfs_dir}/usr/${bin_dir}/flash_erase 755 0 0
file    /usr/sbin/flashcp ${rootfs_dir}/usr/${bin_dir}/flashcp 755 0 0
file    /bin/ls ${rootfs_dir}/usr/${bin_dir}/ls 755 0 0

file    /etc/localtime      ${etc_dir}/UTC                      644 0 0
file    /etc/passwd         ${etc_dir}/passwd                   644 0 0
file    /etc/group          ${etc_dir}/group                    644 0 0
file    /etc/hosts          ${etc_dir}/hosts                    644 0 0
file    /etc/udev/udev.conf      ${etc_dir}/udev/udev.conf                755 0 0
file    /etc/udev/rules.d/udev.conf      ${etc_dir}/udev/rules.d/udev.conf                755 0 0
file    /etc/udev/rules.d/mount.rules      ${etc_dir}/udev/rules.d/mount.rules                755 0 0
file    /etc/udev/rules.d/unmount.rules      ${etc_dir}/udev/rules.d/unmount.rules                755 0 0
file    /etc/mount.sh     ${etc_dir}/mount_arm.sh               755 0 0
file    /etc/unmount.sh     ${etc_dir}/unmount.sh               755 0 0
file    /etc/installOS.sh     ${etc_dir}/installOS.sh               755 0 0
file    /etc/fstab     ${etc_dir}/${board}/fstab               755 0 0

file    /lib/ld-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/ld-${version} 755 0 0
slink   /lib/ld-linux-aarch64.so.1 ld-2.23.so 755 0 0
file    /lib/libc-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libc-${version} 755 0 0
slink   /lib/libc.so.6 libc-${version} 755 0 0
file    /lib/libm-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libm-${version} 755 0 0
slink   /lib/libm.so.6 libm-${version} 755 0 0
file    /lib/librt-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/librt-${version} 755 0 0
slink   /lib/librt.so.1 librt-${version} 755 0 0
file    /lib/libcrypt-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libcrypt-${version} 755 0 0
slink   /lib/libcrypt.so.1 libcrypt-${version} 755 0 0
file    /lib/libutil-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libutil-${version} 755 0 0
slink   /lib/libutil.so.1 libutil-${version} 755 0 0
file    /lib/libnsl-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libnsl-${version} 755 0 0
slink   /lib/libnsl.so.1 libnsl-${version} 755 0 0
file    /lib/libnss_files-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libnss_files-${version} 755 0 0
slink   /lib/libnss_files.so.2 libnss_files-${version} 755 0 0
file    /lib/libnss_dns-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libnss_dns-${version} 755 0 0
slink   /lib/libnss_dns.so.2 libnss_dns-${version} 755 0 0
file    /lib/libresolv-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libresolv-${version} 755 0 0
slink   /lib/libresolv.so.2 libresolv-${version} 755 0 0
file    /lib/libreadline.so.6.3 ${rootfs_dir}/usr/lib/libreadline.so.6.3 755 0 0
slink   /lib/libreadline.so.6 libreadline.so.6.3 755 0 0
file    /lib/libpthread-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libpthread-${version} 755 0 0
slink   /lib/libpthread.so.0 libpthread-${version} 755 0 0
file    /lib/libthread_db-1.0.so ${rootfs_dir}/${rootfs_lib_dir}/lib/libthread_db-1.0.so 755 0 0
slink   /lib/libthread_db.so.1 libthread_db-1.0.so 755 0 0
file    /lib/libdl-${version} ${rootfs_dir}/${rootfs_lib_dir}/lib/libdl-${version} 755 0 0
slink   /lib/libdl.so.2 libdl-${version} 755 0 0

file    /usr/sbin/udevd ${etc_dir}/udevd_arm64 755 0 0
file    /usr/sbin/udevstart ${etc_dir}/udevstart_arm64 755 0 0
file    /usr/bin/mkfs.ext4  ${etc_dir}/mkfs.ext4.arm64 755 0 0
file    /usr/bin/e2fsck  ${etc_dir}/e2fsck.arm64 755 0 0
file    /usr/bin/tune2fs  ${etc_dir}/tune2fs.arm64 755 0 0
file    /mainramfs.lzma  ${kbuild_dir}/main_initramfs.lzma 644 0 0

dir     /var     755 0 0
dir     /var/lock     755 0 0
file    /usr/sbin/fw_printenv ${etc_dir}/fw_printenv.ls1023a 755 0 0
slink   /usr/sbin/fw_setenv             fw_printenv         755 0 0
file    /etc/fw_env.config     ${etc_dir}/fw_env.config.${cpu}               755 0 0
" > ${tmp_file}

if [ "${enable_debian}" != "y" ]; then
echo "
slink   /sbin/init /bin/busybox 755 0 0
slink   /init /sbin/init 755 0 0
slink   /bin/init /bin/busybox 755 0 0
file    /etc/inittab    ${etc_dir}/${board}/inittab.${cpu}              644 0 0
file    /etc/inittab_lc    ${etc_dir}/${board}/inittab.${cpu}              644 0 0
file    /etc/inittab_sup    ${etc_dir}/${board}/inittab.${cpu}              644 0 0
file    /etc/init.d/rcS     ${etc_dir}/${board}/rcS.${cpu}               755 0 0
" >> ${tmp_file}
else
echo "
file    /sbin/init ${etc_dir}/${board}/debian/init.debian 755 0 0
file    /init ${etc_dir}/${board}/debian/init.debian 755 0 0
slink   /bin/init /bin/busybox 755 0 0
file    /etc/inittab    ${etc_dir}/${board}/debian/inittab.rollback              644 0 0
file    /etc/inittab_lc    ${etc_dir}/${board}/debian/inittab.rollback              644 0 0
file    /etc/inittab_sup    ${etc_dir}/${board}/debian/inittab.rollback              644 0 0
file    /etc/init.d/rcS     ${etc_dir}/${board}/debian/rcS.rollback               755 0 0
" >> ${tmp_file}
fi

echo "Create mini initramfs..."

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/mini_initramfs.lzma
rm ${tmp_file}
rm -rf ${tmp_dir}
