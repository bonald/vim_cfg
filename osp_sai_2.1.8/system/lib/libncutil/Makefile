include $(MK_DIR)/sys.mk

LIB = ncutil
SRCS = $(wildcard *.c)

CPPFLAGS += $(NETCONF_CFLAGS)

### proto header path
include $(PROTO_DIR)/Makefile.inc

DEP_LIBS += $(addprefix $(LIB_DIR)/,$(NETCONF_DEP_LIBS))

include $(MK_DIR)/lib.mk
