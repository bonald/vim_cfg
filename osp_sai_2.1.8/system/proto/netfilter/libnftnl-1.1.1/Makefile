include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk
include $(PROTO_DIR)/Makefile.inc

LIB = nftnl
SRCS = $(wildcard ./src/*.c)

CFLAGS += -I. -I./src
CFLAGS += -I./include
CFLAGS += "-Wno-error"

DEP_LIBS += $(LIB_DIR)/libmnl.so
LD_LIBS += -L$(LIB_DIR)
LD_LIBS += -lmnl

include $(MK_DIR)/solib.mk