#! /usr/bin/perl
##
## Virtual terminal interface shell command extractor.
## Copyright (C) 2000 Kunihiro Ishiguro
## 
## This file is part of GNU Zebra.
## 
## GNU Zebra is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2, or (at your option) any
## later version.
## 
## GNU Zebra is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with GNU Zebra; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.  
##

print <<EOF;
#include <zebra.h>
#include "command.h"
#include "vtysh.h"

EOF

$ignore{'"interface IFNAME"'} = "ignore";
$ignore{'"ip vrf NAME"'} = "ignore";
$ignore{'"router rip"'} = "ignore";
$ignore{'"router ripng"'} = "ignore";
$ignore{'"router ospf"'} = "ignore";
$ignore{'"router ospf <0-65535>"'} = "ignore";
$ignore{'"router ospf6"'} = "ignore";
$ignore{'"router bgp <1-65535>"'} = "ignore";
$ignore{'"router bgp <1-65535> view WORD"'} = "ignore";
$ignore{'"address-family ipv4"'} = "ignore";
$ignore{'"address-family ipv4 (unicast|multicast)"'} = "ignore";
$ignore{'"address-family ipv6"'} = "ignore";
$ignore{'"address-family ipv6 unicast"'} = "ignore";
$ignore{'"address-family vpnv4"'} = "ignore";
$ignore{'"address-family vpnv4 unicast"'} = "ignore";
$ignore{'"address-family ipv4 vrf NAME"'} = "ignore";
$ignore{'"exit-address-family"'} = "ignore";
$ignore{'"key chain WORD"'} = "ignore";
$ignore{'"key <0-2147483647>"'} = "ignore";
$ignore{'"route-map WORD (deny|permit) <1-65535>"'} = "ignore";
$ignore{'"show route-map"'} = "ignore";
$ignore{'"quit"'} = "ignore";
$ignore{'"end"'} = "ignore";
$ignore{'"exit"'} = "ignore";
$ignore{'"enter sdk mode"'} = "ignore";
$ignore{'"enter debug mode"'} = "ignore";
$ignore{'"enter sdk v1 mode"'} = "ignore";
$ignore{'"enter cmodel mode"'} = "ignore";
$ignore{'"oam cfm lookup-chan add gport GPORT_ID direction (up |down ) (link-oam |) (vlan VLAN_ID|) (master-gchip GCHIP_ID|)"'} = "ignore";
$ignore{'"oam cfm enter lookup-chan gport GPORT_ID direction (up |down ) (link-oam |) (vlan VLAN_ID|)"'} = "ignore";
$ignore{'"enter lookup-chan gport GPORT_ID direction (up| down ) (link-oam|) (vlan VLAN_ID|)"'} = "ignore";

