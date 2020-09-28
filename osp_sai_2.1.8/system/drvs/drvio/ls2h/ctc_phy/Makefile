LKMOBJDIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))

obj-m = ctc_phy.o

ifeq ($(CPU),ls2h)
EXTRA_CFLAGS += -D_CTC_LOONGSON_2H_
EXTRA_CFLAGS += -D_CTC_LOONGSON_
endif
ifeq ($(CPU),ls2k)
EXTRA_CFLAGS += -D_CTC_LOONGSON_2K_
EXTRA_CFLAGS += -D_CTC_LOONGSON_
endif

EXTRA_CFLAGS += -I$(TOP_DIR)/drvs/drvio/ls2h/ctc_boards
KBUILD_EXTRA_SYMBOLS = $(LKMOBJDIR)/../ctc_boards/Module.symvers

CPPFLAGS += -I$(TOP_DIR)/include/
.PHONY: all
all:
	mkdir -p $(LKMOBJDIR)
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR)
	cp $(LKMOBJDIR)/ctc_phy.ko $(BLD_DIR)/bin

.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR) clean
	rm -f $(BLD_DIR)/bin/ctc_phy.ko
