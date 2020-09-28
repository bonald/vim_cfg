include $(MK_DIR)/sys.mk

LIB = genlog
SRCS = $(shell find . -name "*.c")

CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/genlog/include
CPPFLAGS += -I$(TOP_DIR)/lib/task

include $(MK_DIR)/solib.mk
