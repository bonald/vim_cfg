include ../../../Makefile.env.inc
include $(MK_DIR)/sys.mk

CFLAGS += -Wno-error=deprecated-declarations

ifeq ($(ARCH),um)
GLIB_INCDIR=$(shell pkg-config --cflags glib-2.0)
GLIB_LIB=$(shell pkg-config --libs glib-2.0)
endif

ifeq ($(ARCH),powerpc)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
GLIB_LIB+= -lglib-2.0 -lrt
endif

ifeq ($(ARCH),arm)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
GLIB_LIB+= -lglib-2.0 -lrt
endif

ifeq ($(ARCH),arm64)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
GLIB_LIB+= -lglib-2.0 -lrt
endif

ifeq ($(ARCH),mips)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib/glib-2.0/include
GLIB_LIB+= -lglib-2.0 -lrt
endif

ifeq ($(ARCH),x86_64)
GLIB_INCDIR+= -I$(GLIB_USER)/include/glib-2.0
GLIB_INCDIR+= -I$(GLIB_USER)/lib64/glib-2.0/include
GLIB_LIB+= -lglib-2.0 -lrt
endif

CFLAGS+= -D_GNU_SOURCE -I$(CURDIR) -I$(CURDIR)/.. -I$(CURDIR)/../../eventlog/src $(GLIB_INCDIR)
CPPFLAGS+= -D_GNU_SOURCE -I$(CURDIR) -I$(CURDIR)/.. -I$(CURDIR)/../../eventlog/src $(GLIB_INCDIR)
CFLAGS+= -I$(OBJ_DIR) -DYY_NO_INPUT 
CPPFLAGS+= -I$(OBJ_DIR)
CPPFLAGS += -I$(CTC_LIB_DIR)/include

PROG = syslog-ng
SRCS = $(wildcard *.c)
EXTRA_OBJS+=$(OBJ_DIR)/cfg-grammar.o
EXTRA_OBJS+=$(OBJ_DIR)/cfg-lex.o

DEP_LIBS = $(LIB_DIR)/libevtlog.a
LD_LIBS = -L$(LIB_DIR) -levtlog $(GLIB_LIB)

ETC_DIR?=$(BLD_DIR)/etc
SYSLOGNG_CFG=syslog-ng.conf
INIT_SCRIPT=init-syslog-ng.sh

all: $(BIN_DIR)/$(PROG) $(ETC_DIR)/$(SYSLOGNG_CFG) $(BIN_DIR)/$(INIT_SCRIPT) FORCE

$(OBJ_DIR)/cfg-grammar.h: cfg-grammar.y $(OBJ_DIR)/cfg-grammar.c FORCE

$(OBJ_DIR)/cfg-grammar.c: cfg-grammar.y
	@bison -y -d  `test -f 'cfg-grammar.y' || echo './'`cfg-grammar.y
	@if test -f y.tab.h; then \
		to=`echo "cfg-grammar_H" | sed \
                -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' \
                -e 's/[^ABCDEFGHIJKLMNOPQRSTUVWXYZ]/_/g'`; \
		sed "/^#/ s/Y_TAB_H/$to/g" y.tab.h >cfg-grammar.ht; \
		rm -f y.tab.h; \
		if cmp -s cfg-grammar.ht cfg-grammar.h; then \
			rm -f cfg-grammar.ht ;\
		else \
			mv cfg-grammar.ht cfg-grammar.h; \
		fi; \
	fi
	@if test -f y.output; then \
		mv y.output cfg-grammar.output; \
	fi
	@sed '/^#/ s|y\.tab\.c|cfg-grammar.c|' y.tab.c >cfg-grammar.ct && mv cfg-grammar.ct cfg-grammar.c
	@rm -f y.tab.c
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	mv cfg-grammar.c $@
	@mv cfg-grammar.h $(OBJ_DIR)/cfg-grammar.h

$(OBJ_DIR)/cfg-lex.c: cfg-lex.l $(OBJ_DIR)/cfg-grammar.h
	@flex   `test -f cfg-lex.l || echo './'`cfg-lex.l
	@sed '/^#/ s|lex.yy\.c|cfg-lex.c|' lex.yy.c >cfg-lex.c
	@rm -f lex.yy.c
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	mv cfg-lex.c $@

$(OBJ_DIR)/cfg-lex.d: $(OBJ_DIR)/cfg-lex.c
	echo "MYCX  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(@:%.d=%.o))" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $@ -c -o $(@:%.d=%.o) $<

$(OBJ_DIR)/cfg-grammar.d: $(OBJ_DIR)/cfg-grammar.c
	echo "CX  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(@:%.d=%.o))" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $@ -c -o $(@:%.d=%.o) $<

$(ETC_DIR)/$(SYSLOGNG_CFG): $(SYSLOGNG_CFG)
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $(SYSLOGNG_CFG) $@

$(BIN_DIR)/$(INIT_SCRIPT): $(INIT_SCRIPT) diaglogsync.sh logsync.sh diagdailyrotate.sh dailyrotate.sh dailyrotate.cron
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$@)" && \
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
	cp $(INIT_SCRIPT) $@
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(BIN_DIR)/diaglogsync.sh)" && \
	cp diaglogsync.sh $(BIN_DIR)/ && chmod a+x $(BIN_DIR)/diaglogsync.sh
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(BIN_DIR)/logsync.sh)" && \
	cp logsync.sh $(BIN_DIR)/ && chmod a+x $(BIN_DIR)/logsync.sh
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(BIN_DIR)/dailyrotate.sh)" && \
	cp dailyrotate.sh $(BIN_DIR)/ && chmod a+x $(BIN_DIR)/dailyrotate.sh
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(BIN_DIR)/diagdailyrotate.sh)" && \
	cp diagdailyrotate.sh $(BIN_DIR)/ && chmod a+x $(BIN_DIR)/diagdailyrotate.sh
	@echo "CP  => $(subst $(BLD_DIR),build.$(CPU).$(VER),$(ETC_DIR)/dailyrotate.cron)" && \
	cp dailyrotate.cron $(ETC_DIR)/

EXTRA_CLEANS=$(ETC_DIR)/$(SYSLOGNG_CFG) $(BIN_DIR)/$(INIT_SCRIPT)
EXTRA_CLEANS+=$(OBJ_DIR)/cfg-lex.c $(OBJ_DIR)/cfg-grammar.c $(OBJ_DIR)/cfg-grammar.h
EXTRA_CLEANS+=$(BIN_DIR)/logsync.sh $(BIN_DIR)/dailyrotate.sh $(ETC_DIR)/dailyrotate.cron

include $(MK_DIR)/prog.mk

.PHONY: FORCE
FORCE:
