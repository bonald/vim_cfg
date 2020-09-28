include $(TOP_DIR)/Makefile.env.inc
include $(MK_DIR)/sys.mk

### proto header path
### include $(PROTO_DIR)/Makefile.inc

PROG = fea
#SRCS = $(shell find hsrv -name "*.c")

ifeq ($(PRODUCT),ttp)
#SRCS = $(shell find hsrv \( -path hsrv/src/openflow -o -path hsrv/src/openflow_mdttp \) -prune -o -name '*.c' -print)
SRCS = $(shell find hsrv \( -path hsrv/src/openflow -o -path hsrv/src/openflow_ttp \) -prune -o -name '*.c' -print)
else
SRCS = $(shell find hsrv \( -path hsrv/src/openflow_ttp -o -path hsrv/src/openflow_mdttp \) -prune -o -name '*.c' -print)
endif

ifeq ($(PRODUCT),sai)
#CPPFLAGS += -DLDB_SYNC_TO_RDB
endif

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/ipc
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(CTC_LIB_DIR)/libasn1
CPPFLAGS += -I$(CTC_LIB_DIR)/msg/hagtmsg/gen
CPPFLAGS += -I$(CTC_LIB_DIR)/dbclient

### SDK sal headers should below osp sal because its subset of osp sal headers.
#CPPFLAGS += -I$(TOP_DIR)/sdk/sal/include

CPPFLAGS += -I$(PROTO_DIR)/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include/ds
CPPFLAGS += -I$(CDB_DIR)/core/include
CPPFLAGS += -I$(SDB_DIR)/core/include
CPPFLAGS += -I$(PROTO_DIR)/hal/include

ifeq ($(PRODUCT),ttp)
#CPPFLAGS += -I$(PROTO_DIR)/openflow_ttp/adapt/lib
#CPPFLAGS += -I$(FEA_DIR)/hsrv/include/openflow_ttp
#CPPFLAGS += -I$(FEA_DIR)/hsrv/src/openflow_ttp/ctc_goldengate_ttp_drv/include
CPPFLAGS += -I$(PROTO_DIR)/openflow_mdttp/adapt/lib
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/openflow_mdttp
CPPFLAGS += -I$(FEA_DIR)/hsrv/src/openflow_mdttp/ctc_goldengate_ttp_drv/include
else
CPPFLAGS += -I$(PROTO_DIR)/openflow/adapt/lib
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/openflow
endif

CPPFLAGS += -I$(FEA_DIR)/hsrv/api
CPPFLAGS += -I$(FEA_DIR)/hsrv/include
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/common
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/intf
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/l2
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/l3
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/g8032
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/acl
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/mpls
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/qos
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/misc
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/hostif
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/tap
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/ptp
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/copp
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/db_cdb
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/sdb
CPPFLAGS += -I$(FEA_DIR)/hsrv/include/wb
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/common
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/intf
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/l2
CPPFLAGS += -I$(FEA_DIR)/hsrv/msg/openflow

CPPFLAGS += -I$(SAI_DIR)/inc
CPPFLAGS += -I$(SAI_DIR)/include
CPPFLAGS += -I$(SAI_DIR)/src/port
CPPFLAGS += -I$(SAI_DIR)/src/acl
CPPFLAGS += -I$(SAI_DIR)/src/common

# sdk header files disperse in these directories
CFLAGS += -I$(SDK_DIR)/core
CFLAGS += -I$(SDK_DIR)/core/common/include
CFLAGS += -I$(SDK_DIR)/core/api/include
CFLAGS += -I$(SDK_DIR)/core/init
CFLAGS += -I$(SDK_DIR)/driver/api

CPPFLAGS += -I$(FEA_DIR)/lcm/include
CPPFLAGS += -I$(FEA_DIR)/ptf_agent

# # for thrift c_glib compile
# ifeq ($(ARCH),um)
# # um is m32, then could not uses -I/usr/lib64/glib-2.0/include
# GLIB_INCDIR=$(shell pkg-config --cflags glib-2.0)
# GLIB_LIB= -lgobject-2.0 -lglib-2.0
# endif
# ifeq ($(ARCH),powerpc)
# GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
# GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
# GLIB_LIB= -lgobject-2.0 -lglib-2.0
# endif
# CPPFLAGS += -I$(THIRD_PARTY_DIR)
# CPPFLAGS += -I$(THIRD_PARTY_DIR)/thrift/c_glib
# CPPFLAGS += $(GLIB_INCDIR)

ifeq ($(CHIPNAME),goldengate)
ChipAgent = TRUE
endif

LIBPROTO = $(LIB_DIR)/libproto.so
LIBTASK  = $(LIB_DIR)/libtask.so
LIBSOCK  = $(LIB_DIR)/libsock.so
LIBCTCLIB= $(LIB_DIR)/libctclib.so
LIBGENLOG = $(LIB_DIR)/libgenlog.so

