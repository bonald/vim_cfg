ifndef VER
export VER = d
endif

ifndef CHIPNAME
export CHIPNAME = greatbelt
endif

ifeq ($(ARCH),powerpc)
CPU = ppc_82xx
CROSS_COMPILE =ppc-linux-
endif

ifeq ($(ARCH),mips)
CPU = octeon
CROSS_COMPILE =mips64-octeon-linux-gnu-
EXTRA_CFLAGS += -D_CTC_OCTEON_CN50XX_
EXTRA_CFLAGS += -DDMA_MEM_MODE_PLATFORM
endif

ifeq ($(ARCH),loongson)
ARCH = mips
CPU = ls2f
CROSS_COMPILE =mipsel-linux-
EXTRA_CFLAGS += -DDMA_MEM_MODE_PLATFORM
endif

ifeq ($(ARCH),freescale)
ARCH = powerpc
CPU = fscl
CROSS_COMPILE =powerpc-fsl_networking-linux-gnuspe-
endif

ifeq ($(ARCH),x86)
ARCH = x86
CPU = x86
CROSS_COMPILE =
endif

ifeq ($(ARCH),arm)
CPU = arm
ifeq ($(M64), TRUE)
ARCH = arm64
CROSS_COMPILE =aarch64-linux-gnu-
else
ARCH = arm
CROSS_COMPILE =arm-hisiv200-linux-
endif
endif

DAL_DIR = $(CURDIR)/../../../..
SDK_DIR = $(DAL_DIR)/..

LKMOBJDIR=$(SDK_DIR)/build.$(CPU).$(VER)/obj.$(BOARD)/dal/src/linux/kernel/knet
BLD_DIR=$(SDK_DIR)/build.$(CPU).$(VER)

obj-m := knet.o
knet-objs := knet_kernel.o dal_mpool.o
KNET_KO = knet.ko 

EXTRA_CFLAGS += -I$(DAL_DIR)/include
export CROSS_COMPILE

.PHONY: all
all:
	cp $(DAL_DIR)/src/common/dal_mpool.c .
	cp $(DAL_DIR)/include/dal_*.h .
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR)
	if [ ! -d $(dir $(BLD_DIR)/bin.$(BOARD)/) ]; then mkdir -p $(dir $(BLD_DIR)/bin.$(BOARD)/); fi
	cp $(LKMOBJDIR)/$(KNET_KO) $(BLD_DIR)/bin.$(BOARD)
	rm dal_common.h dal_mpool.*


.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR) clean
	rm -f $(BLD_DIR)/bin.$(BOARD)/$(KNET_KO)
