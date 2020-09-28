LKMOBJDIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))

obj-m = ctc_gpio.o

EXTRA_CFLAGS += -D_NEW_VERSION_LINUX_
ifeq ($(ARCH),mips)
EXTRA_CFLAGS += -D_CTC_OCTEON_CN50XX_
endif
ifeq ($(ARCH), powerpc)
ifeq ($(CPU), p1010)
EXTRA_CFLAGS += -D_CTC_FSCL_P1010_
endif
ifeq ($(CPU), p1014)
EXTRA_CFLAGS += -D_CTC_FSCL_P1014_
endif
ifeq ($(CPU), mpc8308)
EXTRA_CFLAGS += -D_CTC_MPC8308_
endif
endif
ifeq ($(ARCH),x86_64)
EXTRA_CFLAGS += -D_CTC_X86_64_
endif

EXTRA_CFLAGS += -I$(TOP_DIR)/drvs/drvio/powerpc/ctc_boards
KBUILD_EXTRA_SYMBOLS = $(LKMOBJDIR)/../ctc_boards/Module.symvers

CPPFLAGS += -I$(TOP_DIR)/include/
.PHONY: all
all:
	mkdir -p $(LKMOBJDIR)
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR)
	cp $(LKMOBJDIR)/ctc_gpio.ko $(BLD_DIR)/bin

.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR) clean
	rm -f $(BLD_DIR)/bin/ctc_gpio.ko
