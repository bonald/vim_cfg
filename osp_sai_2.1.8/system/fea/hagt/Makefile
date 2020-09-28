include $(TOP_DIR)/Makefile.env.inc
include $(MK_DIR)/sys.mk

LIB = hagt
SRCS += $(shell find $(CHIPNAME) -name "*.c")

CPPFLAGS += -I$(TOP_DIR)/lib/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/ipc
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(CTC_LIB_DIR)/libasn1
CPPFLAGS += -I$(CTC_LIB_DIR)/msg/hagtmsg/gen

CPPFLAGS += -I$(CDB_DIR)/core/include
CPPFLAGS += -I$(SDB_DIR)/core/include

CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/common
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/intf
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/l2
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/l3
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/mpls
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/aclqos
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/misc
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/openflow
CPPFLAGS += -I$(PROTO_DIR)/openflow/adapt/lib
CPPFLAGS += -I$(FEA_DIR)/hagt/$(CHIPNAME)/include/tap
CPPFLAGS += -I$(FEA_DIR)/lcm/include
CPPFLAGS += -I$(SDK_DIR)/core/init
CPPFLAGS += -I$(CMODEL_DIR)/core/include

CPPFLAGS += -I$(SDK_DIR)/core
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/libctccli/include
include $(MK_DIR)/lib.mk
