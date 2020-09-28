OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS)) $(EXTRA_OBJS)
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS)) $(EXTRA_OBJS:%.o=%.d)

ifeq ($(IS_GCOV),yes)
CPPFLAGS += -fprofile-arcs -ftest-coverage
endif

LIBS = $(LIB_DIR)/lib$(LIB).a

$(LIBS): $(OBJS)
	@echo "LIB => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	rm -rf $@ && \
	$(AR) $@ $^ && \
	$(RANLIB) $@

.PHONY: clean
clean:
	rm -rf $(LIB_DIR)/lib$(LIB).a $(OBJS) $(DEPS) $(EXTRA_CLEANS)

ifneq ($(MAKECMDGOALS),clean)
sinclude $(DEPS)
endif

include $(MK_DIR)/obj.mk
