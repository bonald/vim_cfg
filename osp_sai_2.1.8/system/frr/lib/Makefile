export WARN_NOERROR = y
include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc
include ../../Makefile.env.inc
CFLAGS += -Wno-all

ifeq ($(IS_UML), n)
ifneq ($(CPU), hi3535)
CPPFLAGS += -DHAVE_SETNS
endif
endif

CPPFLAGS += -std=c99

LIB = zebra
SRCS = $(wildcard *.c)

#CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(GLIB_USER)/include
DEP_LIBS = $(LIBJSON)
LD_LIBS = -L$(GLIB_USER)/lib -ldl
LD_LIBS += -L$(LIB_DIR) -ljson -lcrypt -lrt 
#ifneq ($(ARCH),um)
#include $(MK_DIR)/solib.mk
#else
include $(MK_DIR)/lib.mk
#endif
