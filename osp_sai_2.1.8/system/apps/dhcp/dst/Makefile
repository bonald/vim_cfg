include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = dst6
SRCS= $(wildcard *.c)

CPPFLAGS+= -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS+= -DLINUX_MAJOR=2 -DLINUX_MINOR=6 -DMINIRES_LIB -DHMAC_MD5 

include $(MK_DIR)/lib.mk
