include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/ipc
CPPFLAGS += -I$(CTC_LIB_DIR)/derhex
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(PROTO_DIR)/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include/ds
CPPFLAGS += -I$(CDB_DIR)/core/include
CPPFLAGS += -I$(SDB_DIR)/core/include
CPPFLAGS += -I$(CDB_DIR)/protobuf/include/gen
CPPFLAGS += -I$(CDB_DIR)/protobuf/include
CPPFLAGS += -I$(PROTO_DIR)/cdbclient/include
CPPFLAGS += -I$(PROTO_DIR)/ccs/include
CPPFLAGS += -I$(PROTO_DIR)/cds/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lldp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/igsp
CPPFLAGS += -I$(THIRD_PARTY_DIR)/cjson
CPPFLAGS += -I$(PROTO_DIR)/switch/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lacp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mlag
CPPFLAGS += -I$(TOP_DIR)/lib/task
CPPFLAGS += -I$(APP_TOP_DIR)/snmp/include
CPPFLAGS += -I$(APP_TOP_DIR)/snmpagt

PROG = snmpagent
SRCS = 
SRCS += $(shell find . -name "*.c")

DEP_LIBS = $(LIBCDBCLIENT) $(LIBPROTOPBC) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL) $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lcdbclient -lprotopbc -lproto -lcdb -lsdb -lsock -ltask -lctclib -lgenlog -lsal -lhiredis -lm -lrt -lcrypt -lcrypto -lderhex -lssl -lnetsnmpagent

include $(MK_DIR)/prog.mk
