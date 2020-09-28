include ../../Makefile.env.inc

all_targets =
#all_targets += liblua
#all_targets += libredis
all_targets += cds

all: $(all_targets) FORCE
	
clean: $(addprefix clean_,$(all_targets)) FORCE

liblua: FORCE
	make -C redis/deps/lua -f Makefile

clean_liblua: FORCE
	make -C redis/deps/lua -f Makefile clean

libredis: FORCE
	make -C redis -f Makefile

clean_libredis: FORCE
	make -C redis -f Makefile clean

cds: FORCE
	make -f Makefile.cds

clean_cds: FORCE
	make -f Makefile.cds clean

.PHONY: FORCE
FORCE:
