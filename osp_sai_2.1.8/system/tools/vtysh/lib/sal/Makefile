
include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = sal
SRCS = $(shell find src/{linux_um,common} -name "*.c")

CPPFLAGS += -I$(ROOT_DIR)/lib/sal/include
CPPFLAGS += -D_GNU_SOURCE
CFLAGS += -Werror -Wall

include $(MK_DIR)/lib.mk
