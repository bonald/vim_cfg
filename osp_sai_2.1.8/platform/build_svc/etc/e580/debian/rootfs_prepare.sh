#!/bin/sh

########################################################################
# Copyright (C) 2004-2015 Centec Networks. All rights reserved.
# rootfs prepare script
########################################################################

####################
# Global configures
INITRAMFS="initramfs.lzma"

####################
# start
PROG=`basename $0`
OVERRIDE_DIR=''

print_usage_and_exit()
{
    echo "Usage: ./rootfs_prepare.sh [OPTIONS] ROOTFS_DIR"
    echo ""
    echo "Available options are listed: " 
    echo "       ROOTFS_DIR           rootfs directory"
    echo "       -o, --override-dir   source directories used to override in directory: ROOTFS_DIR"
    echo "       -i, --initramfs      initramfs file in cpio.lzma format"
    echo "       -h, --help           display this help and exit"
    exit 1
}

ARGS=`getopt  --long help,override-dir:,initramfs: --options h,o:,i: --name $PROG -- "$@"`
eval set -- "$ARGS"
while true; do
    case "$1" in
        -h | --help)
            print_usage_and_exit ;;
        -o | --override-dir)
            OVERRIDE_DIR=$2; shift 2;;
        -i | --initramfs)
            INITRAMFS=$2; shift 2;;
        --)
            shift; break;; # end of options
        *)
            print_usage_and_exit ;;
    esac
done

if [ $# -lt 1 ]; then
    echo -e "You must provide mandatory argument: ROOTFS_DIR\n"
    exit 1
fi

echo "Running rootfs prepare script: ${PROG}"
ROOTFS_DIR=$1

echo "ROOTFS_DIR: ${ROOTFS_DIR}"
echo "OVERRIDE_DIR: ${OVERRIDE_DIR}"
echo "INITRAMFS: ${INITRAMFS}"

CURRENT_DIR=`pwd`

# 1. copy directories to override the directories with same path under rootfs
if [ "${OVERRIDE_DIR}" != "" ];then
    if [ -d "${OVERRIDE_DIR}" ]; then
        COPY_OVERRIDE_DIR_CMD="cp -rf ${OVERRIDE_DIR}/* ${ROOTFS_DIR}/rootfs/"
        echo "${COPY_OVERRIDE_DIR_CMD}"
        ${COPY_OVERRIDE_DIR_CMD}
    else
        echo "Unable to retrieve the directory: ${OVERRIDE_DIR}, please check if you have built the system correctly"
        exit 1    
    fi
fi

# 2. unpack initramfs.lzma into rootfs
if [ "${INITRAMFS}" != "" ];then
    if [ ! -f "${INITRAMFS}" ]; then
        echo "Unable to retrieve the initramfs file: ${INITRAMFS}, please check if you have built the system correctly"
        exit 1
    fi

    # create directory: '/switch' under rootfs
    mkdir -p ${ROOTFS_DIR}/rootfs/switch
    
    # copy cpio.lzma file into '/switch' under rootfs
    cp -rf  ${INITRAMFS} ${ROOTFS_DIR}/rootfs/switch/initramfs.lzma

    # unpack cpio.lzma
    cd ${ROOTFS_DIR}/rootfs/switch
    lzma -d ${ROOTFS_DIR}/rootfs/switch/initramfs.lzma
    cpio -i < initramfs >/dev/null 2>&1

    rm -rf ${ROOTFS_DIR}/rootfs/switch/initramfs.lzma
    rm -rf ${ROOTFS_DIR}/rootfs/switch/initramfs
    cd ${CURRENT_DIR}
fi

# 3. copy extra files into debian rootfs if necessary
SYNC_EXCLUDE_LIST="rc.sysinit"
mkdir -p ${ROOTFS_DIR}/rootfs/cfg >/dev/null 2>&1
mkdir -p ${ROOTFS_DIR}/rootfs/conf/ssh >/dev/null 2>&1
mkdir -p ${ROOTFS_DIR}/rootfs/.overlay_ro >/dev/null 2>&1
mkdir -p ${ROOTFS_DIR}/rootfs/var/run/netns/ >/dev/null 2>&1
rsync --ignore-existing -r ${ROOTFS_DIR}/rootfs/switch/etc/ ${ROOTFS_DIR}/rootfs/etc/ --exclude=${SYNC_EXCLUDE_LIST}

# 3.1 copy /lib/security/* into debian rootfs
mkdir -p ${ROOTFS_DIR}/rootfs/lib/security
rsync --ignore-existing -r ${ROOTFS_DIR}/rootfs/switch/lib/security/ ${ROOTFS_DIR}/rootfs/lib/security/

# 4. write extra swi image metadata into the file: /etc/swi-version
SWI_VERSION_FILE=${ROOTFS_DIR}/rootfs/etc/swi-version
SWI_VERSION=v${version_num}
SWI_BUILD_DATE=`date`
SWI_BUILD_HOST=`hostname`
rm -rf ${SWI_VERSION_FILE}
echo "SWI_VERSION=${SWI_VERSION}" >> ${SWI_VERSION_FILE}
echo "SWI_RELEASE=n/a" >> ${SWI_VERSION_FILE}
echo "BUILD_DATE=${SWI_BUILD_DATE}" >> ${SWI_VERSION_FILE}
echo "BUILD_HOST=${SWI_BUILD_HOST}" >> ${SWI_VERSION_FILE}
echo "SERIALNUM=n/a" >> ${SWI_VERSION_FILE}

# 5. write centec osp release info into the file: /etc/osp-release
OSP_RELEASE_FILE=${ROOTFS_DIR}/rootfs/etc/osp-release
rm -rf ${OSP_RELEASE_FILE}
echo "Centec OSP system ${SWI_VERSION}" >> ${OSP_RELEASE_FILE}
