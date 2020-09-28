include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

CEMSRCS = $(wildcard *.py *.sh trap_agent)

# Default mibs
CEM_DIR?=$(BLD_DIR)/cem/

ALL_PROGS=$(addprefix $(CEM_DIR),$(sort $(CEMSRCS)))
LOCAL_PROGS=$(addprefix ./,$(sort $(CEMSRCS)))

all: $(ALL_PROGS) FORCE

$(ALL_PROGS): $(LOCAL_PROGS)    
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $(subst $(BLD_DIR),..,$@) $@

EXTRA_CLEANS=$(ALL_PROGS)
            
include $(MK_DIR)/prog.mk

.PHONY: FORCE
FORCE:
