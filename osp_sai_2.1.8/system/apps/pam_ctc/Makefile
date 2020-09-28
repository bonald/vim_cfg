include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

LIB = pam_ctc
SRCS = $(wildcard *.c)

ifeq ($(ARCH),um)
CFLAGS += -I${TOP_DIR}/third_party/precompiled/include
endif
ifeq ($(ARCH),arm)
CFLAGS += -I${TOP_DIR}/third_party/precompiled/include
endif
CFLAGS += -I${TOP_DIR}/lib/sal/include
CFLAGS += -I${TOP_DIR}/lib/derhex
CFLAGS += -fPIC 

ifeq ($(ARCH),um)
ifeq ($(CPU),i686)
LD_FLAGS += -L${TOP_DIR}/third_party/precompiled/uml -lpamctc -lpamctc_misc
else
LD_FLAGS += -L${TOP_DIR}/third_party/precompiled/uml/64bit -lpamctc -lpamctc_misc
endif
endif
ifeq ($(CPU),hi3535)
LD_FLAGS += -L${TOP_DIR}/third_party/precompiled/hi3535/lib -lpam -lpam_misc
endif
ifeq ($(CPU),ls1023a)
LD_FLAGS += -L${TOP_DIR}/third_party/precompiled/arm64/lib -lpam -lpam_misc
endif
ifeq ($(CPU),ctc5236)
LD_FLAGS += -L${TOP_DIR}/third_party/precompiled/arm64/lib -lpam -lpam_misc
endif
ifeq ($(CPU),p1010)
LD_FLAGS += -lpam -lpam_misc
endif
STATIC_ADD += -L${BLD_DIR}/lib -lsal -lderhex -lcrypt -lcrypto -lssl -lrt
#LD_FLAGS += -Wl,--as-needed -Wl,--no-undefined -Wl,-O1

DEP_LIBS += $(LIB_DIR)/libderhex.a

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS)) $(EXTRA_OBJS)
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS)) $(EXTRA_OBJS:%.o=%.d)

ifeq ($(CPU), x86_64)
else
ifneq ($(ARCH), arm)
ifneq ($(ARCH), arm64)
LD_FLAGS += -m32
endif
endif
endif

ETC_DIR?=$(BLD_DIR)/etc

all: $(LIB_DIR)/$(LIB).so $(ETC_DIR)/pam_ctc_conf

$(ETC_DIR)/pam_ctc_conf: ./conf
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp conf $@

$(LIB_DIR)/$(LIB).so: $(OBJS)
	@echo "LIB => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	$(CC) $(LD_FLAGS) $^ $(STATIC_ADD) $(LD_ADD) -shared -o $@

.PHONY: clean
clean:
	rm -rf $(LIB_DIR)/$(LIB).so $(OBJS) $(DEPS) $(EXTRA_CLEANS)

ifneq ($(MAKECMDGOALS),clean)
sinclude $(DEPS)
endif

include $(MK_DIR)/obj.mk
