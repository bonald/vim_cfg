include ../Makefile.env.inc
all_targets += sal
all_targets += ctclib
all_targets += task
all_targets += sock
all_targets += genlog
all_targets += derhex
all_targets += json

all: $(all_targets) FORCE

clean: $(addprefix clean_,$(all_targets)) FORCE

derhex: FORCE
	make -C derhex

clean_derhex: FORCE
	make -C derhex clean

sal: FORCE
	make -C sal

clean_sal: FORCE
	make -C sal clean

ctclib: FORCE
	make -C util

clean_ctclib: FORCE
	make -C util clean

task: FORCE
	make -C task

clean_task: FORCE
	make -C task clean

sock: FORCE
	make -C sock

clean_sock: FORCE
	make -C sock clean

genlog: FORCE
	make -C genlog

clean_genlog: FORCE
	make -C genlog clean
	
json: FORCE
	make -C json

clean_json: FORCE
	make -C json clean
	
.PHONY: FORCE
FORCE:
