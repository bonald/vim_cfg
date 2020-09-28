CPPFLAGS += -I$(REDIS_DIR)/deps

#LIBLUA       = $(LIB_DIR)/liblua.a
#LIBGEOHASH   = $(LIB_DIR)/libgeohash.a
#LIBHIREDIS   = $(LIB_DIR)/libhiredis.a
#LIBJEMALLOC  = $(LIB_DIR)/libjemalloc.a
#LIBLINENOISE = $(LIB_DIR)/liblinenoise.a

CPPFLAGS+= -I$(REDIS_DIR)/deps/hiredis -I$(REDIS_DIR)/deps/linenoise 
CPPFLAGS+= -I$(REDIS_DIR)/deps/lua/src -I$(REDIS_DIR)/deps/jemalloc/include/jemalloc
CPPFLAGS+= -I$(REDIS_DIR)/deps/jemalloc/include
CPPFLAGS+= -I$(REDIS_DIR)/deps/geohash-int

