#include ../Makefile.env.inc
include $(MK_DIR)/sys.mk

PROG = ctc_shell
SRCS = $(wildcard *.c)
SRCS += $(wildcard libctccli/src/*.c)
SRCS += $(wildcard lib/afx/*.c)

CPPFLAGS += -D_CENTEC_
CFLAGS   += -DVTYSH_DEBUG -DVTYSH

CPPFLAGS += -I$(TOOLS_DIR)/vtysh/lib/sal/include
CPPFLAGS += -I$(TOOLS_DIR)/vtysh/libctccli/include
CPPFLAGS += -I$(TOOLS_DIR)/vtysh/libctccli/src
CPPFLAGS += -I$(TOOLS_DIR)/vtysh/lib/afx

DEP_LIBS = 
#LD_LIBS = -L$(LIB_DIR) -llibctccli -lreadline -ltermcap -lsdksal -lsal -lcrypt
LD_LIBS = -L$(LIB_DIR) -lreadline -ltermcap -lcrypt -lrt

#added by ychen in 20130723 for show discard command
CP_DIR = $(BLD_DIR)/etc
CP_FILES  = $(wildcard *.txt)

all: $(BIN_DIR)/$(PROG) $(addprefix $(CP_DIR)/,$(CP_FILES))
$(CP_DIR)/%: %
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $< $@ && \
	cp -rf ShowIpeDiscardType.txt $(BLD_DIR)/etc/ && \
	cp -rf ShowEpeDiscardType.txt $(BLD_DIR)/etc/
	
#ychen end

include $(MK_DIR)/prog.mk
