include ../Makefile.env.inc

all_targets =
all_targets += liblub
all_targets += libtinyrl
all_targets += libkonf
all_targets += libclish
all_targets += klish
all_targets += cmds

klish_depends = liblub libtinyrl libkonf libclish

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

liblub: FORCE
	make $(COMPILE_SPEED) -C lub/ -f Makefile

clean_liblub: FORCE
	make $(COMPILE_SPEED) -C lub/ -f Makefile clean

libtinyrl: FORCE
	make $(COMPILE_SPEED) -C tinyrl/ -f Makefile

clean_libtinyrl: FORCE
	make $(COMPILE_SPEED) -C tinyrl/ -f Makefile clean

libkonf: FORCE
	make $(COMPILE_SPEED) -C konf/ -f Makefile

clean_libkonf: FORCE
	make $(COMPILE_SPEED) -C konf/ -f Makefile clean

libclish: FORCE
	make $(COMPILE_SPEED) -C clish/ -f Makefile

clean_libclish: FORCE
	make $(COMPILE_SPEED) -C clish/ -f Makefile clean

klish: $(klish_depends) FORCE
	make $(COMPILE_SPEED) -C bin/ -f Makefile

clean_klish: FORCE
	make $(COMPILE_SPEED) -C bin/ -f Makefile clean

