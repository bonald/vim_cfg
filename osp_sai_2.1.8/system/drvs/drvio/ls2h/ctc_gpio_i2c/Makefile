LKMOBJDIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))

obj-m = ctc_gpio_i2c.o

EXTRA_CFLAGS += -D_NEW_VERSION_LINUX_
EXTRA_CFLAGS += -I$(TOP_DIR)/drvs/drvio/ls2h/ctc_boards
KBUILD_EXTRA_SYMBOLS = $(LKMOBJDIR)/../ctc_boards/Module.symvers

CPPFLAGS += -I$(TOP_DIR)/include/
.PHONY: all
all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR)
	cp $(LKMOBJDIR)/ctc_gpio_i2c.ko $(BLD_DIR)/bin

.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) LKMOBJDIR=$(LKMOBJDIR) clean
	rm -f $(BLD_DIR)/bin/ctc_gpio_i2c.ko