foreach (@ARGV) {
    $file = $_;

    #open (FH, "cpp -I../../../lib/sal/include/ -I../../../include -I../../../include/lcm_drvs -I../../../sys/include -I../../../lib/afx -I../../vtysh -I../../libctccli/include/ -I../../kal/include -I../../core/api/include -I../../core/common/include -I../../core/humber/include/api -I../../core/humber/include/sys -I../../driver/common/include -I../../driver/humber/include -DVTYSH_EXTRACT_PL -I. -I.. -I../lib -I../../mem_model/include -DHUMBER $file |");
    #open (FH, "cpp -I./lib/sal/include/ -I./include -I./include/lcm_drvs -I./sys/include -I./lib/afx -I./vtysh -I./libctccli/include/ -I./sdk/kal/include -I./sdk/core/api/include -I./sdk/core/common/include -I./sdk/core/humber/include/api -I./sdk/core/humber/include/sys -I./sdk/driver/common/include -I./sdk/driver/humber/include -DVTYSH_EXTRACT_PL -I./sdk/ctccli/sdkcli -I./sdk/ctccli/sdkcli/humber -I./sdk/ctccli/diagcli/humber -I./sdk/ctccli -I../lib -I./sdk/mem_model/include -DHUMBER $file |");
    #open (FH, "cpp -I./lib/sal/include/ -I./include -I./include/lcm_drvs -I./sys/include -I./lib/afx -I./vtysh -I./libctccli/include/ -I../../sdk/sal/include -I../../sdk/core/api/include -I./sdk/core/api/include -I../../sdk/core/common/include -I../../sdk/core/greatbelt/include/api -I../../sdk/core/greatbelt/include/sys -I../../sdk/driver/common/include -I../../sdk/driver/greatbelt/include -DVTYSH_EXTRACT_PL -I../../sdk/ctccli/sdkcli -I../../sdk/ctccli/sdkcli/greatbelt -I../../sdk/ctccli/diagcli/greatbelt -I../../sdk/ctccli -I../../lib -I../../sdk/libctccli/include -I./sdk/libctccli/include -I../../sdk/mem_model/include -DHUMBER $file |");
    open (FH, "cpp -I./tools/vtysh/lib/sal/include/ -I./tools/vtysh/include -I./include/lcm_drvs -I./sys/include -I./tools/vtysh/lib/afx -I./tools/vtysh -I./tools/vtysh/libctccli/include/ -I./sdk/sal/include -I./sdk/core/api/include -I./sdk/core/common/include -I./sdk/core/greatbelt/include/api -I./sdk/core/greatbelt/include/ctc -I./sdk/core/greatbelt/include/sys -I./sdk/driver/common/include -I./sdk/driver/greatbelt/include -DVTYSH_EXTRACT_PL -DVTISH_EXTRACT_PL -I./sdk/ctccli/sdkcli -I./sdk/ctccli/sdkcli/greatbelt -I./sdk/ctccli/diagcli/greatbelt -I./sdk/ctccli -I./sdk/libctccli/include -I../lib -I./sdk/mem_model/include -I./sdk/dal/include -DHUMBER $file |");
    local $/; undef $/;
    $line = <FH>;
    close (FH);

    @defun = ($line =~ /(?:DEFUN|ALIAS)\s*\((.+?)\);?\s?\s?\n/sg);
    @install = ($line =~ /\s*install_element\s*\(\s*[0-9a-zA-Z_]+,\s*&[^;]*;\s*\n/sg);
    @helpname = ($line =~ /([0-9A-Za-z_]+)_help\[\]\s=\s\{/sg);
    @helpstr = ($line =~ /_help\[\]\s=\s\{(.+?)\};\s/sg);

    # DEFUN process
    foreach (@defun) {
	my (@defun_array);
	@defun_array = split (/,/);
	$defun_array[0] = '';


	# Actual input command string.
	$str = "$defun_array[2]";
	$str =~ s/^\s+//g;
	$str =~ s/\s+$//g;

	# Get VTY command structure.  This is needed for searching
	# install_element() command.
	$cmd = "$defun_array[1]";
	$cmd =~ s/^\s+//g;
	$cmd =~ s/\s+$//g;

        # $protocol is VTYSH_PROTO format for redirection of user input
    	if ($file =~ /lib/) {
           if ($file =~ /keychain.c/) {
              $protocol = "VTYSH_RIPD";
           }
           if ($file =~ /routemap.c/) {
              $protocol = "VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD";
           }
           if ($file =~ /filter.c/) {
              if ($defun_array[1] =~ m/ipv6/) {
                 $protocol = "VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD";
	      } else {
                 $protocol = "VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD";
              }
           }
           if ($file =~ /plist.c/) {
	      if ($defun_array[1] =~ m/ipv6/) {
                 $protocol = "VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD";
              } else {
                 $protocol = "VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD";
              }
           }
           if ($file =~ /distribute.c/) {
              if ($defun_array[1] =~ m/ipv6/) {
                 $protocol = "VTYSH_RIPNGD";
              } else {
                 $protocol = "VTYSH_RIPD";
              }
           }
       } elsif ($file =~ /ctc_master_cli.c/) {
           $protocol = "VTYSH_SDK";
       } elsif ($file =~ /_cli.c/) {
           $protocol = "VTYSH_SDK";
        } else {
           ($protocol) = ($file =~ /\/([a-z0-9]+)/);
           $protocol = "VTYSH_" . uc $protocol;
        }

	# Append _vtysh to structure then build DEFUN again
	$defun_array[1] = $cmd . "_vtysh";
	$defun_body = join (", ", @defun_array);

	# $cmd -> $str hash for lookup
	$cmd2str{$cmd} = $str;
	$cmd2defun{$cmd} = $defun_body;
	$cmd2proto{$cmd} = $protocol;
    }

    # install_element() process
    foreach (@install) {
	my (@element_array);
	@element_array = split (/,/);
	#modified by cuixl, install all sdk cli as SDK_MODE, we interate all cli mode in vtysh server	
	# Install node
	#$enode = $element_array[0];
	#$enode =~ s/^\s+//g;
	#$enode =~ s/\s+$//g;
	#($enode) = ($enode =~ /([0-9A-Z_]+)$/);
	$enode = 1;
	#modified end
	# VTY command structure.
	($ecmd) = ($element_array[1] =~ /&([^\)]+)/);
	$ecmd =~ s/^\s+//g;
	$ecmd =~ s/\s+$//g;

	# Register $ecmd
	if (defined ($cmd2str{$ecmd})
	    && ! defined ($ignore{$cmd2str{$ecmd}})) {
	    my ($key);
	    $key = $enode . "," . $cmd2str{$ecmd};
	    $ocmd{$key} = $ecmd;
	    $odefun{$key} = $cmd2defun{$ecmd};
	    push (@{$oproto{$key}}, $cmd2proto{$ecmd});
    }

    $count = 0;
    foreach (@helpname) {
        $helpdefun{$helpname[$count]} = $helpstr[$count];
        $count++;
    }
    
    }
}

# Check finaly alive $cmd;
foreach (keys %odefun) {
    my ($node, $str) = (split (/,/));
    my ($cmd) = $ocmd{$_};
    $live{$cmd} = $_;
}

# Output Help string defines
foreach (keys %helpdefun) {
    print "char *$_";
    print "_help[] = { $helpdefun{$_} };\n\n";
}

# Output DEFSH
foreach (keys %live) {
    my ($proto);
    my ($key);
    $key = $live{$_};
    $proto = join ("|", @{$oproto{$key}});
    printf "DEFSH ($proto$odefun{$key})\n\n";
}

# Output install_element
print <<EOF;
void
vtysh_init_cmd ()
{
EOF

foreach (keys %odefun) {
    my ($node, $str) = (split (/,/));
    $cmd = $ocmd{$_};
    $cmd =~ s/_cmd/_cmd_vtysh/;
    printf "  install_element ($node, &$cmd);\n";
}

print <<EOF
}
EOF
