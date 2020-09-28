ifeq ($(CPU), x86_64)
NFT="/opt/x86_64_rootfs/usr/bin/nft"
endif

ifeq ($(ARCH), arm64)
NFT="/opt/aarch64/usr/bin/nft"
endif

all_targets += libmnl
all_targets += libnftnl
all_targets += readline
#all_targets += ncurses
all_targets += libnfnetlink
all_targets += libnetfilter_conntrack
#all_targets += iptstate ---------> c++ code
all_targets += nft

all: $(all_targets) FORCE

clean: $(addprefix clean_,$(all_targets)) FORCE

libmnl: FORCE
	make -C libmnl-1.0.4 -f Makefile

clean_libmnl: FORCE
	make -C libmnl-1.0.4 -f Makefile clean

libnftnl: FORCE
	make -C libnftnl-1.1.1 -f Makefile

clean_libnftnl: FORCE
	make -C libnftnl-1.1.1 -f Makefile clean

readline: FORCE
	make -C readline-6.3 -f Makefile

clean_readline: FORCE
	make -C readline-6.3 -f Makefile clean

ncurses: FORCE
	make -C ncurses-6.0 -f Makefile

clean_ncurses: FORCE
	make -C ncurses-6.0 -f Makefile clean

libnfnetlink: FORCE
	make -C libnfnetlink-1.0.1 -f Makefile

clean_libnfnetlink: FORCE
	make -C libnfnetlink-1.0.1 -f Makefile clean

libnetfilter_conntrack: FORCE
	make -C libnetfilter_conntrack-1.0.7 -f Makefile

clean_libnetfilter_conntrack: FORCE
	make -C libnetfilter_conntrack-1.0.7 -f Makefile clean

iptstate: FORCE
	make -C iptstate-1.0.7 -f Makefile.ctc

clean_iptstate: FORCE
	make -C iptstate-1.0.7 -f Makefile.ctc clean

nft: FORCE
#	make -C nftables-0.9.0 -f Makefile
	@if [ -f ${NFT} ] ; then \
		cp -fr ${NFT} ${BLD_DIR}/bin/ ; \
	fi;

clean_nft: FORCE
#	make -C nftables-0.9.0 -f Makefile clean
	rm ${BLD_DIR}/bin/nft

.PHONY: FORCE
FORCE:
