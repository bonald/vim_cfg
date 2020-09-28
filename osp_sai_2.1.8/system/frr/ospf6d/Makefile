
export WARN_NOERROR = y
include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc
#include ../../Makefile.env.inc

#CPPFLAGS += -std=c99

PROG = ospf6d
SRCS = $(wildcard *.c)

LIBFRR = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBJSON) $(LIBFRR)
LD_LIBS = -L$(LIB_DIR) -lzebra 
LD_LIBS += -ldl
LD_LIBS += -L$(LIB_DIR) -ljson -lcrypt -lrt

include $(MK_DIR)/prog.mk
