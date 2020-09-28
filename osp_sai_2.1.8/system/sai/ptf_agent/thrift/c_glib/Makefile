include $(MK_DIR)/sys64.mk

LIB = thrift
SRCS = $(shell find . -name "*.c")

ifeq ($(ARCH),um)
# um is m32, then could not uses -I/usr/lib64/glib-2.0/include
# GLIB_INCDIR=$(shell pkg-config --cflags glib-2.0)
# GLIB_INCDIR=-I/usr/include/glib-2.0 -I$(THIRD_PARTY_DIR)/thrift/c_glib/lib32
GLIB_INCDIR=-I/ctcdata/users/sw/kcao/usr/include/glib-2.0 -I/ctcdata/users/sw/kcao/usr/lib64/glib-2.0/include 
GLIB_LIB= -lgobject-2.0 -lglib-2.0
endif

ifeq ($(ARCH),powerpc)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
GLIB_LIB= -lgobject-2.0 -lglib-2.0
endif

ifeq ($(ARCH),mips)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
GLIB_LIB= -lgobject-2.0 -lglib-2.0
endif

CPPFLAGS += -I.
CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(THIRD_PARTY_DIR)
CPPFLAGS += -I$(THIRD_PARTY_DIR)/thrift/c_glib
CPPFLAGS += $(GLIB_INCDIR)

include $(MK_DIR)/solib64.mk
