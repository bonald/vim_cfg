include $(MK_DIR)/sys.mk

LIB = dbclient
SRCS = $(shell find . -name "*.c")

CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(TOP_DIR)/redis/deps/hiredis
CPPFLAGS += -I$(TOP_DIR)/lib/dbclient
CPPFLAGS += -I$(TOP_DIR)/lib/genlog

include $(MK_DIR)/solib.mk
