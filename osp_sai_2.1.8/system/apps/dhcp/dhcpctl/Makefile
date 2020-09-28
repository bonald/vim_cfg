include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = dhcpctl6
SRCS=dhcpctl.c callback.c remote.c

CPPFLAGS+= -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS+= -DLINUX_MAJOR=2 -DLINUX_MINOR=6

include $(MK_DIR)/lib.mk
