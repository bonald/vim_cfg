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
CPPFLAGS += -I$(PROTO_DIR)/fei/include/common
CPPFLAGS += -I$(PROTO_DIR)/fei/include/l2
CPPFLAGS += -I$(PROTO_DIR)/fei/include/l3
CPPFLAGS += -I$(PROTO_DIR)/fei/include/mpls
CPPFLAGS += -I$(PROTO_DIR)/fei/include/aclqos
CPPFLAGS += -I$(PROTO_DIR)/fei/include/misc
CPPFLAGS += -I$(PROTO_DIR)/fei/include/openflow
CPPFLAGS += -I$(PROTO_DIR)/fei/include/tap
CPPFLAGS += -I$(PROTO_DIR)/fei/include/g8131
CPPFLAGS += -I$(PROTO_DIR)/ccs/include
CPPFLAGS += -I$(PROTO_DIR)/cds/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lldp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/igsp
CPPFLAGS += -I$(PROTO_DIR)/netfilter/libmnl-1.0.4/include
CPPFLAGS += -I$(PROTO_DIR)/netfilter/libnfnetlink-1.0.1/include
CPPFLAGS += -I$(PROTO_DIR)/netfilter/libnetfilter_conntrack-1.0.7/include

ifeq ($(PRODUCT),ttp)
#CPPFLAGS += -I$(PROTO_DIR)/openflow_mdttp/adapt/lib
CPPFLAGS += -I$(PROTO_DIR)/openflow_mdttp/adapt/lib
else
CPPFLAGS += -I$(PROTO_DIR)/openflow/adapt/lib
endif

CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/common
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/intf
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/l2
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/l3
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/misc
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/mpls
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/aclqos
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/openflow
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/tap
CPPFLAGS += -I$(THIRD_PARTY_DIR)/cjson
CPPFLAGS += -I$(PROTO_DIR)/switch/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/lacp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mstp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mlag
CPPFLAGS += -I$(PROTO_DIR)/opm/include
CPPFLAGS += -I$(PROTO_DIR)/opm/include/erps
CPPFLAGS += -I$(PROTO_DIR)/opm/include/g8131
CPPFLAGS += -I$(PROTO_DIR)/opm/include/g8032
CPPFLAGS += -I$(PROTO_DIR)/opm/include/tpoam
CPPFLAGS += -I$(PROTO_DIR)/opm/include/cfm
CPPFLAGS += -I$(TOP_DIR)/lib/task
CPPFLAGS += -I$(TOP_DIR)/redis/deps/hiredis

ifeq ($(PRODUCT),sai)
#CPPFLAGS += -DLDB_SYNC_TO_RDB
endif

LIBFEI       = $(LIB_DIR)/libfei.so
LIBPROTO     = $(LIB_DIR)/libproto.so
LIBCDB       = $(LIB_DIR)/libcdb.so
LIBSDB       = $(LIB_DIR)/libsdb.so
LIBPROTOPBC  = $(LIB_DIR)/libprotopbc.so
LIBTASK      = $(LIB_DIR)/libtask.so
LIBSOCK      = $(LIB_DIR)/libsock.so
LIBCTCLIB    = $(LIB_DIR)/libctclib.so
LIBGENLOG    = $(LIB_DIR)/libgenlog.so
LIBSAL       = $(LIB_DIR)/libsal.so
LIBCJSON     = $(LIB_DIR)/libcjson.so
LIBASN1      = $(LIB_DIR)/libasn1.a
LIBHIREDIS   = $(LIB_DIR)/libhiredis.a
LIBCDBCLIENT = $(LIB_DIR)/libcdbclient.so
LIBDERHEX    = $(LIB_DIR)/libderhex.a
LIBLCMMSG    = $(LIB_DIR)/liblcmmsg.a
LIBMNL       = $(LIB_DIR)/libmnl.so
LIBNFNETLINK = $(LIB_DIR)/libnfnetlink.so
LIBNETFILTER_CONNTRACK = $(LIB_DIR)/libnetfilter_conntrack.so