export WARN_NOERROR = y 
include $(MK_DIR)/sys.mk

LIB = hiredis
SRCS = $(wildcard *.c)

CFLAGS = -Wno-error
CPPFLAGS += adapters

include $(MK_DIR)/lib.mk
