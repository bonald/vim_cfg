
include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = ctclib
SRCS = $(shell find src -name "*.c")

CPPFLAGS += -I$(ROOT_DIR)/lib/sal/include
CPPFLAGS += -I$(ROOT_DIR)/lib/util/include
CFLAGS += -Werror -Wall

include $(MK_DIR)/lib.mk
