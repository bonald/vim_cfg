CFLAGS += -fPIC
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

$(LIB_DIR)/lib$(LIB).so: $(OBJS)
	@echo "LIB => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	$(CC) $(LD_FLAGS) -shared -o $@ $^ $(STATIC_LIBS)
    
.PHONY: clean
clean:
	@echo "CLEAN => lib$(LIB).so" && rm -rf $(LIB_DIR)/lib$(LIB).so $(OBJS) $(DEPS) $(EXTRA_CLEANS)

ifneq ($(MAKECMDGOALS),clean)
sinclude $(DEPS)
endif

include $(MK_DIR)/obj.mk
