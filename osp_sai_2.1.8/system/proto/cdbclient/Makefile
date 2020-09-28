include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

include $(PROTO_DIR)/Makefile.inc

LIB = cdbclient
SRCS = 
SRCS = $(shell find src -name "*.c")

CFLAGS += -I${TOP_DIR}/lib/derhex
CFLAGS += -Wno-format
#LD_FLAGS += -I
include $(MK_DIR)/solib.mk
