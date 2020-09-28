ifeq ($(targetbase),linux)

include ../Makefile.env.inc
include $(MK_DIR)/sys.mk

LIB=sai
SRCS = $(wildcard *.c)
SRCS += $(shell find src/common -name "*.c")
SRCS += $(shell find src/sai -name "*.c")
ifeq ($(CHIPNAME),tsingma)
SRCS += $(shell find src/ctccfg/tsingma -name "*.c")
else 
SRCS += $(shell find src/ctccfg/gg_gb_d2 -name "*.c")
endif
SRCS += $(shell find src/switch -name "*.c")
SRCS += $(shell find src/vlan -name "*.c")
SRCS += $(shell find src/mirror -name "*.c")
SRCS += $(shell find cli/src -name "*.c")
SRCS += $(shell find src/port -name "*.c")
SRCS += $(shell find src/route/ -name "*.c")
SRCS += $(shell find src/lag/ -name "*.c")
SRCS += $(shell find src/stp/ -name "*.c")
SRCS += $(shell find src/fdb/ -name "*.c")
SRCS += $(shell find src/router -name "*.c")
SRCS += $(shell find src/routerintf -name "*.c")
SRCS += $(shell find src/neighbor  -name "*.c")
SRCS += $(shell find src/nexthop  -name "*.c")
SRCS += $(shell find src/nexthopgroup  -name "*.c")
SRCS += $(shell find src/hostif -name "*.c")
SRCS += $(shell find src/acl -name "*.c")
SRCS += $(shell find src/aps_group -name "*.c")
SRCS += $(shell find src/oam  -name "*.c")
ifeq ($(CHIPNAME),tsingma)
SRCS += $(shell find src/qos -path src/qos/ctc_sai_qosmaps.c -prune -o -name "*.c" -print)
else ifeq ($(CHIPNAME),duet2)
SRCS += $(shell find src/qos -path src/qos/ctc_sai_qosmaps.c -prune -o -name "*.c" -print)
else 
SRCS += $(shell find src/qos -path src/qos/ctc_sai_d2_qosmaps.c -prune -o -name "*.c" -print)
endif
SRCS += $(shell find src/hash -name "*.c")
SRCS += $(shell find src/isolation -name "*.c")
SRCS += $(shell find src/openflow -name "*.c")
SRCS += $(shell find src/vlanclass -name "*.c")
SRCS += $(shell find src/macescape -name "*.c")
ifeq ($(PRODUCT), tap)
SRCS += $(shell find src/tap_group -name "*.c")
endif
SRCS += $(shell find src/copp -name "*.c")
SRCS += $(shell find src/udf -name "*.c")
SRCS += $(shell find src/ptp -name "*.c")
SRCS += $(shell find src/warmboot -name "*.c")
SRCS += $(shell find src/ipmc -name "*.c")
SRCS += $(shell find src/vlanmapping -name "*.c")

#CFLAGS += -Werror -Wall
#CFLAGS += -Wall
CFLAGS += -I$(CTC_LIB_DIR)/include
CFLAGS += -I$(TOP_DIR)/lib/genlog
CFLAGS += -I$(TOP_DIR)/lib/sal/include
CFLAGS += -I$(TOP_DIR)/lib/util/include
CFLAGS += -I$(TOP_DIR)/lib/task
CFLAGS += -I$(SDB_DIR)/core/include
CFLAGS += -I$(SAI_DIR)/include
CFLAGS += -I$(SAI_DIR)/inc
CFLAGS += -I$(SAI_DIR)/src/lag
CFLAGS += -I$(SAI_DIR)/src/acl
CFLAGS += -I$(SAI_DIR)/src/aps_group
CFLAGS += -I$(SAI_DIR)/src/mirror
CFLAGS += -I$(SAI_DIR)/src/nexthop
CFLAGS += -I$(SAI_DIR)/src/hostif
CFLAGS += -I$(SAI_DIR)/src/nexthopgroup
CFLAGS += -I$(SAI_DIR)/src/common
ifeq ($(CHIPNAME),tsingma)
CFLAGS += -I$(SAI_DIR)/src/ctccfg/tsingma
else 
CFLAGS += -I$(SAI_DIR)/src/ctccfg/gg_gb_d2
endif
CFLAGS += -I$(SAI_DIR)/src/switch
CFLAGS += -I$(SAI_DIR)/src/sai
CFLAGS += -I$(SAI_DIR)/src/router
CFLAGS += -I$(SAI_DIR)/src/routerintf
CFLAGS += -I$(SAI_DIR)/src/neighbor
CFLAGS += -I$(SAI_DIR)/src/fdb
CFLAGS += -I$(SAI_DIR)/src/vlan
CFLAGS += -I$(SAI_DIR)/src/route
CFLAGS += -I$(SAI_DIR)/src/port
CFLAGS += -I$(SAI_DIR)/src/stp
CFLAGS += -I$(SAI_DIR)/src/qos
CFLAGS += -I$(SAI_DIR)/src/hash
CFLAGS += -I$(SAI_DIR)/src/isolation
CFLAGS += -I$(SAI_DIR)/src/openflow
CFLAGS += -I$(SAI_DIR)/src/vlanclass
CFLAGS += -I$(SAI_DIR)/src/macescape
CFLAGS += -I$(SAI_DIR)/src/tap_group
CFLAGS += -I$(SAI_DIR)/src/copp
CFLAGS += -I$(SAI_DIR)/src/udf
CFLAGS += -I$(SAI_DIR)/src/ptp
CFLAGS += -I$(SAI_DIR)/src/warmboot
CFLAGS += -I$(SAI_DIR)/src/ipmc
CFLAGS += -I$(SAI_DIR)/src/vlanmapping
CFLAGS += -I$(SAI_DIR)/src/oam
CFLAGS += -I$(SAI_DIR)/cli/include

CFLAGS += -I$(SDK_DIR)/libctccli/include
CFLAGS += -I$(SDK_DIR)/ctccli/sdkcli/include
CFLAGS += -I$(SDK_DIR)/ctccli/sdkcli
CFLAGS += -I$(SDK_DIR)/core/goldengate/include/sys
CFLAGS += -I$(SDK_DIR)/sal/include
CFLAGS += -I$(SDK_DIR)/dal/include
# sdk header files disperse in these directories
CFLAGS += -I$(SDK_DIR)/core
CFLAGS += -I$(SDK_DIR)/core/common/include
CFLAGS += -I$(SDK_DIR)/core/api/include
CFLAGS += -I$(SDK_DIR)/core/init

CPPFLAGS += -I$(CTC_LIB_DIR)/genlog

LIBGENLOG    = $(LIB_DIR)/libgenlog.so

include $(MK_DIR)/lib.mk

endif
