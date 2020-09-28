include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

LIB = ctclib
SRCS = $(shell find src -name "*.c")

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/lib/genlog
CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/util/include

CFLAGS += -Werror -Wall

include $(MK_DIR)/solib.mk
