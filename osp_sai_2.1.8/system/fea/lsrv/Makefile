include $(MK_DIR)/sys.mk

LIB = lsrv
SRCS += $(shell find src -name "*.c")

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/ipc
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include

CPPFLAGS += -I$(PROTO_DIR)/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include/ds
CPPFLAGS += -I$(CDB_DIR)/core/include


CPPFLAGS += -I$(FEA_DIR)/lsrv/include
CPPFLAGS += -I$(TOP_DIR)/lai/include
CPPFLAGS += -I$(TOP_DIR)/lai/inc

include $(MK_DIR)/lib.mk
