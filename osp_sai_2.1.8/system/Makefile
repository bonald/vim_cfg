#
# for geneate Makefile.env.inc
#

ifeq ($(wildcard Makefile.env.inc),)
all_targets = uml
else
include Makefile.env.inc

#
# centec SDK
#
ifndef CHIPNAME
export CHIPNAME = goldengate
endif

#export ChipAgent = TRUE

ifndef AUTO_API_GEN
export AUTO_API_GEN = no
endif

#the OS_TYPE is just for distinguish CNOS_OS and IPI_OS which is used by lcm .etc
ifndef OS_TYPE
export OS_TYPE = cnos
endif

#
# for system
#

### ifeq ($(IS_UML), y)
### all_targets += libcmodel libmemmodel
### endif

all_targets =
ifeq ($(BLD_KERNEL), yes)
all_targets += kernel
all_targets += busybox
endif

ifeq ($(ENABLE_NETCONF),yes)
all_targets += netconf
all_targets += libncutil
endif

all_targets += redis
all_targets += lib
all_targets += libdhcp
all_targets += libcjson
all_targets += ipcmsg
all_targets += libcdb
all_targets += libsdb
all_targets += libcdbpbc
all_targets += proto
all_targets += klish
ifneq ($(PRODUCT), tap)
all_targets += frr
endif
all_targets += libasn1
all_targets += reg_ops
### gen msg
ifeq ($(genmsg),yes)
#all_targets += libhagtmsggen
#all_targets += liblcmmsggen
endif
all_targets += liblcmmsggen

all_targets += libhagtmsg
all_targets += liblcmmsg
all_targets += libsdkcmodel
all_targets += libsai
all_targets += libhagt
all_targets += liblcm
all_targets += liblsrv
all_targets += libdbclient

ifeq ($(PRODUCT), sai)
all_targets += fea
endif

ifeq ($(PRODUCT), tap)
all_targets += fea
endif

ifeq ($(PRODUCT), ofdpa)
all_targets += fea
endif

ifeq ($(PRODUCT), of)
all_targets += fea
endif

ifeq ($(PRODUCT), ttp)
all_targets += fea
endif

all_targets += liblai
all_targets += lcsh
all_targets += apps

ifeq ($(CHIPNAME),greatbelt)
#all_targets += sdksh
endif

all_targets += saiprofile

#ifeq ($(CHIPNAME),goldengate)
all_targets += ctcshell
#endif

fea_deps = libasn1 libhagtmsg liblcmmsg libhagt libsdkcmodel liblsrv liblai liblcm libcjson libsai libcdb libcdbpbc
proto_deps = liblcmmsg libasn1 libcdb libcdbpbc
ifneq ($(PRODUCT), tap)
proto_deps += frr
endif

lcsh_deps = lib

ifeq ($(ARCH),powerpc)
all_targets += board_type_detect
all_targets += reboot_record
all_targets += libdrv
fea_deps    += libdrv
proto_deps  += libdrv
ofagent_deps  += libdrv
endif

ifeq ($(ARCH),arm)
all_targets += board_type_detect
all_targets += reboot_record
all_targets += libdrv
fea_deps    += libdrv
proto_deps  += libdrv
ofagent_deps  += libdrv
endif

ifeq ($(ARCH),arm64)
all_targets += board_type_detect
all_targets += reboot_record
all_targets += libdrv
fea_deps    += libdrv
proto_deps  += libdrv
ofagent_deps  += libdrv
endif

ifeq ($(ARCH),x86_64)
all_targets += board_type_detect
all_targets += reboot_record
all_targets += libdrv
fea_deps    += libdrv
proto_deps  += libdrv
ofagent_deps  += libdrv
endif

all_targets += nopassword
all_targets += third_party_compile

ifeq ($(CHIPNAME),duet2)
export CTC_SHELL_DIR = $(SDK_DIR)/ctc_shell
else ifeq ($(CHIPNAME),tsingma)
export CTC_SHELL_DIR = $(SDK_DIR)/ctc_shell
else
export CTC_SHELL_DIR = $(SDK_DIR)/libctccli/ctc-shell/client
endif

ifeq ($(PRODUCT), ofdpa)
ofagent_deps += libasn1 libsdkcmodel libsai libofdpa liblcm libcdb libcdbpbc libofdpa indigo ofdpadriver
fea_deps += libofdpa libofagent_main indigo ofdpadriver
endif

