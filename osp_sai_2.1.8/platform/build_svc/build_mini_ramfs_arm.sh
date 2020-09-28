#!/bin/sh
set -o errexit

board=$BOARD
rootfs_dir=$BUILD_ROOTFS_DIR
CROSS_COMPILER=$CROSS_COMPILE
etc_dir=$PLAT_TOP_DIR/build_svc/etc
kbuild_dir=$KDIR
top_dir=$TOP_DIR
ver=$VER
version=2.11.1.so
bin_dir=bin
tmp_file=$(mktemp)

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

dir     /mnt        755 0 0
dir     /mnt/flash  755 0 0
dir     /mnt/udisk  755 0 0

slink   /boot /mnt/flash/boot   755 0 0

file    /bin/busybox $BUSYBOX_DIR/busybox 755 0 0
slink   /init /bin/busybox 755 0 0
slink   /bin/sh /bin/busybox 755 0 0
slink   /bin/ash /bin/busybox 755 0 0
slink   /bin/login /bin/busybox 755 0 0
slink   /sbin/init /bin/busybox 755 0 0
slink   /init /sbin/init 755 0 0
slink   /sbin/mount /bin/busybox 755 0 0
slink   /sbin/getty /bin/busybox 755 0 0
slink   /etc/mtab /proc/mounts 755 0 0

file    /usr/sbin/flash_erase ${rootfs_dir}/usr/${bin_dir}/flash_erase 755 0 0
file    /usr/sbin/flashcp ${rootfs_dir}/usr/${bin_dir}/flashcp 755 0 0

file    /etc/localtime      ${etc_dir}/UTC                      644 0 0
file    /etc/inittab    ${etc_dir}/${board}/inittab.hi3535              644 0 0
file    /etc/inittab_lc    ${etc_dir}/${board}/inittab.hi3535              644 0 0
file    /etc/inittab_sup    ${etc_dir}/${board}/inittab.hi3535              644 0 0
file    /etc/passwd         ${etc_dir}/passwd                   644 0 0
file    /etc/group          ${etc_dir}/group                    644 0 0
file    /etc/hosts          ${etc_dir}/hosts                    644 0 0
file    /etc/udev/udev.conf      ${etc_dir}/udev/udev.conf                755 0 0
file    /etc/udev/rules.d/udev.conf      ${etc_dir}/udev/rules.d/udev.conf                755 0 0
file    /etc/udev/rules.d/mount.rules      ${etc_dir}/udev/rules.d/mount.rules                755 0 0
file    /etc/udev/rules.d/unmount.rules      ${etc_dir}/udev/rules.d/unmount.rules                755 0 0
file    /etc/mount.sh     ${etc_dir}/mount_hi3535.sh               755 0 0
file    /etc/unmount.sh     ${etc_dir}/unmount.sh               755 0 0
file    /etc/init.d/rcS     ${etc_dir}/${board}/rcS.hi3535               755 0 0
file    /etc/installOS.sh     ${etc_dir}/installOS.sh               755 0 0

file    /lib/ld-${version} ${rootfs_dir}/lib/ld-${version} 755 0 0
slink   /lib/ld-linux.so.3 ld-${version} 755 0 0
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
file    /lib/libreadline.so.6.2 ${rootfs_dir}/usr/lib/libreadline.so.6.2 755 0 0
slink   /lib/libreadline.so.6 libreadline.so.6.2 755 0 0
file    /lib/libpthread-${version} ${rootfs_dir}/lib/libpthread-${version} 755 0 0
slink   /lib/libpthread.so.0 libpthread-${version} 755 0 0
file    /lib/libthread_db-1.0.so ${rootfs_dir}/lib/libthread_db-1.0.so 755 0 0
slink   /lib/libthread_db.so.1 libthread_db-1.0.so 755 0 0
file    /lib/libdl-${version} ${rootfs_dir}/lib/libdl-${version} 755 0 0
slink   /lib/libdl.so.2 libdl-${version} 755 0 0

file    /usr/sbin/udevd ${rootfs_dir}/usr/${bin_dir}/udevd 755 0 0
file    /usr/sbin/udevstart ${rootfs_dir}/usr/${bin_dir}/udevstart 755 0 0
file    /usr/bin/mkfs.ext4  ${rootfs_dir}/usr/${bin_dir}/mkfs.ext4 755 0 0
file    /usr/bin/e2fsck  ${rootfs_dir}/usr/${bin_dir}/e2fsck 755 0 0
file    /usr/bin/tune2fs  ${rootfs_dir}/usr/${bin_dir}/tune2fs 755 0 0
" > ${tmp_file}

echo "Create mini initramfs..."

${kbuild_dir}/usr/gen_init_cpio ${tmp_file} | lzma -f -9 - > ${kbuild_dir}/mini_initramfs.lzma
rm ${tmp_file}
