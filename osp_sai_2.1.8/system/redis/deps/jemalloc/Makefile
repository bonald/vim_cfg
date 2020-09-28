export WARN_NOERROR = y
include $(MK_DIR)/sys.mk

LIB = jemalloc
SRCS = $(wildcard src/*.c)
CPPFLAGS +=  -D_GNU_SOURCE -D_REENTRANT
CFLAGS += -std=gnu99 -Wall -Werror=declaration-after-statement -pipe -g3 -fvisibility=hidden -O3 -funroll-loops
LIBS +=  -lpthread
CPPFLAGS += -I$(REDIS_DIR)/deps/jemalloc/include

include $(MK_DIR)/lib.mk
