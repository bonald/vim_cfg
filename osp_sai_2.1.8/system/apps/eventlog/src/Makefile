include ../../../Makefile.env.inc
include $(MK_DIR)/sys.mk

CFLAGS+= -DHAVE_CONFIG_H -I$(CURDIR)/..
CPPFLAGS+= -DHAVE_CONFIG_H -I$(CURDIR)/..

LIB = evtlog
SRCS = $(wildcard *.c)

include $(MK_DIR)/lib.mk
