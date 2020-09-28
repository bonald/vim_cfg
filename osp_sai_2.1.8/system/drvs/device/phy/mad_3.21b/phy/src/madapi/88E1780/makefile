# Source files in this directory
TARGET = 
CSOURCES	= 88E1780_MACCtrl.c 88E1780_PatGen.c 88E1780_Stats.c 88E1780_SyncClkCtrl.c 88E1780_SysCtrl.c 88E1780_Vod.c

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
