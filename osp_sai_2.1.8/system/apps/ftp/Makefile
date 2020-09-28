include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

PROG = ctcftp
SRCS = $(wildcard *.c)

CPPFLAGS += -I${TOP_DIR}/lib/derhex

DEP_LIBS = ${LIB_DIR}/libderhex.a
LD_LIBS = -L${LIB_DIR} -lderhex -lcrypto -lssl -lcrypt

include $(MK_DIR)/prog.mk
