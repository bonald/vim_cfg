export WARN_NOERROR = y
include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc
CFLAGS += -Wno-all

CPPFLAGS += -std=c99

PROG = zebra

SRCS = zserv.c main.c interface.c connected.c zebra_rib.c zebra_routemap.c \
       redistribute.c debug.c rtadv.c zebra_snmp.c zebra_vty.c \
       irdp_main.c irdp_interface.c irdp_packet.c router-id.c zebra_fpm.c

SRCS += zebra_fpm_netlink.c if_netlink.c rt_netlink.c rtread_netlink.c ioctl.c ipforward_proc.c \
        kernel_netlink.c zebra_memory.c zebra_mpls.c zebra_mpls_vty.c zebra_mpls_null.c \
		zebra_vrf.c zebra_static.c zebra_rnh.c zebra_ptm.c zebra_ptm_redistribute.c zebra_ns.c

SRCS += label_manager.c zebra_mroute.c zebra_pw.c zebra_pw_null.c


LIBZEBRA = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBJSON) $(LIBZEBRA)
LD_LIBS = -L$(LIB_DIR) -ljson -lcrypt -lrt -lzebra -ldl

include $(MK_DIR)/prog.mk
