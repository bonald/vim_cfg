include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = cds
SRCS = 
SRCS += $(shell find src -name "*.c")

DEP_LIBS = $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lsock -ltask -lctclib -lgenlog -lsal -lrt -ldl -lnsl -lm -lhiredis -lpthread -lcrypt -lcrypto

include $(MK_DIR)/prog.mk
