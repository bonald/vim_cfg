include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

PROG = scfgd
SRCS = $(wildcard *.c)

CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I../libscmsg/gen
CPPFLAGS += -I../libdhc/
CPPFLAGS += -I./gen

EXTRA_CLEANS=gen $(LIB_DIR)/libscparser.a

DEP_LIBS = $(LIB_DIR)/$(LIBZOS) $(LIB_DIR)/$(LIBPAL) $(LIB_DIR)/$(LIBCTCLIB) $(LIB_DIR)/$(LIBSAL)
ifeq ($(IS_UML),y)
DEP_LIBS += $(LIB_DIR)/libasn1.a $(LIB_DIR)/libscmsg.a
else
DEP_LIBS += $(LIB_DIR)/libasn1.so $(LIB_DIR)/libscmsg.a
endif
DEP_LIBS += $(LIB_DIR)/libdhc.a $(LIB_DIR)/libscparser.a
LD_LIBS = -L$(LIB_DIR) -lzos -lctclib -lsal -lpthread -lpal -lcrypt -lcrypto -lrt
LD_LIBS += -ldhc -lscmsg -lscparser -lasn1
ifneq ($(IS_UML),y)
LD_LIBS+=-lm
endif

include $(MK_DIR)/prog.mk

.PHONY: FORCE
FORCE:
