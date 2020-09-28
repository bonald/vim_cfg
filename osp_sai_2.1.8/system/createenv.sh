#!/bin/bash
#
# Copyright (C) 2004-2014 Centec Networks. All rights reserved.
#
#
set -o errexit

usage() {
    echo "Usage: createenv.sh [i686|p1010] [uml|48t4x|] [r|d] [y|n] [gg|gb|d2|tm] [new|old] [tap] [all|en] [centec|h3c] [yes|no]"
    echo "           i686:    Intel X86 i686"
    echo "           p1010:   Freescale Powerpc P1010"
    echo "           uml:     Centec uml Board"
    echo "           48t4x:   Centec 48t4x Board"
    echo "           r:       release version "
    echo "           d:       debug version"
    echo "           y/n:     compile/not kernel"
    echo "           gg/gb/d2/tm:   goldengate/greatbelt/duet2/tsingma" 
    echo "           new/old: new linux/old linux"
    echo "           product: tap"
    echo "           language: web language"
    echo "           vender:   set vender"
	echo "           yes/no   license/no license"
}

# get root directory
CUR_DIR=`dirname $0`
cd $CUR_DIR/..
ROOT_DIR=`pwd`

cd $ROOT_DIR/..
TMP_CUR_DIR=`pwd`
TMP_DIR_0=`basename $TMP_CUR_DIR`
cd $ROOT_DIR/ 
TMP_DIR=`basename $ROOT_DIR`
mkdir -m 777 -p /tmp/software
mkdir -p /tmp/software/$USER
mkdir -p /tmp/software/$USER/$TMP_DIR_0
mkdir -p /tmp/software/$USER/$TMP_DIR_0/$TMP_DIR
BUILD_DIR=/tmp/software/$USER/$TMP_DIR_0/$TMP_DIR

PLAT_TOP_DIR=$ROOT_DIR/platform
TOP_DIR=$ROOT_DIR/system

echo $CUR_DIR
case $# in
    10)
        cpu=$1
        board=$2
        version=$3
        build_kernel=$4
        chip=$5
        linux_version=$6
        product=$7
        language=$8
        vender=$9
        license=$10
        ;;
    9)
        cpu=$1
        board=$2
        version=$3
        build_kernel=$4
        chip=$5
        linux_version=$6
        product=$7
        language=$8
        vender="centec"
        license=$9
        ;;
    8)
        cpu=$1
        board=$2
        version=$3
        build_kernel=$4
        chip=$5
        linux_version=$6
        product=$7
        language="all"
        vender="centec"
        license=$8
        ;;
    7)
        cpu=$1
        board=$2
        version=$3
        build_kernel=$4
        chip=$5
        linux_version=$6
        product="l2+"
        language="all"
        vender="centec"
        license=$7
        ;;
    6)
        cpu=$1
        board=$2
        version=$3
        build_kernel=$4
        chip=$5
        linux_version=$6
        product="l2+"
        language="all"
        vender="centec"
        license="no"
        ;;
    *)
        usage
        exit 1
esac

if [ $cpu == "i686" ]; then
    arch="um"
    crosscompile=""
    withlib="TRUE"
    uml="y"
    m64="FALSE"
    board="ctc-sim"
    sdk_board="ctc-sim"
    toolchain_path="/usr/bin"
    OUT_DIR=$BUILD_DIR/out.uml
    #version=d
    crosscompile=
elif [ $cpu == "x86_64_um" ]; then
    cpu="x86_64"
    arch="um"
    crosscompile=""
    withlib="TRUE"
    uml="y"
    m64="TRUE"
    board="ctc-sim"
    sdk_board="ctc-sim"
    toolchain_path="/usr/bin"
    OUT_DIR=$BUILD_DIR/out.uml
    #version=d
    crosscompile=
elif [ $cpu == "x86_64" ]; then
    arch="x86_64"
    crosscompile=""
    GLIB_USER=/usr
    withlib="TRUE"
    uml="n"
    m64="TRUE"
    if [ $chip == "gg" ] || [ $chip == "gb"  ] || [ $chip == "d2" ] || [ $chip == "tm" ]; then
        sdk_board="linux-board"
    else
        sdk_board="ctc-board"
    fi
    toolchain_path="/usr/bin"
    OUT_DIR=$BUILD_DIR/out.e582
    crosscompile=
elif [ $cpu == "p1010" ] ; then
    arch="powerpc"
    crosscompile="powerpc-fsl_networking-linux-gnuspe-"
    BUILD_ROOTFS_DIR=/opt/fsl-networking/QorIQ-SDK-V1.4/sysroots/ppce500v2-fsl_networking-linux-gnuspe
    BUILD_BIN_DIR=bin
    GLIB_USER=/opt/fsl-networking/QorIQ-SDK-V1.4/sysroots/ppce500v2-fsl_networking-linux-gnuspe/usr
    #withlib="FALSE"
    withlib="TRUE"
    uml="n"
    m64="FALSE"
    if [ $chip == "gg" ] || [ $chip == "gb"  ] || [ $chip == "d2" ] || [ $chip == "tm" ]; then
        sdk_board="linux-board"
    else
        sdk_board="ctc-board"
    fi
    toolchain_path="/opt/fsl-networking/QorIQ-SDK-V1.4/sysroots/i686-fsl_networking_sdk-linux/usr/bin/ppce500v2-fsl_networking-linux-gnuspe"
    OUT_DIR=$BUILD_DIR/out.$board
elif [ $cpu == "hi3535" ] ; then
    arch="arm"
    crosscompile="arm-hisiv200-linux-"
    BUILD_ROOTFS_DIR=/opt/arm/arm-hisiv200-linux/target
    BUILD_BIN_DIR=bin
    GLIB_USER=/opt/arm/arm-hisiv200-linux/target/usr
    #withlib="FALSE"
    withlib="TRUE"
    uml="n"
    m64="FALSE"
    if [ $chip == "gg" ] || [ $chip == "gb"  ] || [ $chip == "d2" ] || [ $chip == "tm" ]; then
        sdk_board="linux-board"
    else
        sdk_board="ctc-board"
    fi
    toolchain_path="/opt/arm/arm-hisiv200-linux/target/bin/"
    OUT_DIR=$BUILD_DIR/out.$board
elif [ $cpu == "ls1023a" ] ; then
    arch="arm64"
    crosscompile="aarch64-linux-gnu-"
    BUILD_ROOTFS_DIR=/opt/aarch64
    BUILD_BIN_DIR=bin
    GLIB_USER=/opt/aarch64/aarch64-linux-gnu/libc/usr
    #withlib="FALSE"
    withlib="TRUE"
    uml="n"
    m64="TRUE"
    if [ $chip == "gg" ] || [ $chip == "gb"  ] || [ $chip == "d2" ] || [ $chip == "tm" ]; then
        sdk_board="linux-board"
    else
        sdk_board="ctc-board"
    fi
    toolchain_path="/opt/aarch64/bin/"
    OUT_DIR=$BUILD_DIR/out.$board
elif [ $cpu == "ctc5236" ] ; then
    arch="arm64"
    crosscompile="aarch64-linux-gnu-"
    BUILD_ROOTFS_DIR=/opt/aarch64
    BUILD_BIN_DIR=bin
    GLIB_USER=/opt/aarch64/aarch64-linux-gnu/libc/usr
    #withlib="FALSE"
    withlib="TRUE"
    uml="n"
    m64="TRUE"
    if [ $chip == "gg" ] || [ $chip == "gb"  ] || [ $chip == "d2" ] || [ $chip == "tm" ]; then
        sdk_board="linux-board"
    else
        sdk_board="ctc-board"
    fi
    toolchain_path="/opt/aarch64/bin/"
    OUT_DIR=$BUILD_DIR/out.$board
else
    usage
    exit 2
fi

if [ $chip == "gg" ]; then
    thechip="goldengate"
    echo "chipname is goldengate"
elif [ $chip == "d2" ]; then
    thechip="duet2"
    postfix="_duet2"
    echo "chipname is duet2"
elif [ $chip == "tm" ]; then
	thechip="tsingma"
    postfix="_tsingma"
    echo "chipname is tsingma"
else
    thechip="greatbelt"
fi

if [ $linux_version == "old" ]; then
	cd $TOP_DIR/mk
	if [ -e "sys.mk" ]; then
	    rm sys.mk
    fi
	ln sys.mk.oldlinux -s sys.mk
	cd ..
else
	cd $TOP_DIR/mk
	if [ -e "sys.mk" ]; then
	    rm sys.mk
	fi
	ln sys.mk.newlinux -s sys.mk
	cd ..
fi

# release version
C_VER=$version

# make env
if [ $arch == "arm64" ] ; then
	CTC_MAKE="make -j12"
else
	CTC_MAKE="make -j12"
fi

# pizza_box or chasis
BTYPE="pizza_box"

# sdk need
SDK_BOARD=$sdk_board
 
MAKEFILE_ENV_INC_FILE=Makefile.env.inc

if [ $arch == "um" ]; then
	rm -f $ROOT_DIR/out.uml
else
	rm -f $ROOT_DIR/out.$board
fi

# please don't edit under this line
#

echo "Generate $MAKEFILE_ENV_INC_FILE ($C_VER) under $BUILD_DIR/ ..."
cat > $MAKEFILE_ENV_INC_FILE <<EOF
#
# please don't edit this file
#
# This file is automatically created by createenv.sh
# edit createenv.sh and run it to setting new environments
#
# Copyrights (C) 2004-2014 Centec Networks (suzhou) Co., Ltd.
# All rights reserved.
# 
# please use absolute path
# source code root directory (is currently directory)
export PATH=$PATH:$toolchain_path
export ROOT_DIR=$ROOT_DIR
export PLAT_TOP_DIR=$PLAT_TOP_DIR
export TOP_DIR=$TOP_DIR
export OUT_DIR=$OUT_DIR
export BUILD_ROOTFS_DIR=$BUILD_ROOTFS_DIR
#export BUILD_BIN_DIR=$BUILD_BIN_DIR
export GLIB_USER=$GLIB_USER
# compile debug or release version (please use d or r)
export VER=$C_VER

export OSP_MAKE=$CTC_MAKE
export CTC_MAKE=$CTC_MAKE

export BUILD_DIR=$BUILD_DIR
#
export SDK_BOARD=$SDK_BOARD
#
#export BTYPE=$BTYPE
#
# please don't modify under this line
#
# cross compile prefix
export CROSS_COMPILE=$crosscompile

# CPU ARCH settings 
export ARCH=$arch
export CPU=$cpu

export targetbase=linux
export BOARD=$board
export WITHLIB=$withlib
export IS_UML=$uml
export M64 = $m64

