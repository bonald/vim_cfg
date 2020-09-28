include $(MK_DIR)/sys.mk

LIB = cjson
SRCS = $(shell find . -name "*.c")

CPPFLAGS += -I.
CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/util/include

include $(MK_DIR)/solib.mk
