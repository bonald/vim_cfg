include $(MK_DIR)/sys.mk

LIB = tinyrl
##SRCS = $(wildcard ./*.c vt100/*.c history/*.c)
SRCS = $(shell find . -name "*.c")

CPPFLAGS += -I$(KLISH_DIR)

CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/lib/genlog/
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(TOP_DIR)/lib/include

include $(MK_DIR)/lib.mk
