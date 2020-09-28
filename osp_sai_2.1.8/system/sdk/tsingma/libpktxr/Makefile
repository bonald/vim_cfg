######################################set linux compile env #############################
ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk
OBJ_DIR=$(OBJ_BASE_DIR)/libpktxr
LIB = pkt
SRCS =
SRCS += $(shell find src -name "*.c")

__LIB_DIR := $(LIB_DIR)

CPPFLAGS += -I$(SDK_DIR)/libpktxr/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/libpktxr/src

CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/mem_model/$(n)/include)
LIBMEMMODEL = $(LIB_DIR)/libmemmodel.a
LIBZEBRACTCCLI  = $(LIB_DIR)/liblibctccli.a
LIBPKT  = $(LIB_DIR)/libpkt.a

include $(MK_DIR)/lib.mk

endif