SDK_CLI_FILES = $(SDK_DIR)/ctccli/sdkcli/*cli.c $(SDK_DIR)/ctccli/ctc_master_cli.c $(SDK_DIR)/ctccli/sdkcli/$(CHIPNAME)/*cli.c 

ifeq ($(CHIPNAME),greatbelt)
#SDK_CLI_FILES += $(TOOLS_DIR)/vtysh/sdk_tool_cli/sdk_debug_tool_cli.c $(SDK_DIR)/debug_tool/debug_cli/*cli.c 
endif

endif

ver_h_file=$(PROTO_DIR)/include/version.h
version_num=$(shell grep '\#define L2PLUS_VERSION' $(ver_h_file) | sed -s -e 's/.*L2PLUS_VERSION.*\"\(.*\)\"/\1/')
ifeq ($(PRODUCT),ttp)
version_num=$(shell grep '\#define OF_VERSION' $(ver_h_file) | sed -s -e 's/.*OF_VERSION.*\"\(.*\)\"/\1/')
endif
ifeq ($(PRODUCT),of)
version_num=$(shell grep '\#define OF_VERSION' $(ver_h_file) | sed -s -e 's/.*OF_VERSION.*\"\(.*\)\"/\1/')
endif
ifeq ($(PRODUCT),tap)
version_num=$(shell grep '\#define TAP_VERSION' $(ver_h_file) | sed -s -e 's/.*TAP_VERSION.*\"\(.*\)\"/\1/')
endif
ifeq ($(PRODUCT),sai)
version_num=$(shell grep '\#define SAI_VERSION' $(ver_h_file) | sed -s -e 's/.*SAI_VERSION.*\"\(.*\)\"/\1/')
endif
ifeq ($(PRODUCT),ofdpa)
version_num=$(shell grep '\#define OFDPA_VERSION' $(ver_h_file) | sed -s -e 's/.*OFDPA_VERSION.*\"\(.*\)\"/\1/')
endif
export version_num

#
# for generate all in one image
#
all: compile_time package debian_package FORCE

COMPFILE="$(CTC_LIB_DIR)/include/glb_compile_time.h"
compile_time: FORCE
	@echo "Product is $(PRODUCT)"
	@echo "Chip is $(CHIPNAME)"
	@echo "Arch is $(ARCH)"
	@echo "Cpu  is $(CPU)"
	@echo "Generate compile time file..."
	@echo "/* Auto generated file, please don't modify!!! */" > $(COMPFILE)
	@echo "#ifndef _COMPILE_TIME_H" >> $(COMPFILE)
	@echo "#define _COMPILE_TIME_H" >> $(COMPFILE)
	@echo "" >> $(COMPFILE)
	@echo -n "#define COMPILE_TIME " >> $(COMPFILE)
	@date "+%s" >> $(COMPFILE)
	@echo -n "#define COMPILE_TIME_STR \"" >> $(COMPFILE)
	@date "+%b %d %H:%M:%S %Y\"" >> $(COMPFILE)
	@echo "" >> $(COMPFILE)
	@echo "#endif" >> $(COMPFILE)

clean_compile_time: FORCE
	rm -fr $(CTC_LIB_DIR)/include/glb_compile_time.h

netconf: FORCE
	make -C $(APP_TOP_DIR)/netconf -f Makefile

clean_netconf: FORCE
	make -C $(APP_TOP_DIR)/netconf -f Makefile clean

libncutil: FORCE
	make -C $(TOP_DIR)/lib/libncutil -f Makefile

clean_libncutil: FORCE
	make -C $(TOP_DIR)/lib/libncutil -f Makefile clean

lib: FORCE

board:
	@chmod a+x ./createenv.sh && ./createenv.sh p1010 48t4x r N
	@echo "Reexecute make automatically for board release version..."
	@sleep 4
	make

board_d:
	@chmod a+x ./createenv.sh && ./createenv.sh p1010 48t4x d N
	@echo "Reexecute make automatically for board debug version..."
	@sleep 4
	make

uml:
	@chmod a+x ./createenv.sh && ./createenv.sh i686 uml d N
	@echo "Reexecute make automatically for uml debug version..."
	@sleep 4
	make
uml_r:
	@chmod a+x ./createenv.sh && ./createenv.sh i686 uml r N
	@echo "Reexecute make automatically for uml release version..."
	@sleep 4
	make

package: $(all_targets) image FORCE

lib: FORCE
	$(OSP_MAKE) -C lib

clean_lib: FORCE
	$(OSP_MAKE) -C lib clean

libdhcp: FORCE
	$(OSP_MAKE) -C apps/dhcp -f Makefile.pre.ctc

clean_libdhcp: FORCE
	$(OSP_MAKE) -C lib -f Makefile.pre.ctc clean

klish: FORCE
	$(OSP_MAKE) -C klish -f Makefile

clean_klish: FORCE
	$(OSP_MAKE) -C klish -f Makefile clean

fea: $(fea_deps) FORCE
	$(OSP_MAKE) -C fea -f Makefile

clean_fea: FORCE
	$(OSP_MAKE) -C fea -f Makefile clean

ofagent: $(ofagent_deps) FORCE
	$(OSP_MAKE) -C ofagent -f Makefile
	ln -sf $(TOP_DIR)/ryu/bin/ryu-manager $(UML_IMAGE_DIR)/osp/sbin/ryu-manager

ofagent_clean: FORCE
	$(OSP_MAKE) -C ofagent -f Makefile clean
	rm -f $(UML_IMAGE_DIR)/osp/sbin/ryu-manager

indigo:
	$(OSP_MAKE) -C ofagent/indigo -f make.ofagent

libofagent_main:
	$(OSP_MAKE) -C ofagent -f Makefile.lib.mk

ofdpadriver:
	$(OSP_MAKE) -C ofagent/ofdpadriver -f Makefile

lcsh: $(lcsh_deps) FORCE
	$(OSP_MAKE) -C fea/lcm/lcsh -f Makefile

clean_lcsh: FORCE
	$(OSP_MAKE) -C fea/lcm/lcsh -f Makefile clean

apps: FORCE
	$(OSP_MAKE) -C apps -f Makefile

clean_apps: FORCE
	$(OSP_MAKE) -C apps -f Makefile clean

proto: $(proto_deps) FORCE
	$(OSP_MAKE) -C proto -f Makefile

clean_proto: FORCE
	$(OSP_MAKE) -C proto -f Makefile clean

libcdb: FORCE
	$(OSP_MAKE) -C cdb/core -f Makefile

clean_libcdb: FORCE
	$(OSP_MAKE) -C cdb/core -f Makefile clean

libsdb: FORCE
	$(OSP_MAKE) -C sdb/core -f Makefile

clean_libsdb: FORCE
	$(OSP_MAKE) -C sdb/core -f Makefile clean

libcdbpbc: FORCE
	$(OSP_MAKE) -C cdb/protobuf -f Makefile

clean_libcdbpbc: FORCE
	$(OSP_MAKE) -C cdb/protobuf -f Makefile clean
	
redis: FORCE
	$(OSP_MAKE) -C redis -f Makefile.ctc

