include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

MIBDSRCS = $(wildcard *.py *.sh trap_agent)
MIBD_DIR =$(BLD_DIR)/mibd/

MIBCOMMONSRCS = $(wildcard ./mib_modules/common/*.py)
MIBD_COMMON_DIR =$(BLD_DIR)/mibd/mib_modules/common/

MIBOFSRCS = $(wildcard ./mib_modules/openflow/*.py)
MIBD_OF_DIR =$(BLD_DIR)/mibd/mib_modules/openflow/

MIBTAPSRCS = $(wildcard ./mib_modules/tap/*.py)
MIBD_TAP_DIR =$(BLD_DIR)/mibd/mib_modules/tap/

MIBCNOSSRCS = $(wildcard ./mib_modules/cnos/*.py)
MIBD_CNOS_DIR =$(BLD_DIR)/mibd/mib_modules/cnos/

ALL_PROGS=$(addprefix $(MIBD_DIR),$(sort $(MIBDSRCS))) $(addprefix $(MIBD_DIR),$(sort $(MIBCOMMONSRCS))) $(addprefix $(MIBD_DIR),$(sort $(MIBOFSRCS))) $(addprefix $(MIBD_DIR),$(sort $(MIBTAPSRCS))) $(addprefix $(MIBD_DIR),$(sort $(MIBCNOSSRCS)))
LOCAL_PROGS=$(addprefix ./,$(sort $(MIBDSRCS))) $(addprefix ./,$(sort $(MIBCOMMONSRCS))) $(addprefix ./,$(sort $(MIBOFSRCS))) $(addprefix ./,$(sort $(MIBTAPSRCS))) $(addprefix ./,$(sort $(MIBCNOSSRCS)))

all: $(ALL_PROGS) FORCE

$(ALL_PROGS): $(LOCAL_PROGS)    
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $(subst $(BLD_DIR),..,$@) $@

EXTRA_CLEANS=$(ALL_PROGS)
            
include $(MK_DIR)/prog.mk

.PHONY: FORCE
FORCE:
