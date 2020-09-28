include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc

PROG = bgpd

ifeq (0,1)
SRCS = $(wildcard *.c)
else
SRCS = \
	bgpd.c bgp_zebra.c bgp_advertise.c bgp_aspath.c bgp_attr.c \
	bgp_attr_evpn.c bgp_bfd.c bgp_clist.c bgp_community.c \
	bgp_damp.c bgp_debug.c bgp_dump.c bgp_ecommunity.c bgp_encap_tlv.c bgp_evpn.c \
	bgp_evpn_vty.c bgp_filter.c bgp_fsm.c bgp_routemap.c \
	bgp_lcommunity.c bgp_memory.c bgp_mpath.c bgp_mplsvpn.c bgp_network.c \
	bgp_nexthop.c bgp_nht.c bgp_open.c bgp_packet.c bgp_regex.c bgp_route.c \
	bgp_table.c bgp_updgrp.c bgp_updgrp_adv.c bgp_updgrp_packet.c bgp_vpn.c bgp_vty.c \
	#end
#SRCS += bgp_snmp.c
SRCS += bgp_main.c
endif

LIBZEBRA = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBZEBRA) $(LIBJSON)
#LD_LIBS = -L$(LIB_DIR) -ljson -lzebra -lcrypt -lrt -lm
LD_LIBS += -L$(LIB_DIR) -ljson -lcrypt -lrt -lgenlog -ltask -lsal -lm -lzebra 
#LD_LIBS = -L$(LIB_DIR) -lfrr
LD_LIBS += -ldl
#LD_LIBS += -L$(LIB_DIR) -lnetsnmpagent -lnetsnmpmibs -lnetsnmp 

include $(MK_DIR)/prog.mk
