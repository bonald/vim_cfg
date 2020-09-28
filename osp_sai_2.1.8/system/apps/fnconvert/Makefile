include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

include $(PROTO_DIR)/Makefile.inc

PROG = fnconvert
SRCS = $(wildcard *.c)

CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(CDB_DIR)/core/include
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/lib/genlog/
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(TOP_DIR)/lib/derhex
CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CFLAGS += -I${TOP_DIR}/third_party/precompiled/include

DEP_LIBS = ${LIB_DIR}/libderhex.a
LD_LIBS = 

LD_LIBS += -lrt
LD_LIBS += -L$(LIB_DIR) -lproto -lcdb -lsdb -lctclib -lgenlog -lpthread -lcrypto -lhiredis -lsock -ltask -lsal
LD_LIBS += -lderhex -lsal -lcrypto -lcrypt -lssl
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

include $(MK_DIR)/prog.mk
