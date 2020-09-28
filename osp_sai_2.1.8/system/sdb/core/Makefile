include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(SDB_DIR)/core/include
CPPFLAGS += -I$(SAI_DIR)/inc

LIB = sdb
SRCS = $(shell find src -name "*.c")

include $(MK_DIR)/solib.mk
