include ../Makefile.env.inc

all_targets =
all_targets += libproto
all_targets += libfei
all_targets += libcdbclient
all_targets += switch
all_targets += cds
all_targets += ccs
all_targets += chsm
all_targets += appcfg
all_targets += pimd_sm
all_targets += pimd_dense
all_targets += ctl
all_targets += nss_cdb
all_targets += authd
all_targets += dhcrelay
all_targets += dhcsnooping
all_targets += dhclient
all_targets += dhclient6

ifeq ($(ARCH), arm64)
all_targets += netfilter
endif

ifeq ($(IS_UML),y)
ifeq ($(CPU), x86_64)
ifeq ($(CHIPNAME), tsingma)
all_targets += netfilter
endif
endif
endif

ifeq ($(IS_UML),y)
all_targets += dhcpd6
endif

all_targets += bhm
all_targets += ncs

ifneq ($(PRODUCT), tap)
all_targets += ssm
all_targets += ptp
all_targets += opm
all_targets += route
endif

#ifeq ($(PRODUCT),tap)
#else
#ifeq ($(LINUX_VERSION),new)
#ifeq ($(ENABLE_OPENFLOW),y)
#all_targets += openflow
#endif
#endif
#endif

ifeq ($(PRODUCT),ttp)
all_targets += openflow_mdttp
else
ifeq ($(ENABLE_OPENFLOW),y)
all_targets += openflow
endif
endif

pm_depends = libproto libfei libcdbclient
#libcdb libcdbpbc

ifeq ($(ARCH), arm64)
pm_depends += netfilter
endif

ifeq ($(IS_UML),y)
ifeq ($(CPU), x86_64)
ifeq ($(CHIPNAME), tsingma)
pm_depends += netfilter
endif
endif
endif

all: $(all_targets) FORCE

clean: $(addprefix clean_,$(all_targets)) FORCE

authd: libfei libproto FORCE
	$(OSP_MAKE) -C authd -f Makefile

clean_authd: FORCE
	$(OSP_MAKE) -C authd -f Makefile clean

dhcsnooping: libfei libproto FORCE
	$(OSP_MAKE) -C dhcsnooping -f Makefile

clean_dhcsnooping: FORCE
	$(OSP_MAKE) -C dhcsnooping -f Makefile clean

dhcrelay: libfei libproto FORCE
	$(OSP_MAKE) -C dhcrelay -f Makefile

clean_dhcrelay: FORCE
	$(OSP_MAKE) -C dhcrelay -f Makefile clean

dhclient: libfei libproto FORCE
	$(OSP_MAKE) -C dhclient -f Makefile

clean_dhclient: FORCE
	$(OSP_MAKE) -C dhclient -f Makefile clean
  
dhclient6: libfei libproto FORCE
	$(OSP_MAKE) -C dhclient6 -f Makefile

clean_dhclient6: FORCE
	$(OSP_MAKE) -C dhclient6 -f Makefile clean

netfilter: FORCE
	$(OSP_MAKE) -C netfilter -f Makefile

clean_netfilter: FORCE
	$(OSP_MAKE) -C netfilter -f Makefile clean

ifeq ($(IS_UML),y)
dhcpd6: libfei libproto FORCE
	$(OSP_MAKE) -C dhcpd6 -f Makefile

clean_dhcpd6: FORCE
	$(OSP_MAKE) -C dhcpd6 -f Makefile clean
endif
	
nss_cdb: FORCE
	$(OSP_MAKE) -C nss_cdb -f Makefile

clean_nss_cdb: FORCE
	$(OSP_MAKE) -C nss_cdb -f Makefile clean

libproto: FORCE
	$(OSP_MAKE) -C lib -f Makefile

clean_libproto: FORCE
	$(OSP_MAKE) -C lib -f Makefile clean

libfei: FORCE
	$(OSP_MAKE) -C fei -f Makefile

clean_libhal: FORCE
	$(OSP_MAKE) -C fei -f Makefile clean

