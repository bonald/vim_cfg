include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc

PROG = ripd

ifeq (0,1)
SRCS = $(wildcard *.c)
else
SRCS = rip_memory.c ripd.c \
	rip_zebra.c rip_interface.c \
	rip_debug.c rip_routemap.c rip_peer.c \
	rip_offset.c
#SRCS += rip_snmp.c
SRCS += rip_main.c
endif

LIBFRR = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBJSON) $(LIBFRR)
LD_LIBS = -L$(LIB_DIR) -lzebra 
LD_LIBS += -ldl
LD_LIBS += -L$(LIB_DIR) -ljson -lcrypt -lrt
#LD_LIBS += -L$(LIB_DIR) -ljson -lreadline -ltermcap -lgenlog -ltask -lsal -lcrypt -lrt -lnetsnmpagent -lnetsnmpmibs -lnetsnmp 
#LD_LIBS += -L$(LIB_DIR) -lnetsnmpagent -lnetsnmpmibs -lnetsnmp
include $(MK_DIR)/prog.mk
