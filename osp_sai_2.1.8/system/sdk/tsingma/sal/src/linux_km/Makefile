KSRCDIR = /lib/modules/$(shell uname -r)/build

EXTRA_CFLAGS += -Wno-strict-prototypes
EXTRA_CFLAGS += -D_SAL_LINUX_KM
EXTRA_CFLAGS += -I$(M)/../../include

obj-m    = sal.o

sal-objs = sal_lkm.o \
           sal_log.o \
           sal_mem.o \
           sal_task.o \
           sal_event.o \
           sal_mutex.o \
           ../common/sal_timer.o \
           ../common/sal_async_queue.o\
           ../common/sal.o
.PHONY: all
all:
	$(MAKE) -C $(KSRCDIR) M=$(shell pwd)

.PHONY: clean
clean:
	$(MAKE) -C $(KSRCDIR) M=$(shell pwd) clean