cmds: FORCE
	@set old_pwd ${shell pwd};
	@rm -rf $(BLD_DIR)/etc/cmd;
	@mkdir -p $(BLD_DIR)/etc/cmd;
	@echo "PP is $(PRODUCT)"
	if [ 'tap' == $(PRODUCT) ] ; then \
		cp -fr $(KLISH_DIR)/xml-examples/tapcmd/*.xml $(BLD_DIR)/etc/cmd/ ; \
		if [ 'goldengate' == $(CHIPNAME) ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v580/*.xml $(BLD_DIR)/etc/cmd/ ; \
		elif [ 'duet2' == $(CHIPNAME) ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v550/*.xml $(BLD_DIR)/etc/cmd/ ; \
		elif [ 'tsingma' == $(CHIPNAME) ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v530/*.xml $(BLD_DIR)/etc/cmd/ ; \
		else \
			cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v350/*.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ -d $(SCRIPT_DIR) ] ; then \
			$(SCRIPT_DIR)/vender-script.sh xml; \
		fi; \
	elif [ 'of' == $(PRODUCT) -o 'ttp' == $(PRODUCT) ] ; then \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/arp.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/arp_limit.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/auth.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/bhm.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/common.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/debug.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/device.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/errdisable.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/fdb.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/interface.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/internal_debug.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/management.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/mirror.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/ntp.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/ospf.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/bgp.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/ping.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/route.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/snmp.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/ssh.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/startup.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/switch_interface.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/syslog.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/system_configuration.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/type.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/vlan.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/license.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/iptables.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/http.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/copp_cfg.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/acl.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/acl_mac.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/acl_policy.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/ipsla.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/track.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/sflow.xml $(BLD_DIR)/etc/cmd/ ; \
		if [ "${ENABLE_NETCONF}" == "yes" ]; then \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/netconf.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/gx/qos_policer.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/ofcmd/openflow.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/ofcmd/stm.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/ofcmd/global-commands.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/ofcmd/igmp_snooping.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/dhclient.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/mstp.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/dhcrelay.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/rstp.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/ofcmd/watch.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/management_ipv6.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/ping_ipv6.xml $(BLD_DIR)/etc/cmd/ ; \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/system_configuration_ipv6.xml $(BLD_DIR)/etc/cmd/ ; \
		if [ 'goldengate' == $(CHIPNAME) -o $(CHIPNAME) == 'greatbelt' ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/show_json.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ 'goldengate' == $(CHIPNAME) -o $(CHIPNAME) == 'greatbelt' ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/g8032.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/vlan_mapping.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/cfm.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ 'goldengate' == $(CHIPNAME) ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/g8131.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/oam_debug.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/openflow_mdttp.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/tpoam.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/tpoam_exec.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/udf.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/cut_through_gg.xml $(BLD_DIR)/etc/cmd/ ; \
		else \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/cut_through.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ $(CHIPNAME) == 'tsingma' ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/tsingma_hash/*.xml $(BLD_DIR)/etc/cmd/ ; \
		else \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/not_tsingma_hash/*.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ $(CHIPNAME) == 'duet2' -o $(CHIPNAME) == 'tsingma' ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/g8131.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/oam_debug.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/tpoam.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/tpoam_exec.xml $(BLD_DIR)/etc/cmd/ ; \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/duet2/*.xml $(BLD_DIR)/etc/cmd/ ; \
		else \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/gx/*.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ 'tsingma' == $(CHIPNAME) -o 'duet2' == $(CHIPNAME) ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/ofcmd/udf.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
	elif [ 'sai' == $(PRODUCT) ] ; then \
		cp -fr $(KLISH_DIR)/xml-examples/saicmd/*.xml $(BLD_DIR)/etc/cmd/ ; \
		if [ $(CHIPNAME) == 'greatbelt' ] ; then \
			rm $(BLD_DIR)/etc/cmd/ptp.xml ; \
		fi; \
		if [ $(CHIPNAME) == 'duet2' -o $(CHIPNAME) == 'goldengate' ] ; then \
			rm $(BLD_DIR)/etc/cmd/nat.xml ; \
		fi; \
		if [ $(CHIPNAME) == 'duet2' -o $(CHIPNAME) == 'tsingma' ] ; then \
			rm $(BLD_DIR)/etc/cmd/g8032.xml ; \
		fi; \
		if [ $(CHIPNAME) == 'tsingma' ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/tsingma_hash/*.xml $(BLD_DIR)/etc/cmd/ ; \
		else \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/not_tsingma_hash/*.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ $(CHIPNAME) == 'duet2' -o $(CHIPNAME) == 'tsingma' ] ; then \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/duet2/*.xml $(BLD_DIR)/etc/cmd/ ; \
		else \
			cp -fr $(KLISH_DIR)/xml-examples/saicmd/gx/*.xml $(BLD_DIR)/etc/cmd/ ; \
		fi; \
		if [ $(CHIPNAME) == 'goldengate' ] ; then \
			rm $(BLD_DIR)/etc/cmd/cut_through.xml ; \
		else \
			rm $(BLD_DIR)/etc/cmd/cut_through_gg.xml ; \
		fi; \
		if [ $(CHIPNAME) == 'duet2' -o $(CHIPNAME) == 'tsingma' ] ; then \
			rm $(BLD_DIR)/etc/cmd/cfm.xml ; \
		fi; \
		rm $(BLD_DIR)/etc/cmd/ipmc.xml ; \
	elif [ 'ofdpa' == $(PRODUCT) ] ; then \
		cp -fr $(KLISH_DIR)/xml-examples/ofdpacmd/*.xml $(BLD_DIR)/etc/cmd/ ; \
	else \
		cp -fr $(KLISH_DIR)/xml-examples/ospcmd/*.xml $(BLD_DIR)/etc/cmd/ ; \
	fi;
	if [ $(CPU) == 'x86_64' ] ; then \
		cp -fr $(KLISH_DIR)/xml-examples/debcmd/*.xml $(BLD_DIR)/etc/cmd/ ; \
	fi;
	if [ $(ENABLE_DEBIAN_LINUX) == 'y' ] ; then \
		cp -fr $(KLISH_DIR)/xml-examples/debcmd/*.xml $(BLD_DIR)/etc/cmd/ ; \
	fi;

clean_cmds: FORCE
	make $(COMPILE_SPEED) -C bin/ -f Makefile clean

.PHONY: FORCE
FORCE:
