include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = opm
SRCS = 
SRCS += $(shell find src -name "*.c")

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)

LD_LIBS = -L$(LIB_DIR) 
LD_LIBS += -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lpthread -lhiredis -lcrypt -lcrypto
LD_LIBS += -lderhex -lssl

include $(MK_DIR)/prog.mk