clean_redis: FORCE
	$(OSP_MAKE) -C redis -f Makefile.ctc clean
	
	
quagga: FORCE
	$(OSP_MAKE) -C quagga -f Makefile

clean_quagga: FORCE
	$(OSP_MAKE) -C quagga -f Makefile clean

frr: FORCE
	$(OSP_MAKE) -C frr -f Makefile

clean_frr: FORCE
	$(OSP_MAKE) -C frr -f Makefile clean	
	
libsdkcmodel: FORCE
	$(OSP_MAKE) -C $(SDK_DIR) targetbase=linux BOARD=$(SDK_BOARD) CHIPGENERATION=$(CHIPNAME) UNITTEST=FALSE BLD_DIR=$(BLD_DIR) TOP_DIR=$(TOP_DIR) VER=$(VER) K_DIR=$(KDIR) SDK_DIR=$(SDK_DIR) SAI_DIR=$(SAI_DIR) -f Makefile.ctc

clean_libsdkcmodel: FORCE
	$(OSP_MAKE) -C $(SDK_DIR) targetbase=linux BOARD=$(SDK_BOARD) CHIPGENERATION=$(CHIPNAME) UNITTEST=FALSE BLD_DIR=$(BLD_DIR) TOP_DIR=$(TOP_DIR) VER=$(VER) K_DIR=$(KDIR) SDK_DIR=$(SDK_DIR) SAI_DIR=$(SAI_DIR) -f Makefile.ctc clean

libsai: FORCE
	$(OSP_MAKE) -C sai -f Makefile

clean_libsai: FORCE
	$(OSP_MAKE) -C sai -f Makefile clean
    
libofdpa: FORCE
	$(OSP_MAKE) -C ofdpa -f Makefile

clean_libofdpa: FORCE
	$(OSP_MAKE) -C ofdpa -f Makefile clean

vanilla_ofdpa_prereq: FORCE
	# When building e350 edition, it's need to build sdk/dal/, and it resulted
	# serveral kernel modules which are placed into out.e350/build.p1010.d/bin
	# directory, but this directory may not be created that time
	if [ ! -d $(BLD_DIR)/bin ]; then mkdir -p $(BLD_DIR)/bin; fi

vanilla_ofdpa: vanilla_ofdpa_prereq libsdkcmodel ctcshell FORCE
	$(OSP_MAKE) -C ofdpa -f Makefile.vanilla.mk

clean: FORCE
	@echo "Delete objs and binaries output directory: $(OUT_DIR)"
	@rm -fr $(OUT_DIR)

libhagt: libhagtmsg FORCE
	$(OSP_MAKE) -C fea/hagt -f Makefile

clean_libhagt: FORCE
	$(OSP_MAKE) -C fea/hagt -f Makefile clean
	
liblcm: liblcmmsg FORCE
	$(OSP_MAKE) -C fea/lcm -f Makefile

clean_liblcm: FORCE
	$(OSP_MAKE) -C fea/lcm -f Makefile clean
    
liblai: FORCE
	$(OSP_MAKE) -C lai -f Makefile
    
clean_liblai: FORCE
	$(OSP_MAKE) -C lai -f Makefile clean

liblsrv: FORCE
	$(OSP_MAKE) -C fea/lsrv -f Makefile

clean_liblsrv: FORCE
	$(OSP_MAKE) -C fea/lsrv -f Makefile clean

libdrv: FORCE
	$(OSP_MAKE) -C drvs -f Makefile.ctc

clean_libdrv: FORCE
	$(OSP_MAKE) -C drvs -f Makefile.ctc clean

libptfagent: FORCE
	$(OSP_MAKE) -C sai/ptf_agent -f Makefile

clean_libptfagent: FORCE
	$(OSP_MAKE) -C sai/ptf_agent -f Makefile clean

libasn1: FORCE
	$(OSP_MAKE) -C lib/libasn1

clean_libasn1: FORCE
	$(OSP_MAKE) -C lib/libasn1 clean

libhagtmsggen: FORCE
	$(OSP_MAKE) -C lib/msg/hagtmsg -f Makefile.gen asn1_gen

clean_libhagtmsggen: FORCE
	$(OSP_MAKE) -C lib/msg/hagtmsg -f Makefile.gen clean

libhagtmsg: FORCE
	$(OSP_MAKE) -C lib/msg/hagtmsg -f Makefile

clean_libhagtmsg: FORCE
	$(OSP_MAKE) -C lib/msg/hagtmsg -f Makefile clean

liblcmmsggen: FORCE
	$(OSP_MAKE) -C fea/lcm/lcmmsg -f Makefile.gen asn1_gen

clean_liblcmmsggen: FORCE
	$(OSP_MAKE) -C fea/lcm/lcmmsg -f Makefile.gen clean

liblcmmsg: liblcmmsggen FORCE
	$(OSP_MAKE) -C fea/lcm/lcmmsg -f Makefile

clean_liblcmmsg: FORCE
	$(OSP_MAKE) -C fea/lcm/lcmmsg -f Makefile clean

kmpeth: FORCE
	$(OSP_MAKE) -C fea/kmpeth -f Makefile

clean_kmpeth: FORCE
	$(OSP_MAKE) -C fea/kmpeth -f Makefile clean

libcjson: FORCE
	$(OSP_MAKE) -C third_party/cjson -f Makefile

clean_libcjson: FORCE
	$(OSP_MAKE) -C third_party/cjson -f Makefile clean

ipcmsg: FORCE
	$(OSP_MAKE) -C third_party/ipcmsg -f Makefile

clean_ipcmsg: FORCE
	$(OSP_MAKE) -C third_party/ipcmsg -f Makefile clean

board_type_detect: FORCE
	$(OSP_MAKE) -C tools/board_type_detect -f Makefile
	$(CTC_MAKE) -C tools/flash_env -f Makefile
	$(CTC_MAKE) -C tools/check_fit_image -f Makefile.ctc	

