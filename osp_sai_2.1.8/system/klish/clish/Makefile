include $(MK_DIR)/sys.mk

LIB = clish
SRCS = 
SRCS += $(wildcard action/*.c) 
SRCS += $(wildcard command/*.c) 
SRCS += $(wildcard config/*.c) 
SRCS += $(wildcard hotkey/*.c) 
SRCS += $(wildcard nspace/*.c) 
SRCS += $(wildcard param/*.c) 
SRCS += $(wildcard pargv/*.c) 
SRCS += $(wildcard ptype/*.c) 
SRCS += $(wildcard shell/*.c) 
SRCS += $(wildcard var/*.c) 
SRCS += $(wildcard view/*.c) 
SRCS += $(wildcard *.c)

CPPFLAGS += -I$(KLISH_DIR)
CPPFLAGS += -I$(KLISH_DIR)/clish
CPPFLAGS += -I$(THIRD_PARTY_DIR)/expat/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(CDB_DIR)/core/include
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/lib/genlog/
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CFLAGS += -I${TOP_DIR}/lib/sal/include
CFLAGS += -I${TOP_DIR}/lib/derhex/

include $(PROTO_DIR)/Makefile.inc

include $(MK_DIR)/lib.mk
