include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = ccs
SRCS = 
SRCS += $(shell find src -name "*.c")

CFLAGS += -I${CTC_LIB_DIR}/derhex

DEP_LIBS = $(LIBPROTOPBC) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
LD_LIBS = -L$(LIB_DIR) -lprotopbc -lproto -lcdb -lsdb -lsock -ltask -lctclib -lgenlog -lsal -lrt -ldl -lnsl -lm -lhiredis -lpthread -lcrypt 
LD_LIBS += -lderhex
LD_LIBS += -lssl

LD_LIBS += -lcrypto

include $(MK_DIR)/prog.mk
