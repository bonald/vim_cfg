ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk

OBJ_DIR=$(OBJ_BASE_DIR)/insight
__LIB_DIR := $(LIB_DIR)


PROG = insight

SRCS = $(shell find apps/ -name "*.c")
SRCS += $(shell find common/ -name "*.c")
SRCS += $(shell find nb_plugin/ -name "*.c")
SRCS += $(shell find sb_plugin/ -name "*.c")
SRCS += $(shell find vendor/ -name "*.c")

CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += -I$(SDK_DIR)/core/
CPPFLAGS += -I$(SDK_DIR)/insight/common
CPPFLAGS += -I$(SDK_DIR)/insight/apps
CPPFLAGS += -I$(SDK_DIR)/insight/nb_plugin
CPPFLAGS += -I$(SDK_DIR)/insight/sb_plugin
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/cjson
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/libxml2
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/libxml2/include
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/libxml2/include/libxml

LIBAPP    = $(LIB_DIR)/libapp.a 
LIBCTCSDK = $(LIB_DIR)/libctcsdk.a

ifeq ($(BOARD),ctc-sim)
LIBMEMMODEL = $(LIB_DIR)/libmemmodel.a
LIBCTCLI  = $(LIB_DIR)/libcmodelcli.a
LIBCMODEL = $(LIB_DIR)/libcmodel.a
LIBMODELCPP = $(LIB_DIR)/libmodelcpp.so
endif


LD_FLAGS += -Wl,--rpath=$(RPATH)
LD_LIBS = -L$(__LIB_DIR) -L$(LIB_DIR) -lapp -lctcsdk -lapp

ifeq ($(BOARD),ctc-sim)
ifneq ($(CHIPNAME), $(filter $(CHIPNAME), greatbelt))
DEP_LIBS += $(LIBMODELCPP) $(LIBCMODEL)
LD_LIBS += -Wl,--start-group,-lmodelcpp,-lcmodel,-lcmodelcli,--end-group -ldl
else
DEP_LIBS += $(LIBCMODEL)
LD_LIBS += -Wl,--start-group,-lcmodel,-lcmodelcli,--end-group -ldl
endif
ifneq ($(USW),yes)
DEP_LIBS += $(LIBMEMMODEL)
LD_LIBS += -lmemmodel
endif
endif

DEP_LIBS += $(LIBCTCSDK)
ifeq ($(BOARD),ctc-sim)
LD_LIBS += -Wl,--start-group,-lctcsdk,--end-group -lcmodelcli -lpthread -lrt -lm -dl
else
LD_LIBS += -Wl,--start-group,-lapp,-lctcsdk,-lapp,--end-group -lpthread -lrt -lm -dl 
endif



ifeq ($(USW),yes)
ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -I/opt/openssl-1.0.2c/usr/local/include/openssl
ifeq ($(M64),TRUE)
LIB_LCRYPTO_DIR := /opt/openssl-1.0.2c/usr/local/lib64
LD_LIBS += -L$(LIB_LCRYPTO_DIR) -lcrypto -ldl -lstdc++
else
LIB_LCRYPTO_DIR := /opt/openssl-1.0.2c/usr/local/lib
LD_LIBS += -L$(LIB_LCRYPTO_DIR) -lcrypto -ldl -lstdc++ -lapp
endif
endif
endif



include $(MK_DIR)/prog.mk

else

OBJ_DIR=$(OBJ_BASE_DIR)/insight
LIB = insight
SRCS = $(shell find apps/ -name "*.c")
SRCS += $(shell find common/ -name "*.c")
SRCS += $(shell find nb_plugin/ -name "*.c")
SRCS += $(shell find sb_plugin/ -name "*.c")
SRCS += $(shell find vendor/ -name "*.c")

CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += -I$(SDK_DIR)/core/
CPPFLAGS += -I$(SDK_DIR)/insight/common
CPPFLAGS += -I$(SDK_DIR)/insight/apps
CPPFLAGS += -I$(SDK_DIR)/insight/nb_plugin
CPPFLAGS += -I$(SDK_DIR)/insight/sb_plugin
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/cjson
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/libxml2
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/libxml2/include
CPPFLAGS += -I$(SDK_DIR)/insight/vendor/libxml2/include/libxml


include $(MK_DIR)/lib.mk


endif

