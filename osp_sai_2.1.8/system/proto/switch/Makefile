include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = switch
SRCS = 
SRCS += $(shell find src -name "*.c")

CPPFLAGS += -I$(PROTO_DIR)/switch/include
CPPFLAGS += -I$(PROTO_DIR)/switch/include/openflow
CPPFLAGS += -I$(PROTO_DIR)/switch/include/tap
CPPFLAGS += -I$(PROTO_DIR)/switch/include/aclm
CPPFLAGS += -I$(PROTO_DIR)/switch/include/mlag
CPPFLAGS += -I$(PROTO_DIR)/switch/include/ipsg
CPPFLAGS += -I$(PROTO_DIR)/switch/include/rt_if
CPPFLAGS += -I$(PROTO_DIR)/switch/include/acl_policy
CPPFLAGS += -I$(PROTO_DIR)/switch/include/ipsour
CPPFLAGS += -I$(PROTO_DIR)/switch/include/copp
CPPFLAGS += -I$(PROTO_DIR)/switch/include/l2protocol
CPPFLAGS += -I$(PROTO_DIR)/switch/include/hash

CPPFLAGS += -I$(PROTO_DIR)/dhcsnooping/include
CFLAGS += -I${CTC_LIB_DIR}/derhex

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)

LD_LIBS = -L$(LIB_DIR) 
LD_LIBS += -lproto -lcjson -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex -lssl

ifeq ($(ENABLE_NETCONF),yes)
CPPFLAGS += $(NETCONF_CFLAGS)
DEP_LIBS += $(addprefix $(LIB_DIR)/,$(NETCONF_DEP_LIBS))
LD_LIBS += $(NETCONF_LD_LIBS)
endif

include $(MK_DIR)/prog.mk
