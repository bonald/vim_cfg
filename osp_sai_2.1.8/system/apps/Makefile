include ../Makefile.env.inc

all_targets = eventlog chvrf ftp ntp
all_targets += syslog-ng
all_targets += tftp
all_targets += angel
all_targets += more
all_targets += fnconvert
all_targets += pam_ctc
all_targets += snmp
all_targets += mibd
all_targets += cem
ifneq ($(PRODUCT), tap)
all_targets += dhcp
endif
all_targets += cpulimit
all_targets += top
all_targets += ctcmon
all_targets += snmpagt
all_targets += getrip
all_targets += ctcwatch
#all_targets += web

ifeq ($(ENABLE_NETCONF),yes)
all_targets += netconf
endif

all: $(all_targets) FORCE

clean: $(addprefix clean_,$(all_targets)) FORCE

netconf: FORCE
	make -C netconf -f Makefile

clean_netconf: FORCE
	make -C netconf -f Makefile clean

angel: FORCE
	make -C angel -f Makefile

clean_angel: FORCE
	make -C angel -f Makefile clean
	
dhcp: FORCE
	$(OSP_MAKE) -C dhcp -f Makefile
clean_dhcp: FORCE
	make -C dhcp -f Makefile clean
	
#web: FORCE
#	make -C web -f Makefile

#clean_web: FORCE
#	make -C web -f Makefile clean
	
pam_ctc: FORCE
	make -C pam_ctc -f Makefile

clean_pam_ctc: FORCE
	make -C pam_ctc -f Makefile clean
	
snmp: FORCE
	$(OSP_MAKE) -C snmp -f Makefile.ctc

clean_snmp: FORCE
	make -C snmp -f Makefile.ctc clean

mibd: FORCE
	make -C mibd -f Makefile

clean_mibd: FORCE
	make -C mibd -f Makefile clean

cem: FORCE
	make -C cem -f Makefile

clean_cem: FORCE
	make -C cem -f Makefile clean

top: FORCE
	make -C top -f Makefile

clean_top: FORCE
	make -C top -f Makefile clean	

ctcmon: FORCE
	make -C ctcmon -f Makefile

clean_ctcmon: FORCE
	make -C ctcmon -f Makefile clean	

ctcwatch: FORCE
	make -C watch -f Makefile

clean_ctcwatch: FORCE
	make -C watch -f Makefile clean	

snmpagt: snmp FORCE
	make -C snmpagt -f Makefile

clean_snmpagt: FORCE
	make -C snmpagt -f Makefile clean	

getrip: snmp FORCE
	make -C getrip -f Makefile

clean_getrip: FORCE
	make -C getrip -f Makefile clean	

cpulimit: FORCE
	make -C cpulimit -f Makefile.ctc

clean_cpulimit: FORCE
	make -C cpulimit -f Makefile.ctc clean	

fnconvert: FORCE
	make -C fnconvert -f Makefile

clean_fnconvert: FORCE
	make -C fnconvert -f Makefile clean

ntp: FORCE
	$(OSP_MAKE) -C ntp -f Makefile.ctc

clean_ntp: FORCE
	make -C ntp -f Makefile clean

tftp: FORCE
	make -C tftp -f Makefile

clean_tftp: FORCE
	make -C tftp -f Makefile clean

chvrf: FORCE
	make -C chvrf -f Makefile

clean_chvrf: FORCE
	make -C chvrf -f Makefile clean

eventlog: FORCE
	$(OSP_MAKE) -C eventlog/src -f Makefile

clean_eventlog: FORCE
	make -C eventlog/src -f Makefile clean

syslog-ng: FORCE
	$(OSP_MAKE) -C syslog-ng/src -f Makefile

clean_syslog-ng: FORCE
	make -C syslog-ng/src -f Makefile clean
    
ftp: FORCE
	make -C ftp -f Makefile

clean_ftp: FORCE
	make -C ftp -f Makefile clean

more: FORCE
	make -C more -f Makefile

clean_more: FORCE
	make -C more -f Makefile clean

openssh: FORCE
	make -C openssh -f Makefile

clean_openssh: FORCE
	make -C openssh -f Makefile clean

.PHONY: FORCE
FORCE:
