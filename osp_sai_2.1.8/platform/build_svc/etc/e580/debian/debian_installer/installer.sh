#!/bin/sh

umask 0022

debian_rootfs_parent_dir=/mnt/flash/debian
debian_rootfs_dir=.rootfs_dir

# decompress archive
ARCHIVE=`awk '/^__ARCHIVE_BELOW__/ {print NR + 1; exit 0; }' $0`
tail -n+$ARCHIVE $0 | tar -xzf - -C /tmp 2>/dev/null

# do some things
rm $debian_rootfs_parent_dir -rf
mkdir -p $debian_rootfs_parent_dir
cp /tmp/debian_powerpc_jessie.squash $debian_rootfs_parent_dir
mkdir -p $debian_rootfs_parent_dir/.xino
mkdir -p $debian_rootfs_parent_dir/.overlay_ro
mkdir -p $debian_rootfs_parent_dir/.overlay_rw
mkdir -p $debian_rootfs_parent_dir/$debian_rootfs_dir
losetup /dev/loop0 $debian_rootfs_parent_dir/debian_powerpc_jessie.squash
sleep 2
mount /dev/loop0 $debian_rootfs_parent_dir/.overlay_ro
sleep 1
mount -t tmpfs none $debian_rootfs_parent_dir/.xino
sleep 1
mount -t aufs -o br=$debian_rootfs_parent_dir/.overlay_rw:$debian_rootfs_parent_dir/.overlay_ro=ro,xino=$debian_rootfs_parent_dir/.xino/.aufs.xino,sync none $debian_rootfs_parent_dir/$debian_rootfs_dir
sleep 1



mv /tmp/inittab $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/inittab
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/inittab
chmod 0640 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/inittab

mv /tmp/fstab $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/fstab
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/fstab
chmod 0640 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/fstab

mv /tmp/nsswitch.conf $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/nsswitch.conf
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/nsswitch.conf
chmod 0640 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/nsswitch.conf

mkdir -p $debian_rootfs_parent_dir/$debian_rootfs_dir/mnt/flash/boot
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/mnt/flash
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/mnt/flash/boot
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/mnt/flash
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/mnt/flash/boot

mkdir -p $debian_rootfs_parent_dir/$debian_rootfs_dir/switch
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/switch
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/switch

mv /tmp/fw_env.config $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/fw_env.config
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/fw_env.config
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/fw_env.config

mv /tmp/get_and_unpack_image.sh $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/get_and_unpack_image.sh
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/get_and_unpack_image.sh
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/get_and_unpack_image.sh

mv /tmp/start_klish_for_console.sh $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/start_klish_for_console.sh
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/start_klish_for_console.sh
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/start_klish_for_console.sh

mv /tmp/switch_sudo_user $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/sudoers.d/
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/sudoers.d/switch_sudo_user
chmod 0640 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/sudoers.d/switch_sudo_user



mv /tmp/80-mount-udisk.rules $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/udev/rules.d/
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/udev/rules.d/80-mount-udisk.rules
chmod 0640 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/udev/rules.d/80-mount-udisk.rules
mv /tmp/90-unmount-udisk.rules $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/udev/rules.d/
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/udev/rules.d/90-unmount-udisk.rules
chmod 0640 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/udev/rules.d/90-unmount-udisk.rules
mv /tmp/mount.sh $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/mount.sh
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/mount.sh
mv /tmp/unmount.sh $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/unmount.sh
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/unmount.sh



mv /tmp/busybox $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/busybox
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/busybox
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/busybox
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/onie-syseeprom
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/fw_printenv
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/fw_setenv
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/hexdump
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/crond
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/udhcpc
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/telnetd
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/usleep
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/env
mv /tmp/udhcpc_def.script $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/
chown root:root $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/udhcpc_def.script
chmod 0755 $debian_rootfs_parent_dir/$debian_rootfs_dir/bin/udhcpc_def.script
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir rm /bin/sh -rf
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir ln -s /bin/busybox /bin/sh
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir rm -rf /sbin/shutdown
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir echo "alias halt='sudo halt -f'" >> $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/profile
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir echo "alias reboot='sudo reboot -f'" >> $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/profile

echo "127.0.0.1 `cat $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/hostname`" >> $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/hosts

mknod $debian_rootfs_parent_dir/$debian_rootfs_dir/dev/console c 5 1

chroot $debian_rootfs_parent_dir/$debian_rootfs_dir insserv -r /etc/init.d/ssh
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir insserv -r /etc/init.d/ntp
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir apt-get -y remove ntp 1>/dev/null 2>&1

mv /tmp/switch-trigger $debian_rootfs_parent_dir/$debian_rootfs_dir/etc/init.d/switch-trigger
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir chown root:root /etc/init.d/switch-trigger
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir chmod 0755 /etc/init.d/switch-trigger
chroot $debian_rootfs_parent_dir/$debian_rootfs_dir insserv -v -d /etc/init.d/switch-trigger

touch /tmp/glb_install_debian_done

sync
sync
sync

exit 0
# image is after here
__ARCHIVE_BELOW__