#LIBSDK   = $(LIB_DIR)/libctccli.a $(LIB_DIR)/liblibctccli.a $(LIB_DIR)/libsdkcore.a $(LIB_DIR)/libdrv.a $(LIB_DIR)/libdal.a $(LIB_DIR)/libchipagent.a
#LIBCMODEL= $(LIB_DIR)/libcmodelcli.a $(LIB_DIR)/libcmodel.a $(LIB_DIR)/libcmodelcli.a
LIBHAGT  = $(LIB_DIR)/libhagt.a
LIBLCM   = $(LIB_DIR)/liblcm.a
LIBLCMMSG= $(LIB_DIR)/liblcmmsg.a
LIBASN1  = $(LIB_DIR)/libasn1.a
LIBLAI   = $(LIB_DIR)/liblai.a
LIBLSRV  = $(LIB_DIR)/liblsrv.a
LIBSAI = $(LIB_DIR)/libsai.a

DEP_LIBS = $(LIBPROTO) $(LIBCDB) $(LIBHAGT) $(LIBLCM) $(LIBLCMMSG) $(LIBASN1) $(LIBSDK) $(LIBSAI) $(LIBLAI) $(LIBLSRV)

ifeq ($(PRODUCT), ofdpa)
LD_LIBS = -L$(LIB_DIR) -Wl,--start-group,-lproto,-lcjson,-lcdb,-lsdb,-lhagt,-llcm,-llcmmsg,-lasn1,-llai,-llsrv,-lofdpa,-lofagent_main,-lofagent,-lofdpadriver,--end-group
else
LD_LIBS = -L$(LIB_DIR) -Wl,--start-group,-lproto,-lcjson,-lcdb,-lsdb,-lhagt,-llcm,-llcmmsg,-lasn1,-llai,-lhiredis,-llsrv,--end-group
endif

ifeq ($(PRODUCT), ofdpa)
DEP_LIBS += $(LIB_DIR)/libofdpa.a
DEP_LIBS += $(LIB_DIR)/libofagent.a
DEP_LIBS += $(LIB_DIR)/libofagent_main.a
DEP_LIBS += $(LIB_DIR)/libofdpadriver.a
endif
DEP_LIBS += $(LIB_DIR)/libsdksal.a
DEP_LIBS += $(LIB_DIR)/libdkit.a
DEP_LIBS += $(LIB_DIR)/libhagt.a
DEP_LIBS += $(LIB_DIR)/libsdksal.a $(LIB_DIR)/libdal.a $(LIB_DIR)/libsdkcore.a $(LIB_DIR)/libsai.a $(LIB_DIR)/libdrv.a $(LIB_DIR)/liblcm.a
DEP_LIBS += $(LIB_DIR)/liblcmmsg.a $(LIB_DIR)/libctccli.a

LIBLIBCTCCLI  = $(LIB_DIR)/liblibctccli.a
LIBCTCCLI  = $(LIB_DIR)/libctccli.a
LIBSDK    = $(LIB_DIR)/libsdkcore.a
LIBMEMMODEL =
LIBCTCLI  = $(LIB_DIR)/libcmodelcli.a
LIBCMODEL = $(LIB_DIR)/libcmodel.a

LIBSAL    = $(LIB_DIR)/libsdksal.a
LIBDAL    = $(LIB_DIR)/libdal.a
LIBDRV    = $(LIB_DIR)/libdrv.a
LIBDKIT = $(LIB_DIR)/libdkit.a

ifeq ($(ChipAgent),TRUE)
LIBCHIPAGENT = $(LIB_DIR)/libchipagent.a
endif

ifeq ($(CHIPNAME),goldengate)
LIBMODELCPP = $(LIB_DIR)/libmodelcpp.a
endif
ifeq ($(CHIPNAME),duet2)
LIBMODELCPP = $(LIB_DIR)/libmodelcpp.a
endif
ifeq ($(CHIPNAME),tsingma)
LIBMODELCPP = $(LIB_DIR)/libmodelcpp.a
endif

