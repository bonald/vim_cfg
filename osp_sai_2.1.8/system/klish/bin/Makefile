include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

PROG = klish
SRCS = 
SRCS += clish.c clish_pam.c

CPPFLAGS += -I$(KLISH_DIR)

ifeq ($(ARCH),um)
CFLAGS += -I${TOP_DIR}/third_party/precompiled/include
endif
CFLAGS += -I${TOP_DIR}/lib/sal/include

include $(PROTO_DIR)/Makefile.inc

LIBCLISH = $(LIB_DIR)/libclish.a
LIBLUB = $(LIB_DIR)/liblub.a
LIBTINYRL = $(LIB_DIR)/libtinyrl.a
LIBKONF = $(LIB_DIR)/libkonf.a

ifeq ($(IS_UML), y)
LIBEXPAT = /usr/lib/libexpat.a
endif
ifeq ($(CPU),hi3535)
LIBEXPAT = $(THIRD_PARTY_DIR)/precompiled/hi3535/lib/libexpat.a
endif
ifeq ($(CPU),p1010)
LIBEXPAT = $(THIRD_PARTY_DIR)/expat/lib/libexpat.a
endif
ifeq ($(CPU),x86_64)
LIBEXPAT = $(THIRD_PARTY_DIR)/precompiled/x86_64/lib/libexpat.a
endif
ifeq ($(CPU),ls1023a)
LIBEXPAT = $(THIRD_PARTY_DIR)/precompiled/arm64/lib/libexpat.a
endif
ifeq ($(CPU),ctc5236)
LIBEXPAT = $(THIRD_PARTY_DIR)/precompiled/arm64/lib/libexpat.a
endif

DEP_LIBS += $(LIBCLISH) $(LIBLUB) $(LIBTINYRL) $(LIBKONF) $(LIBEXPAT)

ifeq ($(IS_UML), y)
LD_LIBS += -L$(LIB_DIR) -lclish -llub -ltinyrl -lkonf -lexpat -lrt 
endif
ifeq ($(CPU),hi3535)
LD_LIBS += -L$(LIB_DIR) -L$(THIRD_PARTY_DIR)/precompiled/hi3535/lib/ -lclish -llub -ltinyrl -lkonf -lexpat -lrt 
endif
ifeq ($(CPU),p1010)
LD_LIBS += -L$(LIB_DIR) -L$(THIRD_PARTY_DIR)/expat/lib -lclish -llub -ltinyrl -lkonf -lexpat -lrt 
endif
ifeq ($(CPU),x86_64)
LD_LIBS += -L$(LIB_DIR) -L$(THIRD_PARTY_DIR)/precompiled/x86_64/lib -lclish -llub -ltinyrl -lkonf -lexpat -lrt 
endif
ifeq ($(CPU),ls1023a)
LD_LIBS += -L$(LIB_DIR) -L$(THIRD_PARTY_DIR)/precompiled/arm64/lib -lclish -llub -ltinyrl -lkonf -lexpat -lrt 
endif
ifeq ($(CPU),ctc5236)
LD_LIBS += -L$(LIB_DIR) -L$(THIRD_PARTY_DIR)/precompiled/arm64/lib -lclish -llub -ltinyrl -lkonf -lexpat -lrt 
endif

LD_LIBS += -lproto -lctclib -lgenlog -lpthread -lcrypto -ltask -lm -lsal

ifeq ($(ARCH),um)
ifeq ($(CPU),i686)
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/uml -lpamctc -lpamctc_misc
else
LD_LIBS += -L${TOP_DIR}/third_party/precompiled/uml/64bit -lpamctc -lpamctc_misc
endif
else
LD_LIBS += -lpam -lpam_misc
endif

LIBCDBCLIENT = $(LIB_DIR)/libcdbclient.so
LIBPROTO     = $(LIB_DIR)/libproto.so
LIBCDB       = $(LIB_DIR)/libcdb.so
LIBPROTOPBC  = $(LIB_DIR)/libprotopbc.so
LIBTASK      = $(LIB_DIR)/libtask.so
LIBSOCK      = $(LIB_DIR)/libsock.so
LIBCTCLIB    = $(LIB_DIR)/libctclib.so
LIBGENLOG    = $(LIB_DIR)/libgenlog.so
LIBSAL       = $(LIB_DIR)/libsal.so
DEP_LIBS += $(LIBCDBCLIENT) $(LIBPROTOPBC) $(LIBPROTO) $(LIBCDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
DEP_LIBS += $(LIB_DIR)/libderhex.a
LD_LIBS += -L$(LIB_DIR) -lcdbclient -lprotopbc -lproto -lcdb -lsdb -lsock -ltask -lctclib -lgenlog -lhiredis -lsal
LD_LIBS += -lderhex -lcrypt -lssl -lcrypto

include $(MK_DIR)/prog.mk
