include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = omapi6
SFILES= $(wildcard *.c)
SRCS= $(subst test.c,,$(SFILES))

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS+= -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS+= -DLINUX_MAJOR=2 -DLINUX_MINOR=6

include $(MK_DIR)/lib.mk
