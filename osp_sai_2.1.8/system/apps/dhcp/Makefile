include ../../Makefile.env.inc

include ./Makefile.pre.ctc

#all_targets += dhcpctl 
#all_targets += libscmsg
#all_targets += relay
#all_targets += server
#all_targets += smartconfig
#all_targets += client

all: $(all_targets) FORCE

clean: $(addprefix clean_,$(all_targets)) FORCE

smartconfig: libscmsg FORCE
	@cd scfgd && chmod a+x ./gen.sh && ./gen.sh
	make -C scfgd/gen -f Makefile
	make -C scfgd -f Makefile

clean_smartconfig: FORCE
	make -C scfgd -f Makefile clean

libscmsg: libscmsg/scfg.asn1 FORCE
	@cd libscmsg && chmod a+x ./gen.sh && ./gen.sh && cd ../
	make -C libscmsg -f Makefile

clean_libscmsg: FORCE
	make -C libscmsg -f Makefile clean
	@cd libscmsg && rm -fr ./gen && cd ../
relay: FORCE
	make -C relay -f Makefile

clean_relay: FORCE
	make -C relay -f Makefile clean

dhcpctl: FORCE
	make -C dhcpctl -f Makefile

clean_dhcpctl: FORCE
	make -C dhcpctl -f Makefile clean

client: FORCE
	make -C client -f Makefile

clean_client: FORCE
	make -C client -f Makefile clean

server: FORCE
	make -C server -f Makefile

clean_server: FORCE
	make -C server -f Makefile clean
    
.PHONY: FORCE
FORCE:
