include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk
include $(PROTO_DIR)/Makefile.inc

LIB = mnl
SRCS = $(wildcard ./src/*.c)

CFLAGS += -I. -I./src
CFLAGS += -I./include
CFLAGS += "-Wno-error"

include $(MK_DIR)/solib.mk
