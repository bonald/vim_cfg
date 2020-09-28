call c:\Tornado2.2\host\x86-win32\bin\torvars.bat

rem simulation: vxworks-sim; board:vxworks;
set BOARD=vxworks-sim
set CHIPNAME=duet2
set SUBCHIPNAME=rama
set VER=d
set targetbase=vxworks

rem configure the compile environment
set CPU=SIMNT
set ARCH=i686
set CC=ccsimpc
set AR=arsimpc
set AS=ccsimpc
set LD=ldsimpc
set RANLIB=ranlibsimpc

rem configure the compile environment (PPC603)
#set BOARD=vxworks
#set CPU=PPC603
#set ARCH=powerpc
#set CC=ccppc
#set AR=arppc
#set AS=ccppc
#set LD=ldppc
#set RANLIB=ranlibppc
#set LD_PARTIAL=ccppc -r -nostdlib -Wl,-X

rem configure the base directory according to your development environment
set SDK_DIR=%CD%
set CURDIR=$(SDK_DIR)
set MK_DIR=$(SDK_DIR)/mk
set WIND_BASE=c:/Tornado2.2
set TGT_DIR=$(WIND_BASE)/target
set PROJ_DIR=$(WIND_BASE)/target
set VX_HEADER=$(WIND_BASE)/target/h
set VXLIB_DIR=$(WIND_BASE)/target/lib/simpc/SIMNT
#set VXLIB_DIR=$(WIND_BASE)/host/x86-win32/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/PPC603gnu
set PROJECT_NAME=ctcsdk
set PRJ_TYPE=vxworks
set TOOL_FAMILY=gnu
set WIND_HOST_TYPE=x86-win32






