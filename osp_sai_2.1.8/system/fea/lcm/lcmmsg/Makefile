include $(MK_DIR)/sys.mk

LIB = lcmmsg
SRCS = $(wildcard *.c)
SRCS += $(wildcard gen/*.c)

CPPFLAGS += -I. -I./gen
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/lib/libasn1

include $(MK_DIR)/lib.mk
