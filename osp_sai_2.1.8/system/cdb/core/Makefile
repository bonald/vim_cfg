include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

ifeq ($(PRODUCT),sai)
#CPPFLAGS += -DLDB_SYNC_TO_RDB
endif

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(PROTO_DIR)/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(CDB_DIR)/core/include
CPPFLAGS += -I$(CDB_DIR)/protobuf/include/gen
CPPFLAGS += -I$(CDB_DIR)/protobuf/include
CPPFLAGS += -I$(PROTO_DIR)/cdbclient/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lacp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/opm/include
CPPFLAGS += -I$(PROTO_DIR)/opm/include/erps

LIB = cdb
SRCS = $(shell find src -name "*.c")

include $(MK_DIR)/solib.mk
