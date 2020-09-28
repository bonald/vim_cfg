include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk
include $(PROTO_DIR)/Makefile.inc

LIB = netfilter_conntrack
SRCS = $(wildcard ./src/*.c)
SRCS += $(wildcard ./src/conntrack/*.c)
SRCS += $(wildcard ./src/expect/*.c)

CFLAGS += -I../libmnl-1.0.4/include/
CFLAGS += -I../libnfnetlink-1.0.1/include/
CFLAGS += -I. -I./src
CFLAGS += -I./include
CFLAGS += "-Wno-error"

LDFLAGS += 

DEP_LIBS += $(LIB_DIR)/libnfnetlink.so $(LIB_DIR)/libmnl.so
LD_LIBS += -L$(LIB_DIR)
LD_LIBS += -lnfnetlink -lmnl

include $(MK_DIR)/solib.mk