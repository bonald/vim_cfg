include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

REBOOT_SCRIPT=reboot_fast.sh

PROG = bhm
SRCS = 
SRCS += $(shell find src -name "*.c")

ifeq ($(CPU),hi3535)
MY_CPU_DIR=hi3535
endif

ifeq ($(CPU),p1010)
MY_CPU_DIR=powerpc
endif

ifeq ($(CPU),p1014)
MY_CPU_DIR=powerpc
endif

ifeq ($(CPU),ls1023a)
MY_CPU_DIR=ls1023a
endif

ifeq ($(CPU),ctc5236)
MY_CPU_DIR=ctc5236
endif

ifeq ($(CPU),x86_64)
MY_CPU_DIR=x86_64
endif

CFLAGS += -I$(PROTO_DIR)/bhm/include
CFLAGS += -I${TOP_DIR}/lib/include
CFLAGS += -I$(DRV_TOP_DIR)/drvio/${MY_CPU_DIR}/ctc_watchdog

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread

all: $(BIN_DIR)/$(PROG) $(BIN_DIR)/$(REBOOT_SCRIPT)

$(BIN_DIR)/$(REBOOT_SCRIPT): $(REBOOT_SCRIPT)
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $(REBOOT_SCRIPT) $@

include $(MK_DIR)/prog.mk
