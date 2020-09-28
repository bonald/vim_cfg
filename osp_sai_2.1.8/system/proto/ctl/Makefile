all_targets =
all_targets += cdbctl
all_targets += viewctl
all_targets += reactorctl

all: $(all_targets) FORCE

cdbctl: FORCE
	make -f Makefile.ctl DEST_CTL=$@

clean_cdbctl: FORCE
	make -f Makefile.ctl clean DEST_CTL=$@

viewctl: FORCE
	make -f Makefile.ctl DEST_CTL=$@

clean_viewctl: FORCE
	make -f Makefile.ctl clean DEST_CTL=$@

reactorctl: FORCE
	make -f Makefile.ctl DEST_CTL=$@

clean_reactorctl: FORCE
	make -f Makefile.ctl clean DEST_CTL=$@

.PHONY: FORCE
FORCE:
