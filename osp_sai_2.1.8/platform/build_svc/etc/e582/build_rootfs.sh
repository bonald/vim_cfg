#!/bin/bash

ADDITIONAL_DEBIAN_PACKAGES="expect,libreadline5,sqlite3,psmisc,python-mysqldb,python-sqlite,python-webob,curl,openssl,ntp"
EXTRA_DEBIAN_PACKAGES="pciutils,gcc,gdb,make,binutils,iptables,i2c-tools,strace,parted,e2fsprogs,grub-efi-amd64,bzip2,libglib2.0-dev,lzma,libc6-dev,patch,libncurses5-dev,virtualenv,libpam0g-dev,ethtool,gdbserver,lsof"
BASE_DEBIAN_PACKAGES="sysvinit-core,sysvinit-utils,kmod,locales,udev,aptitude,dialog,ifupdown,procps,iproute,iputils-ping,pump,nano,wget,netbase,telnet,net-tools,python,python-dev,vim,ssh,file,tftp,ftp,tcpdump,sudo,ftpd"

debootstrap --foreign --arch amd64 --include=${BASE_DEBIAN_PACKAGES},${ADDITIONAL_DEBIAN_PACKAGES},${EXTRA_DEBIAN_PACKAGES} jessie /root/debian_rootfs

chroot /root/debian_rootfs /debootstrap/debootstrap --second-stage

chroot /root/debian_rootfs apt-get clean

(echo "!@#";sleep 1;echo "!@#") | chroot /root/debian_rootfs passwd root

chroot /root/debian_rootfs userdel switch
chroot /root/debian_rootfs rm -rf /home/switch
chroot /root/debian_rootfs useradd -u 1000 -m -U -d /home/switch -s /bin/bash switch
(echo "!@#";sleep 1;echo "!@#") | chroot /root/debian_rootfs passwd switch

chroot /root/debian_rootfs hostname switch
chroot /root/debian_rootfs /bin/bash -c "echo switch > /etc/hostname"

chroot /root/debian_rootfs virtualenv -p python2 /usr/local/ctc_python
