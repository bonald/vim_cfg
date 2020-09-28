all_targets =
all_targets += ptf_lib
all_targets += ptf_client
all_targets += ptf_server

all: $(all_targets) FORCE

ptf_lib: FORCE
	make -f Makefile.lib

clean_ptf_lib: FORCE
	make -f Makefile.lib clean

ptf_client: ptf_lib FORCE
	make -f Makefile.client

clean_ptf_client: FORCE
	make -f Makefile.client clean

ptf_server: ptf_lib FORCE
	make -f Makefile.server

clean_ptf_server: FORCE
	make -f Makefile.server clean

.PHONY: FORCE
FORCE:
