include $(MK_DIR)/sys.mk

LIB = sal
SRCS = $(shell find src/linux_um src/common -name "*.c")

CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC

include $(MK_DIR)/solib.mk

CFLAGS += -Werror -Wall
