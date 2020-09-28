include $(MK_DIR)/sys.mk

LIB = sock
SRCS = $(shell find . -name "*.c")

CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/sock
CPPFLAGS += -I$(TOP_DIR)/lib/sock/ctc
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(TOP_DIR)/lib/task

include $(MK_DIR)/solib.mk
