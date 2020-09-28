#!/bin/bash

tar -czf archive.tar.gz debian_powerpc_jessie.squash inittab fstab busybox fw_env.config get_and_unpack_image.sh switch-trigger start_klish_for_console.sh switch_sudo_user 80-mount-udisk.rules 90-unmount-udisk.rules mount.sh unmount.sh udhcpc_def.script nsswitch.conf
cat installer.sh archive.tar.gz > debian-rootfs-installer
chmod +x debian-rootfs-installer
