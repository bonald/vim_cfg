#include <zebra.h>

#include "command.h"
#include "linklist.h"

#include "vtysh.h"

DEFSH (VTYSH_OSPFD, no_ip_ospf_authentication_key_authkey_addr_cmd_vtysh, 
       "no ip ospf authentication-key [AUTH_KEY [A.B.C.D]]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_priority_cmd_vtysh, 
       "ipv6 ospf6 priority (0-255)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Router priority\n"
       "Priority value\n"
       )

DEFSH (VTYSH_BGPD, vnc_defaults_no_l2rd_cmd_vtysh, 
       "no l2rd", 
       "Negate a command or set its defaults\n"
       "Specify default Local Nve ID value to use in RD for L2 routes\n")

DEFSH (VTYSH_ALL, access_list_standard_host_cmd_vtysh, 
       "access-list <(1-99)|(1300-1999)> <deny|permit> host A.B.C.D", 
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "A single host address\n"
       "Address to match\n")

DEFSH (VTYSH_BGPD, set_ipv6_nexthop_peer_cmd_vtysh, 
       "set ipv6 next-hop peer-address", 
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "Next hop address\n"
       "Use peer address (for BGP only)\n")

DEFSH (VTYSH_PIMD, show_ip_msdp_mesh_group_cmd_vtysh, 
       "show ip msdp mesh-group [json]", 
       "Show running system information\n"
       "IP information\n"
       "MSDP information\n"
       "MSDP mesh-group information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPNGD, ipv6_distribute_list_cmd_vtysh, 
       "ipv6 distribute-list [prefix] WORD <in|out> [WORD]", 
       "IPv6\n"
       "Filter networks in routing updates\n"
       "Specify a prefix\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")

DEFSH (VTYSH_ALL, ipv6_access_list_any_cmd_vtysh, 
       "ipv6 access-list WORD <deny|permit> any", 
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any prefixi to match\n")

DEFSH (VTYSH_ZEBRA, link_params_delay_var_cmd_vtysh, 
       "delay-variation (0-16777215)", 
       "Unidirectional Link Delay Variation\n"
       "delay variation in micro-second as decimal (0...16777215)\n")

DEFSH (VTYSH_PIMD, no_ip_msdp_mesh_group_source_cmd_vtysh, 
       "no ip msdp mesh-group WORD source [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Configure multicast source discovery protocol\n"
       "Delete MSDP mesh-group source\n"
       "mesh group name\n"
       "mesh group source\n"
       "mesh group local address\n")

DEFSH (VTYSH_RIPD, no_debug_rip_events_cmd_vtysh, 
       "no debug rip events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP events\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_area_stub_no_summary_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> stub no-summary", 
       "Negate a command or set its defaults\n"
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "OSPF6 area ID as a decimal value\n"
       "Configure OSPF6 area as stub\n"
       "Do not inject inter-area routes into area\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_distance_cmd_vtysh, 
       "no distance (1-255)", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "OSPF6 Administrative distance\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_hello_interval_cmd_vtysh, 
       "no ip ospf hello-interval [(1-65535) [A.B.C.D]]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Address of interface\n")

DEFSH (VTYSH_BGPD, neighbor_password_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> password LINE", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Set a password\n"
       "The password\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_metric_rmap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|rip|ospf|isis|pim|nhrp|table|vnc|vnc-direct>" " metric (0-4294967295) route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Metric for redistributed routes\n"
       "Default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, no_bgp_distance_source_access_list_cmd_vtysh, 
       "no distance (1-255) A.B.C.D/M WORD", 
       "Negate a command or set its defaults\n"
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n"
       "Access list name\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_router_cmd_vtysh, 
       "show ipv6 ospf6 database <*|adv-router> * A.B.C.D <detail|dump|internal>", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Any Link state Type\n"
       "Search by Advertising Router\n"
       "Any Link state ID\n"
       "Specify Advertising Router as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_sequence_number_cmd_vtysh, 
       "no ip prefix-list sequence-number", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Include/exclude sequence numbers in NVGEN\n")

DEFSH (VTYSH_BGPD, evpnrt5_network_cmd_vtysh, 
      "network <A.B.C.D/M|X:X::X:X/M> rd ASN:nn_or_IP-address:nn ethtag WORD label WORD esi WORD gwip <A.B.C.D|X:X::X:X> routermac WORD [route-map WORD]", 
      "Specify a network to announce via BGP\n"
      "IP prefix\n"
      "IPv6 prefix\n"
      "Specify Route Distinguisher\n"
      "VPN Route Distinguisher\n"
      "Ethernet Tag\n"
      "Ethernet Tag Value\n"
      "BGP label\n"
      "label value\n"
      "Ethernet Segment Identifier\n"
      "ESI value ( 00:11:22:33:44:55:66:77:88:99 format) \n"
      "Gateway IP\n"
      "Gateway IP ( A.B.C.D )\n"
      "Gateway IPv6 ( X:X::X:X )\n"
      "Router Mac Ext Comm\n"
      "Router Mac address Value ( aa:bb:cc:dd:ee:ff format)\n"
      "Route-map to modify the attributes\n"
      "Name of the route map\n")

DEFSH (VTYSH_OSPFD, ospf_default_information_originate_cmd_vtysh, 
       "default-information originate [<always|metric (0-16777214)|metric-type (1-2)|route-map WORD>]", 
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_OSPFD, ospf_area_nssa_translate_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> nssa <translate-candidate|translate-never|translate-always>", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "Configure NSSA-ABR for translate election (default)\n"
       "Configure NSSA-ABR to never translate\n"
       "Configure NSSA-ABR to always translate\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_prefix_cmd_vtysh, 
       "no ipv6 prefix-list WORD <deny|permit> <X:X::X:X/M|any>", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")

DEFSH (VTYSH_BGPD, bgp_wpkt_quanta_cmd_vtysh, 
       "write-quanta (1-10000)", 
       "How many packets to write to peer socket per run\n"
       "Number of packets\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_binding_json_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> binding json", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "Label Information Base (LIB) information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, debug_isis_events_cmd_vtysh, 
       "debug isis events", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS Events\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_prefix_longer_cmd_vtysh, 
       "show ipv6 prefix-list WORD X:X::X:X/M longer", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Lookup longer prefix\n")

DEFSH (VTYSH_ISISD, debug_isis_upd_cmd_vtysh, 
       "debug isis update-packets", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS Update related packets\n")

DEFSH (VTYSH_BGPD, no_bgp_bestpath_aspath_ignore_cmd_vtysh, 
       "no bgp bestpath as-path ignore", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Ignore as-path length in selecting a route\n")

DEFSH (VTYSH_PIMD, show_ip_pim_upstream_rpf_cmd_vtysh, 
       "show ip pim upstream-rpf [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM upstream source rpf\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, vnc_advertise_un_method_cmd_vtysh, 
       "vnc advertise-un-method encap-attr", 
       "VNC/RFP related configuration\n"
       "Method of advertising UN addresses\n"
       "Via Tunnel Encap attribute (in VPN SAFI)\n")

DEFSH (VTYSH_OSPFD, ospf_area_range_not_advertise_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> range A.B.C.D/M not-advertise", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "Area range prefix\n"
       "DoNotAdvertise this range\n")

DEFSH (VTYSH_BGPD, no_bgp_table_map_cmd_vtysh, 
       "no table-map WORD", 
       "Negate a command or set its defaults\n"
       "BGP table to RIB route download filter\n"
       "Name of the route map\n")

DEFSH (VTYSH_BGPD, set_weight_cmd_vtysh, 
       "set weight (0-4294967295)", 
       "Set values in destination routing protocol\n"
       "BGP weight for routing table\n"
       "Weight value\n")

DEFSH (VTYSH_PIMD, ip_pim_spt_switchover_infinity_plist_cmd_vtysh, 
       "ip pim spt-switchover infinity-and-beyond prefix-list WORD", 
       "IP information\n"
       "PIM information\n"
       "SPT-Switchover\n"
       "Never switch to SPT Tree\n"
       "Prefix-List to control which groups to switch\n"
       "Prefix-List name\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_interface_prefix_cmd_vtysh, 
       "show ipv6 ospf6 interface prefix [<X:X::X:X|X:X::X:X/M>] [<match|detail>]", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface infomation\n"
       "Display connected prefixes to advertise\n"
       "Display the route bestmatches the address\n"
       "Display the route\n"
       "Display the route matches the prefix\n"
       "Display details of the prefixes\n")

DEFSH (VTYSH_BGPD, vnc_no_vrf_policy_cmd_vtysh, 
       "no vrf-policy NAME", 
       "Negate a command or set its defaults\n"
       "Remove a VRF policy group\n"
       "VRF name\n")

DEFSH (VTYSH_BGPD, no_vpnv6_network_cmd_vtysh, 
       "no network X:X::X:X/M rd ASN:nn_or_IP-address:nn <tag|label> (0-1048575)", 
       "Negate a command or set its defaults\n"
       "Specify a network to announce via BGP\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Specify Route Distinguisher\n"
       "VPN Route Distinguisher\n"
       "VPN NLRI label (tag)\n"
       "VPN NLRI label (tag)\n"
       "Label value\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_rib_cmd_vtysh, 
       "debug zebra rib", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug RIB events\n")

DEFSH (VTYSH_ZEBRA, ip_route_tag_distance_label_cmd_vtysh, 
       "ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> tag (1-4294967295) (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ZEBRA, no_zebra_ptm_enable_if_cmd_vtysh, 
       "no ptm-enable", 
       "Negate a command or set its defaults\n"
       "Enable neighbor check with specified topology\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_import_list_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> import-list NAME", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFSH (VTYSH_OSPFD, no_write_multiplier_cmd_vtysh, 
      "no write-multiplier (1-100)",  "Negate a command or set its defaults\n"
      "Write multiplier\n"
      "Maximum number of interface serviced per write\n")

DEFSH (VTYSH_RIPD, rip_default_information_originate_cmd_vtysh, 
       "default-information originate", 
       "Control distribution of default route\n"
       "Distribute a default route\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_ipv4_password_word_cmd_vtysh, 
       "neighbor A.B.C.D password WORD", 
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "Configure password for MD5 authentication\n"
       "The password\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_router_preference_cmd_vtysh, 
       "no ipv6 nd router-preference [<high|medium|low>]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Default router preference\n"
       "High default router preference\n"
       "Medium default router preference (default)\n"
       "Low default router preference\n")

DEFSH (VTYSH_LDPD, ldp_show_l2vpn_atom_vc_json_cmd_vtysh, 
       "show l2vpn atom vc [json]", 
       "Show running system information\n"
       "Show information about Layer2 VPN\n"
       "Show Any Transport over MPLS information\n"
       "Show AToM virtual circuit information\n"
       "JavaScript Object Notation\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_priority_cmd_vtysh, 
              "no ospf priority [(0-255) [A.B.C.D]]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Router priority\n"
              "Priority\n"
              "Address of interface")

DEFSH (VTYSH_ZEBRA, ip_irdp_multicast_cmd_vtysh, 
       "ip irdp multicast", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "Use multicast mode\n")

DEFSH (VTYSH_BGPD, vnc_export_nvegroup_cmd_vtysh, 
       "vnc export <bgp|zebra> group-nve group NAME", 
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "NVE group,  used in 'group-nve' export mode\n"
       "NVE group\n" "Group name\n")

DEFSH (VTYSH_BGPD, neighbor_maximum_prefix_warning_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix (1-4294967295) warning-only", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Only give warning message when limit is exceeded\n")

DEFSH (VTYSH_BGPD, set_local_pref_cmd_vtysh, 
       "set local-preference (0-4294967295)", 
       "Set values in destination routing protocol\n"
       "BGP local preference path attribute\n"
       "Preference value\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_zebra_prefix_cmd_vtysh, 
       "no debug bgp zebra prefix <A.B.C.D/M|X:X::X:X/M>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Zebra messages\n"
       "Specify a prefix to debug\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_ospf_metric_rmap_cmd_vtysh, 
       "redistribute <ospf|table> (1-65535) metric (0-4294967295) route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n"
       "Metric for redistributed routes\n"
       "Default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_cmd_vtysh, 
       "show ip ospf (1-65535) [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, neighbor_send_community_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> send-community", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Send Community attribute to this neighbor\n")

DEFSH (VTYSH_PIMD, no_ip_pim_ecmp_rebalance_cmd_vtysh, 
       "no ip pim ecmp rebalance", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Disable PIM ECMP \n"
       "Disable PIM ECMP Rebalance\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_ipv4_session_holdtime_session_time_cmd_vtysh, 
       "no neighbor A.B.C.D session holdtime (15-65535)", 
       "Negate a command or set its defaults\n"
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "Configure session parameters\n"
       "Configure session holdtime\n"
       "Time (seconds)\n")

DEFSH (VTYSH_RIPNGD, debug_ripng_events_cmd_vtysh, 
       "debug ripng events", 
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng events\n")

DEFSH (VTYSH_RIPNGD, no_ripng_allow_ecmp_cmd_vtysh, 
       "no allow-ecmp", 
       "Negate a command or set its defaults\n"
       "Allow Equal Cost MultiPath\n")

DEFSH (VTYSH_ISISD, no_isis_hello_interval_l1_cmd_vtysh, 
       "no isis hello-interval [(1-600)] level-1", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set Hello interval\n"
       "Holdtime 1 second,  interval depends on multiplier\n"
       "Specify hello-interval for level-1 IIHs\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_route_cmd_vtysh, 
       "show ip ospf (1-65535) route", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF routing table\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_ra_lifetime_cmd_vtysh, 
       "ipv6 nd ra-lifetime (0-9000)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router lifetime\n"
       "Router lifetime in seconds (0 stands for a non-default gw)\n")

DEFSH (VTYSH_BGPD, debug_rfapi_query_vn_un_l2o_cmd_vtysh, 
       "debug rfapi-dev query vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> lni LNI target YY:YY:YY:YY:YY:YY", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_query\n"
       "indicate vn addr follows\n"
       "virtual network interface IPv4 address\n"
       "virtual network interface IPv6 address\n"
       "indicate xt addr follows\n"
       "underlay network interface IPv4 address\n"
       "underlay network interface IPv6 address\n"
       "logical network ID follows\n"
       "logical network ID\n"
       "indicate target MAC addr follows\n"
       "target MAC addr\n")

DEFSH (VTYSH_ISISD, isis_priority_l1_cmd_vtysh, 
       "isis priority (0-127) level-1", 
       "IS-IS commands\n"
       "Set priority for Designated Router election\n"
       "Priority value\n"
       "Specify priority for level-1 routing\n")

DEFSH (VTYSH_OSPFD, debug_ospf_ism_cmd_vtysh, 
       "debug ospf [(1-65535)] ism [<status|events|timers>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Interface State Machine\n"
       "ISM Status Information\n"
       "ISM Event Information\n"
       "ISM TImer Information\n")

DEFSH (VTYSH_ISISD, no_area_lsp_mtu_cmd_vtysh, 
       "no lsp-mtu [(128-4352)]", 
       "Negate a command or set its defaults\n"
       "Configure the maximum size of generated LSPs\n"
       "Maximum size of generated LSPs\n")

DEFSH (VTYSH_ISISD, no_router_isis_cmd_vtysh, 
       "no router isis WORD", 
       "no\n" "Enable a routing process\n" "ISO IS-IS\n" "ISO Routing area tag")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_route_cmd_vtysh, 
       "show ipv6 ospf6 route [<intra-area|inter-area|external-1|external-2|X:X::X:X|X:X::X:X/M|detail|summary>]", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Routing Table\n"
       "Display Intra-Area routes\n"
       "Display Inter-Area routes\n"
       "Display Type-1 External routes\n"
       "Display Type-2 External routes\n"
       "Specify IPv6 address\n"
       "Specify IPv6 prefix\n"
       "Detailed information\n"
       "Summary of route table\n")

DEFSH (VTYSH_BGPD, no_bgp_bestpath_med_cmd_vtysh, 
       "no bgp bestpath med <confed [missing-as-worst]|missing-as-worst [confed]>", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Compare MED among confederation paths\n"
       "Treat missing MED as the least preferred one\n"
       "Treat missing MED as the least preferred one\n"
       "Compare MED among confederation paths\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_protocol_nht_rmap_cmd_vtysh, 
       "no ipv6 nht " "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|any>" " [route-map ROUTE-MAP]", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Filter Next Hop tracking route resolution\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_LDPD, ldp_no_label_remote_accept_for_acl_cmd_vtysh, 
       "no label remote accept for <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_BGPD, show_ip_lcommunity_list_cmd_vtysh, 
       "show ip large-community-list", 
       "Show running system information\n"
       "IP information\n"
       "List large-community list\n")

DEFSH (VTYSH_ISISD, isis_mpls_te_on_cmd_vtysh, 
       "mpls-te on", 
       "MPLS-TE specific commands\n"
       "Enable MPLS-TE functionality\n")

DEFSH (VTYSH_OSPF6D, no_area_range_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> range X:X::X:X/M [<advertise|not-advertise|cost (0-16777215)>]", 
       "Negate a command or set its defaults\n"
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "OSPF6 area ID as a decimal value\n"
       "Configured address range\n"
       "Specify IPv6 prefix\n"
       "Advertise\n"
       "Do not advertise\n"
       "User specified metric for this range\n"
       "Advertised metric for this range\n")

DEFSH (VTYSH_OSPFD, debug_ospf_te_cmd_vtysh, 
       "debug ospf te", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF-TE information\n")

DEFSH (VTYSH_BGPD, bgp_default_show_hostname_cmd_vtysh, 
       "bgp default show-hostname", 
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "Show hostname in certain command ouputs\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_flooding_cmd_vtysh, 
       "debug ospf6 flooding", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 flooding function\n"
      )

DEFSH (VTYSH_ISISD, no_isis_hello_interval_l2_cmd_vtysh, 
       "no isis hello-interval [(1-600)] level-2", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set Hello interval\n"
       "Holdtime 1 second,  interval depends on multiplier\n"
       "Specify hello-interval for level-2 IIHs\n")

DEFSH (VTYSH_PIMD, no_debug_pim_trace_cmd_vtysh, 
       "no debug pim trace", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM internal daemon activity\n")

DEFSH (VTYSH_BGPD, no_ip_as_path_all_cmd_vtysh, 
       "no ip as-path access-list WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "BGP autonomous system path filter\n"
       "Specify an access list name\n"
       "Regular expression access list name\n")

DEFSH (VTYSH_BGPD, vnc_vrf_policy_rd_cmd_vtysh, 
       "rd ASN:nn_or_IP-address:nn", 
       "Specify default VRF route distinguisher\n"
       "Route Distinguisher (<as-number>:<number> | <ip-address>:<number> | auto:nh:<number> )\n")

DEFSH (VTYSH_ALL, no_access_list_standard_cmd_vtysh, 
       "no access-list <(1-99)|(1300-1999)> <deny|permit> A.B.C.D A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n"
       "Wildcard bits\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_mtu_cmd_vtysh, 
 "no ip nhrp mtu [(576-1500)|opennhrp]", 
 "Negate a command or set its defaults\n"
 "IP information\n"
 "Next Hop Resolution Protocol functions\n"
 "Configure NHRP advertised MTU\n"
 "MTU value\n"
 "Advertise bound interface MTU similar to OpenNHRP")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_seq_cmd_vtysh, 
       "no ip prefix-list WORD seq (1-4294967295) <deny|permit> <A.B.C.D/M|any>", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Any prefix match.  Same as \"0.0.0.0/0 le 32\"\n")

DEFSH (VTYSH_BGPD, ip_lcommunity_list_name_expanded_cmd_vtysh, 
       "ip large-community-list expanded WORD <deny|permit> LINE...", 
       "IP information\n"
       "Add a large community list entry\n"
       "Specify expanded large-community-list\n"
       "Large Community list name\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "An ordered list as a regular-expression\n")

DEFSH (VTYSH_BGPD, no_bgp_confederation_identifier_cmd_vtysh, 
       "no bgp confederation identifier [(1-4294967295)]", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "AS number\n"
       "Set routing domain confederation AS\n")

DEFSH (VTYSH_BGPD, no_ip_lcommunity_list_name_expanded_cmd_vtysh, 
       "no ip large-community-list expanded WORD <deny|permit> LINE...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a large community list entry\n"
       "Specify expanded large-community-list\n"
       "Large community list name\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "An ordered list as a regular-expression\n")

DEFSH (VTYSH_ISISD, no_isis_priority_l1_cmd_vtysh, 
       "no isis priority [(0-127)] level-1", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set priority for Designated Router election\n"
       "Priority value\n"
       "Specify priority for level-1 routing\n")

DEFSH_HIDDEN (VTYSH_PIMD, interface_ip_igmp_query_max_response_time_dsec_cmd_vtysh, 
       "ip igmp query-max-response-time-dsec (10-250)", 
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP max query response value (deciseconds)\n"
       "Query response value in deciseconds\n")

DEFSH (VTYSH_ISISD, no_psnp_interval_l2_cmd_vtysh, 
       "no isis psnp-interval [(1-120)] level-2", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set PSNP interval in seconds\n"
       "PSNP interval value\n"
       "Specify interval for level-2 PSNPs\n")

DEFSH (VTYSH_RIPD, no_rip_allow_ecmp_cmd_vtysh, 
       "no allow-ecmp", 
       "Negate a command or set its defaults\n"
       "Allow Equal Cost MultiPath\n")

DEFSH (VTYSH_BGPD, no_neighbor_attr_unchanged_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> attribute-unchanged       [<          as-path [<next-hop [med]|med [next-hop]>]|          next-hop [<as-path [med]|med [as-path]>]|          med [<as-path [next-hop]|next-hop [as-path]>]       >]", 





       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP attribute is propagated unchanged to this neighbor\n"
       "As-path attribute\n"
       "Nexthop attribute\n"
       "Med attribute\n"
       "Med attribute\n"
       "Nexthop attribute\n"
       "Nexthop attribute\n"
       "As-path attribute\n"
       "Med attribute\n"
       "Med attribute\n"
       "As-path attribute\n"
       "Med attribute\n"
       "As-path attribute\n"
       "Nexthop attribute\n"
       "Nexthop attribute\n"
       "As-path attribute\n")

DEFSH (VTYSH_OSPFD, debug_ospf_instance_nssa_cmd_vtysh, 
       "debug ospf (1-65535) nssa", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF nssa information\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_neighbor_capabilities_json_cmd_vtysh, 
       "show mpls ldp neighbor capabilities [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Neighbor information\n"
       "Display neighbor capability information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD, accept_lifetime_month_day_day_month_cmd_vtysh, 
       "accept-lifetime HH:MM:SS MONTH (1-31) (1993-2035) HH:MM:SS (1-31) MONTH (1993-2035)", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Day of th month to expire\n"
       "Month of the year to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_ISISD, debug_isis_spfevents_cmd_vtysh, 
       "debug isis spf-events", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS Shortest Path First Events\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_peer_groups_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] peer-group [PGNAME]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Detailed information on BGP peer groups\n"
       "Peer group name\n")

DEFSH (VTYSH_ALL, show_ipv6_access_list_cmd_vtysh, 
       "show ipv6 access-list", 
       "Show running system information\n"
       "IPv6 information\n"
       "List IPv6 access lists\n")

DEFSH (VTYSH_BGPD, no_neighbor_shutdown_cmd_vtysh, 
      "no neighbor <A.B.C.D|X:X::X:X|WORD> shutdown", 
      "Negate a command or set its defaults\n" "Specify neighbor router\n" "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
      "Administratively shut down this neighbor\n")

DEFSH (VTYSH_OSPF6D, auto_cost_reference_bandwidth_cmd_vtysh, 
       "auto-cost reference-bandwidth (1-4294967)", 
       "Calculate OSPF interface cost according to bandwidth\n"
       "Use reference bandwidth method to assign OSPF cost\n"
       "The reference bandwidth in terms of Mbits per second\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_cmd_vtysh, 
       "label local advertise", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n")

DEFSH (VTYSH_LDPD, ldp_no_label_remote_accept_cmd_vtysh, 
       "no label remote accept", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv6_metric_rmap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct>" " metric (0-4294967295) route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Metric for redistributed routes\n"
       "Default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, no_neighbor_capability_orf_prefix_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> capability orf prefix-list <both|send|receive>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Advertise capability to the peer\n"
       "Advertise ORF capability to the peer\n"
       "Advertise prefixlist ORF capability to this neighbor\n"
       "Capability to SEND and RECEIVE the ORF to/from this neighbor\n"
       "Capability to RECEIVE the ORF from this neighbor\n"
       "Capability to SEND the ORF to this neighbor\n")

DEFSH (VTYSH_BGPD, no_ipv6_bgp_distance_source_cmd_vtysh, 
       "no distance (1-255) X:X::X:X/M", 
       "Negate a command or set its defaults\n"
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_ifname_pref_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, no_set_community_cmd_vtysh, 
       "no set community AA:NN...", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP community attribute\n"
       "Community number in AA:NN format (where AA and NN are (0-65535)) or local-AS|no-advertise|no-export|internet or additive\n")

DEFSH (VTYSH_ISISD, isis_metric_cmd_vtysh, 
       "isis metric (0-16777215)", 
       "IS-IS commands\n"
       "Set default metric for circuit\n"
       "Default metric value\n")

DEFSH (VTYSH_OSPF6D, ospf6_distance_cmd_vtysh, 
       "distance (1-255)", 
       "Administrative distance\n"
       "OSPF6 Administrative distance\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n"
      )

DEFSH (VTYSH_OSPFD, debug_ospf_event_cmd_vtysh, 
       "debug ospf event", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF event information\n")

DEFSH (VTYSH_BGPD, no_ip_lcommunity_list_standard_all_cmd_vtysh, 
       "no ip large-community-list <(1-99)|(100-500)|WORD>", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a large community list entry\n"
       "Large Community list number (standard)\n"
       "Large Community list number (expanded)\n"
       "Large Community list name\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_flooding_cmd_vtysh, 
       "no debug ospf6 flooding", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 flooding function\n"
      )

DEFSH (VTYSH_LDPD, ldp_label_remote_accept_cmd_vtysh, 
       "label remote accept", 
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n")

DEFSH (VTYSH_BGPD, no_neighbor_local_as_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> local-as [(1-4294967295) [no-prepend [replace-as]]]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Specify a local-as number\n"
       "AS number used as local AS\n"
       "Do not prepend local-as to updates from ebgp peers\n"
       "Do not prepend local-as to updates from ibgp peers\n")

DEFSH (VTYSH_ZEBRA, show_vrf_cmd_vtysh, 
       "show vrf", 
       "Show running system information\n"
       "VRF\n")

DEFSH (VTYSH_LDPD, ldp_no_vc_type_pwtype_cmd_vtysh, 
       "no vc type <ethernet|ethernet-tagged>", 
       "Negate a command or set its defaults\n"
       "Virtual Circuit options\n"
       "Virtual Circuit type to use\n"
       "Ethernet (type 5)\n"
       "Ethernet-tagged (type 4)\n")

DEFSH (VTYSH_BGPD, vnc_clear_counters_cmd_vtysh, 
       "clear vnc counters", 
       "Reset functions\n"
       "VNC information\n"
       "Reset VNC counters\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_interface_json_cmd_vtysh, 
       "show mpls ldp interface [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "interface information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_prefix_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M>", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n" "IPv6 prefix\n")

DEFSH (VTYSH_OSPFD, ospf_rfc1583_flag_cmd_vtysh, 
      "ospf rfc1583compatibility", 
      "OSPF specific commands\n"
      "Enable the RFC1583Compatibility flag\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_nexthop_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] nexthop [detail]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "BGP nexthop table\n"
       "Show detailed information\n")

DEFSH (VTYSH_BGPD, no_bgp_deterministic_med_cmd_vtysh, 
       "no bgp deterministic-med", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Pick the best-MED path among paths advertised from the neighboring AS\n")

DEFSH (VTYSH_PIMD, debug_igmp_trace_cmd_vtysh, 
       "debug igmp trace", 
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n"
       "IGMP internal daemon activity\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_sequence_number_cmd_vtysh, 
       "ipv6 prefix-list sequence-number", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Include/exclude sequence numbers in NVGEN\n")

DEFSH (VTYSH_BGPD, vnc_vrf_policy_rt_export_cmd_vtysh, 
       "rt export RTLIST...", 
       "Specify route targets\n"
       "Export filter\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_ifname_pref_tag_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE tag (1-4294967295) (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, bgp_network_mask_natural_cmd_vtysh, 
       "network A.B.C.D", 
       "Specify a network to announce via BGP\n"
       "Network number\n")

DEFSH (VTYSH_BGPD, neighbor_soft_reconfiguration_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> soft-reconfiguration inbound", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Per neighbor soft reconfiguration\n"
       "Allow inbound soft reconfiguration for this neighbor\n")

DEFSH (VTYSH_BGPD, show_bgp_views_cmd_vtysh, 
       "show [ip] bgp views", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Show the defined BGP views\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_adv_interval_config_option_cmd_vtysh, 
       "no ipv6 nd adv-interval-option", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Advertisement Interval Option\n")

DEFSH (VTYSH_RIPD, ip_rip_authentication_key_chain_cmd_vtysh, 
       "ip rip authentication key-chain LINE", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Authentication control\n"
       "Authentication key-chain\n"
       "name of key-chain\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_ipv4_ttl_security_disable_cmd_vtysh, 
       "no neighbor A.B.C.D ttl-security disable", 
       "Negate a command or set its defaults\n"
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "LDP ttl security check\n"
       "Disable ttl security\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_database_type_adv_router_cmd_vtysh, 
       "show ip ospf database <asbr-summary|external|network|router|summary|nssa-external|opaque-link|opaque-area|opaque-as> <adv-router A.B.C.D|self-originate>", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n" "External link states\n" "Network link states\n" "Router link states\n" "Network summary link states\n" "NSSA external link state\n" "Link local Opaque-LSA\n" "Link area Opaque-LSA\n" "Link AS Opaque-LSA\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n"
       "Self-originated link states\n")

DEFSH (VTYSH_ALL, no_access_list_extended_mask_any_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip A.B.C.D A.B.C.D any", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Any destination host\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_area_spf_tree_cmd_vtysh, 
       "show ipv6 ospf6 area A.B.C.D spf tree", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Area information\n"
       "Area ID (as an IPv4 notation)\n"
       "Shortest Path First caculation\n"
       "Show SPF tree\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_mask_tag_distance_label_cmd_vtysh, 
       "no ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> tag (1-4294967295) (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Tag of this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPF6D, show_zebra_cmd_vtysh, 
       "show zebra", 
       "Show running system information\n"
       "Zebra information\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_neighbor_cmd_vtysh, 
       "debug ospf6 neighbor [<state|event>]", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 Neighbor\n"
       "Debug OSPFv3 Neighbor State Change\n"
       "Debug OSPFv3 Neighbor Event\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_cmd_vtysh, 
       "ipv6 route X:X::X:X/M [from X:X::X:X/M] <X:X::X:X|INTERFACE|null0> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, no_bgp_graceful_restart_cmd_vtysh, 
       "no bgp graceful-restart", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Graceful restart capability parameters\n")

DEFSH (VTYSH_RIPD, rip_offset_list_ifname_cmd_vtysh, 
       "offset-list WORD <in|out> (0-16) IFNAME", 
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n"
       "Interface to match\n")

DEFSH (VTYSH_ZEBRA, ip_mroute_dist_cmd_vtysh, 
       "ip mroute A.B.C.D/M <A.B.C.D|INTERFACE> [(1-255)]", 
       "IP information\n"
       "Configure static unicast route into MRIB for multicast RPF lookup\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Nexthop address\n"
       "Nexthop interface name\n"
       "Distance\n")

DEFSH (VTYSH_ISISD, no_debug_isis_rtevents_cmd_vtysh, 
       "no debug isis route-events", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS Route related events\n")

DEFSH (VTYSH_ISISD, no_set_overload_bit_cmd_vtysh, 
       "no set-overload-bit", 
       "Reset overload bit to accept transit traffic\n"
       "Reset overload bit\n")

DEFSH_HIDDEN (VTYSH_BGPD, no_neighbor_bfd_type_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> bfd <multihop|singlehop>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Disables BFD support\n"
       "Multihop session\n"
       "Singlehop session\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_cmd_vtysh, 
       "no debug ospf", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n")

DEFSH (VTYSH_PIMD, show_ip_pim_upstream_cmd_vtysh, 
       "show ip pim upstream [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM upstream information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, no_ospf_refresh_timer_val_cmd_vtysh, 
       "no refresh timer [(10-1800)]", 
       "Negate a command or set its defaults\n"
       "Adjust refresh parameters\n"
       "Unset refresh timer\n"
       "Timer value in seconds\n")

DEFSH (VTYSH_BGPD, no_neighbor_interface_peer_group_remote_as_cmd_vtysh, 
       "no neighbor WORD remote-as <(1-4294967295)|internal|external>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Interface name or neighbor tag\n"
       "Specify a BGP neighbor\n"
       "AS number\n"
       "Internal BGP peer\n"
       "External BGP peer\n")

DEFSH (VTYSH_BGPD, no_neighbor_send_community_type_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> send-community <both|all|extended|standard|large>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Send Community attribute to this neighbor\n"
       "Send Standard and Extended Community attributes\n"
       "Send Standard,  Large and Extended Community attributes\n"
       "Send Extended Community attributes\n"
       "Send Standard Community attributes\n"
       "Send Large Community attributes\n")

DEFSH (VTYSH_LDPD, ldp_member_interface_ifname_cmd_vtysh, 
       "member interface IFNAME", 
       "L2VPN member configuration\n"
       "Local interface\n"
       "Interface's name\n")

DEFSH (VTYSH_PIMD, show_ip_pim_assert_metric_cmd_vtysh, 
       "show ip pim assert-metric", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface assert metric\n")

DEFSH (VTYSH_BGPD, neighbor_route_server_client_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> route-server-client", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Configure a neighbor as Route Server client\n")

DEFSH (VTYSH_BGPD, set_origin_cmd_vtysh, 
       "set origin <egp|igp|incomplete>", 
       "Set values in destination routing protocol\n"
       "BGP origin code\n"
       "remote EGP\n"
       "local IGP\n"
       "unknown heritage\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_vpn_rd_cmd_vtysh, 
       "show [ip] bgp ""<ipv4|ipv6>"" vpn rd ASN:nn_or_IP-address:nn", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n"
       "Display information for a route distinguisher\n"
       "VPN Route Distinguisher\n")

DEFSH (VTYSH_OSPFD, ip_ospf_network_cmd_vtysh, 
       "ip ospf network <broadcast|non-broadcast|point-to-multipoint|point-to-point>", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Network type\n"
       "Specify OSPF broadcast multi-access network\n"
       "Specify OSPF NBMA network\n"
       "Specify OSPF point-to-multipoint network\n"
       "Specify OSPF point-to-point network\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_transmitdelay_cmd_vtysh, 
       "ipv6 ospf6 transmit-delay (1-3600)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Link state transmit delay\n"
       "Seconds\n")

DEFSH (VTYSH_ALL, access_list_extended_mask_host_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip A.B.C.D A.B.C.D host A.B.C.D", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "A single destination host\n"
       "Destination address\n")

DEFSH (VTYSH_OSPFD, no_ospf_rfc1583_flag_cmd_vtysh, 
      "no ospf rfc1583compatibility",  "Negate a command or set its defaults\n"
      "OSPF specific commands\n"
      "Disable the RFC1583Compatibility flag\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_for_acl_to_acl_cmd_vtysh, 
       "label local advertise for <(1-199)|(1300-2699)|WORD> to <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "IP Access-list specifying controls on LDP Peers\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_BGPD, no_neighbor_addpath_tx_all_paths_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> addpath-tx-all-paths", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Use addpath to advertise all paths to a neighbor\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_neighbor_cmd_vtysh, 
       "show ipv6 ospf6 neighbor [<detail|drchoice>]", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Neighbor list\n"
       "Display details\n"
       "Display DR choices\n")

DEFSH (VTYSH_LDPD, ldp_no_member_pseudowire_ifname_cmd_vtysh, 
       "no member pseudowire IFNAME", 
       "Negate a command or set its defaults\n"
       "L2VPN member configuration\n"
       "Pseudowire interface\n"
       "Interface's name\n")

DEFSH (VTYSH_BGPD, vnc_show_nves_ptct_cmd_vtysh, 
       "show vnc nves <vn|un> <A.B.C.D|X:X::X:X>", 
       "Show running system information\n"
       "VNC information\n"
       "List known NVEs\n"
       "VN address of NVE\n"
       "UN address of NVE\n"
       "IPv4 interface address\n"
       "IPv6 interface address\n")

DEFSH (VTYSH_ISISD, show_isis_mpls_te_interface_cmd_vtysh, 
       "show isis mpls-te interface [INTERFACE]", 
       "Show running system information\n"
       "IS-IS information\n"
       "MPLS-TE specific commands\n"
       "Interface information\n"
       "Interface name\n")

DEFSH (VTYSH_BGPD, neighbor_bfd_param_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> bfd (2-255) (50-60000) (50-60000)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Enables BFD support\n"
       "Detect Multiplier\n"
       "Required min receive interval\n"
       "Desired min transmit interval\n")

DEFSH (VTYSH_OSPFD, debug_ospf_instance_lsa_cmd_vtysh, 
       "debug ospf (1-65535) lsa [<generate|flooding|install|refresh>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Link State Advertisement\n"
       "LSA Generation\n"
       "LSA Flooding\n"
       "LSA Install/Delete\n"
       "LSA Refresh\n")

DEFSH (VTYSH_BGPD, debug_rfapi_unregister_vn_un_cmd_vtysh, 
       "debug rfapi-dev unregister vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_register\n"
       "indicate vn addr follows\n"
       "virtual network interface address\n"
       "indicate xt addr follows\n"
       "underlay network interface address\n"
       "indicate prefix follows\n" "prefix")

DEFSH (VTYSH_OSPFD, ospf_log_adjacency_changes_cmd_vtysh, 
       "log-adjacency-changes", 
       "Log changes in adjacency state\n")

DEFSH (VTYSH_BGPD, bgp_rr_allow_outbound_policy_cmd_vtysh, 
       "bgp route-reflector allow-outbound-policy", 
       "BGP specific commands\n"
       "Allow modifications made by out route-map\n"
       "on ibgp neighbors\n")

DEFSH (VTYSH_BGPD, no_set_ecommunity_rt_cmd_vtysh, 
       "no set extcommunity rt ASN:nn_or_IP-address:nn...", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP extended community attribute\n"
       "Route Target extended community\n"
       "VPN extended community\n")

DEFSH (VTYSH_ISISD, no_isis_metric_l1_cmd_vtysh, 
       "no isis metric [(0-16777215)] level-1", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set default metric for circuit\n"
       "Default metric value\n"
       "Specify metric for level-1 routing\n")

DEFSH (VTYSH_OSPFD, no_ospf_timers_throttle_spf_cmd_vtysh, 
       "no timers throttle spf [(0-600000)(0-600000)(0-600000)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "Throttling adaptive timer\n"
       "OSPF SPF timers\n"
       "Delay (msec) from first change received till SPF calculation\n"
       "Initial hold time (msec) between consecutive SPF calculations\n"
       "Maximum hold time (msec)\n")

DEFSH (VTYSH_ISISD, isis_default_originate_cmd_vtysh, 
       "default-information originate <ipv4|ipv6> <level-1|level-2> [always] [<metric (0-16777215)|route-map WORD>]", 
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Distribute default route for IPv4\n"
       "Distribute default route for IPv6\n"
       "Distribute default route into level-1\n"
       "Distribute default route into level-2\n"
       "Always advertise default route\n"
       "Metric for default route\n"
       "ISIS default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_PIMD, debug_pim_events_cmd_vtysh, 
       "debug pim events", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM protocol events\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_advertise_prefix_list_cmd_vtysh, 
       "no ipv6 ospf6 advertise prefix-list", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Advertising options\n"
       "Filter prefix using prefix-list\n"
       )

DEFSH (VTYSH_BGPD, show_ip_lcommunity_list_arg_cmd_vtysh, 
       "show ip large-community-list <(1-500)|WORD>", 
       "Show running system information\n"
       "IP information\n"
       "List large-community list\n"
       "large-community-list number\n"
       "large-community-list name\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_shortcut_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> shortcut <enable|disable>", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Deconfigure the area's shortcutting mode\n"
       "Deconfigure enabled shortcutting through the area\n"
       "Deconfigure disabled shortcutting through the area\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_route_type_detail_cmd_vtysh, 
       "show ipv6 ospf6 route <intra-area|inter-area|external-1|external-2> detail", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Routing Table\n"
       "Display Intra-Area routes\n"
       "Display Inter-Area routes\n"
       "Display Type-1 External routes\n"
       "Display Type-2 External routes\n"
       "Detailed information\n"
       )

DEFSH (VTYSH_BGPD, set_community_none_cmd_vtysh, 
       "set community none", 
       "Set values in destination routing protocol\n"
       "BGP community attribute\n"
       "No community attribute\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_default_cost_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> default-cost (0-16777215)", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Set the summary-default cost of a NSSA or stub area\n"
       "Stub's advertised default summary cost\n")

DEFSH (VTYSH_ISISD, no_isis_hello_interval_cmd_vtysh, 
       "no isis hello-interval [(1-600)]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set Hello interval\n"
       "Holdtime 1 second,  interval depends on multiplier\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_messages_sent_all_cmd_vtysh, 
       "debug mpls ldp messages sent all", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Sent messages,  excluding periodic Keep Alives\n"
       "Sent messages,  including periodic Keep Alives\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_rib_detailed_cmd_vtysh, 
       "debug zebra rib detailed", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug RIB events\n"
       "Detailed debugs\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_seq_le_ge_cmd_vtysh, 
       "no ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M le (0-32) ge (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_rd_neighbor_routes_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn rd ASN:nn_or_IP-address:nn neighbors A.B.C.D routes [json]", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information for a route distinguisher\n"
      "VPN Route Distinguisher\n"
      "Detailed information on TCP and BGP neighbor connections\n"
      "Neighbor to display information about\n"
      "Display routes learned from neighbor\n" "JavaScript Object Notation\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_targeted_hello_accept_from_acl_cmd_vtysh, 
       "no discovery targeted-hello accept from <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Accept and respond to targeted hellos\n"
       "Access list to specify acceptable targeted hello source\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_OSPFD, ospf_timers_lsa_cmd_vtysh, 
       "timers lsa min-arrival (0-600000)", 
       "Adjust routing timers\n"
       "OSPF LSA timers\n"
       "Minimum delay in receiving new version of a LSA\n"
       "Delay in milliseconds\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_mask_tag_label_cmd_vtysh, 
       "no ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> tag (1-4294967295) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Tag of this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ZEBRA, ip_route_mask_tag_label_cmd_vtysh, 
       "ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> tag (1-4294967295) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPFD, ospf_default_metric_cmd_vtysh, 
       "default-metric (0-16777214)", 
       "Set metric of redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_RIPNGD, ripng_route_cmd_vtysh, 
       "route IPV6ADDR", 
       "Static route setup\n"
       "Set static RIPng route announcement\n")

DEFSH (VTYSH_NHRPD, no_nhrp_event_socket_cmd_vtysh, 
 "no nhrp event socket [SOCKET]", 
 "Negate a command or set its defaults\n"
 "Next Hop Resolution Protocol functions\n"
 "Event Manager commands\n"
 "Event Manager unix socket path\n"
 "Unix path for the socket")

DEFSH (VTYSH_BGPD, no_ip_lcommunity_list_name_expanded_all_cmd_vtysh, 
       "no ip large-community-list expanded WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a large community list entry\n"
       "Specify expanded large-community-list\n"
       "Large Community list name\n")

DEFSH (VTYSH_ISISD, lsp_refresh_interval_cmd_vtysh, 
       "lsp-refresh-interval [<level-1|level-2>] (1-65235)", 
       "LSP refresh interval\n"
       "LSP refresh interval for Level 1 only\n"
       "LSP refresh interval for Level 2 only\n"
       "LSP refresh interval in seconds\n")

DEFSH (VTYSH_OSPFD, ospf_max_metric_router_lsa_shutdown_cmd_vtysh, 
       "max-metric router-lsa on-shutdown (5-100)", 
       "OSPF maximum / infinite-distance metric\n"
       "Advertise own Router-LSA with infinite distance (stub router)\n"
       "Advertise stub-router prior to full shutdown of OSPF\n"
       "Time (seconds) to wait till full shutdown\n")

DEFSH (VTYSH_ZEBRA, show_interface_vrf_all_cmd_vtysh, 
       "show interface vrf all", 
       "Show running system information\n"
       "Interface status and configuration\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_ifname_flags_cmd_vtysh, 
       "ipv6 route X:X::X:X/M [from X:X::X:X/M] X:X::X:X INTERFACE <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, bgp_distance_source_cmd_vtysh, 
       "distance (1-255) A.B.C.D/M", 
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_life_lnh_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> lifetime (1-4294967295) LNH_OPTIONS...", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_OSPFD, no_ospf_max_metric_router_lsa_shutdown_cmd_vtysh, 
       "no max-metric router-lsa on-shutdown [(5-100)]", 
       "Negate a command or set its defaults\n"
       "OSPF maximum / infinite-distance metric\n"
       "Advertise own Router-LSA with infinite distance (stub router)\n"
       "Advertise stub-router prior to full shutdown of OSPF\n"
       "Time (seconds) to wait till full shutdown\n")

DEFSH (VTYSH_ZEBRA, zebra_ptm_enable_if_cmd_vtysh, 
       "ptm-enable", 
       "Enable neighbor check with specified topology\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_debug_packet_cmd_vtysh, 
       "ip irdp debug packet", 
       "IP information\n"
       "ICMP Router discovery debug Averts. and Solicits (short)\n"
       "IRDP debugging options\n"
       "Enable debugging for IRDP packets\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_ifname_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M [from X:X::X:X/M] X:X::X:X INTERFACE [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ALL, no_access_list_extended_any_host_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip any host A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "A single destination host\n"
       "Destination address\n")

DEFSH (VTYSH_BGPD, bgp_table_map_cmd_vtysh, 
       "table-map WORD", 
       "BGP table to RIB route download filter\n"
       "Name of the route map\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv6_rmap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct>" " route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_PIMD, interface_no_ip_pim_drprio_cmd_vtysh, 
       "no ip pim drpriority [(1-4294967295)]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "PIM information\n"
       "Revert the Designated Router Priority to default\n"
       "Old Value of the Priority\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_reg_flags_cmd_vtysh, 
 "no " "<ip|ipv6>" " nhrp registration no-unique", 
 "Negate a command or set its defaults\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Registration configuration\n"
 "Don't set unique flag\n")

DEFSH (VTYSH_BGPD, no_bgp_confederation_peers_cmd_vtysh, 
       "no bgp confederation peers (1-4294967295)...", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "Peer ASs in BGP confederation\n"
       "AS number\n")

DEFSH (VTYSH_BGPD, neighbor_advertise_interval_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> advertisement-interval (0-600)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Minimum interval between sending BGP routing updates\n"
       "time in seconds\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_neighbor_detail_json_cmd_vtysh, 
       "show mpls ldp neighbor detail [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Neighbor information\n"
       "Show detailed information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, bgp_disable_connected_route_check_cmd_vtysh, 
       "bgp disable-ebgp-connected-route-check", 
       "BGP specific commands\n"
       "Disable checking if nexthop is connected on ebgp sessions\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_log_adjacency_changes_detail_cmd_vtysh, 
       "no log-adjacency-changes detail", 
       "Negate a command or set its defaults\n"
       "Log changes in adjacency state\n"
       "Log all state changes\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_timers_lsa_cmd_vtysh, 
       "no timers lsa min-arrival [(0-600000)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "OSPF6 LSA timers\n"
       "Minimum delay in receiving new version of a LSA\n"
       "Delay in milliseconds\n")

DEFSH (VTYSH_ALL, access_list_any_cmd_vtysh, 
       "access-list WORD <deny|permit> any", 
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n")

DEFSH (VTYSH_PIMD, no_debug_mroute_cmd_vtysh, 
       "no debug mroute", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM interaction with kernel MFC cache\n")

DEFSH (VTYSH_BGPD, no_bgp_default_ipv4_unicast_cmd_vtysh, 
       "no bgp default ipv4-unicast", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "Activate ipv4-unicast for a peer by default\n")

DEFSH (VTYSH_ZEBRA, match_ip_address_prefix_len_cmd_vtysh, 
       "match ip address prefix-len (0-32)", 
       "Match values from routing table\n"
       "IP information\n"
       "Match prefix length of ip address\n"
       "Match prefix length of ip address\n"
       "Prefix length\n")

DEFSH (VTYSH_BGPD, no_match_ip_route_source_prefix_list_cmd_vtysh, 
       "no match ip route-source prefix-list [WORD]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match advertising source address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_ZEBRA, ip_nht_default_route_cmd_vtysh, 
       "ip nht resolve-via-default", 
       "IP information\n"
       "Filter Next Hop tracking route resolution\n"
       "Resolve via default route\n")

DEFSH (VTYSH_ZEBRA, show_interface_cmd_vtysh, 
       "show interface [vrf NAME]", 
       "Show running system information\n"
       "Interface status and configuration\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_ISISD, no_debug_isis_spftrigg_cmd_vtysh, 
       "no debug isis spf-triggers", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS SPF triggering events\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_cost_life_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> cost (0-255) lifetime (1-4294967295)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_BGPD, no_neighbor_cmd_vtysh, 
       "no neighbor <WORD|<A.B.C.D|X:X::X:X> [remote-as <(1-4294967295)|internal|external>]>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Specify a BGP neighbor\n"
       "AS number\n"
       "Internal BGP peer\n"
       "External BGP peer\n")

DEFSH (VTYSH_OSPF6D, ospf6_redistribute_routemap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|isis|bgp|nhrp|table|vnc>" " route-map WORD", 
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Route map reference\n"
       "Route map name\n")

DEFSH (VTYSH_ZEBRA, ip_route_mask_distance_label_cmd_vtysh, 
       "ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_LDPD, ldp_label_remote_accept_from_acl_for_acl_cmd_vtysh, 
       "label remote accept from <(1-199)|(1300-2699)|WORD> for <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "Neighbor from whom to accept label advertisement\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_interface_ifname_cmd_vtysh, 
       "show ipv6 ospf6 interface [IFNAME]", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface infomation\n"
       "Interface name(e.g. ep0)\n")

DEFSH (VTYSH_BGPD, vnc_show_registrations_some_pfx_cmd_vtysh, 
         "show vnc registrations <all|holddown|imported|local|remote> [<A.B.C.D/M|X:X::X:X/M|YY:YY:YY:YY:YY:YY>]", 
         "Show running system information\n"
         "VNC information\n"
         "List active prefix registrations\n"
         "show all registrations\n"
         "show only registrations in holddown\n"
         "show only imported prefixes\n"
         "show only local registrations\n"
         "show only remote registrations\n"
         "Limit output to a particular prefix or address\n"
         "Limit output to a particular prefix or address\n"
         "Limit output to a particular prefix or address\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_mtu_cmd_vtysh, 
       "no ipv6 nd mtu [(1-65535)]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Advertised MTU\n"
       "MTU in bytes\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_ge_le_cmd_vtysh, 
       "no ipv6 prefix-list WORD <deny|permit> X:X::X:X/M ge (0-128) le (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_ZEBRA, no_router_id_cmd_vtysh, 
       "no router-id [A.B.C.D [vrf NAME]]", 
       "Negate a command or set its defaults\n"
       "Remove the manually configured router-id\n"
       "IP address to use for router-id\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_ISISD, isis_hello_padding_cmd_vtysh, 
       "isis hello padding", 
       "IS-IS commands\n"
       "Add padding to IS-IS hello packets\n"
       "Pad hello packets\n"
       "<cr>\n")

DEFSH (VTYSH_BGPD, no_match_ip_route_source_cmd_vtysh, 
       "no match ip route-source [<(1-199)|(1300-2699)|WORD>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match advertising source address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP standard access-list name\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_range_substitute_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> range A.B.C.D/M substitute A.B.C.D/M", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "Area range prefix\n"
       "Announce area range as another prefix\n"
       "Network prefix to be announced instead of range\n")

DEFSH (VTYSH_BGPD, no_bgp_redistribute_ipv4_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|rip|ospf|isis|pim|nhrp|table|vnc|vnc-direct>" " [metric (0-4294967295)] [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Metric for redistributed routes\n"
       "Default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_PIMD, interface_no_pim_use_source_cmd_vtysh, 
       "no ip pim use-source", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Delete source IP address\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_sequence_number_cmd_vtysh, 
       "no ipv6 prefix-list sequence-number", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Include/exclude sequence numbers in NVGEN\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_name_seq_cmd_vtysh, 
       "show ip prefix-list WORD seq (1-4294967295)", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_ipv6_address_cmd_vtysh, 
       "no match ipv6 address WORD", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IPv6 information\n"
       "Match IPv6 address of route\n"
       "IPv6 access-list name\n")

DEFSH (VTYSH_PIMD, show_ip_mroute_cmd_vtysh, 
       "show ip mroute [json]", 
       "Show running system information\n"
       "IP information\n"
       "IP multicast routing table\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_border_routers_cmd_vtysh, 
       "show ip ospf (1-65535) border-routers", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Show all the ABR's and ASBR's\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_ge_cmd_vtysh, 
       "no ip prefix-list WORD <deny|permit> A.B.C.D/M ge (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_OSPF6D, show_version_ospf6_cmd_vtysh, 
       "show version ospf6", 
       "Show running system information\n"
       "Display version\n"
       "Display ospf6d version\n"
      )

DEFSH (VTYSH_ZEBRA, show_ipv6_route_addr_cmd_vtysh, 
       "show ipv6 route [vrf NAME] X:X::X:X", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "IPv6 Address\n")

DEFSH (VTYSH_PIMD, clear_ip_pim_oil_cmd_vtysh, 
       "clear ip pim oil", 
       "Reset functions\n"
       "IP information\n"
       "PIM clear commands\n"
       "Rescan PIM OIL (output interface list)\n")

DEFSH (VTYSH_BGPD, show_bgp_instance_all_ipv6_updgrps_cmd_vtysh, 
       "show [ip] bgp <view|vrf> all update-groups", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nAll Views/VRFs\n"
       "Detailed info about dynamic update groups\n")

DEFSH (VTYSH_ALL, no_ipv6_access_list_remark_cmd_vtysh, 
       "no ipv6 access-list WORD remark", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Access list entry comment\n")

DEFSH (VTYSH_ISISD, show_isis_neighbor_cmd_vtysh, 
       "show isis neighbor", 
       "Show running system information\n"
       "ISIS network information\n"
       "ISIS neighbor adjacencies\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_nht_cmd_vtysh, 
       "debug zebra nht", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra next hop tracking\n")

DEFSH (VTYSH_BGPD, no_bgp_router_id_cmd_vtysh, 
       "no bgp router-id [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Override configured router identifier\n"
       "Manually configured router identifier\n")

DEFSH (VTYSH_ZEBRA, show_mpls_table_lsp_cmd_vtysh, 
       "show mpls table (16-1048575) [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "MPLS table\n"
       "LSP to display information about\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_network_cmd_vtysh, 
       "no ipv6 ospf6 network", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Network type\n"
       "Default to whatever interface type system specifies"
       )

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_addr_cmd_vtysh, 
       "show ip route vrf all A.B.C.D", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Network in the IP routing table to display\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_cmd_vtysh, 
       "show ipv6 route vrf all", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_BGPD, neighbor_attr_unchanged_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> attribute-unchanged       [<          as-path [<next-hop [med]|med [next-hop]>]|          next-hop [<as-path [med]|med [as-path]>]|          med [<as-path [next-hop]|next-hop [as-path]>]       >]", 





       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP attribute is propagated unchanged to this neighbor\n"
       "As-path attribute\n"
       "Nexthop attribute\n"
       "Med attribute\n"
       "Med attribute\n"
       "Nexthop attribute\n"
       "Nexthop attribute\n"
       "As-path attribute\n"
       "Med attribute\n"
       "Med attribute\n"
       "As-path attribute\n"
       "Med attribute\n"
       "As-path attribute\n"
       "Nexthop attribute\n"
       "Nexthop attribute\n"
       "As-path attribute\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_ipv4_session_holdtime_session_time_cmd_vtysh, 
       "neighbor A.B.C.D session holdtime (15-65535)", 
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "Configure session parameters\n"
       "Configure session holdtime\n"
       "Time (seconds)\n")

DEFSH (VTYSH_NHRPD, if_nhrp_map_cmd_vtysh, 
 "<ip|ipv6>" " nhrp map <A.B.C.D|X:X::X:X> <A.B.C.D|local>", 
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Nexthop Server configuration\n"
 "IPv4 protocol address\n"
 "IPv6 protocol address\n"
 "IPv4 NBMA address\n"
 "Handle protocol address locally\n")

DEFSH (VTYSH_ISISD, show_isis_topology_l2_cmd_vtysh, 
       "show isis topology level-2", 
       "Show running system information\n"
       "IS-IS information\n"
       "IS-IS paths to Intermediate Systems\n"
       "Paths to all level-2 routers in the domain\n")

DEFSH (VTYSH_RIPD, rip_timers_cmd_vtysh, 
       "timers basic (5-2147483647) (5-2147483647) (5-2147483647)", 
       "Adjust routing timers\n"
       "Basic routing protocol update timers\n"
       "Routing table update timer value in second. Default is 30.\n"
       "Routing information timeout timer. Default is 180.\n"
       "Garbage collection timer. Default is 120.\n")

DEFSH (VTYSH_BGPD, bgp_maxmed_onstartup_medv_cmd_vtysh, 
       "bgp max-med on-startup (5-86400) (0-4294967294)", 
       "BGP information\n"
       "Advertise routes with max-med\n"
       "Effective on a startup\n"
       "Time (seconds) period for max-med\n"
       "Max MED value to be used\n")

DEFSH (VTYSH_ISISD, no_isis_hello_multiplier_cmd_vtysh, 
       "no isis hello-multiplier [(2-100)]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set multiplier for Hello holding time\n"
       "Hello multiplier value\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_prefix_cmd_vtysh, 
       "show ipv6 route vrf all X:X::X:X/M", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_OSPF6D, ospf6_distance_ospf6_cmd_vtysh, 
       "distance ospf6 {intra-area (1-255)|inter-area (1-255)|external (1-255)}", 
       "Administrative distance\n"
       "OSPF6 administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n")

DEFSH (VTYSH_ZEBRA, no_ip_protocol_cmd_vtysh, 
       "no ip protocol " "<kernel|connected|static|rip|ospf|isis|bgp|pim|nhrp|table|vnc|any>" " [route-map ROUTE-MAP]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Stop filtering routing info between zebra and protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_PIMD, no_ip_pim_keep_alive_cmd_vtysh, 
       "no ip pim keep-alive-timer (31-60000)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Keep alive Timer\n"
       "Seconds\n")

DEFSH (VTYSH_ZEBRA, zebra_route_map_timer_cmd_vtysh, 
       "zebra route-map delay-timer (0-600)", 
       "Zebra information\n"
       "Set route-map parameters\n"
       "Time to wait before route-map updates are processed\n"
       "0 means event-driven updates are disabled\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_le_ge_cmd_vtysh, 
       "no ip prefix-list WORD <deny|permit> A.B.C.D/M le (0-32) ge (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_BGPD, no_neighbor_route_map_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> route-map WORD <in|out>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")

DEFSH (VTYSH_PIMD, interface_ip_mroute_cmd_vtysh, 
       "ip mroute INTERFACE A.B.C.D", 
       "IP information\n"
       "Add multicast route\n"
       "Outgoing interface name\n"
       "Group address\n")

DEFSH (VTYSH_RIPD, no_ip_rip_v2_broadcast_cmd_vtysh, 
       "no ip rip v2-broadcast", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Send ip broadcast v2 update\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_messages_recv_cmd_vtysh, 
       "debug mpls ldp messages recv", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Received messages,  excluding periodic Keep Alives\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_ra_lifetime_cmd_vtysh, 
       "no ipv6 nd ra-lifetime [(0-9000)]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router lifetime\n"
       "Router lifetime in seconds (0 stands for a non-default gw)\n")

DEFSH (VTYSH_BGPD, no_match_ipv6_next_hop_cmd_vtysh, 
       "no match ipv6 next-hop X:X::X:X", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IPv6 information\n"
       "Match IPv6 next-hop address of route\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_BGPD, neighbor_interface_config_remote_as_cmd_vtysh, 
       "neighbor WORD interface remote-as <(1-4294967295)|internal|external>", 
       "Specify neighbor router\n"
       "Interface name or neighbor tag\n"
       "Enable BGP on interface\n"
       "Specify a BGP neighbor\n"
       "AS number\n"
       "Internal BGP peer\n"
       "External BGP peer\n")

DEFSH (VTYSH_ISISD, no_isis_network_cmd_vtysh, 
       "no isis network point-to-point", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set network type for circuit\n"
       "point-to-point network type\n")

DEFSH (VTYSH_RIPD, accept_lifetime_month_day_month_day_cmd_vtysh, 
       "accept-lifetime HH:MM:SS MONTH (1-31) (1993-2035) HH:MM:SS MONTH (1-31) (1993-2035)", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Month of the year to expire\n"
       "Day of th month to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_BGPD, neighbor_interface_config_v6only_cmd_vtysh, 
       "neighbor WORD interface v6only [peer-group WORD]", 
       "Specify neighbor router\n"
       "Interface name or neighbor tag\n"
       "Enable BGP on interface\n"
       "Enable BGP with v6 link-local only\n"
       "Member of the peer-group\n"
       "Peer-group name\n")

DEFSH (VTYSH_OSPFD, ip_ospf_dead_interval_cmd_vtysh, 
       "ip ospf dead-interval (1-65535) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval time after which a neighbor is declared down\n"
       "Seconds\n"
       "Address of interface\n")

DEFSH (VTYSH_ZEBRA, show_ip_rpf_addr_cmd_vtysh, 
       "show ip rpf A.B.C.D", 
       "Show running system information\n"
       "IP information\n"
       "Display RPF information for multicast source\n"
       "IP multicast source address (e.g. 10.0.0.0)\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_pw_cmd_vtysh, 
       "[no] debug zebra pseudowires", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra pseudowires\n")

DEFSH (VTYSH_PIMD, no_ip_pim_rp_prefix_list_cmd_vtysh, 
       "no ip pim rp A.B.C.D prefix-list WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Rendevous Point\n"
       "ip address of RP\n"
       "group prefix-list filter\n"
       "Name of a prefix-list\n")

DEFSH (VTYSH_ALL, no_vty_access_class_cmd_vtysh, 
       "no access-class [WORD]", 
       "Negate a command or set its defaults\n"
       "Filter connections based on an IP access list\n"
       "IP access list\n")

DEFSH (VTYSH_BGPD, debug_bgp_update_direct_cmd_vtysh, 
       "debug bgp updates <in|out>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n"
       "Inbound updates\n"
       "Outbound updates\n")

DEFSH (VTYSH_ISISD, no_debug_isis_events_cmd_vtysh, 
       "no debug isis events", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS Events\n")

DEFSH (VTYSH_BGPD, no_neighbor_default_originate_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> default-originate [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Originate default route to this neighbor\n"
       "Route-map to specify criteria to originate default\n"
       "route-map name\n")

DEFSH (VTYSH_ZEBRA, pseudowire_labels_cmd_vtysh, 
       "[no] mpls label local (16-1048575) remote (16-1048575)", 
       "Negate a command or set its defaults\n"
       "MPLS L2VPN PW command\n"
       "MPLS L2VPN static labels\n"
       "Local pseudowire label\n"
       "Local pseudowire label\n"
       "Remote pseudowire label\n"
       "Remote pseudowire label\n")

DEFSH (VTYSH_BGPD, match_origin_cmd_vtysh, 
       "match origin <egp|igp|incomplete>", 
       "Match values from routing table\n"
       "BGP origin code\n"
       "remote EGP\n"
       "local IGP\n"
       "unknown heritage\n")

DEFSH (VTYSH_LDPD, ldp_mtu_mtu_cmd_vtysh, 
       "mtu (1500-9180)", 
       "set Maximum Transmission Unit\n"
       "Maximum Transmission Unit value\n")

DEFSH (VTYSH_PIMD, interface_no_ip_igmp_join_cmd_vtysh, 
       "no ip igmp join A.B.C.D A.B.C.D", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP join multicast group\n"
       "Multicast group address\n"
       "Source address\n")

DEFSH (VTYSH_ISISD, psnp_interval_l2_cmd_vtysh, 
       "isis psnp-interval (1-120) level-2", 
       "IS-IS commands\n"
       "Set PSNP interval in seconds\n"
       "PSNP interval value\n"
       "Specify interval for level-2 PSNPs\n")

DEFSH (VTYSH_PIMD, ip_pim_ssm_prefix_list_cmd_vtysh, 
       "ip pim ssm prefix-list WORD", 
       "IP information\n"
       "pim multicast routing\n"
       "Source Specific Multicast\n"
       "group range prefix-list filter\n"
       "Name of a prefix-list\n")

DEFSH (VTYSH_ALL, no_access_list_extended_mask_host_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip A.B.C.D A.B.C.D host A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "A single destination host\n"
       "Destination address\n")

DEFSH (VTYSH_LDPD, ldp_no_pw_id_pwid_cmd_vtysh, 
       "no pw-id (1-4294967295)", 
       "Negate a command or set its defaults\n"
       "Set the Virtual Circuit ID\n"
       "Virtual Circuit ID value\n")

DEFSH (VTYSH_ZEBRA, no_ip_zebra_import_table_cmd_vtysh, 
       "no ip import-table (1-252) [distance (1-255)] [route-map NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "import routes from non-main kernel table\n"
       "kernel routing table id\n"
       "Distance for imported routes\n"
       "Default distance value\n"
       "route-map for filtering\n"
       "route-map name\n")

DEFSH (VTYSH_LDPD, ldp_no_mtu_mtu_cmd_vtysh, 
       "no mtu (1500-9180)", 
       "Negate a command or set its defaults\n"
       "set Maximum Transmission Unit\n"
       "Maximum Transmission Unit value\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_timers_throttle_spf_cmd_vtysh, 
       "no timers throttle spf [(0-600000) (0-600000) (0-600000)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "Throttling adaptive timer\n"
       "OSPF6 SPF timers\n"
       "Delay (msec) from first change received till SPF calculation\n"
       "Initial hold time (msec) between consecutive SPF calculations\n"
       "Maximum hold time (msec)\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_rmap_continue_cmd_vtysh, 
       "no continue [(1-65535)]", 
       "Negate a command or set its defaults\n"
       "Continue on a different entry within the route-map\n"
       "Route-map entry sequence number\n")

DEFSH (VTYSH_BGPD, no_bgp_maxpaths_ibgp_cmd_vtysh, 
       "no maximum-paths ibgp [(1-255) [equal-cluster-length]]", 
       "Negate a command or set its defaults\n"
       "Forward packets over multiple paths\n"
       "iBGP-multipath\n"
       "Number of paths\n"
       "Match the cluster length\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_targeted_hello_accept_cmd_vtysh, 
       "no discovery targeted-hello accept", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Accept and respond to targeted hellos\n")

DEFSH (VTYSH_BGPD, set_originator_id_cmd_vtysh, 
       "set originator-id A.B.C.D", 
       "Set values in destination routing protocol\n"
       "BGP originator ID attribute\n"
       "IP address of originator\n")

DEFSH (VTYSH_ZEBRA, link_params_max_rsv_bw_cmd_vtysh, 
       "max-rsv-bw BANDWIDTH", 
       "Maximum bandwidth that may be reserved\n"
       "Bytes/second (IEEE floating point format)\n")

DEFSH (VTYSH_BGPD, match_aspath_cmd_vtysh, 
       "match as-path WORD", 
       "Match values from routing table\n"
       "Match BGP AS path list\n"
       "AS path access-list name\n")

DEFSH (VTYSH_ALL, no_access_list_extended_host_mask_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip host A.B.C.D A.B.C.D A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Destination address\n"
       "Destination Wildcard bits\n")

DEFSH (VTYSH_LDPD, ldp_no_address_family_ipv4_cmd_vtysh, 
       "no address-family ipv4", 
       "Negate a command or set its defaults\n"
       "Configure Address Family and its parameters\n"
       "IPv4\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_mask_label_cmd_vtysh, 
       "no ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ISISD, isis_metric_l2_cmd_vtysh, 
       "isis metric (0-16777215) level-2", 
       "IS-IS commands\n"
       "Set default metric for circuit\n"
       "Default metric value\n"
       "Specify metric for level-2 routing\n")

DEFSH (VTYSH_RIPD, no_router_rip_cmd_vtysh, 
       "no router rip", 
       "Negate a command or set its defaults\n"
       "Enable a routing process\n"
       "Routing Information Protocol (RIP)\n")

DEFSH (VTYSH_OSPFD, ospf_max_metric_router_lsa_admin_cmd_vtysh, 
       "max-metric router-lsa administrative", 
       "OSPF maximum / infinite-distance metric\n"
       "Advertise own Router-LSA with infinite distance (stub router)\n"
       "Administratively applied,  for an indefinite period\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_vn_un_prefix_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> vn <*|A.B.C.D|X:X::X:X> un <*|A.B.C.D|X:X::X:X> prefix <*|A.B.C.D/M|X:X::X:X/M>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "VN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_LDPD, ldp_discovery_targeted_hello_accept_from_acl_cmd_vtysh, 
       "discovery targeted-hello accept from <(1-199)|(1300-2699)|WORD>", 
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Accept and respond to targeted hellos\n"
       "Access list to specify acceptable targeted hello source\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_BGPD, show_ip_community_list_cmd_vtysh, 
       "show ip community-list", 
       "Show running system information\n"
       "IP information\n"
       "List community-list\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_homeagent_config_flag_cmd_vtysh, 
       "ipv6 nd home-agent-config-flag", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent configuration flag\n")

DEFSH (VTYSH_BGPD, set_community_delete_cmd_vtysh, 
       "set comm-list <(1-99)|(100-500)|WORD> delete", 
       "Set values in destination routing protocol\n"
       "set BGP community list (for deletion)\n"
       "Community-list number (standard)\n"
       "Community-list number (expanded)\n"
       "Community-list name\n"
       "Delete matching communities\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_set_ipv6_nexthop_local_cmd_vtysh, 
       "no set ipv6 next-hop local [X:X::X:X]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "IPv6 local address\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_OSPFD, pce_address_cmd_vtysh, 
       "pce address A.B.C.D", 
       "PCE Router Information specific commands\n"
       "Stable IP address of the PCE\n"
       "PCE address in IPv4 address format\n")

DEFSH (VTYSH_ISISD, no_max_lsp_lifetime_cmd_vtysh, 
       "no max-lsp-lifetime [<level-1|level-2>] [(350-65535)]", 
       "Negate a command or set its defaults\n"
       "Maximum LSP lifetime\n"
       "Maximum LSP lifetime for Level 1 only\n"
       "Maximum LSP lifetime for Level 2 only\n"
       "LSP lifetime in seconds\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_cost_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> cost (0-255)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_BGPD, no_neighbor_remove_private_as_all_replace_as_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS all replace-AS", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n"
       "Apply to all AS numbers\n"
       "Replace private ASNs with our ASN in outbound updates\n")

DEFSH (VTYSH_BGPD, show_ip_extcommunity_list_cmd_vtysh, 
       "show ip extcommunity-list", 
       "Show running system information\n"
       "IP information\n"
       "List extended-community list\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_database_cmd_vtysh, 
       "show ip ospf [(1-65535)] database [<asbr-summary|external|network|router|summary|nssa-external|opaque-link|opaque-area|opaque-as> [A.B.C.D [<self-originate|adv-router A.B.C.D>]]]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Database summary\n"
        "ASBR summary link states\n" "External link states\n" "Network link states\n" "Router link states\n" "Network summary link states\n" "NSSA external link state\n" "Link local Opaque-LSA\n" "Link area Opaque-LSA\n" "Link AS Opaque-LSA\n"
       "Link State ID (as an IP address)\n"
       "Self-originated link states\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n")

DEFSH (VTYSH_BGPD, bgp_listen_range_cmd_vtysh, 
       "bgp listen range <A.B.C.D/M|X:X::X:X/M> peer-group WORD", 
       "BGP specific commands\n"
       "Configure BGP dynamic neighbors listen range\n"
       "Configure BGP dynamic neighbors listen range\n"
       "Neighbor address\nIPv6 address\n"
       "Member of the peer-group\n"
       "Peer-group name\n")

DEFSH (VTYSH_BGPD, no_bgp_coalesce_time_cmd_vtysh, 
       "no coalesce-time (0-4294967295)", 
       "Negate a command or set its defaults\n"
       "Subgroup coalesce timer\n"
       "Subgroup coalesce timer value (in ms)\n")

DEFSH (VTYSH_ISISD, domain_passwd_clear_cmd_vtysh, 
       "domain-password clear WORD [authenticate snp <send-only|validate>]", 
       "Set the authentication password for a routing domain\n"
       "Authentication type\n"
       "Area password\n"
       "Authentication\n"
       "SNP PDUs\n"
       "Send but do not check PDUs on receiving\n"
       "Send and check PDUs on receiving\n")

DEFSH (VTYSH_BGPD, no_bgp_enforce_first_as_cmd_vtysh, 
       "no bgp enforce-first-as", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Enforce the first AS for EBGP routes\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_update_groups_cmd_vtysh, 
       "no debug bgp update-groups", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP update-groups\n")

DEFSH (VTYSH_RIPD, rip_default_metric_cmd_vtysh, 
       "default-metric (1-16)", 
       "Set a metric of redistribute routes\n"
       "Default metric\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_messages_sent_all_cmd_vtysh, 
       "no debug mpls ldp messages sent all", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Sent messages,  excluding periodic Keep Alives\n"
       "Sent messages,  including periodic Keep Alives\n")

DEFSH (VTYSH_RIPD, ip_rip_send_version_1_cmd_vtysh, 
       "ip rip send version (1-1) (2-2)", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement transmission\n"
       "Version control\n"
       "RIP version 1\n"
       "RIP version 2\n")

DEFSH (VTYSH_OSPF6D, area_filter_list_cmd_vtysh, 
       "area A.B.C.D filter-list prefix WORD <in|out>", 
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "Filter networks between OSPF6 areas\n"
       "Filter prefixes between OSPF6 areas\n"
       "Name of an IPv6 prefix-list\n"
       "Filter networks sent to this area\n"
       "Filter networks sent from this area\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_transmit_delay_cmd_vtysh, 
              "ospf transmit-delay (1-65535) [A.B.C.D]", 
              "OSPF interface commands\n"
              "Link state transmit delay\n"
              "Seconds\n"
              "Address of interface")

DEFSH (VTYSH_OSPFD, ospf_opaque_cmd_vtysh, 
       "ospf opaque-lsa", 
       "OSPF specific commands\n"
       "Enable the Opaque-LSA capability (rfc2370)\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_messages_sent_cmd_vtysh, 
       "no debug mpls ldp messages sent", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Sent messages,  excluding periodic Keep Alives\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_redistribute_cmd_vtysh, 
       "show ipv6 ospf6 redistribute", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "redistributing External information\n"
       )

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_neighbor_one_cmd_vtysh, 
       "show ipv6 ospf6 neighbor A.B.C.D", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Neighbor list\n"
       "Specify Router-ID as IPv4 address notation\n"
      )

DEFSH (VTYSH_PIMD, no_debug_msdp_events_cmd_vtysh, 
       "no debug msdp events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "MSDP protocol activity\n"
       "MSDP protocol events\n")

DEFSH (VTYSH_BGPD, no_neighbor_passive_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> passive", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Don't send open messages to this neighbor\n")

DEFSH (VTYSH_RIPD, rip_allow_ecmp_cmd_vtysh, 
       "allow-ecmp", 
       "Allow Equal Cost MultiPath\n")

DEFSH (VTYSH_BGPD, neighbor_disable_connected_check_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> <disable-connected-check|enforce-multihop>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "one-hop away EBGP peer using loopback address\n"
       "Enforce EBGP neighbors perform multihop\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_interface_cmd_vtysh, 
       "no match interface [INTERFACE]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match first hop interface of route\n"
       "Interface name\n")

DEFSH (VTYSH_ISISD, no_spf_interval_cmd_vtysh, 
       "no spf-interval [[<level-1|level-2>] (1-120)]", 
       "Negate a command or set its defaults\n"
       "Minimum interval between SPF calculations\n"
       "Set interval for level 1 only\n"
       "Set interval for level 2 only\n"
       "Minimum interval between consecutive SPFs in seconds\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_ge_le_cmd_vtysh, 
       "no ip prefix-list WORD <deny|permit> A.B.C.D/M ge (0-32) le (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_route_cmd_vtysh, 
       "debug ospf6 route <table|intra-area|inter-area|memory>", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug routes\n"
       "Debug route table calculation\n"
       "Debug intra-area route calculation\n"
       "Debug inter-area route calculation\n"
       "Debug route memory use\n"
       )

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_description_cmd_vtysh, 
       "ip prefix-list WORD description LINE...", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")

DEFSH (VTYSH_OSPF6D, no_area_filter_list_cmd_vtysh, 
       "no area A.B.C.D filter-list prefix WORD <in|out>", 
       "Negate a command or set its defaults\n"
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "Filter networks between OSPF6 areas\n"
       "Filter prefixes between OSPF6 areas\n"
       "Name of an IPv6 prefix-list\n"
       "Filter networks sent to this area\n"
       "Filter networks sent from this area\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_holdtime_cmd_vtysh, 
 "no " "<ip|ipv6>" " nhrp holdtime [(1-65000)]", 
 "Negate a command or set its defaults\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Specify NBMA address validity time\n"
 "Time in seconds that NBMA addresses are advertised valid\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_summary_cmd_vtysh, 
       "show ip route vrf all summary ", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Summary of all routes\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_abr_cmd_vtysh, 
       "no debug ospf6 abr", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 ABR function\n"
      )

DEFSH (VTYSH_BGPD, no_bgp_graceful_restart_preserve_fw_cmd_vtysh, 
       "no bgp graceful-restart preserve-fw-state", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Graceful restart capability parameters\n"
       "Unsets F-bit indication that fib is preserved while doing Graceful Restart\n")

DEFSH (VTYSH_ISISD, clear_isis_neighbor_arg_cmd_vtysh, 
       "clear isis neighbor WORD", 
       "Reset functions\n"
       "ISIS network information\n"
       "ISIS neighbor adjacencies\n"
       "System id\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_address_cmd_vtysh, 
       "no ipv6 address X:X::X:X/M", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Set the IP address of an interface\n"
       "IPv6 address (e.g. 3ffe:506::1/48)\n")

DEFSH (VTYSH_ZEBRA, show_ip_forwarding_cmd_vtysh, 
       "show ip forwarding", 
       "Show running system information\n"
       "IP information\n"
       "IP forwarding status\n")

DEFSH (VTYSH_ZEBRA, no_link_params_admin_grp_cmd_vtysh, 
       "no admin-grp", 
       "Negate a command or set its defaults\n"
       "Disable Administrative group membership on this interface\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_nssa_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> nssa [<translate-candidate|translate-never|translate-always> [no-summary]]", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "Configure NSSA-ABR for translate election (default)\n"
       "Configure NSSA-ABR to never translate\n"
       "Configure NSSA-ABR to always translate\n"
       "Do not inject inter-area routes into nssa\n")

DEFSH (VTYSH_ALL, access_list_extended_host_host_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip host A.B.C.D host A.B.C.D", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "A single destination host\n"
       "Destination address\n")

DEFSH (VTYSH_OSPFD, ospf_mpls_te_inter_as_area_cmd_vtysh, 
       "mpls-te inter-as area <A.B.C.D|(0-4294967295)>", 
       "MPLS-TE specific commands\n"
       "Configure MPLS-TE Inter-AS support\n"
       "AREA native mode self originate INTER_AS LSA with Type 10 (area flooding scope)\n"
       "OSPF area ID in IP format\n"
       "OSPF area ID as decimal value\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_brouter_area_cmd_vtysh, 
       "no debug ospf6 border-routers area-id", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug border router\n"
       "Debug border routers in specific Area\n"
      )

DEFSH (VTYSH_BGPD, show_ip_bgp_instance_neighbor_advertised_route_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] "
       "neighbors <A.B.C.D|X:X::X:X|WORD> <received-routes|advertised-routes> [route-map WORD] [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n"
       "Neighbor on BGP configured interface\n"
       "Display the received routes from neighbor\n"
       "Display the routes advertised to a BGP neighbor\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_LDPD, ldp_label_local_allocate_host_routes_cmd_vtysh, 
       "label local allocate host-routes", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure label allocation control\n"
       "allocate local label for host routes only\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_cmd_vtysh, 
       "show ip route [json]", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ALL, vty_access_class_cmd_vtysh, 
       "access-class WORD", 
       "Filter connections based on an IP access list\n"
       "IP access list\n")

DEFSH (VTYSH_OSPFD, ospf_redistribute_source_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|rip|isis|bgp|pim|nhrp|table|vnc>" " [<metric (0-16777214)|metric-type (1-2)|route-map WORD>]", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_RIPD, accept_lifetime_day_month_month_day_cmd_vtysh, 
       "accept-lifetime HH:MM:SS (1-31) MONTH (1993-2035) HH:MM:SS MONTH (1-31) (1993-2035)", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Month of the year to expire\n"
       "Day of th month to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_LDPD, ldp_no_label_remote_accept_from_acl_for_acl_cmd_vtysh, 
       "no label remote accept from <(1-199)|(1300-2699)|WORD> for <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "Neighbor from whom to accept label advertisement\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_homeagent_preference_cmd_vtysh, 
       "ipv6 nd home-agent-preference (0-65535)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent preference\n"
       "preference value (default is 0,  least preferred)\n")

DEFSH (VTYSH_PIMD, no_debug_igmp_events_cmd_vtysh, 
       "no debug igmp events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n"
       "IGMP protocol events\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_rmap_onmatch_next_cmd_vtysh, 
       "no on-match next", 
       "Negate a command or set its defaults\n"
       "Exit policy on matches\n"
       "Next clause\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, clear_ip_prefix_list_cmd_vtysh, 
       "clear ip prefix-list", 
       "Reset functions\n"
       "IP information\n"
       "Build a prefix list\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_addr_cmd_vtysh, 
       "show ipv6 route vrf all X:X::X:X", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "IPv6 Address\n")

DEFSH (VTYSH_BGPD, bgp_always_compare_med_cmd_vtysh, 
       "bgp always-compare-med", 
       "BGP specific commands\n"
       "Allow comparing MED from different neighbors\n")

DEFSH (VTYSH_BGPD, bgp_enforce_first_as_cmd_vtysh, 
       "bgp enforce-first-as", 
       "BGP information\n"
       "Enforce the first AS for EBGP routes\n")

DEFSH (VTYSH_BGPD, debug_bgp_update_direct_peer_cmd_vtysh, 
       "debug bgp updates <in|out> <A.B.C.D|X:X::X:X|WORD>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n"
       "Inbound updates\n"
       "Outbound updates\n"
       "BGP neighbor IP address to debug\n"
       "BGP IPv6 neighbor to debug\n"
       "BGP neighbor on interface to debug\n")

DEFSH (VTYSH_BGPD, bgp_graceful_restart_restart_time_cmd_vtysh, 
       "bgp graceful-restart restart-time (1-3600)", 
       "BGP specific commands\n"
       "Graceful restart capability parameters\n"
       "Set the time to wait to delete stale routes before a BGP open message is received\n"
       "Delay value (seconds)\n")

DEFSH (VTYSH_BGPD, bgp_network_mask_route_map_cmd_vtysh, 
       "network A.B.C.D mask A.B.C.D route-map WORD", 
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Network mask\n"
       "Network mask\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_tag_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> tag (1-4294967295) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_id_cmd_vtysh, 
       "show ipv6 ospf6 database <*|linkstate-id> A.B.C.D [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Any Link state Type\n"
       "Search by Link state ID\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_PIMD, no_ip_ssmpingd_cmd_vtysh, 
       "no ip ssmpingd [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable ssmpingd operation\n"
       "Source address\n")

DEFSH (VTYSH_BGPD, no_bgp_default_show_hostname_cmd_vtysh, 
       "no bgp default show-hostname", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "Show hostname in certain command ouputs\n")

DEFSH (VTYSH_BGPD, no_ip_lcommunity_list_name_standard_cmd_vtysh, 
       "no ip large-community-list standard WORD <deny|permit> AA:AA:NN...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a large community list entry\n"
       "Specify standard large-community-list\n"
       "Large Community list name\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "large community in 'aa:bb:cc' format\n")

DEFSH (VTYSH_RIPD, rip_network_cmd_vtysh, 
       "network <A.B.C.D/M|WORD>", 
       "Enable routing on an IP network\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Interface name\n")

DEFSH (VTYSH_BGPD, neighbor_unsuppress_map_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> unsuppress-map WORD", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Route-map to selectively unsuppress suppressed routes\n"
       "Name of route map\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_zebra_cmd_vtysh, 
       "no debug bgp zebra", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Zebra messages\n")

DEFSH (VTYSH_OSPFD, ospf_timers_throttle_spf_cmd_vtysh, 
       "timers throttle spf (0-600000) (0-600000) (0-600000)", 
       "Adjust routing timers\n"
       "Throttling adaptive timer\n"
       "OSPF SPF timers\n"
       "Delay (msec) from first change received till SPF calculation\n"
       "Initial hold time (msec) between consecutive SPF calculations\n"
       "Maximum hold time (msec)\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_reachable_time_cmd_vtysh, 
       "ipv6 nd reachable-time (1-3600000)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Reachable time\n"
       "Reachable time in milliseconds\n")

DEFSH (VTYSH_OSPF6D, clear_ipv6_ospf6_interface_cmd_vtysh, 
       "clear ipv6 ospf6 interface [IFNAME]", 
       "Reset functions\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface infomation\n"
       "Interface name(e.g. ep0)\n"
       )

DEFSH (VTYSH_BGPD, add_vnc_prefix_life_cost_lnh_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> lifetime (1-4294967295) cost (0-255) LNH_OPTIONS...", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_OSPFD, ospf_area_nssa_no_summary_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> nssa no-summary", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "Do not inject inter-area routes into nssa\n")

DEFSH (VTYSH_RIPD, no_ip_rip_authentication_string_cmd_vtysh, 
       "no ip rip authentication string [LINE]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Authentication control\n"
       "Authentication string\n"
       "Authentication string\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_messages_recv_all_cmd_vtysh, 
       "debug mpls ldp messages recv all", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Received messages,  excluding periodic Keep Alives\n"
       "Received messages,  including periodic Keep Alives\n")

DEFSH (VTYSH_BGPD, neighbor_interface_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X> interface WORD", 
       "Specify neighbor router\n"
       "Neighbor address\nIPv6 address\n"
       "Interface\n"
       "Interface name\n")

DEFSH (VTYSH_BGPD, debug_rfapi_show_nves_cmd_vtysh, 
       "debug rfapi-dev show nves", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "Show running system information\n"
       "NVE Information\n")

DEFSH (VTYSH_BGPD, no_bgp_fast_external_failover_cmd_vtysh, 
       "no bgp fast-external-failover", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Immediately reset session if a link to a directly connected external peer goes down\n")

DEFSH (VTYSH_OSPFD, pce_domain_cmd_vtysh, 
       "pce domain as (0-65535)", 
       "PCE Router Information specific commands\n"
       "Configure PCE domain AS number\n"
       "AS number where the PCE as visibilities for path computation\n"
       "AS number in decimal <0-65535>\n")

DEFSH (VTYSH_BGPD, vnc_vrf_policy_nexthop_cmd_vtysh, 
       "nexthop <A.B.C.D|X:X::X:X|self>", 
       "Specify next hop to use for VRF advertised prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Use configured router-id (default)")

DEFSH (VTYSH_ISISD, no_area_passwd_cmd_vtysh, 
       "no <area-password|domain-password>", 
       "Negate a command or set its defaults\n"
       "Configure the authentication password for an area\n"
       "Set the authentication password for a routing domain\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_route_cmd_vtysh, 
       "show ip ospf route", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "OSPF routing table\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_simulate_spf_tree_root_cmd_vtysh, 
       "show ipv6 ospf6 simulate spf-tree A.B.C.D area A.B.C.D", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Shortest Path First calculation\n"
       "Show SPF tree\n"
       "Specify root's router-id to calculate another router's SPF tree\n"
       "OSPF6 area parameters\n"
       "Area ID (as an IPv4 notation)\n")

DEFSH (VTYSH_OSPFD, no_ospf_mpls_te_cmd_vtysh, 
       "no mpls-te [on]", 
       "Negate a command or set its defaults\n"
       "MPLS-TE specific commands\n"
       "Disable the MPLS-TE functionality\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_tag_distance_label_cmd_vtysh, 
       "no ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> tag (1-4294967295) (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Tag of this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_messages_sent_cmd_vtysh, 
       "debug mpls ldp messages sent", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Sent messages,  excluding periodic Keep Alives\n")

DEFSH (VTYSH_BGPD, rfp_example_config_value_cmd_vtysh, 
       "rfp example-config-value VALUE", 
       "RFP information\n"
       "Example value to be configured\n"
       "Value to display")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_to_acl_for_acl_cmd_vtysh, 
       "no label local advertise to <(1-199)|(1300-2699)|WORD> for <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP Access-list specifying controls on LDP Peers\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ISISD, no_psnp_interval_cmd_vtysh, 
       "no isis psnp-interval [(1-120)]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set PSNP interval in seconds\n"
       "PSNP interval value\n")

DEFSH (VTYSH_BGPD, vnc_no_redistribute_protocol_cmd_vtysh, 
       "no vnc redistribute <ipv4|ipv6> <bgp|bgp-direct|bgp-direct-to-nve-groups|connected|kernel|ospf|rip|static>", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Redistribute from other protocol\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "From BGP\n"
       "From BGP without Zebra\n"
       "From BGP without Zebra,  only to configured NVE groups\n"
       "Connected interfaces\n"
       "From kernel routes\n"
       "From Open Shortest Path First (OSPF)\n"
       "From Routing Information Protocol (RIP)\n" "From Static routes\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_cost_cmd_vtysh, 
       "no ipv6 ospf6 cost", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Calculate interface cost from bandwidth\n"
       )

DEFSH (VTYSH_RIPNGD, ripng_allow_ecmp_cmd_vtysh, 
       "allow-ecmp", 
       "Allow Equal Cost MultiPath\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_cmd_vtysh, 
       "show ip ospf neighbor [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_keepalive_cmd_vtysh, 
       "no debug bgp keepalives", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP keepalives\n")

DEFSH (VTYSH_LDPD, ldp_no_l2vpn_word_type_vpls_cmd_vtysh, 
       "no l2vpn WORD type vpls", 
       "Negate a command or set its defaults\n"
       "Configure l2vpn commands\n"
       "L2VPN name\n"
       "L2VPN type\n"
       "Virtual Private LAN Service\n")

DEFSH (VTYSH_BGPD, no_bgp_network_import_check_cmd_vtysh, 
       "no bgp network import-check", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "BGP network command\n"
       "Check BGP network route exists in IGP\n")

DEFSH (VTYSH_LDPD, ldp_label_remote_accept_for_acl_cmd_vtysh, 
       "label remote accept for <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_kernel_cmd_vtysh, 
       "no debug zebra kernel", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra between kernel interface\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_bfd_cmd_vtysh, 
       "ipv6 ospf6 bfd", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Enables BFD support\n"
       )

DEFSH (VTYSH_RIPD, send_lifetime_day_month_month_day_cmd_vtysh, 
       "send-lifetime HH:MM:SS (1-31) MONTH (1993-2035) HH:MM:SS MONTH (1-31) (1993-2035)", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Month of the year to expire\n"
       "Day of th month to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_BGPD, vnc_redistribute_bgp_exterior_cmd_vtysh, 
       "vnc redistribute <ipv4|ipv6> bgp-direct-to-nve-groups view NAME", 
       "VNC/RFP related configuration\n"
       "Redistribute routes into VNC\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "From BGP without Zebra,  only to configured NVE groups\n"
       "From BGP view\n" "BGP view name\n")

DEFSH (VTYSH_OSPFD, ospf_passive_interface_addr_cmd_vtysh, 
       "passive-interface <IFNAME [A.B.C.D]|default>", 
       "Suppress routing updates on an interface\n"
       "Interface's name\n"
       "IPv4 address\n"
       "Suppress routing updates on interfaces by default\n")

DEFSH (VTYSH_WATCHFRR, config_write_integrated_cmd_vtysh, 
      "write integrated", 
      "Write running configuration to memory,  network,  or terminal\n"
      "Write integrated all-daemon frr.conf file\n")

DEFSH (VTYSH_PIMD, interface_ip_pim_hello_cmd_vtysh, 
       "ip pim hello (1-180) [(1-180)]", 
       "IP information\n"
       "PIM information\n"
       "Hello Interval\n"
       "Time in seconds for Hello Interval\n"
       "Time in seconds for Hold Interval\n")

DEFSH (VTYSH_BGPD, debug_rfapi_register_vn_un_l2o_cmd_vtysh, 
       "debug rfapi-dev register vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M> lifetime SECONDS macaddr YY:YY:YY:YY:YY:YY lni (0-16777215)", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_register\n"
       "indicate vn addr follows\n"
       "virtual network IPv4 interface address\n"
       "virtual network IPv6 interface address\n"
       "indicate un addr follows\n"
       "underlay network IPv4 interface address\n"
       "underlay network IPv6 interface address\n"
       "indicate prefix follows\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "indicate lifetime follows\n"
       "Seconds of lifetime\n"
       "indicate MAC address follows\n"
       "MAC address\n"
       "indicate lni follows\n"
       "lni value range\n")

DEFSH (VTYSH_ALL, no_access_list_extended_host_host_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip host A.B.C.D host A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "A single destination host\n"
       "Destination address\n")

DEFSH (VTYSH_PIMD, show_ip_pim_upstream_join_desired_cmd_vtysh, 
       "show ip pim upstream-join-desired [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM upstream join-desired\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_database_type_adv_router_cmd_vtysh, 
       "show ip ospf (1-65535) database <asbr-summary|external|network|router|summary|nssa-external|opaque-link|opaque-area|opaque-as> <adv-router A.B.C.D|self-originate>", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Database summary\n"
       "ASBR summary link states\n" "External link states\n" "Network link states\n" "Router link states\n" "Network summary link states\n" "NSSA external link state\n" "Link local Opaque-LSA\n" "Link area Opaque-LSA\n" "Link AS Opaque-LSA\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n"
       "Self-originated link states\n")

DEFSH (VTYSH_BGPD, no_bgp_listen_range_cmd_vtysh, 
       "no bgp listen range <A.B.C.D/M|X:X::X:X/M> peer-group WORD", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Unconfigure BGP dynamic neighbors listen range\n"
       "Unconfigure BGP dynamic neighbors listen range\n"
       "Neighbor address\nIPv6 address\n"
       "Member of the peer-group\n"
       "Peer-group name\n")

DEFSH (VTYSH_ZEBRA, show_ip_nht_cmd_vtysh, 
       "show ip nht [vrf NAME]", 
       "Show running system information\n"
       "IP information\n"
       "IP nexthop tracking table\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_stub_no_summary_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> stub no-summary", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n"
       "Do not inject inter-area routes into area\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_summary_prefix_cmd_vtysh, 
       "show ipv6 route [vrf NAME] summary prefix", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Summary of all IPv6 routes\n"
       "Prefix routes\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_nssa_cmd_vtysh, 
       "no debug ospf nssa", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF nssa information\n")

DEFSH (VTYSH_BGPD, vnc_show_responses_some_pfx_cmd_vtysh, 
       "show vnc responses <active|removed> [<A.B.C.D/M|X:X::X:X/M|YY:YY:YY:YY:YY:YY>]", 
       "Show running system information\n"
       "VNC information\n"
       "List recent query responses\n"
       "show only active query responses\n"
       "show only removed query responses\n"
       "Limit output to a particular IPv4 prefix\n"
       "Limit output to a particular IPv6 prefix\n"
       "Limit output to a particular IPV6 address\n")

DEFSH (VTYSH_OSPFD, ip_ospf_authentication_addr_cmd_vtysh, 
       "ip ospf authentication [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Address of interface")

DEFSH (VTYSH_LDPD, ldp_no_member_interface_ifname_cmd_vtysh, 
       "no member interface IFNAME", 
       "Negate a command or set its defaults\n"
       "L2VPN member configuration\n"
       "Local interface\n"
       "Interface's name\n")

DEFSH (VTYSH_BGPD, debug_rfapi_query_vn_un_cmd_vtysh, 
       "debug rfapi-dev query vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> target <A.B.C.D|X:X::X:X>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_query\n"
       "indicate vn addr follows\n"
       "virtual network interface IPv4 address\n"
       "virtual network interface IPv6 address\n"
       "indicate un addr follows\n"
       "IPv4 un address\n"
       "IPv6 un address\n"
       "indicate target follows\n"
       "target IPv4 address\n"
       "target IPv6 address\n")

DEFSH (VTYSH_PIMD, no_ip_pim_ecmp_cmd_vtysh, 
       "no ip pim ecmp", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Disable PIM ECMP \n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_router_info_cmd_vtysh, 
       "show ip ospf router-info", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Router Information\n")

DEFSH (VTYSH_BGPD, no_bgp_graceful_restart_restart_time_cmd_vtysh, 
       "no bgp graceful-restart restart-time [(1-3600)]", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Graceful restart capability parameters\n"
       "Set the time to wait to delete stale routes before a BGP open message is received\n"
       "Delay value (seconds)\n")

DEFSH (VTYSH_BGPD, bgp_client_to_client_reflection_cmd_vtysh, 
       "bgp client-to-client reflection", 
       "BGP specific commands\n"
       "Configure client to client route reflection\n"
       "reflection of routes allowed\n")

DEFSH_HIDDEN (VTYSH_PIMD, no_ip_multicast_routing_cmd_vtysh, 
              "no ip multicast-routing", 
              "Negate a command or set its defaults\n"
              "IP information\n"
              "Global IP configuration subcommands\n"
              "Enable IP multicast forwarding\n")

DEFSH (VTYSH_LDPD, ldp_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh, 
       "discovery targeted-hello holdtime (1-65535)", 
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Targeted hello holdtime\n"
       "Time (seconds) - 65535 implies infinite\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, rmap_onmatch_next_cmd_vtysh, 
       "on-match next", 
       "Exit policy on matches\n"
       "Next clause\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_brouter_area_cmd_vtysh, 
       "debug ospf6 border-routers area-id A.B.C.D", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug border router\n"
       "Debug border routers in specific Area\n"
       "Specify Area-ID\n"
      )

DEFSH_HIDDEN (VTYSH_PIMD, ip_msdp_peer_cmd_vtysh, 
       "ip msdp peer A.B.C.D source A.B.C.D", 
       "IP information\n"
       "Configure multicast source discovery protocol\n"
       "Configure MSDP peer\n"
       "peer ip address\n"
       "Source address for TCP connection\n"
       "local ip address\n")

DEFSH (VTYSH_BGPD, clear_vnc_prefix_un_vn_cmd_vtysh, 
       "clear vnc prefix <*|A.B.C.D/M|X:X::X:X/M> un <*|A.B.C.D|X:X::X:X> vn <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "VN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_NHRPD, tunnel_source_cmd_vtysh, 
 "tunnel source INTERFACE", 
 "NHRP/GRE integration\n"
 "Tunnel device binding tracking\n"
 "Interface name\n")

DEFSH (VTYSH_BGPD, bgp_deterministic_med_cmd_vtysh, 
       "bgp deterministic-med", 
       "BGP specific commands\n"
       "Pick the best-MED path among paths advertised from the neighboring AS\n")

DEFSH (VTYSH_LDPD, ldp_dual_stack_cisco_interop_cmd_vtysh, 
       "dual-stack cisco-interop", 
       "Configure dual stack parameters\n"
       "Use Cisco non-compliant format to send and interpret the Dual-Stack capability TLV\n")

DEFSH (VTYSH_BGPD, bgp_fast_external_failover_cmd_vtysh, 
       "bgp fast-external-failover", 
       "BGP information\n"
       "Immediately reset session if a link to a directly connected external peer goes down\n")

DEFSH (VTYSH_ZEBRA, show_mpls_table_cmd_vtysh, 
       "show mpls table [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "MPLS table\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, no_ospf_auto_cost_reference_bandwidth_cmd_vtysh, 
       "no auto-cost reference-bandwidth [(1-4294967)]", 
       "Negate a command or set its defaults\n"
       "Calculate OSPF interface cost according to bandwidth\n"
       "Use reference bandwidth method to assign OSPF cost\n"
       "The reference bandwidth in terms of Mbits per second\n")

DEFSH (VTYSH_NHRPD, if_nhrp_mtu_cmd_vtysh, 
 "ip nhrp mtu <(576-1500)|opennhrp>", 
 "IP information\n"
 "Next Hop Resolution Protocol functions\n"
 "Configure NHRP advertised MTU\n"
 "MTU value\n"
 "Advertise bound interface MTU similar to OpenNHRP")

DEFSH (VTYSH_LDPD, ldp_clear_mpls_ldp_neighbor_addr_cmd_vtysh, 
       "clear mpls ldp neighbor <A.B.C.D|X:X::X:X>", 
       "Reset functions\n"
       "Reset MPLS statistical information\n"
       "Clear LDP state\n"
       "Clear LDP neighbor sessions\n"
       "IPv4 address\n"
       "IPv6 address\n")

DEFSH (VTYSH_BGPD, no_bgp_multiple_instance_cmd_vtysh, 
       "no bgp multiple-instance", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "BGP multiple instance\n")

DEFSH (VTYSH_ZEBRA, show_ip_nht_vrf_all_cmd_vtysh, 
       "show ip nht vrf all", 
       "Show running system information\n"
       "IP information\n"
       "IP nexthop tracking table\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_ALL, no_access_list_remark_cmd_vtysh, 
       "no access-list <(1-99)|(100-199)|(1300-1999)|(2000-2699)|WORD> remark", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n"
       "Access list entry comment\n")

DEFSH (VTYSH_OSPFD, router_info_area_cmd_vtysh, 
       "router-info <as|area A.B.C.D>", 
       "OSPF Router Information specific commands\n"
       "Enable the Router Information functionality with AS flooding scope\n"
       "Enable the Router Information functionality with Area flooding scope\n"
       "OSPF area ID in IP format")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_for_acl_to_acl_cmd_vtysh, 
       "no label local advertise for <(1-199)|(1300-2699)|WORD> to <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "IP Access-list specifying controls on LDP Peers\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_event_cmd_vtysh, 
       "debug mpls ldp event", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "LDP event information\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_ifname_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, clear_ip_prefix_list_name_cmd_vtysh, 
       "clear ip prefix-list WORD", 
       "Reset functions\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_OSPFD, debug_ospf_packet_cmd_vtysh, 
       "debug ospf [(1-65535)] packet <hello|dd|ls-request|ls-update|ls-ack|all> [<send [detail]|recv [detail]|detail>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF packets\n"
       "OSPF Hello\n"
       "OSPF Database Description\n"
       "OSPF Link State Request\n"
       "OSPF Link State Update\n"
       "OSPF Link State Acknowledgment\n"
       "OSPF all packets\n"
       "Packet sent\n"
       "Detail Information\n"
       "Packet received\n"
       "Detail Information\n"
       "Detail Information\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_ospf_cmd_vtysh, 
       "redistribute <ospf|table> (1-65535)", 
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n")

DEFSH (VTYSH_ALL, show_ipv6_access_list_name_cmd_vtysh, 
       "show ipv6 access-list WORD", 
       "Show running system information\n"
       "IPv6 information\n"
       "List IPv6 access lists\n"
       "IPv6 zebra access-list\n")

DEFSH (VTYSH_ISISD, no_is_type_cmd_vtysh, 
       "no is-type <level-1|level-1-2|level-2-only>", 
       "Negate a command or set its defaults\n"
       "IS Level for this routing process (OSI only)\n"
       "Act as a station router only\n"
       "Act as both a station router and an area router\n"
       "Act as an area router only\n")

DEFSH (VTYSH_LDPD, ldp_no_ttl_security_disable_cmd_vtysh, 
       "no ttl-security disable", 
       "Negate a command or set its defaults\n"
       "LDP ttl security check\n"
       "Disable ttl security\n")

DEFSH (VTYSH_ALL, no_access_list_standard_any_cmd_vtysh, 
       "no access-list <(1-99)|(1300-1999)> <deny|permit> any", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any source host\n")

DEFSH (VTYSH_RIPD, ip_rip_receive_version_2_cmd_vtysh, 
       "ip rip receive version (2-2) (1-1)", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement reception\n"
       "Version control\n"
       "RIP version 2\n"
       "RIP version 1\n")

DEFSH (VTYSH_ZEBRA, ip_route_flags_cmd_vtysh, 
       "ip route A.B.C.D/M <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, bgp_bestpath_aspath_confed_cmd_vtysh, 
       "bgp bestpath as-path confed", 
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Compare path lengths including confederation sets & sequences in selecting a route\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_ospf_rmap_metric_cmd_vtysh, 
       "redistribute <ospf|table> (1-65535) route-map WORD metric (0-4294967295)", 
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n"
       "Route map reference\n"
       "Pointer to route-map entries\n"
       "Metric for redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_RIPNGD, no_ripng_redistribute_ripng_cmd_vtysh, 
       "no redistribute ripng", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "RIPng route\n")

DEFSH (VTYSH_BGPD, bgp_update_delay_establish_wait_cmd_vtysh, 
       "update-delay (0-3600) (1-3600)", 
       "Force initial delay for best-path and updates\n"
       "Seconds\n"
       "Wait for peers to be established\n"
       "Seconds\n")

DEFSH (VTYSH_BGPD, no_neighbor_activate_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> activate", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Enable the Address Family for this Neighbor\n")

DEFSH (VTYSH_BGPD, bgp_graceful_restart_preserve_fw_cmd_vtysh, 
       "bgp graceful-restart preserve-fw-state", 
       "BGP specific commands\n"
       "Graceful restart capability parameters\n"
       "Sets F-bit indication that fib is preserved while doing Graceful Restart\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_rd_tags_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn rd ASN:nn_or_IP-address:nn tags", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information for a route distinguisher\n"
      "VPN Route Distinguisher\n" "Display BGP tags for prefixes\n")

DEFSH (VTYSH_BGPD, debug_rfapi_close_vn_un_cmd_vtysh, 
       "debug rfapi-dev close vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_close\n"
       "indicate vn addr follows\n"
       "virtual network interface IPv4 address\n"
       "virtual network interface IPv6 address\n"
       "indicate xt addr follows\n"
       "underlay network interface IPv4 address\n"
       "underlay network interface IPv6 address\n")

DEFSH (VTYSH_BGPD, vnc_no_l2_group_cmd_vtysh, 
       "no vnc l2-group NAME", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Configure a L2 group\n"
       "Group name\n")

DEFSH (VTYSH_OSPFD, no_pce_domain_cmd_vtysh, 
       "no pce domain as (0-65535)", 
       "Negate a command or set its defaults\n"
       "PCE Router Information specific commands\n"
       "Disable PCE domain AS number\n"
       "AS number where the PCE as visibilities for path computation\n"
       "AS number in decimal <0-65535>\n")

DEFSH (VTYSH_ISISD, debug_isis_err_cmd_vtysh, 
       "debug isis protocol-errors", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS LSP protocol errors\n")

DEFSH (VTYSH_OSPF6D, ospf6_router_id_cmd_vtysh, 
       "router-id A.B.C.D", 
       "Configure OSPF Router-ID\n"
       "specify by IPv4 address notation(e.g. 0.0.0.0)\n")

DEFSH (VTYSH_RIPNGD, ripng_network_cmd_vtysh, 
       "network IF_OR_ADDR", 
       "RIPng enable on specified interface or network.\n"
       "Interface or address")

DEFSH (VTYSH_BGPD, neighbor_capability_orf_prefix_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> capability orf prefix-list <both|send|receive>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Advertise capability to the peer\n"
       "Advertise ORF capability to the peer\n"
       "Advertise prefixlist ORF capability to this neighbor\n"
       "Capability to SEND and RECEIVE the ORF to/from this neighbor\n"
       "Capability to RECEIVE the ORF from this neighbor\n"
       "Capability to SEND the ORF to this neighbor\n")

DEFSH (VTYSH_PIMD, show_ip_pim_assert_cmd_vtysh, 
       "show ip pim assert", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface assert\n")

DEFSH (VTYSH_ISISD, show_isis_topology_l1_cmd_vtysh, 
       "show isis topology level-1", 
       "Show running system information\n"
       "IS-IS information\n"
       "IS-IS paths to Intermediate Systems\n"
       "Paths to all level-1 routers in the area\n")

DEFSH (VTYSH_RIPD, no_rip_distance_source_cmd_vtysh, 
       "no distance (1-255) A.B.C.D/M", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n")

DEFSH (VTYSH_BGPD, no_neighbor_weight_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> weight [(0-65535)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Set default weight for routes from this neighbor\n"
       "default weight\n")

DEFSH (VTYSH_BGPD, bgp_damp_set_cmd_vtysh, 
       "bgp dampening [(1-45) [(1-20000) (1-20000) (1-255)]]", 
       "BGP Specific commands\n"
       "Enable route-flap dampening\n"
       "Half-life time for the penalty\n"
       "Value to start reusing a route\n"
       "Value to start suppressing a route\n"
       "Maximum duration to suppress a stable route\n")

DEFSH (VTYSH_RIPD, accept_lifetime_duration_day_month_cmd_vtysh, 
       "accept-lifetime HH:MM:SS (1-31) MONTH (1993-2035) duration (1-2147483646)", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Duration of the key\n"
       "Duration seconds\n")

DEFSH (VTYSH_BGPD, vnc_defaults_responselifetime_cmd_vtysh, 
       "response-lifetime <LIFETIME|infinite>", 
       "Specify default response lifetime\n"
       "Response lifetime in seconds\n" "Infinite response lifetime\n")

DEFSH (VTYSH_PIMD, debug_igmp_packets_cmd_vtysh, 
       "debug igmp packets", 
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n"
       "IGMP protocol packets\n")

DEFSH (VTYSH_BGPD, bgp_network_backdoor_cmd_vtysh, 
       "network A.B.C.D/M backdoor", 
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n"
       "Specify a BGP backdoor route\n")

DEFSH (VTYSH_BGPD, neighbor_prefix_list_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> prefix-list WORD <in|out>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, set_tag_cmd_vtysh, 
       "set tag (1-4294967295)", 
       "Set values in destination routing protocol\n"
       "Tag value for routing protocol\n"
       "Tag value\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_stub_router_admin_cmd_vtysh, 
       "no stub-router administrative", 
       "Negate a command or set its defaults\n"
       "Make router a stub router\n"
       "Advertise ability to be a transit router\n"
       "Administratively applied,  for an indefinite period\n")

DEFSH (VTYSH_ISISD, no_isis_metric_cmd_vtysh, 
       "no isis metric [(0-16777215)]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set default metric for circuit\n"
       "Default metric value\n")

DEFSH (VTYSH_BGPD, no_neighbor_route_reflector_client_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> route-reflector-client", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Configure a neighbor as Route Reflector client\n")

DEFSH (VTYSH_PIMD, interface_ip_pim_sm_cmd_vtysh, 
       "ip pim sm", 
       "IP information\n"
       "PIM information\n"
       "Enable PIM SM operation\n")

DEFSH (VTYSH_BGPD, no_bgp_config_type_cmd_vtysh, 
       "no bgp config-type [<cisco|zebra>]", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Display configuration type\n"
       "cisco\n"
       "zebra\n")

DEFSH (VTYSH_ISISD, show_debugging_isis_cmd_vtysh, 
       "show debugging isis", 
       "Show running system information\n"
       "State of each debugging option\n"
       "IS-IS information\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_seq_le_cmd_vtysh, 
       "no ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M le (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_BGPD, debug_bgp_zebra_prefix_cmd_vtysh, 
       "debug bgp zebra prefix <A.B.C.D/M|X:X::X:X/M>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Zebra messages\n"
       "Specify a prefix to debug\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_linkstate_detail_cmd_vtysh, 
       "show ipv6 ospf6 linkstate detail", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display linkstate routing table\n"
       "Display detailed information\n")

DEFSH (VTYSH_PIMD, interface_pim_use_source_cmd_vtysh, 
       "ip pim use-source A.B.C.D", 
       "IP information\n"
       "pim multicast routing\n"
       "Configure primary IP address\n"
       "source ip address\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_vlink_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> virtual-link A.B.C.D [authentication] [<message-digest|null>] [<message-digest-key (1-255)|authentication-key>]", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n" "OSPF area ID in IP address format\n" "OSPF area ID as a decimal value\n" "Configure a virtual link\n" "Router ID of the remote ABR\n"
       "Enable authentication on this virtual link\n"
       "Use message-digest authentication\n"
       "Use null authentication\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Simple authentication password (key)")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_linkstate_cmd_vtysh, 
       "show ipv6 ospf6 linkstate <router A.B.C.D|network A.B.C.D A.B.C.D>", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display linkstate routing table\n"
       "Display Router Entry\n"
       "Specify Router ID as IPv4 address notation\n"
       "Display Network Entry\n"
       "Specify Router ID as IPv4 address notation\n"
       "Specify Link state ID as IPv4 address notation\n")

DEFSH_HIDDEN (VTYSH_BGPD, bgp_network_import_check_exact_cmd_vtysh, 
      "bgp network import-check exact", 
      "BGP specific commands\n"
      "BGP network command\n"
      "Check BGP network route exists in IGP\n"
      "Match route precisely\n")

DEFSH (VTYSH_BGPD, clear_vnc_nve_vn_cmd_vtysh, 
       "clear vnc nve vn <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "VN address of NVE\n"
       "All addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_interface_cmd_vtysh, 
       "match interface WORD", 
       "Match values from routing table\n"
       "match first hop interface of route\n"
       "Interface name\n")

DEFSH (VTYSH_BGPD, neighbor_default_originate_rmap_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> default-originate route-map WORD", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Originate default route to this neighbor\n"
       "Route-map to specify criteria to originate default\n"
       "route-map name\n")

DEFSH (VTYSH_BGPD, debug_rfapi_show_nves_vn_un_cmd_vtysh, 
  "debug rfapi-dev show nves <vn|un> <A.B.C.D|X:X::X:X>", 
  "Debugging functions (see also 'undebug')\n"
  "RF API debugging/testing command\n"
  "Show running system information\n"
  "NVE Information\n"
  "Specify virtual network\n"
  "Specify underlay network interface\n"
  "IPv4 address\n"
  "IPv6 address\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_adv_router_linkstate_id_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> adv-router A.B.C.D linkstate-id A.B.C.D [<dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Search by Advertising Router\n"
       "Specify Advertising Router as IPv4 address notation\n"
       "Search by Link state ID\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_ZEBRA, no_allow_external_route_update_cmd_vtysh, 
       "no allow-external-route-update", 
       "Negate a command or set its defaults\n"
       "Allow FRR routes to be overwritten by external processes\n")

DEFSH (VTYSH_BGPD, neighbor_default_originate_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> default-originate", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Originate default route to this neighbor\n")

DEFSH (VTYSH_ISISD, area_passwd_clear_cmd_vtysh, 
       "area-password clear WORD [authenticate snp <send-only|validate>]", 
       "Configure the authentication password for an area\n"
       "Authentication type\n"
       "Area password\n"
       "Authentication\n"
       "SNP PDUs\n"
       "Send but do not check PDUs on receiving\n"
       "Send and check PDUs on receiving\n")

DEFSH (VTYSH_RIPD, no_rip_route_cmd_vtysh, 
       "no route A.B.C.D/M", 
       "Negate a command or set its defaults\n"
       "RIP static route configuration\n"
       "IP prefix <network>/<length>\n")

DEFSH (VTYSH_BGPD, aggregate_address_cmd_vtysh, 
       "aggregate-address A.B.C.D/M [<as-set [summary-only]|summary-only [as-set]>]", 
       "Configure BGP aggregate entries\n"
       "Aggregate prefix\n"
       "Generate AS set path information\n"
       "Filter more specific routes from updates\n"
       "Filter more specific routes from updates\n"
       "Generate AS set path information\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_packet_cmd_vtysh, 
       "no debug zebra packet [<recv|send>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_prefix_cmd_vtysh, 
       "prefix <vn|un> <A.B.C.D/M|X:X::X:X/M>", 
       "Specify prefixes matching NVE VN or UN interfaces\n"
       "VN prefix\n"
       "UN prefix\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_BGPD, ip_lcommunity_list_name_standard1_cmd_vtysh, 
       "ip large-community-list standard WORD <deny|permit> AA:BB:CC...", 
       "IP information\n"
       "Add a large community list entry\n"
       "Specify standard large-community-list\n"
       "Large Community list name\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "large community in 'aa:bb:cc' format\n")

DEFSH (VTYSH_RIPNGD, no_ripng_default_information_originate_cmd_vtysh, 
       "no default-information originate", 
       "Negate a command or set its defaults\n"
       "Default route information\n"
       "Distribute default route\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_message_cmd_vtysh, 
       "debug ospf6 message <unknown|hello|dbdesc|lsreq|lsupdate|lsack|all> [<send|recv>]", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 message\n"
       "Debug Unknown message\n"
       "Debug Hello message\n"
       "Debug Database Description message\n"
       "Debug Link State Request message\n"
       "Debug Link State Update message\n"
       "Debug Link State Acknowledgement message\n"
       "Debug All message\n"
       "Debug only sending message\n"
       "Debug only receiving message\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_hellointerval_cmd_vtysh, 
       "ipv6 ospf6 hello-interval (1-65535)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       )

DEFSH (VTYSH_BGPD, vnc_redist_bgpdirect_no_prefixlist_cmd_vtysh, 
       "no vnc redistribute <bgp-direct|bgp-direct-to-nve-groups> <ipv4|ipv6> prefix-list", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Redistribute from other protocol\n"
       "Redistribute from BGP directly\n"
       "Redistribute from BGP without Zebra,  only to configured NVE groups\n"
       "IPv4 routes\n"
       "IPv6 routes\n" "Prefix-list for filtering redistributed routes\n")

DEFSH (VTYSH_RIPNGD, ripng_redistribute_type_metric_routemap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf6|isis|bgp|nhrp|table|vnc>" " metric (0-16) route-map WORD", 
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_RIPD, no_key_chain_cmd_vtysh, 
       "no key chain WORD", 
       "Negate a command or set its defaults\n"
       "Authentication key management\n"
       "Key-chain management\n"
       "Key-chain name\n")

DEFSH (VTYSH_RIPD, send_lifetime_day_month_day_month_cmd_vtysh, 
       "send-lifetime HH:MM:SS (1-31) MONTH (1993-2035) HH:MM:SS (1-31) MONTH (1993-2035)", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Day of th month to expire\n"
       "Month of the year to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_targeted_hello_interval_disc_time_cmd_vtysh, 
       "no discovery targeted-hello interval (1-65535)", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Targeted hello interval\n"
       "Time (seconds)\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_brouter_cmd_vtysh, 
       "no debug ospf6 border-routers", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug border router\n"
      )

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_ip_next_hop_prefix_list_cmd_vtysh, 
       "no match ip next-hop prefix-list [WORD]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match next-hop address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_PIMD, interface_no_ip_pim_hello_cmd_vtysh, 
       "no ip pim hello [(1-180) (1-180)]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "PIM information\n"
       "Hello Interval\n"
       "Time in seconds for Hello Interval\n"
       "Time in seconds for Hold Interval\n")

DEFSH (VTYSH_ZEBRA, link_params_enable_cmd_vtysh, 
       "enable", 
       "Activate link parameters on this interface\n")

DEFSH (VTYSH_BGPD, no_neighbor_remove_private_as_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n")

DEFSH (VTYSH_OSPFD|VTYSH_OSPF6D, ospf6_routemap_set_metric_type_cmd_vtysh, 
       "set metric-type <type-1|type-2>", 
       "Set value\n"
       "Type of metric\n"
       "OSPF6 external type 1 metric\n"
       "OSPF6 external type 2 metric\n")

DEFSH (VTYSH_PIMD, show_ip_pim_join_cmd_vtysh, 
       "show ip pim join [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface join information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_bgp_log_neighbor_changes_cmd_vtysh, 
       "no bgp log-neighbor-changes", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Log neighbor up/down and reset reason\n")

DEFSH (VTYSH_LDPD, ldp_discovery_transport_address_ipv6_cmd_vtysh, 
       "discovery transport-address X:X::X:X", 
       "Configure discovery parameters\n"
       "Specify transport address for TCP connection\n"
       "IPv6 address to be used as transport address\n")

DEFSH (VTYSH_ISISD, no_psnp_interval_l1_cmd_vtysh, 
       "no isis psnp-interval [(1-120)] level-1", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set PSNP interval in seconds\n"
       "PSNP interval value\n"
       "Specify interval for level-1 PSNPs\n")

DEFSH (VTYSH_OSPFD, no_ospf_timers_min_ls_arrival_cmd_vtysh, 
       "no timers lsa arrival [(0-1000)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "Throttling link state advertisement delays\n"
       "OSPF minimum arrival interval delay\n"
       "Delay (msec) between accepted LSAs\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_debug_disable_cmd_vtysh, 
       "ip irdp debug disable", 
       "IP information\n"
       "ICMP Router discovery debug Averts. and Solicits (short)\n"
       "IRDP debugging options\n"
       "Disable debugging for all IRDP events\n")

DEFSH (VTYSH_BGPD, neighbor_ebgp_multihop_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> ebgp-multihop", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Allow EBGP neighbors not on directly connected networks\n")

DEFSH (VTYSH_BGPD, vnc_nve_export_routemap_cmd_vtysh, 
       "vnc export <bgp|zebra> route-map NAME", 
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "Filters,  used in 'registering-nve' export mode\n"
       "Route-map for filtering exported routes\n" "Route map name\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_ifname_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_summary_name_cmd_vtysh, 
       "show ipv6 prefix-list summary WORD", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Summary of prefix lists\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_nht_vrf_all_cmd_vtysh, 
       "show ipv6 nht vrf all", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 nexthop tracking table\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_NHRPD, show_ip_nhrp_cmd_vtysh, 
 "show " "<ip|ipv6>" " nhrp [cache|nhs|shortcut|opennhrp]", 
 "Show running system information\n"
 "IP information\n" "IPv6 information\n"
 "NHRP information\n"
 "Forwarding cache information\n"
 "Next hop server information\n"
 "Shortcut information\n"
 "opennhrpctl style cache dump\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_spf_time_cmd_vtysh, 
       "debug ospf6 spf time", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug SPF Calculation\n"
       "Measure time taken by SPF Calculation\n"
      )

DEFSH (VTYSH_ISISD, max_lsp_lifetime_cmd_vtysh, 
       "max-lsp-lifetime [<level-1|level-2>] (350-65535)", 
       "Maximum LSP lifetime\n"
       "Maximum LSP lifetime for Level 1 only\n"
       "Maximum LSP lifetime for Level 2 only\n"
       "LSP lifetime in seconds\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_mroute_vrf_all_cmd_vtysh, 
       "show ipv6 mroute vrf all", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 Multicast routing table\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_BGPD, bgp_maxmed_admin_medv_cmd_vtysh, 
       "bgp max-med administrative (0-4294967294)", 
       "BGP information\n"
       "Advertise routes with max-med\n"
       "Administratively applied,  for an indefinite period\n"
       "Max MED value to be used\n")

DEFSH (VTYSH_OSPFD, clear_ip_ospf_interface_cmd_vtysh, 
       "clear ip ospf interface [IFNAME]", 
       "Reset functions\n"
       "IP information\n"
       "OSPF information\n"
       "Interface information\n"
       "Interface name\n")

DEFSH (VTYSH_BGPD, vnc_show_nves_cmd_vtysh, 
       "show vnc nves", 
       "Show running system information\n"
       "VNC information\n"
       "List known NVEs\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_rd_neighbor_advertised_routes_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn rd ASN:nn_or_IP-address:nn neighbors A.B.C.D advertised-routes [json]", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information for a route distinguisher\n"
      "VPN Route Distinguisher\n"
      "Detailed information on TCP and BGP neighbor connections\n"
      "Neighbor to display information about\n"
      "Display the routes advertised to a BGP neighbor\n" "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> addpath-tx-bestpath-per-AS", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Use addpath to advertise the bestpath per each neighboring AS\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_ospf_metric_cmd_vtysh, 
       "redistribute <ospf|table> (1-65535) metric (0-4294967295)", 
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n"
       "Metric for redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_te_cmd_vtysh, 
       "no debug ospf te", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF-TE information\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_metric_cmd_vtysh, 
       "match metric (0-4294967295)", 
       "Match values from routing table\n"
       "Match metric of route\n"
       "Metric value\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_self_originated_linkstate_id_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> self-originated linkstate-id A.B.C.D [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Display Self-originated LSAs\n"
       "Search by Link state ID\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_NHRPD, if_nhrp_holdtime_cmd_vtysh, 
 "<ip|ipv6>" " nhrp holdtime (1-65000)", 
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Specify NBMA address validity time\n"
 "Time in seconds that NBMA addresses are advertised valid\n")

DEFSH (VTYSH_RIPD, no_rip_version_cmd_vtysh, 
       "no version [(1-2)]", 
       "Negate a command or set its defaults\n"
       "Set routing protocol version\n"
       "Version\n")

DEFSH (VTYSH_ZEBRA, no_ns_netns_cmd_vtysh, 
       "no logical-router (1-65535) ns NAME", 
       "Negate a command or set its defaults\n"
       "Enable a Logical-Router\n"
       "Specify the Logical-Router identifier\n"
       "The Name Space\n"
       "The file name in " "/var/run/netns" ",  or a full pathname\n")

DEFSH (VTYSH_PIMD, interface_ip_igmp_cmd_vtysh, 
       "ip igmp", 
       "IP information\n"
       "Enable IGMP operation\n")

DEFSH (VTYSH_OSPFD, pce_cap_flag_cmd_vtysh, 
       "pce flag BITPATTERN", 
       "PCE Router Information specific commands\n"
       "Capabilities of the PCE for path computation\n"
       "32-bit Hexadecimal value\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_seq_ge_le_cmd_vtysh, 
       "no ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M ge (0-32) le (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_all_cmd_vtysh, 
       "show ip ospf neighbor all [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "include down status neighbor\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_PIMD, interface_ip_igmp_version_cmd_vtysh, 
       "ip igmp version (2-3)", 
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP version\n"
       "IGMP version number\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_event_cmd_vtysh, 
       "no debug ospf event", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF event information\n")

DEFSH (VTYSH_RIPD, send_lifetime_duration_month_day_cmd_vtysh, 
       "send-lifetime HH:MM:SS MONTH (1-31) (1993-2035) duration (1-2147483646)", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Duration of the key\n"
       "Duration seconds\n")

DEFSH (VTYSH_BGPD, neighbor_interface_config_cmd_vtysh, 
       "neighbor WORD interface [peer-group WORD]", 
       "Specify neighbor router\n"
       "Interface name or neighbor tag\n"
       "Enable BGP on interface\n"
       "Member of the peer-group\n"
       "Peer-group name\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_adv_router_linkstate_id_cmd_vtysh, 
       "show ipv6 ospf6 database adv-router A.B.C.D linkstate-id A.B.C.D [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Search by Advertising Router\n"
       "Specify Advertising Router as IPv4 address notation\n"
       "Search by Link state ID\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_RIPD, show_debugging_rip_cmd_vtysh, 
       "show debugging rip", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n")

DEFSH (VTYSH_RIPD, distribute_list_cmd_vtysh, 
       "distribute-list [prefix] WORD <in|out> [WORD]", 
       "Filter networks in routing updates\n"
       "Specify a prefix\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_name_cmd_vtysh, 
       "show ipv6 prefix-list WORD", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_cmd_vtysh, 
       "show ipv6 ospf6 database [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_BGPD, bgp_timers_cmd_vtysh, 
       "timers bgp (0-65535) (0-65535)", 
       "Adjust routing timers\n"
       "BGP timers\n"
       "Keepalive interval\n"
       "Holdtime\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_seq_cmd_vtysh, 
       "ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> <X:X::X:X/M|any>", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")

DEFSH (VTYSH_ISISD, no_debug_isis_err_cmd_vtysh, 
       "no debug isis protocol-errors", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS LSP protocol errors\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_bestpath_cmd_vtysh, 
       "no debug bgp bestpath", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP bestpath\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_ipv4_ttl_security_disable_cmd_vtysh, 
       "neighbor A.B.C.D ttl-security disable", 
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "LDP ttl security check\n"
       "Disable ttl security\n")

DEFSH (VTYSH_LDPD, ldp_no_session_holdtime_session_time_cmd_vtysh, 
       "no session holdtime (15-65535)", 
       "Negate a command or set its defaults\n"
       "Configure session parameters\n"
       "Configure session holdtime\n"
       "Time (seconds)\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_ism_cmd_vtysh, 
       "no debug ospf [(1-65535)] ism [<status|events|timers>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Interface State Machine\n"
       "ISM Status Information\n"
       "ISM Event Information\n"
       "ISM TImer Information\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_zebra_sendrecv_cmd_vtysh, 
       "no debug ospf6 zebra [<send|recv>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug connection between zebra\n"
       "Debug Sending zebra\n"
       "Debug Receiving zebra\n"
      )

DEFSH (VTYSH_ZEBRA, no_debug_zebra_events_cmd_vtysh, 
       "no debug zebra events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra events\n")

DEFSH (VTYSH_BGPD, set_aspath_prepend_asn_cmd_vtysh, 
       "set as-path prepend (1-4294967295)...", 
       "Set values in destination routing protocol\n"
       "Transform BGP AS_PATH attribute\n"
       "Prepend to the as-path\n"
       "AS number\n")

DEFSH (VTYSH_RIPNGD, debug_ripng_packet_cmd_vtysh, 
       "debug ripng packet", 
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng packet\n")

DEFSH (VTYSH_RIPD, no_rip_timers_cmd_vtysh, 
       "no timers basic [(0-65535) (0-65535) (0-65535)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "Basic routing protocol update timers\n"
       "Routing table update timer value in second. Default is 30.\n"
       "Routing information timeout timer. Default is 180.\n"
       "Garbage collection timer. Default is 120.\n")

DEFSH (VTYSH_ALL, no_vty_ipv6_access_class_cmd_vtysh, 
       "no ipv6 access-class [WORD]", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Filter connections based on an IP access list\n"
       "IPv6 access list\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_tag_label_cmd_vtysh, 
       "no ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> tag (1-4294967295) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Tag of this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_PIMD, debug_static_cmd_vtysh, 
       "debug static", 
       "Debugging functions (see also 'undebug')\n"
       "PIM Static Multicast Route activity\n")

DEFSH (VTYSH_LDPD, ldp_no_label_remote_accept_for_acl_from_acl_cmd_vtysh, 
       "no label remote accept for <(1-199)|(1300-2699)|WORD> from <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "Neighbor from whom to accept label advertisement\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_PIMD, show_ip_pim_rp_cmd_vtysh, 
       "show ip pim rp-info [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM RP information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, vrf_debug_cmd_vtysh, 
      "debug vrf", 
      "Debugging functions (see also 'undebug')\n"
      "VRF Debugging\n")

DEFSH (VTYSH_ZEBRA, no_fpm_remote_ip_cmd_vtysh, 
        "no fpm connection ip A.B.C.D port (1-65535)", 
        "fpm connection remote ip and port\n"
        "Connection\n"
        "Remote fpm server ip A.B.C.D\n"
        "Enter ip ")

DEFSH (VTYSH_ISISD, no_debug_isis_csum_cmd_vtysh, 
       "no debug isis checksum-errors", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS LSP checksum errors\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]]          [<             cidr-only             |dampening <flap-statistics|dampened-paths|parameters>             |route-map WORD             |prefix-list WORD             |filter-list WORD             |statistics             |community [<AA:NN|local-AS|no-advertise|no-export> [exact-match]]             |community-list <(1-500)|WORD> [exact-match]             |A.B.C.D/M longer-prefixes             |X:X::X:X/M longer-prefixes>          ] [json]", 
# 8432 "../bgpd/bgp_route.c"
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Display only routes with non-natural netmasks\n"
       "Display detailed information about dampening\n"
       "Display flap statistics of routes\n"
       "Display paths suppressed due to dampening\n"
       "Display detail of configured dampening parameters\n"
       "Display routes matching the route-map\n"
       "A route-map to match on\n"
       "Display routes conforming to the prefix-list\n"
       "Prefix-list name\n"
       "Display routes conforming to the filter-list\n"
       "Regular expression access list name\n"
       "BGP RIB advertisement statistics\n"
       "Display routes matching the communities\n"
       "Community number where AA and NN are (0-65535)\n"
       "Do not send outside local AS (well-known community)\n"
       "Do not advertise to any peer (well-known community)\n"
       "Do not export to next AS (well-known community)\n"
       "Exact match of the communities\n"
       "Display routes matching the community-list\n"
       "community-list number\n"
       "community-list name\n"
       "Exact match of the communities\n"
       "IPv4 prefix\n"
       "Display route and more specific routes\n"
       "IPv6 prefix\n"
       "Display route and more specific routes\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_neighbor_bfd_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> bfd [(2-255) (50-60000) (50-60000)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Disables BFD support\n"
       "Detect Multiplier\n"
       "Required min receive interval\n"
       "Desired min transmit interval\n")

DEFSH (VTYSH_PIMD, no_debug_pim_events_cmd_vtysh, 
       "no debug pim events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM protocol events\n")

DEFSH (VTYSH_BGPD, debug_rfapi_register_vn_un_cmd_vtysh, 
       "debug rfapi-dev register vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M> lifetime SECONDS", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_register\n"
       "indicate vn addr follows\n"
       "virtual network IPv4 interface address\n"
       "virtual network IPv6 interface address\n"
       "indicate un addr follows\n"
       "underlay network IPv4 interface address\n"
       "underlay network IPv6 interface address\n"
       "indicate prefix follows\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "indicate lifetime follows\n"
       "lifetime\n")

DEFSH (VTYSH_BGPD, dump_bgp_all_cmd_vtysh, 
       "dump bgp <all|all-et|updates|updates-et|routes-mrt> PATH [INTERVAL]", 
       "Dump packet\n"
       "BGP packet dump\n"
       "Dump all BGP packets\nDump all BGP packets (Extended Timestamp Header)\n"
       "Dump BGP updates only\nDump BGP updates only (Extended Timestamp Header)\n"
       "Dump whole BGP routing table\n"
       "Output filename\n"
       "Interval of output\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_authentication_key_authkey_addr_cmd_vtysh, 
              "no ospf authentication-key [AUTH_KEY [A.B.C.D]]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Authentication password (key)\n"
              "The OSPF password (key)")

DEFSH (VTYSH_RIPD, no_if_rmap_cmd_vtysh, 
       "no route-map ROUTEMAP_NAME <in|out> IFNAME", 
       "Negate a command or set its defaults\n"
       "Route map unset\n"
       "Route map name\n"
       "Route map for input filtering\n"
       "Route map for output filtering\n"
       "Route map interface name\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_other_config_flag_cmd_vtysh, 
       "no ipv6 nd other-config-flag", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Other statefull configuration flag\n")

DEFSH (VTYSH_OSPFD, ip_ospf_message_digest_key_cmd_vtysh, 
       "ip ospf message-digest-key (1-255) md5 KEY [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)\n"
       "Address of interface\n")

DEFSH (VTYSH_OSPFD, ospf_max_metric_router_lsa_startup_cmd_vtysh, 
       "max-metric router-lsa on-startup (5-86400)", 
       "OSPF maximum / infinite-distance metric\n"
       "Advertise own Router-LSA with infinite distance (stub router)\n"
       "Automatically advertise stub Router-LSA on startup of OSPF\n"
       "Time (seconds) to advertise self as stub-router\n")

DEFSH (VTYSH_BGPD, vnc_l2_group_lni_cmd_vtysh, 
       "logical-network-id <0-4294967295>", 
       "Specify Logical Network ID associated with group\n"
       "value\n")

DEFSH (VTYSH_BGPD, bgp_coalesce_time_cmd_vtysh, 
       "coalesce-time (0-4294967295)", 
       "Subgroup coalesce timer\n"
       "Subgroup coalesce timer value (in ms)\n")

DEFSH (VTYSH_BGPD, debug_bgp_update_cmd_vtysh, 
       "debug bgp updates", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_to_acl_cmd_vtysh, 
       "label local advertise to <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP Access-list specifying controls on LDP Peers\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ALL, access_list_extended_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Destination address\n"
       "Destination Wildcard bits\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_protocol_nht_cmd_vtysh, 
       "show ipv6 nht route-map", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Next Hop filtering status\n"
       "Route-map\n")

DEFSH (VTYSH_ISISD, area_passwd_md5_cmd_vtysh, 
       "area-password md5 WORD [authenticate snp <send-only|validate>]", 
       "Configure the authentication password for an area\n"
       "Authentication type\n"
       "Level-wide password\n"
       "Authentication\n"
       "SNP PDUs\n"
       "Send but do not check PDUs on receiving\n"
       "Send and check PDUs on receiving\n")

DEFSH (VTYSH_BGPD, no_neighbor_strict_capability_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X> strict-capability-match", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nIPv6 address\n"
       "Strict capability negotiation match\n")

DEFSH (VTYSH_BGPD, bgp_damp_unset_cmd_vtysh, 
       "no bgp dampening [(1-45) [(1-20000) (1-20000) (1-255)]]", 
       "Negate a command or set its defaults\n"
       "BGP Specific commands\n"
       "Enable route-flap dampening\n"
       "Half-life time for the penalty\n"
       "Value to start reusing a route\n"
       "Value to start suppressing a route\n"
       "Maximum duration to suppress a stable route\n")

DEFSH (VTYSH_PIMD, show_ip_igmp_sources_retransmissions_cmd_vtysh, 
       "show ip igmp sources retransmissions", 
       "Show running system information\n"
       "IP information\n"
       "IGMP information\n"
       "IGMP sources information\n"
       "IGMP source retransmissions\n")

DEFSH (VTYSH_OSPFD, ospf_network_area_cmd_vtysh, 
       "network A.B.C.D/M area <A.B.C.D|(0-4294967295)>", 
       "Enable routing on an IP network\n"
       "OSPF network prefix\n"
       "Set the OSPF area ID\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_set_metric_cmd_vtysh, 
       "no set metric [(0-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "Metric value for destination routing protocol\n"
       "Metric value\n")

DEFSH (VTYSH_BGPD, ip_as_path_cmd_vtysh, 
       "ip as-path access-list WORD <deny|permit> LINE...", 
       "IP information\n"
       "BGP autonomous system path filter\n"
       "Specify an access list name\n"
       "Regular expression access list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "A regular-expression to match the BGP AS paths\n")

DEFSH (VTYSH_BGPD, no_neighbor_route_server_client_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> route-server-client", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Configure a neighbor as Route Server client\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_cmd_vtysh, 
       "ip prefix-list WORD <deny|permit> <A.B.C.D/M|any>", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Any prefix match. Same as \"0.0.0.0/0 le 32\"\n")

DEFSH (VTYSH_BGPD, set_ip_nexthop_unchanged_cmd_vtysh, 
       "set ip next-hop unchanged", 
       "Set values in destination routing protocol\n"
       "IP information\n"
       "Next hop address\n"
       "Don't modify existing Next hop address\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_redist_bgpdirect_prefixlist_cmd_vtysh, 
       "redistribute bgp-direct <ipv4|ipv6> prefix-list NAME", 
       "Redistribute from other protocol\n"
       "Redistribute from BGP directly\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "Prefix-list for filtering redistributed routes\n"
       "prefix list name\n")

DEFSH (VTYSH_ZEBRA, zebra_ptm_enable_cmd_vtysh, 
       "ptm-enable", 
       "Enable neighbor check with specified topology\n")

DEFSH (VTYSH_OSPFD, no_ospf_redistribute_instance_source_cmd_vtysh, 
       "no redistribute <ospf|table> (1-65535) [{metric (0-16777214)|metric-type (1-2)|route-map WORD}]", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table Id\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, vnc_show_responses_pfx_cmd_vtysh, 
       "show vnc responses [<A.B.C.D/M|X:X::X:X/M|YY:YY:YY:YY:YY:YY>]", 
       "Show running system information\n"
       "VNC information\n"
       "List recent query responses\n"
       "Limit output to a particular IPv4 prefix\n"
       "Limit output to a particular IPv6 prefix\n"
       "Limit output to a particular IPv6 address\n" )

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_zebra_cmd_vtysh, 
       "debug mpls ldp zebra", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "LDP zebra information\n")

DEFSH (VTYSH_BGPD, no_set_community_delete_cmd_vtysh, 
       "no set comm-list [<(1-99)|(100-500)|WORD> delete]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "set BGP community list (for deletion)\n"
       "Community-list number (standard)\n"
       "Community-list number (expanded)\n"
       "Community-list name\n"
       "Delete matching communities\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_retransmit_interval_addr_cmd_vtysh, 
       "no ip ospf retransmit-interval [(3-65535)] [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"
       "Address of interface\n")

DEFSH (VTYSH_ZEBRA, no_mpls_transit_lsp_all_cmd_vtysh, 
       "no mpls lsp (16-1048575)", 
       "Negate a command or set its defaults\n"
       "MPLS information\n"
       "Establish label switched path\n"
       "Incoming MPLS label\n")

DEFSH (VTYSH_ISISD, no_debug_isis_lsp_sched_cmd_vtysh, 
       "no debug isis lsp-sched", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS scheduling of LSP generation\n")

DEFSH (VTYSH_BGPD, neighbor_remove_private_as_all_replace_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS all replace-AS", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n"
       "Apply to all AS numbers\n"
       "Replace private ASNs with our ASN in outbound updates\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_brouter_router_cmd_vtysh, 
       "debug ospf6 border-routers router-id A.B.C.D", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug border router\n"
       "Debug specific border router\n"
       "Specify border-router's router-id\n"
      )

DEFSH (VTYSH_BGPD, clear_ip_bgp_all_cmd_vtysh, 
       "clear [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] <*|A.B.C.D|X:X::X:X|WORD|(1-4294967295)|external|peer-group WORD> [<soft [<in|out>]|in [prefix-filter]|out>]", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Clear all peers\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "BGP neighbor on interface to clear\n"
       "Clear peers with the AS number\n"
       "Clear all external peers\n"
       "Clear all members of peer-group\n"
       "BGP peer-group name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Soft reconfig inbound and outbound updates\n"
       "Send route-refresh unless using 'soft-reconfiguration inbound'\n"
       "Resend all outbound updates\n"
       "Send route-refresh unless using 'soft-reconfiguration inbound'\n"
       "Push out prefix-list ORF and do inbound soft reconfig\n"
       "Resend all outbound updates\n")

DEFSH (VTYSH_OSPFD, ospf_area_range_substitute_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> range A.B.C.D/M substitute A.B.C.D/M", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "Area range prefix\n"
       "Announce area range as another prefix\n"
       "Network prefix to be announced instead of range\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_cmd_vtysh, 
       "show ip route vrf all", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_seq_le_ge_cmd_vtysh, 
       "no ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M le (0-128) ge (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_instance_event_cmd_vtysh, 
       "no debug ospf (1-65535) event", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF event information\n")

DEFSH (VTYSH_OSPFD, pce_neighbor_cmd_vtysh, 
       "pce neighbor as (0-65535)", 
       "PCE Router Information specific commands\n"
       "Configure PCE neighbor domain AS number\n"
       "AS number of PCE neighbors\n"
       "AS number in decimal <0-65535>\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_network_cmd_vtysh, 
              "ospf network <broadcast|non-broadcast|point-to-multipoint|point-to-point>", 
              "OSPF interface commands\n"
              "Network type\n"
              "Specify OSPF broadcast multi-access network\n"
              "Specify OSPF NBMA network\n"
              "Specify OSPF point-to-multipoint network\n"
              "Specify OSPF point-to-point network\n")

DEFSH (VTYSH_BGPD, show_ip_extcommunity_list_arg_cmd_vtysh, 
       "show ip extcommunity-list <(1-500)|WORD>", 
       "Show running system information\n"
       "IP information\n"
       "List extended-community list\n"
       "Extcommunity-list number\n"
       "Extcommunity-list name\n")

DEFSH (VTYSH_OSPFD, no_pce_cap_flag_cmd_vtysh, 
       "no pce flag", 
       "Negate a command or set its defaults\n"
       "PCE Router Information specific commands\n"
       "Disable PCE capabilities\n")

DEFSH (VTYSH_ZEBRA, no_shutdown_if_cmd_vtysh, 
       "no shutdown", 
       "Negate a command or set its defaults\n"
       "Shutdown the selected interface\n")

DEFSH (VTYSH_BGPD, vnc_redistribute_protocol_cmd_vtysh, 
       "vnc redistribute <ipv4|ipv6> <bgp|bgp-direct|bgp-direct-to-nve-groups|connected|kernel|ospf|rip|static>", 
       "VNC/RFP related configuration\n"
       "Redistribute routes into VNC\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "From BGP\n"
       "From BGP without Zebra\n"
       "From BGP without Zebra,  only to configured NVE groups\n"
       "Connected interfaces\n"
       "From kernel routes\n"
       "From Open Shortest Path First (OSPF)\n"
       "From Routing Information Protocol (RIP)\n" "From Static routes\n")

DEFSH (VTYSH_RIPD, send_lifetime_duration_day_month_cmd_vtysh, 
       "send-lifetime HH:MM:SS (1-31) MONTH (1993-2035) duration (1-2147483646)", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Duration of the key\n"
       "Duration seconds\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_event_cmd_vtysh, 
       "no debug mpls ldp event", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "LDP event information\n")

DEFSH (VTYSH_NHRPD, no_debug_nhrp_cmd_vtysh, 
 "no debug nhrp " "<all|common|event|interface|kernel|route|vici>", 
 "Negate a command or set its defaults\n"
 "Disable debug messages for specific or all parts.\n"
 "NHRP information\n"
 "All messages\n" "Common messages (default)\n" "Event manager messages\n" "Interface messages\n" "Kernel messages\n" "Route messages\n" "VICI messages\n")

DEFSH (VTYSH_ISISD, no_debug_isis_upd_cmd_vtysh, 
       "no debug isis update-packets", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS Update related packets\n")

DEFSH (VTYSH_BGPD, neighbor_shutdown_msg_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> shutdown message MSG...", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Administratively shut down this neighbor\n"
       "Add a shutdown message (draft-ietf-idr-shutdown-06)\n"
       "Shutdown message\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_ip_address_prefix_list_cmd_vtysh, 
       "match ip address prefix-list WORD", 
       "Match values from routing table\n"
       "IP information\n"
       "Match address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_OSPFD, ospf_mpls_te_inter_as_cmd_vtysh, 
       "mpls-te inter-as as", 
       "MPLS-TE specific commands\n"
       "Configure MPLS-TE Inter-AS support\n"
       "AS native mode self originate INTER_AS LSA with Type 11 (as flooding scope)\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_summary_cmd_vtysh, 
       "show ipv6 route vrf all summary", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Summary of all IPv6 routes\n")

DEFSH (VTYSH_BGPD, neighbor_weight_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> weight (0-65535)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Set default weight for routes from this neighbor\n"
       "default weight\n")

DEFSH (VTYSH_ALL, no_exec_timeout_cmd_vtysh, 
       "no exec-timeout", 
       "Negate a command or set its defaults\n"
       "Set the EXEC timeout\n")

DEFSH (VTYSH_ISISD, psnp_interval_cmd_vtysh, 
       "isis psnp-interval (1-120)", 
       "IS-IS commands\n"
       "Set PSNP interval in seconds\n"
       "PSNP interval value\n")

DEFSH (VTYSH_BGPD, add_vrf_prefix_rd_label_pref_cmd_vtysh, 
      "add vrf NAME prefix <A.B.C.D/M|X:X::X:X/M> [{rd ASN:nn_or_IP-address|label (0-1048575)|preference (0-4294967295)}]", 
       "Add\n"
       "To a VRF\n"
       "VRF name\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Override configured VRF Route Distinguisher\n"
       "<as-number>:<number> or <ip-address>:<number>\n"
       "Override configured VRF label\n"
       "Label Value <0-1048575>\n"
       "Set advertised local preference\n"
       "local preference (higher=more preferred)\n")

DEFSH (VTYSH_ALL, no_access_list_exact_cmd_vtysh, 
       "no access-list WORD <deny|permit> A.B.C.D/M [exact-match]", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Exact match of the prefixes\n")

DEFSH (VTYSH_ISISD, no_spf_delay_ietf_cmd_vtysh, 
       "no spf-delay-ietf", 
       "Negate a command or set its defaults\n"
       "IETF SPF delay algorithm\n")

DEFSH (VTYSH_BGPD, set_aspath_prepend_lastas_cmd_vtysh, 
       "set as-path prepend last-as (1-10)", 
       "Set values in destination routing protocol\n"
       "Transform BGP AS_PATH attribute\n"
       "Prepend to the as-path\n"
       "Use the peer's AS-number\n"
       "Number of times to insert\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_route_match_detail_cmd_vtysh, 
       "show ipv6 ospf6 route X:X::X:X/M match detail", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Routing Table\n"
       "Specify IPv6 prefix\n"
       "Display routes which match the specified route\n"
       "Detailed information\n"
       )

DEFSH (VTYSH_ZEBRA, ipv6_route_ifname_pref_tag_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE tag (1-4294967295) (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ISISD, dynamic_hostname_cmd_vtysh, 
       "hostname dynamic", 
       "Dynamic hostname for IS-IS\n"
       "Dynamic hostname\n")

DEFSH (VTYSH_BGPD, vnc_defaults_l2rd_cmd_vtysh, 
       "l2rd <(1-255)|auto-vn>", 
       "Specify default Local Nve ID value to use in RD for L2 routes\n"
       "Fixed value 1-255\n"
       "use the low-order octet of the NVE's VN address\n")

DEFSH (VTYSH_BGPD, debug_rfapi_query_vn_un_done_cmd_vtysh, 
       "debug rfapi-dev query done vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> target <A.B.C.D|X:X::X:X>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_query_done\n"
       "rfapi_query_done\n"
       "indicate vn addr follows\n"
       "virtual network interface IPv4 address\n"
       "virtual network interface IPv6 address\n"
       "indicate xt addr follows\n"
       "underlay network interface IPv4 address\n"
       "underlay network interface IPv6 address\n"
       "indicate target follows\n"
       "Target IPv4 address\n"
       "Target IPv6 address\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_update_direct_peer_cmd_vtysh, 
       "no debug bgp updates <in|out> <A.B.C.D|X:X::X:X|WORD>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n"
       "Inbound updates\n"
       "Outbound updates\n"
       "BGP neighbor IP address to debug\n"
       "BGP IPv6 neighbor to debug\n"
       "BGP neighbor on interface to debug\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv6_metric_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct>" " metric (0-4294967295)", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Metric for redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_ZEBRA, no_link_params_delay_cmd_vtysh, 
       "no delay", 
       "Negate a command or set its defaults\n"
       "Disable Unidirectional Average,  Min & Max Link Delay on this interface\n")

DEFSH (VTYSH_ZEBRA, no_mpls_transit_lsp_out_label_cmd_vtysh, 
      "no mpls lsp (16-1048575) <A.B.C.D|X:X::X:X> <(16-1048575)|explicit-null|implicit-null>", 
      "Negate a command or set its defaults\n" "MPLS information\n"
      "Establish label switched path\n"
      "Incoming MPLS label\n"
      "IPv4 gateway address\n"
      "IPv6 gateway address\n"
      "Outgoing MPLS label\n"
      "Use Explicit-Null label\n"
      "Use Implicit-Null label\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_nht_cmd_vtysh, 
       "no debug bgp nht", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP nexthop tracking events\n")

DEFSH (VTYSH_RIPNGD, no_ripng_offset_list_ifname_cmd_vtysh, 
       "no offset-list WORD <in|out> (0-16) IFNAME", 
       "Negate a command or set its defaults\n"
       "Modify RIPng metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n"
       "Interface to match\n")

DEFSH (VTYSH_RIPD, no_debug_rip_packet_cmd_vtysh, 
       "no debug rip packet", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP packet\n")

DEFSH (VTYSH_ISISD, isis_hello_interval_cmd_vtysh, 
       "isis hello-interval (1-600)", 
       "IS-IS commands\n"
       "Set Hello interval\n"
       "Hello interval value\n"
       "Holdtime 1 seconds,  interval depends on multiplier\n")

DEFSH (VTYSH_RIPD, send_lifetime_infinite_month_day_cmd_vtysh, 
       "send-lifetime HH:MM:SS MONTH (1-31) (1993-2035) infinite", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Never expires")

DEFSH (VTYSH_ALL, ipv6_access_list_exact_cmd_vtysh, 
       "ipv6 access-list WORD <deny|permit> X:X::X:X/M [exact-match]", 
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix\n"
       "Exact match of the prefixes\n")

DEFSH (VTYSH_RIPD, no_rip_neighbor_cmd_vtysh, 
       "no neighbor A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Specify a neighbor router\n"
       "Neighbor address\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_seq_ge_cmd_vtysh, 
       "ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M ge (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_ifmtu_cmd_vtysh, 
       "no ipv6 ospf6 ifmtu", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface MTU\n"
       )

DEFSH (VTYSH_ZEBRA, ip_route_cmd_vtysh, 
       "ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_messages_recv_cmd_vtysh, 
       "no debug mpls ldp messages recv", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Received messages,  excluding periodic Keep Alives\n")

DEFSH (VTYSH_BGPD, show_vnc_queries_pfx_cmd_vtysh, 
       "show vnc queries [<A.B.C.D/M|X:X::X:X/M|YY:YY:YY:YY:YY:YY>]", 
       "Show running system information\n"
       "VNC information\n"
       "List active queries\n"
       "Limit output to a particular IPv4 prefix or address\n"
       "Limit output to a particular IPv6 prefix\n"
       "Limit output to a particualr IPV6 address\n")

DEFSH (VTYSH_BGPD, bgp_distance_cmd_vtysh, 
       "distance bgp (1-255) (1-255) (1-255)", 
       "Define an administrative distance\n"
       "BGP distance\n"
       "Distance for routes external to the AS\n"
       "Distance for routes internal to the AS\n"
       "Distance for local routes\n")

DEFSH (VTYSH_ZEBRA, show_ip_rpf_cmd_vtysh, 
       "show ip rpf", 
       "Show running system information\n"
       "IP information\n"
       "Display RPF information for multicast source\n")

DEFSH (VTYSH_OSPF6D, no_auto_cost_reference_bandwidth_cmd_vtysh, 
       "no auto-cost reference-bandwidth [(1-4294967)]", 
       "Negate a command or set its defaults\n"
       "Calculate OSPF interface cost according to bandwidth\n"
       "Use reference bandwidth method to assign OSPF cost\n"
       "The reference bandwidth in terms of Mbits per second\n")

DEFSH (VTYSH_PIMD, ip_pim_ecmp_cmd_vtysh, 
       "ip pim ecmp", 
       "IP information\n"
       "pim multicast routing\n"
       "Enable PIM ECMP \n")

DEFSH (VTYSH_ISISD, no_isis_hello_multiplier_l1_cmd_vtysh, 
       "no isis hello-multiplier [(2-100)] level-1", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set multiplier for Hello holding time\n"
       "Hello multiplier value\n"
       "Specify hello multiplier for level-1 IIHs\n")

DEFSH (VTYSH_OSPFD, ospf_compatible_rfc1583_cmd_vtysh, 
       "compatible rfc1583", 
       "OSPF compatibility list\n"
       "compatible with RFC 1583\n")

DEFSH (VTYSH_RIPD, key_string_cmd_vtysh, 
       "key-string LINE", 
       "Set key string\n"
       "The key\n")

DEFSH (VTYSH_OSPFD, ospf_area_filter_list_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> filter-list prefix WORD <in|out>", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Filter networks between OSPF areas\n"
       "Filter prefixes between OSPF areas\n"
       "Name of an IP prefix-list\n"
       "Filter networks sent to this area\n"
       "Filter networks sent from this area\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_prefix_cmd_vtysh, 
       "no ipv6 nd prefix X:X::X:X/M [<(0-4294967295)|infinite> <(0-4294967295)|infinite>] [<router-address|off-link [no-autoconfig]|no-autoconfig [off-link]>]", 
        "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Set Router Address flag\n"
       "Do not use prefix for onlink determination\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for onlink determination\n")

DEFSH (VTYSH_PIMD, ip_pim_rp_prefix_list_cmd_vtysh, 
       "ip pim rp A.B.C.D prefix-list WORD", 
       "IP information\n"
       "pim multicast routing\n"
       "Rendevous Point\n"
       "ip address of RP\n"
       "group prefix-list filter\n"
       "Name of a prefix-list\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "JavaScript Object Notation\n")

DEFSH_HIDDEN (VTYSH_BGPD, neighbor_bfd_type_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> bfd <multihop|singlehop>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Enables BFD support\n"
       "Multihop session\n"
       "Single hop session\n")

DEFSH (VTYSH_ZEBRA, link_params_admin_grp_cmd_vtysh, 
       "admin-grp BITPATTERN", 
       "Administrative group membership\n"
       "32-bit Hexadecimal value (e.g. 0xa1)\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_seq_le_cmd_vtysh, 
       "no ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M le (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_ZEBRA, ip_route_tag_label_cmd_vtysh, 
       "ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> tag (1-4294967295) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_interface_json_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> interface [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "interface information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_bgp_distance_source_cmd_vtysh, 
       "no distance (1-255) A.B.C.D/M", 
       "Negate a command or set its defaults\n"
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_prefix_first_match_cmd_vtysh, 
       "show ip prefix-list WORD A.B.C.D/M first-match", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "First matched prefix\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_cmd_vtysh, 
       "ipv6 prefix-list WORD <deny|permit> <X:X::X:X/M|any>", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")

DEFSH (VTYSH_PIMD, show_ip_igmp_groups_retransmissions_cmd_vtysh, 
       "show ip igmp groups retransmissions", 
       "Show running system information\n"
       "IP information\n"
       "IGMP information\n"
       "IGMP groups information\n"
       "IGMP group retransmissions\n")

DEFSH (VTYSH_OSPFD, ip_ospf_priority_cmd_vtysh, 
       "ip ospf priority (0-255) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Router priority\n"
       "Priority\n"
       "Address of interface")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, rmap_onmatch_goto_cmd_vtysh, 
       "on-match goto (1-65535)", 
       "Exit policy on matches\n"
       "Goto Clause number\n"
       "Number\n")

DEFSH (VTYSH_OSPFD, ip_ospf_bfd_param_cmd_vtysh, 
       "ip ospf bfd (2-255) (50-60000) (50-60000)", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Enables BFD support\n"
       "Detect Multiplier\n"
       "Required min receive interval\n"
       "Desired min transmit interval\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_protocol_cmd_vtysh, 
       "show ip route vrf all " "<kernel|connected|static|rip|ospf|isis|bgp|pim|nhrp|table|vnc>", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n""\n")

DEFSH (VTYSH_BGPD, debug_rfapi_response_omit_self_cmd_vtysh, 
       "debug rfapi-dev response-omit-self <on|off>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "Omit self in RFP responses\n"
       "filter out self from responses\n" "leave self in responses\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_map_cmd_vtysh, 
 "no " "<ip|ipv6>" " nhrp map <A.B.C.D|X:X::X:X>", 
 "Negate a command or set its defaults\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Nexthop Server configuration\n"
 "IPv4 protocol address\n"
 "IPv6 protocol address\n")

DEFSH (VTYSH_BGPD, clear_vnc_nve_all_cmd_vtysh, 
       "clear vnc nve *", 
       "clear\n"
       "VNC Information\n"
       "Clear per NVE information\n"
       "For all NVEs\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_ifname_tag_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE tag (1-4294967295) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_LDPD, ldp_no_interface_ifname_cmd_vtysh, 
       "no interface IFNAME", 
       "Negate a command or set its defaults\n"
       "Enable LDP on an interface and enter interface submode\n"
       "Interface's name\n")

DEFSH (VTYSH_BGPD, bgp_graceful_restart_stalepath_time_cmd_vtysh, 
       "bgp graceful-restart stalepath-time (1-3600)", 
       "BGP specific commands\n"
       "Graceful restart capability parameters\n"
       "Set the max time to hold onto restarting peer's stale paths\n"
       "Delay value (seconds)\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_un_vn_prefix_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> un <*|A.B.C.D|X:X::X:X> vn <*|A.B.C.D|X:X::X:X> prefix <*|A.B.C.D/M|X:X::X:X/M> prefix <*|A.B.C.D/M|X:X::X:X/M>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "VN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_OSPFD, no_ospf_default_information_originate_cmd_vtysh, 
       "no default-information originate [<always|metric (0-16777214)|metric-type (1-2)|route-map WORD>]", 
       "Negate a command or set its defaults\n"
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_OSPFD, no_ospf_compatible_rfc1583_cmd_vtysh, 
       "no compatible rfc1583", 
       "Negate a command or set its defaults\n"
       "OSPF compatibility list\n"
       "compatible with RFC 1583\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, set_ipv6_nexthop_local_cmd_vtysh, 
       "set ipv6 next-hop local X:X::X:X", 
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "IPv6 local address\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_ipv4_ttl_security_hops_hops_cmd_vtysh, 
       "neighbor A.B.C.D ttl-security hops (1-254)", 
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "LDP ttl security check\n"
       "IP hops\n"
       "maximum number of hops\n")

DEFSH (VTYSH_PIMD, debug_msdp_packets_cmd_vtysh, 
       "debug msdp packets", 
       "Debugging functions (see also 'undebug')\n"
       "MSDP protocol activity\n"
       "MSDP protocol packets\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_managed_config_flag_cmd_vtysh, 
       "ipv6 nd managed-config-flag", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Managed address configuration flag\n")

DEFSH (VTYSH_BGPD, vnc_vrf_policy_label_cmd_vtysh, 
       "label (0-1048575)", 
       "Default label value for VRF\n"
       "Label Value <0-1048575>\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_homeagent_config_flag_cmd_vtysh, 
       "no ipv6 nd home-agent-config-flag", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent configuration flag\n")

DEFSH (VTYSH_OSPFD, show_debugging_ospf_cmd_vtysh, 
       "show debugging ospf", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_instance_zebra_cmd_vtysh, 
       "no debug ospf (1-65535) zebra [<interface|redistribute>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Zebra information\n"
       "Zebra interface\n"
       "Zebra redistribute\n")

DEFSH (VTYSH_BGPD, no_match_local_pref_cmd_vtysh, 
       "no match local-preference [(0-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match local preference of route\n"
       "Local preference value\n")

DEFSH (VTYSH_BGPD, neighbor_remove_private_as_replace_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS replace-AS", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n"
       "Replace private ASNs with our ASN in outbound updates\n")

DEFSH (VTYSH_ZEBRA, no_set_src_cmd_vtysh, 
       "no set src [<A.B.C.D|X:X::X:X>]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "Source address for route\n"
       "IPv4 address\n"
       "IPv6 address\n")

DEFSH (VTYSH_ISISD, no_csnp_interval_l1_cmd_vtysh, 
       "no isis csnp-interval [(1-600)] level-1", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set CSNP interval in seconds\n"
       "CSNP interval value\n"
       "Specify interval for level-1 CSNPs\n")

DEFSH (VTYSH_BGPD, no_set_aggregator_as_cmd_vtysh, 
       "no set aggregator as [(1-4294967295) A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP aggregator attribute\n"
       "AS number of aggregator\n"
       "AS number\n"
       "IP address of aggregator\n")

DEFSH (VTYSH_RIPD, rip_route_cmd_vtysh, 
       "route A.B.C.D/M", 
       "RIP static route configuration\n"
       "IP prefix <network>/<length>\n")

DEFSH (VTYSH_BGPD, set_aggregator_as_cmd_vtysh, 
       "set aggregator as (1-4294967295) A.B.C.D", 
       "Set values in destination routing protocol\n"
       "BGP aggregator attribute\n"
       "AS number of aggregator\n"
       "AS number\n"
       "IP address of aggregator\n")

DEFSH (VTYSH_ISISD, isis_priority_cmd_vtysh, 
       "isis priority (0-127)", 
       "IS-IS commands\n"
       "Set priority for Designated Router election\n"
       "Priority value\n")

DEFSH (VTYSH_BGPD, bgp_listen_limit_cmd_vtysh, 
       "bgp listen limit (1-5000)", 
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "maximum number of BGP Dynamic Neighbors that can be created\n"
       "Configure Dynamic Neighbors listen limit value\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_ra_interval_cmd_vtysh, 
       "ipv6 nd ra-interval (1-1800)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n"
       "Router Advertisement interval in seconds\n")

DEFSH (VTYSH_PIMD, no_debug_pim_cmd_vtysh, 
       "no debug pim", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n")

DEFSH (VTYSH_OSPFD, no_ospf_abr_type_cmd_vtysh, 
       "no ospf abr-type <cisco|ibm|shortcut|standard>", 
       "Negate a command or set its defaults\n"
       "OSPF specific commands\n"
       "Set OSPF ABR type\n"
       "Alternative ABR,  cisco implementation\n"
       "Alternative ABR,  IBM implementation\n"
       "Shortcut ABR\n"
       "Standard ABR\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_neighbors_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [<ipv4|ipv6|vpnv4 <all|rd ASN:nn_or_IP-address:nn>>] neighbors [<A.B.C.D|X:X::X:X|WORD>] [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\n"
       "Address Family\n"
       "Address Family\n"
       "Display information about all VPNv4 NLRIs\n"
       "Display information for a route distinguisher\n"
       "VPN Route Distinguisher\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n"
       "Neighbor on BGP configured interface\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_cmd_vtysh, 
       "no ip prefix-list WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_ALL, vty_ipv6_access_class_cmd_vtysh, 
       "ipv6 access-class WORD", 
       "IPv6 information\n"
       "Filter connections based on an IP access list\n"
       "IPv6 access list\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_flags_cmd_vtysh, 
       "no ip route A.B.C.D/M <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Tag of this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_NHRPD, no_nhrp_nflog_group_cmd_vtysh, 
 "no nhrp nflog-group [(1-65535)]", 
 "Negate a command or set its defaults\n"
 "Next Hop Resolution Protocol functions\n"
 "Specify NFLOG group number\n"
 "NFLOG group number\n")

DEFSH (VTYSH_ALL, access_list_extended_any_host_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip any host A.B.C.D", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "A single destination host\n"
       "Destination address\n")

DEFSH (VTYSH_BGPD, match_ip_route_source_prefix_list_cmd_vtysh, 
       "match ip route-source prefix-list WORD", 
       "Match values from routing table\n"
       "IP information\n"
       "Match advertising source address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_BGPD, clear_vnc_prefix_un_cmd_vtysh, 
       "clear vnc prefix <*|A.B.C.D/M|X:X::X:X/M> un <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_debug_messages_cmd_vtysh, 
       "ip irdp debug messages", 
       "IP information\n"
       "ICMP Router discovery debug Averts. and Solicits (short)\n"
       "IRDP debugging options\n"
       "Enable debugging for IRDP messages\n")

DEFSH (VTYSH_ISISD, no_set_attached_bit_cmd_vtysh, 
       "no set-attached-bit", 
       "Negate a command or set its defaults\n"
       "Reset attached bit\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_cmd_vtysh, 
       "no ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Tag of this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_prefix_longer_cmd_vtysh, 
       "show ip prefix-list WORD A.B.C.D/M longer", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Lookup longer prefix\n")

DEFSH (VTYSH_ISISD, no_isis_passwd_cmd_vtysh, 
       "no isis password [<md5|clear> WORD]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Configure the authentication password for a circuit\n"
       "HMAC-MD5 authentication\n"
       "Cleartext password\n"
       "Circuit password\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_ipv6_address_cmd_vtysh, 
       "match ipv6 address WORD", 
       "Match values from routing table\n"
       "IPv6 information\n"
       "Match IPv6 address of route\n"
       "IPv6 access-list name\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_ip_address_prefix_list_cmd_vtysh, 
       "no match ip address prefix-list [WORD]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_cmd_vtysh, 
       "show ipv6 prefix-list", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_rd_cmd_vtysh, 
       "rd ASN:nn_or_IP-address:nn", 
       "Specify route distinguisher\n"
       "Route Distinguisher (<as-number>:<number> | <ip-address>:<number> | auto:vn:<number> )\n")

DEFSH (VTYSH_ZEBRA, link_params_use_bw_cmd_vtysh, 
       "use-bw BANDWIDTH", 
       "Unidirectional Utilised Bandwidth\n"
       "Bytes/second (IEEE floating point format)\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_messages_recv_all_cmd_vtysh, 
       "no debug mpls ldp messages recv all", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Messages\n"
       "Received messages,  excluding periodic Keep Alives\n"
       "Received messages,  including periodic Keep Alives\n")

DEFSH (VTYSH_BGPD, match_ip_route_source_cmd_vtysh, 
       "match ip route-source <(1-199)|(1300-2699)|WORD>", 
       "Match values from routing table\n"
       "IP information\n"
       "Match advertising source address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP standard access-list name\n")

DEFSH (VTYSH_BGPD|VTYSH_BGPD, show_ip_bgp_updgrps_adj_s_cmd_vtysh, 
       "show [ip] bgp update-groups SUBGROUP-ID <advertise-queue|advertised-routes|packet-queue>", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Detailed info about dynamic update groups\n"
       "Specific subgroup to display info for\n"
       "Advertisement queue\n"
       "Announced routes\n"
       "Packet queue\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_attr_info_cmd_vtysh, 
       "show [ip] bgp attribute-info", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "List all bgp attribute information\n")

DEFSH (VTYSH_PIMD, no_ip_msdp_mesh_group_member_cmd_vtysh, 
       "no ip msdp mesh-group WORD member A.B.C.D", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Configure multicast source discovery protocol\n"
       "Delete MSDP mesh-group member\n"
       "mesh group name\n"
       "mesh group member\n"
       "peer ip address\n")

DEFSH (VTYSH_BGPD, no_bgp_network_mask_natural_cmd_vtysh, 
       "no network A.B.C.D [<backdoor|route-map WORD>]", 
       "Negate a command or set its defaults\n"
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Specify a BGP backdoor route\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_BGPD, no_dump_bgp_all_cmd_vtysh, 
       "no dump bgp <all|all-et|updates|updates-et|routes-mrt> [PATH [INTERVAL]]", 
       "Negate a command or set its defaults\n"
       "Stop dump packet\n"
       "Stop BGP packet dump\n"
       "Stop dump process all\n"
       "Stop dump process all-et\n"
       "Stop dump process updates\n"
       "Stop dump process updates-et\n"
       "Stop dump process route-mrt\n"
       "Output filename\n"
       "Interval of output\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_prefix_cmd_vtysh, 
       "show ip route [vrf NAME] A.B.C.D/M", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n")

DEFSH (VTYSH_RIPD, rip_distance_source_cmd_vtysh, 
       "distance (1-255) A.B.C.D/M", 
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n")

DEFSH (VTYSH_ISISD, no_dynamic_hostname_cmd_vtysh, 
       "no hostname dynamic", 
       "Negate a command or set its defaults\n"
       "Dynamic hostname for IS-IS\n"
       "Dynamic hostname\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_update_direct_cmd_vtysh, 
       "no debug bgp updates <in|out>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n"
       "Inbound updates\n"
       "Outbound updates\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_mpls_te_router_cmd_vtysh, 
       "show ip ospf mpls-te router", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "MPLS-TE information\n"
       "MPLS-TE Router parameters\n")

DEFSH (VTYSH_BGPD, no_bgp_disable_connected_route_check_cmd_vtysh, 
       "no bgp disable-ebgp-connected-route-check", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Disable checking if nexthop is connected on ebgp sessions\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_vn_un_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> vn <*|A.B.C.D|X:X::X:X> un <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "Virtual network identifier\n"
       "VN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_database_max_cmd_vtysh, 
       "show ip ospf (1-65535) database <max-age|self-originate>", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Database summary\n"
       "LSAs in MaxAge list\n"
       "Self-originated link states\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_life_cost_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> lifetime (1-4294967295) cost (0-255)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_BGPD, no_ip_lcommunity_list_expanded_cmd_vtysh, 
       "no ip large-community-list (100-500) <deny|permit> LINE...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a large community list entry\n"
       "Large Community list number (expanded)\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "An ordered list as a regular-expression\n")

DEFSH (VTYSH_ZEBRA, link_params_res_bw_cmd_vtysh, 
       "res-bw BANDWIDTH", 
       "Unidirectional Residual Bandwidth\n"
       "Bytes/second (IEEE floating point format)\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_description_comment_cmd_vtysh, 
       "no ipv6 prefix-list WORD description LINE...", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_protocol_cmd_vtysh, 
       "no ipv6 protocol " "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|any>" " [route-map ROUTE-MAP]", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Stop filtering IPv6 routing info between zebra and protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_LDPD, ldp_pw_status_disable_cmd_vtysh, 
       "pw-status disable", 
       "Configure PW status\n"
       "Disable PW status\n")

DEFSH (VTYSH_PIMD, interface_no_ip_igmp_cmd_vtysh, 
       "no ip igmp", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable IGMP operation\n")

DEFSH (VTYSH_BGPD, debug_bgp_vnc_cmd_vtysh, 
       "debug bgp vnc <rfapi-query|import-bi-attach|import-del-remote|verbose>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "VNC information\n"
       "rfapi query handling\n"
       "import BI atachment\n"
       "import delete remote routes\n"
       "verbose logging\n")

DEFSH (VTYSH_RIPD, no_debug_rip_packet_direct_cmd_vtysh, 
       "no debug rip packet <recv|send>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP packet\n"
       "RIP option set for receive packet\n"
       "RIP option set for send packet\n")

DEFSH (VTYSH_BGPD, show_debugging_bgp_cmd_vtysh, 
       "show debugging bgp", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n")

DEFSH (VTYSH_BGPD, neighbor_remove_private_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_other_config_flag_cmd_vtysh, 
       "ipv6 nd other-config-flag", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Other statefull configuration flag\n")

DEFSH (VTYSH_LDPD, ldp_no_mpls_ldp_cmd_vtysh, 
       "no mpls ldp", 
       "Negate a command or set its defaults\n"
       "Global MPLS configuration subcommands\n"
       "Label Distribution Protocol\n")

DEFSH (VTYSH_RIPNGD, ripng_default_metric_cmd_vtysh, 
       "default-metric (1-16)", 
       "Set a metric of redistribute routes\n"
       "Default metric\n")

DEFSH (VTYSH_LDPD, ldp_no_address_family_ipv6_cmd_vtysh, 
       "no address-family ipv6", 
       "Negate a command or set its defaults\n"
       "Configure Address Family and its parameters\n"
       "IPv6\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_network_id_cmd_vtysh, 
 "no " "<ip|ipv6>" " nhrp network-id [(1-4294967295)]", 
 "Negate a command or set its defaults\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Enable NHRP and specify network-id\n"
 "System local ID to specify interface group\n")

DEFSH (VTYSH_BGPD, neighbor_filter_list_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> filter-list WORD <in|out>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_allow_martians_cmd_vtysh, 
       "no debug bgp allow-martians", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP allow martian next hops\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_update_prefix_cmd_vtysh, 
       "no debug bgp updates prefix <A.B.C.D/M|X:X::X:X/M>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n"
       "Specify a prefix to debug\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_PIMD, show_ip_pim_neighbor_cmd_vtysh, 
       "show ip pim neighbor [detail|WORD] [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM neighbor information\n"
       "Detailed output\n"
       "Name of interface or neighbor\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, ip_protocol_nht_rmap_cmd_vtysh, 
       "ip nht " "<kernel|connected|static|rip|ospf|isis|bgp|pim|nhrp|table|vnc|any>" " route-map ROUTE-MAP", 
       "IP information\n"
       "Filter Next Hop tracking route resolution\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_ifname_flags_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M [from X:X::X:X/M] X:X::X:X INTERFACE <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ZEBRA, no_config_table_cmd_vtysh, 
       "no table [TABLENO]", 
       "Negate a command or set its defaults\n"
       "Configure target kernel routing table\n"
       "TABLE integer\n")

DEFSH (VTYSH_ZEBRA, show_zebra_client_summary_cmd_vtysh, 
       "show zebra client summary", 
       "Show running system information\n"
       "Zebra information brief\n"
       "Client information brief\n"
       "Brief Summary\n")

DEFSH (VTYSH_OSPFD, ospf_neighbor_poll_interval_cmd_vtysh, 
       "neighbor A.B.C.D poll-interval (1-65535) [priority (0-255)]", 
       "Specify neighbor router\n"
       "Neighbor IP address\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n"
       "OSPF priority of non-broadcast neighbor\n"
       "Priority\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_mroute_cmd_vtysh, 
       "show ipv6 mroute [vrf NAME]", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 Multicast routing table\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_authentication_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> authentication", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Enable authentication\n")

DEFSH (VTYSH_RIPD, send_lifetime_infinite_day_month_cmd_vtysh, 
       "send-lifetime HH:MM:SS (1-31) MONTH (1993-2035) infinite", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Never expires")

DEFSH (VTYSH_BGPD, no_match_peer_cmd_vtysh, 
       "no match peer [<local|A.B.C.D|X:X::X:X>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match peer address\n"
       "Static or Redistributed routes\n"
       "IP address of peer\n"
       "IPv6 address of peer\n")

DEFSH (VTYSH_LDPD, ldp_ttl_security_disable_cmd_vtysh, 
       "ttl-security disable", 
       "LDP ttl security check\n"
       "Disable ttl security\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_description_comment_cmd_vtysh, 
       "no ip prefix-list WORD description LINE...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, rmap_call_cmd_vtysh, 
       "call WORD", 
       "Jump to another Route-Map after match+set\n"
       "Target route-map name\n")

DEFSH (VTYSH_OSPFD, ospf_area_import_list_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> import-list NAME", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Set the filter for networks from other areas announced to the specified one\n"
       "Name of the access-list\n")

DEFSH (VTYSH_PIMD, show_ip_pim_local_membership_cmd_vtysh, 
       "show ip pim local-membership [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface local-membership\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ALL, ipv6_access_list_remark_cmd_vtysh, 
       "ipv6 access-list WORD remark LINE...", 
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_neighbor_json_cmd_vtysh, 
       "show mpls ldp neighbor json", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Neighbor information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, bgp_network_mask_natural_backdoor_cmd_vtysh, 
       "network A.B.C.D backdoor", 
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Specify a BGP backdoor route\n")

DEFSH (VTYSH_BGPD, neighbor_description_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> description LINE...", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Neighbor specific description\n"
       "Up to 80 characters describing this neighbor\n")

DEFSH (VTYSH_ISISD, no_ip_router_isis_cmd_vtysh, 
       "no <ip|ipv6> router isis WORD", 
       "Negate a command or set its defaults\n"
       "Interface Internet Protocol config commands\n"
       "IP router interface commands\n"
       "IP router interface commands\n"
       "IS-IS Routing for IP\n"
       "Routing process tag\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_discovery_json_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> discovery json", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "Discovery Hello Information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPNGD, ripng_offset_list_cmd_vtysh, 
       "offset-list WORD <in|out> (0-16)", 
       "Modify RIPng metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n")

DEFSH (VTYSH_BGPD, vnc_defaults_rt_export_cmd_vtysh, 
       "rt export RTLIST...", 
       "Configure default route targets\n"
       "Export filter\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_BGPD, neighbor_remote_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> remote-as <(1-4294967295)|internal|external>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Specify a BGP neighbor\n"
       "AS number\n"
       "Internal BGP peer\n"
       "External BGP peer\n")

DEFSH (VTYSH_PIMD, show_ip_pim_group_type_cmd_vtysh, 
       "show ip pim group-type A.B.C.D [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "multicast group type\n"
       "group address\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, link_params_ava_bw_cmd_vtysh, 
       "ava-bw BANDWIDTH", 
       "Unidirectional Available Bandwidth\n"
       "Bytes/second (IEEE floating point format)\n")

DEFSH (VTYSH_OSPFD, ospf_area_range_cost_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> range A.B.C.D/M cost (0-16777215)", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "Area range prefix\n"
       "User specified metric for this range\n"
       "Advertised metric for this range\n")

DEFSH (VTYSH_NHRPD, if_nhrp_flags_cmd_vtysh, 
 "<ip|ipv6>" " nhrp <shortcut|redirect>", 
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Allow shortcut establishment\n"
 "Send redirect notifications\n")

DEFSH (VTYSH_BGPD, clear_bgp_instance_ipv6_safi_prefix_cmd_vtysh, 
       "clear [ip] bgp <view|vrf> WORD ipv6 ""<unicast|multicast|vpn>"" prefix X:X::X:X/M", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Clear bestpath and re-advertise\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_ISISD, net_cmd_vtysh, 
       "net WORD", 
       "A Network Entity Title for this process (OSI only)\n"
       "XX.XXXX. ... .XXX.XX  Network entity title (NET)\n")

DEFSH (VTYSH_BGPD, no_ip_lcommunity_list_standard_cmd_vtysh, 
       "no ip large-community-list (1-99) <deny|permit> AA:AA:NN...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a large community list entry\n"
       "Large Community list number (standard)\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "large community in 'aa:bb:cc' format\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_kernel_msgdump_cmd_vtysh, 
       "no debug zebra kernel msgdump [<recv|send>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra between kernel interface\n"
       "Dump raw netlink messages,  sent and received\n"
       "Dump raw netlink messages received\n"
       "Dump raw netlink messages sent\n")

DEFSH (VTYSH_LDPD, ldp_clear_mpls_ldp_neighbor_cmd_vtysh, 
       "clear mpls ldp neighbor", 
       "Reset functions\n"
       "Reset MPLS statistical information\n"
       "Clear LDP state\n"
       "Clear LDP neighbor sessions\n")

DEFSH (VTYSH_RIPD, send_lifetime_month_day_month_day_cmd_vtysh, 
       "send-lifetime HH:MM:SS MONTH (1-31) (1993-2035) HH:MM:SS MONTH (1-31) (1993-2035)", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Month of the year to expire\n"
       "Day of th month to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_ALL, access_list_standard_nomask_cmd_vtysh, 
       "access-list <(1-99)|(1300-1999)> <deny|permit> A.B.C.D", 
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n")

DEFSH (VTYSH_BGPD, no_bgp_maxmed_onstartup_cmd_vtysh, 
       "no bgp max-med on-startup [(5-86400) [(0-4294967294)]]", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Advertise routes with max-med\n"
       "Effective on a startup\n"
       "Time (seconds) period for max-med\n"
       "Max MED value to be used\n")

DEFSH (VTYSH_BGPD, neighbor_timers_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> timers (0-65535) (0-65535)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP per neighbor timers\n"
       "Keepalive interval\n"
       "Holdtime\n")

DEFSH (VTYSH_RIPNGD, ripng_redistribute_ripng_cmd_vtysh, 
       "redistribute ripng", 
       "Redistribute information from another routing protocol\n"
       "RIPng route\n")

DEFSH (VTYSH_OSPFD, ip_ospf_authentication_key_addr_cmd_vtysh, 
       "ip ospf authentication-key AUTH_KEY [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Authentication password (key)\n"
       "The OSPF password (key)\n"
       "Address of interface")

DEFSH (VTYSH_OSPFD, no_capability_opaque_cmd_vtysh, 
       "no capability opaque", 
       "Negate a command or set its defaults\n"
       "Enable specific OSPF feature\n"
       "Opaque LSA\n")

DEFSH (VTYSH_OSPFD, no_ospf_neighbor_poll_cmd_vtysh, 
       "no neighbor A.B.C.D poll-interval (1-65535) [priority (0-255)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor IP address\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n"
       "Neighbor Priority\n"
       "Priority\n")

DEFSH (VTYSH_BGPD, neighbor_passive_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> passive", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Don't send open messages to this neighbor\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_prefix_cmd_vtysh, 
       "show ipv6 route [vrf NAME] X:X::X:X/M", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_ZEBRA, ip_multicast_mode_cmd_vtysh, 
       "ip multicast rpf-lookup-mode <urib-only|mrib-only|mrib-then-urib|lower-distance|longer-prefix>", 
       "IP information\n"
       "Multicast options\n"
       "RPF lookup behavior\n"
       "Lookup in unicast RIB only\n"
       "Lookup in multicast RIB only\n"
       "Try multicast RIB first,  fall back to unicast RIB\n"
       "Lookup both,  use entry with lower distance\n"
       "Lookup both,  use entry with longer prefix\n")

DEFSH (VTYSH_ISISD, clear_isis_neighbor_cmd_vtysh, 
       "clear isis neighbor", 
       "Reset functions\n"
       "Reset ISIS network information\n"
       "Reset ISIS neighbor adjacencies\n")

DEFSH (VTYSH_BGPD, no_bgp_default_subgroup_pkt_queue_max_cmd_vtysh, 
       "no bgp default subgroup-pkt-queue-max [(20-100)]", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "subgroup-pkt-queue-max\n"
       "Configure subgroup packet queue max\n")

DEFSH (VTYSH_BGPD, no_bgp_network_mask_cmd_vtysh, 
       "no network A.B.C.D mask A.B.C.D [<backdoor|route-map WORD>]", 
       "Negate a command or set its defaults\n"
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Network mask\n"
       "Network mask\n"
       "Specify a BGP backdoor route\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_PIMD, interface_no_ip_pim_sm_cmd_vtysh, 
       "no ip pim sm", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "PIM information\n"
       "Enable PIM SM operation\n")

DEFSH (VTYSH_PIMD, show_debugging_pim_cmd_vtysh, 
       "show debugging pim", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM information\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_sequence_number_cmd_vtysh, 
       "ip prefix-list sequence-number", 
       "IP information\n"
       "Build a prefix list\n"
       "Include/exclude sequence numbers in NVGEN\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_advertise_prefix_list_cmd_vtysh, 
       "ipv6 ospf6 advertise prefix-list WORD", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Advertising options\n"
       "Filter prefix using prefix-list\n"
       "Prefix list name\n"
       )

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_tag_cmd_vtysh, 
       "show ip route vrf all tag (1-4294967295)", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Show only routes with tag\n"
       "Tag value\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_no_l2rd_cmd_vtysh, 
       "no l2rd", 
       "Negate a command or set its defaults\n"
       "Specify default Local Nve ID value to use in RD for L2 routes\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_seq_cmd_vtysh, 
       "no ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> <X:X::X:X/M|any>", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")

DEFSH (VTYSH_ZEBRA, no_ip_forwarding_cmd_vtysh, 
       "no ip forwarding", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Turn off IP forwarding")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_binding_detail_json_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> binding detail [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "Label Information Base (LIB) information\n"
       "Show detailed information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_LDPD, ldp_no_dual_stack_cisco_interop_cmd_vtysh, 
       "no dual-stack cisco-interop", 
       "Negate a command or set its defaults\n"
       "Configure dual stack parameters\n"
       "Use Cisco non-compliant format to send and interpret the Dual-Stack capability TLV\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_int_cmd_vtysh, 
       "show ip ospf neighbor IFNAME [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "Interface name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_zebra_sendrecv_cmd_vtysh, 
       "debug ospf6 zebra [<send|recv>]", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug connection between zebra\n"
       "Debug Sending zebra\n"
       "Debug Receiving zebra\n"
      )

DEFSH (VTYSH_BGPD, neighbor_maximum_prefix_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix (1-4294967295)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_filter_list_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> filter-list prefix WORD <in|out>", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Filter networks between OSPF areas\n"
       "Filter prefixes between OSPF areas\n"
       "Name of an IP prefix-list\n"
       "Filter networks sent to this area\n"
       "Filter networks sent from this area\n")

DEFSH (VTYSH_ZEBRA, show_table_cmd_vtysh, 
       "show table", 
       "Show running system information\n"
       "default routing table to use for all clients\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_spf_database_cmd_vtysh, 
       "no debug ospf6 spf database", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug SPF Calculation\n"
       "Quit Logging number of LSAs at SPF Calculation time\n"
      )

DEFSH (VTYSH_ZEBRA, multicast_cmd_vtysh, 
       "multicast", 
       "Set multicast flag to interface\n")

DEFSH (VTYSH_LDPD, ldp_label_remote_accept_from_acl_cmd_vtysh, 
       "label remote accept from <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "Neighbor from whom to accept label advertisement\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_cmd_vtysh, 
       "show ip route vrf NAME [json]", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD, no_rip_distance_cmd_vtysh, 
       "no distance (1-255)", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "Distance value\n")

DEFSH (VTYSH_PIMD, interface_ip_igmp_join_cmd_vtysh, 
       "ip igmp join A.B.C.D A.B.C.D", 
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP join multicast group\n"
       "Multicast group address\n"
       "Source address\n")

DEFSH (VTYSH_RIPD, accept_lifetime_day_month_day_month_cmd_vtysh, 
       "accept-lifetime HH:MM:SS (1-31) MONTH (1993-2035) HH:MM:SS (1-31) MONTH (1993-2035)", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Day of th month to expire\n"
       "Month of the year to expire\n"
       "Year to expire\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_address_addr_cmd_vtysh, 
       "neighbor address <A.B.C.D|X:X::X:X>", 
       "Remote endpoint configuration\n"
       "Specify the IPv4 or IPv6 address of the remote endpoint\n"
       "IPv4 address\n"
       "IPv6 address\n")

DEFSH (VTYSH_OSPFD, no_ospf_router_id_cmd_vtysh, 
       "no ospf router-id [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "OSPF specific commands\n"
       "router-id for the OSPF process\n"
       "OSPF router-id in IP address format\n")

DEFSH (VTYSH_PIMD, debug_pim_packetdump_send_cmd_vtysh, 
       "debug pim packet-dump send", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM packet dump\n"
       "Dump sent packets\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_cmd_vtysh, 
       "show ipv6 route [vrf NAME] [json]", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Output JSON\n")

DEFSH (VTYSH_OSPF6D, area_export_list_cmd_vtysh, 
       "area A.B.C.D export-list NAME", 
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "Set the filter for networks announced to other areas\n"
       "Name of the acess-list\n")

DEFSH (VTYSH_ZEBRA, no_link_params_res_bw_cmd_vtysh, 
       "no res-bw", 
       "Negate a command or set its defaults\n"
       "Disable Unidirectional Residual Bandwidth on this interface\n")

DEFSH (VTYSH_ZEBRA, link_params_inter_as_cmd_vtysh, 
       "neighbor A.B.C.D as (1-4294967295)", 
       "Configure remote ASBR information (Neighbor IP address and AS number)\n"
       "Remote IP address in dot decimal A.B.C.D\n"
       "Remote AS number\n"
       "AS number in the range <1-4294967295>\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_detail_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor detail [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "detail of all neighbors\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_mtu_ignore_cmd_vtysh, 
       "no ipv6 ospf6 mtu-ignore", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Disable MTU mismatch detection on this interface\n"
       )

DEFSH (VTYSH_OSPFD, no_ip_ospf_dead_interval_cmd_vtysh, 
       "no ip ospf dead-interval [<(1-65535)|minimal hello-multiplier (1-10)> [A.B.C.D]]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval time after which a neighbor is declared down\n"
       "Seconds\n"
       "Address of interface")

DEFSH (VTYSH_BGPD, no_bgp_listen_limit_cmd_vtysh, 
       "no bgp listen limit [(1-5000)]", 
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "unset maximum number of BGP Dynamic Neighbors that can be created\n"
       "Configure Dynamic Neighbors listen limit value to default\n"
       "Configure Dynamic Neighbors listen limit value\n")

DEFSH (VTYSH_RIPD, rip_redistribute_type_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_BGPD, no_set_ipv6_nexthop_prefer_global_cmd_vtysh, 
       "no set ipv6 next-hop prefer-global", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "Prefer global over link-local if both exist\n")

DEFSH (VTYSH_BGPD, ip_lcommunity_list_standard_cmd_vtysh, 
       "ip large-community-list (1-99) <deny|permit>", 
       "IP information\n"
       "Add a large community list entry\n"
       "Large Community list number (standard)\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "large community in 'aa:bb:cc' format\n")

DEFSH (VTYSH_BGPD, no_bgp_distance_cmd_vtysh, 
       "no distance bgp [(1-255) (1-255) (1-255)]", 
       "Negate a command or set its defaults\n"
       "Define an administrative distance\n"
       "BGP distance\n"
       "Distance for routes external to the AS\n"
       "Distance for routes internal to the AS\n"
       "Distance for local routes\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_all_neighbor_routes_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn all neighbors A.B.C.D routes [json]", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information about all EVPN NLRIs\n"
      "Detailed information on TCP and BGP neighbor connections\n"
      "Neighbor to display information about\n"
      "Display routes learned from neighbor\n" "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_homeagent_lifetime_cmd_vtysh, 
       "no ipv6 nd home-agent-lifetime [(0-65520)]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent lifetime\n"
       "Home Agent lifetime in seconds (0 to track ra-lifetime)\n")

DEFSH (VTYSH_RIPNGD, show_ipv6_ripng_status_cmd_vtysh, 
       "show ipv6 ripng status", 
       "Show running system information\n"
       "IPv6 information\n"
       "Show RIPng routes\n"
       "IPv6 routing protocol process parameters and statistics\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_addr_cmd_vtysh, 
       "show ip route [vrf NAME] A.B.C.D", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Network in the IP routing table to display\n")

DEFSH (VTYSH_ZEBRA, no_ip_nht_default_route_cmd_vtysh, 
       "no ip nht resolve-via-default", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Filter Next Hop tracking route resolution\n"
       "Resolve via default route\n")

DEFSH (VTYSH_PIMD, ip_pim_register_suppress_cmd_vtysh, 
       "ip pim register-suppress-time (5-60000)", 
       "IP information\n"
       "pim multicast routing\n"
       "Register Suppress Timer\n"
       "Seconds\n")

DEFSH (VTYSH_BGPD, debug_rfapi_open_cmd_vtysh, 
       "debug rfapi-dev open vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_open\n"
       "indicate vn addr follows\n"
       "virtual network interface IPv4 address\n"
       "virtual network interface IPv6 address\n"
       "indicate xt addr follows\n"
       "underlay network interface IPv4 address\n"
       "underlay network interface IPv6 address\n")

DEFSH (VTYSH_OSPFD, ip_ospf_dead_interval_minimal_addr_cmd_vtysh, 
       "ip ospf dead-interval minimal hello-multiplier (1-10) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval time after which a neighbor is declared down\n"
       "Minimal 1s dead-interval with fast sub-second hellos\n"
       "Hello multiplier factor\n"
       "Number of Hellos to send each second\n"
       "Address of interface\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_prefix_longer_cmd_vtysh, 
       "show ipv6 route vrf all X:X::X:X/M longer-prefixes", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "IPv6 prefix\n"
       "Show route matching the specified Network/Mask pair only\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_for_acl_cmd_vtysh, 
       "label local advertise for <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ISISD, set_overload_bit_cmd_vtysh, 
       "set-overload-bit", 
       "Set overload bit to avoid any transit traffic\n"
       "Set overload bit\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_router_id_old_cmd_vtysh, 
              "router-id A.B.C.D", 
              "router-id for the OSPF process\n"
              "OSPF router-id in IP address format\n")

DEFSH (VTYSH_PIMD, show_ip_pim_secondary_cmd_vtysh, 
       "show ip pim secondary", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM neighbor addresses\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_bfd_cmd_vtysh, 
       "no ipv6 ospf6 bfd", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Disables BFD support\n"
       )

DEFSH (VTYSH_BGPD, no_neighbor_set_peer_group_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> peer-group WORD", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Member of the peer-group\n"
       "Peer-group name\n")

DEFSH (VTYSH_BGPD, no_auto_summary_cmd_vtysh, 
       "no auto-summary", 
       "Negate a command or set its defaults\n"
       "Enable automatic network number summarization\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_abr_cmd_vtysh, 
       "debug ospf6 abr", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 ABR function\n"
      )

DEFSH (VTYSH_PIMD, ip_msdp_mesh_group_source_cmd_vtysh, 
       "ip msdp mesh-group WORD source A.B.C.D", 
       "IP information\n"
       "Configure multicast source discovery protocol\n"
       "Configure MSDP mesh-group\n"
       "mesh group name\n"
       "mesh group local address\n"
       "source ip address for the TCP connection\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_un_vn_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> un <*|A.B.C.D|X:X::X:X> vn <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "VN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_OSPFD, ospf_redistribute_instance_source_cmd_vtysh, 
       "redistribute <ospf|table> (1-65535) [{metric (0-16777214)|metric-type (1-2)|route-map WORD}]", 
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, vnc_nve_export_prefixlist_cmd_vtysh, 
       "vnc export <bgp|zebra> <ipv4|ipv6> prefix-list NAME", 
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "Filters,  used in 'registering-nve' export mode\n"
       "IPv4 prefixes\n"
       "IPv6 prefixes\n"
       "Prefix-list for filtering exported routes\n" "Prefix list name\n")

DEFSH (VTYSH_ISISD, no_isis_priority_l2_cmd_vtysh, 
       "no isis priority [(0-127)] level-2", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set priority for Designated Router election\n"
       "Priority value\n"
       "Specify priority for level-2 routing\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_instance_neighbor_prefix_counts_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [<ipv4|ipv6> [<unicast|multicast|vpn>]] "
       "neighbors <A.B.C.D|X:X::X:X|WORD> prefix-counts [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\n"
       "Address Family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n"
       "Neighbor on BGP configured interface\n"
       "Display detailed prefix count information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, no_metric_style_cmd_vtysh, 
       "no metric-style", 
       "Negate a command or set its defaults\n"
       "Use old-style (ISO 10589) or new-style packet formats\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_neighbor_routes_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] "
       "neighbors <A.B.C.D|X:X::X:X|WORD> <flap-statistics|dampened-routes|routes> [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n"
       "Neighbor on BGP configured interface\n"
       "Display flap statistics of the routes learned from neighbor\n"
       "Display the dampened routes received from neighbor\n"
       "Display routes learned from neighbor\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, ip_ospf_hello_interval_cmd_vtysh, 
       "ip ospf hello-interval (1-65535) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Address of interface\n")

DEFSH (VTYSH_ISISD, set_attached_bit_cmd_vtysh, 
       "set-attached-bit", 
       "Set attached bit to identify as L1/L2 router for inter-area traffic\n"
       "Set attached bit\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv6_rmap_metric_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct>" " route-map WORD metric (0-4294967295)", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n"
       "Metric for redistributed routes\n"
       "Default metric\n")

/* modified by liwh for bug 45793, 2017-12-07 */
#ifdef _CENTEC_
DEFSH (VTYSH_OSPFD, no_debug_ospf_event_sub_cmd_vtysh, 
       "no debug ospf event <abr|ia|route|spf>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF event information\n"
       "OSPF Area Border Router\n"
       "OSPF Inter-area Routing\n"
       "OSPF Route Table\n"
       "OSPF Shortest Path First Calculation\n")
#endif
/*liwh end */

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_seq_cmd_vtysh, 
       "ip prefix-list WORD seq (1-4294967295) <deny|permit> <A.B.C.D/M|any>", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Any prefix match. Same as \"0.0.0.0/0 le 32\"\n")

DEFSH (VTYSH_OSPFD, ospf_abr_type_cmd_vtysh, 
       "ospf abr-type <cisco|ibm|shortcut|standard>", 
       "OSPF specific commands\n"
       "Set OSPF ABR type\n"
       "Alternative ABR,  cisco implementation\n"
       "Alternative ABR,  IBM implementation\n"
       "Shortcut ABR\n"
       "Standard behavior (RFC2328)\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_errors_cmd_vtysh, 
       "no debug mpls ldp errors", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Errors\n")

DEFSH (VTYSH_RIPNGD, no_ripng_redistribute_type_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ospf6|isis|bgp|nhrp|table|vnc>" " [metric (0-16)] [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_detail_cmd_vtysh, 
       "show ip prefix-list detail", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Detail of prefix lists\n")

DEFSH (VTYSH_BGPD, set_ipv6_nexthop_prefer_global_cmd_vtysh, 
       "set ipv6 next-hop prefer-global", 
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "Prefer global over link-local if both exist\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_redist_bgpdirect_no_prefixlist_cmd_vtysh, 
       "no redistribute bgp-direct <ipv4|ipv6> prefix-list", 
       "Negate a command or set its defaults\n"
       "Redistribute from other protocol\n"
       "Redistribute from BGP directly\n"
       "Disable redistribute filter\n"
       "IPv4 routes\n"
       "IPv6 routes\n" "Prefix-list for filtering redistributed routes\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_rmap_call_cmd_vtysh, 
       "no call", 
       "Negate a command or set its defaults\n"
       "Jump to another Route-Map after match+set\n")

DEFSH (VTYSH_BGPD, debug_bgp_bestpath_prefix_cmd_vtysh, 
       "debug bgp bestpath <A.B.C.D/M|X:X::X:X/M>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP bestpath\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_RIPD, rip_passive_interface_cmd_vtysh, 
       "passive-interface <IFNAME|default>", 
       "Suppress routing updates on an interface\n"
       "Interface name\n"
       "default for all interfaces\n")

DEFSH (VTYSH_BGPD, debug_bgp_neighbor_events_peer_cmd_vtysh, 
       "debug bgp neighbor-events <A.B.C.D|X:X::X:X|WORD>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Neighbor Events\n"
       "BGP neighbor IP address to debug\n"
       "BGP IPv6 neighbor to debug\n"
       "BGP neighbor on interface to debug\n")

DEFSH (VTYSH_LDPD, ldp_discovery_transport_address_ipv4_cmd_vtysh, 
       "discovery transport-address A.B.C.D", 
       "Configure discovery parameters\n"
       "Specify transport address for TCP connection\n"
       "IP address to be used as transport address\n")

DEFSH (VTYSH_ZEBRA, no_match_ip_address_prefix_len_cmd_vtysh, 
       "no match ip address prefix-len [(0-32)]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match prefix length of ip address\n"
       "Match prefix length of ip address\n"
       "Prefix length\n")

DEFSH (VTYSH_ZEBRA, match_source_protocol_cmd_vtysh, 
       "match source-protocol <bgp|ospf|rip|ripng|isis|ospf6|connected|system|kernel|static>", 
       "Match values from routing table\n"
       "Match protocol via which the route was learnt\n"
       "BGP protocol\n"
       "OSPF protocol\n"
       "RIP protocol\n"
       "RIPNG protocol\n"
       "ISIS protocol\n"
       "OSPF6 protocol\n"
       "Routes from directly connected peer\n"
       "Routes from system configuration\n"
       "Routes from kernel\n"
       "Statically configured routes\n")

DEFSH (VTYSH_RIPNGD, no_ipv6_ripng_split_horizon_cmd_vtysh, 
       "no ipv6 ripng split-horizon [poisoned-reverse]", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n"
       "With poisoned-reverse\n")

DEFSH (VTYSH_ZEBRA, show_zebra_fpm_stats_cmd_vtysh, 
       "show zebra fpm stats", 
       "Show running system information\n"
       "Zebra information\n"
       "Forwarding Path Manager information\n"
       "Statistics\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_interface_cmd_vtysh, 
       "show ip ospf interface [INTERFACE] [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Interface information\n"
       "Interface name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, neighbor_port_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X> port (0-65535)", 
       "Specify neighbor router\n"
       "Neighbor address\nIPv6 address\n"
       "Neighbor's BGP port\n"
       "TCP port number\n")

DEFSH (VTYSH_PIMD, ip_pim_rp_cmd_vtysh, 
       "ip pim rp A.B.C.D [A.B.C.D/M]", 
       "IP information\n"
       "pim multicast routing\n"
       "Rendevous Point\n"
       "ip address of RP\n"
       "Group Address range to cover\n")

DEFSH (VTYSH_BGPD, no_set_aspath_exclude_cmd_vtysh, 
       "no set as-path exclude (1-4294967295)...", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "Transform BGP AS_PATH attribute\n"
       "Exclude from the as-path\n"
       "AS number\n")

DEFSH (VTYSH_BGPD, no_neighbor_override_capability_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> override-capability", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Override capability negotiation result\n")

DEFSH (VTYSH_BGPD, no_ipv6_bgp_network_cmd_vtysh, 
       "no network X:X::X:X/M [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Specify a network to announce via BGP\n"
       "IPv6 prefix\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_lsa_hex_cmd_vtysh, 
       "debug ospf6 lsa <router|network|inter-prefix|inter-router|as-external|link|intra-prefix|unknown> [<originate|examine|flooding>]", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug Link State Advertisements (LSAs)\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Display LSAs of unknown origin\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_id_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> linkstate-id A.B.C.D [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Search by Link state ID\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n"
      )

DEFSH (VTYSH_ISISD, show_database_cmd_vtysh, 
       "show isis database [detail] [WORD]", 
       "Show running system information\n"
       "IS-IS information\n"
       "IS-IS link state database\n"
       "Detailed information\n"
       "LSP ID\n")

DEFSH (VTYSH_BGPD, no_router_bgp_cmd_vtysh, 
       "no router bgp [(1-4294967295) [<view|vrf> WORD]]", 
       "Negate a command or set its defaults\n"
       "Enable a routing process\n"
       "BGP information\n"
       "AS number\n"
       "BGP view\nBGP VRF\nView/VRF name\n")

DEFSH (VTYSH_ALL, service_advanced_vty_cmd_vtysh, 
       "service advanced-vty", 
       "Set up miscellaneous service\n"
       "Enable advanced mode vty interface\n")

DEFSH (VTYSH_BGPD, no_neighbor_timers_connect_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> timers connect [(1-65535)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP per neighbor timers\n"
       "BGP connect timer\n"
       "Connect timer\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_dead_interval_cmd_vtysh, 
              "no ospf dead-interval [<(1-65535)|minimal hello-multiplier (1-10)> [A.B.C.D]]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Interval time after which a neighbor is declared down\n"
              "Seconds\n"
              "Address of interface")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M [from X:X::X:X/M] <X:X::X:X|INTERFACE|null0> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_RIPD, no_rip_offset_list_cmd_vtysh, 
       "no offset-list WORD <in|out> (0-16)", 
       "Negate a command or set its defaults\n"
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n")

DEFSH (VTYSH_ISISD, spf_interval_l2_cmd_vtysh, 
       "spf-interval level-2 (1-120)", 
       "Minimum interval between SPF calculations\n"
       "Set interval for level 2 only\n"
       "Minimum interval between consecutive SPFs in seconds\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_mtu_cmd_vtysh, 
       "ipv6 nd mtu (1-65535)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Advertised MTU\n"
       "MTU in bytes\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_pref_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, no_ipv6_aggregate_address_cmd_vtysh, 
       "no aggregate-address X:X::X:X/M [summary-only]", 
       "Negate a command or set its defaults\n"
       "Configure BGP aggregate entries\n"
       "Aggregate prefix\n"
       "Filter more specific routes from updates\n")

DEFSH (VTYSH_RIPNGD, ripng_redistribute_type_metric_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf6|isis|bgp|nhrp|table|vnc>" " metric (0-16)", 
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n")

DEFSH (VTYSH_RIPD, no_ip_rip_authentication_key_chain_cmd_vtysh, 
       "no ip rip authentication key-chain [LINE]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Authentication control\n"
       "Authentication key-chain\n"
       "name of key-chain\n")

DEFSH (VTYSH_ISISD, show_isis_interface_arg_cmd_vtysh, 
       "show isis interface WORD", 
       "Show running system information\n"
       "ISIS network information\n"
       "ISIS interface\n"
       "ISIS interface name\n")

DEFSH (VTYSH_OSPFD, ip_ospf_transmit_delay_addr_cmd_vtysh, 
       "ip ospf transmit-delay (1-65535) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Link state transmit delay\n"
       "Seconds\n"
       "Address of interface")

DEFSH (VTYSH_BGPD, no_debug_bgp_neighbor_events_cmd_vtysh, 
       "no debug bgp neighbor-events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "Neighbor Events\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_description_cmd_vtysh, 
       "no ipv6 prefix-list WORD description", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Prefix-list specific description\n")

DEFSH (VTYSH_RIPD, no_ip_rip_split_horizon_cmd_vtysh, 
       "no ip rip split-horizon", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_cost_lnh_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> cost (0-255) LNH_OPTIONS...", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_BGPD, neighbor_local_as_no_prepend_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> local-as (1-4294967295) no-prepend", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Specify a local-as number\n"
       "AS number used as local AS\n"
       "Do not prepend local-as to updates from ebgp peers\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_export_no_routemap_cmd_vtysh, 
       "no export <bgp|zebra> route-map [NAME]", 
       "Negate a command or set its defaults\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "Route-map for filtering exported routes\n" "route map name\n")

DEFSH (VTYSH_RIPNGD, ripng_aggregate_address_cmd_vtysh, 
       "aggregate-address X:X::X:X/M", 
       "Set aggregate RIPng route announcement\n"
       "Aggregate network\n")

DEFSH (VTYSH_RIPD, no_rip_redistribute_type_metric_routemap_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>" " metric (0-16) route-map WORD", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_all_overlay_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn all overlay", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information about all EVPN NLRIs\n"
      "Display BGP Overlay Information for prefixes\n")

DEFSH (VTYSH_BGPD, no_bgp_network_cmd_vtysh, 
       "no network A.B.C.D/M [<backdoor|route-map WORD>]", 
       "Negate a command or set its defaults\n"
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n"
       "Specify a BGP backdoor route\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_ISISD, debug_isis_spfstats_cmd_vtysh, 
       "debug isis spf-statistics ", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS SPF Timing and Statistic Data\n")

DEFSH (VTYSH_PIMD, show_ip_igmp_interface_cmd_vtysh, 
       "show ip igmp interface [detail|WORD] [json]", 
       "Show running system information\n"
       "IP information\n"
       "IGMP information\n"
       "IGMP interface information\n"
       "Detailed output\n"
       "interface name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_detail_all_cmd_vtysh, 
       "show ip ospf neighbor detail all [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "detail of all neighbors\n"
       "include down status neighbor\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_allocate_for_acl_cmd_vtysh, 
       "no label local allocate for <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure label allocation control\n"
       "IP access-list\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_seq_le_cmd_vtysh, 
       "ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M le (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_RIPD, ip_rip_split_horizon_cmd_vtysh, 
       "ip rip split-horizon", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_summary_prefix_cmd_vtysh, 
       "show ipv6 route vrf all summary prefix", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Summary of all IPv6 routes\n"
       "Prefix routes\n")

DEFSH (VTYSH_PIMD, clear_ip_igmp_interfaces_cmd_vtysh, 
       "clear ip igmp interfaces", 
       "Reset functions\n"
       "IP information\n"
       "IGMP clear commands\n"
       "Reset IGMP interfaces\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_packet_cmd_vtysh, 
       "debug zebra packet [<recv|send>] [detail]", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n"
       "Debug option set for detailed info\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_for_acl_cmd_vtysh, 
       "no label local advertise for <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_OSPF6D, no_area_export_list_cmd_vtysh, 
       "no area A.B.C.D export-list NAME", 
       "Negate a command or set its defaults\n"
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_summary_name_cmd_vtysh, 
       "show ip prefix-list summary WORD", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Summary of prefix lists\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_un_prefix_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> un <*|A.B.C.D|X:X::X:X> prefix <*|A.B.C.D/M|X:X::X:X/M>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 Prefix\n"
       "IPv6 Prefix\n")

DEFSH (VTYSH_BGPD, no_neighbor_solo_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> solo", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Solo peer - part of its own update group\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_vnc_all_cmd_vtysh, 
       "<no debug|undebug> all bgp vnc", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Undebug command\n"
       "Disable all VNC debugging\n"
       "BGP information\n"
       "VNC information\n")

DEFSH (VTYSH_BGPD, debug_bgp_update_prefix_cmd_vtysh, 
       "debug bgp updates prefix <A.B.C.D/M|X:X::X:X/M>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n"
       "Specify a prefix to debug\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_area_cmd_vtysh, 
       "no ip ospf [(1-65535)] area [<A.B.C.D|(0-4294967295)>]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Instance ID\n"
       "Disable OSPF on this interface\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n")

DEFSH (VTYSH_NHRPD, nhrp_nflog_group_cmd_vtysh, 
 "nhrp nflog-group (1-65535)", 
 "Next Hop Resolution Protocol functions\n"
 "Specify NFLOG group number\n"
 "NFLOG group number\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_flags_cmd_vtysh, 
       "ipv6 route X:X::X:X/M [from X:X::X:X/M] <X:X::X:X|INTERFACE> <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_authentication_key_cmd_vtysh, 
              "ospf authentication-key AUTH_KEY [A.B.C.D]", 
              "OSPF interface commands\n"
              "Authentication password (key)\n"
              "The OSPF password (key)\n"
              "Address of interface\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_cost_cmd_vtysh, 
       "no ip ospf cost [(1-65535)] [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "OSPF interface commands\n"
       "Interface cost\n"
       "Address of interface")

DEFSH (VTYSH_BGPD, no_ipv6_bgp_distance_source_access_list_cmd_vtysh, 
       "no distance (1-255) X:X::X:X/M WORD", 
       "Negate a command or set its defaults\n"
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n"
       "Access list name\n")

DEFSH (VTYSH_BGPD, vpnv4_network_route_map_cmd_vtysh, 
       "network A.B.C.D/M rd ASN:nn_or_IP-address:nn <tag|label> (0-1048575) route-map WORD", 
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n"
       "Specify Route Distinguisher\n"
       "VPN Route Distinguisher\n"
       "VPN NLRI label (tag)\n"
       "VPN NLRI label (tag)\n"
       "Label value\n"
       "route map\n"
       "route map name\n")

DEFSH (VTYSH_OSPFD|VTYSH_OSPF6D, ospf6_routemap_no_set_metric_type_cmd_vtysh, 
       "no set metric-type [<type-1|type-2>]", 
       "Negate a command or set its defaults\n"
       "Set value\n"
       "Type of metric\n"
       "OSPF6 external type 1 metric\n"
       "OSPF6 external type 2 metric\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_le_cmd_vtysh, 
       "ipv6 prefix-list WORD <deny|permit> X:X::X:X/M le (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_BGPD, no_match_ecommunity_cmd_vtysh, 
       "no match extcommunity [<(1-99)|(100-500)|WORD>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match BGP/VPN extended community list\n"
       "Extended community-list number (standard)\n"
       "Extended community-list number (expanded)\n"
       "Extended community-list name\n")

DEFSH (VTYSH_BGPD, bgp_bestpath_aspath_multipath_relax_cmd_vtysh, 
       "bgp bestpath as-path multipath-relax [<as-set|no-as-set>]", 
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Allow load sharing across routes that have different AS paths (but same length)\n"
       "Generate an AS_SET\n"
       "Do not generate an AS_SET\n")

DEFSH (VTYSH_BGPD, vnc_show_summary_cmd_vtysh, 
       "show vnc summary", 
       "Show running system information\n"
       "VNC information\n"
       "Display VNC status summary\n")

DEFSH (VTYSH_PIMD, clear_ip_mroute_cmd_vtysh, 
       "clear ip mroute", 
       "Reset functions\n"
       "IP information\n"
       "Reset multicast routes\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_mtu_ignore_addr_cmd_vtysh, 
       "no ip ospf mtu-ignore [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Disable MTU mismatch detection on this interface\n"
       "Address of interface")

DEFSH (VTYSH_BGPD, bgp_confederation_peers_cmd_vtysh, 
       "bgp confederation peers (1-4294967295)...", 
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "Peer ASs in BGP confederation\n"
       "AS number\n")

DEFSH (VTYSH_BGPD, no_neighbor_shutdown_msg_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> shutdown message MSG...", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Administratively shut down this neighbor\n"
       "Remove a shutdown message (draft-ietf-idr-shutdown-06)\n"
       "Shutdown message\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_passive_cmd_vtysh, 
       "ipv6 ospf6 passive", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Passive interface; no adjacency will be formed on this interface\n"
       )

DEFSH (VTYSH_BGPD, clear_vnc_mac_all_prefix_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> prefix <*|A.B.C.D/M|X:X::X:X/M>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_ge_cmd_vtysh, 
       "ip prefix-list WORD <deny|permit> A.B.C.D/M ge (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_ZEBRA, link_params_pkt_loss_cmd_vtysh, 
       "packet-loss PERCENTAGE", 
       "Unidirectional Link Packet Loss\n"
       "percentage of total traffic by 0.000003tep and less than 50.331642%\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_bfd_param_cmd_vtysh, 
       "ipv6 ospf6 bfd (2-255) (50-60000) (50-60000)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Enables BFD support\n"
       "Detect Multiplier\n"
       "Required min receive interval\n"
       "Desired min transmit interval\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_id_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor A.B.C.D [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "Neighbor ID\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD, ip_rip_split_horizon_poisoned_reverse_cmd_vtysh, 
       "ip rip split-horizon poisoned-reverse", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n"
       "With poisoned-reverse\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_redist_bgpdirect_no_routemap_cmd_vtysh, 
       "no redistribute bgp-direct route-map", 
       "Negate a command or set its defaults\n"
       "Redistribute from other protocols\n"
       "Redistribute from BGP directly\n"
       "Disable redistribute filter\n"
       "Route-map for filtering redistributed routes\n")

DEFSH (VTYSH_ISISD, show_isis_interface_detail_cmd_vtysh, 
       "show isis interface detail", 
       "Show running system information\n"
       "ISIS network information\n"
       "ISIS interface\n"
       "show detailed information\n")

DEFSH (VTYSH_RIPNGD, ripng_passive_interface_cmd_vtysh, 
       "passive-interface IFNAME", 
       "Suppress routing updates on an interface\n"
       "Interface name\n")

DEFSH (VTYSH_RIPD, ip_rip_authentication_string_cmd_vtysh, 
       "ip rip authentication string LINE", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Authentication control\n"
       "Authentication string\n"
       "Authentication string\n")

DEFSH (VTYSH_ALL, no_access_list_standard_host_cmd_vtysh, 
       "no access-list <(1-99)|(1300-1999)> <deny|permit> host A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "A single host address\n"
       "Address to match\n")

DEFSH (VTYSH_NHRPD, clear_nhrp_cmd_vtysh, 
 "clear " "<ip|ipv6>" " nhrp <cache|shortcut>", 
 "Reset functions\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Dynamic cache entries\n"
 "Shortcut entries\n")

DEFSH (VTYSH_BGPD, clear_ip_bgp_dampening_address_cmd_vtysh, 
       "clear ip bgp dampening A.B.C.D", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "Clear route flap dampening information\n"
       "Network to clear damping information\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_label_cmd_vtysh, 
       "no ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, neighbor_addpath_tx_all_paths_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> addpath-tx-all-paths", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Use addpath to advertise all paths to a neighbor\n")

DEFSH (VTYSH_BGPD, no_neighbor_description_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> description [LINE]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Neighbor specific description\n"
       "Up to 80 characters describing this neighbor\n")

DEFSH (VTYSH_PIMD, no_debug_mroute_detail_cmd_vtysh, 
       "no debug mroute detail", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM interaction with kernel MFC cache\n"
       "detailed\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_interface_cmd_vtysh, 
       "no debug ospf6 interface", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 Interface\n"
      )

DEFSH (VTYSH_RIPNGD, ripng_default_information_originate_cmd_vtysh, 
       "default-information originate", 
       "Default route information\n"
       "Distribute default route\n")

DEFSH (VTYSH_ISISD, show_isis_spf_ietf_cmd_vtysh, 
       "show isis spf-delay-ietf", 
       "Show running system information\n"
       "IS-IS information\n"
       "IS-IS SPF delay IETF information\n")

DEFSH (VTYSH_ZEBRA, clear_zebra_fpm_stats_cmd_vtysh, 
       "clear zebra fpm stats", 
       "Reset functions\n"
       "Zebra information\n"
       "Clear Forwarding Path Manager information\n"
       "Statistics\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_prefix_cmd_vtysh, 
       "show ipv6 prefix-list WORD X:X::X:X/M", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n")

DEFSH (VTYSH_BGPD, clear_vnc_prefix_vn_un_cmd_vtysh, 
       "clear vnc prefix <*|A.B.C.D/M|X:X::X:X/M> vn <*|A.B.C.D|X:X::X:X> un <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_le_ge_cmd_vtysh, 
       "ip prefix-list WORD <deny|permit> A.B.C.D/M le (0-32) ge (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_LDPD, ldp_discovery_targeted_hello_accept_cmd_vtysh, 
       "discovery targeted-hello accept", 
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Accept and respond to targeted hellos\n")

DEFSH (VTYSH_BGPD, debug_bgp_nht_cmd_vtysh, 
       "debug bgp nht", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP nexthop tracking events\n")

DEFSH (VTYSH_BGPD, ip_lcommunity_list_name_standard_cmd_vtysh, 
       "ip large-community-list standard WORD <deny|permit>", 
       "IP information\n"
       "Add a large community list entry\n"
       "Specify standard large-community-list\n"
       "Large Community list name\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_summary_cmd_vtysh, 
       "show ipv6 route [vrf NAME] summary", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Summary of all IPv6 routes\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_address_addr_cmd_vtysh, 
       "no neighbor address <A.B.C.D|X:X::X:X>", 
       "Negate a command or set its defaults\n"
       "Remote endpoint configuration\n"
       "Specify the IPv4 or IPv6 address of the remote endpoint\n"
       "IPv4 address\n"
       "IPv6 address\n")

DEFSH (VTYSH_ALL, no_service_advanced_vty_cmd_vtysh, 
       "no service advanced-vty", 
       "Negate a command or set its defaults\n"
       "Set up miscellaneous service\n"
       "Enable advanced mode vty interface\n")

DEFSH (VTYSH_ISISD, csnp_interval_l2_cmd_vtysh, 
       "isis csnp-interval (1-600) level-2", 
       "IS-IS commands\n"
       "Set CSNP interval in seconds\n"
       "CSNP interval value\n"
       "Specify interval for level-2 CSNPs\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_ge_cmd_vtysh, 
       "ipv6 prefix-list WORD <deny|permit> X:X::X:X/M ge (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_OSPFD, no_ospf_log_adjacency_changes_detail_cmd_vtysh, 
       "no log-adjacency-changes detail", 
       "Negate a command or set its defaults\n"
       "Log changes in adjacency state\n"
       "Log all state changes\n")

DEFSH (VTYSH_BGPD, neighbor_allowas_in_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> allowas-in [<(1-10)|origin>]", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Accept as-path with my AS present in it\n"
       "Number of occurances of AS number\n"
       "Only accept my AS in the as-path if the route was originated in my AS\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_stub_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> stub", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_rmap_onmatch_goto_cmd_vtysh, 
       "no on-match goto", 
       "Negate a command or set its defaults\n"
       "Exit policy on matches\n"
       "Goto Clause number\n")

DEFSH (VTYSH_BGPD, match_local_pref_cmd_vtysh, 
       "match local-preference (0-4294967295)", 
       "Match values from routing table\n"
       "Match local-preference of route\n"
       "Metric value\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_discovery_hello_dir_cmd_vtysh, 
       "debug mpls ldp discovery hello <recv|sent>", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Discovery messages\n"
       "Discovery hello message\n"
       "Received messages\n"
       "Sent messages\n")

DEFSH (VTYSH_OSPFD, ospf_timers_min_ls_arrival_cmd_vtysh, 
       "timers lsa arrival (0-1000)", 
       "Adjust routing timers\n"
       "Throttling link state advertisement delays\n"
       "OSPF minimum arrival interval delay\n"
       "Delay (msec) between accepted LSAs\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_ge_le_cmd_vtysh, 
       "ip prefix-list WORD <deny|permit> A.B.C.D/M ge (0-32) le (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_self_originated_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> self-originated [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Display Self-originated LSAs\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_PIMD, no_debug_msdp_cmd_vtysh, 
       "no debug msdp", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "MSDP protocol activity\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_int_detail_cmd_vtysh, 
       "show ip ospf neighbor IFNAME detail [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "Interface name\n"
       "detail of all neighbors\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_PIMD, no_debug_igmp_cmd_vtysh, 
       "no debug igmp", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n")

DEFSH (VTYSH_BGPD, ipv6_bgp_distance_source_cmd_vtysh, 
       "distance (1-255) X:X::X:X/M", 
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n")

DEFSH (VTYSH_BGPD, bgp_network_route_map_cmd_vtysh, 
       "network A.B.C.D/M route-map WORD", 
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_route_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]]"
       "<A.B.C.D|A.B.C.D/M|X:X::X:X|X:X::X:X/M> [<bestpath|multipath>] [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Network in the BGP routing table to display\n"
       "IPv4 prefix\n"
       "Network in the BGP routing table to display\n"
       "IPv6 prefix\n"
       "Display only the bestpath\n"
       "Display only multipaths\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, set_ecommunity_soo_cmd_vtysh, 
       "set extcommunity soo ASN:nn_or_IP-address:nn...", 
       "Set values in destination routing protocol\n"
       "BGP extended community attribute\n"
       "Site-of-Origin extended community\n"
       "VPN extended community\n")

DEFSH (VTYSH_RIPD, show_ip_rip_cmd_vtysh, 
       "show ip rip", 
       "Show running system information\n"
       "IP information\n"
       "Show RIP routes\n")

DEFSH (VTYSH_BGPD, show_ip_as_path_access_list_cmd_vtysh, 
       "show ip as-path-access-list WORD", 
       "Show running system information\n"
       "IP information\n"
       "List AS path access lists\n"
       "AS path access list name\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_deadinterval_cmd_vtysh, 
       "ipv6 ospf6 dead-interval (1-65535)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interval time after which a neighbor is declared down\n"
       "Seconds\n"
       )

DEFSH (VTYSH_ZEBRA, ip_irdp_maxadvertinterval_cmd_vtysh, 
       "ip irdp maxadvertinterval (4-1800)", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "Set maximum time between advertisement\n"
       "Maximum advertisement interval in seconds\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_updgrps_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] update-groups [SUBGROUP-ID]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Detailed info about dynamic update groups\n"
       "Specific subgroup to display detailed info for\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X>", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n" "UN IPv6 interface address\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_rt_export_cmd_vtysh, 
       "rt export RTLIST...", 
       "Specify route targets\n"
       "Export filter\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_LDPD, ldp_discovery_targeted_hello_interval_disc_time_cmd_vtysh, 
       "discovery targeted-hello interval (1-65535)", 
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Targeted hello interval\n"
       "Time (seconds)\n")

DEFSH (VTYSH_BGPD, no_bgp_client_to_client_reflection_cmd_vtysh, 
       "no bgp client-to-client reflection", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Configure client to client route reflection\n"
       "reflection of routes allowed\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_seq_le_cmd_vtysh, 
       "ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M le (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_ISISD, isis_hello_interval_l1_cmd_vtysh, 
       "isis hello-interval (1-600) level-1", 
       "IS-IS commands\n"
       "Set Hello interval\n"
       "Hello interval value\n"
       "Holdtime 1 second,  interval depends on multiplier\n"
       "Specify hello-interval for level-1 IIHs\n")

DEFSH (VTYSH_PIMD, undebug_msdp_events_cmd_vtysh,  "undebug msdp events", 
      "Disable debugging functions (see also 'debug')\n" "MSDP protocol activity\n" "MSDP protocol events\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_packet_cmd_vtysh, 
       "no debug ospf [(1-65535)] packet <hello|dd|ls-request|ls-update|ls-ack|all> [<send [detail]|recv [detail]|detail>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF packets\n"
       "OSPF Hello\n"
       "OSPF Database Description\n"
       "OSPF Link State Request\n"
       "OSPF Link State Update\n"
       "OSPF Link State Acknowledgment\n"
       "OSPF all packets\n"
       "Packet sent\n"
       "Detail Information\n"
       "Packet received\n"
       "Detail Information\n"
       "Detail Information\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_cost_cmd_vtysh, 
       "ipv6 ospf6 cost (1-65535)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface cost\n"
       "Outgoing metric of this interface\n"
       )

DEFSH (VTYSH_BGPD, no_neighbor_password_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> password [LINE]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Set a password\n"
       "The password\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_all_tags_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn all tags", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information about all EVPN NLRIs\n"
      "Display BGP tags for prefixes\n")

DEFSH (VTYSH_OSPFD, ip_ospf_bfd_cmd_vtysh, 
       "ip ospf bfd", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Enables BFD support\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_id_cmd_vtysh, 
       "show ip ospf neighbor A.B.C.D [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "Neighbor ID\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_PIMD, show_ip_ssmpingd_cmd_vtysh, 
       "show ip ssmpingd", 
       "Show running system information\n"
       "IP information\n"
       "ssmpingd operation\n")

DEFSH (VTYSH_RIPD, no_rip_passive_interface_cmd_vtysh, 
       "no passive-interface <IFNAME|default>", 
       "Negate a command or set its defaults\n"
       "Suppress routing updates on an interface\n"
       "Interface name\n"
       "default for all interfaces\n")

DEFSH (VTYSH_ISISD, no_debug_isis_lupd_cmd_vtysh, 
       "no debug isis local-updates", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS local update packets\n")

DEFSH (VTYSH_PIMD, undebug_msdp_cmd_vtysh,  "undebug msdp", 
      "Disable debugging functions (see also 'debug')\n" "MSDP protocol activity\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_homeagent_lifetime_cmd_vtysh, 
       "ipv6 nd home-agent-lifetime (0-65520)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent lifetime\n"
       "Home Agent lifetime in seconds (0 to track ra-lifetime)\n")

DEFSH (VTYSH_RIPD, no_rip_offset_list_ifname_cmd_vtysh, 
       "no offset-list WORD <in|out> (0-16) IFNAME", 
       "Negate a command or set its defaults\n"
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n"
       "Interface to match\n")

DEFSH (VTYSH_BGPD, show_bgp_vrfs_cmd_vtysh, 
       "show [ip] bgp vrfs [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Show BGP VRFs\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_bgp_always_compare_med_cmd_vtysh, 
       "no bgp always-compare-med", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Allow comparing MED from different neighbors\n")

DEFSH (VTYSH_RIPD, no_ip_rip_send_version_cmd_vtysh, 
       "no ip rip send version [(1-2)]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement transmission\n"
       "Version control\n"
       "Version 1\n"
       "Version 2\n")

DEFSH (VTYSH_ALL, no_vty_login_cmd_vtysh, 
       "no login", 
       "Negate a command or set its defaults\n"
       "Enable password checking\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_fpm_cmd_vtysh, 
       "debug zebra fpm", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug zebra FPM events\n")

DEFSH (VTYSH_OSPF6D, no_router_ospf6_cmd_vtysh, 
       "no router ospf6", 
       "Negate a command or set its defaults\n"
       "Enable a routing process\n"
       "Open Shortest Path First (OSPF) for IPv6\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh, 
       "no discovery targeted-hello holdtime (1-65535)", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "LDP Targeted Hellos\n"
       "Targeted hello holdtime\n"
       "Time (seconds) - 65535 implies infinite\n")

DEFSH (VTYSH_OSPFD, no_ospf_distance_ospf_cmd_vtysh, 
       "no distance ospf [{intra-area [(1-255)]|inter-area [(1-255)]|external [(1-255)]}]", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "OSPF administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n")

DEFSH (VTYSH_ZEBRA, no_link_params_pkt_loss_cmd_vtysh, 
       "no packet-loss", 
       "Negate a command or set its defaults\n"
       "Disable Unidirectional Link Packet Loss on this interface\n")

DEFSH (VTYSH_BGPD, show_bgp_updgrps_afi_adj_s_cmd_vtysh, 
       "show [ip] bgp ""<ipv4|ipv6>"" ""<unicast|multicast|vpn>"" update-groups SUBGROUP-ID <advertise-queue|advertised-routes|packet-queue>", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Address Family\nAddress Family\n" "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Detailed info about dynamic update groups\n"
       "Specific subgroup to display info for\n"
       "Advertisement queue\n"
       "Announced routes\n"
       "Packet queue\n"
       "Specific subgroup info wanted for\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_ipv6_address_prefix_list_cmd_vtysh, 
       "match ipv6 address prefix-list WORD", 
       "Match values from routing table\n"
       "IPv6 information\n"
       "Match address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_retransmit_interval_cmd_vtysh, 
              "ospf retransmit-interval (3-65535) [A.B.C.D]", 
              "OSPF interface commands\n"
              "Time between retransmitting lost link state advertisements\n"
              "Seconds\n"
              "Address of interface")

DEFSH (VTYSH_BGPD, no_set_ipv6_nexthop_peer_cmd_vtysh, 
       "no set ipv6 next-hop peer-address", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "Use peer address (for BGP only)\n")

DEFSH (VTYSH_RIPD, no_ip_rip_authentication_mode_cmd_vtysh, 
       "no ip rip authentication mode [<md5|text> [auth-length <rfc|old-ripd>]]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Authentication control\n"
       "Authentication mode\n"
       "Keyed message digest\n"
       "Clear text authentication\n"
       "MD5 authentication data length\n"
       "RFC compatible\n"
       "Old ripd compatible\n")

DEFSH (VTYSH_OSPFD, ip_ospf_mtu_ignore_addr_cmd_vtysh, 
       "ip ospf mtu-ignore [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Disable MTU mismatch detection on this interface\n"
       "Address of interface")

DEFSH (VTYSH_BGPD, no_ip_extcommunity_list_standard_all_cmd_vtysh, 
       "no ip extcommunity-list <(1-99)|standard WORD> <deny|permit> AA:NN...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a extended community list entry\n"
       "Extended Community list number (standard)\n"
       "Specify standard extcommunity-list\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Extended community attribute in 'rt aa:nn_or_IPaddr:nn' OR 'soo aa:nn_or_IPaddr:nn' format\n")

DEFSH (VTYSH_BGPD, neighbor_shutdown_cmd_vtysh, 
      "neighbor <A.B.C.D|X:X::X:X|WORD> shutdown", 
      "Specify neighbor router\n" "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
      "Administratively shut down this neighbor\n")

DEFSH (VTYSH_BGPD, vpnv6_network_cmd_vtysh, 
       "network X:X::X:X/M rd ASN:nn_or_IP-address:nn <tag|label> (0-1048575) [route-map WORD]", 
       "Specify a network to announce via BGP\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Specify Route Distinguisher\n"
       "VPN Route Distinguisher\n"
       "VPN NLRI label (tag)\n"
       "VPN NLRI label (tag)\n"
       "Label value\n"
       "route map\n"
       "route map name\n")

DEFSH (VTYSH_RIPD, rip_redistribute_type_metric_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>" " metric (0-16)", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n")

DEFSH (VTYSH_BGPD, bgp_update_delay_cmd_vtysh, 
       "update-delay (0-3600)", 
       "Force initial delay for best-path and updates\n"
       "Seconds\n")

DEFSH (VTYSH_ZEBRA, pseudowire_control_word_cmd_vtysh, 
       "[no] control-word <exclude|include>", 
       "Negate a command or set its defaults\n"
       "Control-word options\n"
       "Exclude control-word in pseudowire packets\n"
       "Include control-word in pseudowire packets\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_tag_cmd_vtysh, 
       "no match tag [(1-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match tag of route\n"
       "Tag value\n")

DEFSH (VTYSH_PIMD, interface_no_ip_igmp_query_interval_cmd_vtysh, 
       "no ip igmp query-interval", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP host query interval\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_lsr_id_ipv4_cmd_vtysh, 
       "no neighbor lsr-id A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Remote endpoint configuration\n"
       "Specify the LSR-ID of the remote endpoint\n"
       "IPv4 address\n")

DEFSH (VTYSH_RIPNGD, no_debug_ripng_packet_direct_cmd_vtysh, 
       "no debug ripng packet <recv|send>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_cost_cmd_vtysh, 
              "no ospf cost [(1-65535)] [A.B.C.D]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Interface cost\n"
              "Cost\n"
              "Address of interface\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_prefix_longer_cmd_vtysh, 
       "show ip route [vrf NAME] A.B.C.D/M longer-prefixes", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Show route matching the specified Network/Mask pair only\n")

DEFSH (VTYSH_OSPFD, ospf_area_vlink_intervals_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> virtual-link A.B.C.D {hello-interval (1-65535)|retransmit-interval (1-65535)|transmit-delay (1-65535)|dead-interval (1-65535)}", 
       "OSPF area parameters\n" "OSPF area ID in IP address format\n" "OSPF area ID as a decimal value\n" "Configure a virtual link\n" "Router ID of the remote ABR\n"
       "Time between HELLO packets\n" "Seconds\n" "Time between retransmitting lost link state advertisements\n" "Seconds\n" "Link state transmit delay\n" "Seconds\n" "Interval time after which a neighbor is declared down\n" "Seconds\n")

DEFSH (VTYSH_LDPD, ldp_discovery_hello_holdtime_disc_time_cmd_vtysh, 
       "discovery hello holdtime (1-65535)", 
       "Configure discovery parameters\n"
       "LDP Link Hellos\n"
       "Hello holdtime\n"
       "Time (seconds) - 65535 implies infinite\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_discovery_cmd_vtysh, 
       "show mpls ldp discovery", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Discovery Hello Information\n")

DEFSH (VTYSH_RIPNGD, no_ripng_aggregate_address_cmd_vtysh, 
       "no aggregate-address X:X::X:X/M", 
       "Negate a command or set its defaults\n"
       "Delete aggregate RIPng route announcement\n"
       "Aggregate network")

DEFSH (VTYSH_ISISD, no_isis_mpls_te_on_cmd_vtysh, 
       "no mpls-te", 
       "Negate a command or set its defaults\n"
       "Disable the MPLS-TE functionality\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_message_digest_key_cmd_vtysh, 
       "no ip ospf message-digest-key (1-255) [md5 KEY] [A.B.C.D]", 
        "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)\n"
       "Address of interface\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_all_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor all [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "include down status neighbor\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, show_isis_neighbor_detail_cmd_vtysh, 
       "show isis neighbor detail", 
       "Show running system information\n"
       "ISIS network information\n"
       "ISIS neighbor adjacencies\n"
       "show detailed information\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_border_routers_cmd_vtysh, 
       "show ipv6 ospf6 border-routers [<A.B.C.D|detail>]", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display routing table for ABR and ASBR\n"
       "Router ID\n"
       "Show detailed output\n")

DEFSH (VTYSH_BGPD, match_community_cmd_vtysh, 
       "match community <(1-99)|(100-500)|WORD> [exact-match]", 
       "Match values from routing table\n"
       "Match BGP community list\n"
       "Community-list number (standard)\n"
       "Community-list number (expanded)\n"
       "Community-list name\n"
       "Do exact matching of communities\n")

DEFSH (VTYSH_BGPD, bgp_default_subgroup_pkt_queue_max_cmd_vtysh, 
       "bgp default subgroup-pkt-queue-max (20-100)", 
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "subgroup-pkt-queue-max\n"
       "Configure subgroup packet queue max\n")

DEFSH (VTYSH_ISISD, isis_hello_multiplier_l1_cmd_vtysh, 
       "isis hello-multiplier (2-100) level-1", 
       "IS-IS commands\n"
       "Set multiplier for Hello holding time\n"
       "Hello multiplier value\n"
       "Specify hello multiplier for level-1 IIHs\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_rib_detailed_cmd_vtysh, 
       "no debug zebra rib detailed", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug zebra RIB\n"
       "Detailed debugs\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_vlink_cmd_vtysh, 
       "show ip ospf virtual-link", 
       SHOW_STR
       IP_STR
       OSPF_STR
       "Show virtual links information")

DEFSH (VTYSH_BGPD, neighbor_solo_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> solo", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Solo peer - part of its own update group\n")

DEFSH (VTYSH_RIPD, no_ip_rip_receive_version_cmd_vtysh, 
       "no ip rip receive version [(1-2)]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement reception\n"
       "Version control\n"
       "Version 1\n"
       "Version 2\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_rd_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn rd ASN:nn_or_IP-address:nn [json]", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information for a route distinguisher\n"
      "VPN Route Distinguisher\n" "JavaScript Object Notation\n")

DEFSH (VTYSH_LDPD, ldp_vc_type_pwtype_cmd_vtysh, 
       "vc type <ethernet|ethernet-tagged>", 
       "Virtual Circuit options\n"
       "Virtual Circuit type to use\n"
       "Ethernet (type 5)\n"
       "Ethernet-tagged (type 4)\n")

DEFSH (VTYSH_OSPFD, ospf_area_authentication_message_digest_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> authentication message-digest", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Enable authentication\n"
       "Use message-digest authentication\n")

DEFSH (VTYSH_ZEBRA, no_linkdetect_cmd_vtysh, 
       "no link-detect", 
       "Negate a command or set its defaults\n"
       "Disable link detection on interface\n")

DEFSH (VTYSH_ISISD, isis_priority_l2_cmd_vtysh, 
       "isis priority (0-127) level-2", 
       "IS-IS commands\n"
       "Set priority for Designated Router election\n"
       "Priority value\n"
       "Specify priority for level-2 routing\n")

DEFSH (VTYSH_BGPD, no_bgp_wpkt_quanta_cmd_vtysh, 
       "no write-quanta (1-10000)", 
       "Negate a command or set its defaults\n"
       "How many packets to write to peer socket per run\n"
       "Number of packets\n")

DEFSH (VTYSH_OSPFD, no_ospf_log_adjacency_changes_cmd_vtysh, 
       "no log-adjacency-changes", 
       "Negate a command or set its defaults\n"
       "Log changes in adjacency state\n")

DEFSH (VTYSH_NHRPD, if_nhrp_reg_flags_cmd_vtysh, 
 "<ip|ipv6>" " nhrp registration no-unique", 
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Registration configuration\n"
 "Don't set unique flag\n")

DEFSH (VTYSH_BGPD, ip_lcommunity_list_standard1_cmd_vtysh, 
       "ip large-community-list (1-99) <deny|permit> AA:BB:CC...", 
       "IP information\n"
       "Add a large community list entry\n"
       "Large Community list number (standard)\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "large community in 'aa:bb:cc' format\n")

DEFSH (VTYSH_BGPD, no_neighbor_send_community_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> send-community", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Send Community attribute to this neighbor\n")

DEFSH (VTYSH_RIPD, no_rip_redistribute_type_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_ISISD, isis_redistribute_cmd_vtysh, 
       "redistribute <ipv4|ipv6> " "<kernel|connected|static|rip|ripng|ospf|ospf6|bgp|pim|nhrp|table|vnc>" " <level-1|level-2> [<metric (0-16777215)|route-map WORD>]", 
       "Redistribute information from another routing protocol\n"
       "Redistribute IPv4 routes\n"
       "Redistribute IPv6 routes\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (OSPFv2)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Redistribute into level-1\n"
       "Redistribute into level-2\n"
       "Metric for redistributed routes\n"
       "ISIS default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, set_ecommunity_rt_cmd_vtysh, 
       "set extcommunity rt ASN:nn_or_IP-address:nn...", 
       "Set values in destination routing protocol\n"
       "BGP extended community attribute\n"
       "Route Target extended community\n"
       "VPN extended community\n")

DEFSH (VTYSH_OSPFD, ip_ospf_retransmit_interval_addr_cmd_vtysh, 
       "ip ospf retransmit-interval (3-65535) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"
       "Address of interface")

DEFSH (VTYSH_PIMD, no_ip_pim_rp_cmd_vtysh, 
       "no ip pim rp A.B.C.D [A.B.C.D/M]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Rendevous Point\n"
       "ip address of RP\n"
       "Group Address range to cover\n")

DEFSH (VTYSH_BGPD, no_neighbor_disable_connected_check_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> <disable-connected-check|enforce-multihop>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "one-hop away EBGP peer using loopback address\n"
       "Enforce EBGP neighbors perform multihop\n")

DEFSH (VTYSH_BGPD, no_set_weight_cmd_vtysh, 
       "no set weight [(0-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP weight for routing table\n"
       "Weight value\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_vlink_intervals_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> virtual-link A.B.C.D {hello-interval|retransmit-interval|transmit-delay|dead-interval}", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n" "OSPF area ID in IP address format\n" "OSPF area ID as a decimal value\n" "Configure a virtual link\n" "Router ID of the remote ABR\n"
       "Time between HELLO packets\n" "Time between retransmitting lost link state advertisements\n" "Link state transmit delay\n" "Interval time after which a neighbor is declared down\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_export_prefixlist_cmd_vtysh, 
       "export <bgp|zebra> <ipv4|ipv6> prefix-list NAME", 
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "Prefix-list for filtering exported routes\n" "prefix list name\n")

DEFSH (VTYSH_PIMD, debug_msdp_cmd_vtysh, 
       "debug msdp", 
       "Debugging functions (see also 'undebug')\n"
       "MSDP protocol activity\n")

DEFSH (VTYSH_ISISD, ip6_router_isis_cmd_vtysh, 
       "ipv6 router isis WORD", 
       "Interface Internet Protocol config commands\n"
       "IP router interface commands\n"
       "IS-IS Routing for IP\n"
       "Routing process tag\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_large_community_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [<ipv4|ipv6> [<unicast|multicast|vpn>]] large-community [AA:BB:CC] [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\n"
       "Address Family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Display routes matching the large-communities\n"
       "List of large-community numbers\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, ip_ospf_authentication_args_addr_cmd_vtysh, 
       "ip ospf authentication <null|message-digest> [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Use null authentication\n"
       "Use message-digest authentication\n"
       "Address of interface\n")

DEFSH (VTYSH_BGPD, no_neighbor_update_source_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> update-source [<A.B.C.D|X:X::X:X|WORD>]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Source of routing updates\n"
       "IPv4 address\n" "IPv6 address\n" "Interface name (requires zebra to be running)\n")

DEFSH (VTYSH_PIMD, interface_ip_pim_drprio_cmd_vtysh, 
       "ip pim drpriority (1-4294967295)", 
       "IP information\n"
       "PIM information\n"
       "Set the Designated Router Election Priority\n"
       "Value of the new DR Priority\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_id_self_originated_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> A.B.C.D self-originated [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Display Self-originated LSAs\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_OSPFD, debug_ospf_zebra_cmd_vtysh, 
       "debug ospf zebra [<interface|redistribute>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF Zebra information\n"
       "Zebra interface\n"
       "Zebra redistribute\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_metric_cmd_vtysh, 
       "no match metric [(0-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match metric of route\n"
       "Metric value\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_asbr_cmd_vtysh, 
       "debug ospf6 asbr", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 ASBR function\n"
      )

DEFSH (VTYSH_BGPD, vnc_vrf_policy_rt_import_cmd_vtysh, 
       "rt import RTLIST...", 
       "Specify route targets\n"
       "Import filter\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_BGPD, bgp_network_import_check_cmd_vtysh, 
       "bgp network import-check", 
       "BGP specific commands\n"
       "BGP network command\n"
       "Check BGP network route exists in IGP\n")

DEFSH (VTYSH_NHRPD, no_tunnel_protection_cmd_vtysh, 
 "no tunnel protection", 
 "Negate a command or set its defaults\n"
 "NHRP/GRE integration\n"
 "IPsec protection\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_large_community_list_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [<ipv4|ipv6> [<unicast|multicast|vpn>]] large-community-list <(1-500)|WORD> [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\n"
       "Address Family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Display routes matching the large-community-list\n"
       "large-community-list number\n"
       "large-community-list name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_neighbor_dont_capability_negotiate_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> dont-capability-negotiate", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Do not perform capability negotiation\n")

DEFSH (VTYSH_ISISD, no_isis_passive_cmd_vtysh, 
       "no isis passive", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Configure the passive mode for interface\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_nht_cmd_vtysh, 
       "no debug zebra nht", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra next hop tracking\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_instance_all_cmd_vtysh, 
       "show [ip] bgp <view|vrf> all [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nAll Views/VRFs\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_bgp_update_delay_cmd_vtysh, 
       "no update-delay [(0-3600) [(1-3600)]]", 
       "Negate a command or set its defaults\n"
       "Force initial delay for best-path and updates\n"
       "Seconds\n"
       "Wait for peers to be established\n")

DEFSH (VTYSH_OSPF6D, no_area_import_list_cmd_vtysh, 
       "no area A.B.C.D import-list NAME", 
       "Negate a command or set its defaults\n"
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_ospf_rmap_cmd_vtysh, 
       "redistribute <ospf|table> (1-65535) route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_PIMD, show_ip_igmp_join_cmd_vtysh, 
       "show ip igmp join", 
       "Show running system information\n"
       "IP information\n"
       "IGMP information\n"
       "IGMP static join information\n")

DEFSH (VTYSH_BGPD, set_lcommunity_cmd_vtysh, 
       "set large-community AA:BB:CC...", 
       "Set values in destination routing protocol\n"
       "BGP large community attribute\n"
       "Large Community number in aa:bb:cc format or additive\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_le_cmd_vtysh, 
       "ip prefix-list WORD <deny|permit> A.B.C.D/M le (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_PIMD, no_ip_pim_ssm_prefix_list_name_cmd_vtysh, 
       "no ip pim ssm prefix-list WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Source Specific Multicast\n"
       "group range prefix-list filter\n"
       "Name of a prefix-list\n")

DEFSH (VTYSH_OSPFD|VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_ip_next_hop_cmd_vtysh, 
       "match ip next-hop <(1-199)|(1300-2699)|WORD>", 
       "Match values from routing table\n"
       "IP information\n"
       "Match next-hop address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

DEFSH (VTYSH_PIMD, no_debug_pim_packets_cmd_vtysh, 
       "no debug pim packets [<hello|joins|register>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM protocol packets\n"
       "PIM Hello protocol packets\n"
       "PIM Join/Prune protocol packets\n"
       "PIM Register/Reg-Stop protocol packets\n")

DEFSH (VTYSH_BGPD, vnc_redistribute_lifetime_cmd_vtysh, 
       "vnc redistribute lifetime <LIFETIME|infinite>", 
       "VNC/RFP related configuration\n"
       "Redistribute\n"
       "Assign a lifetime to routes redistributed from another routing protocol\n"
       "lifetime value (32 bit)\n"
       "Allow lifetime to never expire\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_protocol_cmd_vtysh, 
       "show ipv6 protocol", 
        "Show running system information\n"
        "IPv6 Information\n"
       "IPv6 protocol filtering status\n")

DEFSH (VTYSH_ALL, no_access_list_all_cmd_vtysh, 
       "no access-list <(1-99)|(100-199)|(1300-1999)|(2000-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list name\n")

DEFSH (VTYSH_ZEBRA, ip_route_mask_tag_distance_label_cmd_vtysh, 
       "ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> tag (1-4294967295) (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPFD, no_ospf_max_metric_router_lsa_startup_cmd_vtysh, 
       "no max-metric router-lsa on-startup [(5-86400)]", 
       "Negate a command or set its defaults\n"
       "OSPF maximum / infinite-distance metric\n"
       "Advertise own Router-LSA with infinite distance (stub router)\n"
       "Automatically advertise stub Router-LSA on startup of OSPF\n"
       "Time (seconds) to advertise self as stub-router\n")

DEFSH (VTYSH_OSPFD, no_ospf_default_metric_cmd_vtysh, 
       "no default-metric [(0-16777214)]", 
       "Negate a command or set its defaults\n"
       "Set metric of redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_PIMD, clear_ip_pim_interfaces_cmd_vtysh, 
       "clear ip pim interfaces", 
       "Reset functions\n"
       "IP information\n"
       "PIM clear commands\n"
       "Reset PIM interfaces\n")

DEFSH (VTYSH_RIPD, ip_rip_receive_version_cmd_vtysh, 
       "ip rip receive version <(1-2)|none>", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement reception\n"
       "Version control\n"
       "RIP version 1\n"
       "RIP version 2\n"
       "None\n")

DEFSH (VTYSH_ISISD, spf_delay_ietf_cmd_vtysh, 
       "spf-delay-ietf init-delay (0-60000) short-delay (0-60000) long-delay (0-60000) holddown (0-60000) time-to-learn (0-60000)", 
       "IETF SPF delay algorithm\n"
       "Delay used while in QUIET state\n"
       "Delay used while in QUIET state in milliseconds\n"
       "Delay used while in SHORT_WAIT state\n"
       "Delay used while in SHORT_WAIT state in milliseconds\n"
       "Delay used while in LONG_WAIT\n"
       "Delay used while in LONG_WAIT state in milliseconds\n"
       "Time with no received IGP events before considering IGP stable\n"
       "Time with no received IGP events before considering IGP stable (in milliseconds)\n"
       "Maximum duration needed to learn all the events related to a single failure\n"
       "Maximum duration needed to learn all the events related to a single failure (in milliseconds)\n")

DEFSH (VTYSH_ISISD, no_debug_isis_adj_cmd_vtysh, 
       "no debug isis adj-packets", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS Adjacency related packets\n")

DEFSH (VTYSH_ZEBRA, no_ip_address_label_cmd_vtysh, 
       "no ip address A.B.C.D/M label LINE", 
       "Negate a command or set its defaults\n"
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP address (e.g. 10.0.0.1/8)\n"
       "Label of this address\n"
       "Label\n")

DEFSH (VTYSH_OSPFD, debug_ospf_nsm_cmd_vtysh, 
       "debug ospf nsm [<status|events|timers>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF Neighbor State Machine\n"
       "NSM Status Information\n"
       "NSM Event Information\n"
       "NSM Timer Information\n")

DEFSH (VTYSH_BGPD, no_ip_as_path_cmd_vtysh, 
       "no ip as-path access-list WORD <deny|permit> LINE...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "BGP autonomous system path filter\n"
       "Specify an access list name\n"
       "Regular expression access list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "A regular-expression to match the BGP AS paths\n")

DEFSH (VTYSH_BGPD, debug_bgp_as4_segment_cmd_vtysh, 
       "debug bgp as4 segment", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP AS4 actions\n"
       "BGP AS4 aspath segment handling\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_ge_le_cmd_vtysh, 
       "ipv6 prefix-list WORD <deny|permit> X:X::X:X/M ge (0-128) le (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_ISISD, no_isis_priority_cmd_vtysh, 
       "no isis priority [(0-127)]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set priority for Designated Router election\n"
       "Priority value\n")

DEFSH (VTYSH_PIMD, no_debug_pim_zebra_cmd_vtysh, 
       "no debug pim zebra", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "ZEBRA protocol activity\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_log_adjacency_changes_cmd_vtysh, 
       "no log-adjacency-changes", 
       "Negate a command or set its defaults\n"
       "Log changes in adjacency state\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_database_max_cmd_vtysh, 
       "show ip ospf database <max-age|self-originate>", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Database summary\n"
       "LSAs in MaxAge list\n"
       "Self-originated link states\n")

DEFSH (VTYSH_LDPD, ldp_no_control_word_cword_cmd_vtysh, 
       "no control-word <exclude|include>", 
       "Negate a command or set its defaults\n"
       "Control-word options\n"
       "Exclude control-word in pseudowire packets\n"
       "Include control-word in pseudowire packets\n")

DEFSH (VTYSH_BGPD, bgp_default_local_preference_cmd_vtysh, 
       "bgp default local-preference (0-4294967295)", 
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "local preference (higher=more preferred)\n"
       "Configure default local preference value\n")

DEFSH (VTYSH_ISISD, no_debug_isis_spfstats_cmd_vtysh, 
       "no debug isis spf-statistics", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS SPF Timing and Statistic Data\n")

DEFSH (VTYSH_BGPD, clear_vnc_nve_vn_un_cmd_vtysh, 
       "clear vnc nve vn <*|A.B.C.D|X:X::X:X> un <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "VN address of NVE\n"
       "For all NVEs\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "For all UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_asbr_cmd_vtysh, 
       "no debug ospf6 asbr", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 ASBR function\n"
      )

DEFSH (VTYSH_ZEBRA, no_ip_irdp_address_preference_cmd_vtysh, 
       "no ip irdp address A.B.C.D preference (0-2147483647)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Alter ICMP Router discovery preference on this interface\n"
       "Select IRDP address\n"
       "IPv4 address\n"
       "Reset ICMP Router discovery preference on this interface\n"
       "Old preference level\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_ifname_pref_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_NHRPD, nhrp_event_socket_cmd_vtysh, 
 "nhrp event socket SOCKET", 
 "Next Hop Resolution Protocol functions\n"
 "Event Manager commands\n"
 "Event Manager unix socket path\n"
 "Unix path for the socket")

DEFSH (VTYSH_BGPD, no_bgp_bestpath_compare_router_id_cmd_vtysh, 
       "no bgp bestpath compare-routerid", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "Compare router-id for identical EBGP paths\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_prefix_longer_cmd_vtysh, 
       "show ip route vrf all A.B.C.D/M longer-prefixes", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Show route matching the specified Network/Mask pair only\n")

DEFSH (VTYSH_BGPD|VTYSH_BGPD, show_ip_bgp_updgrps_adj_cmd_vtysh, 
       "show [ip] bgp update-groups <advertise-queue|advertised-routes|packet-queue>", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Detailed info about dynamic update groups\n"
       "Advertisement queue\n"
       "Announced routes\n"
       "Packet queue\n")

DEFSH (VTYSH_BGPD, neighbor_set_peer_group_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> peer-group WORD", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Member of the peer-group\n"
       "Peer-group name\n")

DEFSH (VTYSH_LDPD, ldp_control_word_cword_cmd_vtysh, 
       "control-word <exclude|include>", 
       "Control-word options\n"
       "Exclude control-word in pseudowire packets\n"
       "Include control-word in pseudowire packets\n")

DEFSH (VTYSH_BGPD, bgp_network_mask_cmd_vtysh, 
       "network A.B.C.D mask A.B.C.D", 
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Network mask\n"
       "Network mask\n")

DEFSH (VTYSH_LDPD, ldp_no_pw_status_disable_cmd_vtysh, 
       "no pw-status disable", 
       "Negate a command or set its defaults\n"
       "Configure PW status\n"
       "Disable PW status\n")

DEFSH (VTYSH_OSPFD, ospf_area_default_cost_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> default-cost (0-16777215)", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Set the summary-default cost of a NSSA or stub area\n"
       "Stub's advertised default summary cost\n")

DEFSH (VTYSH_ISISD, show_isis_interface_cmd_vtysh, 
       "show isis interface", 
       "Show running system information\n"
       "ISIS network information\n"
       "ISIS interface\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_ipv6_targeted_cmd_vtysh, 
       "neighbor X:X::X:X targeted", 
       "Configure neighbor parameters\n"
       "IPv6 address of neighbor\n"
       "Establish targeted session\n")

DEFSH (VTYSH_ZEBRA, no_zebra_route_map_timer_cmd_vtysh, 
       "no zebra route-map delay-timer [(0-600)]", 
       "Negate a command or set its defaults\n"
       "Zebra information\n"
       "Set route-map parameters\n"
       "Reset delay-timer to default value,  30 secs\n"
       "0 means event-driven updates are disabled\n")

DEFSH (VTYSH_BGPD, no_neighbor_nexthop_self_force_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> next-hop-self force", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Disable the next hop calculation for this neighbor\n"
       "Set the next hop to self for reflected routes\n")

DEFSH (VTYSH_BGPD, no_neighbor_ebgp_multihop_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> ebgp-multihop [(1-255)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Allow EBGP neighbors not on directly connected networks\n"
       "maximum hop count\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_mask_cmd_vtysh, 
       "no ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Tag of this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_capabilities_json_cmd_vtysh, 
       "show mpls ldp capabilities [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Display LDP Capabilities information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_PIMD, ip_pim_packets_cmd_vtysh, 
       "ip pim packets (1-100)", 
       "IP information\n"
       "pim multicast routing\n"
       "packets to process at one time per fd\n"
       "Number of packets\n")

DEFSH (VTYSH_OSPFD, no_ospf_redistribute_source_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|rip|isis|bgp|pim|nhrp|table|vnc>" " [<metric (0-16777214)|metric-type (1-2)|route-map WORD>]", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_ISISD, no_net_cmd_vtysh, 
       "no net WORD", 
       "Negate a command or set its defaults\n"
       "A Network Entity Title for this process (OSI only)\n"
       "XX.XXXX. ... .XXX.XX  Network entity title (NET)\n")

DEFSH (VTYSH_RIPNGD, no_ripng_timers_cmd_vtysh, 
       "no timers basic [(0-65535) (0-65535) (0-65535)]", 
       "Negate a command or set its defaults\n"
       "RIPng timers setup\n"
       "Basic timer\n"
       "Routing table update timer value in second. Default is 30.\n"
       "Routing information timeout timer. Default is 180.\n"
       "Garbage collection timer. Default is 120.\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_priority_cmd_vtysh, 
       "no ip ospf priority [(0-255) [A.B.C.D]]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Router priority\n"
       "Priority\n"
       "Address of interface")

DEFSH (VTYSH_OSPFD, ospf_area_shortcut_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> shortcut <default|enable|disable>", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure the area's shortcutting mode\n"
       "Set default shortcutting behavior\n"
       "Enable shortcutting through the area\n"
       "Disable shortcutting through the area\n")

DEFSH (VTYSH_BGPD, neighbor_maximum_prefix_restart_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix (1-4294967295) restart (1-65535)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Restart bgp connection after limit is exceeded\n"
       "Restart interval in minutes")

DEFSH (VTYSH_ISISD, no_csnp_interval_l2_cmd_vtysh, 
       "no isis csnp-interval [(1-600)] level-2", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set CSNP interval in seconds\n"
       "CSNP interval value\n"
       "Specify interval for level-2 CSNPs\n")

DEFSH (VTYSH_ISISD, no_isis_circuit_type_cmd_vtysh, 
       "no isis circuit-type <level-1|level-1-2|level-2-only>", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Configure circuit type for interface\n"
       "Level-1 only adjacencies are formed\n"
       "Level-1-2 adjacencies are formed\n"
       "Level-2 only adjacencies are formed\n")

DEFSH (VTYSH_RIPD, rip_distance_cmd_vtysh, 
       "distance (1-255)", 
       "Administrative distance\n"
       "Distance value\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_pref_tag_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> tag (1-4294967295) (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_RIPD, no_rip_redistribute_rip_cmd_vtysh, 
       "no redistribute rip", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Routing Information Protocol (RIP)\n")

DEFSH (VTYSH_OSPF6D, ospf6_redistribute_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|isis|bgp|nhrp|table|vnc>", 
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_cmd_vtysh, 
       "no debug bgp", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n")

DEFSH (VTYSH_ZEBRA, ipv6_protocol_cmd_vtysh, 
       "ipv6 protocol " "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|any>" " route-map ROUTE-MAP", 
       "IPv6 Information\n"
       "Filter IPv6 routing info exchanged between zebra and protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_tag_cmd_vtysh, 
       "show ipv6 route vrf all tag (1-4294967295)", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Show only routes with tag\n"
       "Tag value\n")

DEFSH (VTYSH_BGPD, no_neighbor_interface_config_cmd_vtysh, 
       "no neighbor WORD interface [v6only] [peer-group WORD] [remote-as <(1-4294967295)|internal|external>]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Interface name\n"
       "Configure BGP on interface\n"
       "Enable BGP with v6 link-local only\n"
       "Member of the peer-group\n"
       "Peer-group name\n"
       "Specify a BGP neighbor\n"
       "AS number\n"
       "Internal BGP peer\n"
       "External BGP peer\n")

DEFSH (VTYSH_ISISD, area_lsp_mtu_cmd_vtysh, 
       "lsp-mtu (128-4352)", 
       "Configure the maximum size of generated LSPs\n"
       "Maximum size of generated LSPs\n")

DEFSH (VTYSH_BGPD, clear_vrf_all_cmd_vtysh, 
       "clear vrf NAME all", 
       "Clear stored data\n"
       "From a VRF\n"
       "VRF name\n"
       "All prefixes\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_shutdown_cmd_vtysh, 
       "ip irdp shutdown", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "ICMP Router discovery shutdown on this interface\n")

DEFSH (VTYSH_RIPD, debug_rip_packet_direct_cmd_vtysh, 
       "debug rip packet <recv|send>", 
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP packet\n"
       "RIP receive packet\n"
       "RIP send packet\n")

DEFSH (VTYSH_ZEBRA, no_ip_multicast_mode_cmd_vtysh, 
       "no ip multicast rpf-lookup-mode [<urib-only|mrib-only|mrib-then-urib|lower-distance|longer-prefix>]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Multicast options\n"
       "RPF lookup behavior\n"
       "Lookup in unicast RIB only\n"
       "Lookup in multicast RIB only\n"
       "Try multicast RIB first,  fall back to unicast RIB\n"
       "Lookup both,  use entry with lower distance\n"
       "Lookup both,  use entry with longer prefix\n")

DEFSH (VTYSH_BGPD, ip_extcommunity_list_name_expanded_cmd_vtysh, 
       "ip extcommunity-list <(100-500)|expanded WORD> <deny|permit> LINE...", 
       "IP information\n"
       "Add a extended community list entry\n"
       "Extended Community list number (expanded)\n"
       "Specify expanded extcommunity-list\n"
       "Extended Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "An ordered list as a regular-expression\n")

DEFSH (VTYSH_PIMD, debug_mroute_detail_cmd_vtysh, 
       "debug mroute detail", 
       "Debugging functions (see also 'undebug')\n"
       "PIM interaction with kernel MFC cache\n"
       "detailed\n")

DEFSH (VTYSH_BGPD, no_synchronization_cmd_vtysh, 
       "no synchronization", 
       "Negate a command or set its defaults\n"
       "Perform IGP synchronization\n")

DEFSH (VTYSH_RIPD, rip_distance_source_access_list_cmd_vtysh, 
       "distance (1-255) A.B.C.D/M WORD", 
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n"
       "Access list name\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_spf_time_cmd_vtysh, 
       "no debug ospf6 spf time", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Quit Debugging SPF Calculation\n"
       "Quit Measuring time taken by SPF Calculation\n"
      )

DEFSH (VTYSH_BGPD, neighbor_local_as_no_prepend_replace_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> local-as (1-4294967295) no-prepend replace-as", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Specify a local-as number\n"
       "AS number used as local AS\n"
       "Do not prepend local-as to updates from ebgp peers\n"
       "Do not prepend local-as to updates from ibgp peers\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, clear_ip_prefix_list_name_prefix_cmd_vtysh, 
       "clear ip prefix-list WORD A.B.C.D/M", 
       "Reset functions\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n")

DEFSH (VTYSH_BGPD, match_peer_local_cmd_vtysh, 
        "match peer local", 
        "Match values from routing table\n"
        "Match peer address\n"
        "Static or Redistributed routes\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_prefix_cmd_vtysh, 
       "no ip prefix-list WORD <deny|permit> <A.B.C.D/M|any>", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Any prefix match.  Same as \"0.0.0.0/0 le 32\"\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_binding_json_cmd_vtysh, 
       "show mpls ldp binding json", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Label Information Base (LIB) information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, no_isis_default_originate_cmd_vtysh, 
       "no default-information originate <ipv4|ipv6> <level-1|level-2>", 
       "Negate a command or set its defaults\n"
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Distribute default route for IPv4\n"
       "Distribute default route for IPv6\n"
       "Distribute default route into level-1\n"
       "Distribute default route into level-2\n")

DEFSH (VTYSH_ALL, no_access_list_standard_nomask_cmd_vtysh, 
       "no access-list <(1-99)|(1300-1999)> <deny|permit> A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n")

DEFSH (VTYSH_BGPD, match_ecommunity_cmd_vtysh, 
       "match extcommunity <(1-99)|(100-500)|WORD>", 
       "Match values from routing table\n"
       "Match BGP/VPN extended community list\n"
       "Extended community-list number (standard)\n"
       "Extended community-list number (expanded)\n"
       "Extended community-list name\n")

DEFSH (VTYSH_OSPFD, no_ospf_opaque_cmd_vtysh, 
       "no ospf opaque-lsa", 
       "Negate a command or set its defaults\n"
       "OSPF specific commands\n"
       "Enable the Opaque-LSA capability (rfc2370)\n")

DEFSH (VTYSH_RIPD, ip_rip_receive_version_1_cmd_vtysh, 
       "ip rip receive version (1-1) (2-2)", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement reception\n"
       "Version control\n"
       "RIP version 1\n"
       "RIP version 2\n")

DEFSH (VTYSH_OSPFD, ip_ospf_area_cmd_vtysh, 
       "ip ospf [(1-65535)] area <A.B.C.D|(0-4294967295)>", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Instance ID\n"
       "Enable OSPF on this interface\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_authentication_addr_cmd_vtysh, 
       "no ip ospf authentication [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Address of interface")

DEFSH (VTYSH_PIMD, no_debug_pim_packetdump_send_cmd_vtysh, 
       "no debug pim packet-dump send", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM packet dump\n"
       "Dump sent packets\n")

DEFSH (VTYSH_BGPD, clear_ip_bgp_dampening_address_mask_cmd_vtysh, 
       "clear ip bgp dampening A.B.C.D A.B.C.D", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "Clear route flap dampening information\n"
       "Network to clear damping information\n"
       "Network mask\n")

DEFSH (VTYSH_ALL, no_ipv6_access_list_remark_comment_cmd_vtysh, 
       "no ipv6 access-list WORD remark LINE...", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")

DEFSH (VTYSH_ISISD, csnp_interval_l1_cmd_vtysh, 
       "isis csnp-interval (1-600) level-1", 
       "IS-IS commands\n"
       "Set CSNP interval in seconds\n"
       "CSNP interval value\n"
       "Specify interval for level-1 CSNPs\n")

DEFSH (VTYSH_OSPFD, no_ospf_network_area_cmd_vtysh, 
       "no network A.B.C.D/M area <A.B.C.D|(0-4294967295)>", 
       "Negate a command or set its defaults\n"
       "Enable routing on an IP network\n"
       "OSPF network prefix\n"
       "Set the OSPF area ID\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n")

DEFSH (VTYSH_BGPD, neighbor_send_community_type_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> send-community <both|all|extended|standard|large>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Send Community attribute to this neighbor\n"
       "Send Standard and Extended Community attributes\n"
       "Send Standard,  Large and Extended Community attributes\n"
       "Send Extended Community attributes\n"
       "Send Standard Community attributes\n"
       "Send Large Community attributes\n")

DEFSH (VTYSH_BGPD, no_ip_community_list_expanded_all_cmd_vtysh, 
       "no ip community-list <(100-500)|expanded WORD> <deny|permit> AA:NN...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a community list entry\n"
       "Community list number (expanded)\n"
       "Add an expanded community-list entry\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Community number in AA:NN format (where AA and NN are (0-65535)) or local-AS|no-advertise|no-export|internet or additive\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_distance_ospf6_cmd_vtysh, 
       "no distance ospf6 [{intra-area [(1-255)]|inter-area [(1-255)]|external [(1-255)]}]", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "OSPF6 distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n")

DEFSH (VTYSH_RIPD, no_rip_redistribute_type_routemap_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>" " route-map WORD", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_summary_cmd_vtysh, 
       "show ip prefix-list summary", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Summary of prefix lists\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_ge_cmd_vtysh, 
       "no ipv6 prefix-list WORD <deny|permit> X:X::X:X/M ge (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_BGPD, vnc_vrf_policy_no_label_cmd_vtysh, 
       "no label", 
       "Negate a command or set its defaults\n"
       "Remove VRF default label\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_cmd_vtysh, 
       "no ipv6 prefix-list WORD", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_OSPF6D, ospf6_stub_router_admin_cmd_vtysh, 
       "stub-router administrative", 
       "Make router a stub router\n"
       "Advertise inability to be a transit router\n"
       "Administratively applied,  for an indefinite period\n")

DEFSH (VTYSH_RIPNGD, no_debug_ripng_events_cmd_vtysh, 
       "no debug ripng events", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng events\n")

DEFSH (VTYSH_RIPNGD, no_ripng_default_metric_cmd_vtysh, 
       "no default-metric [(1-16)]", 
       "Negate a command or set its defaults\n"
       "Set a metric of redistribute routes\n"
       "Default metric\n")

DEFSH (VTYSH_OSPFD|VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_ip_next_hop_cmd_vtysh, 
       "no match ip next-hop [<(1-199)|(1300-2699)|WORD>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match next-hop address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

DEFSH (VTYSH_ISISD, metric_style_cmd_vtysh, 
       "metric-style <narrow|transition|wide>", 
       "Use old-style (ISO 10589) or new-style packet formats\n"
       "Use old style of TLVs with narrow metric\n"
       "Send and accept both styles of TLVs during transition\n"
       "Use new style of TLVs to carry wider metric\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_description_cmd_vtysh, 
       "no ip prefix-list WORD description", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Prefix-list specific description\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, rmap_show_name_cmd_vtysh, 
       "show route-map [WORD]", 
       "Show running system information\n"
       "route-map information\n"
       "route-map name\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_as4_segment_cmd_vtysh, 
       "no debug bgp as4 segment", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP AS4 actions\n"
       "BGP AS4 aspath segment handling\n")

DEFSH (VTYSH_ZEBRA, no_ip_mroute_dist_cmd_vtysh, 
       "no ip mroute A.B.C.D/M <A.B.C.D|INTERFACE> [(1-255)]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Configure static unicast route into MRIB for multicast RPF lookup\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Nexthop address\n"
       "Nexthop interface name\n"
       "Distance\n")

DEFSH (VTYSH_BGPD, show_bgp_updgrps_stats_cmd_vtysh, 
       "show [ip] bgp update-groups statistics", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Detailed info about dynamic update groups\n"
       "Statistics\n")

DEFSH (VTYSH_BGPD, vnc_export_mode_cmd_vtysh, 
       "vnc export <bgp|zebra> mode <group-nve|ce|none|registering-nve>", 
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "Select export mode\n"
       "Export routes with nve-group next-hops\n"
       "Export routes with NVE connected router next-hops\n"
       "Disable export\n" "Export routes with registering NVE as next-hop\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_ipv4_targeted_cmd_vtysh, 
       "neighbor A.B.C.D targeted", 
       "Configure neighbor parameters\n"
       "IP address of neighbor\n"
       "Establish targeted session\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_instance_nsm_cmd_vtysh, 
       "no debug ospf (1-65535) nsm [<status|events|timers>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Neighbor State Machine\n"
       "NSM Status Information\n"
       "NSM Event Information\n"
       "NSM Timer Information\n")

DEFSH (VTYSH_ISISD, debug_isis_snp_cmd_vtysh, 
       "debug isis snp-packets", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS CSNP/PSNP packets\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_to_acl_cmd_vtysh, 
       "no label local advertise to <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP Access-list specifying controls on LDP Peers\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ZEBRA, no_link_params_enable_cmd_vtysh, 
       "no enable", 
       "Negate a command or set its defaults\n"
       "Disable link parameters on this interface\n")

DEFSH (VTYSH_BGPD, ipv6_aggregate_address_cmd_vtysh, 
       "aggregate-address X:X::X:X/M [summary-only]", 
       "Configure BGP aggregate entries\n"
       "Aggregate prefix\n"
       "Filter more specific routes from updates\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_cost_cmd_vtysh, 
              "ospf cost (1-65535) [A.B.C.D]", 
              "OSPF interface commands\n"
              "Interface cost\n"
              "Cost\n"
              "Address of interface\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_le_cmd_vtysh, 
       "no ipv6 prefix-list WORD <deny|permit> X:X::X:X/M le (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_lnh_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> LNH_OPTIONS...", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_BGPD, no_aggregate_address_cmd_vtysh, 
       "no aggregate-address A.B.C.D/M [<as-set [summary-only]|summary-only [as-set]>]", 
       "Negate a command or set its defaults\n"
       "Configure BGP aggregate entries\n"
       "Aggregate prefix\n"
       "Generate AS set path information\n"
       "Filter more specific routes from updates\n"
       "Filter more specific routes from updates\n"
       "Generate AS set path information\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_seq_ge_cmd_vtysh, 
       "ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M ge (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_area_stub_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> stub", 
       "Negate a command or set its defaults\n"
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "OSPF6 area ID as a decimal value\n"
       "Configure OSPF6 area as stub\n")

DEFSH (VTYSH_BGPD, no_set_lcommunity1_cmd_vtysh, 
       "no set large-community AA:BB:CC...", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP large community attribute\n"
       "Large community in AA:BB:CC... format or additive\n")

DEFSH (VTYSH_ISISD, psnp_interval_l1_cmd_vtysh, 
       "isis psnp-interval (1-120) level-1", 
       "IS-IS commands\n"
       "Set PSNP interval in seconds\n"
       "PSNP interval value\n"
       "Specify interval for level-1 PSNPs\n")

DEFSH (VTYSH_BGPD, vnc_l2_group_no_labels_cmd_vtysh, 
       "no labels LABELLIST...", 
       "Negate a command or set its defaults\n"
       "Remove label values associated with L2 group\n"
       "Specify label values associated with L2 group\n"
       "Space separated list of label values <0-1048575>\n")

DEFSH (VTYSH_ZEBRA, no_ip_protocol_nht_rmap_cmd_vtysh, 
       "no ip nht " "<kernel|connected|static|rip|ospf|isis|bgp|pim|nhrp|table|vnc|any>" " [route-map ROUTE-MAP]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Filter Next Hop tracking route resolution\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_ZEBRA, ip_protocol_cmd_vtysh, 
       "ip protocol " "<kernel|connected|static|rip|ospf|isis|bgp|pim|nhrp|table|vnc|any>" " route-map ROUTE-MAP", 
       "IP information\n"
       "Filter routing info exchanged between zebra and protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route-map\n"
       "Route map name\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_seq_ge_le_cmd_vtysh, 
       "ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M ge (0-32) le (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_ZEBRA, link_params_metric_cmd_vtysh, 
       "metric (0-4294967295)", 
       "Link metric for MPLS-TE purpose\n"
       "Metric value in decimal\n")

DEFSH (VTYSH_OSPFD, ospf_distribute_list_out_cmd_vtysh, 
       "distribute-list WORD out " "<kernel|connected|static|rip|isis|bgp|pim|nhrp|table|vnc>", 
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter outgoing routing updates\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_BGPD, bgp_maxpaths_cmd_vtysh, 
       "maximum-paths (1-255)", 
       "Forward packets over multiple paths\n"
       "Number of paths\n")

DEFSH (VTYSH_ZEBRA, router_id_cmd_vtysh, 
       "router-id A.B.C.D [vrf NAME]", 
       "Manually set the router-id\n"
       "IP address to use for router-id\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_OSPFD, ospf_router_id_cmd_vtysh, 
       "ospf router-id A.B.C.D", 
       "OSPF specific commands\n"
       "router-id for the OSPF process\n"
       "OSPF router-id in IP address format\n")

DEFSH (VTYSH_BGPD, ipv6_bgp_network_cmd_vtysh, 
       "network X:X::X:X/M", 
       "Specify a network to announce via BGP\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_ZEBRA, mpls_transit_lsp_cmd_vtysh, 
       "mpls lsp (16-1048575) <A.B.C.D|X:X::X:X> <(16-1048575)|explicit-null|implicit-null>", 
       "MPLS information\n"
       "Establish label switched path\n"
       "Incoming MPLS label\n"
       "IPv4 gateway address\n"
       "IPv6 gateway address\n"
       "Outgoing MPLS label\n"
       "Use Explicit-Null label\n"
       "Use Implicit-Null label\n")

DEFSH (VTYSH_ALL, access_list_remark_cmd_vtysh, 
       "access-list <(1-99)|(100-199)|(1300-1999)|(2000-2699)|WORD> remark LINE...", 
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")

DEFSH (VTYSH_ZEBRA, set_src_cmd_vtysh, 
       "set src <A.B.C.D|X:X::X:X>", 
       "Set values in destination routing protocol\n"
       "src address for route\n"
       "IPv4 src address\n"
       "IPv6 src address\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_route_map_cmd_vtysh, 
       "no route-map WORD <deny|permit> (1-65535)", 
       "Negate a command or set its defaults\n"
       "Create route-map or enter route-map command mode\n"
       "Route map tag\n"
       "Route map denies set operations\n"
       "Route map permits set operations\n"
       "Sequence to insert to/delete from existing route-map entry\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_minadvertinterval_cmd_vtysh, 
       "ip irdp minadvertinterval (3-1800)", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "Set minimum time between advertisement\n"
       "Minimum advertisement interval in seconds\n")

DEFSH (VTYSH_RIPNGD, debug_ripng_packet_direct_cmd_vtysh, 
       "debug ripng packet <recv|send>", 
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")

DEFSH (VTYSH_ALL, vty_login_cmd_vtysh, 
       "login", 
       "Enable password checking\n")

DEFSH (VTYSH_ISISD, no_debug_isis_lsp_gen_cmd_vtysh, 
       "no debug isis lsp-gen", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS generation of own LSPs\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_spf_database_cmd_vtysh, 
       "debug ospf6 spf database", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug SPF Calculation\n"
       "Log number of LSAs at SPF Calculation time\n"
      )

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_cmd_vtysh, 
       "no debug ospf6", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n")

DEFSH (VTYSH_RIPD, accept_lifetime_infinite_month_day_cmd_vtysh, 
       "accept-lifetime HH:MM:SS MONTH (1-31) (1993-2035) infinite", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Never expires")

DEFSH (VTYSH_ZEBRA, link_params_unrsv_bw_cmd_vtysh, 
       "unrsv-bw (0-7) BANDWIDTH", 
       "Unreserved bandwidth at each priority level\n"
       "Priority\n"
       "Bytes/second (IEEE floating point format)\n")

DEFSH (VTYSH_OSPFD, no_ospf_distance_cmd_vtysh, 
       "no distance (1-255)", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "OSPF Administrative distance\n")

DEFSH (VTYSH_ISISD, isis_network_cmd_vtysh, 
       "isis network point-to-point", 
       "IS-IS commands\n"
       "Set network type\n"
       "point-to-point network type\n")

DEFSH (VTYSH_OSPFD, ospf_refresh_timer_cmd_vtysh, 
       "refresh timer (10-1800)", 
       "Adjust refresh parameters\n"
       "Set refresh timer\n"
       "Timer value in seconds\n")

DEFSH (VTYSH_ALL, no_ipv6_access_list_exact_cmd_vtysh, 
       "no ipv6 access-list WORD <deny|permit> X:X::X:X/M [exact-match]", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Exact match of the prefixes\n")

DEFSH (VTYSH_OSPF6D, ospf6_routemap_set_forwarding_cmd_vtysh, 
       "set forwarding-address X:X::X:X", 
       "Set value\n"
       "Forwarding Address\n"
       "IPv6 Address\n")

DEFSH (VTYSH_LDPD, ldp_discovery_hello_interval_disc_time_cmd_vtysh, 
       "discovery hello interval (1-65535)", 
       "Configure discovery parameters\n"
       "LDP Link Hellos\n"
       "Hello interval\n"
       "Time (seconds)\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_cmd_vtysh, 
       "show ip ospf [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_cost_life_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> cost (0-255) lifetime (1-4294967295)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n")

DEFSH (VTYSH_RIPD, no_rip_redistribute_type_metric_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>" " metric (0-16)", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n")

DEFSH (VTYSH_PIMD, show_ip_pim_state_cmd_vtysh, 
       "show ip pim state [A.B.C.D [A.B.C.D]] [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM state information\n"
       "Unicast or Multicast address\n"
       "Multicast address\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_retransmitinterval_cmd_vtysh, 
       "ipv6 ospf6 retransmit-interval (1-65535)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"
       )

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_rmap_description_cmd_vtysh, 
       "no description", 
       "Negate a command or set its defaults\n"
       "Route-map comment\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_preference_cmd_vtysh, 
       "ip irdp preference (0-2147483647)", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "Set default preference level for this interface\n"
       "Preference level\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_to_acl_for_acl_cmd_vtysh, 
       "label local advertise to <(1-199)|(1300-2699)|WORD> for <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "IP Access-list specifying controls on LDP Peers\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_BGPD, no_neighbor_allowas_in_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> allowas-in [<(1-10)|origin>]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "allow local ASN appears in aspath attribute\n"
       "Number of occurances of AS number\n"
       "Only accept my AS in the as-path if the route was originated in my AS\n")

DEFSH (VTYSH_ZEBRA, fpm_remote_ip_cmd_vtysh, 
        "fpm connection ip A.B.C.D port (1-65535)", 
        "fpm connection remote ip and port\n"
        "Remote fpm server ip A.B.C.D\n"
        "Enter ip ")

DEFSH (VTYSH_OSPFD, show_ip_ospf_neighbor_detail_cmd_vtysh, 
       "show ip ospf neighbor detail [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Neighbor list\n"
       "detail of all neighbors\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, isis_passive_cmd_vtysh, 
       "isis passive", 
       "IS-IS commands\n"
       "Configure the passive mode for interface\n")

DEFSH (VTYSH_OSPFD, no_router_info_cmd_vtysh, 
       "no router-info", 
       "Negate a command or set its defaults\n"
       "Disable the Router Information functionality\n")

DEFSH (VTYSH_BGPD, no_bgp_default_local_preference_cmd_vtysh, 
       "no bgp default local-preference [(0-4294967295)]", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "local preference (higher=more preferred)\n"
       "Configure default local preference value\n")

DEFSH (VTYSH_ZEBRA, ipv6_address_cmd_vtysh, 
       "ipv6 address X:X::X:X/M", 
       "Interface IPv6 config commands\n"
       "Set the IP address of an interface\n"
       "IPv6 address (e.g. 3ffe:506::1/48)\n")

DEFSH (VTYSH_PIMD, no_ip_pim_spt_switchover_infinity_plist_cmd_vtysh, 
       "no ip pim spt-switchover infinity-and-beyond prefix-list WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "PIM information\n"
       "SPT_Switchover\n"
       "Never switch to SPT Tree\n"
       "Prefix-List to control which groups to switch\n"
       "Prefix-List name\n")

/* upgrade frr to 3.0, modified by liwh for bug 46090, 2018-01-22 */
#ifdef _CENTEC_
DEFSH (VTYSH_ZEBRA, no_bandwidth_if_cmd_vtysh, 
       "no bandwidth [(1-10000000)]", 
       "Negate a command or set its defaults\n"
       "Set bandwidth informational parameter\n"
       "Bandwidth in megabits\n")
#else
DEFSH (VTYSH_ZEBRA, no_bandwidth_if_cmd_vtysh, 
       "no bandwidth [(1-100000)]", 
       "Negate a command or set its defaults\n"
       "Set bandwidth informational parameter\n"
       "Bandwidth in megabits\n")
#endif
/*liwh end */

DEFSH (VTYSH_ISISD, isis_mpls_te_router_addr_cmd_vtysh, 
       "mpls-te router-address A.B.C.D", 
       "MPLS-TE specific commands\n"
       "Stable IP address of the advertising router\n"
       "MPLS-TE router address in IPv4 address format\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_nsm_cmd_vtysh, 
       "no debug ospf nsm [<status|events|timers>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF Neighbor State Machine\n"
       "NSM Status Information\n"
       "NSM Event Information\n"
       "NSM Timer Information\n")

DEFSH (VTYSH_BGPD, vnc_defaults_rt_import_cmd_vtysh, 
       "rt import RTLIST...", 
       "Specify default route targets\n"
       "Import filter\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_all_route_prefix_cmd_vtysh, 
       "show [ip] bgp l2vpn evpn all <A.B.C.D|A.B.C.D/M> [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Layer 2 Virtual Private Network\n"
       "Ethernet Virtual Private Network\n"
       "Display information about all EVPN NLRIs\n"
       "Network in the BGP routing table to display\n"
       "Network in the BGP routing table to display\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_spf_process_cmd_vtysh, 
       "no debug ospf6 spf process", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Quit Debugging SPF Calculation\n"
       "Quit Debugging Detailed SPF Process\n"
      )

DEFSH (VTYSH_BGPD, neighbor_nexthop_self_force_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> next-hop-self force", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Disable the next hop calculation for this neighbor\n"
       "Set the next hop to self for reflected routes\n")

DEFSH (VTYSH_NHRPD, show_debugging_nhrp_cmd_vtysh, 
 "show debugging nhrp", 
 "Show running system information\n"
 "Debugging information\n"
 "NHRP configuration\n")

DEFSH (VTYSH_RIPNGD, ripng_redistribute_type_routemap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf6|isis|bgp|nhrp|table|vnc>" " route-map WORD", 
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_ISISD, show_isis_summary_cmd_vtysh, 
       "show isis summary", 
       "Show running system information\n" "IS-IS information\n" "IS-IS summary\n")

DEFSH (VTYSH_ALL, access_list_extended_any_mask_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip any A.B.C.D A.B.C.D", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Destination address\n"
       "Destination Wildcard bits\n")

DEFSH (VTYSH_ALL, log_commands_cmd_vtysh, 
       "log commands", 
       "Logging control\n"
       "Log all commands (can't be unset without restart)\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_all_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> *", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "From any NVE\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_suppress_ra_cmd_vtysh, 
       "ipv6 nd suppress-ra", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Suppress Router Advertisement\n")

DEFSH (VTYSH_PIMD, no_debug_ssmpingd_cmd_vtysh, 
       "no debug ssmpingd", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "ssmpingd activity\n")

DEFSH (VTYSH_BGPD, neighbor_maximum_prefix_threshold_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix (1-4294967295) (1-100)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Threshold value (%) at which to generate a warning msg\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_self_originated_cmd_vtysh, 
       "show ipv6 ospf6 database self-originated [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Self-originated LSAs\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_BGPD, vnc_redist_bgpdirect_prefixlist_cmd_vtysh, 
       "vnc redistribute <bgp-direct|bgp-direct-to-nve-groups> <ipv4|ipv6> prefix-list NAME", 
       "VNC/RFP related configuration\n"
       "Redistribute from other protocol\n"
       "Redistribute from BGP directly\n"
       "Redistribute from BGP without Zebra,  only to configured NVE groups\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "Prefix-list for filtering redistributed routes\n"
       "prefix list name\n")

DEFSH (VTYSH_ZEBRA, no_link_params_use_bw_cmd_vtysh, 
       "no use-bw", 
       "Negate a command or set its defaults\n"
       "Disable Unidirectional Utilised Bandwidth on this interface\n")

DEFSH (VTYSH_BGPD, set_atomic_aggregate_cmd_vtysh, 
       "set atomic-aggregate", 
       "Set values in destination routing protocol\n"
       "BGP atomic aggregate attribute\n" )

DEFSH (VTYSH_ZEBRA, no_link_params_ava_bw_cmd_vtysh, 
       "no ava-bw", 
       "Negate a command or set its defaults\n"
       "Disable Unidirectional Available Bandwidth on this interface\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_broadcast_cmd_vtysh, 
       "ip irdp broadcast", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "Use broadcast mode\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_ip_address_cmd_vtysh, 
       "match ip address <(1-199)|(1300-2699)|WORD>", 
       "Match values from routing table\n"
       "IP information\n"
       "Match address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

DEFSH (VTYSH_ISISD, log_adj_changes_cmd_vtysh, 
       "log-adjacency-changes", 
       "Log changes in adjacency state\n")

DEFSH (VTYSH_PIMD, no_ip_pim_packets_cmd_vtysh, 
       "no ip pim packets (1-100)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "packets to process at one time per fd\n"
       "Number of packets\n")

DEFSH (VTYSH_OSPFD, ospf_auto_cost_reference_bandwidth_cmd_vtysh, 
       "auto-cost reference-bandwidth (1-4294967)", 
       "Calculate OSPF interface cost according to bandwidth\n"
       "Use reference bandwidth method to assign OSPF cost\n"
       "The reference bandwidth in terms of Mbits per second\n")

DEFSH (VTYSH_OSPFD, no_ospf_neighbor_cmd_vtysh, 
       "no neighbor A.B.C.D [priority (0-255) [poll-interval (1-65525)]]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor IP address\n"
       "Neighbor Priority\n"
       "Priority\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_supernets_cmd_vtysh, 
       "show ip route vrf all supernets-only", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Show supernet entries only\n")

DEFSH (VTYSH_ZEBRA, ipv6_protocol_nht_rmap_cmd_vtysh, 
       "ipv6 nht " "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|any>" " route-map ROUTE-MAP", 
       "IPv6 Information\n"
       "Filter Next Hop tracking route resolution\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "Any of the above protocols\n"
       "Specify route map\n"
       "Route map name\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_le_ge_cmd_vtysh, 
       "no ipv6 prefix-list WORD <deny|permit> X:X::X:X/M le (0-128) ge (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_RIPNGD, clear_ipv6_rip_cmd_vtysh, 
       "clear ipv6 ripng", 
       "Reset functions\n"
       "IPv6 information\n"
       "Clear IPv6 RIP database")

DEFSH (VTYSH_RIPNGD, no_ripng_offset_list_cmd_vtysh, 
       "no offset-list WORD <in|out> (0-16)", 
       "Negate a command or set its defaults\n"
       "Modify RIPng metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n")

DEFSH (VTYSH_BGPD, show_bgp_updgrps_afi_adj_cmd_vtysh, 
       "show [ip] bgp ""<ipv4|ipv6>"" ""<unicast|multicast|vpn>"" update-groups <advertise-queue|advertised-routes|packet-queue>", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Address Family\nAddress Family\n" "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Detailed info about dynamic update groups\n"
       "Advertisement queue\n"
       "Announced routes\n"
       "Packet queue\n"
       "Specific subgroup info wanted for\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, rmap_continue_cmd_vtysh, 
       "continue (1-65535)", 
       "Continue on a different entry within the route-map\n"
       "Route-map entry sequence number\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_lsa_cmd_vtysh, 
       "no debug ospf lsa [<generate|flooding|install|refresh>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF Link State Advertisement\n"
       "LSA Generation\n"
       "LSA Flooding\n"
       "LSA Install/Delete\n"
       "LSA Refres\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nht_default_route_cmd_vtysh, 
       "no ipv6 nht resolve-via-default", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Filter Next Hop tracking route resolution\n"
       "Resolve via default route\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_int_detail_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor IFNAME detail [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "Interface name\n"
       "detail of all neighbors\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_OSPFD, ospf_neighbor_cmd_vtysh, 
       "neighbor A.B.C.D [priority (0-255) [poll-interval (1-65535)]]", 
       "Specify neighbor router\n"
       "Neighbor IP address\n"
       "Neighbor Priority\n"
       "Priority\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n")

DEFSH (VTYSH_PIMD, show_ip_pim_interface_cmd_vtysh, 
       "show ip pim interface [detail|WORD] [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface information\n"
       "Detailed output\n"
       "interface name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, neighbor_as_override_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> as-override", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Override ASNs in outbound updates if aspath equals remote-as\n")

DEFSH (VTYSH_BGPD, no_neighbor_soft_reconfiguration_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> soft-reconfiguration inbound", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Per neighbor soft reconfiguration\n"
       "Allow inbound soft reconfiguration for this neighbor\n")

DEFSH (VTYSH_ISISD, no_isis_redistribute_cmd_vtysh, 
       "no redistribute <ipv4|ipv6> " "<kernel|connected|static|rip|ripng|ospf|ospf6|bgp|pim|nhrp|table|vnc>" " <level-1|level-2>", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Redistribute IPv4 routes\n"
       "Redistribute IPv6 routes\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (OSPFv2)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Redistribute into level-1\n"
       "Redistribute into level-2\n")

DEFSH (VTYSH_BGPD|VTYSH_BGPD, show_ip_bgp_instance_updgrps_adj_s_cmd_vtysh, 
       "show [ip] bgp <view|vrf> WORD update-groups SUBGROUP-ID <advertise-queue|advertised-routes|packet-queue>", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Detailed info about dynamic update groups\n"
       "Specific subgroup to display info for\n"
       "Advertisement queue\n"
       "Announced routes\n"
       "Packet queue\n")

DEFSH (VTYSH_RIPD, no_key_string_cmd_vtysh, 
       "no key-string [LINE]", 
       "Negate a command or set its defaults\n"
       "Unset key string\n"
       "The key\n")

DEFSH (VTYSH_BGPD, no_set_atomic_aggregate_cmd_vtysh, 
       "no set atomic-aggregate", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP atomic aggregate attribute\n" )

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_summary_cmd_vtysh, 
       "show ipv6 prefix-list summary", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Summary of prefix lists\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, ip_prefix_list_seq_le_ge_cmd_vtysh, 
       "ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M le (0-32) ge (0-32)", 
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_explicit_null_cmd_vtysh, 
       "no label local advertise explicit-null", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "Configure explicit-null advertisement\n")

DEFSH (VTYSH_BGPD, show_debugging_bgp_vnc_cmd_vtysh, 
       "show debugging bgp vnc", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "VNC information\n")

DEFSH (VTYSH_BGPD, set_lcommunity_none_cmd_vtysh, 
       "set large-community none", 
       "Set values in destination routing protocol\n"
       "BGP large community attribute\n"
       "No large community attribute\n")

DEFSH (VTYSH_NHRPD, if_nhrp_nhs_cmd_vtysh, 
 "<ip|ipv6>" " nhrp nhs <A.B.C.D|X:X::X:X|dynamic> nbma <A.B.C.D|FQDN>", 
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Nexthop Server configuration\n"
 "IPv4 protocol address\n"
 "IPv6 protocol address\n"
 "Automatic detection of protocol address\n"
 "NBMA address\n"
 "IPv4 NBMA address\n"
 "Fully qualified domain name for NBMA address(es)\n")

DEFSH (VTYSH_PIMD, no_debug_pim_packetdump_recv_cmd_vtysh, 
       "no debug pim packet-dump receive", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM packet dump\n"
       "Dump received packets\n")

DEFSH (VTYSH_OSPFD, ospf_log_adjacency_changes_detail_cmd_vtysh, 
       "log-adjacency-changes detail", 
       "Log changes in adjacency state\n"
       "Log all state changes\n")

DEFSH (VTYSH_ALL, access_list_standard_any_cmd_vtysh, 
       "access-list <(1-99)|(1300-1999)> <deny|permit> any", 
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any source host\n")

DEFSH (VTYSH_BGPD, debug_bgp_as4_cmd_vtysh, 
       "debug bgp as4", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP AS4 actions\n")

DEFSH (VTYSH_BGPD, bgp_confederation_identifier_cmd_vtysh, 
       "bgp confederation identifier (1-4294967295)", 
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "AS number\n"
       "Set routing domain confederation AS\n")

DEFSH (VTYSH_OSPFD, ospf_area_export_list_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> export-list NAME", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Set the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_mpls_cmd_vtysh, 
       "no debug zebra mpls", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra MPLS LSPs\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_rt_both_cmd_vtysh, 
       "rt both RTLIST...", 
       "Specify route targets\n"
       "Export+import filters\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH_HIDDEN (VTYSH_PIMD, interface_no_ip_pim_ssm_cmd_vtysh, 
       "no ip pim ssm", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "PIM information\n"
       "Enable PIM SSM operation\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_lsa_hex_cmd_vtysh, 
       "no debug ospf6 lsa <router|network|inter-prefix|inter-router|as-external|link|intra-prefix|unknown> [<originate|examine|flooding>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug Link State Advertisements (LSAs)\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Display LSAs of unknown origin\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_prefix_cost_life_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M> cost (0-255) lifetime (1-4294967295)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n")

DEFSH (VTYSH_BGPD, no_vpnv4_network_cmd_vtysh, 
       "no network A.B.C.D/M rd ASN:nn_or_IP-address:nn <tag|label> (0-1048575)", 
       "Negate a command or set its defaults\n"
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n"
       "Specify Route Distinguisher\n"
       "VPN Route Distinguisher\n"
       "VPN NLRI label (tag)\n"
       "VPN NLRI label (tag)\n"
       "Label value\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_message_cmd_vtysh, 
       "no debug ospf6 message <unknown|hello|dbdesc|lsreq|lsupdate|lsack|all> [<send|recv>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 message\n"
       "Debug Unknown message\n"
       "Debug Hello message\n"
       "Debug Database Description message\n"
       "Debug Link State Request message\n"
       "Debug Link State Update message\n"
       "Debug Link State Acknowledgement message\n"
       "Debug All message\n"
       "Debug only sending message\n"
       "Debug only receiving message\n")

DEFSH (VTYSH_ISISD, debug_isis_lupd_cmd_vtysh, 
       "debug isis local-updates", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS local update packets\n")

DEFSH (VTYSH_ZEBRA, show_interface_desc_vrf_all_cmd_vtysh, 
       "show interface description vrf all", 
       "Show running system information\n"
       "Interface status and configuration\n"
       "Interface description\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH_HIDDEN (VTYSH_PIMD, no_ip_msdp_peer_cmd_vtysh, 
       "no ip msdp peer A.B.C.D", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Configure multicast source discovery protocol\n"
       "Delete MSDP peer\n"
       "peer ip address\n")

DEFSH (VTYSH_ZEBRA, bandwidth_if_cmd_vtysh, 
       "bandwidth (1-10000000)", 
       "Set bandwidth informational parameter\n"
       "Bandwidth in megabits\n")

DEFSH (VTYSH_RIPD, rip_redistribute_type_routemap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>" " route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_flags_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M [from X:X::X:X/M] <X:X::X:X|INTERFACE> <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, no_neighbor_maximum_prefix_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix [(1-4294967295) [(1-100)] [restart (1-65535)] [warning-only]]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefixes to accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Threshold value (%) at which to generate a warning msg\n"
       "Restart bgp connection after limit is exceeded\n"
       "Restart interval in minutes\n"
       "Only give warning message when limit is exceeded\n")

DEFSH (VTYSH_PIMD, interface_ip_mroute_source_cmd_vtysh, 
       "ip mroute INTERFACE A.B.C.D A.B.C.D", 
       "IP information\n"
       "Add multicast route\n"
       "Outgoing interface name\n"
       "Group address\n"
       "Source address\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_responselifetime_cmd_vtysh, 
       "response-lifetime <LIFETIME|infinite>", 
       "Specify response lifetime\n"
       "Response lifetime in seconds\n" "Infinite response lifetime\n")

DEFSH (VTYSH_BGPD, no_set_ipx_vpn_nexthop_cmd_vtysh, 
       "no set <ipv4|ipv6> vpn next-hop [<A.B.C.D|X:X::X:X>]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "IPv4 information\n"
       "IPv6 information\n"
       "VPN information\n"
       "VPN next-hop address\n"
       "IP address of next hop\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_PIMD, show_ip_igmp_sources_cmd_vtysh, 
       "show ip igmp sources", 
       "Show running system information\n"
       "IP information\n"
       "IGMP information\n"
       "IGMP sources information\n")

DEFSH (VTYSH_BGPD|VTYSH_BGPD, show_ip_bgp_instance_updgrps_adj_cmd_vtysh, 
       "show [ip] bgp <view|vrf> WORD update-groups <advertise-queue|advertised-routes|packet-queue>", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Detailed info about dynamic update groups\n"
       "Advertisement queue\n"
       "Announced routes\n"
       "Packet queue\n")

DEFSH (VTYSH_BGPD, set_ipv6_nexthop_global_cmd_vtysh, 
       "set ipv6 next-hop global X:X::X:X", 
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "IPv6 global address\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_binding_cmd_vtysh, 
       "show mpls ldp binding", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Label Information Base (LIB) information\n")

DEFSH (VTYSH_BGPD, no_set_aspath_prepend_cmd_vtysh, 
       "no set as-path prepend [(1-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "Transform BGP AS_PATH attribute\n"
       "Prepend to the as-path\n"
       "AS number\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_rmap_metric_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|rip|ospf|isis|pim|nhrp|table|vnc|vnc-direct>" " route-map WORD metric (0-4294967295)", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n"
       "Metric for redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_redistribute_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ripng|isis|bgp|nhrp|table|vnc>" " [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Route map reference\n"
       "Route map name\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_id_router_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> A.B.C.D A.B.C.D [<dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Specify Advertising Router as IPv4 address notation\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_keepalive_peer_cmd_vtysh, 
       "no debug bgp keepalives <A.B.C.D|X:X::X:X|WORD>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP keepalives\n"
       "BGP neighbor IP address to debug\n"
       "BGP IPv6 neighbor to debug\n"
       "BGP neighbor on interface to debug\n")

DEFSH (VTYSH_OSPFD, no_ospf_distribute_list_out_cmd_vtysh, 
       "no distribute-list WORD out " "<kernel|connected|static|rip|isis|bgp|pim|nhrp|table|vnc>", 
       "Negate a command or set its defaults\n"
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter outgoing routing updates\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_BGPD, no_set_lcommunity_cmd_vtysh, 
       "no set large-community none", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP large community attribute\n"
       "No community attribute\n")

DEFSH (VTYSH_PIMD, debug_pim_packets_cmd_vtysh, 
       "debug pim packets [<hello|joins|register>]", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM protocol packets\n"
       "PIM Hello protocol packets\n"
       "PIM Join/Prune protocol packets\n"
       "PIM Register/Reg-Stop protocol packets\n")

DEFSH (VTYSH_ISISD, debug_isis_packet_dump_cmd_vtysh, 
       "debug isis packet-dump", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS packet dump\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_ifname_tag_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE tag (1-4294967295) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_retransmit_interval_cmd_vtysh, 
       "no ospf retransmit-interval [(3-65535)] [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"
       "Address of interface\n")

DEFSH (VTYSH_ISISD, no_lsp_gen_interval_cmd_vtysh, 
       "no lsp-gen-interval [<level-1|level-2>] [(1-120)]", 
       "Negate a command or set its defaults\n"
       "Minimum interval between regenerating same LSP\n"
       "Set interval for level 1 only\n"
       "Set interval for level 2 only\n"
       "Minimum interval in seconds\n")

DEFSH (VTYSH_BGPD, no_neighbor_remove_private_as_replace_as_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS replace-AS", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n"
       "Replace private ASNs with our ASN in outbound updates\n")

DEFSH (VTYSH_ISISD, isis_mpls_te_inter_as_cmd_vtysh, 
       "mpls-te inter-as <level-1|level-1-2|level-2-only>", 
       "MPLS-TE specific commands\n"
       "Configure MPLS-TE Inter-AS support\n"
       "AREA native mode self originate INTER-AS LSP with L1 only flooding scope)\n"
       "AREA native mode self originate INTER-AS LSP with L1 and L2 flooding scope)\n"
       "AS native mode self originate INTER-AS LSP with L2 only flooding scope\n")

DEFSH (VTYSH_PIMD, ip_pim_spt_switchover_infinity_cmd_vtysh, 
       "ip pim spt-switchover infinity-and-beyond", 
       "IP information\n"
       "PIM information\n"
       "SPT-Switchover\n"
       "Never switch to SPT Tree\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_mpls_cmd_vtysh, 
       "debug zebra mpls", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra MPLS LSPs\n")

DEFSH (VTYSH_ALL, no_access_list_extended_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Destination address\n"
       "Destination Wildcard bits\n")

DEFSH (VTYSH_BGPD, debug_bgp_zebra_cmd_vtysh, 
       "debug bgp zebra", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Zebra messages\n")

DEFSH (VTYSH_BGPD, set_community_cmd_vtysh, 
       "set community AA:NN...", 
       "Set values in destination routing protocol\n"
       "BGP community attribute\n"
       "Community number in AA:NN format (where AA and NN are (0-65535)) or local-AS|no-advertise|no-export|internet or additive\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_network_cmd_vtysh, 
       "no ip ospf network [<broadcast|non-broadcast|point-to-multipoint|point-to-point>]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Network type\n"
       "Specify OSPF broadcast multi-access network\n"
       "Specify OSPF NBMA network\n"
       "Specify OSPF point-to-multipoint network\n"
       "Specify OSPF point-to-point network\n")

DEFSH (VTYSH_BGPD, no_aggregate_address_mask_cmd_vtysh, 
       "no aggregate-address A.B.C.D A.B.C.D [<as-set [summary-only]|summary-only [as-set]>]", 
       "Negate a command or set its defaults\n"
       "Configure BGP aggregate entries\n"
       "Aggregate address\n"
       "Aggregate mask\n"
       "Generate AS set path information\n"
       "Filter more specific routes from updates\n"
       "Filter more specific routes from updates\n"
       "Generate AS set path information\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_tag_cmd_vtysh, 
       "match tag (1-4294967295)", 
       "Match values from routing table\n"
       "Match tag of route\n"
       "Tag value\n")

DEFSH (VTYSH_ISISD, domain_passwd_md5_cmd_vtysh, 
       "domain-password md5 WORD [authenticate snp <send-only|validate>]", 
       "Set the authentication password for a routing domain\n"
       "Authentication type\n"
       "Level-wide password\n"
       "Authentication\n"
       "SNP PDUs\n"
       "Send but do not check PDUs on receiving\n"
       "Send and check PDUs on receiving\n")

DEFSH (VTYSH_ZEBRA, no_link_params_metric_cmd_vtysh, 
       "no metric", 
       "Negate a command or set its defaults\n"
       "Disable Link Metric on this interface\n")

DEFSH (VTYSH_PIMD, show_ip_rib_cmd_vtysh, 
       "show ip rib A.B.C.D", 
       "Show running system information\n"
       "IP information\n"
       "IP unicast routing table\n"
       "Unicast address\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_brouter_cmd_vtysh, 
       "debug ospf6 border-routers", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug border router\n"
      )

DEFSH (VTYSH_ALL, show_ip_access_list_name_cmd_vtysh, 
       "show ip access-list <(1-99)|(100-199)|(1300-1999)|(2000-2699)|WORD>", 
       "Show running system information\n"
       "IP information\n"
       "List IP access lists\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n")

DEFSH (VTYSH_BGPD, match_lcommunity_cmd_vtysh, 
       "match large-community <(1-99)|(100-500)|WORD>", 
       "Match values from routing table\n"
       "Match BGP large community list\n"
       "Large Community-list number (standard)\n"
       "Large Community-list number (expanded)\n"
       "Large Community-list name\n")

DEFSH (VTYSH_ZEBRA, no_zebra_ptm_enable_cmd_vtysh, 
       "no ptm-enable", 
       "Negate a command or set its defaults\n"
       "Enable neighbor check with specified topology\n")

DEFSH (VTYSH_ZEBRA, show_ip_protocol_nht_cmd_vtysh, 
       "show ip nht route-map", 
       "Show running system information\n"
       "IP information\n"
       "IP nexthop tracking table\n"
       "IP Next Hop tracking filtering status\n")

DEFSH (VTYSH_RIPD, send_lifetime_month_day_day_month_cmd_vtysh, 
       "send-lifetime HH:MM:SS MONTH (1-31) (1993-2035) HH:MM:SS (1-31) MONTH (1993-2035)", 
       "Set send lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Time to expire\n"
       "Day of th month to expire\n"
       "Month of the year to expire\n"
       "Year to expire\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_message_digest_key_cmd_vtysh, 
              "no ospf message-digest-key (1-255) [md5 KEY] [A.B.C.D]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Message digest authentication password (key)\n"
              "Key ID\n"
              "Address of interface")

DEFSH (VTYSH_BGPD, neighbor_override_capability_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> override-capability", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Override capability negotiation result\n")

DEFSH (VTYSH_OSPFD, ospf_mpls_te_on_cmd_vtysh, 
       "mpls-te on", 
       "MPLS-TE specific commands\n"
       "Enable the MPLS-TE functionality\n")

DEFSH (VTYSH_PIMD, show_ip_multicast_cmd_vtysh, 
       "show ip multicast", 
       "Show running system information\n"
       "IP information\n"
       "Multicast global information\n")

DEFSH (VTYSH_BGPD, debug_bgp_update_groups_cmd_vtysh, 
       "debug bgp update-groups", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP update-groups\n")

DEFSH (VTYSH_ISISD, isis_hello_multiplier_l2_cmd_vtysh, 
       "isis hello-multiplier (2-100) level-2", 
       "IS-IS commands\n"
       "Set multiplier for Hello holding time\n"
       "Hello multiplier value\n"
       "Specify hello multiplier for level-2 IIHs\n")

DEFSH (VTYSH_OSPFD, ospf_area_stub_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> stub", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n")

DEFSH (VTYSH_ZEBRA, no_link_params_delay_var_cmd_vtysh, 
       "no delay-variation", 
       "Negate a command or set its defaults\n"
       "Disable Unidirectional Delay Variation on this interface\n")

DEFSH (VTYSH_LDPD, ldp_no_router_id_ipv4_cmd_vtysh, 
       "no router-id A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Configure router Id\n"
       "LSR Id (in form of an IPv4 address)\n")

DEFSH (VTYSH_ISISD, isis_passwd_cmd_vtysh, 
       "isis password <md5|clear> WORD", 
       "IS-IS commands\n"
       "Configure the authentication password for a circuit\n"
       "HMAC-MD5 authentication\n"
       "Cleartext password\n"
       "Circuit password\n")

DEFSH (VTYSH_OSPF6D, ospf6_timers_lsa_cmd_vtysh, 
       "timers lsa min-arrival (0-600000)", 
       "Adjust routing timers\n"
       "OSPF6 LSA timers\n"
       "Minimum delay in receiving new version of a LSA\n"
       "Delay in milliseconds\n")

DEFSH (VTYSH_ALL, access_list_extended_mask_any_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip A.B.C.D A.B.C.D any", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Any destination host\n")

DEFSH (VTYSH_BGPD, no_neighbor_timers_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> timers [(0-65535) (0-65535)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP per neighbor timers\n"
       "Keepalive interval\n"
       "Holdtime\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_route_pref_tag_label_cmd_vtysh, 
       "no ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> tag (1-4294967295) (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_BGPD, aggregate_address_mask_cmd_vtysh, 
       "aggregate-address A.B.C.D A.B.C.D [<as-set [summary-only]|summary-only [as-set]>]", 
       "Configure BGP aggregate entries\n"
       "Aggregate address\n"
       "Aggregate mask\n"
       "Generate AS set path information\n"
       "Filter more specific routes from updates\n"
       "Filter more specific routes from updates\n"
       "Generate AS set path information\n")

DEFSH (VTYSH_BGPD, no_neighbor_prefix_list_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> prefix-list WORD <in|out>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

DEFSH (VTYSH_ISISD, debug_isis_csum_cmd_vtysh, 
       "debug isis checksum-errors", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS LSP checksum errors\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_name_cmd_vtysh, 
       "show ip prefix-list WORD", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_PIMD, no_ip_pim_register_suppress_cmd_vtysh, 
       "no ip pim register-suppress-time (5-60000)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Register Suppress Timer\n"
       "Seconds\n")

DEFSH (VTYSH_ZEBRA, ipv6_nht_default_route_cmd_vtysh, 
       "ipv6 nht resolve-via-default", 
       "IPv6 Information\n"
       "Filter Next Hop tracking route resolution\n"
       "Resolve via default route\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_network_cmd_vtysh, 
       "ipv6 ospf6 network <broadcast|point-to-point>", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Network type\n"
       "Specify OSPF6 broadcast network\n"
       "Specify OSPF6 point-to-point network\n"
       )

DEFSH (VTYSH_BGPD, no_neighbor_remove_private_as_all_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS all", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n"
       "Apply to all AS numbers\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_ifname_cmd_vtysh, 
       "ipv6 route X:X::X:X/M [from X:X::X:X/M] X:X::X:X INTERFACE [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 source-dest route\n"
       "IPv6 source prefix\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this prefix\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_reachable_time_cmd_vtysh, 
       "no ipv6 nd reachable-time [(1-3600000)]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Reachable time\n"
       "Reachable time in milliseconds\n")

DEFSH (VTYSH_ZEBRA, ip_route_mask_flags_cmd_vtysh, 
       "ip route A.B.C.D A.B.C.D <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ISISD, isis_metric_l1_cmd_vtysh, 
       "isis metric (0-16777215) level-1", 
       "IS-IS commands\n"
       "Set default metric for circuit\n"
       "Default metric value\n"
       "Specify metric for level-1 routing\n")

DEFSH (VTYSH_LDPD, ldp_no_bridge_ifname_cmd_vtysh, 
       "no bridge IFNAME", 
       "Negate a command or set its defaults\n"
       "Bridge interface\n"
       "Interface's name\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_prefix_cmd_vtysh, 
       "ipv6 nd prefix X:X::X:X/M [<(0-4294967295)|infinite> <(0-4294967295)|infinite>] [<router-address|off-link [no-autoconfig]|no-autoconfig [off-link]>]", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Set Router Address flag\n"
       "Do not use prefix for onlink determination\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for onlink determination\n")

DEFSH (VTYSH_RIPD, rip_neighbor_cmd_vtysh, 
       "neighbor A.B.C.D", 
       "Specify a neighbor router\n"
       "Neighbor address\n")

DEFSH (VTYSH_OSPF6D, redistribute_ospf6_cmd_vtysh, 
       "redistribute ospf6", 
       "Redistribute control\n"
       "OSPF6 route\n")

DEFSH (VTYSH_RIPD, accept_lifetime_infinite_day_month_cmd_vtysh, 
       "accept-lifetime HH:MM:SS (1-31) MONTH (1993-2035) infinite", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Day of th month to start\n"
       "Month of the year to start\n"
       "Year to start\n"
       "Never expires")

DEFSH (VTYSH_BGPD, bgp_default_ipv4_unicast_cmd_vtysh, 
       "bgp default ipv4-unicast", 
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "Activate ipv4-unicast for a peer by default\n")

DEFSH (VTYSH_ZEBRA, ip_zebra_import_table_distance_cmd_vtysh, 
       "ip import-table (1-252) [distance (1-255)] [route-map WORD]", 
       "IP information\n"
       "import routes from non-main kernel table\n"
       "kernel routing table id\n"
       "Distance for imported routes\n"
       "Default distance value\n"
       "route-map for filtering\n"
       "route-map name\n")

DEFSH (VTYSH_BGPD, vpnv4_network_cmd_vtysh, 
       "network A.B.C.D/M rd ASN:nn_or_IP-address:nn <tag|label> (0-1048575)", 
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n"
       "Specify Route Distinguisher\n"
       "VPN Route Distinguisher\n"
       "VPN NLRI label (tag)\n"
       "VPN NLRI label (tag)\n"
       "Label value\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, match_ip_next_hop_prefix_list_cmd_vtysh, 
       "match ip next-hop prefix-list WORD", 
       "Match values from routing table\n"
       "IP information\n"
       "Match next-hop address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_ISISD, no_debug_isis_spfevents_cmd_vtysh, 
       "no debug isis spf-events", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS Shortest Path First Events\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_rt_import_cmd_vtysh, 
       "rt import RTLIST...", 
       "Specify route targets\n"
       "Import filter\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_ZEBRA, config_table_cmd_vtysh, 
       "table TABLENO", 
       "Configure target kernel routing table\n"
       "TABLE integer\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_hello_interval_disc_time_cmd_vtysh, 
       "no discovery hello interval (1-65535)", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "LDP Link Hellos\n"
       "Hello interval\n"
       "Time (seconds)\n")

DEFSH (VTYSH_ISISD, lsp_gen_interval_cmd_vtysh, 
       "lsp-gen-interval [<level-1|level-2>] (1-120)", 
       "Minimum interval between regenerating same LSP\n"
       "Set interval for level 1 only\n"
       "Set interval for level 2 only\n"
       "Minimum interval in seconds\n")

DEFSH (VTYSH_ZEBRA, show_ip_protocol_cmd_vtysh, 
       "show ip protocol", 
        "Show running system information\n"
        "IP information\n"
       "IP protocol filtering status\n")

DEFSH (VTYSH_BGPD, no_neighbor_port_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X> port [(0-65535)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nIPv6 address\n"
       "Neighbor's BGP port\n"
       "TCP port number\n")

DEFSH (VTYSH_BGPD, set_ipx_vpn_nexthop_cmd_vtysh, 
       "set <ipv4|ipv6> vpn next-hop <A.B.C.D|X:X::X:X>", 
       "Set values in destination routing protocol\n"
       "IPv4 information\n"
       "IPv6 information\n"
       "VPN information\n"
       "VPN next-hop address\n"
       "IP address of next hop\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_metric_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|rip|ospf|isis|pim|nhrp|table|vnc|vnc-direct>" " metric (0-4294967295)", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Metric for redistributed routes\n"
       "Default metric\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_kernel_cmd_vtysh, 
       "debug zebra kernel", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra between kernel interface\n")

DEFSH (VTYSH_ISISD, debug_isis_lsp_gen_cmd_vtysh, 
       "debug isis lsp-gen", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS generation of own LSPs\n")

DEFSH (VTYSH_OSPFD, pce_path_scope_cmd_vtysh, 
       "pce scope BITPATTERN", 
       "PCE Router Information specific commands\n"
       "Path scope visibilities of the PCE for path computation\n"
       "32-bit Hexadecimal value\n")

DEFSH (VTYSH_OSPFD, debug_ospf_instance_event_cmd_vtysh, 
       "debug ospf (1-65535) event", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF event information\n")

DEFSH (VTYSH_RIPD, no_distribute_list_cmd_vtysh, 
       "no [ipv6] distribute-list [prefix] WORD <in|out> [WORD]", 
       "Negate a command or set its defaults\n"
       "IPv6\n"
       "Filter networks in routing updates\n"
       "Specify a prefix\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")

DEFSH (VTYSH_BGPD, no_bgp_rr_allow_outbound_policy_cmd_vtysh, 
       "no bgp route-reflector allow-outbound-policy", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Allow modifications made by out route-map\n"
       "on ibgp neighbors\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_interface_cmd_vtysh, 
       "debug ospf6 interface", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 Interface\n"
      )

DEFSH (VTYSH_ZEBRA, show_ipv6_route_protocol_cmd_vtysh, 
       "show ipv6 route [vrf NAME] " "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc>", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_seq_le_ge_cmd_vtysh, 
       "ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M le (0-128) ge (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_OSPFD, no_ospf_mpls_te_inter_as_cmd_vtysh, 
       "no mpls-te inter-as", 
       "Negate a command or set its defaults\n"
       "MPLS-TE specific commands\n"
       "Disable MPLS-TE Inter-AS support\n")

DEFSH (VTYSH_RIPNGD, debug_ripng_zebra_cmd_vtysh, 
       "debug ripng zebra", 
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng and zebra communication\n")

DEFSH (VTYSH_ALL, no_access_list_any_cmd_vtysh, 
       "no access-list WORD <deny|permit> any", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n")

DEFSH (VTYSH_BGPD, no_match_community_cmd_vtysh, 
       "no match community [<(1-99)|(100-500)|WORD> [exact-match]]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match BGP community list\n"
       "Community-list number (standard)\n"
       "Community-list number (expanded)\n"
       "Community-list name\n"
       "Do exact matching of communities\n")

DEFSH (VTYSH_RIPD, no_rip_default_information_originate_cmd_vtysh, 
       "no default-information originate", 
       "Negate a command or set its defaults\n"
       "Control distribution of default route\n"
       "Distribute a default route\n")

DEFSH (VTYSH_BGPD, neighbor_addpath_tx_bestpath_per_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> addpath-tx-bestpath-per-AS", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Use addpath to advertise the bestpath per each neighboring AS\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_message_digest_key_cmd_vtysh, 
              "ospf message-digest-key (1-255) md5 KEY [A.B.C.D]", 
              "OSPF interface commands\n"
              "Message digest authentication password (key)\n"
              "Key ID\n"
              "Use MD5 algorithm\n"
              "The OSPF password (key)\n"
              "Address of interface\n")

DEFSH (VTYSH_ALL, no_access_list_extended_host_any_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip host A.B.C.D any", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Any destination host\n")

DEFSH (VTYSH_BGPD, no_set_ipv6_nexthop_global_cmd_vtysh, 
       "no set ipv6 next-hop global X:X::X:X", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "IPv6 information\n"
       "IPv6 next-hop address\n"
       "IPv6 global address\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_brouter_router_cmd_vtysh, 
       "no debug ospf6 border-routers router-id", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug border router\n"
       "Debug specific border router\n"
      )

DEFSH (VTYSH_PIMD, ip_ssmpingd_cmd_vtysh, 
       "ip ssmpingd [A.B.C.D]", 
       "IP information\n"
       "Enable ssmpingd operation\n"
       "Source address\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_lcommunity_info_cmd_vtysh, 
       "show ip bgp large-community-info", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "List all bgp large-community information\n")

DEFSH (VTYSH_ISISD, debug_isis_adj_cmd_vtysh, 
       "debug isis adj-packets", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS Adjacency related packets\n")

DEFSH (VTYSH_BGPD, neighbor_maximum_prefix_threshold_warning_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix (1-4294967295) (1-100) warning-only", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Threshold value (%) at which to generate a warning msg\n"
       "Only give warning message when limit is exceeded\n")

DEFSH (VTYSH_OSPFD, no_pce_neighbor_cmd_vtysh, 
       "no pce neighbor as (0-65535)", 
       "Negate a command or set its defaults\n"
       "PCE Router Information specific commands\n"
       "Disable PCE neighbor AS number\n"
       "AS number of PCE neighbor\n"
       "AS number in decimal <0-65535>\n")

DEFSH (VTYSH_BGPD, bgp_maxmed_admin_cmd_vtysh, 
       "bgp max-med administrative ", 
       "BGP information\n"
       "Advertise routes with max-med\n"
       "Administratively applied,  for an indefinite period\n")

DEFSH (VTYSH_BGPD, bgp_multiple_instance_cmd_vtysh, 
       "bgp multiple-instance", 
       "BGP information\n"
       "Enable bgp multiple instance\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_summary_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] summary [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Summary of BGP neighbor status\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, neighbor_remove_private_as_all_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> remove-private-AS all", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Remove private ASNs in outbound updates\n"
       "Apply to all AS numbers")

DEFSH (VTYSH_PIMD, debug_msdp_events_cmd_vtysh, 
       "debug msdp events", 
       "Debugging functions (see also 'undebug')\n"
       "MSDP protocol activity\n"
       "MSDP protocol events\n")

DEFSH (VTYSH_BGPD, no_neighbor_nexthop_local_unchanged_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> nexthop-local unchanged", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Configure treatment of outgoing link-local-nexthop attribute\n"
       "Leave link-local nexthop unchanged for this peer\n")

DEFSH (VTYSH_ZEBRA, no_mpls_transit_lsp_cmd_vtysh, 
       "no mpls lsp (16-1048575) <A.B.C.D|X:X::X:X>", 
       "Negate a command or set its defaults\n"
       "MPLS information\n"
       "Establish label switched path\n"
       "Incoming MPLS label\n"
       "IPv4 gateway address\n"
       "IPv6 gateway address\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_export_list_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> export-list NAME", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFSH (VTYSH_RIPNGD, ipv6_ripng_split_horizon_poisoned_reverse_cmd_vtysh, 
       "ipv6 ripng split-horizon poisoned-reverse", 
       "IPv6 information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n"
       "With poisoned-reverse\n")

DEFSH (VTYSH_PIMD, interface_no_ip_mroute_cmd_vtysh, 
       "no ip mroute INTERFACE A.B.C.D", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add multicast route\n"
       "Outgoing interface name\n"
       "Group Address\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_priority_cmd_vtysh, 
              "ospf priority (0-255) [A.B.C.D]", 
              "OSPF interface commands\n"
              "Router priority\n"
              "Priority\n"
              "Address of interface")

DEFSH (VTYSH_BGPD, no_bgp_redistribute_ipv4_ospf_cmd_vtysh, 
       "no redistribute <ospf|table> (1-65535) [metric (0-4294967295)] [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Non-main Kernel Routing Table\n"
       "Instance ID/Table ID\n"
       "Metric for redistributed routes\n"
       "Default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_BGPD, clear_vnc_nve_un_cmd_vtysh, 
       "clear vnc nve un <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "UN address of NVE\n"
       "All un nves\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n")

DEFSH (VTYSH_RIPD, no_rip_default_metric_cmd_vtysh, 
       "no default-metric [(1-16)]", 
       "Negate a command or set its defaults\n"
       "Set a metric of redistribute routes\n"
       "Default metric\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_ipv4_password_word_cmd_vtysh, 
       "no neighbor A.B.C.D password WORD", 
       "Negate a command or set its defaults\n"
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "Configure password for MD5 authentication\n"
       "The password\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_export_no_prefixlist_cmd_vtysh, 
       "no export <bgp|zebra> <ipv4|ipv6> prefix-list [NAME]", 
       "Negate a command or set its defaults\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "IPv4 routes\n"
       "IPv6 routes\n"
       "Prefix-list for filtering exported routes\n" "prefix list name\n")

DEFSH (VTYSH_LDPD, ldp_label_remote_accept_for_acl_from_acl_cmd_vtysh, 
       "label remote accept for <(1-199)|(1300-2699)|WORD> from <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n"
       "Neighbor from whom to accept label advertisement\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ZEBRA, show_interface_desc_cmd_vtysh, 
       "show interface description [vrf NAME]", 
       "Show running system information\n"
       "Interface status and configuration\n"
       "Interface description\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_BGPD, no_neighbor_nexthop_self_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> next-hop-self", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Disable the next hop calculation for this neighbor\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_debug_misc_cmd_vtysh, 
       "ip irdp debug misc", 
       "IP information\n"
       "ICMP Router discovery debug Averts. and Solicits (short)\n"
       "IRDP debugging options\n"
       "Enable debugging for miscellaneous IRDP events\n")

DEFSH (VTYSH_BGPD, no_bgp_bestpath_aspath_multipath_relax_cmd_vtysh, 
       "no bgp bestpath as-path multipath-relax [<as-set|no-as-set>]", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Allow load sharing across routes that have different AS paths (but same length)\n"
       "Generate an AS_SET\n"
       "Do not generate an AS_SET\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_seq_ge_cmd_vtysh, 
       "no ip prefix-list WORD seq (1-4294967295) <deny|permit> A.B.C.D/M ge (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_LDPD, ldp_no_dual_stack_transport_connection_prefer_ipv4_cmd_vtysh, 
       "no dual-stack transport-connection prefer ipv4", 
       "Negate a command or set its defaults\n"
       "Configure dual stack parameters\n"
       "Configure TCP transport parameters\n"
       "Configure prefered address family for TCP transport connection with neighbor\n"
       "IPv4\n")

DEFSH (VTYSH_OSPFD, no_ospf_timers_min_ls_interval_cmd_vtysh, 
       "no timers throttle lsa all [(0-5000)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "Throttling adaptive timer\n"
       "LSA delay between transmissions\n"
       "All LSA types\n"
       "Delay (msec) between sending LSAs\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_discovery_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> discovery", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "Discovery Hello Information\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_instance_cmd_vtysh, 
       "ipv6 ospf6 instance-id (0-255)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Instance ID for this interface\n"
       "Instance ID value\n"
       )

DEFSH (VTYSH_ISISD, show_isis_mpls_te_router_cmd_vtysh, 
       "show isis mpls-te router", 
       "Show running system information\n"
       "IS-IS information\n"
       "MPLS-TE specific commands\n"
       "Router information\n")

DEFSH (VTYSH_BGPD, no_neighbor_ttl_security_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> ttl-security hops (1-254)", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP ttl-security parameters\n"
       "Specify the maximum number of hops to the BGP peer\n"
       "Number of hops to BGP peer\n")

DEFSH (VTYSH_OSPFD, write_multiplier_cmd_vtysh,  "write-multiplier (1-100)", 
      "Write multiplier\n"
      "Maximum number of interface serviced per write\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_authentication_args_addr_cmd_vtysh, 
       "no ip ospf authentication <null|message-digest> [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Use null authentication\n"
       "Use message-digest authentication\n"
       "Address of interface")

DEFSH (VTYSH_OSPFD, ospf_mpls_te_router_addr_cmd_vtysh, 
       "mpls-te router-address A.B.C.D", 
       "MPLS-TE specific commands\n"
       "Stable IP address of the advertising router\n"
       "MPLS-TE router address in IPv4 address format\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn [json]", 
      "Show running system information\n" "IP information\n" "BGP information\n" "Layer 2 Virtual Private Network\n" "Ethernet Virtual Private Network\n" "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, bgp_distance_source_access_list_cmd_vtysh, 
       "distance (1-255) A.B.C.D/M WORD", 
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n"
       "Access list name\n")

DEFSH (VTYSH_RIPNGD, ripng_redistribute_type_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf6|isis|bgp|nhrp|table|vnc>", 
       "Redistribute\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_set_tag_cmd_vtysh, 
       "no set tag [(1-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "Tag value for routing protocol\n"
       "Tag value\n")

DEFSH (VTYSH_ISISD, no_csnp_interval_cmd_vtysh, 
       "no isis csnp-interval [(1-600)]", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set CSNP interval in seconds\n"
       "CSNP interval value\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_bestpath_prefix_cmd_vtysh, 
       "no debug bgp bestpath <A.B.C.D/M|X:X::X:X/M>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP bestpath\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_prefix_cmd_vtysh, 
       "show ip route vrf all A.B.C.D/M", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_vn_prefix_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> vn <*|A.B.C.D|X:X::X:X> prefix <*|A.B.C.D/M|X:X::X:X/M>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 Prefix\n"
       "IPv6 Prefix\n")

DEFSH (VTYSH_ZEBRA, show_interface_name_vrf_cmd_vtysh, 
       "show interface IFNAME vrf NAME", 
       "Show running system information\n"
       "Interface status and configuration\n"
       "Interface name\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_ISISD, debug_isis_spftrigg_cmd_vtysh, 
       "debug isis spf-triggers", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS SPF triggering events\n")

DEFSH (VTYSH_BGPD, bgp_bestpath_med_cmd_vtysh, 
       "bgp bestpath med <confed [missing-as-worst]|missing-as-worst [confed]>", 
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Compare MED among confederation paths\n"
       "Treat missing MED as the least preferred one\n"
       "Treat missing MED as the least preferred one\n"
       "Compare MED among confederation paths\n")

DEFSH (VTYSH_ZEBRA, link_params_maxbw_cmd_vtysh, 
       "max-bw BANDWIDTH", 
       "Maximum bandwidth that can be used\n"
       "Bytes/second (IEEE floating point format)\n")

DEFSH (VTYSH_RIPD, ip_rip_send_version_cmd_vtysh, 
       "ip rip send version (1-2)", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement transmission\n"
       "Version control\n"
       "RIP version 1\n"
       "RIP version 2\n")

DEFSH (VTYSH_PIMD, no_debug_igmp_packets_cmd_vtysh, 
       "no debug igmp packets", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n"
       "IGMP protocol packets\n")

DEFSH (VTYSH_NHRPD, debug_nhrp_cmd_vtysh, 
 "debug nhrp " "<all|common|event|interface|kernel|route|vici>", 
 "Enable debug messages for specific or all parts.\n"
 "NHRP information\n"
 "All messages\n" "Common messages (default)\n" "Event manager messages\n" "Interface messages\n" "Kernel messages\n" "Route messages\n" "VICI messages\n")

DEFSH (VTYSH_NHRPD, show_dmvpn_cmd_vtysh, 
 "show dmvpn", 
 "Show running system information\n"
 "DMVPN information\n")

DEFSH (VTYSH_BGPD, show_ip_community_list_arg_cmd_vtysh, 
       "show ip community-list <(1-500)|WORD>", 
       "Show running system information\n"
       "IP information\n"
       "List community-list\n"
       "Community-list number\n"
       "Community-list name\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_discovery_json_cmd_vtysh, 
       "show mpls ldp discovery json", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Discovery Hello Information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_set_ip_nexthop_cmd_vtysh, 
       "no set ip next-hop [<peer-address|A.B.C.D>]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "IP information\n"
       "Next hop address\n"
       "Use peer address (for BGP only)\n"
       "IP address of next hop\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_export_routemap_cmd_vtysh, 
       "export <bgp|zebra> route-map NAME", 
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "Route-map for filtering exported routes\n" "route map name\n")

DEFSH (VTYSH_BGPD, clear_ip_bgp_dampening_prefix_cmd_vtysh, 
       "clear ip bgp dampening A.B.C.D/M", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "Clear route flap dampening information\n"
       "IPv4 prefix\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_interface_ifname_prefix_cmd_vtysh, 
       "show ipv6 ospf6 interface IFNAME prefix [<X:X::X:X|X:X::X:X/M>] [<match|detail>]", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface infomation\n"
       "Interface name(e.g. ep0)\n"
       "Display connected prefixes to advertise\n"
       "Display the route bestmatches the address\n"
       "Display the route\n"
       "Display the route matches the prefix\n"
       "Display details of the prefixes\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_ip_address_cmd_vtysh, 
       "no match ip address [<(1-199)|(1300-2699)|WORD>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

DEFSH (VTYSH_PIMD, ip_pim_ecmp_rebalance_cmd_vtysh, 
       "ip pim ecmp rebalance", 
       "IP information\n"
       "pim multicast routing\n"
       "Enable PIM ECMP \n"
       "Enable PIM ECMP Rebalance\n")

DEFSH (VTYSH_BGPD, no_bgp_maxmed_admin_cmd_vtysh, 
       "no bgp max-med administrative [(0-4294967294)]", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Advertise routes with max-med\n"
       "Administratively applied,  for an indefinite period\n"
       "Max MED value to be used\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_rmap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|rip|ospf|isis|pim|nhrp|table|vnc|vnc-direct>" " route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH_HIDDEN (VTYSH_PIMD, interface_no_ip_igmp_query_max_response_time_dsec_cmd_vtysh, 
       "no ip igmp query-max-response-time-dsec", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP max query response value (deciseconds)\n")

DEFSH (VTYSH_OSPFD, debug_ospf_lsa_cmd_vtysh, 
       "debug ospf lsa [<generate|flooding|install|refresh>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF Link State Advertisement\n"
       "LSA Generation\n"
       "LSA Flooding\n"
       "LSA Install/Delete\n"
       "LSA Refresh\n")

DEFSH (VTYSH_PIMD, no_debug_static_cmd_vtysh, 
       "no debug static", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "PIM Static Multicast Route activity\n")

DEFSH (VTYSH_PIMD, show_ip_igmp_groups_cmd_vtysh, 
       "show ip igmp groups [json]", 
       "Show running system information\n"
       "IP information\n"
       "IGMP information\n"
       "IGMP groups information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD, show_ip_rip_status_cmd_vtysh, 
       "show ip rip status", 
       "Show running system information\n"
       "IP information\n"
       "Show RIP routes\n"
       "IP routing protocol process parameters and statistics\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_address_preference_cmd_vtysh, 
       "ip irdp address A.B.C.D preference (0-2147483647)", 
       "IP information\n"
       "Alter ICMP Router discovery preference on this interface\n"
       "Set IRDP address for advertise\n"
       "IPv4 address\n"
       "Specify IRDP non-default preference to advertise\n"
       "Preference level\n")

DEFSH (VTYSH_ISISD, isis_circuit_type_cmd_vtysh, 
       "isis circuit-type <level-1|level-1-2|level-2-only>", 
       "IS-IS commands\n"
       "Configure circuit type for interface\n"
       "Level-1 only adjacencies are formed\n"
       "Level-1-2 adjacencies are formed\n"
       "Level-2 only adjacencies are formed\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_id_router_cmd_vtysh, 
       "show ipv6 ospf6 database * A.B.C.D A.B.C.D [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Any Link state Type\n"
       "Specify Link state ID as IPv4 address notation\n"
       "Specify Advertising Router as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n"
      )

DEFSH (VTYSH_BGPD, no_set_local_pref_cmd_vtysh, 
       "no set local-preference [(0-4294967295)]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP local preference path attribute\n"
       "Preference value\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_ipv4_targeted_cmd_vtysh, 
       "no neighbor A.B.C.D targeted", 
       "Negate a command or set its defaults\n"
       "Configure neighbor parameters\n"
       "IP address of neighbor\n"
       "Establish targeted session\n")

DEFSH (VTYSH_BGPD, bgp_log_neighbor_changes_cmd_vtysh, 
       "bgp log-neighbor-changes", 
       "BGP specific commands\n"
       "Log neighbor up/down and reset reason\n")

DEFSH (VTYSH_BGPD, neighbor_bfd_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> bfd", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Enables BFD support\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, rmap_description_cmd_vtysh, 
       "description LINE...", 
       "Route-map comment\n"
       "Comment describing this route-map rule\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_rib_cmd_vtysh, 
       "no debug zebra rib", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug zebra RIB\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_un_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> un <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All UN addresses\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n")

DEFSH (VTYSH_RIPD, no_ip_rip_split_horizon_poisoned_reverse_cmd_vtysh, 
       "no ip rip split-horizon poisoned-reverse", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n"
       "With poisoned-reverse\n")

DEFSH (VTYSH_BGPD, clear_vnc_prefix_all_cmd_vtysh, 
       "clear vnc prefix <*|A.B.C.D/M|X:X::X:X/M> *", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "From any NVE\n")

DEFSH (VTYSH_ISISD, debug_isis_lsp_sched_cmd_vtysh, 
       "debug isis lsp-sched", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS scheduling of LSP generation\n")

DEFSH (VTYSH_BGPD, no_bgp_cluster_id_cmd_vtysh, 
       "no bgp cluster-id [<A.B.C.D|(1-4294967295)>]", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Configure Route-Reflector Cluster-id\n"
       "Route-Reflector Cluster-id in IP address format\n"
       "Route-Reflector Cluster-id as 32 bit quantity\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_detail_name_cmd_vtysh, 
       "show ip prefix-list detail WORD", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Detail of prefix lists\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_cost_life_lnh_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> cost (0-255) lifetime (1-4294967295) LNH_OPTIONS...", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Administrative cost   [default: 255]\n"
       "Administrative cost\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_mpls_te_link_cmd_vtysh, 
       "show ip ospf mpls-te interface [INTERFACE]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "MPLS-TE information\n"
       "Interface information\n"
       "Interface name\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_homeagent_preference_cmd_vtysh, 
       "no ipv6 nd home-agent-preference [(0-65535)]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent preference\n"
       "preference value (default is 0,  least preferred)\n")

DEFSH (VTYSH_RIPD, ip_rip_v2_broadcast_cmd_vtysh, 
       "ip rip v2-broadcast", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Send ip broadcast v2 update\n")

DEFSH (VTYSH_BGPD, no_match_probability_cmd_vtysh, 
       "no match probability [(1-99)]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match portion of routes defined by percentage value\n"
       "Percentage of routes\n")

DEFSH (VTYSH_BGPD, no_set_ecommunity_soo_cmd_vtysh, 
       "no set extcommunity soo ASN:nn_or_IP-address:nn...", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP extended community attribute\n"
       "Site-of-Origin extended community\n"
       "VPN extended community\n")

DEFSH (VTYSH_LDPD, ldp_router_id_ipv4_cmd_vtysh, 
       "router-id A.B.C.D", 
       "Configure router Id\n"
       "LSR Id (in form of an IPv4 address)\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, clear_ipv6_prefix_list_name_prefix_cmd_vtysh, 
       "clear ipv6 prefix-list WORD X:X::X:X/M", 
       "Reset functions\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_detail_all_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor detail all [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "detail of all neighbors\n"
       "include down status neighbor\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, ip_route_label_cmd_vtysh, 
       "ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_PIMD, show_ip_pim_nexthop_cmd_vtysh, 
       "show ip pim nexthop", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM cached nexthop rpf information\n")

DEFSH (VTYSH_OSPFD, ospf_write_multiplier_cmd_vtysh, 
       "ospf write-multiplier (1-100)", 
       "OSPF specific commands\n"
       "Write multiplier\n"
       "Maximum number of interface serviced per write\n")

DEFSH (VTYSH_BGPD, vnc_no_nve_group_cmd_vtysh, 
       "no vnc nve-group NAME", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Configure a NVE group\n"
       "Group name\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_paths_cmd_vtysh, 
       "show [ip] bgp [""<unicast|multicast|vpn>""] paths", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Path information\n")

DEFSH (VTYSH_ISISD, no_spf_interval_l1_cmd_vtysh, 
       "no spf-interval level-1", 
       "Negate a command or set its defaults\n"
       "Minimum interval between SPF calculations\n"
       "Set interval for level 1 only\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_match_ipv6_address_prefix_list_cmd_vtysh, 
       "no match ipv6 address prefix-list WORD", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IPv6 information\n"
       "Match address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFSH (VTYSH_BGPD, no_bgp_bestpath_aspath_confed_cmd_vtysh, 
       "no bgp bestpath as-path confed", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Compare path lengths including confederation sets & sequences in selecting a route\n")

DEFSH (VTYSH_RIPNGD, no_router_ripng_cmd_vtysh, 
       "no router ripng", 
       "Negate a command or set its defaults\n"
       "Enable a routing process\n"
       "Make RIPng instance command\n")

DEFSH (VTYSH_BGPD, no_set_origin_cmd_vtysh, 
       "no set origin [<egp|igp|incomplete>]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP origin code\n"
       "remote EGP\n"
       "local IGP\n"
       "unknown heritage\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_community_info_cmd_vtysh, 
       "show [ip] bgp community-info", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "List all bgp community information\n")

DEFSH (VTYSH_ISISD, show_hostname_cmd_vtysh, 
       "show isis hostname", 
       "Show running system information\n"
       "IS-IS information\n"
       "IS-IS Dynamic hostname mapping\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_protocol_cmd_vtysh, 
       "show ip route [vrf NAME] " "<kernel|connected|static|rip|ospf|isis|bgp|pim|nhrp|table|vnc>", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_ZEBRA, no_debug_zebra_fpm_cmd_vtysh, 
       "no debug zebra fpm", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug zebra FPM events\n")

DEFSH (VTYSH_BGPD, neighbor_timers_connect_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> timers connect (1-65535)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP per neighbor timers\n"
       "BGP connect timer\n"
       "Connect timer\n")

DEFSH (VTYSH_PIMD, debug_pim_cmd_vtysh, 
       "debug pim", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n")

DEFSH (VTYSH_ISISD, no_spf_interval_l2_cmd_vtysh, 
       "no spf-interval level-2", 
       "Negate a command or set its defaults\n"
       "Minimum interval between SPF calculations\n"
       "Set interval for level 2 only\n")

DEFSH (VTYSH_OSPFD, debug_ospf_instance_nsm_cmd_vtysh, 
       "debug ospf (1-65535) nsm [<status|events|timers>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Neighbor State Machine\n"
       "NSM Status Information\n"
       "NSM Event Information\n"
       "NSM Timer Information\n")

DEFSH (VTYSH_BGPD, bgp_maxmed_onstartup_cmd_vtysh, 
       "bgp max-med on-startup (5-86400)", 
       "BGP information\n"
       "Advertise routes with max-med\n"
       "Effective on a startup\n"
       "Time (seconds) period for max-med\n")

DEFSH (VTYSH_BGPD, clear_bgp_ipv6_safi_prefix_cmd_vtysh, 
       "clear [ip] bgp ipv6 ""<unicast|multicast|vpn>"" prefix X:X::X:X/M", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "Address Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Clear bestpath and re-advertise\n"
       "IPv6 prefix\n")

DEFSH (VTYSH_BGPD, debug_rfapi_show_import_cmd_vtysh, 
       "debug rfapi-dev show import", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "Show running system information\n"
       "import\n")

DEFSH (VTYSH_RIPD, no_rip_network_cmd_vtysh, 
       "no network <A.B.C.D/M|WORD>", 
       "Negate a command or set its defaults\n"
       "Enable routing on an IP network\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Interface name\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_hello_interval_cmd_vtysh, 
              "ospf hello-interval (1-65535) [A.B.C.D]", 
              "OSPF interface commands\n"
              "Time between HELLO packets\n"
              "Seconds\n"
              "Address of interface\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_explicit_null_for_acl_cmd_vtysh, 
       "no label local advertise explicit-null for <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "Configure explicit-null advertisement\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ISISD, no_lsp_refresh_interval_cmd_vtysh, 
       "no lsp-refresh-interval [<level-1|level-2>] [(1-65235)]", 
       "Negate a command or set its defaults\n"
       "LSP refresh interval\n"
       "LSP refresh interval for Level 1 only\n"
       "LSP refresh interval for Level 2 only\n"
       "LSP refresh interval in seconds\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_prefix_first_match_cmd_vtysh, 
       "show ipv6 prefix-list WORD X:X::X:X/M first-match", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "First matched prefix\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_seq_ge_le_cmd_vtysh, 
       "no ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M ge (0-128) le (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_BGPD, ip_lcommunity_list_expanded_cmd_vtysh, 
       "ip large-community-list (100-500) <deny|permit> LINE...", 
       "IP information\n"
       "Add a large community list entry\n"
       "Large Community list number (expanded)\n"
       "Specify large community to reject\n"
       "Specify large community to accept\n"
       "An ordered list as a regular-expression\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_prefix_cost_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M> cost (0-255)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Administrative cost   [default: 255]\n" "Administrative cost\n")

DEFSH (VTYSH_ZEBRA, show_interface_name_vrf_all_cmd_vtysh, 
       "show interface IFNAME [vrf all]", 
       "Show running system information\n"
       "Interface status and configuration\n"
       "Interface name\n"
       "Specify the VRF\nAll VRFs\n")

DEFSH (VTYSH_RIPNGD, ipv6_ripng_split_horizon_cmd_vtysh, 
       "ipv6 ripng split-horizon", 
       "IPv6 information\n"
       "Routing Information Protocol\n"
       "Perform split horizon\n")

DEFSH (VTYSH_BGPD, debug_bgp_allow_martians_cmd_vtysh, 
       "debug bgp allow-martians", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP allow martian next hops\n")

DEFSH (VTYSH_ISISD, isis_hello_multiplier_cmd_vtysh, 
       "isis hello-multiplier (2-100)", 
       "IS-IS commands\n"
       "Set multiplier for Hello holding time\n"
       "Hello multiplier value\n")

DEFSH (VTYSH_BGPD, neighbor_maximum_prefix_threshold_restart_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> maximum-prefix (1-4294967295) (1-100) restart (1-65535)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Maximum number of prefixes to accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Threshold value (%) at which to generate a warning msg\n"
       "Restart bgp connection after limit is exceeded\n"
       "Restart interval in minutes\n")

DEFSH (VTYSH_OSPF6D, area_range_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> range X:X::X:X/M [<advertise|not-advertise|cost (0-16777215)>]", 
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "OSPF6 area ID as a decimal value\n"
       "Configured address range\n"
       "Specify IPv6 prefix\n"
       "Advertise\n"
       "Do not advertise\n"
       "User specified metric for this range\n"
       "Advertised metric for this range\n")

DEFSH (VTYSH_RIPD, no_rip_distance_source_access_list_cmd_vtysh, 
       "no distance (1-255) A.B.C.D/M WORD", 
       "Negate a command or set its defaults\n"
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n"
       "Access list name\n")

DEFSH (VTYSH_LDPD, ldp_neighbor_lsr_id_ipv4_cmd_vtysh, 
       "neighbor lsr-id A.B.C.D", 
       "Remote endpoint configuration\n"
       "Specify the LSR-ID of the remote endpoint\n"
       "IPv4 address\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_mtu_ignore_cmd_vtysh, 
       "ipv6 ospf6 mtu-ignore", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Disable MTU mismatch detection on this interface\n"
       )

DEFSH (VTYSH_OSPFD, no_ospf_passive_interface_addr_cmd_vtysh, 
       "no passive-interface <IFNAME [A.B.C.D]|default>", 
       "Negate a command or set its defaults\n"
       "Allow routing updates on an interface\n"
       "Interface's name\n"
       "IPv4 address\n"
       "Allow routing updates on interfaces by default\n")

DEFSH (VTYSH_BGPD, bgp_bestpath_compare_router_id_cmd_vtysh, 
       "bgp bestpath compare-routerid", 
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "Compare router-id for identical EBGP paths\n")

DEFSH (VTYSH_OSPF6D, no_redistribute_ospf6_cmd_vtysh, 
       "no redistribute ospf6", 
       "Negate a command or set its defaults\n"
       "Redistribute control\n"
       "OSPF6 route\n")

DEFSH (VTYSH_PIMD, ip_pim_joinprune_time_cmd_vtysh, 
       "ip pim join-prune-interval (60-600)", 
       "IP information\n"
       "pim multicast routing\n"
       "Join Prune Send Interval\n"
       "Seconds\n")

DEFSH (VTYSH_BGPD, no_ip_extcommunity_list_expanded_all_cmd_vtysh, 
       "no ip extcommunity-list <(100-500)|expanded WORD> <deny|permit> LINE...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a extended community list entry\n"
       "Extended Community list number (expanded)\n"
       "Specify expanded extcommunity-list\n"
       "Extended Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "An ordered list as a regular-expression\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_as4_cmd_vtysh, 
       "no debug bgp as4", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP AS4 actions\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_ipv6_targeted_cmd_vtysh, 
       "no neighbor X:X::X:X targeted", 
       "Negate a command or set its defaults\n"
       "Configure neighbor parameters\n"
       "IPv6 address of neighbor\n"
       "Establish targeted session\n")

DEFSH (VTYSH_RIPNGD, no_ripng_passive_interface_cmd_vtysh, 
       "no passive-interface IFNAME", 
       "Negate a command or set its defaults\n"
       "Suppress routing updates on an interface\n"
       "Interface name\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_flags_cmd_vtysh, 
 "no " "<ip|ipv6>" " nhrp <shortcut|redirect>", 
 "Negate a command or set its defaults\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Allow shortcut establishment\n"
 "Send redirect notifications\n")

DEFSH (VTYSH_OSPFD, debug_ospf_nssa_cmd_vtysh, 
       "debug ospf nssa", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF nssa information\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, clear_ipv6_prefix_list_cmd_vtysh, 
       "clear ipv6 prefix-list", 
       "Reset functions\n"
       "IPv6 information\n"
       "Build a prefix list\n")

DEFSH (VTYSH_PIMD, debug_igmp_cmd_vtysh, 
       "debug igmp", 
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n")

DEFSH (VTYSH_BGPD, ipv6_bgp_network_route_map_cmd_vtysh, 
       "network X:X::X:X/M route-map WORD", 
       "Specify a network to announce via BGP\n"
       "IPv6 prefix\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_ZEBRA, ip_route_mask_cmd_vtysh, 
       "ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_explicit_null_for_acl_cmd_vtysh, 
       "label local advertise explicit-null for <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "Configure explicit-null advertisement\n"
       "IP access-list for destination prefixes\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_ISISD, spf_interval_l1_cmd_vtysh, 
       "spf-interval level-1 (1-120)", 
       "Minimum interval between SPF calculations\n"
       "Set interval for level 1 only\n"
       "Minimum interval between consecutive SPFs in seconds\n")

DEFSH (VTYSH_BGPD, bgp_graceful_restart_cmd_vtysh, 
       "bgp graceful-restart", 
       "BGP specific commands\n"
       "Graceful restart capability parameters\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_transmit_delay_addr_cmd_vtysh, 
       "no ip ospf transmit-delay [(1-65535)] [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Link state transmit delay\n"
       "Address of interface")

DEFSH (VTYSH_OSPFD, ospf_area_nssa_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> nssa", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n")

DEFSH (VTYSH_PIMD, debug_mroute_cmd_vtysh, 
       "debug mroute", 
       "Debugging functions (see also 'undebug')\n"
       "PIM interaction with kernel MFC cache\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_summary_cmd_vtysh, 
       "show ip route [vrf NAME] summary", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Summary of all routes\n")

/* SYSTEM MODIFIED by chenc for bug 49502, 2018-10-16 */
/*write ospf fib num to file*/
#ifdef _CENTEC_
DEFSH (VTYSH_ZEBRA, show_ip_route_summary_write_cmd_vtysh, 
       "show ip route [vrf NAME] summary write", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Summary of all routes\n"
       "Write ospf fib num to file\n")
#endif

DEFSH (VTYSH_BGPD, vnc_redistribute_nvegroup_cmd_vtysh, 
       "vnc redistribute nve-group NAME", 
       "VNC/RFP related configuration\n"
       "Assign a NVE group to routes redistributed from another routing protocol\n"
       "NVE group\n" "Group name\n")

DEFSH (VTYSH_BGPD, no_neighbor_capability_dynamic_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> capability dynamic", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Advertise capability to the peer\n"
       "Advertise dynamic capability to this neighbor\n")

DEFSH (VTYSH_PIMD, no_ip_pim_joinprune_time_cmd_vtysh, 
       "no ip pim join-prune-interval (60-600)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Join Prune Send Interval\n"
       "Seconds\n")

DEFSH (VTYSH_RIPNGD, no_debug_ripng_zebra_cmd_vtysh, 
       "no debug ripng zebra", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng and zebra communication\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_tag_cmd_vtysh, 
       "show ip route [vrf NAME] tag (1-4294967295)", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Show only routes with tag\n"
       "Tag value\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_pref_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_PIMD, no_debug_igmp_trace_cmd_vtysh, 
       "no debug igmp trace", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n"
       "IGMP internal daemon activity\n")

DEFSH (VTYSH_BGPD, neighbor_nexthop_self_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> next-hop-self", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Disable the next hop calculation for this neighbor\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_vrf_all_summary_prefix_cmd_vtysh, 
       "show ip route vrf all summary prefix", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Summary of all routes\n"
       "Prefix routes\n")

DEFSH (VTYSH_RIPD, ip_rip_authentication_mode_cmd_vtysh, 
       "ip rip authentication mode <md5|text> [auth-length <rfc|old-ripd>]", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Authentication control\n"
       "Authentication mode\n"
       "Keyed message digest\n"
       "Clear text authentication\n"
       "MD5 authentication data length\n"
       "RFC compatible\n"
       "Old ripd compatible\n")

DEFSH (VTYSH_ZEBRA, match_ip_nexthop_prefix_len_cmd_vtysh, 
       "match ip next-hop prefix-len (0-32)", 
       "Match values from routing table\n"
       "IP information\n"
       "Match prefixlen of nexthop ip address\n"
       "Match prefixlen of given nexthop\n"
       "Prefix length\n")

DEFSH (VTYSH_BGPD, no_bgp_set_route_map_delay_timer_cmd_vtysh, 
       "no bgp route-map delay-timer [(0-600)]", 
       "Negate a command or set its defaults\n"
       "BGP information\n"
       "Default BGP route-map delay timer\n"
       "Reset to default time to wait for processing route-map changes\n"
       "0 disables the timer,  no route updates happen when route-maps change\n")

DEFSH (VTYSH_ALL, exec_timeout_sec_cmd_vtysh, 
       "exec-timeout (0-35791) (0-2147483)", 
       "Set the EXEC timeout\n"
       "Timeout in minutes\n"
       "Timeout in seconds\n")

DEFSH (VTYSH_BGPD, set_aspath_exclude_cmd_vtysh, 
       "set as-path exclude (1-4294967295)...", 
       "Set values in destination routing protocol\n"
       "Transform BGP AS-path attribute\n"
       "Exclude from the as-path\n"
       "AS number\n")

DEFSH (VTYSH_BGPD, no_neighbor_filter_list_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> filter-list WORD <in|out>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")

DEFSH (VTYSH_OSPF6D, ospf6_interface_area_cmd_vtysh, 
       "interface IFNAME area A.B.C.D", 
       "Enable routing on an IPv6 interface\n"
       "Interface name(e.g. ep0)\n"
       "Specify the OSPF6 area ID\n"
       "OSPF6 area ID in IPv4 address notation\n"
      )

DEFSH (VTYSH_BGPD, neighbor_peer_group_cmd_vtysh, 
       "neighbor WORD peer-group", 
       "Specify neighbor router\n"
       "Interface name or neighbor tag\n"
       "Configure peer-group\n")

DEFSH (VTYSH_BGPD, bgp_network_mask_natural_route_map_cmd_vtysh, 
       "network A.B.C.D route-map WORD", 
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Route-map to modify the attributes\n"
       "Name of the route map\n")

DEFSH (VTYSH_ALL, show_ip_access_list_cmd_vtysh, 
       "show ip access-list", 
       "Show running system information\n"
       "IP information\n"
       "List IP access lists\n")

DEFSH (VTYSH_ZEBRA, ip_route_mask_label_cmd_vtysh, 
       "ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_instance_lsa_cmd_vtysh, 
       "no debug ospf (1-65535) lsa [<generate|flooding|install|refresh>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Link State Advertisement\n"
       "LSA Generation\n"
       "LSA Flooding\n"
       "LSA Install/Delete\n"
       "LSA Refres\n")

DEFSH (VTYSH_NHRPD, no_tunnel_source_cmd_vtysh, 
 "no tunnel source", 
 "NHRP/GRE integration\n"
 "Tunnel device binding tracking\n"
 "Interface name\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_transport_address_ipv6_cmd_vtysh, 
       "no discovery transport-address X:X::X:X", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "Specify transport address for TCP connection\n"
       "IPv6 address to be used as transport address\n")

DEFSH (VTYSH_BGPD, ipv6_bgp_distance_source_access_list_cmd_vtysh, 
       "distance (1-255) X:X::X:X/M WORD", 
       "Define an administrative distance\n"
       "Administrative distance\n"
       "IP source prefix\n"
       "Access list name\n")

DEFSH (VTYSH_ZEBRA, no_match_source_protocol_cmd_vtysh, 
       "no match source-protocol [<bgp|ospf|rip|ripng|isis|ospf6|connected|system|kernel|static>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "No match protocol via which the route was learnt\n"
       "BGP protocol\n"
       "OSPF protocol\n"
       "RIP protocol\n"
       "RIPNG protocol\n"
       "ISIS protocol\n"
       "OSPF6 protocol\n"
       "Routes from directly connected peer\n"
       "Routes from system configuration\n"
       "Routes from kernel\n"
       "Statically configured routes\n")

DEFSH (VTYSH_LDPD, ldp_debug_mpls_ldp_errors_cmd_vtysh, 
       "debug mpls ldp errors", 
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Errors\n")

DEFSH (VTYSH_PIMD, no_debug_msdp_packets_cmd_vtysh, 
       "no debug msdp packets", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "MSDP protocol activity\n"
       "MSDP protocol packets\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_ipv6_prefix_list_seq_ge_cmd_vtysh, 
       "no ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M ge (0-128)", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X>", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_life_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> lifetime (1-4294967295)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n")

DEFSH (VTYSH_PIMD, debug_ssmpingd_cmd_vtysh, 
       "debug ssmpingd", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "ssmpingd activity\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, set_metric_cmd_vtysh, 
       "set metric <(0-4294967295)|rtt|+rtt|-rtt|+metric|-metric>", 
       "Set values in destination routing protocol\n"
       "Metric value for destination routing protocol\n"
       "Metric value\n"
       "Assign round trip time\n"
       "Add round trip time\n"
       "Subtract round trip time\n"
       "Add metric\n"
       "Subtract metric\n")

DEFSH (VTYSH_BGPD, debug_bgp_keepalive_peer_cmd_vtysh, 
       "debug bgp keepalives <A.B.C.D|X:X::X:X|WORD>", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Neighbor Events\n"
       "BGP neighbor IP address to debug\n"
       "BGP IPv6 neighbor to debug\n"
       "BGP neighbor on interface to debug\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_instance_all_nexthop_cmd_vtysh, 
       "show [ip] bgp <view|vrf> all nexthop", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nAll Views/VRFs\n"
       "BGP nexthop table\n")

DEFSH (VTYSH_BGPD, no_match_origin_cmd_vtysh, 
       "no match origin [<egp|igp|incomplete>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "BGP origin code\n"
       "remote EGP\n"
       "local IGP\n"
       "unknown heritage\n")

DEFSH (VTYSH_RIPNGD, show_debugging_ripng_cmd_vtysh, 
       "show debugging ripng", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n")

DEFSH (VTYSH_BGPD, show_bgp_memory_cmd_vtysh, 
       "show [ip] bgp memory", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Global BGP memory statistics\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_border_routers_cmd_vtysh, 
       "show ip ospf border-routers", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Show all the ABR's and ASBR's\n")

DEFSH (VTYSH_BGPD, show_bgp_instance_updgrps_stats_cmd_vtysh, 
       "show [ip] bgp <view|vrf> WORD update-groups statistics", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Detailed info about dynamic update groups\n"
       "Statistics\n")

DEFSH (VTYSH_BGPD, vnc_l2_group_labels_cmd_vtysh, 
       "labels LABELLIST...", 
       "Specify label values associated with group\n"
       "Space separated list of label values <0-1048575>\n")

DEFSH (VTYSH_BGPD, neighbor_nexthop_local_unchanged_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> nexthop-local unchanged", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Configure treatment of outgoing link-local nexthop attribute\n"
       "Leave link-local nexthop unchanged for this peer\n")

DEFSH (VTYSH_BGPD, clear_vnc_prefix_vn_cmd_vtysh, 
       "clear vnc prefix <*|A.B.C.D/M|X:X::X:X/M> vn <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "All prefixes\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "UN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_BGPD, debug_rfapi_close_rfd_cmd_vtysh, 
       "debug rfapi-dev close rfd HANDLE", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "rfapi_close\n"
       "indicate handle follows\n" "rfapi handle in hexadecimal\n")

DEFSH (VTYSH_PIMD, interface_ip_igmp_query_interval_cmd_vtysh, 
       "ip igmp query-interval (1-1800)", 
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP host query interval\n"
       "Query interval in seconds\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_neighbor_int_cmd_vtysh, 
       "show ip ospf (1-65535) neighbor IFNAME [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Neighbor list\n"
       "Interface name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, no_ip_irdp_cmd_vtysh, 
       "no ip irdp", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Disable ICMP Router discovery on this interface\n")

DEFSH (VTYSH_BGPD, vnc_show_registrations_pfx_cmd_vtysh, 
       "show vnc registrations [<A.B.C.D/M|X:X::X:X/M|YY:YY:YY:YY:YY:YY>]", 
       "Show running system information\n"
       "VNC information\n"
       "List active prefix registrations\n"
       "Limit output to a particular IPv4 prefix\n"
       "Limit output to a particular IPv6 prefix\n"
       "Limit output to a particular IPv6 address\n")

DEFSH (VTYSH_OSPF6D, ospf6_log_adjacency_changes_cmd_vtysh, 
       "log-adjacency-changes", 
       "Log changes in adjacency state\n")

DEFSH (VTYSH_RIPD, rip_version_cmd_vtysh, 
       "version (1-2)", 
       "Set routing protocol version\n"
       "version\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_zebra_cmd_vtysh, 
       "no debug mpls ldp zebra", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "LDP zebra information\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_neighbor_received_prefix_filter_cmd_vtysh, 
       "show [ip] bgp [<ipv4|ipv6> [unicast]] neighbors <A.B.C.D|X:X::X:X|WORD> received prefix-filter [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Address Family\n"
       "Address Family\n"
       "Address Family modifier\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n"
       "Neighbor on BGP configured interface\n"
       "Display information received from a BGP neighbor\n"
       "Display the prefixlist filter\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_cmd_vtysh, 
       "show ip prefix-list", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n")

DEFSH (VTYSH_ISISD, no_isis_hello_multiplier_l2_cmd_vtysh, 
       "no isis hello-multiplier [(2-100)] level-2", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set multiplier for Hello holding time\n"
       "Hello multiplier value\n"
       "Specify hello multiplier for level-2 IIHs\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_vnc_cmd_vtysh, 
       "<no debug|undebug> bgp vnc <rfapi-query|import-bi-attach|import-del-remote|verbose>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Undebug\n"
       "BGP information\n"
       "VNC information\n"
       "rfapi query handling\n"
       "import BI atachment\n"
       "import delete remote routes\n"
       "verbose logging\n")

DEFSH (VTYSH_BGPD, clear_ip_bgp_dampening_cmd_vtysh, 
       "clear ip bgp dampening", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "Clear route flap dampening information\n")

DEFSH (VTYSH_BGPD, no_bgp_timers_cmd_vtysh, 
       "no timers bgp [(0-65535) (0-65535)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "BGP timers\n"
       "Keepalive interval\n"
       "Holdtime\n")

DEFSH (VTYSH_OSPFD, no_ospf_write_multiplier_cmd_vtysh, 
       "no ospf write-multiplier (1-100)", 
       "Negate a command or set its defaults\n"
       "OSPF specific commands\n"
       "Write multiplier\n"
       "Maximum number of interface serviced per write\n")

DEFSH (VTYSH_PIMD, interface_ip_igmp_query_max_response_time_cmd_vtysh, 
       "ip igmp query-max-response-time (10-250)", 
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP max query response value (seconds)\n"
       "Query response value in deci-seconds\n")

DEFSH (VTYSH_ZEBRA, ip_route_distance_label_cmd_vtysh, 
       "ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> (1-255) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_NHRPD, tunnel_protection_cmd_vtysh, 
 "tunnel protection vici profile PROFILE [fallback-profile FALLBACK]", 
 "NHRP/GRE integration\n"
 "IPsec protection\n"
 "VICI (StrongSwan)\n"
 "IPsec profile\n"
 "IPsec profile name\n"
 "Fallback IPsec profile\n"
 "Fallback IPsec profile name\n")

DEFSH (VTYSH_OSPF6D, no_ipv6_ospf6_passive_cmd_vtysh, 
       "no ipv6 ospf6 passive", 
       "Negate a command or set its defaults\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "passive interface: No Adjacency will be formed on this I/F\n"
       )

DEFSH (VTYSH_OSPFD, ospf_area_authentication_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> authentication", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Enable authentication\n")

DEFSH (VTYSH_PIMD, show_ip_pim_ssm_range_cmd_vtysh, 
       "show ip pim group-type [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM group type\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, no_debug_isis_snp_cmd_vtysh, 
       "no debug isis snp-packets", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS CSNP/PSNP packets\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_spf_tree_cmd_vtysh, 
       "show ipv6 ospf6 spf tree", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Shortest Path First caculation\n"
       "Show SPF tree\n")

DEFSH (VTYSH_BGPD, bgp_maxpaths_ibgp_cmd_vtysh, 
       "maximum-paths ibgp (1-255)", 
       "Forward packets over multiple paths\n"
       "iBGP-multipath\n"
       "Number of paths\n")

DEFSH (VTYSH_ZEBRA, no_ip_address_cmd_vtysh, 
       "no ip address A.B.C.D/M", 
       "Negate a command or set its defaults\n"
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP Address (e.g. 10.0.0.1/8)")

DEFSH (VTYSH_BGPD, show_ip_as_path_access_list_all_cmd_vtysh, 
       "show ip as-path-access-list", 
       "Show running system information\n"
       "IP information\n"
       "List AS path access lists\n")

DEFSH (VTYSH_BGPD, neighbor_dont_capability_negotiate_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> dont-capability-negotiate", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Do not perform capability negotiation\n")

DEFSH (VTYSH_PIMD, clear_ip_interfaces_cmd_vtysh, 
       "clear ip interfaces", 
       "Reset functions\n"
       "IP information\n"
       "Reset interfaces\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_managed_config_flag_cmd_vtysh, 
       "no ipv6 nd managed-config-flag", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Managed address configuration flag\n")

DEFSH (VTYSH_BGPD, vnc_redistribute_rh_roo_localadmin_cmd_vtysh, 
       "vnc redistribute resolve-nve roo-ec-local-admin (0-65535)", 
       "VNC/RFP related configuration\n"
       "Redistribute routes into VNC\n"
       "Resolve-NVE mode\n"
       "Route Origin Extended Community Local Admin Field\n" "Field value\n")

DEFSH (VTYSH_BGPD, neighbor_update_source_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> update-source <A.B.C.D|X:X::X:X|WORD>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Source of routing updates\n"
       "IPv4 address\n" "IPv6 address\n" "Interface name (requires zebra to be running)\n")

DEFSH (VTYSH_OSPF6D, ospf6_timers_throttle_spf_cmd_vtysh, 
       "timers throttle spf (0-600000) (0-600000) (0-600000)", 
       "Adjust routing timers\n"
       "Throttling adaptive timer\n"
       "OSPF6 SPF timers\n"
       "Delay (msec) from first change received till SPF calculation\n"
       "Initial hold time (msec) between consecutive SPF calculations\n"
       "Maximum hold time (msec)\n")

DEFSH (VTYSH_ALL, no_access_list_extended_any_mask_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip any A.B.C.D A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Destination address\n"
       "Destination Wildcard bits\n")

DEFSH (VTYSH_RIPD, rip_redistribute_rip_cmd_vtysh, 
       "redistribute rip", 
       "Redistribute information from another routing protocol\n"
       "Routing Information Protocol (RIP)\n")

DEFSH (VTYSH_PIMD, no_ip_pim_spt_switchover_infinity_cmd_vtysh, 
       "no ip pim spt-switchover infinity-and-beyond", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "PIM information\n"
       "SPT_Switchover\n"
       "Never switch to SPT Tree\n")

DEFSH (VTYSH_OSPF6D, ipv6_ospf6_ifmtu_cmd_vtysh, 
       "ipv6 ospf6 ifmtu (1-65535)", 
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Interface MTU\n"
       "OSPFv3 Interface MTU\n"
       )

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_detail_name_cmd_vtysh, 
       "show ipv6 prefix-list detail WORD", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Detail of prefix lists\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_ra_interval_cmd_vtysh, 
       "no ipv6 nd ra-interval [<(1-1800)|msec (1-1800000)>]", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n"
       "Router Advertisement interval in seconds\n"
       "Specify millisecond router advertisement interval\n"
       "Router Advertisement interval in milliseconds\n")

DEFSH (VTYSH_BGPD, show_bgp_afi_vpn_rd_route_cmd_vtysh, 
       "show bgp ""<ipv4|ipv6>"" vpn rd ASN:nn_or_IP-address:nn <A.B.C.D/M|X:X::X:X/M> [json]", 
       "Show running system information\n"
       "BGP information\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n"
       "Display information for a route distinguisher\n"
       "Route Distinguisher\n"
       "Network in the BGP routing table to display\n"
       "Network in the BGP routing table to display\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_detail_cmd_vtysh, 
       "show ipv6 prefix-list detail", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Detail of prefix lists\n")

DEFSH (VTYSH_ALL, access_list_extended_host_mask_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip host A.B.C.D A.B.C.D A.B.C.D", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Destination address\n"
       "Destination Wildcard bits\n")

DEFSH (VTYSH_BGPD, vnc_nve_export_no_routemap_cmd_vtysh, 
       "no vnc export <bgp|zebra> route-map [NAME]", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "Route-map for filtering exported routes\n" "Route map name\n")

DEFSH (VTYSH_ZEBRA, link_params_delay_cmd_vtysh, 
       "delay (0-16777215) [min (0-16777215) max (0-16777215)]", 
       "Unidirectional Average Link Delay\n"
       "Average delay in micro-second as decimal (0...16777215)\n"
       "Minimum delay\n"
       "Minimum delay in micro-second as decimal (0...16777215)\n"
       "Maximum delay\n"
       "Maximum delay in micro-second as decimal (0...16777215)\n")

DEFSH (VTYSH_RIPD, rip_offset_list_cmd_vtysh, 
       "offset-list WORD <in|out> (0-16)", 
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_binding_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> binding", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "Label Information Base (LIB) information\n")

DEFSH (VTYSH_BGPD, bgp_network_mask_backdoor_cmd_vtysh, 
       "network A.B.C.D mask A.B.C.D backdoor", 
       "Specify a network to announce via BGP\n"
       "Network number\n"
       "Network mask\n"
       "Network mask\n"
       "Specify a BGP backdoor route\n")

DEFSH (VTYSH_OSPFD, capability_opaque_cmd_vtysh, 
       "capability opaque", 
       "Enable specific OSPF feature\n"
       "Opaque LSA\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv6_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct>", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n")

DEFSH (VTYSH_ZEBRA, linkdetect_cmd_vtysh, 
       "link-detect", 
       "Enable link detection on interface\n")

DEFSH (VTYSH_ISISD, debug_isis_rtevents_cmd_vtysh, 
       "debug isis route-events", 
       "Debugging functions (see also 'undebug')\n"
       "IS-IS information\n"
       "IS-IS Route related events\n")

DEFSH (VTYSH_BGPD, vnc_redist_bgpdirect_routemap_cmd_vtysh, 
       "vnc redistribute <bgp-direct|bgp-direct-to-nve-groups> route-map NAME", 
       "VNC/RFP related configuration\n"
       "Redistribute from other protocols\n"
       "Redistribute from BGP directly\n"
       "Redistribute from BGP without Zebra,  only to configured NVE groups\n"
       "Route-map for filtering exported routes\n" "route map name\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_router_info_pce_cmd_vtysh, 
       "show ip ospf router-info pce", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Router Information\n"
       "PCE information\n")

DEFSH (VTYSH_BGPD, no_match_lcommunity_cmd_vtysh, 
       "no match large-community [<(1-99)|(100-500)|WORD>]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match BGP large community list\n"
       "Large Community-list number (standard)\n"
       "Large Community-list number (expanded)\n"
       "Large Community-list name\n")

DEFSH (VTYSH_BGPD, vnc_no_export_nvegroup_cmd_vtysh, 
       "vnc export <bgp|zebra> group-nve no group NAME", 
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "NVE group,  used in 'group-nve' export mode\n"
       "Disable export of VNC routes\n" "NVE group\n" "Group name\n")

DEFSH (VTYSH_LDPD, ldp_no_label_remote_accept_from_acl_cmd_vtysh, 
       "no label remote accept from <(1-199)|(1300-2699)|WORD>", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure remote/peer label control and policies\n"
       "Configure inbound label acceptance control\n"
       "Neighbor from whom to accept label advertisement\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_OSPF6D, area_import_list_cmd_vtysh, 
       "area A.B.C.D import-list NAME", 
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "Set the filter for networks from other areas announced to the specified one\n"
       "Name of the acess-list\n")

DEFSH (VTYSH_ZEBRA, ip_address_label_cmd_vtysh, 
       "ip address A.B.C.D/M label LINE", 
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP address (e.g. 10.0.0.1/8)\n"
       "Label of this address\n"
       "Label\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_transport_address_ipv4_cmd_vtysh, 
       "no discovery transport-address A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "Specify transport address for TCP connection\n"
       "IP address to be used as transport address\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_update_cmd_vtysh, 
       "no debug bgp updates", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP updates\n")

DEFSH_HIDDEN (VTYSH_PIMD, ip_multicast_routing_cmd_vtysh, 
              "ip multicast-routing", 
              "IP information\n"
              "Enable IP multicast forwarding\n")

DEFSH (VTYSH_OSPF6D, debug_ospf6_spf_process_cmd_vtysh, 
       "debug ospf6 spf process", 
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug SPF Calculation\n"
       "Debug Detailed SPF Process\n"
      )

DEFSH (VTYSH_BGPD, no_match_aspath_cmd_vtysh, 
       "no match as-path [WORD]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "Match BGP AS path list\n"
       "AS path access-list name\n")

DEFSH (VTYSH_RIPD, debug_rip_packet_cmd_vtysh, 
       "debug rip packet", 
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP packet\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_ra_interval_msec_cmd_vtysh, 
       "ipv6 nd ra-interval msec (70-1800000)", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n"
       "Router Advertisement interval in milliseconds\n"
       "Router Advertisement interval in milliseconds\n")

DEFSH (VTYSH_BGPD, neighbor_ttl_security_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> ttl-security hops (1-254)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "BGP ttl-security parameters\n"
       "Specify the maximum number of hops to the BGP peer\n"
       "Number of hops to BGP peer\n")

DEFSH (VTYSH_BGPD, vnc_redistribute_mode_cmd_vtysh, 
       "vnc redistribute mode <nve-group|plain|resolve-nve>", 
       "VNC/RFP related configuration\n"
       "Redistribute routes into VNC\n"
       "Redistribution mode\n"
       "Based on redistribute nve-group\n"
       "Unmodified\n" "Resolve each nexthop to connected NVEs\n")

DEFSH (VTYSH_ZEBRA, pseudowire_neighbor_cmd_vtysh, 
       "[no] neighbor <A.B.C.D|X:X::X:X>", 
       "Negate a command or set its defaults\n"
       "Specify the IPv4 or IPv6 address of the remote endpoint\n"
       "IPv4 address\n"
       "IPv6 address\n")

DEFSH (VTYSH_BGPD, debug_rfapi_show_import_vn_un_cmd_vtysh, 
       "debug rfapi-dev show import vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X>", 
       "Debugging functions (see also 'undebug')\n"
       "RF API debugging/testing command\n"
       "Show running system information\n"
       "import\n"
       "indicate vn addr follows\n"
       "virtual network interface IPv4 address\n"
       "virtual network interface IPv6 address\n"
       "indicate xt addr follows\n"
       "underlay network interface IPv4 address\n"
       "underlay network interface IPv6 address\n")

DEFSH (VTYSH_RIPNGD, ripng_timers_cmd_vtysh, 
       "timers basic (0-65535) (0-65535) (0-65535)", 
       "RIPng timers setup\n"
       "Basic timer\n"
       "Routing table update timer value in second. Default is 30.\n"
       "Routing information timeout timer. Default is 180.\n"
       "Garbage collection timer. Default is 120.\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_neighbor_cmd_vtysh, 
       "show mpls ldp neighbor", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Neighbor information\n")

DEFSH (VTYSH_ALL, no_ipv6_access_list_all_cmd_vtysh, 
       "no ipv6 access-list WORD", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n")

DEFSH (VTYSH_BGPD, clear_vnc_nve_un_vn_cmd_vtysh, 
       "clear vnc nve un <*|A.B.C.D|X:X::X:X> vn <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear prefix registration information\n"
       "UN address of NVE\n"
       "For all un NVEs\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "VN address of NVE\n"
       "For all vn NVEs\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_PIMD, show_ip_msdp_peer_detail_cmd_vtysh, 
       "show ip msdp peer [detail|A.B.C.D] [json]", 
       "Show running system information\n"
       "IP information\n"
       "MSDP information\n"
       "MSDP peer information\n"
       "Detailed output\n"
       "peer ip address\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ALL, access_list_exact_cmd_vtysh, 
       "access-list WORD <deny|permit> A.B.C.D/M [exact-match]", 
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Exact match of the prefixes\n")

DEFSH (VTYSH_BGPD, debug_bgp_neighbor_events_cmd_vtysh, 
       "debug bgp neighbor-events", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP Neighbor Events\n")

DEFSH (VTYSH_BGPD, bgp_maxpaths_ibgp_cluster_cmd_vtysh, 
       "maximum-paths ibgp (1-255) equal-cluster-length", 
       "Forward packets over multiple paths\n"
       "iBGP-multipath\n"
       "Number of paths\n"
       "Match the cluster length\n")

DEFSH (VTYSH_ISISD, no_debug_isis_packet_dump_cmd_vtysh, 
       "no debug isis packet-dump", 
       "Negate a command or set its defaults\n"
       "Disable debugging functions (see also 'debug')\n"
       "IS-IS information\n"
       "IS-IS packet dump\n")

DEFSH (VTYSH_OSPF6D, no_ospf6_interface_area_cmd_vtysh, 
       "no interface IFNAME area A.B.C.D", 
       "Negate a command or set its defaults\n"
       "Disable routing on an IPv6 interface\n"
       "Interface name(e.g. ep0)\n"
       "Specify the OSPF6 area ID\n"
       "OSPF6 area ID in IPv4 address notation\n"
       )

DEFSH (VTYSH_BGPD, no_neighbor_as_override_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> as-override", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Override ASNs in outbound updates if aspath equals remote-as\n")

DEFSH (VTYSH_ALL, no_access_list_extended_any_any_cmd_vtysh, 
       "no access-list <(100-199)|(2000-2699)> <deny|permit> ip any any", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Any destination host\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, show_ip_prefix_list_prefix_cmd_vtysh, 
       "show ip prefix-list WORD A.B.C.D/M", 
       "Show running system information\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_regexp_cmd_vtysh, 
       "show [ip] bgp [<view|vrf> WORD] [""<ipv4|ipv6>"" [""<unicast|multicast|vpn>""]] regexp REGEX...", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Address Family\nAddress Family\n"
       "Address Family modifier\n" "Address Family modifier\n" "Address Family modifier\n"
       "Display routes matching the AS path regular expression\n"
       "A regular-expression to match the BGP AS paths\n")

DEFSH (VTYSH_PIMD, debug_igmp_events_cmd_vtysh, 
       "debug igmp events", 
       "Debugging functions (see also 'undebug')\n"
       "IGMP protocol activity\n"
       "IGMP protocol events\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_allocate_host_routes_cmd_vtysh, 
       "no label local allocate host-routes", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure label allocation control\n"
       "allocate local label for host routes only\n")

DEFSH (VTYSH_BGPD, clear_vrf_prefix_rd_cmd_vtysh, 
       "clear vrf NAME [prefix <A.B.C.D/M|X:X::X:X/M>] [rd ASN:nn_or_IP-address]", 
       "Clear stored data\n"
       "From a VRF\n"
       "VRF name\n"
       "Prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Specific VRF Route Distinguisher\n"
       "<as-number>:<number> or <ip-address>:<number>\n")

DEFSH (VTYSH_OSPF6D, ospf6_area_stub_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> stub", 
       "OSPF6 area parameters\n"
       "OSPF6 area ID in IP address format\n"
       "OSPF6 area ID as a decimal value\n"
       "Configure OSPF6 area as stub\n")

DEFSH (VTYSH_PIMD, show_ip_pim_nexthop_lookup_cmd_vtysh, 
       "show ip pim nexthop-lookup A.B.C.D A.B.C.D", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM cached nexthop rpf lookup\n"
       "Source/RP address\n"
       "Multicast Group address\n")

DEFSH (VTYSH_PIMD, debug_pim_packetdump_recv_cmd_vtysh, 
       "debug pim packet-dump receive", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM packet dump\n"
       "Dump received packets\n")

DEFSH (VTYSH_PIMD, ip_pim_keep_alive_cmd_vtysh, 
       "ip pim keep-alive-timer (31-60000)", 
       "IP information\n"
       "pim multicast routing\n"
       "Keep alive Timer\n"
       "Seconds\n")

DEFSH (VTYSH_OSPFD, ospf_area_stub_no_summary_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> stub no-summary", 
       "OSPF stub parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n"
       "Do not inject inter-area routes into stub\n")

DEFSH (VTYSH_RIPNGD, ripng_offset_list_ifname_cmd_vtysh, 
       "offset-list WORD <in|out> (0-16) IFNAME", 
       "Modify RIPng metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n"
       "Interface to match\n")

DEFSH (VTYSH_BGPD, vnc_defaults_rd_cmd_vtysh, 
       "rd ASN:nn_or_IP-address:nn", 
       "Specify default route distinguisher\n"
       "Route Distinguisher (<as-number>:<number> | <ip-address>:<number> | auto:vn:<number> )\n")

DEFSH (VTYSH_BGPD, bgp_network_cmd_vtysh, 
       "network A.B.C.D/M", 
       "Specify a network to announce via BGP\n"
       "IPv4 prefix\n")

DEFSH (VTYSH_ZEBRA, ip_address_cmd_vtysh, 
       "ip address A.B.C.D/M", 
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP address (e.g. 10.0.0.1/8)\n")

DEFSH (VTYSH_LDPD, ldp_no_neighbor_ipv4_ttl_security_hops_hops_cmd_vtysh, 
       "no neighbor A.B.C.D ttl-security hops (1-254)", 
       "Negate a command or set its defaults\n"
       "Configure neighbor parameters\n"
       "LDP Id of neighbor\n"
       "LDP ttl security check\n"
       "IP hops\n"
       "maximum number of hops\n")

DEFSH (VTYSH_PIMD, undebug_msdp_packets_cmd_vtysh,  "undebug msdp packets", 
      "Disable debugging functions (see also 'debug')\n" "MSDP protocol activity\n" "MSDP protocol packets\n")

DEFSH (VTYSH_BGPD, neighbor_interface_v6only_config_remote_as_cmd_vtysh, 
       "neighbor WORD interface v6only remote-as <(1-4294967295)|internal|external>", 
       "Specify neighbor router\n"
       "Interface name or neighbor tag\n"
       "Enable BGP with v6 link-local only\n"
       "Enable BGP on interface\n"
       "Specify a BGP neighbor\n"
       "AS number\n"
       "Internal BGP peer\n"
       "External BGP peer\n")

DEFSH (VTYSH_BGPD, match_probability_cmd_vtysh, 
       "match probability (0-100)", 
       "Match values from routing table\n"
       "Match portion of routes defined by percentage value\n"
       "Percentage of routes\n")

DEFSH (VTYSH_BGPD, add_vnc_prefix_life_cmd_vtysh, 
       "add vnc prefix <A.B.C.D/M|X:X::X:X/M> vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> lifetime (1-4294967295)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n"
       "[local-next-hop (A.B.C.D|X:X::X:X)] [local-cost <0-255>]\n")

DEFSH (VTYSH_OSPFD, show_debugging_ospf_instance_cmd_vtysh, 
       "show debugging ospf (1-65535)", 
       "Show running system information\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n")

DEFSH (VTYSH_OSPFD, ospf_area_vlink_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> virtual-link A.B.C.D [authentication] [<message-digest|null>] [<message-digest-key (1-255) md5 KEY|authentication-key AUTH_KEY>]", 
       "OSPF area parameters\n" "OSPF area ID in IP address format\n" "OSPF area ID as a decimal value\n" "Configure a virtual link\n" "Router ID of the remote ABR\n"
       "Enable authentication on this virtual link\n"
       "Use null authentication\n"
       "Use message-digest authentication\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")

DEFSH (VTYSH_ZEBRA, show_ipv6_forwarding_cmd_vtysh, 
       "show ipv6 forwarding", 
       "Show running system information\n"
       "IPv6 information\n"
       "Forwarding status\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_distance_label_cmd_vtysh, 
       "no ip route A.B.C.D/M <A.B.C.D|INTERFACE|null0> (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_ISISD, show_isis_topology_cmd_vtysh, 
       "show isis topology", 
       "Show running system information\n"
       "IS-IS information\n"
       "IS-IS paths to Intermediate Systems\n")

DEFSH (VTYSH_ZEBRA, show_mpls_status_cmd_vtysh, 
       "show mpls status", 
       "Show running system information\n"
       "MPLS information\n"
       "MPLS status\n")

DEFSH (VTYSH_RIPNGD, no_debug_ripng_packet_cmd_vtysh, 
       "no debug ripng packet", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIPng configuration\n"
       "Debug option set for ripng packet\n")

DEFSH (VTYSH_OSPFD, ospf_area_range_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> range A.B.C.D/M [advertise [cost (0-16777215)]]", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "Area range prefix\n"
       "Advertise this range (default)\n"
       "User specified metric for this range\n"
       "Advertised metric for this range\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_zebra_cmd_vtysh, 
       "no debug ospf zebra [<interface|redistribute>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF Zebra information\n"
       "Zebra interface\n"
       "Zebra redistribute\n")

DEFSH (VTYSH_LDPD, ldp_bridge_ifname_cmd_vtysh, 
       "bridge IFNAME", 
       "Bridge interface\n"
       "Interface's name\n")

DEFSH (VTYSH_BGPD, neighbor_ebgp_multihop_ttl_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> ebgp-multihop (1-255)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Allow EBGP neighbors not on directly connected networks\n"
       "maximum hop count\n")

DEFSH (VTYSH_OSPFD, no_pce_path_scope_cmd_vtysh, 
       "no pce scope [BITPATTERN]", 
       "Negate a command or set its defaults\n"
       "PCE Router Information specific commands\n"
       "Disable PCE path scope\n"
       "32-bit Hexadecimal value\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_cmd_vtysh, 
       "show ipv6 ospf6", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n")

DEFSH (VTYSH_LDPD, ldp_no_discovery_hello_holdtime_disc_time_cmd_vtysh, 
       "no discovery hello holdtime (1-65535)", 
       "Negate a command or set its defaults\n"
       "Configure discovery parameters\n"
       "LDP Link Hellos\n"
       "Hello holdtime\n"
       "Time (seconds) - 65535 implies infinite\n")

DEFSH (VTYSH_RIPD, rip_redistribute_type_metric_routemap_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|ospf|isis|bgp|pim|nhrp|table|vnc>" " metric (0-16) route-map WORD", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n"
       "Metric\n"
       "Metric value\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_route_match_cmd_vtysh, 
       "show ipv6 ospf6 route X:X::X:X/M <match|longer>", 
       "Show running system information\n"
       "IPv6 Information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Routing Table\n"
       "Specify IPv6 prefix\n"
       "Display routes which match the specified route\n"
       "Display routes longer than the specified route\n")

DEFSH (VTYSH_OSPFD, ospf_distance_cmd_vtysh, 
       "distance (1-255)", 
       "Administrative distance\n"
       "OSPF Administrative distance\n")

DEFSH (VTYSH_ISISD, isis_hello_interval_l2_cmd_vtysh, 
       "isis hello-interval (1-600) level-2", 
       "IS-IS commands\n"
       "Set Hello interval\n"
       "Hello interval value\n"
       "Holdtime 1 second,  interval depends on multiplier\n"
       "Specify hello-interval for level-2 IIHs\n")

DEFSH (VTYSH_ISISD, ip_router_isis_cmd_vtysh, 
       "ip router isis WORD", 
       "Interface Internet Protocol config commands\n"
       "IP router interface commands\n"
       "IS-IS Routing for IP\n"
       "Routing process tag\n")

DEFSH (VTYSH_LDPD, ldp_pw_id_pwid_cmd_vtysh, 
       "pw-id (1-4294967295)", 
       "Set the Virtual Circuit ID\n"
       "Virtual Circuit ID value\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, show_ipv6_prefix_list_name_seq_cmd_vtysh, 
       "show ipv6 prefix-list WORD seq (1-4294967295)", 
       "Show running system information\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n")

DEFSH (VTYSH_BGPD, vnc_vrf_policy_rt_both_cmd_vtysh, 
       "rt both RTLIST...", 
       "Specify route targets\n"
       "Export+import filters\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_ZEBRA, no_ip_irdp_shutdown_cmd_vtysh, 
       "no ip irdp shutdown", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "ICMP Router discovery no shutdown on this interface\n")

DEFSH (VTYSH_ZEBRA, no_link_params_inter_as_cmd_vtysh, 
       "no neighbor", 
       "Negate a command or set its defaults\n"
       "Remove Neighbor IP address and AS number for Inter-AS TE\n")

DEFSH (VTYSH_PIMD, show_ip_mroute_count_cmd_vtysh, 
       "show ip mroute count", 
       "Show running system information\n"
       "IP information\n"
       "IP multicast routing table\n"
       "Route and packet count data\n")

DEFSH (VTYSH_PIMD, show_ip_msdp_sa_sg_cmd_vtysh, 
       "show ip msdp sa [A.B.C.D [A.B.C.D]] [json]", 
       "Show running system information\n"
       "IP information\n"
       "MSDP information\n"
       "MSDP active-source information\n"
       "source or group ip\n"
       "group ip\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, set_lcommunity_delete_cmd_vtysh, 
       "set large-comm-list <(1-99)|(100-500)|WORD> delete", 
       "Set values in destination routing protocol\n"
       "set BGP large community list (for deletion)\n"
       "Large Community-list number (standard)\n"
       "Large Communitly-list number (expanded)\n"
       "Large Community-list name\n"
       "Delete matching large communities\n")

DEFSH (VTYSH_OSPFD, no_pce_address_cmd_vtysh, 
       "no pce address [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "PCE Router Information specific commands\n"
       "Disable PCE address\n"
       "PCE address in IPv4 address format\n")

DEFSH (VTYSH_BGPD, vnc_l2_group_rt_cmd_vtysh, 
       "rt <both|export|import> ASN:nn_or_IP-address:nn", 
       "Specify route targets\n"
       "Export+import filters\n"
       "Export filters\n"
       "Import filters\n"
       "A route target\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_description_cmd_vtysh, 
       "ipv6 prefix-list WORD description LINE...", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")

DEFSH (VTYSH_ZEBRA, show_pseudowires_cmd_vtysh, 
       "show mpls pseudowires", 
       "Show running system information\n"
       "MPLS information\n"
       "Pseudowires")

DEFSH (VTYSH_BGPD, show_bgp_ip_vpn_all_rd_cmd_vtysh, 
       "show bgp ""<ipv4|ipv6>"" vpn all [rd ASN:nn_or_IP-address:nn] [json]", 
       "Show running system information\n"
       "IP information\n"
       "BGP information\n"
       "Address Family\n" "Address Family\n"
       "Display VPN NLRI specific information\n"
       "Display information for a route distinguisher\n"
       "VPN Route Distinguisher\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, no_bgp_graceful_restart_stalepath_time_cmd_vtysh, 
       "no bgp graceful-restart stalepath-time [(1-3600)]", 
       "Negate a command or set its defaults\n"
       "BGP specific commands\n"
       "Graceful restart capability parameters\n"
       "Set the max time to hold onto restarting peer's stale paths\n"
       "Delay value (seconds)\n")

DEFSH (VTYSH_BGPD, set_ip_nexthop_peer_cmd_vtysh, 
       "set ip next-hop peer-address", 
       "Set values in destination routing protocol\n"
       "IP information\n"
       "Next hop address\n"
       "Use peer address (for BGP only)\n")

DEFSH (VTYSH_ALL, access_list_standard_cmd_vtysh, 
       "access-list <(1-99)|(1300-1999)> <deny|permit> A.B.C.D A.B.C.D", 
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n"
       "Wildcard bits\n")

DEFSH (VTYSH_BGPD, ip_community_list_standard_cmd_vtysh, 
       "ip community-list <(1-99)|standard WORD> <deny|permit> AA:NN...", 
       "IP information\n"
       "Add a community list entry\n"
       "Community list number (standard)\n"
       "Add an standard community-list entry\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Community number in AA:NN format (where AA and NN are (0-65535)) or local-AS|no-advertise|no-export|internet or additive\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_transmit_delay_cmd_vtysh, 
              "no ospf transmit-delay", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Link state transmit delay\n")

DEFSH (VTYSH_BGPD, no_ip_community_list_standard_all_cmd_vtysh, 
       "no ip community-list <(1-99)|standard WORD> <deny|permit> AA:NN...", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add a community list entry\n"
       "Community list number (standard)\n"
       "Add an standard community-list entry\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Community number in AA:NN format (where AA and NN are (0-65535)) or local-AS|no-advertise|no-export|internet or additive\n")

DEFSH (VTYSH_LDPD, ldp_no_debug_mpls_ldp_discovery_hello_dir_cmd_vtysh, 
       "no debug mpls ldp discovery hello <recv|sent>", 
       "Negate a command or set its defaults\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Discovery messages\n"
       "Discovery hello message\n"
       "Received messages\n"
       "Sent messages\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_l2rd_cmd_vtysh, 
       "l2rd <(1-255)|auto-vn>", 
       "Specify default Local Nve ID value to use in RD for L2 routes\n"
       "Fixed value 1-255\n"
       "use the low-order octet of the NVE's VN address\n")

DEFSH (VTYSH_BGPD, debug_bgp_keepalive_cmd_vtysh, 
       "debug bgp keepalives", 
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "BGP keepalives\n")

DEFSH (VTYSH_ISISD, show_isis_neighbor_arg_cmd_vtysh, 
       "show isis neighbor WORD", 
       "Show running system information\n"
       "ISIS network information\n"
       "ISIS neighbor adjacencies\n"
       "System id\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_cost_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> cost (0-255)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Administrative cost   [default: 255]\n" "Administrative cost\n")

DEFSH (VTYSH_RIPNGD, no_ripng_network_cmd_vtysh, 
       "no network IF_OR_ADDR", 
       "Negate a command or set its defaults\n"
       "RIPng enable on specified interface or network.\n"
       "Interface or address")

DEFSH (VTYSH_OSPFD, debug_ospf_instance_zebra_cmd_vtysh, 
       "debug ospf (1-65535) zebra [<interface|redistribute>]", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF Zebra information\n"
       "Zebra interface\n"
       "Zebra redistribute\n")

DEFSH (VTYSH_ZEBRA, show_debugging_zebra_cmd_vtysh, 
       "show debugging zebra", 
       "Show running system information\n"
       "Debugging information\n"
       "Zebra configuration\n")

DEFSH (VTYSH_ZEBRA, ip_forwarding_cmd_vtysh, 
       "ip forwarding", 
       "IP information\n"
       "Turn on IP forwarding")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_discovery_detail_json_cmd_vtysh, 
       "show mpls ldp discovery detail [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Discovery Hello Information\n"
       "Show detailed information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPD, debug_rip_events_cmd_vtysh, 
       "debug rip events", 
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP events\n")

DEFSH (VTYSH_ZEBRA, ipv6_nd_router_preference_cmd_vtysh, 
       "ipv6 nd router-preference <high|medium|low>", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Default router preference\n"
       "High default router preference\n"
       "Low default router preference\n"
       "Medium default router preference (default)\n")

DEFSH (VTYSH_BGPD, clear_vnc_mac_vn_cmd_vtysh, 
       "clear vnc mac <*|YY:YY:YY:YY:YY:YY> virtual-network-identifier <*|(1-4294967295)> vn <*|A.B.C.D|X:X::X:X>", 
       "clear\n"
       "VNC Information\n"
       "Clear mac registration information\n"
       "All macs\n"
       "MAC address\n"
       "VNI keyword\n"
       "Any virtual network identifier\n"
       "Virtual network identifier\n"
       "UN address of NVE\n"
       "All VN addresses\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n")

DEFSH (VTYSH_PIMD, no_ip_pim_ssm_prefix_list_cmd_vtysh, 
       "no ip pim ssm prefix-list", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "pim multicast routing\n"
       "Source Specific Multicast\n"
       "group range prefix-list filter\n")

DEFSH (VTYSH_BGPD, neighbor_distribute_list_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> distribute-list <(1-199)|(1300-2699)|WORD> <in|out>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Filter updates to/from this neighbor\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

DEFSH (VTYSH_ALL, exec_timeout_min_cmd_vtysh, 
       "exec-timeout (0-35791)", 
       "Set timeout value\n"
       "Timeout value in minutes\n")

DEFSH (VTYSH_BGPD, vnc_defaults_rt_both_cmd_vtysh, 
       "rt both RTLIST...", 
       "Configure default route targets\n"
       "Export+import filters\n"
       "Space separated route target list (A.B.C.D:MN|EF:OPQR|GHJK:MN)\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_seq_ge_le_cmd_vtysh, 
       "ipv6 prefix-list WORD seq (1-4294967295) <deny|permit> X:X::X:X/M ge (0-128) le (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_mask_distance_label_cmd_vtysh, 
       "no ip route A.B.C.D A.B.C.D <A.B.C.D|INTERFACE|null0> (1-255) label WORD", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPFD, ospf_timers_min_ls_interval_cmd_vtysh, 
       "timers throttle lsa all (0-5000)", 
       "Adjust routing timers\n"
       "Throttling adaptive timer\n"
       "LSA delay between transmissions\n"
       "All LSA types\n"
       "Delay (msec) between sending LSAs\n")

DEFSH (VTYSH_BGPD, bgp_cluster_id_cmd_vtysh, 
       "bgp cluster-id <A.B.C.D|(1-4294967295)>", 
       "BGP information\n"
       "Configure Route-Reflector Cluster-id\n"
       "Route-Reflector Cluster-id in IP address format\n"
       "Route-Reflector Cluster-id as 32 bit quantity\n")

DEFSH (VTYSH_OSPF6D, show_ipv6_ospf6_database_type_router_cmd_vtysh, 
       "show ipv6 ospf6 database <router|network|inter-prefix|inter-router|as-external|group-membership|type-7|link|intra-prefix> <*|adv-router> A.B.C.D [<detail|dump|internal>]", 
       "Show running system information\n"
       "IPv6 information\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Display Link state database\n"
       "Display Router LSAs\n"
       "Display Network LSAs\n"
       "Display Inter-Area-Prefix LSAs\n"
       "Display Inter-Area-Router LSAs\n"
       "Display As-External LSAs\n"
       "Display Group-Membership LSAs\n"
       "Display Type-7 LSAs\n"
       "Display Link LSAs\n"
       "Display Intra-Area-Prefix LSAs\n"
       "Any Link state ID\n"
       "Search by Advertising Router\n"
       "Specify Advertising Router as IPv4 address notation\n"
       "Display details of LSAs\n"
       "Dump LSAs\n"
       "Display LSA's internal information\n"
      )

DEFSH (VTYSH_ZEBRA, ipv6_nd_adv_interval_config_option_cmd_vtysh, 
       "ipv6 nd adv-interval-option", 
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Advertisement Interval Option\n")

DEFSH (VTYSH_PIMD, show_ip_pim_assert_winner_metric_cmd_vtysh, 
       "show ip pim assert-winner-metric", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface assert winner metric\n")

DEFSH (VTYSH_NHRPD, if_nhrp_network_id_cmd_vtysh, 
 "<ip|ipv6>" " nhrp network-id (1-4294967295)", 
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Enable NHRP and specify network-id\n"
 "System local ID to specify interface group\n")

DEFSH (VTYSH_ZEBRA, allow_external_route_update_cmd_vtysh, 
       "allow-external-route-update", 
       "Allow FRR routes to be overwritten by external processes\n")

DEFSH (VTYSH_LDPD, ldp_dual_stack_transport_connection_prefer_ipv4_cmd_vtysh, 
       "dual-stack transport-connection prefer ipv4", 
       "Configure dual stack parameters\n"
       "Configure TCP transport parameters\n"
       "Configure prefered address family for TCP transport connection with neighbor\n"
       "IPv4\n")

DEFSH (VTYSH_ZEBRA, show_zebra_client_cmd_vtysh, 
       "show zebra client", 
       "Show running system information\n"
       "Zebra information\n"
       "Client information\n")

DEFSH (VTYSH_OSPFD, no_debug_ospf_instance_nssa_cmd_vtysh, 
       "no debug ospf (1-65535) nssa", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "Instance ID\n"
       "OSPF nssa information\n")

DEFSH (VTYSH_LDPD, ldp_show_l2vpn_atom_binding_json_cmd_vtysh, 
       "show l2vpn atom binding [json]", 
       "Show running system information\n"
       "Show information about Layer2 VPN\n"
       "Show Any Transport over MPLS information\n"
       "Show AToM label binding information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_ospf_instance_cmd_vtysh, 
       "show ip route ospf (1-65535)", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Open Shortest Path First (OSPFv2)\n"
       "Instance ID\n")

DEFSH (VTYSH_BGPD, clear_ip_bgp_prefix_cmd_vtysh, 
       "clear [ip] bgp [<view|vrf> WORD] prefix A.B.C.D/M", 
       "Reset functions\n"
       "IP information\n"
       "BGP information\n"
       "BGP view\nBGP VRF\nView/VRF name\n"
       "Clear bestpath and re-advertise\n"
       "IPv4 prefix\n")

DEFSH (VTYSH_BGPD, no_neighbor_unsuppress_map_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> unsuppress-map WORD", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Route-map to selectively unsuppress suppressed routes\n"
       "Name of route map\n")

DEFSH (VTYSH_BGPD, no_bgp_redistribute_ipv6_cmd_vtysh, 
       "no redistribute " "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct>" " [metric (0-4294967295)] [route-map WORD]", 
       "Negate a command or set its defaults\n"
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n"
       "Metric for redistributed routes\n"
       "Default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_summary_prefix_cmd_vtysh, 
       "show ip route [vrf NAME] summary prefix", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Summary of all routes\n"
       "Prefix routes\n")

DEFSH (VTYSH_BGPD, no_set_originator_id_cmd_vtysh, 
       "no set originator-id [A.B.C.D]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "BGP originator ID attribute\n"
       "IP address of originator\n")

DEFSH (VTYSH_ZEBRA, ipv6_forwarding_cmd_vtysh, 
       "ipv6 forwarding", 
       "IPv6 information\n"
       "Turn on IPv6 forwarding")

DEFSH (VTYSH_RIPD, accept_lifetime_duration_month_day_cmd_vtysh, 
       "accept-lifetime HH:MM:SS MONTH (1-31) (1993-2035) duration (1-2147483646)", 
       "Set accept lifetime of the key\n"
       "Time to start\n"
       "Month of the year to start\n"
       "Day of th month to start\n"
       "Year to start\n"
       "Duration of the key\n"
       "Duration seconds\n")

DEFSH (VTYSH_LDPD, ldp_label_local_allocate_for_acl_cmd_vtysh, 
       "label local allocate for <(1-199)|(1300-2699)|WORD>", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure label allocation control\n"
       "IP access-list\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP access-list name\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_route_map_all_cmd_vtysh, 
       "no route-map WORD", 
       "Negate a command or set its defaults\n"
       "Create route-map or enter route-map command mode\n"
       "Route map tag\n")

DEFSH (VTYSH_BGPD, neighbor_capability_dynamic_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> capability dynamic", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Advertise capability to the peer\n"
       "Advertise dynamic capability to this neighbor\n")

DEFSH (VTYSH_BGPD, bgp_set_route_map_delay_timer_cmd_vtysh, 
       "bgp route-map delay-timer (0-600)", 
       "Set values in destination routing protocol\n"
       "BGP route-map delay timer\n"
       "Time in secs to wait before processing route-map changes\n"
       "0 disables the timer,  no route updates happen when route-maps change\n")

DEFSH (VTYSH_PIMD, interface_no_ip_igmp_query_max_response_time_cmd_vtysh, 
       "no ip igmp query-max-response-time (10-250)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP max query response value (seconds)\n"
       "Time for response in deci-seconds\n")

DEFSH (VTYSH_OSPF6D, ospf6_routemap_no_set_forwarding_cmd_vtysh, 
       "no set forwarding-address X:X::X:X", 
       "Negate a command or set its defaults\n"
       "Set value\n"
       "Forwarding Address\n"
       "IPv6 Address\n")

DEFSH (VTYSH_BGPD, vnc_nve_export_no_prefixlist_cmd_vtysh, 
       "no vnc export <bgp|zebra> <ipv4|ipv6> prefix-list [NAME]", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Export to other protocols\n"
       "Export to BGP\n"
       "Export to Zebra (experimental)\n"
       "IPv4 prefixes\n"
       "IPv6 prefixes\n"
       "Prefix-list for filtering exported routes\n" "Prefix list name\n")

DEFSH (VTYSH_ISISD, is_type_cmd_vtysh, 
       "is-type <level-1|level-1-2|level-2-only>", 
       "IS Level for this routing process (OSI only)\n"
       "Act as a station router only\n"
       "Act as both a station router and an area router\n"
       "Act as an area router only\n")

DEFSH (VTYSH_PIMD, show_ip_msdp_sa_detail_cmd_vtysh, 
       "show ip msdp sa detail [json]", 
       "Show running system information\n"
       "IP information\n"
       "MSDP information\n"
       "MSDP active-source information\n"
       "Detailed output\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ISISD, no_isis_metric_l2_cmd_vtysh, 
       "no isis metric [(0-16777215)] level-2", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Set default metric for circuit\n"
       "Default metric value\n"
       "Specify metric for level-2 routing\n")

DEFSH (VTYSH_OSPFD, ospf_area_nssa_translate_no_summary_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> nssa <translate-candidate|translate-never|translate-always> no-summary", 
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "Configure NSSA-ABR for translate election (default)\n"
       "Configure NSSA-ABR to never translate\n"
       "Configure NSSA-ABR to always translate\n"
       "Do not inject inter-area routes into nssa\n")

DEFSH (VTYSH_BGPD, match_ipv6_next_hop_cmd_vtysh, 
       "match ipv6 next-hop X:X::X:X", 
       "Match values from routing table\n"
       "IPv6 information\n"
       "Match IPv6 next-hop address of route\n"
       "IPv6 address of next hop\n")

DEFSH (VTYSH_OSPFD, no_ospf_max_metric_router_lsa_admin_cmd_vtysh, 
       "no max-metric router-lsa administrative", 
       "Negate a command or set its defaults\n"
       "OSPF maximum / infinite-distance metric\n"
       "Advertise own Router-LSA with infinite distance (stub router)\n"
       "Administratively applied,  for an indefinite period\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_kernel_msgdump_cmd_vtysh, 
       "debug zebra kernel msgdump [<recv|send>]", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra between kernel interface\n"
       "Dump raw netlink messages,  sent and received\n"
       "Dump raw netlink messages received\n"
       "Dump raw netlink messages sent\n")

DEFSH (VTYSH_ZEBRA, show_ip_route_supernets_cmd_vtysh, 
       "show ip route [vrf NAME] supernets-only", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Show supernet entries only\n")

DEFSH (VTYSH_BGPD, neighbor_strict_capability_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X> strict-capability-match", 
       "Specify neighbor router\n"
       "Neighbor address\nIPv6 address\n"
       "Strict capability negotiation match\n")

DEFSH (VTYSH_BGPD, bgp_config_type_cmd_vtysh, 
       "bgp config-type <cisco|zebra>", 
       "BGP information\n"
       "Configuration type\n"
       "cisco\n"
       "zebra\n")

DEFSH (VTYSH_OSPF6D, ospf6_log_adjacency_changes_detail_cmd_vtysh, 
       "log-adjacency-changes detail", 
       "Log changes in adjacency state\n"
       "Log all state changes\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_l2vpn_evpn_all_neighbor_advertised_routes_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn all neighbors A.B.C.D advertised-routes [json]", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information about all EVPN NLRIs\n"
      "Detailed information on TCP and BGP neighbor connections\n"
      "Neighbor to display information about\n"
      "Display the routes advertised to a BGP neighbor\n" "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_nd_suppress_ra_cmd_vtysh, 
       "no ipv6 nd suppress-ra", 
       "Negate a command or set its defaults\n"
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Suppress Router Advertisement\n")

DEFSH (VTYSH_BGPD, bgp_router_id_cmd_vtysh, 
       "bgp router-id A.B.C.D", 
       "BGP information\n"
       "Override configured router identifier\n"
       "Manually configured router identifier\n")

DEFSH (VTYSH_NHRPD, if_no_nhrp_nhs_cmd_vtysh, 
 "no " "<ip|ipv6>" " nhrp nhs <A.B.C.D|X:X::X:X|dynamic> nbma <A.B.C.D|FQDN>", 
 "Negate a command or set its defaults\n"
 "IP information\n" "IPv6 information\n"
 "Next Hop Resolution Protocol functions\n"
 "Nexthop Server configuration\n"
 "IPv4 protocol address\n"
 "IPv6 protocol address\n"
 "Automatic detection of protocol address\n"
 "NBMA address\n"
 "IPv4 NBMA address\n"
 "Fully qualified domain name for NBMA address(es)\n")

DEFSH (VTYSH_ALL, no_ipv6_access_list_any_cmd_vtysh, 
       "no ipv6 access-list WORD <deny|permit> any", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any prefixi to match\n")

DEFSH (VTYSH_OSPFD, no_router_ospf_cmd_vtysh, 
       "no router ospf [(1-65535)]", 
       "Negate a command or set its defaults\n"
       "Enable a routing process\n"
       "Start OSPF configuration\n"
       "Instance ID\n")

DEFSH (VTYSH_BGPD, no_evpnrt5_network_cmd_vtysh, 
      "no network <A.B.C.D/M|X:X::X:X/M> rd ASN:nn_or_IP-address:nn ethtag WORD label WORD esi WORD gwip <A.B.C.D|X:X::X:X>", 
      "Negate a command or set its defaults\n"
      "Specify a network to announce via BGP\n"
      "IP prefix\n"
      "IPv6 prefix\n"
      "Specify Route Distinguisher\n"
      "VPN Route Distinguisher\n"
      "Ethernet Tag\n"
      "Ethernet Tag Value\n"
      "BGP label\n"
      "label value\n"
      "Ethernet Segment Identifier\n"
      "ESI value ( 00:11:22:33:44:55:66:77:88:99 format) \n"
      "Gateway IP\n" "Gateway IP ( A.B.C.D )\n" "Gateway IPv6 ( X:X::X:X )\n")

DEFSH (VTYSH_PIMD, interface_no_ip_igmp_version_cmd_vtysh, 
       "no ip igmp version (2-3)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Enable IGMP operation\n"
       "IGMP version\n"
       "IGMP version number\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, set_ip_nexthop_cmd_vtysh, 
       "set ip next-hop A.B.C.D", 
       "Set values in destination routing protocol\n"
       "IP information\n"
       "Next hop address\n"
       "IP address of next hop\n")

DEFSH (VTYSH_OSPF6D, ospf6_area_stub_no_summary_cmd_vtysh, 
       "area <A.B.C.D|(0-4294967295)> stub no-summary", 
       "OSPF6 stub parameters\n"
       "OSPF6 area ID in IP address format\n"
       "OSPF6 area ID as a decimal value\n"
       "Configure OSPF6 area as stub\n"
       "Do not inject inter-area routes into stub\n")

DEFSH (VTYSH_PIMD, show_ip_pim_assert_internal_cmd_vtysh, 
       "show ip pim assert-internal", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM interface internal assert state\n")

DEFSH (VTYSH_BGPD, no_bgp_maxpaths_cmd_vtysh, 
       "no maximum-paths [(1-255)]", 
       "Negate a command or set its defaults\n"
       "Forward packets over multiple paths\n"
       "Number of paths\n")

DEFSH (VTYSH_ZEBRA, no_ip_route_mask_flags_cmd_vtysh, 
       "no ip route A.B.C.D A.B.C.D <reject|blackhole> [tag (1-4294967295)] [(1-255)] [vrf NAME]", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Tag of this route\n"
       "Tag value\n"
       "Distance value for this route\n"
       "Specify the VRF\nThe VRF name\n"
       "Specify labels for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_OSPFD, show_ip_ospf_instance_interface_cmd_vtysh, 
       "show ip ospf (1-65535) interface [INTERFACE] [json]", 
       "Show running system information\n"
       "IP information\n"
       "OSPF information\n"
       "Instance ID\n"
       "Interface information\n"
       "Interface name\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_BGPD, vnc_redist_bgpdirect_no_routemap_cmd_vtysh, 
       "no vnc redistribute <bgp-direct|bgp-direct-to-nve-groups> route-map", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Redistribute from other protocols\n"
       "Redistribute from BGP directly\n"
       "Redistribute from BGP without Zebra,  only to configured NVE groups\n"
       "Route-map for filtering redistributed routes\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_nht_cmd_vtysh, 
       "show ipv6 nht [vrf NAME]", 
       "Show running system information\n"
       "IPv6 information\n"
       "IPv6 nexthop tracking table\n"
       "Specify the VRF\nThe VRF name\n")

DEFSH (VTYSH_ISISD, no_log_adj_changes_cmd_vtysh, 
       "no log-adjacency-changes", 
       "Negate a command or set its defaults\n"
       "Stop logging changes in adjacency state\n")

DEFSH (VTYSH_RIPD, debug_rip_zebra_cmd_vtysh, 
       "debug rip zebra", 
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP and ZEBRA communication\n")

DEFSH (VTYSH_BGPD, no_neighbor_peer_group_cmd_vtysh, 
       "no neighbor WORD peer-group", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor tag\n"
       "Configure peer-group\n")

DEFSH (VTYSH_ISISD, csnp_interval_cmd_vtysh, 
       "isis csnp-interval (1-600)", 
       "IS-IS commands\n"
       "Set CSNP interval in seconds\n"
       "CSNP interval value\n")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, ipv6_prefix_list_le_ge_cmd_vtysh, 
       "ipv6 prefix-list WORD <deny|permit> X:X::X:X/M le (0-128) ge (0-128)", 
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>,  e.g.,  3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")

DEFSH (VTYSH_ALL, no_access_list_remark_comment_cmd_vtysh, 
       "no access-list <(1-99)|(100-199)|(1300-1999)|(2000-2699)|WORD> remark LINE...", 
       "Negate a command or set its defaults\n"
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")

DEFSH (VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, no_vrf_debug_cmd_vtysh, 
      "no debug vrf", 
      "Negate a command or set its defaults\n"
      "Debugging functions (see also 'undebug')\n"
      "VRF Debugging\n")

DEFSH (VTYSH_BGPD, bgp_redistribute_ipv4_cmd_vtysh, 
       "redistribute " "<kernel|connected|static|rip|ospf|isis|pim|nhrp|table|vnc|vnc-direct>", 
       "Redistribute information from another routing protocol\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol (RIP)\n" "Open Shortest Path First (OSPFv2)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Protocol Independent Multicast (PIM)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n" "VNC direct (not via zebra) routes\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_binding_detail_json_cmd_vtysh, 
       "show mpls ldp binding detail [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "Label Information Base (LIB) information\n"
       "Show detailed information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_LDPD, ldp_no_label_local_advertise_cmd_vtysh, 
       "no label local advertise", 
       "Negate a command or set its defaults\n"
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n")

DEFSH (VTYSH_ZEBRA, no_ipv6_forwarding_cmd_vtysh, 
       "no ipv6 forwarding", 
       "Negate a command or set its defaults\n"
       "IPv6 information\n"
       "Turn off IPv6 forwarding")

DEFSH (VTYSH_RIPNGD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ZEBRA, clear_ipv6_prefix_list_name_cmd_vtysh, 
       "clear ipv6 prefix-list WORD", 
       "Reset functions\n"
       "IPv6 information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n")

DEFSH (VTYSH_ZEBRA, debug_zebra_events_cmd_vtysh, 
       "debug zebra events", 
       "Debugging functions (see also 'undebug')\n"
       "Zebra configuration\n"
       "Debug option set for zebra events\n")

DEFSH (VTYSH_OSPFD, no_ospf_area_range_cmd_vtysh, 
       "no area <A.B.C.D|(0-4294967295)> range A.B.C.D/M [<cost (0-16777215)|advertise [cost (0-16777215)]|not-advertise>]", 
       "Negate a command or set its defaults\n"
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "Area range prefix\n"
       "User specified metric for this range\n"
       "Advertised metric for this range\n"
       "Advertise this range (default)\n"
       "User specified metric for this range\n"
       "Advertised metric for this range\n"
       "DoNotAdvertise this range\n")

DEFSH (VTYSH_ZEBRA, ip_irdp_holdtime_cmd_vtysh, 
       "ip irdp holdtime (0-9000)", 
       "IP information\n"
       "ICMP Router discovery on this interface\n"
       "Set holdtime value\n"
       "Holdtime value in seconds. Default is 1800 seconds\n")

DEFSH (VTYSH_PIMD, ip_msdp_mesh_group_member_cmd_vtysh, 
       "ip msdp mesh-group WORD member A.B.C.D", 
       "IP information\n"
       "Configure multicast source discovery protocol\n"
       "Configure MSDP mesh-group\n"
       "mesh group name\n"
       "mesh group member\n"
       "peer ip address\n")

DEFSH (VTYSH_BGPD, show_ip_bgp_evpn_rd_overlay_cmd_vtysh, 
      "show [ip] bgp l2vpn evpn rd ASN:nn_or_IP-address:nn overlay", 
      "Show running system information\n"
      "IP information\n"
      "BGP information\n"
      "Layer 2 Virtual Private Network\n"
      "Ethernet Virtual Private Network\n"
      "Display information for a route distinguisher\n"
      "VPN Route Distinguisher\n"
      "Display BGP Overlay Information for prefixes\n")

DEFSH (VTYSH_BGPD, ip_extcommunity_list_standard_cmd_vtysh, 
       "ip extcommunity-list <(1-99)|standard WORD> <deny|permit> AA:NN...", 
       "IP information\n"
       "Add a extended community list entry\n"
       "Extended Community list number (standard)\n"
       "Specify standard extcommunity-list\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Extended community attribute in 'rt aa:nn_or_IPaddr:nn' OR 'soo aa:nn_or_IPaddr:nn' format\n")

DEFSH (VTYSH_BGPD, neighbor_capability_enhe_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> capability extended-nexthop", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Advertise capability to the peer\n"
       "Advertise extended next-hop capability to the peer\n")

DEFSH (VTYSH_BGPD, no_set_lcommunity_delete_cmd_vtysh, 
       "no set large-comm-list <(1-99)|(100-500)|WORD> [delete]", 
       "Negate a command or set its defaults\n"
       "Set values in destination routing protocol\n"
       "set BGP large community list (for deletion)\n"
       "Large Community-list number (standard)\n"
       "Large Communitly-list number (expanded)\n"
       "Large Community-list name\n"
       "Delete matching large communities\n")

DEFSH_HIDDEN (VTYSH_PIMD, interface_ip_pim_ssm_cmd_vtysh, 
       "ip pim ssm", 
       "IP information\n"
       "PIM information\n"
       "Enable PIM SSM operation\n")

DEFSH (VTYSH_LDPD, ldp_show_debugging_mpls_ldp_cmd_vtysh, 
       "show debugging mpls ldp", 
       "Show running system information\n"
       "Debugging functions\n"
       "MPLS information\n"
       "Label Distribution Protocol\n")

DEFSH (VTYSH_BGPD, no_debug_bgp_neighbor_events_peer_cmd_vtysh, 
       "no debug bgp neighbor-events <A.B.C.D|X:X::X:X|WORD>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "BGP information\n"
       "Neighbor Events\n"
       "BGP neighbor IP address to debug\n"
       "BGP IPv6 neighbor to debug\n"
       "BGP neighbor on interface to debug\n")

DEFSH (VTYSH_BGPD, match_peer_cmd_vtysh, 
       "match peer <A.B.C.D|X:X::X:X>", 
       "Match values from routing table\n"
       "Match peer address\n"
       "IP address of peer\n"
       "IPv6 address of peer\n")

DEFSH (VTYSH_RIPNGD, no_ripng_route_cmd_vtysh, 
       "no route IPV6ADDR", 
       "Negate a command or set its defaults\n"
       "Static route setup\n"
       "Delete static RIPng route announcement\n")

DEFSH (VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD|VTYSH_ZEBRA|VTYSH_PIMD, no_ip_prefix_list_le_cmd_vtysh, 
       "no ip prefix-list WORD <deny|permit> A.B.C.D/M le (0-32)", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Build a prefix list\n"
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>,  e.g.,  35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

DEFSH (VTYSH_BGPD, add_vnc_mac_vni_prefix_life_cmd_vtysh, 
       "add vnc mac YY:YY:YY:YY:YY:YY virtual-network-identifier (1-4294967295) vn <A.B.C.D|X:X::X:X> un <A.B.C.D|X:X::X:X> prefix <A.B.C.D/M|X:X::X:X/M> lifetime (1-4294967295)", 
       "Add registration\n"
       "VNC Information\n"
       "Add/modify mac address information\n"
       "MAC address\n"
       "Virtual Network Identifier follows\n"
       "Virtual Network Identifier\n"
       "VN address of NVE\n"
       "VN IPv4 interface address\n"
       "VN IPv6 interface address\n"
       "UN address of NVE\n"
       "UN IPv4 interface address\n"
       "UN IPv6 interface address\n"
       "Add/modify prefix related information\n"
       "IPv4 prefix\n"
       "IPv6 prefix\n"
       "Registration lifetime [default: infinite]\n"
       "Lifetime value in seconds\n")

DEFSH (VTYSH_BGPD, vnc_redistribute_no_nvegroup_cmd_vtysh, 
       "no vnc redistribute nve-group", 
       "Negate a command or set its defaults\n"
       "VNC/RFP related configuration\n"
       "Redistribute from other protocol\n"
       "Assign a NVE group to routes redistributed from another routing protocol\n")

DEFSH (VTYSH_BGPD, no_neighbor_advertise_interval_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> advertisement-interval [(0-600)]", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Minimum interval between sending BGP routing updates\n"
       "time in seconds\n")

DEFSH (VTYSH_BGPD, neighbor_local_as_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> local-as (1-4294967295)", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Specify a local-as number\n"
       "AS number used as local AS\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_route_cmd_vtysh, 
       "no debug ospf6 route <table|intra-area|inter-area|memory>", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug routes\n"
       "Debug route table calculation\n"
       "Debug intra-area route calculation\n"
       "Debug inter-area route calculation\n"
       "Debug route memory use\n")

DEFSH (VTYSH_PIMD, debug_pim_trace_cmd_vtysh, 
       "debug pim trace", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "PIM internal daemon activity\n")

DEFSH (VTYSH_ALL, access_list_extended_host_any_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip host A.B.C.D any", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Any destination host\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_prefix_longer_cmd_vtysh, 
       "show ipv6 route [vrf NAME] X:X::X:X/M longer-prefixes", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "IPv6 prefix\n"
       "Show route matching the specified Network/Mask pair only\n")

DEFSH (VTYSH_ISISD, no_isis_hello_padding_cmd_vtysh, 
       "no isis hello padding", 
       "Negate a command or set its defaults\n"
       "IS-IS commands\n"
       "Add padding to IS-IS hello packets\n"
       "Pad hello packets\n"
       "<cr>\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_vrf_all_protocol_cmd_vtysh, 
       "show ipv6 route vrf all " "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc>", 
       "Show running system information\n"
       "IP information\n"
       "IP routing table\n"
       "Specify the VRF\nAll VRFs\n"
       "Kernel routes (not installed via the zebra RIB)\n" "Connected routes (directly attached subnet or host)\n" "Statically configured routes\n" "Routing Information Protocol next-generation (IPv6) (RIPng)\n" "Open Shortest Path First (IPv6) (OSPFv3)\n" "Intermediate System to Intermediate System (IS-IS)\n" "Border Gateway Protocol (BGP)\n" "Next Hop Resolution Protocol (NHRP)\n" "Non-main Kernel Routing Table\n" "Virtual Network Control (VNC)\n")

DEFSH (VTYSH_BGPD, no_neighbor_interface_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> interface WORD", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Interface\n"
       "Interface name\n")

DEFSH (VTYSH_RIPD, ip_rip_send_version_2_cmd_vtysh, 
       "ip rip send version (2-2) (1-1)", 
       "IP information\n"
       "Routing Information Protocol\n"
       "Advertisement transmission\n"
       "Version control\n"
       "RIP version 2\n"
       "RIP version 1\n")

DEFSH (VTYSH_LDPD, ldp_show_mpls_ldp_address_family_discovery_detail_json_cmd_vtysh, 
       "show mpls ldp <ipv4|ipv6> discovery detail [json]", 
       "Show running system information\n"
       "MPLS information\n"
       "Label Distribution Protocol\n"
       "IPv4 Address Family\n"
       "IPv6 Address Family\n"
       "Discovery Hello Information\n"
       "Show detailed information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_RIPNGD, show_ipv6_ripng_cmd_vtysh, 
       "show ipv6 ripng", 
       "Show running system information\n"
       "IPv6 information\n"
       "Show RIPng routes\n")

DEFSH (VTYSH_BGPD, neighbor_route_map_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> route-map WORD <in|out>", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")

DEFSH (VTYSH_BGPD, vnc_nve_group_redist_bgpdirect_routemap_cmd_vtysh, 
       "redistribute bgp-direct route-map NAME", 
       "Redistribute from other protocols\n"
       "Redistribute from BGP directly\n"
       "Route-map for filtering exported routes\n" "route map name\n")

DEFSH (VTYSH_PIMD, show_ip_pim_rpf_cmd_vtysh, 
       "show ip pim rpf [json]", 
       "Show running system information\n"
       "IP information\n"
       "PIM information\n"
       "PIM cached source rpf information\n"
       "JavaScript Object Notation\n")

DEFSH (VTYSH_ZEBRA, show_ipv6_route_tag_cmd_vtysh, 
       "show ipv6 route [vrf NAME] tag (1-4294967295)", 
       "Show running system information\n"
       "IP information\n"
       "IPv6 routing table\n"
       "Specify the VRF\nThe VRF name\n"
       "Show only routes with tag\n"
       "Tag value\n")

DEFSH (VTYSH_ZEBRA, no_multicast_cmd_vtysh, 
       "no multicast", 
       "Negate a command or set its defaults\n"
       "Unset multicast flag to interface\n")

DEFSH (VTYSH_LDPD, ldp_label_local_advertise_explicit_null_cmd_vtysh, 
       "label local advertise explicit-null", 
       "Configure label control and policies\n"
       "Configure local label control and policies\n"
       "Configure outbound label advertisement control\n"
       "Configure explicit-null advertisement\n")

DEFSH (VTYSH_BGPD, ip_community_list_expanded_all_cmd_vtysh, 
       "ip community-list <(100-500)|expanded WORD> <deny|permit> AA:NN...", 
       "IP information\n"
       "Add a community list entry\n"
       "Community list number (expanded)\n"
       "Add an expanded community-list entry\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Community number in AA:NN format (where AA and NN are (0-65535)) or local-AS|no-advertise|no-export|internet or additive\n")

DEFSH (VTYSH_OSPFD, ip_ospf_cost_cmd_vtysh, 
       "ip ospf cost (1-65535) [A.B.C.D]", 
       "IP Information\n"
       "OSPF interface commands\n"
       "Interface cost\n"
       "Cost\n"
       "Address of interface\n")

DEFSH (VTYSH_ZEBRA, ipv6_route_tag_label_cmd_vtysh, 
       "ipv6 route X:X::X:X/M <X:X::X:X|INTERFACE> tag (1-4294967295) label WORD", 
       "IP information\n"
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Set tag for this route\n"
       "Tag value\n"
       "Specify label(s) for this route\n"
       "One or more labels separated by '/'\n")

DEFSH (VTYSH_RIPD, no_debug_rip_zebra_cmd_vtysh, 
       "no debug rip zebra", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "RIP information\n"
       "RIP and ZEBRA communication\n")

DEFSH (VTYSH_PIMD, interface_no_ip_mroute_source_cmd_vtysh, 
       "no ip mroute INTERFACE A.B.C.D A.B.C.D", 
       "Negate a command or set its defaults\n"
       "IP information\n"
       "Add multicast route\n"
       "Outgoing interface name\n"
       "Group Address\n"
       "Source Address\n")

DEFSH (VTYSH_PIMD, debug_pim_zebra_cmd_vtysh, 
       "debug pim zebra", 
       "Debugging functions (see also 'undebug')\n"
       "PIM protocol activity\n"
       "ZEBRA protocol activity\n")

DEFSH (VTYSH_BGPD, no_neighbor_distribute_list_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> distribute-list <(1-199)|(1300-2699)|WORD> <in|out>", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Filter updates to/from this neighbor\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

DEFSH (VTYSH_BGPD, neighbor_activate_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> activate", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Enable the Address Family for this Neighbor\n")

DEFSH (VTYSH_ISISD, spf_interval_cmd_vtysh, 
       "spf-interval (1-120)", 
       "Minimum interval between SPF calculations\n"
       "Minimum interval between consecutive SPFs in seconds\n")

DEFSH (VTYSH_ISISD, no_isis_mpls_te_inter_as_cmd_vtysh, 
       "no mpls-te inter-as", 
       "Negate a command or set its defaults\n"
       "Disable the MPLS-TE functionality\n"
       "Disable MPLS-TE Inter-AS support\n")

DEFSH (VTYSH_ALL, access_list_extended_any_any_cmd_vtysh, 
       "access-list <(100-199)|(2000-2699)> <deny|permit> ip any any", 
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Any destination host\n")

DEFSH (VTYSH_ZEBRA, shutdown_if_cmd_vtysh, 
       "shutdown", 
       "Shutdown the selected interface\n")

DEFSH (VTYSH_BGPD, no_neighbor_capability_enhe_cmd_vtysh, 
       "no neighbor <A.B.C.D|X:X::X:X|WORD> capability extended-nexthop", 
       "Negate a command or set its defaults\n"
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Advertise capability to the peer\n"
       "Advertise extended next-hop capability to the peer\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_hello_interval_cmd_vtysh, 
              "no ospf hello-interval [(1-65535) [A.B.C.D]]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Time between HELLO packets\n"
              "Seconds\n"
              "Address of interface\n")

DEFSH_HIDDEN (VTYSH_OSPFD, ospf_dead_interval_cmd_vtysh, 
              "ospf dead-interval (1-65535) [A.B.C.D]", 
              "OSPF interface commands\n"
              "Interval time after which a neighbor is declared down\n"
              "Seconds\n"
              "Address of interface\n")

DEFSH (VTYSH_RIPD, no_key_cmd_vtysh, 
       "no key (0-2147483647)", 
       "Negate a command or set its defaults\n"
       "Delete a key\n"
       "Key identifier number\n")

DEFSH (VTYSH_OSPFD, no_ip_ospf_bfd_cmd_vtysh, 
       "no ip ospf bfd [(2-255) (50-60000) (50-60000)]", 
       "Negate a command or set its defaults\n"
       "IP Information\n"
       "OSPF interface commands\n"
       "Disables BFD support\n"
       "Detect Multiplier\n"
       "Required min receive interval\n"
       "Desired min transmit interval\n")

DEFSH (VTYSH_OSPF6D, no_debug_ospf6_neighbor_cmd_vtysh, 
       "no debug ospf6 neighbor [<state|event>]", 
       "Negate a command or set its defaults\n"
       "Debugging functions (see also 'undebug')\n"
       "Open Shortest Path First (OSPF) for IPv6\n"
       "Debug OSPFv3 Neighbor\n"
       "Debug OSPFv3 Neighbor State Change\n"
       "Debug OSPFv3 Neighbor Event\n")

DEFSH (VTYSH_BGPD, bgp_bestpath_aspath_ignore_cmd_vtysh, 
       "bgp bestpath as-path ignore", 
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Ignore as-path length in selecting a route\n")

DEFSH (VTYSH_OSPFD, no_ospf_timers_lsa_cmd_vtysh, 
       "no timers lsa min-arrival [(0-600000)]", 
       "Negate a command or set its defaults\n"
       "Adjust routing timers\n"
       "OSPF LSA timers\n"
       "Minimum delay in receiving new version of a LSA\n"
       "Delay in milliseconds\n")

DEFSH (VTYSH_RIPD, if_rmap_cmd_vtysh, 
       "route-map RMAP_NAME <in|out> IFNAME", 
       "Route map set\n"
       "Route map name\n"
       "Route map set for input filtering\n"
       "Route map set for output filtering\n"
       "Route map interface name\n")

DEFSH (VTYSH_OSPFD, ospf_distance_ospf_cmd_vtysh, 
       "distance ospf {intra-area (1-255)|inter-area (1-255)|external (1-255)}", 
       "Administrative distance\n"
       "OSPF administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n")

DEFSH (VTYSH_LDPD, ldp_session_holdtime_session_time_cmd_vtysh, 
       "session holdtime (15-65535)", 
       "Configure session parameters\n"
       "Configure session holdtime\n"
       "Time (seconds)\n")

DEFSH_HIDDEN (VTYSH_OSPFD, no_ospf_network_cmd_vtysh, 
              "no ospf network [<broadcast|non-broadcast|point-to-multipoint|point-to-point>]", 
              "Negate a command or set its defaults\n"
              "OSPF interface commands\n"
              "Network type\n"
              "Specify OSPF broadcast multi-access network\n"
              "Specify OSPF NBMA network\n"
              "Specify OSPF point-to-multipoint network\n"
              "Specify OSPF point-to-point network\n")

DEFSH (VTYSH_ZEBRA, no_match_ip_nexthop_prefix_len_cmd_vtysh, 
       "no match ip next-hop prefix-len [(0-32)]", 
       "Negate a command or set its defaults\n"
       "Match values from routing table\n"
       "IP information\n"
       "Match prefixlen of nexthop ip address\n"
       "Match prefix length of nexthop\n"
       "Prefix length\n")

DEFSH (VTYSH_BGPD, neighbor_route_reflector_client_cmd_vtysh, 
       "neighbor <A.B.C.D|X:X::X:X|WORD> route-reflector-client", 
       "Specify neighbor router\n"
       "Neighbor address\nNeighbor IPv6 address\nInterface name or neighbor tag\n"
       "Configure a neighbor as Route Reflector client\n")

/* modified by liwh for bug 45793, 2017-12-07 */
#ifdef _CENTEC_
DEFSH (VTYSH_OSPFD, debug_ospf_event_sub_cmd_vtysh, 
       "debug ospf event <abr|ia|route|spf>", 
       "Debugging functions (see also 'undebug')\n"
       "OSPF information\n"
       "OSPF event information\n"
       "OSPF Area Border Router\n"
       "OSPF Inter-area Routing\n"
       "OSPF Route Table\n"
       "OSPF Shortest Path First Calculation\n")
#endif
/*liwh end */

/* modified by liwh for bug 44589, 2017-07-06 */
#ifdef _CENTEC_
DEFSH (VTYSH_OSPFD, clear_ip_ospf_process_cmd_vtysh,
       "clear ip ospf process",
       "Reset functions\n"
       "IP information\n"
       "OSPF information\n"
       "Reset OSPF process\n")
#endif
/* liwh end */

/* modified by liwh for bug 44929, 2017-08-08 */
#ifdef _CENTEC_
DEFSH (VTYSH_OSPFD, reset_ip_ospf_process_cmd_vtysh,
       "reset ip ospf process",
       "Reset functions\n"
       "IP information\n"
       "OSPF information\n"
       "Reset OSPF process\n")
#endif
/* liwh end */

void
vtysh_init_cmd ()
{
  install_element (VIEW_NODE, &show_ip_pim_rp_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_standard_host_cmd_vtysh);
  install_element (OSPF6_NODE, &auto_cost_reference_bandwidth_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ipx_vpn_nexthop_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_lsa_hex_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_brouter_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_pim_ssm_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_neighbor_ipv6_targeted_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_address_prefix_len_cmd_vtysh);
  install_element (RMAP_NODE, &ospf6_routemap_no_set_forwarding_cmd_vtysh);
  install_element (ISIS_NODE, &log_adj_changes_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_ospf_rmap_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_timers_min_ls_interval_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_authentication_string_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_igmp_trace_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_neighbor_address_addr_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_retransmitinterval_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (ISIS_NODE, &no_set_overload_bit_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_network_mask_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_protocol_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_safi_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_l2vpn_atom_binding_json_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_rip_status_cmd_vtysh);
  install_element (RIP_NODE, &if_rmap_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_network_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_protocol_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_bgp_redistribute_ipv6_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_abr_type_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_packet_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_nexthop_prefix_len_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_local_as_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_event_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_stub_router_admin_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_mpls_te_inter_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_managed_config_flag_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_ge_le_cmd_vtysh);
  install_element (ISIS_NODE, &no_lsp_refresh_interval_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ipv6_nexthop_local_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_opaque_cmd_vtysh);
  install_element (ISIS_NODE, &no_metric_style_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_distance_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_local_membership_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_brouter_router_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_nhrp_cmd_vtysh);
  install_element (BGP_NODE, &aggregate_address_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_default_metric_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_neighbor_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_updgrps_adj_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_ip_vpn_all_rd_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_all_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (ENABLE_NODE, &show_zebra_client_summary_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_cluster_id_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_description_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_address_prefix_list_cmd_vtysh);
  install_element (RMAP_NODE, &set_community_none_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_rip_zebra_cmd_vtysh);
  install_element (RIP_NODE, &rip_default_information_originate_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_igmp_events_cmd_vtysh);
  install_element (RMAP_NODE, &set_tag_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_pw_id_pwid_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_ospf_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_route_source_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (ENABLE_NODE, &show_debugging_ospf_instance_cmd_vtysh);
  install_element (BGP_NODE, &no_aggregate_address_mask_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_ssm_prefix_list_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_range_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_maxpaths_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_compatible_rfc1583_cmd_vtysh);
  install_element (RIP_NODE, &rip_offset_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_prefix_longer_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (ENABLE_NODE, &vnc_clear_counters_cmd_vtysh);
  install_element (RMAP_NODE, &no_rmap_onmatch_next_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_network_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_spfstats_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_distance_ospf6_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_un_vn_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_psnp_interval_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rip_zebra_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_split_horizon_poisoned_reverse_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_igmp_trace_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_mask_natural_route_map_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_aspath_prepend_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_range_cost_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (ISIS_NODE, &no_max_lsp_lifetime_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_circuit_type_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_errors_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_mask_any_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_msdp_sa_sg_cmd_vtysh);
  install_element (BGP_NODE, &bgp_always_compare_med_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_compatible_rfc1583_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_instance_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_rd_tags_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_event_cmd_vtysh);
  install_element (RIP_NODE, &rip_version_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_maxmed_admin_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_to_acl_for_acl_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_rip_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_damp_unset_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_distance_label_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_spfevents_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_description_comment_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_timers_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_le_ge_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_memory_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_lsp_gen_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_mtu_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_lifetime_cmd_vtysh);
  install_element (RMAP_NODE, &set_aggregator_as_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_update_groups_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_summary_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_multicast_mode_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_life_cost_lnh_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_ecmp_rebalance_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_area_stub_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &ipv6_aggregate_address_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_neighbor_ipv4_session_holdtime_session_time_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ecommunity_soo_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_nssa_cmd_vtysh);
  install_element (ISIS_NODE, &no_isis_redistribute_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_ssmpingd_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_any_mask_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_adv_router_linkstate_id_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_ge_le_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_homeagent_config_flag_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_targeted_hello_accept_from_acl_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_update_delay_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_protocol_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_kernel_cmd_vtysh);
  install_element (ISIS_NODE, &set_overload_bit_cmd_vtysh);
  install_element (LDP_NODE, &ldp_router_id_ipv4_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_int_detail_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_shortcut_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_route_match_detail_cmd_vtysh);
  install_element (BGP_EVPN_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_adv_interval_config_option_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_brouter_area_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_neighbor_arg_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_redist_bgpdirect_no_routemap_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_remote_accept_for_acl_from_acl_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_timers_throttle_spf_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_update_source_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_authentication_args_addr_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_brouter_router_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_nhs_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_bgp_instance_ipv6_safi_prefix_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_summary_prefix_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_spf_database_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_igmp_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_targeted_hello_accept_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_offset_list_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_nsm_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_distance_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_clear_mpls_ldp_neighbor_addr_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_override_capability_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_summary_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_bgp_distance_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_l2rd_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_nssa_no_summary_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_neighbor_detail_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_detail_name_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_split_horizon_poisoned_reverse_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_pim_oil_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_instance_updgrps_stats_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_deterministic_med_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_interface_config_v6only_cmd_vtysh);
  install_element (CONFIG_NODE, &no_zebra_route_map_timer_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_packet_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_zebra_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_rtevents_cmd_vtysh);
  install_element (BGP_NODE, &vnc_export_nvegroup_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_capability_enhe_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_asbr_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_network_route_map_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_updgrps_stats_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_cmd_vtysh);
  install_element (ENABLE_NODE, &rmap_show_name_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_allocate_host_routes_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_msdp_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_allow_martians_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_tag_label_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_network_mask_cmd_vtysh);
  install_element (OSPF6_NODE, &area_range_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_ssmpingd_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_protocol_nht_rmap_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_wpkt_quanta_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_use_bw_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_tag_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_summary_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_transport_address_ipv4_cmd_vtysh);
  install_element (BGP_VNC_L2_GROUP_NODE, &vnc_l2_group_no_labels_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_next_hop_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_interface_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_timers_min_ls_arrival_cmd_vtysh);
  install_element (RMAP_NODE, &match_local_pref_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_weight_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_next_hop_prefix_list_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_redistribute_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_extcommunity_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_lsa_hex_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_show_import_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_as4_segment_cmd_vtysh);
  install_element (RIP_NODE, &rip_timers_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_next_hop_prefix_list_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_range_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_remote_as_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_mpls_te_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_targeted_hello_accept_from_acl_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_instance_zebra_cmd_vtysh);
  install_element (OSPF_NODE, &pce_cap_flag_cmd_vtysh);
  install_element (INTERFACE_NODE, &bandwidth_if_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ripng_zebra_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_id_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_lsp_sched_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_errors_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_psnp_interval_l1_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_allow_ecmp_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_csnp_interval_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_metric_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_mtu_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_authentication_key_chain_cmd_vtysh);
  install_element (ENABLE_NODE, &show_debugging_bgp_vnc_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_kernel_msgdump_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_messages_recv_cmd_vtysh);
  install_element (CONFIG_NODE, &undebug_msdp_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_zebra_fpm_stats_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_access_list_any_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_atomic_aggregate_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_area_stub_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_redistribute_routemap_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_nht_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_instance_updgrps_adj_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_abr_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_ospf_metric_rmap_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_transmitdelay_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_route_source_prefix_list_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_pw_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rip_packet_cmd_vtysh);
  install_element (ISIS_NODE, &no_area_passwd_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_targeted_hello_interval_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_pim_packets_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_afi_vpn_rd_route_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_dead_interval_minimal_addr_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_no_label_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_zebra_sendrecv_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_description_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_messages_sent_all_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_messages_recv_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_timers_connect_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_any_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_zebra_ptm_enable_if_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_ttl_security_disable_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_adj_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_send_version_2_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_padding_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_remote_accept_cmd_vtysh);
  install_element (ISIS_NODE, &domain_passwd_clear_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_local_pref_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_bgp_cmd_vtysh);
  install_element (RMAP_NODE, &set_ipx_vpn_nexthop_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_label_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_passive_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (LDP_NODE, &ldp_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_ge_le_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_query_vn_un_cmd_vtysh);
  install_element (BGP_NODE, &bgp_bestpath_compare_router_id_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_debug_packet_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_homeagent_lifetime_cmd_vtysh);
  install_element (RMAP_NODE, &set_ipv6_nexthop_peer_cmd_vtysh);
  install_element (PW_NODE, &pseudowire_labels_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_updgrps_afi_adj_cmd_vtysh);
  install_element (RMAP_NODE, &set_ecommunity_rt_cmd_vtysh);
  install_element (ISIS_NODE, &spf_delay_ietf_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_all_overlay_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ecommunity_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_no_bridge_ifname_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_upstream_rpf_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_default_originate_rmap_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_to_acl_for_acl_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_joinprune_time_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_maxmed_onstartup_cmd_vtysh);
  install_element (ISIS_NODE, &domain_passwd_md5_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_mroute_cmd_vtysh);
  install_element (LDP_IPV6_IFACE_NODE, &ldp_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_adv_interval_config_option_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_protocol_nht_cmd_vtysh);
  install_element (CONFIG_NODE, &no_nhrp_event_socket_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_hello_interval_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_msdp_events_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_community_list_standard_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_ospf_interface_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_le_ge_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_targeted_hello_accept_from_acl_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_default_cost_cmd_vtysh);
  install_element (CONFIG_NODE, &vrf_debug_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_debugging_mpls_ldp_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_cmd_vtysh);
  install_element (ISIS_NODE, &net_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_msdp_peer_cmd_vtysh);
  install_element (ISIS_NODE, &metric_style_cmd_vtysh);
  install_element (OSPF_NODE, &pce_neighbor_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_range_substitute_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_messages_recv_all_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_default_originate_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_nves_ptct_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_update_direct_peer_cmd_vtysh);
  install_element (ENABLE_NODE, &show_ip_bgp_instance_neighbor_prefix_counts_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_self_originated_linkstate_id_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_pim_drprio_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_addr_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_standard_cmd_vtysh);
  install_element (RIP_NODE, &rip_neighbor_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_local_as_no_prepend_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_vlink_intervals_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_ipv6_bgp_distance_source_access_list_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_ge_cmd_vtysh);
  install_element (BGP_NODE, &aggregate_address_mask_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_minadvertinterval_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_interval_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_to_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_update_direct_peer_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_bgp_set_route_map_delay_timer_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_multicast_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_interval_msec_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_priority_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_ge_le_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_neighbor_capabilities_json_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_prefix_vn_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_instance_zebra_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_dual_stack_cisco_interop_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_spfevents_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_no_mtu_mtu_cmd_vtysh);
  install_element (VIEW_NODE, &show_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_multicast_routing_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_binding_json_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_neighbor_events_peer_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_register_vn_un_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_map_cmd_vtysh);
  install_element (ISIS_NODE, &isis_mpls_te_router_addr_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (LDP_IPV6_IFACE_NODE, &ldp_no_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (RMAP_NODE, &set_ip_nexthop_unchanged_cmd_vtysh);
  install_element (BGP_EVPN_NODE, &no_evpnrt5_network_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (ENABLE_NODE, &show_zebra_client_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_enable_cmd_vtysh);
  install_element (RMAP_NODE, &rmap_description_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (OSPF6_NODE, &area_export_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_as4_segment_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_asbr_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_member_interface_ifname_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_remote_accept_from_acl_for_acl_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_timers_lsa_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_instance_lsa_cmd_vtysh);
  install_element (VIEW_NODE, &show_vrf_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_flooding_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_homeagent_config_flag_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_maxpaths_ibgp_cluster_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_log_neighbor_changes_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_mroute_count_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_network_mask_natural_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_pw_status_disable_cmd_vtysh);
  install_element (INTERFACE_NODE, &csnp_interval_l2_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_mroute_cmd_vtysh);
  install_element (BGP_NODE, &bgp_enforce_first_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_mpls_transit_lsp_out_label_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_remote_accept_for_acl_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (RMAP_NODE, &no_rmap_onmatch_goto_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_neighbor_ipv4_ttl_security_hops_hops_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_ifname_flags_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_timers_cmd_vtysh);
  install_element (LDP_IPV4_IFACE_NODE, &ldp_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_multicast_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_rt_import_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_spf_database_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_standard_cmd_vtysh);
  install_element (ISIS_NODE, &area_lsp_mtu_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_bestpath_prefix_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_l2rd_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_priority_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_lcommunity_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_coalesce_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_shutdown_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_no_l2rd_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (VTY_NODE, &vty_ipv6_access_class_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_default_information_originate_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_zebra_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_auto_cost_reference_bandwidth_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_spftrigg_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (BGP_NODE, &bgp_distance_cmd_vtysh);
  install_element (BGP_NODE, &bgp_default_subgroup_pkt_queue_max_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_priority_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_zebra_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_any_host_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_assert_metric_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_igmp_packets_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_bgp_maxpaths_ibgp_cmd_vtysh);
  install_element (ENABLE_NODE, &show_zebra_fpm_stats_cmd_vtysh);
  install_element (BGP_VNC_L2_GROUP_NODE, &vnc_l2_group_rt_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_authentication_key_authkey_addr_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_aggregate_address_mask_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_update_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_nsm_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (ISIS_NODE, &area_passwd_clear_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_upd_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_for_acl_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_graceful_restart_stalepath_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_router_isis_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_nht_vrf_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_interface_name_vrf_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_addr_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_infinite_month_day_cmd_vtysh);
  install_element (BGP_NODE, &bgp_fast_external_failover_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_prefix_list_name_prefix_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_messages_recv_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_range_not_advertise_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_mtu_ignore_addr_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_update_groups_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_rtevents_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_ifmtu_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_default_originate_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_lsp_sched_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_rip_packet_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_other_config_flag_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_rt_export_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_label_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_distance_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_pim_use_source_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_neighbor_json_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_tunnel_protection_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_ism_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_priority_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_lcommunity_list_expanded_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_network_import_check_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_event_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_nsm_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_passive_interface_addr_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_neighbor_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ipv6_rip_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_multicast_mode_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_maxadvertinterval_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_mask_host_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_aggregate_address_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ripng_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_disable_connected_check_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_network_mask_natural_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_zebra_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_upstream_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_bestpath_aspath_multipath_relax_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ripng_packet_cmd_vtysh);
  install_element (RIP_NODE, &rip_allow_ecmp_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_distance_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_message_digest_key_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_ospf6_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ssmpingd_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_distance_ospf_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_clear_mpls_ldp_neighbor_cmd_vtysh);
  install_element (RMAP_NODE, &match_lcommunity_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_advertise_interval_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_v2_broadcast_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_packet_dump_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_distance_source_cmd_vtysh);
  install_element (VTY_NODE, &no_vty_access_class_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_description_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_lcommunity_info_cmd_vtysh);
  install_element (RIP_NODE, &rip_redistribute_type_metric_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_bestpath_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_kernel_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_mask_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_le_cmd_vtysh);
  install_element (BGP_NODE, &no_synchronization_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_weight_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_stub_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_label_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_nhrp_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_mpls_te_router_addr_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_neighbor_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_linkdetect_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_brouter_router_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_mode_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_remote_accept_for_acl_from_acl_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &aggregate_address_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_lcommunity_list_standard1_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_interface_detail_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_timers_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_msdp_mesh_group_source_cmd_vtysh);
  install_element (RIPNG_NODE, &ipv6_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_instance_nsm_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_flags_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_instance_event_cmd_vtysh);
  install_element (RMAP_NODE, &match_ipv6_address_prefix_list_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_neighbor_events_peer_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_distance_source_access_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_zebra_sendrecv_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_mroute_cmd_vtysh);
  install_element (OSPF_NODE, &no_router_info_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_stub_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_as4_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_database_max_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_neighbor_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_network_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_lsa_hex_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_igmp_events_cmd_vtysh);
  install_element (OSPF6_NODE, &no_area_import_list_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_shutdown_msg_cmd_vtysh);
  install_element (ISIS_NODE, &spf_interval_l1_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_adj_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_table_map_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_community_list_standard_all_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_hello_interval_cmd_vtysh);
  install_element (BGP_NODE, &vnc_no_vrf_policy_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_rip_events_cmd_vtysh);
  install_element (BGP_NODE, &bgp_timers_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_multiplier_l2_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_ge_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_import_check_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_igmp_events_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (RMAP_NODE, &no_rmap_description_cmd_vtysh);
  install_element (BGP_NODE, &bgp_disable_connected_route_check_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_igmp_join_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_zebra_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_protocol_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_hellointerval_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_msdp_packets_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_kernel_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_priority_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_nhs_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_sequence_number_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_session_holdtime_session_time_cmd_vtysh);
  install_element (CONFIG_NODE, &allow_external_route_update_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_metric_l2_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_interfaces_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_ge_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxmed_onstartup_medv_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_managed_config_flag_cmd_vtysh);
  install_element (OSPF_NODE, &no_pce_cap_flag_cmd_vtysh);
  install_element (BGP_NODE, &vnc_no_redistribute_protocol_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_int_detail_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_no_neighbor_lsr_id_ipv4_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_irdp_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_aggregator_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_update_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_remote_accept_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_registrations_pfx_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_topology_l1_cmd_vtysh);
  install_element (BGP_EVPN_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_discovery_hello_dir_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &no_key_chain_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_events_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_capability_dynamic_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_cost_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_kernel_msgdump_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_csum_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_area_stub_no_summary_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_spf_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_route_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_vn_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_route_cmd_vtysh);
  install_element (RMAP_NODE, &set_weight_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_pim_trace_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_access_list_exact_cmd_vtysh);
  install_element (BGP_NODE, &bgp_set_route_map_delay_timer_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_distribute_list_out_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_redistribute_ipv4_cmd_vtysh);
  install_element (OSPF_NODE, &pce_address_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_nexthop_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_extcommunity_list_standard_all_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_rp_prefix_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_rd_neighbor_routes_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (BGP_NODE, &bgp_bestpath_aspath_confed_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_summary_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_supernets_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &vpnv6_network_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_register_suppress_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_transmit_delay_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_snp_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_rib_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_mroute_dist_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ripng_zebra_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_confederation_identifier_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_bgp_dampening_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_label_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_ifname_pref_label_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_table_map_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_network_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_graceful_restart_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_rip_events_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_nhrp_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_hello_interval_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_for_acl_to_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_fpm_cmd_vtysh);
  install_element (LDP_IPV4_IFACE_NODE, &ldp_no_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_zebra_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_router_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_max_metric_router_lsa_admin_cmd_vtysh);
  install_element (VTY_NODE, &no_vty_login_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_messages_sent_all_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_cost_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_local_pref_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_vc_type_pwtype_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &csnp_interval_l1_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_remote_accept_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_all_neighbor_routes_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_priority_l2_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_table_map_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_nexthop_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_default_originate_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_aggregate_address_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (VTY_NODE, &exec_timeout_sec_cmd_vtysh);
  install_element (ISIS_NODE, &no_set_attached_bit_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_spf_process_cmd_vtysh);
  install_element (CONFIG_NODE, &nhrp_event_socket_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_rp_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_all_route_prefix_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_authentication_args_addr_cmd_vtysh);
  install_element (OSPF6_NODE, &area_filter_list_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_route_source_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_remote_accept_for_acl_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_interface_area_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_nhrp_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_cost_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_access_list_any_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_passwd_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_day_month_day_month_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_topology_l2_cmd_vtysh);
  install_element (ENABLE_NODE, &show_ip_access_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_border_routers_cmd_vtysh);
  install_element (ISIS_NODE, &isis_default_originate_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rip_packet_direct_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_peer_group_cmd_vtysh);
  install_element (BGP_NODE, &bgp_rr_allow_outbound_policy_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_multiplier_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_timers_cmd_vtysh);
  install_element (RMAP_NODE, &match_tag_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (ISIS_NODE, &no_log_adj_changes_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_events_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_passive_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_as_path_all_cmd_vtysh);
  install_element (CONFIG_NODE, &fpm_remote_ip_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_neighbor_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_duration_month_day_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_rfc1583_flag_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_open_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_as4_segment_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_nssa_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_month_day_month_day_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_cost_life_lnh_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_brouter_area_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_prefix_longer_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_instance_event_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_route_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_ge_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_strict_capability_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_responselifetime_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_prefix_cost_life_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_weight_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_errors_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_max_rsv_bw_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ripng_events_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_allocate_host_routes_cmd_vtysh);
  install_element (BGP_NODE, &bgp_deterministic_med_cmd_vtysh);
  install_element (ENABLE_NODE, &undebug_msdp_packets_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_passive_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_prefix_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_multiplier_l1_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_split_horizon_cmd_vtysh);
  install_element (PW_NODE, &pseudowire_control_word_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_bestpath_compare_router_id_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (BGP_NODE, &vnc_no_nve_group_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_max_metric_router_lsa_shutdown_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_host_mask_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_update_direct_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_zebra_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ripng_split_horizon_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_rh_roo_localadmin_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ssmpingd_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_redistribute_ipv4_ospf_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_pkt_loss_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_packet_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_network_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_rd_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_lsp_gen_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_prefix_cost_cmd_vtysh);
  install_element (ISIS_NODE, &no_spf_delay_ietf_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_prefix_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_holdtime_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &psnp_interval_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_upd_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_multiplier_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_ava_bw_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_standard_host_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_static_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_protocol_nht_rmap_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_timers_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_redistribute_ipv6_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_nve_all_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_update_direct_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_interface_config_remote_as_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_network_cmd_vtysh);
  install_element (BGP_NODE, &bgp_confederation_peers_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_timers_throttle_spf_cmd_vtysh);
  install_element (OSPF_NODE, &no_capability_opaque_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_default_originate_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_spf_time_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_remote_accept_from_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &show_debugging_ospf_cmd_vtysh);
  install_element (RMAP_NODE, &no_rmap_call_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_offset_list_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_msdp_events_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_area_stub_no_summary_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_log_adjacency_changes_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_instance_lsa_cmd_vtysh);
  install_element (BGP_NODE, &bgp_coalesce_time_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_update_prefix_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ripng_zebra_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_prefix_longer_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_lupd_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_mroute_dist_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_allow_martians_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_instance_nssa_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_update_groups_cmd_vtysh);
  install_element (ISIS_NODE, &no_dynamic_hostname_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_rt_export_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_err_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_discovery_detail_json_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_mroute_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_id_self_originated_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_message_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_discovery_hello_dir_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_targeted_hello_interval_disc_time_cmd_vtysh);
  install_element (BGP_NODE, &bgp_default_ipv4_unicast_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_circuit_type_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_network_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_update_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_shutdown_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_admin_grp_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_spf_process_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_filter_list_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_passive_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_lcommunity_list_name_expanded_all_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_messages_sent_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_tag_distance_label_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_mask_tag_label_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_msdp_mesh_group_source_cmd_vtysh);
  install_element (RMAP_NODE, &rmap_call_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_router_id_ipv4_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_database_type_adv_router_cmd_vtysh);
  install_element (RMAP_NODE, &set_local_pref_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_spt_switchover_infinity_plist_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_le_ge_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_metric_l2_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_le_ge_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_probability_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_month_day_month_day_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_community_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_opaque_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_default_originate_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redist_bgpdirect_prefixlist_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_nht_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_prefix_vn_un_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_range_substitute_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_res_bw_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_csum_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_cost_life_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_err_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_redistribute_ipv4_ospf_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_exact_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_to_acl_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_ospf_metric_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_route_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_targeted_hello_accept_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_bfd_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ripng_packet_cmd_vtysh);
  install_element (RMAP_NODE, &set_aspath_exclude_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_import_check_exact_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_keepalive_peer_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_ttl_security_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_forwarding_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_prefix_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_nve_un_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_remote_accept_for_acl_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_rip_packet_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_export_routemap_cmd_vtysh);
  install_element (ISIS_NODE, &isis_mpls_te_inter_as_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxmed_onstartup_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_ecmp_rebalance_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_priority_l2_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_v2_broadcast_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_neighbor_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_neighbor_ipv6_targeted_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_binding_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_packet_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_instance_nsm_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_tag_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_router_id_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_activate_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_instance_nssa_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_database_type_adv_router_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_advertise_prefix_list_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_default_information_originate_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_other_config_flag_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_timers_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_receive_version_2_cmd_vtysh);
  install_element (RMAP_NODE, &set_lcommunity_none_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_id_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_lsa_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_static_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_assert_internal_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &aggregate_address_mask_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_explicit_null_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (BGP_NODE, &bgp_bestpath_med_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_infinite_day_month_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_instance_updgrps_adj_s_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_brouter_cmd_vtysh);
  install_element (ZEBRA_NODE, &redistribute_ospf6_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_host_mask_cmd_vtysh);
  install_element (LDP_IPV4_IFACE_NODE, &ldp_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_packet_dump_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_lcommunity_list_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_mask_host_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_network_mask_natural_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_zebra_sendrecv_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_fpm_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_metric_cmd_vtysh);
  install_element (RIP_NODE, &no_if_rmap_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_session_holdtime_session_time_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_mask_any_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_distance_source_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_remote_accept_for_acl_from_acl_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ipv6_nexthop_global_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_mtu_ignore_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_bestpath_prefix_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_auto_cost_reference_bandwidth_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_address_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_msdp_cmd_vtysh);
  install_element (CONFIG_NODE, &undebug_msdp_packets_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_zebra_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_mask_flags_cmd_vtysh);
  install_element (VIEW_NODE, &show_pseudowires_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (BGP_NODE, &vnc_no_l2_group_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_mpls_te_inter_as_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_nexthop_local_unchanged_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_life_cost_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_packet_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_remark_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_id_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_distance_source_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_ifname_label_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_brouter_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_tag_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_interface_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_instance_neighbor_advertised_route_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_confederation_peers_cmd_vtysh);
  install_element (ISIS_NODE, &no_isis_mpls_te_on_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_redistribute_ipv6_rmap_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_ge_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_infinite_month_day_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_distance_ospf6_cmd_vtysh);
  install_element (OSPF6_NODE, &no_auto_cost_reference_bandwidth_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_ecmp_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_packet_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_route_map_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_irdp_address_preference_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_rip_packet_direct_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_enable_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_network_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_static_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_all_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_event_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_packetdump_send_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_ssm_range_cmd_vtysh);
  install_element (VIEW_NODE, &show_debugging_pim_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_ospf_rmap_metric_cmd_vtysh);
  install_element (RIPNG_NODE, &distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ripng_packet_direct_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_cost_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_linkstate_detail_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_as4_segment_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ipv6_address_prefix_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_rib_detailed_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_instance_lsa_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_maxpaths_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_mroute_detail_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_interface_peer_group_remote_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_redistribute_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_rp_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_nht_vrf_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_reachable_time_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_description_comment_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_timers_min_ls_interval_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &no_key_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_detail_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_targeted_hello_interval_disc_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_pim_use_source_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_tag_label_cmd_vtysh);
  install_element (LDP_NODE, &ldp_neighbor_ipv4_ttl_security_disable_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_pref_label_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_ifname_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_authentication_key_chain_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_distribute_list_out_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_interface_config_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_origin_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_le_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_sequence_number_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_allow_martians_cmd_vtysh);
  install_element (INTERFACE_NODE, &psnp_interval_l2_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_to_acl_for_acl_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxpaths_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_as_path_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_ospf_instance_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_trace_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_cost_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_allocate_host_routes_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_spfstats_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ipv6_prefix_list_name_prefix_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_write_multiplier_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_ifname_flags_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_targeted_hello_accept_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_remote_accept_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_mroute_vrf_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_detail_all_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_spfevents_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_igmp_query_interval_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_csnp_interval_l1_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_allocate_for_acl_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_upd_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_prefix_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_for_acl_to_acl_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_bgp_exterior_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (OSPF_NODE, &write_multiplier_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_mpls_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_area_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_metric_rmap_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_interface_area_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_bgp_distance_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_csnp_interval_l2_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_network_mask_cmd_vtysh);
  install_element (BGP_NODE, &bgp_listen_limit_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_life_lnh_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_metric_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_as4_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_message_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_update_source_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_protocol_nht_cmd_vtysh);
  install_element (VIEW_NODE, &show_database_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_vn_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_border_routers_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_forwarding_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_cost_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_mask_distance_label_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ripng_packet_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_cost_lnh_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_brouter_router_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_pim_events_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_summary_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_ism_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_host_host_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_spfstats_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_log_adjacency_changes_detail_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_igmp_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_asbr_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_static_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_mask_tag_distance_label_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_for_acl_to_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_pim_interfaces_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_mask_tag_label_cmd_vtysh);
  install_element (RMAP_NODE, &set_atomic_aggregate_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_self_originated_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_lcommunity_list_name_expanded_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_access_list_remark_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_bfd_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_kernel_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_distance_source_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_err_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_route_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_messages_recv_all_cmd_vtysh);
  install_element (ENABLE_NODE, &show_debugging_isis_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_prefix_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_address_prefix_len_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_updgrps_afi_adj_s_cmd_vtysh);
  install_element (ZEBRA_NODE, &ripng_redistribute_ripng_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_snp_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_interface_config_cmd_vtysh);
  install_element (BGP_NODE, &vnc_nve_export_no_routemap_cmd_vtysh);
  install_element (ENABLE_NODE, &undebug_msdp_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_discovery_hello_dir_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (BGP_NODE, &bgp_table_map_cmd_vtysh);
  install_element (ZEBRA_NODE, &no_ripng_redistribute_ripng_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_lupd_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_filter_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_standard_any_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_password_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_ge_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_authentication_mode_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_maxpaths_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (RMAP_NODE, &set_lcommunity_delete_cmd_vtysh);
  install_element (INTERFACE_NODE, &zebra_ptm_enable_if_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_default_originate_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_strict_capability_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (LDP_NODE, &ldp_dual_stack_cisco_interop_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_lnh_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_remark_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_router_id_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (CONFIG_NODE, &bgp_multiple_instance_cmd_vtysh);
  install_element (ZEBRA_NODE, &no_rip_redistribute_rip_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &ipv6_bgp_distance_source_access_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_summary_name_cmd_vtysh);
  install_element (RMAP_NODE, &match_ipv6_next_hop_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_nssa_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_redistribute_ipv6_metric_rmap_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_binding_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_igmp_query_max_response_time_dsec_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_rd_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_ospf_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ripng_events_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_network_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (ISIS_NODE, &no_area_lsp_mtu_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_interface_ifname_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_rip_packet_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_protocol_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ripng_split_horizon_poisoned_reverse_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_delay_var_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_views_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_import_list_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_cmd_vtysh);
  install_element (BGP_NODE, &bgp_graceful_restart_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_neighbor_events_peer_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_metric_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_mpls_te_on_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (OSPF_NODE, &router_info_area_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &bgp_distance_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_paths_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_message_digest_key_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_lcommunity_list_name_standard_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_protocol_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_retransmit_interval_cmd_vtysh);
  install_element (RMAP_NODE, &match_probability_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_passive_interface_addr_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_vn_un_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_messages_sent_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_debug_messages_cmd_vtysh);
  install_element (RMAP_NODE, &rmap_onmatch_goto_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_distance_source_access_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_detail_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_msdp_mesh_group_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_un_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_allocate_host_routes_cmd_vtysh);
  install_element (RIP_NODE, &rip_redistribute_type_metric_routemap_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_mask_natural_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_distance_source_access_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_name_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_source_protocol_cmd_vtysh);
  install_element (OSPF6_NODE, &no_area_filter_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_packet_dump_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_vn_un_prefix_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_ospf_metric_cmd_vtysh);
  install_element (ZEBRA_NODE, &rip_redistribute_rip_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_vlink_intervals_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_import_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_holdtime_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_multiplier_l2_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_retransmit_interval_addr_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_upstream_join_desired_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_default_information_originate_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_address_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_all_cmd_vtysh);
  install_element (VIEW_NODE, &clear_isis_neighbor_arg_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_binding_detail_json_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &no_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_lsp_sched_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_metric_l1_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_linkstate_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redist_bgpdirect_no_prefixlist_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_isis_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_join_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (LDP_IPV4_IFACE_NODE, &ldp_no_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_stub_router_admin_cmd_vtysh);
  install_element (ISIS_NODE, &lsp_gen_interval_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_pim_zebra_cmd_vtysh);
  install_element (CONFIG_NODE, &no_zebra_ptm_enable_cmd_vtysh);
  install_element (ISIS_NODE, &isis_mpls_te_on_cmd_vtysh);
  install_element (BGP_NODE, &vnc_nve_export_prefixlist_cmd_vtysh);
  install_element (CONFIG_NODE, &undebug_msdp_events_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_any_host_cmd_vtysh);
  install_element (VIEW_NODE, &show_interface_vrf_all_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_allow_external_route_update_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_ge_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_prefix_longer_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &vpnv4_network_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_te_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_no_pw_status_disable_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_preference_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_prefix_un_vn_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_redistribute_type_metric_cmd_vtysh);
  install_element (ISIS_NODE, &spf_interval_l2_cmd_vtysh);
  install_element (VIEW_NODE, &show_hostname_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_lcommunity_list_name_standard_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_redistribute_type_routemap_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_vnc_cmd_vtysh);
  install_element (LDP_NODE, &ldp_neighbor_ipv4_password_word_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_spf_database_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_int_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_message_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_authentication_key_addr_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_debug_misc_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_offset_list_ifname_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (KEYCHAIN_NODE, &no_key_cmd_vtysh);
  install_element (CONFIG_NODE, &no_mpls_transit_lsp_all_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_ifname_cmd_vtysh);
  install_element (CONFIG_NODE, &config_table_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_suppress_ra_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_to_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_res_bw_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_nhrp_cmd_vtysh);
  install_element (BGP_NODE, &bgp_graceful_restart_preserve_fw_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_metric_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_neighbor_events_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_route_cmd_vtysh);
  install_element (BGP_NODE, &vnc_export_mode_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_enforce_first_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_holdtime_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_sequence_number_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_nhrp_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_activate_cmd_vtysh);
  install_element (INTERFACE_NODE, &linkdetect_cmd_vtysh);
  install_element (ZEBRA_NODE, &no_redistribute_ospf6_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_prefix_un_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_router_isis_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_packet_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_msdp_events_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ripng_status_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (ISIS_NODE, &lsp_refresh_interval_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_abr_cmd_vtysh);
  install_element (ENABLE_NODE, &undebug_msdp_events_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_rib_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_standard_nomask_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_multicast_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_zebra_sendrecv_cmd_vtysh);
  install_element (LDP_NODE, &ldp_neighbor_ipv4_session_holdtime_session_time_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (VTY_NODE, &no_exec_timeout_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_mask_flags_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_msdp_peer_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_distance_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (BGP_EVPN_NODE, &neighbor_activate_cmd_vtysh);
  install_element (RMAP_NODE, &set_ipv6_nexthop_global_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_rt_import_cmd_vtysh);
  install_element (RMAP_NODE, &match_origin_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_all_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ripng_packet_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_le_ge_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_mtu_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_weight_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_ttl_security_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_tag_label_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_simulate_spf_tree_root_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_neighbor_lsr_id_ipv4_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_remote_accept_for_acl_from_acl_cmd_vtysh);
  install_element (BGP_NODE, &bgp_default_local_preference_cmd_vtysh);
  install_element (RIP_NODE, &distribute_list_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_network_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_retransmit_interval_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_nexthop_lookup_cmd_vtysh);
  install_element (LDP_NODE, &ldp_discovery_targeted_hello_interval_disc_time_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ripng_events_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_for_acl_to_acl_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_distance_source_access_list_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ipv6_prefix_list_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_nssa_cmd_vtysh);
  install_element (ENABLE_NODE, &show_ipv6_access_list_name_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_priority_l1_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_duration_day_month_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_neighbor_routes_cmd_vtysh);
  install_element (VIEW_NODE, &show_interface_name_vrf_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_bfd_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_lcommunity1_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_zebra_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_rt_both_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxmed_admin_medv_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_authentication_string_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_timers_throttle_spf_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_pim_trace_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_deadinterval_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_distance_source_cmd_vtysh);
  install_element (RMAP_NODE, &match_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_ifname_tag_label_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_offset_list_ifname_cmd_vtysh);
  install_element (RMAP_NODE, &set_ecommunity_soo_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_mroute_detail_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_mtu_ignore_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_cmd_vtysh);
  install_element (RMAP_NODE, &match_metric_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_reg_flags_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_show_import_vn_un_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_register_vn_un_l2o_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_messages_sent_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_interval_l2_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_keepalive_peer_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_nht_default_route_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_receive_version_1_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_mroute_source_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_rib_cmd_vtysh);
  install_element (RMAP_NODE, &match_community_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_explicit_null_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_rib_detailed_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_spftrigg_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_priority_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_max_metric_router_lsa_admin_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_le_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ip_nexthop_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_upd_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_mask_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_events_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_session_holdtime_session_time_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_responses_some_pfx_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_zebra_import_table_cmd_vtysh);
  install_element (ISIS_NODE, &no_lsp_gen_interval_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_mpls_te_interface_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip6_router_isis_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_spf_tree_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_instance_event_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_rd_neighbor_advertised_routes_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_passive_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_to_acl_for_acl_cmd_vtysh);
  install_element (RIP_NODE, &rip_distance_source_access_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_send_version_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_prefix_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_interval_l1_cmd_vtysh);
  install_element (RMAP_NODE, &set_aspath_prepend_asn_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_ge_le_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_router_preference_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_spf_process_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_damp_unset_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_dead_interval_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (PW_NODE, &pseudowire_neighbor_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_homeagent_preference_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_protocol_cmd_vtysh);
  install_element (VIEW_NODE, &show_mpls_table_lsp_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_igmp_query_max_response_time_cmd_vtysh);
  install_element (VIEW_NODE, &clear_isis_neighbor_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_spf_ietf_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_as_path_access_list_all_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (ENABLE_NODE, &config_write_integrated_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_authentication_key_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_remote_accept_from_acl_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_default_show_hostname_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_refresh_timer_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_maxpaths_ibgp_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_nssa_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_lcommunity_list_name_standard1_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_network_mask_natural_cmd_vtysh);
  install_element (INTERFACE_NODE, &tunnel_protection_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_nht_default_route_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_neighbor_ipv4_targeted_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_rtevents_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_community_delete_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_ge_le_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_interface_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (BGP_NODE, &bgp_confederation_identifier_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_igmp_packets_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_disable_connected_route_check_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_tag_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_override_capability_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_network_mask_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_default_subgroup_pkt_queue_max_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_nvegroup_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_log_adjacency_changes_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_ssm_prefix_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_broadcast_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_nht_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_le_cmd_vtysh);
  install_element (BGP_EVPN_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_ava_bw_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_distance_source_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_transport_address_ipv6_cmd_vtysh);
  install_element (OSPF_NODE, &capability_opaque_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_ospf_metric_rmap_cmd_vtysh);
  install_element (RIP_NODE, &rip_default_metric_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_igmp_interface_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vrf_all_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_msdp_packets_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_update_direct_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_pref_label_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_no_member_interface_ifname_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_send_version_1_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_vnc_all_cmd_vtysh);
  install_element (LDP_NODE, &ldp_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_prefix_first_match_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_interface_ifname_cmd_vtysh);
  install_element (RMAP_NODE, &set_ip_nexthop_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_instance_event_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_pim_drprio_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_rd_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_update_direct_peer_cmd_vtysh);
  install_element (RIP_NODE, &rip_offset_list_ifname_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_update_direct_peer_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_set_route_map_delay_timer_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_default_originate_rmap_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (RMAP_NODE, &set_aspath_prepend_lastas_cmd_vtysh);
  install_element (ENABLE_NODE, &show_interface_desc_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_allow_ecmp_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_messages_sent_all_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_targeted_hello_holdtime_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_pw_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_tunnel_source_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_ebgp_multihop_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_lcommunity_list_expanded_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_mpls_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_assert_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_ism_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_mpls_te_inter_as_area_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_snp_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_peer_group_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_adj_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_spfevents_cmd_vtysh);
  install_element (BGP_NODE, &bgp_default_show_hostname_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_peer_groups_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_maxpaths_ibgp_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_rip_zebra_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_advertise_explicit_null_for_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_adj_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_protocol_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_interval_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_un_vn_prefix_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_int_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_neighbor_cmd_vtysh);
  install_element (ENABLE_NODE, &show_interface_desc_vrf_all_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_lsa_hex_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_igmp_join_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_as_path_cmd_vtysh);
  install_element (CONFIG_NODE, &no_service_advanced_vty_cmd_vtysh);
  install_element (OSPF_NODE, &no_pce_domain_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_remote_accept_for_acl_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_self_originated_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_updgrps_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_network_id_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_neighbor_received_prefix_filter_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_neighbor_events_cmd_vtysh);
  install_element (RMAP_NODE, &set_origin_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_bfd_type_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_split_horizon_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_nht_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_nht_default_route_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_state_cmd_vtysh);
  install_element (RMAP_NODE, &set_ip_nexthop_peer_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_router_info_pce_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_mpls_te_router_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_passive_interface_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_zebra_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_log_adjacency_changes_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_delay_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_discovery_transport_address_ipv4_cmd_vtysh);
  install_element (BGP_NODE, &bgp_graceful_restart_restart_time_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_mroute_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_mask_backdoor_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_messages_recv_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_lifetime_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ipv6_ospf6_interface_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_table_map_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_log_adjacency_changes_cmd_vtysh);
  install_element (BGP_NODE, &bgp_distance_source_access_list_cmd_vtysh);
  install_element (RMAP_NODE, &set_originator_id_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_session_holdtime_session_time_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_lcommunity_delete_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_description_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_mask_tag_distance_label_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_refresh_timer_val_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ipv6_next_hop_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_prefix_first_match_cmd_vtysh);
  install_element (RMAP_NODE, &set_community_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_interface_arg_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_ipv6_aggregate_address_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_flooding_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_infinite_day_month_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_igmp_groups_retransmissions_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_ifname_pref_tag_label_cmd_vtysh);
  install_element (CONFIG_NODE, &no_route_map_all_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_ifname_pref_tag_label_cmd_vtysh);
  install_element (BGP_NODE, &vnc_no_export_nvegroup_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_redist_bgpdirect_routemap_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_no_vc_type_pwtype_cmd_vtysh);
  install_element (OSPF6_NODE, &no_area_export_list_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_border_routers_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_mask_label_cmd_vtysh);
  install_element (LDP_NODE, &ldp_neighbor_ipv4_ttl_security_hops_hops_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_address_preference_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_month_day_day_month_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_reg_flags_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_protocol_nht_rmap_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_export_no_prefixlist_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_address_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_any_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_interface_json_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_client_to_client_reflection_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_activate_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_neighbor_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_password_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_binding_detail_json_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_ifmtu_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_nht_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_address_prefix_list_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_shutdown_msg_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ripng_zebra_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_nssa_translate_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_exact_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_interface_ifname_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_interval_cmd_vtysh);
  install_element (CONFIG_NODE, &dump_bgp_all_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_discovery_json_cmd_vtysh);
  install_element (VIEW_NODE, &show_dmvpn_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_interface_v6only_config_remote_as_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_always_compare_med_cmd_vtysh);
  install_element (VIEW_NODE, &show_mpls_table_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_rt_both_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_summary_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_network_area_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_igmp_trace_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_ospf6_network_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_tag_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_as4_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_err_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_bfd_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ripng_packet_direct_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_route_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_aspath_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_bandwidth_if_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_export_no_routemap_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_bgp_dampening_address_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_maxpaths_ibgp_cluster_cmd_vtysh);
  install_element (BGP_NODE, &bgp_wpkt_quanta_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_nht_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_trace_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (CONFIG_NODE, &no_vrf_debug_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_mroute_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_redistribute_source_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_spfstats_cmd_vtysh);
  install_element (OSPF_NODE, &pce_domain_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_neighbor_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_month_day_day_month_cmd_vtysh);
  install_element (BGP_NODE, &bgp_cluster_id_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_zebra_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_name_seq_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_bestpath_med_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_aspath_exclude_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_spftrigg_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_abr_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_update_prefix_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_redistribute_type_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_nexthop_cmd_vtysh);
  install_element (ISIS_NODE, &is_type_cmd_vtysh);
  install_element (BGP_NODE, &vnc_advertise_un_method_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_day_month_day_month_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_packets_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_register_suppress_cmd_vtysh);
  install_element (CONFIG_NODE, &service_advanced_vty_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_prefix_cmd_vtysh);
  install_element (RMAP_NODE, &match_source_protocol_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_update_prefix_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_nexthop_self_force_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_keepalive_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_abr_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_advertise_prefix_list_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_log_adjacency_changes_detail_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_nve_vn_un_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_homeagent_preference_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_l2vpn_atom_vc_json_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_suppress_ra_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_bridge_ifname_cmd_vtysh);
  install_element (BGP_NODE, &bgp_client_to_client_reflection_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_filter_list_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_prefix_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_address_label_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_delay_var_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_nexthop_prefix_len_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_default_originate_rmap_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_zebra_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_rpf_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_any_mask_cmd_vtysh);
  install_element (ISIS_NODE, &spf_interval_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_rib_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_explicit_null_for_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_keepalive_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &aggregate_address_mask_cmd_vtysh);
  install_element (RMAP_NODE, &match_aspath_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_zebra_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_vnc_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_events_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_inter_as_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_port_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_aggregate_address_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_mpls_ldp_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_peer_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_lsp_gen_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_no_control_word_cword_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_update_groups_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_as4_cmd_vtysh);
  install_element (CONFIG_NODE, &bgp_set_route_map_delay_timer_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_authentication_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_activate_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_lcommunity_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_supernets_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_interface_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_le_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_spt_switchover_infinity_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_spf_database_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_le_ge_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_msdp_sa_detail_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_metric_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_area_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_igmp_query_interval_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_multicast_routing_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_remote_accept_from_acl_for_acl_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_csum_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_maxpaths_ibgp_cmd_vtysh);
  install_element (RMAP_NODE, &match_ecommunity_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_lcommunity_list_arg_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_zebra_prefix_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_vpnv4_network_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_binding_json_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (BGP_NODE, &bgp_graceful_restart_stalepath_time_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (BGP_NODE, &bgp_update_delay_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_mpls_te_router_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_cost_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_bestpath_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_ospf_rmap_metric_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_for_acl_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_ssm_prefix_list_name_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_reachable_time_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ripng_packet_direct_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_unregister_vn_un_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_description_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_zebra_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_remove_private_as_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_disable_connected_check_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_pim_packets_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_src_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_keepalive_peer_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_addr_cmd_vtysh);
  install_element (RMAP_NODE, &set_src_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_ism_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_router_info_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_offset_list_ifname_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_lsa_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_remote_accept_from_acl_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_interval_l1_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_lupd_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_responses_pfx_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_kernel_msgdump_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_route_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_snp_cmd_vtysh);
  install_element (BGP_NODE, &bgp_bestpath_aspath_ignore_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_te_cmd_vtysh);
  install_element (VIEW_NODE, &show_version_ospf6_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_distance_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_close_rfd_cmd_vtysh);
  install_element (BGP_NODE, &no_aggregate_address_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (OSPF6_NODE, &no_ospf6_log_adjacency_changes_detail_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_network_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_timers_lsa_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_cmd_vtysh);
  install_element (CONFIG_NODE, &no_config_table_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_redistribute_type_metric_routemap_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_export_prefixlist_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_authentication_message_digest_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_metric_rmap_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_keepalive_peer_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_passive_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_fpm_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_le_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_database_max_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_send_community_type_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ipv6_nexthop_peer_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_spf_time_cmd_vtysh);
  install_element (RIP_NODE, &no_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_flags_cmd_vtysh);
  install_element (BGP_NODE, &bgp_damp_unset_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_listen_range_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_ttl_security_disable_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_redistribute_ipv6_metric_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_timers_throttle_spf_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_authentication_mode_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_receive_version_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_distance_source_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_unsuppress_map_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_route_reflector_client_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_interval_l2_cmd_vtysh);
  install_element (BGP_NODE, &bgp_bestpath_aspath_multipath_relax_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_default_originate_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_psnp_interval_l2_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_packets_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_nssa_translate_no_summary_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_padding_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_ripng_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_mtu_ignore_addr_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_msdp_mesh_group_member_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ripng_events_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_graceful_restart_preserve_fw_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_bgp_table_map_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_msdp_packets_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_show_nves_vn_un_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_access_list_remark_comment_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_keepalive_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_irdp_shutdown_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_csum_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_explicit_null_for_acl_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_advertise_explicit_null_for_acl_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_vlink_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_vlink_cmd_vtysh); /* djd */
  install_element (CONFIG_NODE, &no_debug_msdp_events_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_tag_distance_label_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_brouter_area_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_network_route_map_cmd_vtysh);
  install_element (LDP_IPV6_IFACE_NODE, &ldp_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_packets_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_address_family_discovery_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_pim_ssm_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_life_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_route_map_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_igmp_interfaces_cmd_vtysh);
  install_element (OSPF6_NODE, &area_import_list_cmd_vtysh);
  install_element (ISIS_NODE, &isis_redistribute_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_pim_hello_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_use_bw_cmd_vtysh);
  install_element (BGP_NODE, &bgp_update_delay_establish_wait_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (LDP_NODE, &ldp_dual_stack_transport_connection_prefer_ipv4_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_priority_cmd_vtysh);
  install_element (INTERFACE_NODE, &csnp_interval_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ns_netns_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_remove_private_as_all_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_redistribute_source_cmd_vtysh);
  install_element (RMAP_NODE, &ospf6_routemap_set_metric_type_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_default_cost_cmd_vtysh);
  install_element (ISIS_NODE, &area_passwd_md5_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (RMAP_NODE, &no_rmap_continue_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_distance_source_access_list_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_neighbor_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ipv6_nexthop_prefer_global_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_prefix_longer_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_rib_detailed_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_graceful_restart_restart_time_cmd_vtysh);
  install_element (VTY_NODE, &exec_timeout_min_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_rmap_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_rip_send_version_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_brouter_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_remove_private_as_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_ecmp_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_origin_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_id_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_igmp_trace_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_all_tags_cmd_vtysh);
  install_element (BGP_EVPN_NODE, &evpnrt5_network_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_default_originate_rmap_cmd_vtysh);
  install_element (VTY_NODE, &vty_access_class_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_rt_both_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_host_any_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &key_string_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_zebra_kernel_msgdump_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_allocate_for_acl_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_address_label_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_admin_grp_cmd_vtysh);
  install_element (BGP_NODE, &bgp_distance_source_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_query_vn_un_done_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_ebgp_multihop_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_authentication_addr_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_default_originate_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_extcommunity_list_name_expanded_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_discovery_hello_dir_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf6_flooding_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_maxbw_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_rip_events_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_packetdump_recv_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_bfd_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_igmp_join_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_bgp_prefix_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_nve_vn_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_summary_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_default_originate_cmd_vtysh);
  install_element (INTERFACE_NODE, &multicast_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_shutdown_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_default_metric_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_cost_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_discovery_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_map_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_zebra_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_igmp_query_max_response_time_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_route_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_no_neighbor_address_addr_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_discovery_json_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_show_nves_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_cost_life_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_activate_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_default_metric_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_network_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_bgp_dampening_address_mask_cmd_vtysh);
  install_element (CONFIG_NODE, &no_fpm_remote_ip_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_igmp_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_neighbor_poll_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_neighbor_one_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_unrsv_bw_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_interface_json_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_query_vn_un_l2o_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (ENABLE_NODE, &show_debugging_bgp_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_ge_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_name_seq_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_detail_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_redist_bgpdirect_no_prefixlist_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ipv6_prefix_list_name_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_summary_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_event_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (RIPNG_NODE, &no_distribute_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_instance_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_maximum_prefix_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_redist_bgpdirect_prefixlist_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_rfc1583_flag_cmd_vtysh);
  install_element (RIP_NODE, &rip_redistribute_type_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_nht_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (RIP_NODE, &rip_distance_source_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_authentication_key_authkey_addr_cmd_vtysh);
  install_element (OSPF_NODE, &no_pce_neighbor_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_rip_receive_version_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_rip_packet_direct_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_pref_tag_label_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_route_map_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_forwarding_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_summary_cmd_vtysh);
/* SYSTEM MODIFIED by chenc for bug 49502, 2018-10-16 */
/*write ospf fib num to file*/
#ifdef _CENTEC_
  install_element (VIEW_NODE, &show_ip_route_summary_write_cmd_vtysh);
#endif
  install_element (ENABLE_NODE, &debug_zebra_nht_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_distribute_list_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_remove_private_as_replace_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_router_preference_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vrf_prefix_rd_label_pref_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_for_acl_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_router_id_old_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_shortcut_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_msdp_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_le_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_explicit_null_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_label_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_debug_mpls_ldp_messages_recv_all_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_abr_type_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_dont_capability_negotiate_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_instance_nssa_cmd_vtysh);
  install_element (CONFIG_NODE, &no_bgp_multiple_instance_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_prefix_all_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_bgp_vnc_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_packet_dump_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_pim_sm_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_interface_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_te_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_redistribute_type_routemap_cmd_vtysh);
  install_element (RIP_NODE, &rip_route_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_targeted_hello_interval_disc_time_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_capability_enhe_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_network_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_instance_zebra_cmd_vtysh);
  install_element (OSPF_NODE, &pce_path_scope_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_forwarding_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_events_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_detail_name_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_extcommunity_list_arg_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_extcommunity_list_expanded_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_bfd_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_activate_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_pim_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_rpf_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_pim_hello_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_maximum_prefix_threshold_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_addr_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_weight_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_igmp_query_max_response_time_dsec_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_fpm_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_protocol_nht_rmap_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_igmp_sources_retransmissions_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_igmp_packets_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_irdp_debug_disable_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_log_adjacency_changes_detail_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_dual_stack_transport_connection_prefer_ipv4_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_large_community_list_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_redistribute_ipv4_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_address_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_lcommunity_list_standard_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_redistribute_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_spf_process_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_prefix_life_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_le_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_prefix_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_mroute_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_day_month_month_day_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_instance_nsm_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_as_override_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_message_digest_key_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_attr_info_cmd_vtysh);
  install_element (ISIS_NODE, &no_isis_mpls_te_inter_as_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_default_information_originate_cmd_vtysh);
  install_element (RMAP_NODE, &match_peer_local_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_mask_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_export_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_neighbor_events_peer_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_shutdown_if_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_capabilities_json_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_flags_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_msdp_packets_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_lcommunity_list_name_expanded_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_bestpath_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_route_distance_label_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_redistribute_type_metric_routemap_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_cmd_vtysh);
  install_element (ISIS_NODE, &no_is_type_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_ttl_security_disable_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_isis_spftrigg_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_local_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_prefix_list_name_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_pim_events_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_maximum_prefix_threshold_warning_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_community_list_expanded_all_cmd_vtysh);
  install_element (VTY_NODE, &no_vty_ipv6_access_class_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_response_omit_self_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_nd_homeagent_lifetime_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_lsp_sched_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_max_metric_router_lsa_startup_cmd_vtysh);
  install_element (ISIS_NODE, &no_net_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_mpls_te_link_cmd_vtysh);
  install_element (CONFIG_NODE, &no_nhrp_nflog_group_cmd_vtysh);
  install_element (ISIS_NODE, &no_spf_interval_l2_cmd_vtysh);
  install_element (BGP_VNC_L2_GROUP_NODE, &vnc_l2_group_lni_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_weight_cmd_vtysh);
  install_element (RMAP_NODE, &set_community_delete_cmd_vtysh);
  install_element (RIP_NODE, &rip_passive_interface_cmd_vtysh);
  install_element (BGP_VRF_POLICY_NODE, &vnc_vrf_policy_rt_export_cmd_vtysh);
  install_element (BGP_NODE, &no_auto_summary_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (RIP_NODE, &rip_distance_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_mask_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_rtevents_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_msdp_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_transmit_delay_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_mpls_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_event_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_network_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_interface_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_unsuppress_map_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_zebra_import_table_distance_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_mask_distance_label_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_timers_lsa_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_messages_sent_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_passive_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_router_id_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_redistribute_instance_source_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_mask_natural_backdoor_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_events_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_weight_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_any_any_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_mtu_cmd_vtysh);
  install_element (CONFIG_NODE, &mpls_transit_lsp_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_rt_import_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_as_path_access_list_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_advertise_interval_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_redistribute_ipv6_rmap_metric_cmd_vtysh);
  install_element (VIEW_NODE, &show_debugging_ripng_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_control_word_cword_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redist_bgpdirect_no_routemap_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_fast_external_failover_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_metric_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ipv6_address_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_remote_accept_from_acl_for_acl_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (RMAP_NODE, &set_ipv6_nexthop_prefer_global_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_pim_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_max_metric_router_lsa_shutdown_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_nexthop_self_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_mtu_mtu_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_community_list_arg_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_te_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_joinprune_time_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_remark_comment_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_asbr_cmd_vtysh);
  install_element (BGP_NODE, &bgp_log_neighbor_changes_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_vlink_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_network_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_event_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_interface_ifname_prefix_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redistribute_no_nvegroup_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_to_acl_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_stub_no_summary_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &vpnv4_network_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_all_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_allow_martians_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_rmap_cmd_vtysh);
  install_element (ISIS_NODE, &no_spf_interval_l1_cmd_vtysh);
  install_element (VIEW_NODE, &show_debugging_zebra_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_distance_source_access_list_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ssmpingd_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_packetdump_send_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_evpn_rd_overlay_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_default_ipv4_unicast_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_send_community_type_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_forwarding_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_neighbor_ipv4_password_word_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_write_multiplier_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_rpf_addr_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_any_any_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_label_local_allocate_for_acl_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_description_cmd_vtysh);
  install_element (RMAP_NODE, &match_ip_route_source_prefix_list_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_rd_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_update_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_summary_prefix_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_mroute_detail_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_distance_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_route_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_instance_zebra_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_network_mask_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_updgrps_adj_s_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_l2vpn_evpn_all_neighbor_advertised_routes_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_retransmit_interval_addr_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_lsa_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_access_list_all_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_nhrp_flags_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &no_key_string_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_bestpath_prefix_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_address_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_capability_dynamic_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_network_mask_natural_cmd_vtysh);
  install_element (ENABLE_NODE, &show_ip_access_list_name_cmd_vtysh);
  install_element (RMAP_NODE, &ospf6_routemap_set_forwarding_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_interface_cmd_vtysh);
  install_element (BGP_NODE, &vnc_nve_export_no_prefixlist_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_rib_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_igmp_version_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_nsm_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_bfd_param_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_extended_host_any_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_group_type_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_dead_interval_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_bestpath_aspath_ignore_cmd_vtysh);
  install_element (OSPF6_NODE, &no_area_range_cmd_vtysh);
  install_element (OSPF6_NODE, &ospf6_timers_lsa_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_igmp_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_msdp_peer_detail_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_offset_list_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_maxpaths_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_router_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_timers_min_ls_arrival_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_detail_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxpaths_ibgp_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_standard_any_cmd_vtysh);
  install_element (KEYCHAIN_NODE, &no_key_chain_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_network_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_neighbor_detail_json_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_network_area_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_zebra_prefix_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_network_mask_cmd_vtysh);
  install_element (RMAP_NODE, &match_ipv6_address_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_dead_interval_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_authentication_addr_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (RIP_NODE, &rip_network_cmd_vtysh);
  install_element (ISIS_NODE, &max_lsp_lifetime_cmd_vtysh);
  install_element (BGP_NODE, &bgp_damp_set_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_lcommunity_list_standard_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_rmap_metric_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf6_brouter_area_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_solo_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_nexthop_local_unchanged_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_le_ge_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_maxpaths_ibgp_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_hello_interval_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_regexp_cmd_vtysh);
  install_element (ISIS_NODE, &no_spf_interval_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_igmp_version_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_detail_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_instance_lsa_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_allowas_in_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_nexthop_self_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_packet_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_timers_connect_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ssmpingd_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_secondary_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_bgp_keepalive_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (INTERFACE_NODE, &ospf_hello_interval_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_adv_router_linkstate_id_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_events_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_instance_nsm_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_extcommunity_list_standard_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_hello_multiplier_l1_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_neighbor_ipv4_ttl_security_disable_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_default_information_originate_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_maximum_prefix_restart_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_bfd_type_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_metric_l1_cmd_vtysh);
  install_element (RMAP_NODE, &set_ipv6_nexthop_local_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_redistribute_type_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_ecommunity_rt_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_tag_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_bfd_param_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxpaths_ibgp_cluster_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_weight_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_message_digest_key_cmd_vtysh);
  install_element (BGP_NODE, &bgp_router_id_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_neighbor_ipv4_targeted_cmd_vtysh);
  install_element (RMAP_NODE, &rmap_continue_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_warning_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ripng_packet_direct_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_vrfs_cmd_vtysh);
  install_element (OSPF_NODE, &no_pce_address_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_solo_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_distance_source_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_neighbor_filter_list_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_update_direct_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_network_mask_route_map_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_mroute_source_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_addpath_tx_bestpath_per_as_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_spf_time_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_rib_detailed_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ssmpingd_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_passive_interface_cmd_vtysh);
  install_element (RMAP_NODE, &ospf6_routemap_no_set_metric_type_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_interface_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_route_type_detail_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &aggregate_address_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_community_info_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_summary_prefix_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_flags_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_neighbor_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_neighbor_distribute_list_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_distance_cmd_vtysh);
  install_element (INTERFACE_NODE, &shutdown_if_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_no_ip_pim_sm_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_address_family_ipv6_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_neighbor_route_reflector_client_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_zebra_events_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_ipv6_bgp_distance_source_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (INTERFACE_NODE, &psnp_interval_l1_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_neighbor_cmd_vtysh);
  install_element (BGP_VNC_DEFAULTS_NODE, &vnc_defaults_responselifetime_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_distance_cmd_vtysh);
  install_element (RMAP_NODE, &rmap_onmatch_next_cmd_vtysh);
  install_element (CONFIG_NODE, &access_list_extended_host_host_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_mroute_cmd_vtysh);
  install_element (BGP_VNC_L2_GROUP_NODE, &vnc_l2_group_labels_cmd_vtysh);
  install_element (CONFIG_NODE, &log_commands_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_rip_packet_direct_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_default_originate_rmap_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_id_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_redistribute_instance_source_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_protocol_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_ge_le_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_lsp_gen_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_pim_packetdump_recv_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_export_list_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_ifname_tag_label_cmd_vtysh);
  install_element (BGP_NODE, &rfp_example_config_value_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_vpnv6_network_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_table_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_rp_prefix_list_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_community_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_nves_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_transport_address_ipv6_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_remote_accept_from_acl_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_id_router_cmd_vtysh);
  install_element (CONFIG_NODE, &no_access_list_standard_nomask_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (INTERFACE_NODE, &interface_ip_igmp_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &ipv6_bgp_distance_source_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_zebra_mpls_cmd_vtysh);
  install_element (CONFIG_NODE, &no_mpls_transit_lsp_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_prefix_list_name_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_default_metric_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_network_mask_natural_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_errors_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_access_list_remark_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf6_message_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_igmp_packets_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_distance_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_address_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_attr_unchanged_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_ge_le_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rip_events_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_as_override_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_max_metric_router_lsa_startup_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_spt_switchover_infinity_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_pim_packets_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_isis_events_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_allowas_in_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_remote_accept_from_acl_for_acl_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_isis_lupd_cmd_vtysh);
  install_element (OSPF_NODE, &no_write_multiplier_cmd_vtysh);
  install_element (CONFIG_NODE, &no_key_chain_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &no_bgp_distance_source_access_list_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_ospf_rmap_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_distance_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_send_community_cmd_vtysh);
  install_element (VIEW_NODE, &show_zebra_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_tag_label_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_pim_zebra_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &neighbor_activate_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_set_peer_group_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_no_debug_mpls_ldp_messages_recv_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_summary_name_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_network_cmd_vtysh);
  install_element (ENABLE_NODE, &show_ipv6_access_list_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_rfapi_close_vn_un_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &send_lifetime_day_month_month_day_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_ifname_pref_label_cmd_vtysh);
  install_element (CONFIG_NODE, &bgp_config_type_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_route_ifname_label_cmd_vtysh);
  install_element (CONFIG_NODE, &zebra_ptm_enable_cmd_vtysh);
  install_element (CONFIG_NODE, &no_dump_bgp_all_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &neighbor_addpath_tx_all_paths_cmd_vtysh);
  install_element (ENABLE_NODE, &vrf_debug_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_lcommunity_list_standard_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &tunnel_source_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_capability_orf_prefix_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &bgp_table_map_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_zebra_nht_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_nht_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_nexthop_self_force_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_instance_nssa_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_rip_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_maximum_prefix_threshold_restart_cmd_vtysh);
  install_element (CONFIG_NODE, &zebra_route_map_timer_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_prefix_longer_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_area_spf_tree_cmd_vtysh);
  install_element (CONFIG_NODE, &no_router_ospf_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_bgp_table_map_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_large_community_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_damp_set_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_isis_priority_l1_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_aggregate_address_mask_cmd_vtysh);
  install_element (BGP_NODE, &bgp_maxmed_admin_cmd_vtysh);
  install_element (LDP_PSEUDOWIRE_NODE, &ldp_no_pw_id_pwid_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_send_community_cmd_vtysh);
  install_element (RMAP_NODE, &no_set_originator_id_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_lsa_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_protocol_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_duration_day_month_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_route_pref_tag_label_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_no_l2rd_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_prefix_list_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_discovery_targeted_hello_accept_from_acl_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_route_cmd_vtysh);
  install_element (CONFIG_NODE, &router_id_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_default_originate_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_route_vrf_all_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &bgp_redistribute_ipv4_rmap_metric_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_pim_assert_winner_metric_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_igmp_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_msdp_mesh_group_member_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &ipv6_bgp_network_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_bgp_dampening_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_igmp_sources_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_label_local_advertise_explicit_null_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ospf_dead_interval_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vrf_prefix_rd_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ospf6_bfd_param_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_inter_as_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_interface_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_le_ge_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_mac_un_prefix_cmd_vtysh);
  install_element (RIPNG_NODE, &no_ripng_allow_ecmp_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ip_ospf_transmit_delay_addr_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_mroute_detail_cmd_vtysh);
  install_element (RIPNG_NODE, &ripng_redistribute_type_metric_cmd_vtysh);
  install_element (RIP_NODE, &rip_redistribute_type_routemap_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_route_match_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_mac_vni_life_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_distance_ospf_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_route_mask_label_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_listen_limit_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_route_vrf_all_summary_prefix_cmd_vtysh);
  install_element (RMAP_NODE, &set_lcommunity_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_damp_set_cmd_vtysh);
  install_element (ISIS_NODE, &no_isis_default_originate_cmd_vtysh);
  install_element (VIEW_NODE, &show_bgp_instance_all_ipv6_updgrps_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_isis_events_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_ebgp_multihop_ttl_cmd_vtysh);
  install_element (LDP_IPV6_IFACE_NODE, &ldp_no_discovery_hello_interval_disc_time_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_rr_allow_outbound_policy_cmd_vtysh);
  install_element (CONFIG_NODE, &ipv6_prefix_list_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_attr_unchanged_cmd_vtysh);
  install_element (ISIS_NODE, &dynamic_hostname_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_discovery_targeted_hello_accept_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_vnc_nve_un_vn_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &no_aggregate_address_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_bestpath_aspath_confed_cmd_vtysh);
  install_element (BGP_VNC_NVE_GROUP_NODE, &vnc_nve_group_prefix_cmd_vtysh);
  install_element (BGP_IPV4_NODE, &neighbor_activate_cmd_vtysh);
  install_element (VTY_NODE, &vty_login_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_bgp_neighbor_events_cmd_vtysh);
  install_element (BGP_NODE, &vnc_redist_bgpdirect_routemap_cmd_vtysh);
  install_element (LDP_L2VPN_NODE, &ldp_no_member_pseudowire_ifname_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_default_metric_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_sequence_number_cmd_vtysh);
  install_element (BGP_NODE, &bgp_redistribute_ipv4_metric_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_bgp_maxpaths_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_igmp_groups_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_area_authentication_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &neighbor_remove_private_as_all_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_ttl_security_disable_cmd_vtysh);
  install_element (ENABLE_NODE, &ldp_debug_mpls_ldp_event_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_distance_source_access_list_cmd_vtysh);
  install_element (INTERFACE_NODE, &isis_passwd_cmd_vtysh);
  install_element (CONFIG_NODE, &no_bgp_config_type_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_port_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_discovery_targeted_hello_interval_disc_time_cmd_vtysh);
  install_element (RMAP_NODE, &set_metric_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_community_list_cmd_vtysh);
  install_element (BGP_NODE, &no_bgp_default_local_preference_cmd_vtysh);
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_lifetime_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_redistribute_type_cmd_vtysh);
  install_element (INTERFACE_NODE, &ip_ospf_transmit_delay_addr_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_neighbors_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_nht_default_route_cmd_vtysh);
  install_element (CONFIG_NODE, &nhrp_nflog_group_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_access_list_exact_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_vpn_rd_cmd_vtysh);
  install_element (LDP_NODE, &ldp_no_address_family_ipv4_cmd_vtysh);
  install_element (OSPF_NODE, &no_pce_path_scope_cmd_vtysh);
  install_element (KEYCHAIN_KEY_NODE, &accept_lifetime_duration_month_day_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_route_server_client_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (RMAP_NODE, &no_match_ip_next_hop_cmd_vtysh);
  install_element (LDP_IPV4_NODE, &ldp_no_label_local_advertise_for_acl_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_area_stub_no_summary_cmd_vtysh);
  install_element (BGP_NODE, &bgp_network_backdoor_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_route_server_client_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (VIEW_NODE, &show_debugging_nhrp_cmd_vtysh);
  install_element (VIEW_NODE, &vnc_show_registrations_some_pfx_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_update_cmd_vtysh);
  install_element (LDP_IPV6_NODE, &ldp_no_label_local_allocate_for_acl_cmd_vtysh);
  install_element (VIEW_NODE, &show_isis_topology_cmd_vtysh);
  install_element (VIEW_NODE, &show_mpls_status_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &no_link_params_pkt_loss_cmd_vtysh);
  install_element (BGP_NODE, &vnc_nve_export_routemap_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &neighbor_remove_private_as_all_replace_as_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_mroute_cmd_vtysh);
  install_element (VIEW_NODE, &show_vnc_queries_pfx_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ipv6_protocol_cmd_vtysh);
  install_element (ENABLE_NODE, &add_vnc_prefix_cmd_vtysh);
  install_element (ISIS_NODE, &set_attached_bit_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_weight_cmd_vtysh);
  install_element (LINK_PARAMS_NODE, &link_params_delay_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_zebra_cmd_vtysh);
  install_element (ENABLE_NODE, &no_vrf_debug_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_forwarding_cmd_vtysh);
  install_element (INTERFACE_NODE, &ipv6_ripng_split_horizon_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_debug_mpls_ldp_messages_sent_all_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_network_id_cmd_vtysh);
  install_element (RIP_NODE, &no_rip_version_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_network_mask_natural_route_map_cmd_vtysh);
  install_element (BGP_IPV6M_NODE, &no_ipv6_bgp_network_cmd_vtysh);
  install_element (INTERFACE_NODE, &if_no_nhrp_flags_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_flooding_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_igmp_events_cmd_vtysh);
  install_element (OSPF_NODE, &no_ospf_router_id_cmd_vtysh);
  install_element (RMAP_NODE, &match_peer_cmd_vtysh);
  install_element (ENABLE_NODE, &debug_ospf_nssa_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_community_list_expanded_all_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_spt_switchover_infinity_plist_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf6_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_bgp_instance_all_nexthop_cmd_vtysh);
  install_element (OSPF_NODE, &ospf_neighbor_poll_interval_cmd_vtysh);
  install_element (CONFIG_NODE, &ldp_no_l2vpn_word_type_vpls_cmd_vtysh);
  install_element (CONFIG_NODE, &no_ip_pim_keep_alive_cmd_vtysh);
  install_element (VIEW_NODE, &show_ip_ospf_instance_database_cmd_vtysh);
  install_element (BGP_NODE, &neighbor_local_as_no_prepend_replace_as_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_rip_zebra_cmd_vtysh);
  install_element (ENABLE_NODE, &show_debugging_rip_cmd_vtysh);
  install_element (LDP_NODE, &ldp_discovery_hello_holdtime_disc_time_cmd_vtysh);
  install_element (BGP_VPNV4_NODE, &no_neighbor_soft_reconfiguration_cmd_vtysh);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_cmd_vtysh);
  install_element (VIEW_NODE, &ldp_show_mpls_ldp_discovery_detail_json_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_bgp_neighbor_events_cmd_vtysh);
  install_element (BGP_NODE, &bgp_listen_range_cmd_vtysh);
  install_element (BGP_IPV6_NODE, &no_neighbor_maximum_prefix_cmd_vtysh);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_id_router_cmd_vtysh);
  install_element (CONFIG_NODE, &ip_pim_keep_alive_cmd_vtysh);
  install_element (BGP_VPNV6_NODE, &no_neighbor_set_peer_group_cmd_vtysh);
  install_element (BGP_NODE, &no_neighbor_dont_capability_negotiate_cmd_vtysh);
  install_element (BGP_IPV4M_NODE, &bgp_network_route_map_cmd_vtysh);

/* modified by liwh for bug 45793, 2017-12-07 */
#ifdef _CENTEC_
  install_element (ENABLE_NODE, &debug_ospf_event_sub_cmd_vtysh);
  install_element (CONFIG_NODE, &debug_ospf_event_sub_cmd_vtysh);
  install_element (ENABLE_NODE, &no_debug_ospf_event_sub_cmd_vtysh);
  install_element (CONFIG_NODE, &no_debug_ospf_event_sub_cmd_vtysh);
#endif
/*liwh end */

/* modified by liwh for bug 44589, 2017-07-06 */
#ifdef _CENTEC_  
  install_element (ENABLE_NODE, &clear_ip_ospf_process_cmd_vtysh);
#endif
/* liwh end */

/* modified by liwh for bug 44589, 2017-07-06 */
#ifdef _CENTEC_  
  install_element (ENABLE_NODE, &reset_ip_ospf_process_cmd_vtysh);
#endif
/* liwh end */
}
