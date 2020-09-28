ifeq ($(targetbase),linux)
all_targets = libdal 

OBJ_DIR=$(OBJ_BASE_DIR)/dal

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

libdal: FORCE
	$(CTC_MAKE) -f Makefile_libdal

clean_libdal: FORCE
	$(CTC_MAKE) -f Makefile_libdal clean

.PHONY: FORCE
FORCE:

endif


######################################set vxworks compile env #############################
ifeq ($(targetbase),vxworks)

OBJ_DIR=$(OBJ_BASE_DIR)/dal


all_targets = src libdal

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

src: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dal/src -f Makefile.vx

clean_src: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dal/src -f Makefile.vx clean

libdal: FORCE
	$(CTC_MAKE) -f Makefile_libdal

clean_libdal: FORCE
	$(CTC_MAKE) -f Makefile_libdal clean

	
.PHONY: FORCE
FORCE:
endif
