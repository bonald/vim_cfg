include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = routed
SRCS = 
SRCS += $(shell find src -name "*.c")

CPPFLAGS += -I$(PROTO_DIR)/route/include

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)

LD_LIBS = -L$(LIB_DIR) -lproto -lcjson -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto

ifeq ($(ARCH), arm64)
DEP_LIBS += $(LIBMNL) $(LIBNFNETLINK) $(LIBNETFILTER_CONNTRACK)
LD_LIBS += -lmnl -lnfnetlink -lnetfilter_conntrack
endif

ifeq ($(IS_UML),y)
ifeq ($(CPU), x86_64)
ifeq ($(CHIPNAME), tsingma)
DEP_LIBS += $(LIBMNL) $(LIBNFNETLINK) $(LIBNETFILTER_CONNTRACK)
LD_LIBS += -lmnl -lnfnetlink -lnetfilter_conntrack
endif
endif
endif

DEP_LIBS = $(LIBZEBRA)
LD_LIBS += -L$(LIB_DIR) -lvtyclt -lzebra -lreadline -ltermcap -lcrypt -lrt -ljson 
LD_LIBS += -L$(GLIB_USER)/lib -ldl

include $(MK_DIR)/prog.mk
