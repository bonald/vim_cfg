include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = ncs
SRCS = 
SRCS += $(shell find src -name "*.c")

CFLAGS += -I$(PROTO_DIR)/ncs/include
CFLAGS += -I${CTC_LIB_DIR}/derhex

DEP_LIBS = $(LIBPROTOPBC) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
LD_LIBS = -L$(LIB_DIR) -lprotopbc -lproto -lcdb -lsdb -lsock -ltask -lctclib -lgenlog -lsal -lrt -ldl -lnsl -lm -lhiredis -lpthread -lcrypt 
LD_LIBS += -lderhex
LD_LIBS += -lssl

LD_LIBS += -lcrypto

#ifeq ($(ENABLE_NETCONF),yes)
CPPFLAGS += $(NETCONF_CFLAGS)
DEP_LIBS += $(addprefix $(LIB_DIR)/,$(NETCONF_DEP_LIBS))
LD_LIBS += $(NETCONF_LD_LIBS)
#endif

include $(MK_DIR)/prog.mk
