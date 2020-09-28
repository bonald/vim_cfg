include ../../../Makefile.env.inc
include $(MK_DIR)/sys.mk

LIB = hagtmsg
SRCS = $(wildcard *.c)
SRCS += $(wildcard gen/*.c)

CPPFLAGS += -I./gen
CPPFLAGS += -I$(TOP_DIR)/lib/libasn1

include $(MK_DIR)/lib.mk