ifeq ($(IS_UML),y)
ifeq ($(CHIPNAME),goldengate)
#DEP_LIBS += $(LIBMODELCPP) 
#LD_LIBS += -lmodelcpp
DEP_LIBS += $(LIBLIBCTCCLI) $(LIBCTCCLI) $(LIBSDK) $(LIBSAI) $(LIBCMODEL) $(LIBMODELCPP) $(LIBMEMMODEL) $(LIBDRV)  $(LIBDAL) $(LIBSAL) $(LIBDKIT)
LD_LIBS += -L$(LIB_DIR) -lctccli -Wl,--start-group,-ldkit,-llibctccli,-lcmodel,-lmodelcpp,-lcmodelcli,-lsai,-lsdksal,-lsdkcore,--end-group -lmemmodel -ldrv -ldal -lsdksal -lpthread -lstdc++
endif
ifeq ($(CHIPNAME),greatbelt)
DEP_LIBS += $(LIBLIBCTCCLI) $(LIBCTCCLI) $(LIBSDK) $(LIBSAI) $(LIBCMODEL) $(LIBMEMMODEL) $(LIBDRV) $(LIBDAL) $(LIBSAL) $(LIBDKIT)
LD_LIBS += -L$(LIB_DIR) -lctccli -Wl,--start-group,-ldkit,-llibctccli,-lcmodel,-lcmodelcli,-lsai,-lsdksal,-lsdkcore,--end-group -lmemmodel -ldrv -ldal -lsdksal -lpthread -lstdc++
endif
ifeq ($(CHIPNAME),duet2)
DEP_LIBS += $(LIBLIBCTCCLI) $(LIBCTCCLI) $(LIBSDK) $(LIBSAI) $(LIBCMODEL) $(LIBDRV) $(LIBDAL) $(LIBSAL) $(LIBDKIT)
LD_LIBS += -L$(LIB_DIR) -lctccli -Wl,--start-group,-llibctccli,-ldkit,-lcmodel,-lmodelcpp,-lcmodelcli,-lsai,-lsdksal,-lsdkcore,--end-group -ldrv -ldal -lsdksal -lpthread -lstdc++
endif
ifeq ($(CHIPNAME),tsingma)
DEP_LIBS += $(LIBLIBCTCCLI) $(LIBCTCCLI) $(LIBSDK) $(LIBSAI) $(LIBCMODEL) $(LIBDRV) $(LIBDAL) $(LIBSAL) $(LIBDKIT)
LD_LIBS += -L$(LIB_DIR) -lctccli -Wl,--start-group,-llibctccli,-ldkit,-lcmodel,-lmodelcpp,-lcmodelcli,-lsai,-lsdksal,-lsdkcore,--end-group -ldrv -ldal -lsdksal -lpthread -lstdc++
endif
else
#DEP_LIBS += $(LIB_DIR)/libdrvs.a 
#LD_LIBS += -ldrvs
#DEP_LIBS += $(LIB_DIR)/libvsc7390.a $(LIB_DIR)/libvsc7395.a $(LIB_DIR)/libvsc7398.a
#LD_LIBS += -lvsc7390 -lvsc7395 -lvsc7398

DEP_LIBS += $(LIB_DIR)/libclock.a $(LIB_DIR)/libbootrom.a
LD_LIBS += -lclock -lbootrom
DEP_LIBS += $(LIB_DIR)/libepld.a $(LIB_DIR)/libeeprom.a $(LIB_DIR)/libfiber.a
LD_LIBS += -lepld -leeprom -lfiber
DEP_LIBS += $(LIB_DIR)/libpower.a $(LIB_DIR)/libgpio.a $(LIB_DIR)/libphy.a $(LIB_DIR)/libfan.a $(LIB_DIR)/libvcm.a $(LIB_DIR)/libsysenv.a $(LIB_DIR)/libpoe.a
LD_LIBS += -lpower -lgpio -lphy -lfan -lvcm -lsysenv -lpoe
DEP_LIBS += $(LIB_DIR)/libled.a $(LIB_DIR)/libltc2991.a
LD_LIBS += -lled -lltc2991
DEP_LIBS += $(LIB_DIR)/libmux.a $(LIB_DIR)/libsensor.a $(LIB_DIR)/libvsc3308.a
LD_LIBS += -lmux -lsensor -lvsc3308
DEP_LIBS += $(LIB_DIR)/liblog.a
LD_LIBS += -llog
DEP_LIBS += $(LIB_DIR)/libasicmdio.a $(LIB_DIR)/libcpui2c.a $(LIB_DIR)/libcpupci.a $(LIB_DIR)/libcpuspi.a
LD_LIBS += -lasicmdio -lcpui2c -lcpupci -lcpuspi
ifeq ($(CPU),x86_64)
DEP_LIBS += $(DRV_TOP_DIR)/bus/cpu/i2c/src/susi/Driver/libSUSI-4.00.so
LD_LIBS += -Wl,-rpath, $(DRV_TOP_DIR)/bus/cpu/i2c/src/susi/Driver/libSUSI-4.00.so
endif

ifeq ($(ChipAgent),TRUE)
DEP_LIBS += $(LIBCHIPAGENT)
endif
DEP_LIBS += $(LIBLIBCTCCLI) $(LIBCTCCLI) $(LIBSDK) $(LIBSAI) $(LIBMEMMODEL) $(LIBDRV) $(LIBDAL) $(LIBSAL) $(LIBDKIT)
LD_LIBS += -L$(LIB_DIR) -Wl,--start-group,-ldkit,-llibctccli,-lctccli,-lsai,-lsdksal,-lsdkcore,--end-group -ldrv -ldal -lsdksal -lpthread -lstdc++
endif

ifeq ($(ChipAgent),TRUE)
LD_LIBS += -lchipagent
endif

LD_LIBS += -ldrv -ldal -lsdksal -lctccli -lsdkcore -llibctccli -ldkit -lsai -ldbclient

DEP_LIBS += $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
LD_LIBS += -lsock -ltask -lctclib -lgenlog -lsal -lm -lrt -lpthread -lpcap -lasn1 -lcrypt -lcrypto

include $(MK_DIR)/prog.mk
