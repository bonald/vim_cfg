LKMOBJDIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))

obj-m = ctc_hw.o

EXTRA_CFLAGS += -I$(TOP_DIR)/drvs/drvio/ls2h/ctc_boards
KBUILD_EXTRA_SYMBOLS = $(LKMOBJDIR)/../ctc_boards/Module.symvers

CPPFLAGS += -I$(TOP_DIR)/include/
.PHONY: all
all:
	mkdir -p $(LKMOBJDIR)
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR)
	cp $(LKMOBJDIR)/ctc_hw.ko $(BLD_DIR)/bin

.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR) clean
	rm -f $(BLD_DIR)/bin/ctc_hw.ko
