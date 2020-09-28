include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = libnss_cdb.so.2
SRCS = $(wildcard *.c)

CFLAGS += -I${PROTO_DIR}/nss_cdb
CFLAGS += -I${TOP_DIR}/lib/sal/include
CFLAGS += -I${TOP_DIR}/lib/derhex
CFLAGS += -fPIC

STATIC_ADD += -L${BLD_DIR}/lib -lsal -lderhex -lcrypt -lhiredis -lcrypto -lpthread
#LD_FLAGS += -Wl,--as-needed -Wl,--no-undefined -Wl,-O1
LD_FLAGS += -Wl,-soname,libnss_cdb.so.2

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS)) $(EXTRA_OBJS)
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS)) $(EXTRA_OBJS:%.o=%.d)

ifeq ($(CPU), x86_64)
else
ifeq ($(CPU), ls1023a)
else
ifeq ($(CPU), ctc5236)
else
LD_FLAGS += -m32
endif
endif
endif

ETC_DIR?=$(BLD_DIR)/etc

all: $(LIB_DIR)/$(LIB) $(ETC_DIR)/nss_ctc_conf

$(ETC_DIR)/nss_ctc_conf: conf
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp conf $@

$(LIB_DIR)/$(LIB): $(OBJS)
	@echo "LIB => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	$(CC) $(LD_FLAGS) $^ $(STATIC_ADD) $(LD_ADD) -shared -o $@

.PHONY: clean
clean:
	rm -rf $(LIB_DIR)/$(LIB) $(OBJS) $(DEPS) $(EXTRA_CLEANS)

ifneq ($(MAKECMDGOALS),clean)
sinclude $(DEPS)
endif

include $(MK_DIR)/obj.mk
