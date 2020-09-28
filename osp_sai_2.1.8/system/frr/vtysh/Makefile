include $(MK_DIR)/sys.mk
include $(QUAGGA_TOP_DIR)/Makefile.env.inc
include ../../Makefile.env.inc

PROG = vtysh
#SRCS = $(shell find -name "*.c")
SRCS = vtysh.c vtysh_config.c vtysh_main.c vtysh_user.c

all: $(BIN_DIR)/$(PROG) FORCE

CPPFLAGS += -I$(GLIB_USER)/include
LD_LIBS = -L$(GLIB_USER)/lib -ldl

LIBZEBRA = $(LIB_DIR)/libzebra.a
DEP_LIBS = $(LIBZEBRA)
LD_LIBS += -L$(LIB_DIR) -lzebra -lreadline -ltermcap -lcrypt -lrt -ljson 

EXTRA_OBJS = $(OBJ_DIR)/vtysh_cmd.o
EXTRA_CLEANS = $(QUAGGA_TOP_DIR)/vtysh/vtysh_cmd.c

EXTRA_DIST = extract.pl
vtysh_cmd_FILES = $(QUAGGA_TOP_DIR)/bgpd/*.c \
                  $(QUAGGA_TOP_DIR)/ospfd/*.c \
                  $(QUAGGA_TOP_DIR)/ospf6d/*.c \
                  $(QUAGGA_TOP_DIR)/zebra/debug.c \
                  $(QUAGGA_TOP_DIR)/zebra/interface.c \
                  $(QUAGGA_TOP_DIR)/zebra/rtadv.c \
                  $(QUAGGA_TOP_DIR)/zebra/zebra_vty.c \
                  $(QUAGGA_TOP_DIR)/zebra/zserv.c \
                  $(QUAGGA_TOP_DIR)/zebra/router-id.c \
                  $(QUAGGA_TOP_DIR)/zebra/zebra_routemap.c \
                  $(QUAGGA_TOP_DIR)/zebra/zebra_fpm.c \
                  $(QUAGGA_TOP_DIR)/lib/keychain.c \
                  $(QUAGGA_TOP_DIR)/lib/vty.c \
                  $(QUAGGA_TOP_DIR)/lib/filter.c \
                  $(QUAGGA_TOP_DIR)/lib/plist.c \
                  $(QUAGGA_TOP_DIR)/lib/distribute.c \
                  $(QUAGGA_TOP_DIR)/lib/if_rmap.c \
                  $(QUAGGA_TOP_DIR)/lib/routemap.c

$(EXTRA_CLEANS): $(vtysh_cmd_FILES)
	./$(EXTRA_DIST) $(vtysh_cmd_FILES) > $(EXTRA_CLEANS)

include $(MK_DIR)/prog.mk

.PHONY: FORCE
FORCE:
