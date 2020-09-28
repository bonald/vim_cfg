
include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = scmsg
SRCS = $(wildcard gen/*.c)

CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I./gen

include $(MK_DIR)/lib.mk
