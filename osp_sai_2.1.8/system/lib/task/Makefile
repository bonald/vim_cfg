include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

LIB = task
SRCS = $(wildcard ivykis/src/*.c)
SRCS += ctc_task.c

CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/lib/task
CPPFLAGS += -I$(TOP_DIR)/lib/task/ivykis/src
CPPFLAGS += -I$(TOP_DIR)/lib/task/ivykis/src/include

include $(MK_DIR)/solib.mk