libcdbclient: FORCE
	$(OSP_MAKE) -C cdbclient -f Makefile

clean_libcdbclient: FORCE
	$(OSP_MAKE) -C cdbclient -f Makefile clean

switch: $(pm_depends) FORCE
	$(OSP_MAKE) -C switch -f Makefile

clean_switch: FORCE
	$(OSP_MAKE) -C switch -f Makefile clean

opm: $(pm_depends) FORCE
	$(OSP_MAKE) -C opm -f Makefile

clean_opm: FORCE
	$(OSP_MAKE) -C opm -f Makefile clean

route: $(pm_depends) FORCE
	$(OSP_MAKE) -C route -f Makefile

clean_route: FORCE
	$(OSP_MAKE) -C route -f Makefile clean

cds: $(pm_depends) FORCE
	$(OSP_MAKE) -C cds -f Makefile

clean_cds: FORCE
	$(OSP_MAKE) -C cds -f Makefile clean

ccs: $(pm_depends) FORCE
	$(OSP_MAKE) -C ccs -f Makefile

clean_ccs: FORCE
	$(OSP_MAKE) -C ccs -f Makefile clean

chsm: $(pm_depends) FORCE
	$(OSP_MAKE) -C chsm -f Makefile

clean_chsm: FORCE
	$(OSP_MAKE) -C chsm -f Makefile clean

appcfg: $(pm_depends) FORCE
	$(OSP_MAKE) -C appcfg -f Makefile

clean_appcfg: FORCE
	$(OSP_MAKE) -C appcfg -f Makefile clean

pimd_sm: $(pm_depends) FORCE
	$(OSP_MAKE) -C pimd-sm/src -f Makefile.ctc

clean_pimd_sm: FORCE
	$(OSP_MAKE) -C pimd-sm/src -f Makefile.ctc clean

pimd_dense: FORCE
	$(OSP_MAKE) -C pimd-dense -f Makefile
	cp pimd-dense/pimdd ${BLD_DIR}/bin

clean_pimd_dense: FORCE
	$(OSP_MAKE) -C pimd-dense -f Makefile clean

ptp: $(pm_depends) FORCE
	$(OSP_MAKE) -C ptp -f Makefile

clean_ptp: FORCE
	$(OSP_MAKE) -C ptp -f Makefile clean

bhm: $(pm_depends) FORCE
	$(OSP_MAKE) -C bhm -f Makefile

clean_bhm: $(pm_depends) FORCE
	$(OSP_MAKE) -C clean_bhm -f Makefile

ssm: $(pm_depends) FORCE
	$(OSP_MAKE) -C ssm -f Makefile

clean_ssm: $(pm_depends) FORCE
	$(OSP_MAKE) -C clean_ssm -f Makefile

ncs: $(pm_depends) FORCE
	$(OSP_MAKE) -C ncs -f Makefile

clean_ncs: $(pm_depends) FORCE
	$(OSP_MAKE) -C clean_ncs -f Makefile

ctl: $(pm_depends) FORCE
	$(OSP_MAKE) -C ctl -f Makefile

clean_ctl: FORCE
	$(OSP_MAKE) -C ctl -f Makefile clean

openflow: $(pm_depends) FORCE
	$(OSP_MAKE) -C openflow -f Makefile

clean_openflow: FORCE
	$(OSP_MAKE) -C openflow -f Makefile clean
    
#openflow_ttp: $(pm_depends) FORCE
#	$(OSP_MAKE) -C openflow_ttp -f Makefile

#clean_openflow_ttp: FORCE
#	$(OSP_MAKE) -C openflow_ttp -f Makefile clean

openflow_mdttp: $(pm_depends) FORCE
	$(OSP_MAKE) -C openflow_mdttp -f Makefile

clean_openflow_mdttp: FORCE
	$(OSP_MAKE) -C openflow_mdttp -f Makefile clean

.PHONY: FORCE
FORCE:
