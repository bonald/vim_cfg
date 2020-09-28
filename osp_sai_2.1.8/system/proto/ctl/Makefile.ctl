include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = $(DEST_CTL)
SRCS = 
SRCS += $(DEST_CTL).c 

DEP_LIBS = $(LIBCDBCLIENT) $(LIBPROTOPBC) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lcdbclient -lprotopbc -lproto -lcdb -lsdb -lsock -ltask -lctclib -lgenlog -lsal -lhiredis -lm -lrt -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex -lssl

include $(MK_DIR)/prog.mk
