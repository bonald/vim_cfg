
export WARN_NOERROR = y
include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc
include ../../Makefile.env.inc

CPPFLAGS += -std=c99

PROG = ospfd
SRCS = $(wildcard *.c)

CPPFLAGS += -I$(GLIB_USER)/include
LD_LIBS = -L$(GLIB_USER)/lib -ldl

LIBZEBRA = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBJSON) $(LIBZEBRA)
LD_LIBS += -L$(LIB_DIR) -ljson -lzebra -lreadline -ltermcap -lgenlog -ltask -lsal -lcrypt -lrt 

include $(MK_DIR)/prog.mk
