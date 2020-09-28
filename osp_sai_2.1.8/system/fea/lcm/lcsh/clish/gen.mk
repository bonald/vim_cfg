
CPPFLAGS += -I. -I$(TOP_DIR)/lib/libepsutil
CPPFLAGS += -I$(TOP_DIR)/include

CPPFLAGS += -I$(TOP_DIR)/lib/libdbg

DEP_LIBS = $(LIB_DIR)/$(LIBEPSUTIL) 
DEP_LIBS += $(LIB_DIR)/$(LIBEPSDBG)

LD_LIBS +=  -L$(LIB_DIR) -lreadline -ltermcap -lcrypt -lcrypto

LD_LIBS += -lresolv