clean_board_type_detect: FORCE
	$(OSP_MAKE) -C tools/board_type_detect -f Makefile clean
	$(CTC_MAKE) -C tools/flash_env -f Makefile clean
	$(CTC_MAKE) -C tools/check_fit_image -f Makefile.ctc clean	

reboot_record: FORCE
	$(OSP_MAKE) -C tools/reboot_record -f Makefile.ctc
	
clean_reboot_record: FORCE
	$(OSP_MAKE) -C tools/reboot_record -f Makefile.ctc clean

reg_ops: FORCE
	$(CTC_MAKE) -C $(TOP_DIR)/tools/reg_ops -f Makefile.ctc

clean_reg_ops: FORCE
	$(CTC_MAKE) -C $(TOP_DIR)/tools/reg_ops -f Makefile.ctc clean


nopassword:
	$(OSP_MAKE) -C tools/bootcon_nopassword -f Makefile

clean_nopassword:
	$(OSP_MAKE) -C tools/bootcon_nopassword -f Makefile clean

saiprofile: FORCE
	@echo "Copy SAI profiles" && \
	cp -rf sai/profile/$(CHIPNAME)/mem_profile_*.cfg $(BLD_DIR)/etc/

clean_saiprofile: FORCE
	@echo "Delete SAI profiles" && \
	rm -fr $(BLD_DIR)/etc/mem_profile_*.cfg

libdbclient: FORCE
	$(OSP_MAKE) -C lib/dbclient -f Makefile
	
clean_libdbclient: FORCE
	$(OSP_MAKE) -C lib/dbclient -f Makefile clean
	
#sdksh: $(SDK_CLI_FILES) FORCE
#	@echo ""
#	@echo "  GEN vtysh_cmd.c ......"
#	@echo ""
#	@chmod a+x $(TOP_DIR)/tools/vtysh/extract.pl
#	@$(TOOLS_DIR)/vtysh/extract.pl $(SDK_CLI_FILES) > $(TOOLS_DIR)/vtysh/vtysh_cmd.c
#	$(OSP_MAKE) -C tools/vtysh -f Makefile

# Build libsdkcmodel first, then ctc_shell can be directly built.
ctcshell: libsdkcmodel FORCE
	$(OSP_MAKE) -C $(CTC_SHELL_DIR) targetbase=linux BOARD=$(SDK_BOARD) CHIPGENERATION=$(CHIPNAME) UNITTEST=FALSE BLD_DIR=$(BLD_DIR) TOP_DIR=$(TOP_DIR) VER=$(VER) K_DIR=$(KDIR) SDK_DIR=$(SDK_DIR) -f Makefile.ctc

clean_ctcshell:
	$(OSP_MAKE) -C $(CTC_SHELL_DIR) targetbase=linux BOARD=$(SDK_BOARD) CHIPGENERATION=$(CHIPNAME) UNITTEST=FALSE BLD_DIR=$(BLD_DIR) TOP_DIR=$(TOP_DIR) VER=$(VER) K_DIR=$(KDIR) SDK_DIR=$(SDK_DIR) -f Makefile.ctc clean

#clean_sdksh: FORCE
#	$(OSP_MAKE) -C tools/vtysh -f Makefile clean
#	@rm -fr $(TOOLS_DIR)/vtysh/vtysh_cmd.c


########## kernel ##########
ifeq ($(ARCH),powerpc)
kernel:
	@if [ ! -f $(KDIR)/.config ] ; then \
		cd $(PLAT_TOP_DIR)/linux; \
		chmod +x ./build_powerpc.sh && ./build_powerpc.sh $(KDIR) $(VER); \
		if [ ! -d $(ROOT_DIR)/out.$(BOARD) ]; then \
			ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
		fi; \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) uImage; \
		$(OSP_MAKE) $(BOARD)_ctc.dtb; \
		if [ ! -d $(ROOT_DIR)/out.$(BOARD) ]; then \
			ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
		fi; \
	fi;
endif

ifeq ($(ARCH),arm)
kernel:
	@if [ ! -f $(KDIR)/.config ] ; then \
		cd $(PLAT_TOP_DIR)/linux-hi3535; \
		chmod +x ./build_arm.sh && ./build_arm.sh $(KDIR) $(VER); \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) uImage; \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	fi;
endif

ifeq ($(ARCH),arm64)
ifeq ($(CPU), ls1023a)
kernel:
	@if [ ! -f $(KDIR)/.config ] ; then \
		cd $(PLAT_TOP_DIR)/linux-4.9.62; \
		chmod +x ./build_arm64.sh && ./build_arm64.sh $(KDIR) $(VER); \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) ; \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	fi;
endif
ifeq ($(CPU), ctc5236)
kernel:
	@if [ ! -f $(KDIR)/.config ] ; then \
		cd $(PLAT_TOP_DIR)/linux-4.9.62; \
		chmod +x ./build_arm64.sh && ./build_arm64.sh $(KDIR) $(VER); \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) ; \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	fi;
endif
endif

ifeq ($(ARCH),x86_64)
kernel:
	@if [ ! -f $(KDIR)/.config ] ; then \
		if [ ! -d $(PLAT_TOP_DIR)/linux-3.10.28 ]; then \
			echo "kernel 3.10.28 not found!"; \
			exit 1; \
		fi; \
		cd $(PLAT_TOP_DIR)/linux-3.10.28; \
		chmod +x ./build_x86_64.sh && ./build_x86_64.sh $(KDIR) $(VER); \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) ; \
		ln -s $(BUILD_DIR)/out.$(BOARD) $(ROOT_DIR)/out.$(BOARD); \
	fi;
endif

ifeq ($(ARCH),um)
ifeq ($(CPU),x86_64)
kernel: FORCE
	@if [ ! -f $(KDIR)/.config ] ; then \
		cd $(PLAT_TOP_DIR)/linux-4.9.62; \
		chmod +x ./build_um_64.sh && ./build_um_64.sh $(KDIR); \
		ln -s $(BUILD_DIR)/out.uml $(ROOT_DIR)/out.uml; \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) SUBARCH=x86_64 linux ; \
		ln -s $(BUILD_DIR)/out.uml $(ROOT_DIR)/out.uml; \
        fi;
else
kernel: FORCE
	@if [ ! -f $(KDIR)/.config ] ; then \
		cd $(PLAT_TOP_DIR)/linux; \
		chmod +x ./build_um.sh && ./build_um.sh $(KDIR); \
		ln -s $(BUILD_DIR)/out.uml $(ROOT_DIR)/out.uml; \
	else \
		cd $(KDIR); \
		$(OSP_MAKE) SUBARCH=i386 linux ; \
		ln -s $(BUILD_DIR)/out.uml $(ROOT_DIR)/out.uml; \
        fi;
endif
endif


clean_kernel:
	@if [ -f $(KDIR)/.config ]; then \
		cd $(KDIR); \
	make mrproper; \
	fi;

