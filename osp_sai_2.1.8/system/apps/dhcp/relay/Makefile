
include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

PROG= dhcrelay6
SRCS= $(wildcard *.c)

CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(TOP_DIR)/lib/genlog
CPPFLAGS += -I$(APP_TOP_DIR)/include
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS += -DLINUX_MAJOR=2 -DLINUX_MINOR=6 -DLOCALSTATEDIR='"/var"'

DEP_LIBS  = $(LIB_DIR)/$(LIBZOS) $(LIB_DIR)/$(LIBPAL)
DEP_LIBS += $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a $(LIB_DIR)/$(LIBCTCLIB) $(LIB_DIR)/$(LIBSAL)
LD_LIBS   = -L$(LIB_DIR) -lzos  -lctclib -lsal -lpthread -lpal -lcrypt -lcrypto
LD_LIBS  += -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6 -lrt

include $(MK_DIR)/prog.mk
