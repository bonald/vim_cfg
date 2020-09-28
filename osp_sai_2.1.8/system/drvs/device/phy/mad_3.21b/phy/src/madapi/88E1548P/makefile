# Source files in this directory
CSOURCES	=  88E1548P_SyncClkCtrl.c 88E1548P_SysCtrl.c

ASOURCES	=

# Include common variable definitions
ifeq ($(OS_RUN),VXWORKS)
include $(MAD_TOOL_DIR)\make.defs
endif
ifeq ($(OS_RUN),LINUX)
include $(MAD_TOOL_DIR)/makelnx.defs
endif

# Add in extra stuffs
EXTRA_INCLUDE	+= 
EXTRA_DEFINE	+=
ADDED_CFLAGS	+=

# Include common build rules
ifeq ($(OS_RUN),VXWORKS)
include $(MAD_TOOL_DIR)\make.rules
endif
ifeq ($(OS_RUN),LINUX)
include $(MAD_TOOL_DIR)/makelnx.rules
endif

# end of file
