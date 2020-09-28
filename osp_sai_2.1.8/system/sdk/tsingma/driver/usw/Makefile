ifeq ($(targetbase),vxworks)
#all_targets = common usw libdrv 
all_targets = usw libdrv

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

#common: FORCE
#	$(CTC_MAKE) -C $(SDK_DIR)/driver/common -f Makefile.vx
#
#clean_common: FORCE
#	$(CTC_MAKE) -C $(SDK_DIR)/driver/common -f Makefile.vx  clean	
#	
usw: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/usw -f Makefile.vx

clean_usw: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/usw -f Makefile.vx  clean		
	
libdrv: FORCE
	$(CTC_MAKE) -f Makefile_libdrv.vx

clean_libdrv: FORCE
	$(CTC_MAKE) -f Makefile_libdrv.vx clean	
		

.PHONY: FORCE
FORCE:
endif

