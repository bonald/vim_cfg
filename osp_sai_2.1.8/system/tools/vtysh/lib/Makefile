
include ../Makefile.env.inc

all_targets += sal
all_targets += afx
all_targets += ctclib

ifeq ($(CPU),i686)
all_targets += libcmodel
endif

all: $(all_targets) FORCE

clean: $(addprefix clean_,$(all_targets)) FORCE

libcmodel: FORCE
	@if [ ! -f $(BLD_DIR)/lib/libcmodel.a ]; then \
		echo "Copy libcmodel.a to it's destination!" ; \
		mkdir -p $(BLD_DIR)/lib ; \
		cp ./pre-compiled/libcmodel.a $(BLD_DIR)/lib/libcmodel.a ; \
	fi;

clean_libcmodel: FORCE
	@rm -fr $(BLD_DIR)/lib/libcmodel.a

afx: FORCE
	make -C afx

clean_afx: FORCE
	make -C afx clean

ctclib: FORCE
	make -C util

clean_ctclib: FORCE
	make -C util clean

sal: FORCE
	make -C sal

clean_sal: FORCE
	make -C sal clean

.PHONY: FORCE
FORCE:
