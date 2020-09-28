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
export PATH=/opt/aarch64/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/root/bin:/opt/aarch64/bin/
export ROOT_DIR=/mnt/test/CNOS-code-tsingma/osp_sai_2.1.8
export PLAT_TOP_DIR=/mnt/test/CNOS-code-tsingma/osp_sai_2.1.8/platform
export TOP_DIR=/mnt/test/CNOS-code-tsingma/osp_sai_2.1.8/system
export OUT_DIR=/tmp/software//CNOS-code-tsingma/osp_sai_2.1.8/out.e530
export BUILD_ROOTFS_DIR=/opt/aarch64
#export BUILD_BIN_DIR=bin
export GLIB_USER=/opt/aarch64/aarch64-linux-gnu/libc/usr
# compile debug or release version (please use d or r)
export VER=r

export OSP_MAKE=make -j12
export CTC_MAKE=make -j12

export BUILD_DIR=/tmp/software//CNOS-code-tsingma/osp_sai_2.1.8
#
export SDK_BOARD=linux-board
#
#export BTYPE=pizza_box
#
# please don't modify under this line
#
# cross compile prefix
export CROSS_COMPILE=aarch64-linux-gnu-

# CPU ARCH settings 
export ARCH=arm64
export CPU=ctc5236

export targetbase=linux
export BOARD=e530
export WITHLIB=TRUE
export IS_UML=n
export M64 = TRUE

export SDK_DIR=$(TOP_DIR)/sdk/tsingma
export CMODEL_DIR=$(TOP_DIR)/cmodel/tsingma
export HAL_DIR=$(TOP_DIR)/lc
export SAI_DIR=$(TOP_DIR)/sai
export OFDPA_DIR=$(TOP_DIR)/ofdpa
export LAI_DIR=$(TOP_DIR)/lai
#export LC_DIR=$(TOP_DIR)/lc
#export SAL_TOP_DIR=$(TOP_DIR)/sal
export DRV_TOP_DIR=$(TOP_DIR)/drvs
export MK_DIR=$(TOP_DIR)/mk
export BLD_DIR=$(OUT_DIR)/build.$(CPU).$(VER)
export KDIR=$(OUT_DIR)/kbuild.$(CPU).$(VER)
export BUSYBOX_DIR=$(OUT_DIR)/build.busybox.$(CPU).$(VER)
export APP_TOP_DIR=$(TOP_DIR)/apps
export KLISH_DIR=$(TOP_DIR)/klish
export RPC_API_DIR=$(TOP_DIR)/rpc_api
export FEA_DIR=$(TOP_DIR)/fea
export PROTO_DIR=$(TOP_DIR)/proto
export CDB_DIR=$(TOP_DIR)/cdb
export SDB_DIR=$(TOP_DIR)/sdb
export CTC_LIB_DIR=$(TOP_DIR)/lib
export REDIS_DIR=$(TOP_DIR)/redis
export QUAGGA_TOP_DIR=$(TOP_DIR)/frr
export UML_IMAGE_DIR=$(TOP_DIR)/../umlimage
export TOOLS_DIR=$(TOP_DIR)/tools
export THIRD_PARTY_DIR=$(TOP_DIR)/third_party
export MY_ARCH_DIR=ppc
export CTCLIBROOT=$(OUT_DIR)/build.$(CPU).$(VER)/lib
export APPLICATION=no

export SCRIPT_DIR=$(TOP_DIR)/oem

export CHIPNAME=tsingma

export USE_LICENSE=no

export IS_GCOV=no
export CHIPGENERATION=tsingma
export UNITTEST=FALSE
export K_DIR=$(KDIR)
export PRODUCT=sai
export LANGUAGE=all
export VENDER=centec

# Import OpenFlow and SDN feature related environment variable to Makefile variable.
export ENABLE_DEBIAN_LINUX=n
export ENABLE_PYTHON=n
export DEBIAN_ARCH=arm64
export ENABLE_OPENFLOW=n

export BLD_KERNEL=yes
export ENABLE_NETCONF=no
export OVSROOT=$(TOP_DIR)/proto/openflow/openvswitch-2.3.0
