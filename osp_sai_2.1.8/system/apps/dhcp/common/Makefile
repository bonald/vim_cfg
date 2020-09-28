include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = dhcp6
SRCS = $(wildcard *.c)

CPPFLAGS+= -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS += -I$(CTC_LIB_DIR)/include  
CPPFLAGS+= -DLINUX_MAJOR=2 -DLINUX_MINOR=6

include $(MK_DIR)/lib.mk
