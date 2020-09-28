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
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/common
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/l2
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/l3
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/mpls
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/aclqos
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/misc
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/openflow
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/tap
#CPPFLAGS += -I$(PROTO_DIR)/fei/include/g8131
CPPFLAGS += -I$(PROTO_DIR)/ccs/include
CPPFLAGS += -I$(PROTO_DIR)/cds/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lldp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/igsp
#CPPFLAGS += -I$(PROTO_DIR)/openflow/adapt/lib
CPPFLAGS += -I$(THIRD_PARTY_DIR)/cjson
CPPFLAGS += -I$(PROTO_DIR)/switch/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lacp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mlag
#CPPFLAGS += -I$(PROTO_DIR)/opm/include
#CPPFLAGS += -I$(PROTO_DIR)/opm/include/erps
#CPPFLAGS += -I$(PROTO_DIR)/opm/include/g8131
#CPPFLAGS += -I$(PROTO_DIR)/opm/include/tpoam
CPPFLAGS += -I$(TOP_DIR)/lib/task


PROG = ctcmon
SRCS = $(wildcard *.c)

LD_LIBS = -L$(LIB_DIR) -lsock -ltask -lctclib -lgenlog -lsal -lm -lrt 

include $(MK_DIR)/prog.mk
