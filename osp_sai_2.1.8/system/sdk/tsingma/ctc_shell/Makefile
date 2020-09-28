ifndef ARCH
export ARCH = i686
endif

ifeq ($(ARCH),powerpc)
CROSS_COMPILE =ppc-linux-
endif

ifeq ($(ARCH),mips)
CROSS_COMPILE =mips64-octeon-linux-gnu-
endif

ifeq ($(ARCH),freescale)
CROSS_COMPILE =powerpc-fsl_networking-linux-gnuspe-
endif

ifeq ($(ARCH),loongson)
CROSS_COMPILE =mipsel-linux-
endif

ifeq ($(ARCH),i686)
CROSS_COMPILE =
endif

ifeq ($(ARCH),arm)
CPU = arm
ifeq ($(M64), TRUE)
ARCH = arm64
CROSS_COMPILE=aarch64-linux-gnu-
else
ARCH = arm
CROSS_COMPILE=arm-hisiv200-linux-
endif
endif
ctc_shell: ctc_shell.c
	 $(CROSS_COMPILE)gcc -o $@ $^ -lpthread

clean:
	rm ctc_shell -rf