export SDK_DIR=\$(TOP_DIR)/sdk/$thechip
export CMODEL_DIR=\$(TOP_DIR)/cmodel/$thechip
export HAL_DIR=\$(TOP_DIR)/lc
export SAI_DIR=\$(TOP_DIR)/sai
export OFDPA_DIR=\$(TOP_DIR)/ofdpa
export LAI_DIR=\$(TOP_DIR)/lai
#export LC_DIR=\$(TOP_DIR)/lc
#export SAL_TOP_DIR=\$(TOP_DIR)/sal
export DRV_TOP_DIR=\$(TOP_DIR)/drvs
export MK_DIR=\$(TOP_DIR)/mk
export BLD_DIR=\$(OUT_DIR)/build.\$(CPU).\$(VER)
export KDIR=\$(OUT_DIR)/kbuild.\$(CPU).\$(VER)
export BUSYBOX_DIR=\$(OUT_DIR)/build.busybox.\$(CPU).\$(VER)
export APP_TOP_DIR=\$(TOP_DIR)/apps
export KLISH_DIR=\$(TOP_DIR)/klish
export RPC_API_DIR=\$(TOP_DIR)/rpc_api
export FEA_DIR=\$(TOP_DIR)/fea
export PROTO_DIR=\$(TOP_DIR)/proto
export CDB_DIR=\$(TOP_DIR)/cdb
export SDB_DIR=\$(TOP_DIR)/sdb
export CTC_LIB_DIR=\$(TOP_DIR)/lib
export REDIS_DIR=\$(TOP_DIR)/redis
export QUAGGA_TOP_DIR=\$(TOP_DIR)/frr
export UML_IMAGE_DIR=\$(TOP_DIR)/../umlimage
export TOOLS_DIR=\$(TOP_DIR)/tools
export THIRD_PARTY_DIR=\$(TOP_DIR)/third_party
export MY_ARCH_DIR=ppc
export CTCLIBROOT=\$(OUT_DIR)/build.\$(CPU).\$(VER)/lib
export APPLICATION=no

export SCRIPT_DIR=\$(TOP_DIR)/oem

export CHIPNAME=$thechip

export USE_LICENSE=$license

export IS_GCOV=no
export CHIPGENERATION=$thechip
export UNITTEST=FALSE
export K_DIR=\$(KDIR)
export PRODUCT=$product
export LANGUAGE=$language
export VENDER=$vender

# Import OpenFlow and SDN feature related environment variable to Makefile variable.
export ENABLE_DEBIAN_LINUX=$ENABLE_DEBIAN_LINUX
export ENABLE_PYTHON=$ENABLE_PYTHON
export DEBIAN_ARCH=$DEBIAN_ARCH
export ENABLE_OPENFLOW=$ENABLE_OPENFLOW

EOF
if [ "$build_kernel" == "N" ]; then
    if [ ! -f $OUT_DIR/build.busybox.$cpu.$version/busybox ]; then
        build_kernel="Y"
    fi
    if [ "$arch" == "um" ]; then
        if [ ! -f $OUT_DIR/kbuild.$cpu.$version/linux ]; then
            build_kernel="Y"
        fi
    else
        if [ ! -f $OUT_DIR/kbuild.$cpu.$version/vmlinux.bin.gz ]; then
            build_kernel="Y"
        fi
    fi
fi

if [ "$build_kernel" == "Y" ]; then
    echo "export BLD_KERNEL=yes" >> $MAKEFILE_ENV_INC_FILE 
else
    echo "export BLD_KERNEL=no" >> $MAKEFILE_ENV_INC_FILE
fi

if [ "$ENABLE_NETCONF" == "y" ]; then
    echo "export ENABLE_NETCONF=yes" >> $MAKEFILE_ENV_INC_FILE
else
    echo "export ENABLE_NETCONF=no" >> $MAKEFILE_ENV_INC_FILE
fi

if [ $product == "ttp" ]; then
    #echo "export OVSROOT=\$(TOP_DIR)/proto/openflow_ttp/openvswitch-2.3.0" >> $MAKEFILE_ENV_INC_FILE
    echo "export OVSROOT=\$(TOP_DIR)/proto/openflow_mdttp/openvswitch-2.3.0" >> $MAKEFILE_ENV_INC_FILE
else
    echo "export OVSROOT=\$(TOP_DIR)/proto/openflow/openvswitch-2.3.0" >> $MAKEFILE_ENV_INC_FILE
fi

#if [ -e "sdk" ]; then echo "rm sdk" && rm sdk; fi 
#echo "ln sdk_${chip} -s sdk" && ln sdk_${chip} -s sdk

#if [ -e "fea/Makefile" ]; then echo "rm fea/Makefile" && rm fea/Makefile; fi
#echo "ln Makefile.${chip} -s fea/Makefile" && ln Makefile.${chip} -s fea/Makefile

echo "  Done...."
