LKMOBJDIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))

obj-m = ctc_gpio.o

EXTRA_CFLAGS += -I$(TOP_DIR)/drvs/drvio/${CPU}/ctc_boards
KBUILD_EXTRA_SYMBOLS = $(LKMOBJDIR)/../ctc_boards/Module.symvers

.PHONY: all
all:
	mkdir -p $(LKMOBJDIR)
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR)
	cp $(LKMOBJDIR)/ctc_gpio.ko $(BLD_DIR)/bin

.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR) clean
	rm -f $(BLD_DIR)/bin/ctc_gpio.ko
