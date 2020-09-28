include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = appcfg
SRCS = 
SRCS += $(shell find src -name "*.c")

CPPFLAGS += -I$(PROTO_DIR)/appcfg/include
CFLAGS += -I${CTC_LIB_DIR}/derhex

CFLAGS += -I${TOP_DIR}/third_party/precompiled/include
CFLAGS += -I${TOP_DIR}/lib/include

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBCDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lpthread -lhiredis -lcrypt -lcrypto
LD_LIBS += -lderhex
LD_LIBS += -lssl
ifeq ($(ARCH),um)
ifeq ($(CPU),i686)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/uml -lssh -lopenbsd-compat -lssh
else
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/uml/64bit -lssh -lopenbsd-compat
endif
endif
ifeq ($(CPU),hi3535)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/hi3535/lib -lssh -lopenbsd-compat
endif
ifeq ($(CPU),p1010)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/p1010/lib -lssh -lopenbsd-compat
endif
ifeq ($(CPU),x86_64)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/x86_64/lib -lssh -lopenbsd-compat
endif
ifeq ($(CPU),ls1023a)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/arm64/lib -lssh -lopenbsd-compat
endif
ifeq ($(CPU),ctc5236)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/arm64/lib -lssh -lopenbsd-compat
endif

ifeq ($(ENABLE_NETCONF),yes)
CPPFLAGS += $(NETCONF_CFLAGS)
DEP_LIBS += $(addprefix $(LIB_DIR)/,$(NETCONF_DEP_LIBS))
LD_LIBS += $(NETCONF_LD_LIBS)
endif

include $(MK_DIR)/prog.mk
