include ../../Makefile.env.inc

export WARN_NOERROR = y
include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG= dhclient
SRCS = 
SRCS += $(shell find src -name "*.c")
CURRDIR = $(shell pwd)
SRCS += $(shell find $(CURRDIR)/../../apps/dhcp/client -name "*.c")

CPPFLAGS += -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes -I$(APP_TOP_DIR)/dhcp/client
CPPFLAGS += -DLINUX_MAJOR=2 -DLINUX_MINOR=6 -DLOCALSTATEDIR='"/var"' -DCLIENT_PATH='"/sbin"'
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp/libdhc
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp/client

CPPFLAGS += -I${TOP_DIR}/lib/task/ivykis/src/include

CPPFLAGS += -I$(PROTO_DIR)/dhclient/include
CFLAGS += -I${CTC_LIB_DIR}/derhex

CFLAGS += -I${TOP_DIR}/third_party/precompiled/include

###
DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex
LD_LIBS += -lssl

DEP_LIBS += $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a 
DEP_LIBS += $(LIB_DIR)/libsock.so
LD_LIBS  += -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6 -lrt -lsock
###

### DEP_LIBS =  $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a
### DEP_LIBS += $(LIB_DIR)/$(LIBCTCLIB) $(LIB_DIR)/$(LIBSAL)
### DEP_LIBS += $(LIB_DIR)/$(LIBCJSON)
### #### DEP_LIBS += $(LIB_DIR)/libdhc.a
### LD_LIBS  =  -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6 -ldhc
### LD_LIBS  += -lctclib -lsal -lpthread -lcrypt -lcrypto -lrt -lm -lcjson

### ifeq ($(ENABLE_SMARTCFG),yes)
### SRCS+=$(wildcard ../../apps/dhcp/clien/scfgctl/*.c)
### CPPFLAGS += -I$(APP_TOP_DIR)/dhcp/client/scfgctl
### ## CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
### CPPFLAGS += -I$(TOP_DIR)/include
### endif	
### 
### SCRIPT_FILE=dhclient6-script
### LOCAL_SCRIPT_FILE=./scripts/linux
### 
### all: $(BIN_DIR)/$(PROG) $(BIN_DIR)/$(SCRIPT_FILE) FORCE
### 
### EXTRA_CLEANS=$(BIN_DIR)/$(SCRIPT_FILE)
### 
### $(BIN_DIR)/$(SCRIPT_FILE): $(LOCAL_SCRIPT_FILE)
### 	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
### 	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
### 	cp $(LOCAL_SCRIPT_FILE) $@
### 
include $(MK_DIR)/prog.mk
