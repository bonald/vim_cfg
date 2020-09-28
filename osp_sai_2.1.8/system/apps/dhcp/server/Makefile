include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

PROG= dhcpd6
SRCS= $(wildcard *.c)

ifeq ($(ENABLE_L2),yes)
CPPFLAGS += -I$(ZEBOS_TOP_DIR)/lib/L2
endif

ifeq ($(ENABLE_MPLS),yes)
CPPFLAGS += -I$(ZEBOS_TOP_DIR)/lib/mpls
CPPFLAGS += -I$(ZEBOS_TOP_DIR)/hal/MPLS
endif

CPPFLAGS += -I$(APP_TOP_DIR)/include
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS += -DLINUX_MAJOR=2 -DLINUX_MINOR=6 -DLOCALSTATEDIR='"/var"'

DEP_LIBS  = $(LIB_DIR)/$(LIBZOS) $(LIB_DIR)/$(LIBPAL)
DEP_LIBS += $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a $(LIB_DIR)/$(LIBCTCLIB) $(LIB_DIR)/$(LIBSAL)
LD_LIBS   = -L$(LIB_DIR) -lzos  -lctclib -lsal -lpthread -lpal -lcrypt -lcrypto -lrt
LD_LIBS  += -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6

include $(MK_DIR)/prog.mk
