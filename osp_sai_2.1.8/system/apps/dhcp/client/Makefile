include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk

PROG= dhclient
SRCS= $(wildcard *.c)

SRCS+=$(wildcard scfgctl/*.c)
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp/client/scfgctl
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp/libscmsg/gen
CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I$(CTC_LIB_DIR)/include

CPPFLAGS += -I$(ZEBOS_TOP_DIR)/chsm/include
CPPFLAGS+= -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS+= -I$(APP_TOP_DIR)/dhcp/libdhc
CPPFLAGS+= -DLINUX_MAJOR=2 -DLINUX_MINOR=6
CPPFLAGS+= -DLOCALSTATEDIR='"/var"' -DCLIENT_PATH='"/sbin"'

DEP_LIBS= $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a
DEP_LIBS+=$(LIB_DIR)/$(LIBCTCLIB) $(LIB_DIR)/$(LIBSAL)
DEP_LIBS+=$(LIB_DIR)/$(LIBZOS) $(LIB_DIR)/$(LIBPAL)
DEP_LIBS+=$(LIB_DIR)/libdhc.a
LD_LIBS= -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6 -ldhc
LD_LIBS+=-lzos  -lctclib -lsal -lpthread -lpal -lcrypt -lcrypto -lrt

ifeq ($(IS_UML),y)
DEP_LIBS+=$(LIB_DIR)/libasn1.a $(LIB_DIR)/libscmsg.a
LD_LIBS+=-lscmsg -lasn1
else
DEP_LIBS+=$(LIB_DIR)/libasn1.so $(LIB_DIR)/libscmsg.a
LD_LIBS+=-lscmsg -lasn1 -lm
endif

SCRIPT_FILE=dhclient6-script
LOCAL_SCRIPT_FILE=./scripts/linux

all: $(BIN_DIR)/$(PROG) $(BIN_DIR)/$(SCRIPT_FILE) FORCE

EXTRA_CLEANS=$(BIN_DIR)/$(SCRIPT_FILE)

$(BIN_DIR)/$(SCRIPT_FILE): $(LOCAL_SCRIPT_FILE)
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $(LOCAL_SCRIPT_FILE) $@

include $(MK_DIR)/prog.mk

.PHONY: FORCE
FORCE:
