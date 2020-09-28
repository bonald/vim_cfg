include $(QUAGGA_TOP_DIR)/Makefile.env.inc
include ../Makefile.env.inc

CPPFLAGS += -I$(GLIB_USER)/include
LD_LIBS += -L$(GLIB_USER)/lib -ldl

all_targets += libzebra vtysh zebra
all_targets += libvtyclt

ifeq ($(ENABLE_OSPFD),yes)
all_targets += ospfd
all_targets += ripd
all_targets += bgpd
all_targets += ripngd
all_targets += ospf6d
endif

#all_targets += pimd

all: $(all_targets) FORCE

libzebra: FORCE
	$(OSP_MAKE) -C lib -f Makefile

clean_libzebra: FORCE
	$(OSP_MAKE) -C lib -f Makefile clean

vtysh: libzebra FORCE
	@mkdir -p $(BLD_DIR)/etc ;
	@cp -fr $(QUAGGA_TOP_DIR)/vtysh/vtysh.conf.sample $(BLD_DIR)/etc/vtysh.conf ;
	$(OSP_MAKE) -C vtysh -f Makefile

clean_vtysh: FORCE
	$(OSP_MAKE) -C vtysh -f Makefile clean

libvtyclt: libzebra vtysh FORCE
	$(OSP_MAKE) -C vtysh -f Makefile.libvtyclt

clean_libvtyclt: FORCE
	$(OSP_MAKE) -C vtysh -f Makefile.libvtyclt clean

zebra: libzebra FORCE
	@mkdir -p $(BLD_DIR)/etc ;
	@cp -fr $(QUAGGA_TOP_DIR)/zebra/zebra.conf.sample $(BLD_DIR)/etc/zebra.conf ;
	$(OSP_MAKE) -C zebra -f Makefile

clean_zebra: FORCE
	$(OSP_MAKE) -C zebra -f Makefile clean

ospfd: libzebra FORCE
	@mkdir -p $(BLD_DIR)/etc ;
	@cp -fr $(QUAGGA_TOP_DIR)/ospfd/ospfd.conf.sample $(BLD_DIR)/etc/ospfd.conf ;
	$(OSP_MAKE) -C ospfd -f Makefile

clean_ospfd: FORCE
	$(OSP_MAKE) -C ospfd -f Makefile clean

pimd: libzebra FORCE
	@mkdir -p $(BLD_DIR)/etc ;
	@cp -fr $(QUAGGA_TOP_DIR)/pimd/pimd.conf.sample $(BLD_DIR)/etc/pimd.conf ;
	$(OSP_MAKE) -C pimd -f Makefile

clean_pimd: FORCE
	$(OSP_MAKE) -C pimd -f Makefile clean

ripd: libzebra FORCE
	@cp -fr $(QUAGGA_TOP_DIR)/ripd/ripd.conf.sample $(BLD_DIR)/etc/ripd.conf ;
	$(OSP_MAKE) -C ripd -f Makefile

clean_ripd: FORCE
	$(OSP_MAKE) -C ripd -f Makefile clean
	
bgpd: libzebra FORCE
	@cp -fr $(QUAGGA_TOP_DIR)/bgpd/bgpd.conf.sample $(BLD_DIR)/etc/bgpd.conf ;
	$(OSP_MAKE) -C bgpd -f Makefile

clean_bgpd: FORCE
	$(OSP_MAKE) -C bgpd -f Makefile clean
	
ripngd: libzebra FORCE
	@cp -fr $(QUAGGA_TOP_DIR)/ripngd/ripngd.conf.sample $(BLD_DIR)/etc/ripngd.conf ;
	$(OSP_MAKE) -C ripngd -f Makefile

clean_ripngd: FORCE
	$(OSP_MAKE) -C ripngd -f Makefile clean

ospf6d: libzebra FORCE
	@cp -fr $(QUAGGA_TOP_DIR)/ospf6d/ospf6d.conf.sample $(BLD_DIR)/etc/ospf6d.conf ;
	$(OSP_MAKE) -C ospf6d -f Makefile

clean_ospf6d: FORCE
	$(OSP_MAKE) -C ospf6d -f Makefile clean

clean: $(addprefix clean_,$(all_targets)) FORCE

.PHONY: FORCE
FORCE:
