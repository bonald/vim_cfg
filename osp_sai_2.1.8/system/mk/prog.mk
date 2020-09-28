OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS)) $(EXTRA_OBJS)
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS)) $(EXTRA_OBJS:%.o=%.d)

ifeq ($(IS_GCOV),yes)
CPPFLAGS += -fprofile-arcs -ftest-coverage
LD_LIBS += -fprofile-arcs
endif

ifeq ($(IS_GPROF),yes)
CPPFLAGS += -pg
LD_LIBS += -pg 
endif

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

$(BIN_DIR)/$(PROG): $(OBJS) $(DEP_LIBS)
	@echo "LD  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	$(CC) $(LD_FLAGS) -o $@ $(OBJS) $(LD_LIBS)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)/$(PROG) $(OBJS) $(DEPS) $(EXTRA_CLEANS)

ifneq ($(MAKECMDGOALS),clean)
sinclude $(DEPS)
endif

include $(MK_DIR)/obj.mk