########## busybox ##########
ifeq ($(ARCH),powerpc)
busybox:
	@if [ ! -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		chmod u+x ./build_powerpc.sh; ./build_powerpc.sh $(VER) $(BUSYBOX_DIR); \
	else \
		cd $(BUSYBOX_DIR); \
		$(OSP_MAKE) ARCH=ppc; \
	fi;
endif

ifeq ($(ARCH),arm)
busybox:
	@if [ ! -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		chmod u+x ./build_arm.sh; ./build_arm.sh $(VER) $(BUSYBOX_DIR); \
	else \
		cd $(BUSYBOX_DIR); \
		$(OSP_MAKE) ARCH=arm; \
	fi;
endif

ifeq ($(ARCH),arm64)
busybox:
	@if [ ! -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		chmod u+x ./build_arm64.sh; ./build_arm64.sh $(VER) $(BUSYBOX_DIR); \
	else \
		cd $(BUSYBOX_DIR); \
		$(OSP_MAKE) ARCH=arm64; \
	fi;
endif

ifeq ($(ARCH),x86_64)
busybox:
	@if [ ! -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		chmod u+x ./build_x86_64.sh; ./build_x86_64.sh $(VER) $(BUSYBOX_DIR); \
	else \
		cd $(BUSYBOX_DIR); \
		$(OSP_MAKE) ;\
	fi;
endif

ifeq ($(ARCH),um)
ifeq ($(CPU),x86_64)
busybox:
	@if [ ! -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		chmod u+x ./build_um_64.sh; ./build_um_64.sh $(VER) $(BUSYBOX_DIR); \
	else \
		cd $(BUSYBOX_DIR); \
		$(OSP_MAKE); \
	fi;
else
busybox:
	@if [ ! -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		chmod u+x ./build_um.sh; ./build_um.sh $(VER) $(BUSYBOX_DIR); \
	else \
		cd $(BUSYBOX_DIR); \
		$(OSP_MAKE) ARCH=i386; \
	fi;
endif
endif

clean_busybox:
	@if [ -f $(BUSYBOX_DIR)/.config ]; then \
		cd $(PLAT_TOP_DIR)/busybox; \
		make clean; \
	fi;


########## package ##########
IMAGE_NAME_PREFIX=uImage

ifeq ($(ENABLE_DEBIAN_LINUX),y)
debian_package: FORCE
	@echo "Building debian package"; \
	cd $(PLAT_TOP_DIR)/build_svc/etc/$(BOARD)/debian/debian_installer; \
	cp $(BUSYBOX_DIR)/busybox .; \
	chmod u+x ./make_debian_rootfs_installer.sh; ./make_debian_rootfs_installer.sh; \
	rm ./busybox ./archive.tar.gz -rf; \
	mv $(PLAT_TOP_DIR)/build_svc/etc/$(BOARD)/debian/debian_installer/debian-rootfs-installer $(PLAT_TOP_DIR)/build_svc/debian-rootfs-installer.$(BOARD).$(VER); \
	echo "Build debian package done"
else
debian_package: FORCE
	@echo "Skipped building debian package"
endif
# ifeq ($(ENABLE_DEBIAN_LINUX),y)
# IMAGE_NAME_PREFIX=cboot
# #ROOTFS_FILENAME=rootfs.tar.gz
# #INITRAMFS_FILENAME=ctc_debian_initramfs.lzma
# #MKSQUASHFS_CMD=$(PLAT_TOP_DIR)/build_svc/mksquashfs
# #TEMP_ROOTFS_DIR=$(OUT_DIR)/$(DEBIAN_PACKAGE_NAME)_$(VER)
# #ROOTFS_SQUASH_FILENAME=$(PLAT_TOP_DIR)/build_svc/debian_rootfs_v$(version_num).swi
# #ROOTFS_PATH=$(PLAT_TOP_DIR)/build_svc/debian/rootfs_repo/$(DEBIAN_ARCH)/$(ROOTFS_FILENAME)
# #ROOTFS_PREPARE_SCRIPT=$(PLAT_TOP_DIR)/build_svc/etc/$(BOARD)/debian/rootfs_prepare.sh
# #ROOTFS_PREPARE_SCRIPT_ARGUMENTS=--override-dir $(PLAT_TOP_DIR)/build_svc/debian/override_dir --initramfs=${KDIR}/$(INITRAMFS_FILENAME) $(TEMP_ROOTFS_DIR)
# #BUILD_ROOTFS_SCRIPT_PATH=$(PLAT_TOP_DIR)/build_svc/debian/build_rootfs.sh
# #BUILD_ROOTFS_CMD=$(BUILD_ROOTFS_SCRIPT_PATH) -a $(ARCH) -k -T $(MKSQUASHFS_CMD) -f squash -d $(TEMP_ROOTFS_DIR) -o $(ROOTFS_SQUASH_FILENAME)  -i '$(ROOTFS_PREPARE_SCRIPT) $(ROOTFS_PREPARE_SCRIPT_ARGUMENTS)' $(ROOTFS_PATH)
# 
# # Must depend on package since we need all the neccesary files to be built
# debian_package: package FORCE
# 	@echo ""
# #	@if [ ! -f $(ROOTFS_PATH) ]; then \
# #        echo "You must provide correct rootfs tarball file: $(ROOTFS_PATH)"; \
# #        echo "You may run the script '$(BUILD_ROOTFS_SCRIPT_PATH)' under ubuntu/debian linux to build the rootfs file: $(ROOTFS_FILENAME)"; \
# #        exit ; \
# #	fi
# #	@if [ ! -f $(ROOTFS_PREPARE_SCRIPT) ]; then \
# #        echo "Unable to find rootfs prepare script: $(ROOTFS_PREPARE_SCRIPT)"; \
# #        exit ; \
# #	fi
# #	@chmod  +x ../platform/build_svc/debian/build_rootfs.sh
# #	@chmod +x ${ROOTFS_PREPARE_SCRIPT}
# #	@$(BUILD_ROOTFS_CMD)
# else
# # dummy debian_package
# debian_package: FORCE
# 	@echo "Skipped building debian package"
# endif


ifeq ($(ARCH),um)
xml:FORCE
	@if [ 'tap' == $(PRODUCT) ] ; then \
	    echo"cp -fr $(KLISH_DIR)/xml-examples/tapcmd/* $(UML_IMAGE_DIR)/osp/etc/cmd/" ; \
	    cp -fr $(KLISH_DIR)/xml-examples/tapcmd/* $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
        if [ 'goldengate' == $(CHIPNAME) ] ; then \
            cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v580/*.xml $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
        elif [ 'duet2' == $(CHIPNAME) ] ; then \
		    cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v550/*.xml $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
		elif [ 'tsingma' == $(CHIPNAME) ] ; then \
		    cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v530/*.xml $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
        else \
            cp -fr $(KLISH_DIR)/xml-examples/tapcmd/v350/*.xml $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
        fi; \
	elif [ 'of' == $(PRODUCT) ] ; then \
	    echo " " ; \
    elif [ 'ttp' == $(PRODUCT) ] ; then \
	    echo"cp -fr $(KLISH_DIR)/xml-examples/ofcmd/* $(UML_IMAGE_DIR)/osp/etc/cmd/" ; \
	    cp -fr $(KLISH_DIR)/xml-examples/ofcmd/* $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
	else \
		cp -fr $(KLISH_DIR)/xml-examples/ospcmd/* $(UML_IMAGE_DIR)/osp/etc/cmd/ ; \
	fi;
image:FORCE
#package: FORCE
	@echo "Copy binaries to $(PRODUCT) UML directory: ../umlimage/osp/sbin"
	@mkdir -p $(UML_IMAGE_DIR)/osp/etc
	@mkdir -p $(UML_IMAGE_DIR)/osp/conf
	@mkdir -p $(UML_IMAGE_DIR)/osp/mibs
	@mkdir -p $(UML_IMAGE_DIR)/osp/sbin
	@cp -fr $(BLD_DIR)/mibs/* $(UML_IMAGE_DIR)/osp/mibs
	@cp -fr $(BLD_DIR)/mibd/* $(UML_IMAGE_DIR)/osp/sbin/
	@cp -fr $(BLD_DIR)/mibd/mib_modules/common/* $(UML_IMAGE_DIR)/osp/sbin/
	@cp -fr $(BLD_DIR)/cem/* $(UML_IMAGE_DIR)/osp/sbin/
	@mkdir -p $(UML_IMAGE_DIR)/osp/etc/openflow
	@cp -fr $(BLD_DIR)/etc/* $(UML_IMAGE_DIR)/osp/etc/
	@if [ ! -f $(UML_IMAGE_DIR)/osp/sbin ] ; then \
		mkdir -p $(UML_IMAGE_DIR)/osp/sbin ; \
		mkdir -p $(UML_IMAGE_DIR)/osp/etc/cmd ; \
	fi
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/sshd_uml $(UML_IMAGE_DIR)/osp/sbin/sshd
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/ssh $(UML_IMAGE_DIR)/osp/sbin/ssh
	@mkdir -p $(UML_IMAGE_DIR)/osp/bin 
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/scp $(UML_IMAGE_DIR)/osp/bin/sshd
	@mkdir -p $(UML_IMAGE_DIR)/osp/lib/
	@mkdir -p $(UML_IMAGE_DIR)/osp/lib/security
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/libpam* $(UML_IMAGE_DIR)/osp/lib/
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/pam_*.so $(UML_IMAGE_DIR)/osp/lib/security
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/mod_*.so $(UML_IMAGE_DIR)/osp/lib
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/neo_cgi.so $(UML_IMAGE_DIR)/osp/lib
	@if [ 'x86_64' == $(CPU) ] ; then \
		cp -fr $(TOP_DIR)/third_party/precompiled/uml/64bit/libpam* $(UML_IMAGE_DIR)/osp/lib/ ; \
	fi
	@cp -fr $(BLD_DIR)/lib/lib*.so $(UML_IMAGE_DIR)/osp/lib/
	@cp -fr $(BLD_DIR)/lib/pam_*.so $(UML_IMAGE_DIR)/osp/lib/security
	@cp -fr $(BLD_DIR)/lib/libnss_* $(UML_IMAGE_DIR)/osp/lib
	@if [ 'yes' == $(ENABLE_NETCONF) ]; then \
		mkdir -p $(UML_IMAGE_DIR)/osp/lib/sysrepo/plugins; \
		mkdir -p $(UML_IMAGE_DIR)/osp/lib/libyang/extensions; \
		mkdir -p $(UML_IMAGE_DIR)/osp/lib/libyang/user_types; \
		cp -fr $(BLD_DIR)/lib/metadata.so $(UML_IMAGE_DIR)/osp/lib/libyang/extensions; \
		cp -fr $(BLD_DIR)/lib/nacm.so $(UML_IMAGE_DIR)/osp/lib/libyang/extensions; \
		cp -fr $(BLD_DIR)/lib/yangdata.so $(UML_IMAGE_DIR)/osp/lib/libyang/extensions; \
		cp -fr $(BLD_DIR)/lib/oven.so $(UML_IMAGE_DIR)/osp/lib/sysrepo/plugins; \
	fi
	@mkdir -p $(UML_IMAGE_DIR)/osp/etc/ssh
	@cp -fr $(TOP_DIR)/third_party/precompiled/p1010/etc/* $(UML_IMAGE_DIR)/osp/etc/ssh
	@ln -sf $(TOP_DIR)/ryu/bin/ryu-manager $(UML_IMAGE_DIR)/osp/bin/ryu-manager
	@cp -fr $(BLD_DIR)/bin/* $(UML_IMAGE_DIR)/osp/sbin/ ;
	@cp -fr $(BUSYBOX_DIR)/busybox $(UML_IMAGE_DIR)/osp/sbin/ ;
	@cp -fr $(QUAGGA_TOP_DIR)/zebra/zebra.conf.sample $(UML_IMAGE_DIR)/osp/etc/zebra.conf ;
	@cp -fr $(QUAGGA_TOP_DIR)/ospfd/ospfd.conf.sample $(UML_IMAGE_DIR)/osp/etc/ospfd.conf ;
	@cp -fr $(REDIS_DIR)/redis.conf $(UML_IMAGE_DIR)/osp/etc/redis.conf ;
	@cp -fr $(BLD_DIR)/etc/cmd/* $(UML_IMAGE_DIR)/osp/etc/cmd/ ;
	@mkdir -p $(UML_IMAGE_DIR)/osp/etc/profile
	@if [ ! -f $(UML_IMAGE_DIR)/osp/etc/profile/tcam_mode.txt ] ; then \
		cp -fr $(TOP_DIR)/proto/chsm/profile/* $(UML_IMAGE_DIR)/osp/etc/profile/ ; \
	fi
	@cp -fr $(PLAT_TOP_DIR)/build_svc/etc/openflow/scripts/openflow_controller_op.py $(UML_IMAGE_DIR)/osp/sbin/openflow_controller_op.py
	@cp -fr $(PLAT_TOP_DIR)/build_svc/etc/openflow/uml/conf.db $(UML_IMAGE_DIR)/osp/etc/openflow/conf.db
	@cp -fr $(PLAT_TOP_DIR)/build_svc/etc/init-veth.sh $(UML_IMAGE_DIR)/osp/etc/init-veth.sh
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/conntrack $(UML_IMAGE_DIR)/osp/sbin/conntrack
	@cp -fr $(TOP_DIR)/apps/ntp/ntpd/ntp-iptables-uml.sh $(UML_IMAGE_DIR)/osp/sbin/ntp-iptables.sh
	@cp -fr $(TOP_DIR)/third_party/precompiled/uml/bin/mgmt-ip.sh $(UML_IMAGE_DIR)/osp/sbin/mgmt-ip.sh
	@find $(UML_IMAGE_DIR)/osp/sbin/ -name "[^lib]*" -exec chmod a+x {} \;
	@echo "Finish!";
endif

ifeq ($(ARCH),powerpc)

mini_initramfs:FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_mini_ramfs_powerpc.sh && ./build_mini_ramfs_powerpc.sh

base_image: mini_initramfs
	@echo "Package powerpc binaries, please wait..."; \
	@echo "Generate kernel..."; \
	cd $(KDIR); \
	$(OSP_MAKE) uImage
	@echo "Create powerpc all in one image..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	mkimage -A ppc \
		-O linux -T multi -C gzip -a 00000000 \
		-e 00000000 -n "v$(version_num)" -d ${KDIR}/vmlinux.bin.gz:${KDIR}/mini_initramfs.lzma:${KDIR}/arch/powerpc/boot/$(BOARD)_ctc.dtb \
		$(PLAT_TOP_DIR)/build_svc/$(IMAGE_NAME_PREFIX).$(CPU).$(BOARD).$(VER); \
	echo "Enjoy It!";

ifeq ($(ENABLE_DEBIAN_LINUX),discard)    
debian_image:
	@echo "Generate Image is TODO..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_powerpc_debian.sh && ./build_powerpc_debian.sh; \
	mv onie-installer cnosall.$(CPU).$(BOARD).$(VER); \
	mv centec_cnos.deb cnos.$(CPU).$(BOARD).$(VER)
	echo "Enjoy It!";

image:FORCE
	@echo "Package powerpc cboot binaries, please wait..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_powerpc_cboot.sh && ./build_powerpc_cboot.sh
	@echo "Generate kernel..."; \
	cd $(KDIR); \
	$(OSP_MAKE) uImage
	@echo "Create powerpc cboot image..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	mkimage -A ppc \
		-O linux -T multi -C gzip -a 00000000 \
		-e 00000000 -n "v$(version_num)" -d ${KDIR}/vmlinux.bin.gz:${KDIR}/initramfs.lzma:${KDIR}/arch/powerpc/boot/$(BOARD)_ctc.dtb \
		$(PLAT_TOP_DIR)/build_svc/$(IMAGE_NAME_PREFIX).$(CPU).$(BOARD).$(VER); \
	echo "Enjoy It!";
    
else
image:FORCE
	@echo "Package powerpc binaries, please wait..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_powerpc.sh && ./build_powerpc.sh || exit "$$?";
	@echo "Generate kernel..."; \
	cd $(KDIR); \
	$(OSP_MAKE) uImage || exit"$$?";
	@echo "Create powerpc all in one image..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	mkimage -A ppc \
		-O linux -T multi -C gzip -a 00000000 \
		-e 00000000 -n "v$(version_num)" -d ${KDIR}/vmlinux.bin.gz:${KDIR}/initramfs.lzma:${KDIR}/arch/powerpc/boot/$(BOARD)_ctc.dtb \
		$(PLAT_TOP_DIR)/build_svc/$(IMAGE_NAME_PREFIX).$(CPU).$(BOARD).$(VER); \
	echo "Enjoy It!";
    
endif
    
endif

ifeq ($(ARCH),arm)
mini_initramfs:FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_mini_ramfs_arm.sh && ./build_mini_ramfs_arm.sh

main_initramfs: FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_main_ramfs_arm.sh && ./build_main_ramfs_arm.sh

image:mini_initramfs main_initramfs
	@echo "Generate kernel..."; \
	cd $(KDIR); \
	make uImage
	@echo "Create arm all in one image..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	./mkimage_arm -A arm \
		-O linux -T multi -C none -a 0x80008000 \
		-e 0x80008000 -n "v$(version_num)" -d ${KDIR}/arch/arm/boot/compressed/vmlinux:${KDIR}/mini_initramfs.lzma\
		$(PLAT_TOP_DIR)/build_svc/$(IMAGE_NAME_PREFIX).$(CPU).$(BOARD).$(VER); \
		dd if=${KDIR}/main_initramfs.lzma of=$(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER) \
		ibs=1024 obs=1024 seek=8192 ;\
	echo "Enjoy It!";
endif

ifeq ($(ARCH),arm64)
ifeq ($(CPU), ls1023a)
main_initramfs: FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_main_ramfs_arm64.sh && ./build_main_ramfs_arm64.sh

mini_initramfs: main_initramfs FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_mini_ramfs_arm64.sh && ./build_mini_ramfs_arm64.sh

image:mini_initramfs main_initramfs
	@echo "Create arm64 all in one image..."; \
	cd $(ROOT_DIR);\
	cd $(PLAT_TOP_DIR)/build_svc; \
	if [ "$(CHIPNAME)" = "duet2" ];then\
		./mkimage_arm64 -n "v$(version_num)" -f ./its/linux_arm64_ls1023a_duet2.$(VER).its $(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER);\
		echo -n "version:v$(version_num)#" >> $(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER);\
	else \
		./mkimage_arm64 -n "v$(version_num)" -f ./its/linux_arm64_ls1023a_tsingma.$(VER).its $(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER);\
		echo -n "version:v$(version_num)#" >> $(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER);\
	fi
	echo "Enjoy It!";
endif
ifeq ($(CPU), ctc5236)
main_initramfs: FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_main_ramfs_arm64.sh && ./build_main_ramfs_arm64.sh

mini_initramfs: main_initramfs FORCE
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_mini_ramfs_arm64.sh && ./build_mini_ramfs_arm64.sh

image:mini_initramfs main_initramfs
	@echo "Create arm64 all in one image..."; \
	cd $(ROOT_DIR);\
	cd $(PLAT_TOP_DIR)/build_svc; \
	./mkimage_arm64_ctc5236 -n "v$(version_num)" -f ./its/linux_arm64_ctc5236.$(VER).its $(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER);\
	echo -n "version:v$(version_num)#" >> $(PLAT_TOP_DIR)/build_svc/uImage.$(CPU).$(BOARD).$(VER);\
	echo "Enjoy It!";
endif
endif

ifeq ($(ARCH),x86_64)
image:
	@echo "Generate Image is TODO..."; \
	cd $(PLAT_TOP_DIR)/build_svc; \
	chmod +x ./build_x86_64.sh && ./build_x86_64.sh; \
	mv onie-installer cnosall.$(CPU).$(BOARD).$(VER); \
	mv centec_cnos.deb cnos.$(CPU).$(BOARD).$(VER)
	echo "Enjoy It!";
endif

clean_package: FORCE
	rm -fr $(BLD_DIR)/bin

clean_debian_package: FORCE
	@echo "Cleaning debian package"
	@rm -rf $(TEMP_ROOTFS_DIR)

tags: FORCE
	find . -type f | grep -v 'goldengate' | grep -v 'humber' | grep -v 'greatbelt.bak.with.gg' | grep -v '.svn' | grep -v 'ryu' | grep -v '.java' | grep -v 'cscope.out' | grep -v 'tags' >/tmp/cscope.files
	cscope -bk -i/tmp/cscope.files
	ctags -R --exclude=$(SDK_DIR)/core/goldengate --exclude=$(SDK_DIR)/core/humber --exclude=$(CMODEL_DIR)/goldengate --exclude=$(CMODEL_DIR)/humber --exclude=$(CMODEL_DIR)/greatbelt.bak.with.gg --exclude=$(SDK_DIR)/driver/humber --exclude=$(SDK_DIR)/driver/goldengate --exclude=$(SDK_DIR)/mem_model/goldengate --exclude=ryu/ .

.PHONY: FORCE
FORCE:
third_party_compile:
	$(OSP_MAKE) -C third_party/compile/ -f Makefile
	INSTALL_DIR_BIN="$(BLD_DIR)/bin" $(OSP_MAKE) -C third_party/compile/ -f Makefile install

clean_third_party_compile:
	$(OSP_MAKE) -C third_party/compile/ -f Makefile clean
