include $(MK_DIR)/sys.mk
include ./clish/gen.mk

PROG = lcsh
SRCS = $(wildcard *.c)
SRCS += $(wildcard clish/*.c)
SRCS += $(wildcard lcmtlk/*.c)

ifeq ($(OS_TYPE),cnos)
UPM_DIR = $(FEA_DIR)
endif
ifeq ($(OS_TYPE),ipi)
UPM_DIR = $(LC_DIR)
endif



CPPFLAGS += -I.
ifeq ($(OS_TYPE),cnos)
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(UPM_DIR)/lcm/lcmmsg/gen
CPPFLAGS += -I$(CTC_LIB_DIR)/libasn1
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/include
endif
ifeq ($(OS_TYPE),ipi)
CPPFLAGS += -I$(SAL_TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/KAL/include
CPPFLAGS += -I$(TOP_DIR)/lib/Utility/include
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(UPM_DIR)/lcm/lcmmsg/gen
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I$(TOP_DIR)/Platforms/lib/lcmmsg/gen
endif

CPPFLAGS += -I$(UPM_DIR)/lcm/include
CPPFLAGS += -I$(UPM_DIR)/lcm/lcsh/include

CPPFLAGS += -I./clish -D_SYS_LC_

ifeq ($(OS_TYPE),cnos)
DEP_LIBS += $(LIB_DIR)/libsal.so
DEP_LIBS += $(LIB_DIR)/libgenlog.so
DEP_LIBS += $(LIB_DIR)/$(LIBCTCLIB)
endif
ifeq ($(OS_TYPE),ipi)
ifneq ($(ARCH),um)
DEP_LIBS += $(LIB_DIR)/libsal.so
DEP_LIBS += $(LIB_DIR)/libctclib.so
else
DEP_LIBS += $(LIB_DIR)/libsal.a
DEP_LIBS += $(LIB_DIR)/$(LIBCTCLIB)
endif
endif


LD_LIBS  += -L$(LIB_DIR) -lctclib -lsal -lpthread -lrt
ifeq ($(OS_TYPE),cnos)
LD_LIBS  += -L$(LIB_DIR) -ltask -lgenlog
endif

include $(MK_DIR)/prog.mk
