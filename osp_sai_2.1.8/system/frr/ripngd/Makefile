include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc

PROG = ripngd

ifeq (0,1)
SRCS = $(wildcard *.c)
else
SRCS = ripngd.c ripng_debug.c \
	   ripng_interface.c ripng_main.c \
	   ripng_memory.c ripng_nexthop.c \
	   ripng_offset.c ripng_peer.c \
	   ripng_route.c ripng_routemap.c \
	   ripng_zebra.c
endif

LIBFRR = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBJSON) $(LIBFRR)
LD_LIBS = -L$(LIB_DIR) -lzebra 
LD_LIBS += -ldl
LD_LIBS += -L$(LIB_DIR) -ljson -lcrypt -lrt
#LD_LIBS += -L$(LIB_DIR) -ljson -lreadline -ltermcap -lgenlog -ltask -lsal -lcrypt -lrt -lnetsnmpagent -lnetsnmpmibs -lnetsnmp 
#LD_LIBS += -L$(LIB_DIR) -lnetsnmpagent -lnetsnmpmibs -lnetsnmp
include $(MK_DIR)/prog.mk
