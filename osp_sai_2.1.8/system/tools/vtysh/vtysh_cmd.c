#include <zebra.h>
#include "command.h"
#include "vtysh.h"

char *ctc_cli_learning_aging_port_nhid_mapping_cmd_help[] = { "Learning module", "Port to nexthop mapping", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Logic port", "<0-8191>", "Forwarding ID", "<0-16383>", "Next hop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipv4_member_info_cmd_help[] = { "Show running system information", "IPMC module", "Members of a given multicast group", "Information, including gport, vlan, l3 type.", "Group address, class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "Do IP based L2 mcast", ((void *)0) };

char *ctc_cli_bpe_get_interlaken_en_cmd_help[] = { "Show running system information", "bpe module", "interlaken interface", ((void *)0) };

char *ctc_cli_oam_bfd_add_rmep_cmd_help[] = { "Remote MEP", "BFD MEP", "Add remote BFD MEP", "My Discriminator", "My Discriminator value, <0~4294967295>", "Your Discriminator", "Your Discriminator value, <0~4294967295>", "MEP is active or not", "BFD required min rx interval", "<1-1023> ms", "BFD remote state", "Admin down :0 ; Down: 1; Init : 2; Up :3", "BFD remote diag", "<0~31>", "BFD remote detect mult", "<1-15>", "IP BFD OAM option", "MPLS BFD OAM option", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", "MEP index allocated by system", "2~max MEP index", ((void *)0) };

char *ctc_cli_vlan_set_bridge_cmd_help[] = { "VLAN module", "<1-4095>", "If set, bridge is enable on the vlan", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_port_set_preamble_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port preamble", "Preamble value, humber:bytes(2~31); greatbelt:bytes(2~15)", ((void *)0) };

char *ctc_cli_parser_show_l3_l3flex_cmd_help[] = { "Show running system information", "Parser module", "Layer 3 header of packet", "Flexible config", ((void *)0) };

char *ctc_cli_vlan_add_vlan_mapping_default_entry_cmd_help[] = { "VLAN module", "Vlan mapping", "Add vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Mismatch action", "Discard packet", "Normal forwarding", "Redirect to CPU", "Service queue id", "Service id", "Ingress service queue", "Egress service queue", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", ((void *)0) };

char *ctc_cli_chip_read_i2c_cmd_help[] = { "chip module", "read operate", "i2c device", "device address", "address value", "device bit map", "bit map value", "offset", "offset value", "length", "length value", ((void *)0) };

char *ctc_cli_port_allocate_internal_port_cmd_help[] = { "Port module", "Allocate", "Internal port", "Global chip id", "<0-29>", "Type", "Iloop", "Eloop", "Discard", "Fwd", "Port module", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_ipmc_remove_ipv6_group_cmd_help[] = { "IPMC module", "IPv6", "Remove", "IP multicast group", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route", "VRF id of route <0-8191>", "Do IP based L2 mcast", ((void *)0) };

char *ctc_cli_chip_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Chip Module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_common_parity_error_show_mapping_memory_cmd_help[] = { "Show running system information", "parity error mapping memory", "table", "0-293", "the index of table", "0-XXX", ((void *)0) };

char *ctc_cli_gb_dma_show_state_cmd_help[] = { "Show running system information", "Dma module", "State for global config", "Detail", ((void *)0) };

char *ctc_cli_qos_set_queue_pkt_len_cmd_help[] = { "QoS module", "QoS shaping", "Packet Length adjust", "Service id", "Value <1-0xFFFF", "Global port", "<0-0x1DFF>", "Queue id", "<0-7>", "Cpu reason", "Cpu reason id", "Packet adjust len", "value", ((void *)0) };

char *ctc_cli_learning_aging_set_aging_property_cmd_help[] = { "Aging module", "Fifo threshold", "Threshold value <0-15>", "Aging interval", "Aging interval(unit: seconds)", "Aging fifo scan", "Open aging fifo scan", "Close aging fifo scan", "Aging table", "MAC table", "SCL table", ((void *)0) };

char *ctc_cli_qos_set_port_group_class_priority_cmd_help[] = { "QoS module", "QoS schedule", "Queue class propagation to Priority", "Service identification", "<0-0xFFFF>", "Global port", "<0-0x1DFF>", "Queue class", "Value <0-7>", "Priority", "Value <0-3>", ((void *)0) };

char *ctc_cli_parser_l2_set_vlan_pas_num_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Ethernet l2", "Vlan parser num", "Vlan num <0-2>", ((void *)0) };

char *show_cli_drv_write_reg_or_tbl_field_cmd_help[] = { "Write table or register", "Table name", "Table name value example DsMac", "Table index", "Table index value <0-0xFFFFFFFF>", "Field id", "Field id range <0-0xFF>", "Field value", "Value <0-0xFFFFFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_show_error_desc_cmd_help[] = { "Show running system information", "Error description", "<-30000 ~ -10000>", ((void *)0) };

char *ctc_cli_sync_ether_show_cfg_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "SyncE clock", "SyncE clock ID", ((void *)0) };

char *ctc_cli_stats_clear_log_cmd_help[] = { "Clear functions", "Statistics", "Global port", "<0-0x1DFF>", "ingress direction", "egress direction", "log statis", ((void *)0) };

char *ctc_cli_qos_set_policer_stats_enable_cmd_help[] = { "QoS module", "QoS policer", "QoS statstics", "Globally enable QoS policer statistics", "Globally disable QoS policer statistics", ((void *)0) };

char *ctc_cli_l2mcast_remove_addr_cmd_help[] = { "Layer 2 module", "Multicast", "Remove operation", "Multicast group", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Forwarding ID", "<0-16383>", ((void *)0) };

char *ctc_cli_stacking_debug_on_cmd_help[] = { "Debugging functions", "stacking", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_Parser_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show Parser statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_qos_set_cpu_reason_gourp_mode_cmd_help[] = { "QoS module", "Cpu group", "Excption Reason group", "<0-15>", "Fwd to cpu group", "Destination to", "CPU", "DMA", ((void *)0) };

char *ctc_cli_dbg_tool_scan_gephy_sp_dis_cmd_help[] = { "Scan cmd", "Set gephy special scan disable cmd", "Disable functions", "<0-1>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Enable debugging information", ((void *)0) };

char *ctc_cli_dma_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "DMA Module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_qos_create_policer_cmd_help[] = { "QoS module", "QoS policer", "Attach policer", "Global port", "<0-0x1DFF>", "Ingress direction", "Egress direction", "QoS flow policer", "<1-0xFFFF>", "Service policer", "Service id <1-0xFFFF>", "Policer mode", "RFC2697, srTCM", "RFC2698, trTCM", "RFC4115, enhaned trTCM", "MEF 10.1,bandwidth profile", "Color blind mode, default policing mode", "Color aware mode", "Config CIR (Commit Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", "Config CBS (Commit Burst Size)", "<0-1024>, burst size in Kbits (1000), default is 64", "Config PIR (Peak Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", "Config EIR (Excess Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", "Config PBS (Peak Burst Size)", "<0-1024>, burst size in Kbits (1000), default is 64", "Config EBS (Excess Burst Size)", "<0-1024>, burst size in Kbits (1000), default is 64", "Couple flag enable, only for BWP", "Drop color config", "Red, default drop color", "Yellow", "Use packet length from layer 3 header for metering", "Stats enable", "HBWP enable, only support service policer on GB", "scheulde mode", "Strict priority", "Wdrr, GB only support cos 0 and cos 1, other is SP", "Weight", "Weight value<0-0x3FF>, GB only support cos 1 weight, cos 0 is (0x3FF - Weight)", "Cos index", "Value <0-3>", "Cir max", "Config EIR (Excess Information Rate)", "Pir max", "Config EIR (Excess Information Rate)", "Share flag enable", "Coupling total disable", ((void *)0) };

char *ctc_cli_ipuc_show_ipv6_info_cmd_help[] = { "IPUC module", "IPV6", "show ipuc ipv6 route", "VRF id of route <0-8191>", "IPv6 address in X:X::X:X format", "The length of Ipv6 mask <0~128>", "show the detail of this entry", ((void *)0) };

char *ctc_cli_dbg_tool_access_mode_cmd_help[] = { "Set access mode cmd", "PCI mode", "I2C mode", ((void *)0) };

char *ctc_cli_ipuc_debug_on_cmd_help[] = { "Debugging functions", "IPUC module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *humber_cli_aps_remvoe_aps_bridge_group_cmd_help[] = { "Aps Module", "Remove", "Aps bridge", "Aps bridge group", "<0-2047>", ((void *)0) };

char *ctc_cli_common_error_debug_cmd_help[] = { "Debugging functions", "Error return", "ON", "OFF", ((void *)0) };

char *ctc_cli_port_set_loopback_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port loopback to", "Dst port, if dst port equal src port, packet will loopback to self", "<0-0x1DFF>", "Enable functions", "Disable functions", "Swap-mac enable", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_enable_cmd_help[] = { "Local MEP", "Enable local MEP on OAM channel", "Disable local MEP on OAM channel", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_bpe_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "bpe module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_dbg_tool_read_gephy_cmd_help[] = { "Read cmd", "GEPHY cmd", "Mdio bus", "<0-1>", "Phy address", "<0-0x1F>", "Reg address", "<0-0x1F>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_parser_show_global_hash_type_cmd_help[] = { "Show running system information", "Parser module", "Layer2 3 4", "Hash type", ((void *)0) };

char *ctc_cli_parser_l4_set_flex_cmd_help[] = { "Parser module", "Layer 4 header of packet", "Flexible operation", "Offset to l4 flex dest port", "OFFSET <0x0 - 0x1E>", "Layer4 minum length", "Minum length value <0x0 - 0x1F>", ((void *)0) };

char *ctc_cli_gb_aps_show_bridge_cmd_help[] = { "Show running system information", "Aps Module", "APS Bridge", "APS bridge group", "<0-2047>", ((void *)0) };

char *ctc_cli_oam_get_mep_stats_cmd_help[] = { "Local MEP", "local MEP on OAM stats information", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_gb_drvio_clear_cache_cmd_help[] = { "Driver IO", "Clear", "Cache", "Statistics", "Cache and Statistics", ((void *)0) };

char *ctc_cli_show_port_cpu_mac_en_cmd_help[] = { "Show running system information", "Port module", "cpu mac", "Enable", ((void *)0) };

char *ctc_cli_mpls_remove_ilm_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Incoming label mapping", "Remove ilm(inluding l3vpn-vc, vpls-vc and vpws-vc)", "Mpls label space", "Label space id, <0-255>, space 0 is platform space", "Mpls label", "Label, <0-1048575>", "Nexthop id, humber must set", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_parser_show_linkagg_hash_cmd_help[] = { "Show running system information", "Parser module", "Linkagg hash field", "Layer2 ethernet", "Add vlan into linkagg hash", "Add cos into linkagg hash", "Add ethertype into linkagg hash", "Add double vlan into linkagg hash", "Add macsa into linkagg hash", "Add macda into linkagg hash", "Layer3 ip", "Use ip hash", "Add l3protocol into linkagg hash", "Add ipsa into linkagg hash", "Add ipda into linkagg hash", "Layer4", "Use layer4 hash", "Add src port into linkagg hash", "Add dst port into linkagg hash", "Add gre key into linkagg hash", "Provider backbone", "Add bvlan into linkagg hash", "Add bcos into linkagg hash", "Add bmacsa into linkagg hash", "Add bmacda into linkagg hash", "Mpls", "Use mpls hash", "Add mpls l3protocol into linkagg hash", "Add mpls ipsa into linkagg hash", "Add mpls ipda into linkagg hash", "Fcoe", "Use fcoe hash", "Add sid into linkagg hash", "Add did into linkagg hash", "Trill", "Use trill hash", "Add ingress nickname into linkagg hash", "Add egress nickname into linkagg hash", ((void *)0) };

char *ctc_cli_ptp_set_tod_intf_tx_code_cmd_help[] = { "PTP module", "<0-1>", "Tod interface output time code config", "Class of messag", "<0-0xFF>", "Identify of this message", "<0-0xFF>", "Length of the payload of this message", "<0-0xFFFF>", "Leap seconds (GPS-UTC)", "<0-0xFF>", "1PPS status", "<0-4>", "1PPS accuracy", "<0-0xFF>", "Clock source", "<0-3>", "Clock source work status", "<0-0xFFFF>", "Monitor alarm", "<0-0xFFFF>", ((void *)0) };

char *ctc_cli_l3if_show_l3if_id_cmd_help[] = { "Show", "L3 interface module", "L3 interface type", "Physical interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan interface", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_port_set_speed_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port speed mode", "Ethernet port with 10M", "Fast Eth port with 100M", "GEth port with 1G", "GEth port with 2.5G", ((void *)0) };

char *ctc_cli_parser_l4_set_app_data_cmd_help[] = { "Parser module", "Layer 4 header of packet", "Application entry", "Add layer4 application", "Entry index", "Index Value <0 - 3>", "Dest port mask", "Dest port mask value <0x0 -0xFFFF>", "Dest port value", "Dest port value <0x0 -0xFFFF>", "Is tcp mask", "Is tcp mask value <0 - 1>", "Is tcp value", "Is tcp value value <0 - 1>", "Is udp mask", "Is udp mask value <0 - 1>", "Is udp value", "Is udp Value value <0 - 1>", ((void *)0) };

char *ctc_cli_mpls_show_ilm_cmd_help[] = { "Show running system information", "Multi-Protocol Label Switching (MPLS)", "Ilm entries", "Label space id, <0-255>, space 0 is platform space", "Label, <0-1048575>", ((void *)0) };

char *ctc_cli_chip_set_xgphy_scan_special_reg_cmd_help[] = { "chip module", "set operate", "auto scan operate", "special reg", "xg phy", "device type", "<1~5>", "phy register", "<0~31>", "enable", "bit control", "<0~15>", "interrupt", "interrupt enable", "interrupt disable", "scan disable", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_wrapback_cmd_help[] = { "Hss12g cmd", "Wrapback cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "HSS12G link num", "<0-8>, 8 means broadcast", "Tx wrapback", "Rx wrapback sync", "Rx wrapback async", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_security_mac_security_show_system_mac_limit_cmd_help[] = { "Show running system information", "Security module", "Mac limit", "System based mac limit", ((void *)0) };

char *ctc_cli_vlan_remove_mac_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Remove one entry", "Mac based vlan", "MAC SA address", "MAC address in HHHH.HHHH.HHHH format", "MAC DA address", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_dbg_tool_queue_depth_cmd_help[] = { "Show cmd", "bsr queue info", "queue depth info", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_cross_connect_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port cross connect", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "0xFFFFFFFF means disable port cross-connect", ((void *)0) };

char *ctc_cli_mirror_remove_session_cmd_help[] = { "Mirror module", "Mirror destination", "Remove mirror destination", "Configure a mirror session", "Session_id range <0-3>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", "Mirror received and transmitted traffic", "Port session", "Vlan session", "Acl session", "Acl priority id", "GB range <0-3>, HB range <0-1>", "Cpu session", ((void *)0) };

char *ctc_cli_chip_set_serdes_param_cmd_help[] = { "chip module", "set operate", "serdes", "serdes id<0~31>", "ffe parameter", "mother board type", "M4 material", "TR4 material", "Trace length", "0~4inch", "4~7inch", "7~10inch", "Local chip", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_check_bist_cmd_help[] = { "Debug cmd", "Check bist", "Check lpmtcam bist", "Check inttcam bist", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l2_get_nhid_by_logic_port_cmd_help[] = { "Show running system information", "Layer 2 module", "FDB table", "logic-port to nexthop mapping", "Condition", "Logic port", "Reference to global config", ((void *)0) };

char *ctc_cli_port_efm_lb_mode_enable_cmd_help[] = { "Port module", "<0-0x1DFF>", "EFM loop-back mode", "Enable EFM loop-back mode this port", "Redirect to cpu", "Index <0-15>", ((void *)0) };

char *ctc_cli_dbg_tool_read_tbl_reg_cmd_help[] = { "Read cmd", "Table or register information", "Table/register id name", "Table/register entry index", "Index num", "Table/register fileld id name", "Table/register field name(string)", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_show_mac_cmd_help[] = { "Show running system information", "Statistics", "Mac reception statistics", "Mac transmission statistics", "Global port", "<0-0x1DFF>", "statistics information from mode", "Read info from Asic", "Read info from Table", ((void *)0) };

char *ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "Classify pdu based on type", "Layer2 header protocol", "Entry index", "<0-15>", ((void *)0) };

char *ctc_cli_l2_show_fdb_cmd_help[] = { "Show running system information", "Layer 2 module", "FDB table", "FDB entries", "Query condition", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "MAC+Fid", "MAC address in HHHH.HHHH.HHHH format", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Logic port", "Reference to global config", "Forwarding ID", "<0-16383>", "Port+Fid", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "<0-16383>", "All condition", "Static fdb table", "All dynamic fdb table", "Local chip dynamic fdb table", "Static and dynamic", "File store fdb entry", "File name", ((void *)0) };

char *ctc_cli_gb_chip_agent_clear_cmd_help[] = { "Clear functions", "Chip Agent module", "Statistics", "Driver IO", "Packet IO", "Cache of Driver IO", ((void *)0) };

char *ctc_cli_qos_show_policer_stats_cmd_help[] = { "Show running system information", "QoS module", "QoS policer", "Statistics", "Global port", "<0-0x1DFF>", "Ingress direction", "Egress direction", "QoS flow policer", "<1-0xFFFF>", "Service policer", "Service id <1-0xFFFF>", "Hbwp policer", "Specify the cos index", "Value <0-3>", ((void *)0) };

char *ctc_cli_qos_set_reason_shp_base_pkt_cmd_help[] = { "QoS module", "QoS shaping", "Shape base on packet", "Globally enable ", "Globally disable", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_TcamCtlInt_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show TcamCtlInt statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_show_global_cfg_cmd_help[] = { "Show running system information", "Statistics", "Saturate when at max value", "StatsPtrFifo on hold", "Clear after read", "Ingress port", "Egress port", "Ingress global fwd", "Egress global fwd", "Forward", "Gmac", "Sgmac", "Xgmac", "Cpumac", ((void *)0) };

char *ctc_cli_port_set_default_vlan_cmd_help[] = { "Port module", "<0-0x1DFF>", "Default vlan tag", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_security_mac_security_set_vlan_learn_limit_cmd_help[] = { "Security module", "Mac limit", "Vlan based mac limit", "<1-4095>", "The action of mac limit", "Do nothing", "Forwarding packet", "Discard packet", "Redirect to cpu", ((void *)0) };

char *ctc_cli_stacking_set_header_cmd_help[] = { "Stacking", "Header", "Layer2 header", "Vlan tpid such as <0x8100>", "Value <0-0xFFFF>", "Cos", "Value <0-7>", "Ether type", "Value <0-0xFFFF>", "Mac da check enable when receive stacking header", "Ether type check enable when receive stacking header", "Layer3 header", "Ipv4 header", "IPv4 source address", "IPv4 source address in format of A.B.C.D", "Ipv6 header", "IPv6 source address", "IPv6 source address in format of X:X::X:X", "Ttl", "Value <0-255>", "Dscp", "Value <0-63>", "Ip protocol", "Value <0-255>", "Layer4 header", "Udp", "Udp source port", "Value <0-0xFFFF>", "Udp dest port", "Value <0-0xFFFF>", ((void *)0) };

char *ctc_cli_ptp_set_sync_intf_toggle_time_cmd_help[] = { "PTP module", "<0-1>", "Sync interface toggle time", "Seconds of offset", "<0-0xFFFFFFFF>", "Nanoseconds of offset", "<0-(10^9-1)>", "Nano nanoseconds of offset", "<0-(10^9-1)>", ((void *)0) };

char *ctc_cli_scl_destroy_group_cmd_help[] = { "SCL module", "Destroy SCL group", "SCL group id", "<0~0xFFFF0000>", ((void *)0) };

char *ctc_cli_nh_add_remove_nh_member_cmd_help[] = { "Nexthop module", "Mcast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Add operation", "Remove operation", "The nexthop member of multicast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Port check enable", ((void *)0) };

char *ctc_cli_l3if_create_or_destroy_l3_sub_if_cmd_help[] = { "L3 interface module", "Create l3 interface", "Destroy l3 interface", "L3if ID", "<0-1022>", "L3 interface type", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_gb_chip_agent_debug_io_on_cmd_help[] = { "Debugging functions", "Chip Agent module", "Driver IO", "Socket", "Encode and Decode", "Port proxy", "All", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_QuadPcs_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show QuadPcs statistic", "Index", "<0-5>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_nh_dump_mpls_tunnel_cmd_help[] = { "Show running system information", "Nexthop module", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "Display detail information", ((void *)0) };

char *ctc_cli_ptp_show_captured_ts_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Timestamp captured by local clock", "Captured type", "Ts captured by mac tx", "Local port", "<0-0xFF>", "Sequence ID of the ts", "<0-3>", "Ts captured by syncPulse rx", "Ts captured by ToD 1PPS rx", "Sequence ID of the ts", "<0-15>", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_Ds_TCAM_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show Ds&TCAM statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_scl_add_mac_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0~0xFFFF0000>", "SCL entry id", "<0~0x7FFFFFFF>", "SCL entry priority", "<0~0xFFFFFFFF> :0 is lowest", "Mac tcam entry", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_mirror_rspan_escape_mac_cmd_help[] = { "Mirror module", "Remote mirror", "Discard some special mirrored packet", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Mac1 mask", "MAC mask in HHHH.HHHH.HHHH format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Mac2 mask", "MAC mask in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_l2_set_nhid_by_logic_port_cmd_help[] = { "Layer 2 module", "FDB table", "Set logic-port to nexthop mapping", "Logic port", "Reference to global config", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Next hop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_gb_ipuc_show_ipv4_cmd_help[] = { "Show running system information", "IPUC module", ((void *)0) };

char *ctc_cli_port_set_internal_port_cmd_help[] = { "Port module", "Set", "Internal port", "<64-255>", "Global chip id", "<0-29>", "Type", "Iloop", "Eloop", "Discard", "Fwd", "Port module", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_port_set_system_port_mac_prefix_cmd_help[] = { "System attribution", "Set", "Port MAC", "Port Prefix MAC 0", "MAC address in HHHH.HHHH.HHHH format", "Port Prefix MAC 1", "MAC address in HHHH.HHHH.HHHH format, high 32bits must same as port mac prefix 0", ((void *)0) };

char *ctc_cli_dbg_tool_cpu_access_static_ram_file_cmd_help[] = { "Static ram access cmd", "Static tbl ram cfg file", "Static tbl ram cfg file name", "Repeat time", "Repeat time value", "Repeat mode", "0: parallel 1:serial", "Operation mode ", "0:none 1:RW 2:RO", "Access entry type", "1: all entry; 0: entry 0 & max -1", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_linkagg_show_member_port_cmd_help[] = { "Show running system information", "Linkagg module", "<0-127>", "Member ports of linkagg group", ((void *)0) };

char *ctc_cli_stats_create_statsid_cmd_help[] = { "Statistics", "Create", "Stats id", "<0-0xFFFFFFFF>", "Vlan", "1-4095", "Vrf", "Vrf id", "Ipmc", "Mpls", "Scl", "Acl", "Tunnel", "Nexthop", "L3if", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_pdu_l2_set_global_action_by_macda_low24_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Global action", "Base on 24bit Macda", "Entry index", "<0-7>", "Entry valid", "<0-0xFF>,when great than 1,as 1 to process", "By pass all, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "Per-port action index", "<0-14>", ((void *)0) };

char *ctc_cli_acl_update_action_cmd_help[] = { "ACL module", "Update action", "ACL entry id", "<0~0xFFFFFFFF>", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *cli_oam_cfm_update_fs_cmd_help[] = { "OAM module", "CFM", "Update MEP", "Chip id on linecard", "<0-1>", "MEP index", "Min MEP index <2-2047>", "Max MEP index <2-2047>", ((void *)0) };

char *ctc_cli_aps_debug_on_cmd_help[] = { "Debugging functions", "Aps Module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_mdio1g_clock_cmd_help[] = { "Mdio1g cmd", "Core clk", "Core Clock value (MHZ)", "Mdio clock", "Mdio1g Clock value (MHZ)", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_SharedDs_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show SharedDs statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *show_cli_linkread_cmd_help[] = { "Read linked tables", "Table name", "Table name value example [DsMac]", "Table index", "Table index value <0-0xFFFFFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_end_cmd_help[] = { "End current mode and change to EXEC mode", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_calc_eye_height_cmd_help[] = { "Hss12g cmd", "Calc-eye-height cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "HSS12G link num", "<0-8>, 8 means broadcast", "save result data file", "input eye file name (default file name: ./calc_eye.txt)", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_bfd_remove_lmep_cmd_help[] = { "Local MEP", "BFD MEP", "Remove local BFD MEP", "My Discriminator", "My Discriminator value, <0~4294967295>", "IP BFD OAM option", "MPLS BFD OAM option", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", ((void *)0) };

char *ctc_cli_gb_nh_show_resource_usage_cmd_help[] = { "Show running system information", "Nexthop module", "Nexthop's resource used status", ((void *)0) };

char *ctc_cli_dbg_tool_mac_switch_sys_cmd_help[] = { "Mac ctl cmd", "Switch cmd", "Start mac id <0-59>", "End mac id <0-59>", "Swich times", "Times Value", "Switch mode", "0: serial; 1: parallel", "Net tx mode", "0: EPE chan mode; 1: netTx chan mode", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_default_cfg_cmd_help[] = { "Port module", "<0-0x1DFF>", "Set port configure to default", ((void *)0) };

char *ctc_cli_dbg_tool_check_net_rx_packet_capture_cmd_help[] = { "Check cmd", "Net-rx packet buffer capture cmd", "Port id", "Port id <0-60>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_set_port_oam_enable_cmd_help[] = { "OAM module", "CFM", "Global port", "<0-0x1DFF>", "Ingress direction", "Egress direction", "Both direction", "Enable OAM on this port", "Disable OAM on this port", ((void *)0) };

char *ctc_cli_bpe_show_mux_demux_cmd_help[] = { "Show running system information", "bpe module", "mux/demux function", "Port module", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_acl_add_ipv6_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", " Ipv6 entry ", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Source CoS", "<0-7>", "<0-7>", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "Ether type", "<0-0xFFFF>", "<0-0xFFFF>", "Stag valid", "<0-1>", "Ctag valid", "<0-1>", "Dscp", "<0-0x3F>", "<0-0x3F>", "Ip precedence", "<0-0x7>", "<0-0x7>", "Ip fragment information", "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment", "Ip option", "<0-1>", "Routed packet", "<0-1>", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "<0-0xFF>", "Layer4 source port", "Use mask.", "Use range. ", "Use mask:Src Port data. Use range: Src port min. ", "<0-0xFFFF>", "Use mask:Src Port mask. Use range: Src port max. ", "<0-0xFFFF>", "Layer4 destination port", "Use mask.", "Use range. ", "Use mask:Dst Port data. Use range: Dst port min. ", "<0-0xFFFF>", "Use mask:Dst Port mask. Use range: Dst port max. ", "<0-0xFFFF>", "Icmp type", "<0-0xFF>", "<0-0xFF>", "Icmp code", "<0-0xFF>", "<0-0xFF>", "Tcp flags", "Match any tcp flag", "Match all tcp flags", "<0-0x3F>", "IPv6 source address", "IPv6 address in X:X::X:X format", "Host address", "IPv6 mask in X:X::X:X format", "IPv6 destination address", "IPv6 address in X:X::X:X format", "Host address", "IPv6 mask in X:X::X:X format", "Ipv6 Extension header", "<0-0xFF>", "<0-0xFF>", "Ipv6 Flow label", "<0-0xFFFFF>", "<0-0xFFFFF>", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *ctc_cli_chip_set_i2c_scan_para_cmd_help[] = { "chip module", "set operate", "auto scan operate", "scan parameter", "i2c device address", "address value", "scan reg offset", "offset value", "scan data length", "length value", "i2c device bit map", "bit map value", "scan interval", "interval value", ((void *)0) };

char *ctc_cli_ptp_clear_tod_intf_rx_code_cmd_help[] = { "PTP module", "<0-1>", "Clear tod interface input time code", ((void *)0) };

char *ctc_cli_qos_set_policer_first_cmd_help[] = { "QoS module", "QoS policer", "Ingress direction", "Egress direction", "Both direction", "Flow policer first", "Port policer first", ((void *)0) };

char *ctc_cli_linkagg_add_or_remove_port_cmd_help[] = { "Linkagg module", "<0-127>", "Add member port to linkagg group", "Remove member port from linkagg group", "Member-port, global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_gb_vlan_show_vlan_mapping_default_entry_cmd_help[] = { "Show running system information", "VLAN module", "Vlan mapping", "Default", "Vlan mapping default information", "Global port", CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC, ((void *)0) };

char *ctc_cli_dma_mem_dump_cmd_help[] = { "dma", "memory", "dump", "address", "address value", "dump mem len", "len value", ((void *)0) };

char *ctc_cli_dbg_tool_drain_on_off_file_cmd_help[] = { "Drain on off ctl cmd", "Drain on off file", "Drain on off file name", "Repeat time", "Repeat time value", "Drain on off mode", "0: parallel 1:serial", "Off time ", "Off time vlaue ms", "On time ", "On time vlaue ms", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l2_remove_default_entry_cmd_help[] = { "Layer 2 module", "FDB table", "Remove operation", "Vlan default entry", "Forwarding ID", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_dbg_tool_write_reg_cmd_help[] = { "write address value cmd", "address 0xXXXXXXXX", "value to write", "rand", ((void *)0) };

char *ctc_cli_gb_qos_show_element_info_cmd_help[] = { "Show", "QoS module", "Port policer ", "Ingress port", "Egress port", "Flow policer", "Cpu reason range <0-max>", "Queue range <0-1023>", "Group range <0-255>", "Channel range <0-63>", "Range", "Range start", "Range end", "Detail", ((void *)0) };

char *ctc_cli_qos_set_group_shape_cmd_help[] = { "QoS module", "QoS shaping", "Group shaping", "Service identification", "<0-0xFFFF>", "Global port", "<0-0x1DFF>", "Cancel port shaping", "Config PIR (Peak Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", ((void *)0) };

char *ctc_cli_ptp_show_clock_drift_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Local clock drift", ((void *)0) };

char *ctc_cli_gb_ipuc_show_ipv4_nat_sa_cmd_help[] = { "Show running system information", "IPUC module", "Nat sa", ((void *)0) };

char *ctc_cli_stats_show_port_cmd_help[] = { "Show running system information", "Statistics", "Ingress port statistics", "Egress port statistics", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_oam_set_lbr_use_lbm_da_cmd_help[] = { "OAM module", "CFM", "Set reply LBR MAC SA mode", "Enable LBR MAC SA use LBM DA", "Disable LBR MAC SA use LBM DA", ((void *)0) };

char *ctc_cli_chip_show_discard_same_mac_or_ip_cmd_help[] = { "show", "chip module", "discard same mac adrress", "discard same ip adrress", ((void *)0) };

char *ctc_cli_l3if_create_or_destroy_l3_vlan_if_cmd_help[] = { "L3 interface module", "Create l3 interface", "Destroy l3 interface", "L3if ID", "<0-1022>", "L3 interface type", "Vlan interface", "VLAN", "<1-4095>", ((void *)0) };

char *show_cli_drv_read_reg_or_tbl_field_cmd_help[] = { "Read table or register", "Table name", "Table name value example [DsMac]", "Table index", "Table index value <0-0xFFFFFFFF>", "The counts of sequential registers you need to read ", "Value <1-8192>", "Step how many count. Default:0", "Step count. 2 means 0 2 4...", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_parser_show_l2_tpid_cmd_help[] = { "Show running system information", "Parser module", "Layer 2 header of packet", "Ethernet l2", "Type ID", "Customer vlan", "Itag tpid", "Backbone vlan", "Service vlan0", "Service vlan1", "Service vlan2", "Service vlan3", "Cntag tpid", ((void *)0) };

char *ctc_cli_gb_pdu_show_global_config_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "Layer3 PDU", "Global configuration info", ((void *)0) };

char *ctc_cli_vlan_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "VLAN module", "Vlan sub module", "Vlan class sub mdoule", "Vlan mapping sub module", "Protocol vlan sub module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_ptp_set_tod_intf_cmd_help[] = { "PTP module", "<0-1>", "Tod interface config", "Tod interface work mode", "Input mode", "Out mode", "1PPS signal duty", "<1-99>", "1PPS signal delay between master and slave", "<0-999999999>", "How many stop bits will be sent", "<0-0xFFFF>", "Bit clock cycles before time code", "<0-0xFFF>", ((void *)0) };

char *ctc_cli_add_ip_tunnel_cmd_help[] = { "Nexthop module", "Add nexthop", "IP Tunnel nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Unresolved nexthop", "Forward Nexthop", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "Tunnel type", "Tunnel over ipv4", "Ipv6inIpv4", "Ipv4inIpv4", "IP-BFD enable the nexthop", "6to4", "Use Manual IPSA", "6rd", "6rd IPv6 SP prefix length (default is 32)", "6rd IPv6 SP prefix length (0:16 1:24 2:32 3:40 4:48 5:56)", "IPv4 source address mask length (default is 32) ", "IPv4 source address mask length ", "IPv4 destination address mask length (default is 32)", "IPv4 destination address mask length ", "ISATAP", "GRE in Ipv4", "Protocol type of GRE Header", "Protocol type of GRE Header(Ipv4:0x0800,ipv6:0x86DD,ethernet:0x6558)", "GRE header with GRE key", "GRE key,if mtu-check-en is set,GRE Key only 16 bit, else 32 bit", "Mirror over GRE tunnel", "Ip sa", "IPv4 address in A.B.C.D format", "Ip da", "IPv4 address in A.B.C.D format", "If set , means new dont frag will copy payload ip dont frag ", "Use defined dont frag", "Dont frag <0~1>", "Tunnel over ipv6", "Ipv4inIpv6", "IP-BFD enable the nexthop", "Ipv6inIpv6", "GRE in Ipv6", "Protocol type of GRE Header", "Protocol type of GRE Header(Ipv4:0x0800,ipv6:0x86DD,ethernet:0x6558)", "GRE header with GRE key", "GRE key 32 bit", "Mirror over GRE tunnel", "Ip sa", "IPv6 address in X:X::X:X format", "Ip da", "IPv6 address in X:X::X:X format", "IVI", "IPv6 translate to IPv4", "IPv4 translate to IPv6", "IPv6 prefix address", "IPv6 address in X:X::X:X format", "Prefix length", "Length value 0-32, 1-40, 2-48, 3-56, 4-64, 5-72, 6-80, 7-96", "Prefix obey rfc6052, if set, IPv6 address's 64 to 71 bits are set to zero", "NAT", "New ipda", "IPv4 address in A.B.C.D format", "New layer4 dest port", "Value <0-65535>", "If set,encap packet will re-route with new tunnel header", "Dscp if ipv4 tunnel, TOS if ipv6 tunnel", "Dscp value: <0~0x3F>, Tos value: <0~0xFF>", "Dscp select mode <0~2>", "Dscp select mode:0,Use user-define DSCP to outer header,1-Use DSCP value from DSCP map,2-Copy packet DSCP to outer header", "TTL", "TTL value: <0~0xFF>", "TTL mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "IPv6 flow label mode (default is do not set flow label value)", "IPv6 flow label mode:0-Do not set flow label valu, 1-Use (user-define flow label + header hash)to outer header , 2- Use user-define flow label to outer header", "IPv6 flow label value (default is 0)", "IPv6 flow label value <0-0xFFFFF>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_seq_num_en_cmd_help[] = { "Local MEP", "CCM sequence number", "Enable CCM sequence number", "Disable CCM sequence number", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_mel_cmd_help[] = { "Local MEP", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_chip_write_xgphy_interface_cmd_help[] = { "chip module", "write operate", "xg phy", "index of port", "<0-0x1DFF>", "device eg:pmd/wis/phy xgxs/dte xgxs", "device no", "phy register", "<0~31>", "value", "reg value", ((void *)0) };

char *ctc_cli_stacking_create_trunk_cmd_help[] = { "Stacking", "Create", "Trunk", "Trunk id <1-63>", "Encap header", "Layer2 header", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "VLAN", "<1-4095>", "Ipv4 header", "IPv4 destination address", "IPv4 destination address in format of A.B.C.D", "Ipv6 header", "IPv6 destination address", "IPv6 destination address in format of X:X::X:X", ((void *)0) };

char *ctc_cli_port_set_link_intr_cmd_help[] = { "Interrupt module", "<0-0x1DFF>", "mac", "link change interrupt", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_port_set_ipg_index_cmd_help[] = { "Port module", "<0-0x1DFF>", "Ipg index", "Value <0-3>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_lm_cos_type_cmd_help[] = { "Local MEP", "Loss measurement cos type", "Loss measurement all cos packet together", "Loss measurement only specified cos packet", "Loss measurement per cos", ((void *)0) };

char *ctc_cli_dbg_tool_monitor_sgmac_prbs_xfipcs_cmd_help[] = { "Monitor cmd", "Sgmac-prbs cmd", "Pcsmode xfipcs", "Port id", "<0-12>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_read_hss12g_cmd_help[] = { "Read cmd", "Hss12g cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "Transmitter section", "Receiver section", "HS PLL section", "HSS12G link num", "<0-8>, 8 means broadcast", "HSS12G addr offset", "<0-0x3F>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_PtpFrc_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show PtpFrc statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_vlan_add_egress_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Use egress vlan mapping", "Add vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Stag vlan", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Stag priority or cos", "<0-7>", "Ctag vlan", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Ctag priority or cos", "<0-7>", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, "Mapping to", "Mapped cvlan", "<1-4095>", "Mapped svlan", "<1-4095>", "New stag cos", "<0-7>", "New ctag cos", "<0-7>", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Enable aging", ((void *)0) };

char *ctc_cli_port_efm_lb_mode_disable_cmd_help[] = { "Port module", "<0-0x1DFF>", "EFM loop-back mode", "Disable EFM loop-back mode this port", ((void *)0) };

char *ctc_cli_vlan_create_vlan_cmd_help[] = { "VLAN module", "Create one vlan", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_port_show_ipg_size_cmd_help[] = { "Show running system information", "System attribution", "Ipg index", "Value <0-3>", "Size", ((void *)0) };

char *ctc_cli_acl_add_pbr_ipv6_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", "Pbr Ipv6 entry ", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Source CoS", "<0-7>", "<0-7>", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "Ether type", "<0-0xFFFF>", "<0-0xFFFF>", "Dscp", "<0-0x3F>", "<0-0x3F>", "Ip precedence", "<0-0x7>", "<0-0x7>", "Ip fragment information", "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment", "<0-1>", "Vrfid", "VRF id of route <0-8191>", "<0-0x3FFF>", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "<0-0xFF>", "Layer4 source port", "Use mask.", "Use range. ", "Use mask:Src Port data. Use range: Src port min. ", "<0-0xFFFF>", "Use mask:Src Port mask. Use range: Src port max. ", "<0-0xFFFF>", "Layer4 destination port", "Use mask.", "Use range. ", "Use mask:Dst Port data. Use range: Dst port min. ", "<0-0xFFFF>", "Use mask:Dst Port mask. Use range: Dst port max. ", "<0-0xFFFF>", "Icmp type", "<0-0xFF>", "<0-0xFF>", "Icmp code", "<0-0xFF>", "<0-0xFF>", "Tcp flags", "Match any tcp flag", "Match all tcp flags", "<0-0x3F>", "IPv6 source address", "IPv6 address in X:X::X:X format", "Host address", "IPv6 mask in X:X::X:X format", "IPv6 destination address", "IPv6 address in X:X::X:X format", "Host address", "IPv6 mask in X:X::X:X format", "Ipv6 Extension header", "<0-0xFF>", "<0-0xFF>", "Ipv6 Flow label", "<0-0xFFFFF>", "<0-0xFFFFF>", "Permit", "Deny", "Forward to nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Ecmp nexthop", "TTL check", "ICMP redirect check", "Copy to cpu", ((void *)0) };

char *ctc_cli_ptp_show_sync_intf_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Sync interface config", ((void *)0) };

char *ctc_cli_pdu_l3_set_global_action_cmd_help[] = { "Pdu modue", "Layer3 PDU", "Global action", "OSPF, not need entry index", "PIM,  not need entry index", "VRRP, not need entry index", "RSVP, not need entry index", "RIP,  not need entry index", "BGP,  not need entry index", "MSDP, not need entry index", "LDP,  not need entry index", "Base on layer3 protocol", "Base on layer4 port", "Base on layer4 type", "Entry index", "<0-15>", "Entry valid", "<0-0xFF>,when great than 1,as 1 to process", "Per-l3if action index", "<0-14>", ((void *)0) };

char *ctc_cli_stats_reset_port_cmd_help[] = { "Clear functions", "Statistics", "Ingress port statistics", "Egress port statistics", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_chip_show_datapath_mode_cmd_help[] = { "show", "chip module", "user configuration", "mode", "chip", "<0~1>", ((void *)0) };

char *ctc_cli_gb_qos_show_cpu_reason_cmd_help[] = { "Show", "QoS module", "Cpu reason id for maping,destination,shaping", ((void *)0) };

char *ctc_cli_oam_cfm_exit_chan_cmd_help[] = { "Exit channel mode", ((void *)0) };

char *ctc_cli_oam_cfm_add_chan_cmd_help[] = { "OAM module", "CFM", "MEP lookup channel", "Add MEP lookup channel", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "MEP direction", "Up MEP", "Down MEP", "Is link level oam", "VLAN", "<1-4095>", "VLAN", "<1-4095>", "Master chip id for link agg", "<0-29>", ((void *)0) };

char *ctc_cli_gb_chip_agent_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Chip Agent module", ((void *)0) };

char *ctc_cli_parser_show_l3_ip_frag_cmd_help[] = { "Show running system information", "Parser module", "Layer 3 header of packet", "Layer3 ip", "Small fragment offset<0-3>", ((void *)0) };

char *ctc_cli_dbg_tool_oam_autogen_pkt_cmd_help[] = { "Oam cmd", "Autogen pkt cmd", "Port id", "<0-59>", "Pkt size", "<64B~9600B>", "Band-width", "<1000Mbps~10000Mbps>", "Tx-mode", "0: normal; 1: continuously", "Tx-pattern", "0:repeat(0x5a5aa5a5); 1:random; 2:increment byte; 3:decrement byte; 4:increment word; 5:decrement word", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_bfd_show_mep_info_with_key_cmd_help[] = { "Show running system information", "BFD", "BFD information", "My Discriminator", "My Discriminator value, <0~4294967295>", "IP BFD OAM option", "MPLS BFD OAM option", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", ((void *)0) };

char *ctc_cli_gb_nh_set_service_queue_cmd_help[] = { "Nexthop module", "Service queue", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Enable", "Disable", ((void *)0) };

char *ctc_cli_mpls_ac_remove_vlan_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "L2vpn attachment circuit entry", "Remove l2vpn ac", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Port binding to l2vpn with svlan", "Port binding to l2vpn with cvlan", "0~4094", ((void *)0) };

char *ctc_cli_pdu_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Pdu modue", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_bpe_set_mux_demux_cmd_help[] = { "bpe module", "port extender function", "Port module", "<0-0x1DFF>", "vlan base", "<1-4095>", "internal port start", "<64-127>", "extender number", "<0-63>", "enable", "disable", ((void *)0) };

char *ctc_cli_gb_vlan_show_vlan_l3if_id_cmd_help[] = { "Show running system information", "VLAN", "<1-4095>", "L3if id", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_HashDs_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show HashDs statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_dma_tbl_wr_cmd_help[] = { "Dma module", "Write table", "Asic table", "Table address", "Length of entry, unit:bytes", "Value <1-65535>", "Length of system buffer", "Value <1-65535>", ((void *)0) };

char *ctc_cli_oam_cfm_remove_lmep_cmd_help[] = { "Local MEP", "Remove local MEP on oam channel", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_nh_get_l2uc_cmd_help[] = { "Show running system information", "Nexthop module", "L2Uc nexthop", "Global port", "<0-0x1DFF>", "Bypass nexthop", ((void *)0) };

char *ctc_cli_oam_cfm_set_port_max_md_level_cmd_help[] = { "OAM module", "CFM", "Global port", "<0-0x1DFF>", "Ingress direction", "Egress direction", "Both direction", "Set max MD level on this port", "<0-7>", ((void *)0) };

char *ctc_cli_chip_set_phy_scan_para_cmd_help[] = { "chip module", "set operate", "auto scan operate", "scan parameter", "ge phy bit map0", "bit map0 value", "ge phy bit map1", "bit map1 value", "xg phy bit map0", "bit map0 value", "xg phy bit map1", "bit map1 value", "scan interval", "interval value", "xg phy device link mask", "mask value", "xgphy scan twince", "enable", "disable", ((void *)0) };

char *ctc_cli_gb_interrupt_set_en_cmd_help[] = { "Interrupt module", "<0-1>", "Interrupt sup-level type", "<0-0xFFFFFFFF>", "Interrupt sub-level type", "<0-0xFFFFFFFF>", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_read_tbl_reg_c_cmd_help[] = { "Read cmd", "Table or register information", "Table/register id name", "Table/register entry begin index ", "Table/register entry end index", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_common_set_exception_cmd_help[] = { "OAM module", "OAM Common", "OAM exception to CPU", "Invalid OAM PDU", "Some RDI defect", "Some mac status defect", "High version ETH OAM to CPU", "ErrorCcmDefect RMEP not found", "XconCCMdefect", "CCM sequence number error", "Table or Control Register Configure error", "CCM has optional TLV", "Slow OAM to cpu", "Source mac mismatch", "APS PDU to CPU", "ErrorCcmDefect CCM interval mismatch to CPU", "Y.1731 & TP Y.1731 DM PDU to CPU", "Equal LBR to CPU", "LM PDU to CPU", "TEST PDU to CPU", "Invalid MPLS TP DM DLM PDU", "Big interval CCM", "MPLS TP DLM PDU to CPU", "MPLS TP DLM-DM PDU to CPU", "CSF PDU to CPU", "MIP not support PDU", "MCC PDU to CPU", "LT PDU to CPU", "LBM mac or MEP ID check fail", "BFD learning to CPU", "Error BFD PDU", "BFD timer negotiation", "SCC PDU to CPU", "All exception to CPU", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeCla_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeCla statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_emulation_dump_config_cmd_help[] = { "debug cmd", "dump", "cfg", "src file-based on the file's address to dump", "generate the dest file name", ((void *)0) };

char *ctc_cli_dbg_tool_read_sfp_cmd_help[] = { "Read cmd", "I2C master cmd", "Select which slaves to access", "<0-0xFFFFFFFF>", "Address", "Address value", "Slave internal memory offset", "<0-0xFFFF>", "Data len", "<0-32767>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_write_pcie_io_cmd_help[] = { "Write cmd", "PCI io cmd", "Address", "Address value", "Set value", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_dbg_tool_queueid_mon_cmd_help[] = { "Show cmd", "bsr queue info", "queue-id monitor info", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stacking_set_break_point_en_cmd_help[] = { "Stacking", "Break if exist break point", "Full mesh topo", "Enable", "Disable", ((void *)0) };

char *ctc_cli_parser_l2_mapping_l3type_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Mapping Layer3 type for Layer3 parsing", "Mapping l3 type entry", "Entry index", "Index Value <0-2>", "None l2 type", "l2 type ethernet-V2", "l2 type ethernet-sap", "l2 type ethernet-snap", "l2 type ppp 1b", "l2 type ppp 2b", "Layer2 header protocol", "Layer2 header protocol value <0x0-0xFFFF>", "Mask", "Mask value <0x0-0xFFFFFFFF>,when greater than 0x7FFFFF,as 0x7FFFFF to process ", "Layer3 type", "Layer3 type value <13-15>", "Addition offset to be added", "Offset value <0-15>", ((void *)0) };

char *ctc_cli_ipuc_remove_ipv6_cmd_help[] = { "IPUC module", "IPV6", "Remove ipv6 route", "VRF id of route <0-8191>", "IPv6 address in X:X::X:X format", "The length of Ipv6 mask <0~128>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "ECMP group", ((void *)0) };

char *ctc_cli_gb_oam_set_mep_maid_len_cmd_help[] = { "OAM module", "OAM MAID length", "Maid len 16bytes", "Maid len 32bytes", "Maid len 48bytes", ((void *)0) };

char *ctc_cli_gb_l2_show_dflt_cmd_help[] = { "Show running system information", "Layer 2 module", "Vlan default entry", "VLAN", "<0-16383>", ((void *)0) };

char *ctc_cli_stats_show_stats_cmd_help[] = { "Show running system information", "Statistics", "Stats id", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_I2CMaster_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show I2CMaster statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_mux_demux_cmd_help[] = { "Port module", "<0-0x1DFF>", "Mux, port range <64-255>", "Demux, port range <0-51>", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_security_ip_src_guard_add_entry_cmd_help[] = { "Security module", "Ip source guard", "Add ip source guard entry", "IPv6 source address", "IPv6 address in X:X::X:X format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_qos_set_policer_configure_cmd_help[] = { "QoS module", "QoS policer", "Sequential policer", "Ipg used for policer", "Update token", "HBWP share mode", "Globally enable ", "Globally disable", ((void *)0) };

char *ctc_cli_gb_vlan_overlay_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Overlay vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Stag vlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Ctag vlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Vlan range group id", "<0~63>", "Stag priority or cos", "<0-7>", "Ctag priority or cos", "<0-7>", "Mapping to", "Mapped cvlan", "<1-4095>", "New stag cos", "<0-7>", "New ctag cos", "<0-7>", "Service queue id", "Service id", "Ingress service ", "Egress service ", "Enable service acl", "Enable service policer", "Enable service queue", "Mapped svlan", "<1-4095>", "Logic port", "Logic-port-type", "<0 -0x1FFF>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Forwarding ID", "<0-16383>", "Vpls fid", "Disable vsi learning", "Enable vsi mac security", "It is leaf node in E-Tree networks", "Nexthop instance to forward", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpws nexthop", "It is leaf node in E-Tree networks", "Vlan switching", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Statistic", "0~0xFFFFFFFF", "Enable aging", ((void *)0) };

char *ctc_cli_dbg_tool_show_mac_MacMux_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "mac status", "show MacMux statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_scl_copy_entry_cmd_help[] = { "SCL module", "SCL entry id", "<0~0x7FFFFFFF>", "Copy to", "SCL group id", "<0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", ((void *)0) };

char *ctc_cli_packet_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Packet module", "Ctc layer", "System layer", ((void *)0) };

char *ctc_cli_stats_show_log_cmd_help[] = { "Show running system information", "Statistics", "Global port", "<0-0x1DFF>", "ingress direction", "egress direction", "log statis", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_NetRx_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show NetRx statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_chip_agent_debug_simulator_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Chip Agent module", "Simulator", "Packet IO", "All simulator modules", ((void *)0) };

char *ctc_cli_mpls_reset_in_sqn_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Ingress direction", "Sequence counter", "Sequence counter index, <0-255>", ((void *)0) };

char *ctc_cli_mpls_add_pw_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "L2vpn pw mapping", "Add vpls/vpws pw mapping", "Mpls label space", "Label space id, <0-255>, space 0 is platform space", "Vpls vc incoming label, <0-1048575>", "Virtual private wire services ", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "VPWS source port", "VPWS source port, 0xffff means no used", "Virtual private lan services", "<0-16383>", "VPLS source port", "VPLS source port, 0xffff means no used", "Vpls port type, means this vpls PW is not a H-VPLS tunnel", "enable igmp snooping ", "disable vsi learning", "vsi mac security discard", "APS select ", "<0-2047>", "Indicate working path", "Indicate protection path", "Enable service aclqos", "PW encapsulation mode", "Raw mode", "Tagged mode", "Control word used", "OAM Enable", "Qos use packet outer info", "Service id", "Service id number", "Service policer enable", "Service queue enable", "Statistic supported", "0~0xFFFFFFFF", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipv6_group_param_cmd_help[] = { "Show running system information", "IPv6", "IPMC module", "IP multicast group", "Information, including source and group IP adress, etc.", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route", "VRF id of route <0-8191>", "Do IP based L2 mcast", ((void *)0) };

char *ctc_cli_ipmc_set_ipv4_force_route_cmd_help[] = { "IPMC module", "Force route", "IPv4 address0", "IPv4 address in A.B.C.D format", "The length of ipv4 address mask <32>", "IPv4 address1", "IPv4 address in A.B.C.D format", "The length of ipv4 address mask <32>", "Force bridge", "Force unicast", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipv4_group_param_cmd_help[] = { "Show running system information", "IPMC module", "IP multicast group", "Information, including source and group IP adress, etc.", "Group address, class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "Do IP based L2 mcast", ((void *)0) };

char *ctc_cli_nh_update_mpls_pop_nexthop_cmd_help[] = { "Nexthop module", "Update nexthop", "MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Mpls pop nexthop", "Update forward nexthop", "Unresolved nexthop to forward nexthop", "Mpls pop nexthop payload operation type", "POP PHP operation", "PHP's Pipe mode", "PHP's Short Pipe mode", "PHP's Uniform mode", "Enable sequence order", "Sequence counter index, <0-255>", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", ((void *)0) };

char *ctc_cli_dbg_tool_show_resrc_mgr_cmd_help[] = { "show", "resrc manger", "ingress", "egress", "port", "<0-63>", "cos", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_monitor_inter_laken_prbs_cmd_help[] = { "Monitor cmd", "Inter-laken prbs cmd", "Lane id", "<0-8>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_reset_cpu_mac_cmd_help[] = { "Clear functions", "Statistics", "cpu mac statistics", "Global port", "<0-0x1DXX>, low 8bit value is ignored", ((void *)0) };

char *ctc_cli_stats_drop_log_cmd_help[] = { "Statistics", "drop paket", "log", "flow", "enable log drop", "disable log drop", ((void *)0) };

char *ctc_cli_nh_remove_mpls_tunnel_label_cmd_help[] = { "Nexthop module", "Delete nexthop", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", ((void *)0) };

char *ctc_cli_show_dma_desc_cmd_help[] = { "CTC_CLI_SHOW_STR", "DMA module", "Descriptor", "Stats descriptor", "Packet RX descriptor", "DMA channel", "DMA channel ID", "Packet TX descriptor", "Table read descriptor", "Table write descriptor", ((void *)0) };

char *ctc_cli_vlan_remove_vlan_mapping_default_entry_cmd_help[] = { "VLAN module", "Vlan mapping", "Remove vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Default entry", ((void *)0) };

char *ctc_cli_dbg_tool_check_capture_cmd_help[] = { "Debug cmd", "Capture lpmtcam intcam data", "LPM tcam", "Internal tcam", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_csf_en_cmd_help[] = { "Local MEP", "Client signal fail", "Enable Client signal fail", "Disable Client signal fail", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_chip_set_serdes_mode_cmd_help[] = { "chip module", "set operate", "serdes", "serdes id<0~31>", "serdes mode", "xfi mode", "sgmii mode", "xsgmii mode", "qsgmii mode", "xaui mode", "local chip", "<0-1>", ((void *)0) };

char *ctc_cli_stacking_add_remove_trunk_rchip_cmd_help[] = { "Stacking", "Trunk", "Trunk id <1-63>", "Add", "Remove", "Remote chip", "Remote chip <0-29>", ((void *)0) };

char *ctc_cli_common_debug_on_cmd_help[] = { "Debugging functions", "Enable debugging information", ((void *)0) };

char *ctc_cli_ipuc_add_ipv4_cmd_help[] = { "IPUC module", "Add route", "VRF id of route <0-8191>", "IPv4 address in A.B.C.D format", "The length of Ipv4 mask <0~32>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "ECMP group", "TTL check enable", "RPF check enable", "ICMP redirect check", "ICMP error message check", "Copy to cpu", "Protocol-entry", "Self address, used for oam or ptp", "This route is a Neighbor", "This route is a Connect route", "L3 interface of this connect route", "L3 interface ID", "Layer4 destination port for NAPT", "<0-0xFFFF>", "If set, indicate it is a tcp port, or is a udp port", "Stats id", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_port_set_port_mac_postfix_cmd_help[] = { "Port module", "<0-0x1DFF>", "Prefix port mac index", "Value <0-1>", "Port MAC low 8bit", "<0~255>", ((void *)0) };

char *ctc_cli_port_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Port module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_learning_aging_remove_port_nhid_mapping_cmd_help[] = { "Learning module", "Port to nexthop mapping", "Remove Port to nexthop mapping", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vpls port", "<0-8191>", "Forwarding ID", "<0-16383>", ((void *)0) };

char *ctc_cli_gb_oam_trpt_show_state_cmd_help[] = { "Show running system information", "OAM module", "ThroughPut", "State", "gchip", "gchip id", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_DsFib_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show DsFib statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_pdu_show_l2pdu_port_action_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "L2pdu Port-ction type" "Port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_l3if_add_vmac_low_8bits_cmd_help[] = { "L3 interface module", "VRRP mac", "L3if ID", "<0-1022>", "Add low 8bits of router mac", "Vmac prefix type", "Prefix type", "The index of vmac entry", "Index range <0-3>", "Low 8bits mac", "The value of low 8bits mac, in 0xHH format", ((void *)0) };

char *ctc_cli_sync_ether_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "SyncE module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_scl_add_port_svlan_cos_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port svlan cos hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "Direction", "Ingress direction", "Egress direction", "Service VLAN", "<1-4095>", "Service VLAN CoS", "<0-7>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_common_show_ver_cmd_help[] = { "Show running system information", "Sdk version", ((void *)0) };

char *ctc_cli_dbg_tool_write_i2c_cmd_help[] = { "Write cmd", "I2C raw cmd", "I2C device address", "Address value", "Set value", "<0-0xFF>", ((void *)0) };

char *ctc_cli_vlan_set_ipv4_l2mc_cmd_help[] = { "VLAN module", "<1-4095>", "Ipv4 based l2mc", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_QMgrQueEntry_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show QMgrQueEntry statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_vlan_destroy_vlan_vrange_cmd_help[] = { "VLAN module", "Vlan range", "Destroy", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, "Direction", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_scl_install_entry_cmd_help[] = { "SCL module", "Install to hardware", "SCL entry id", "<0~0x7FFFFFFF>", ((void *)0) };

char *ctc_cli_security_debug_on_cmd_help[] = { "Debugging functions", "Security module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_stats_set_port_cfg_cmd_help[] = { "Statistics", "Ingress port statistics option type", "Egress port statistics option type", "Option type", "Port do no stats", "Basic port", "Mac based port", "Protocol based port", "PHB", "Per port PHB", ((void *)0) };

char *ctc_cli_port_set_stag_tpid_index_cmd_help[] = { "Port module", "<0-0x1DFF>", "Stag TPID index point to stag TPID value", "Direction", "Ingress", "Egress", "Both direction", "Index value 0-3", ((void *)0) };

char *ctc_cli_parser_l3_set_l3flex_cmd_help[] = { "Parser module", "Layer 3 header of packet", "Flexible config", "Ip da basic offset", "Offset value <0x0-0x18>", "Minium length", "Length value <0x0-0xFF>", "Layer3 basic offset", "Offset value <0x0-0xFF>", "Protocol byte select", "Value <0x0-0x1F>", ((void *)0) };

char *ctc_cli_parser_l2_set_tpid_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Ethernet l2", "Type ID", "Customer vlan", "Itag tpid", "Backbone vlan", "Service vlan0", "Service vlan1", "Service vlan2", "Service vlan3", "Cntag tpid", "Tpid value<0x0 - 0xFFFF>", ((void *)0) };

char *ctc_cli_nh_add_remove_ipmc_member_cmd_help[] = { "Nexthop module", "Mcast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Add operation", "Remove operation", "The ip member of multicast nexthop", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN interface", "VLAN", "<1-4095>", "Global port", "Global port", "Vlan port" , "Physical interface/remote chip id", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_acl_install_entry_cmd_help[] = { "ACL module", "Install to hardware", "ACL entry id", "<0~0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_nh_debug_on_cmd_help[] = { "Debugging functions", "Nexthop module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_security_port_isolation_show_route_obey_cmd_help[] = { "Show running system information", "Security module", "Port isolation", "Route obey isolate", ((void *)0) };

char *ctc_cli_oam_cfm_set_bridge_mac_cmd_help[] = { "OAM module", "CFM", "Set bridge mac", "Bridge mac", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_scl_create_group_cmd_help[] = { "SCL module", "Create SCL group", "SCL group id", "<0~0xFFFF0000>", "Group type", "Port scl", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Global", "Port class", "<0~63>", "Lchip", "<0-1>", "Logic Port", "Logic port value", ((void *)0) };

char *ctc_cli_ptp_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "PTP module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_acl_add_mpls_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", " Mpls entry ", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Source CoS", "<0-7>", "<0-7>", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "Layer 2 Type", "<0-15>", "<0-0xF>", "Routed packet", "<0-1>", "Mpls label 0, include: Label + Exp + S + TTL ", "<0-0xFFFFFFFF>", "<0-0xFFFFFFFF>", "Mpls label 1, include: Label + Exp + S + TTL ", "<0-0xFFFFFFFF>", "<0-0xFFFFFFFF>", "Mpls label 2, include: Label + Exp + S + TTL ", "<0-0xFFFFFFFF>", "<0-0xFFFFFFFF>", "Mpls label 3, include: Label + Exp + S + TTL ", "<0-0xFFFFFFFF>", "<0-0xFFFFFFFF>", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *ctc_cli_port_set_vlan_classify_cmd_help[] = { "Port module", "<0-0x1DFF>", "Vlan classify", "Mac based vlan", "IPv4 based vlan", "IPv6 based vlan", "Protocol based vlan", "Enable functions", "Vlan classify policy", "<0-63>", "Disable functions", ((void *)0) };

char *ctc_cli_chip_set_mac_led_mode_cmd_help[] = { "chip module", "set operate", "mac led ", "parameter", "gport", "CTC_CLI_GPHYPORT_ID_DESC", "mac led mode", "<0-1>", "polarity", "polarity value", "first led mode", "first mode value", "second led mode", "second mode value", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeHdrProc_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeHdrProc statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_vlan_show_vlan_class_default_entry_cmd_help[] = { "Show running system information", "VLAN module", "Vlan classification", "Default", "Vlan mapping default information", ((void *)0) };

char *ctc_cli_port_set_receive_cmd_help[] = { "Port module", "<0-0x1DFF>", "Reception port state", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_nh_add_misc_l2l3hdr_cmd_help[] = { "Nexthop module", "Add nexthop", "Misc nexthop", "Replace L2 & L3 Header", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "If set,packet will be reflective to ingress port", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Swap macda and macsa", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Edit vlan", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "Replace ipda", "IPv4 address in A.B.C.D format", "Replace layer4 dest port", "Value <0-65535>", ((void *)0) };

char *ctc_cli_chip_set_discard_same_mac_or_ip_cmd_help[] = { "chip module", "discard same mac adrress", "discard same ip adrress", "enable", "disable", ((void *)0) };

char *ctc_cli_mpls_ac_add_port_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "L2vpn attachment circuit entry", "Add l2vpn ac", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Virtual private wire services ", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Virtual private lan services", "<0-16383>", ((void *)0) };

char *ctc_cli_parser_show_l4_app_cmd_help[] = { "Show running system information", "Parser module", "Layer 4 header of packet", "Udp application operation", ((void *)0) };

char *ctc_cli_gb_vlan_add_vlan_vrange_cmd_help[] = { "VLAN module", "Vlan range", "Add", "<1-4095>", "To", "<1-4095>", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, "Direction", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_nh_remove_xlate_cmd_help[] = { "Nexthop module", "Delete nexthop", "Egress Vlan Editing nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_IntLkIntf_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show IntLkIntf statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_scl_show_entry_info_cmd_help[] = { "Show running system information", "SCL module", "Entry info", "All entries", "By entry", "<0~0x7FFFFFFF>", "By group", "<0xFFFF000C>", "By priority", "By type", "SCL Mac-entry", "SCL Ipv4-entry", "SCL Ipv6-entry", "SCL Vlan-entry", "SCL Hash-port-entry", "SCL Hash-port-cvlan-entry", "SCL Hash-port-svlan-entry", "SCL Hash-port-2vlan-entry", "SCL Hash-port-cvlan-cos-entry", "SCL Hash-port-svlan-cos-entry", "SCL Hash-mac-entry", "SCL Hash-port-mac-entry", "SCL Hash-ipv4-entry", "SCL Hash-port-ipv4-entry", "SCL Hash-ipv6-entry", "SCL Hash-l2-entry", "SCL Hash-tunnel-ipv4-entry", "SCL Hash-tunnel-ipv4-gre-entry", "SCL Hash-tunnel-ipv4-rpf-entry", "Detail", ((void *)0) };

char *ctc_cli_nh_show_max_ecmp_cmd_help[] = { "Show running system information", "Nexthop module", "Max_ecmp the maximum ECMP paths allowed for a route.<1-8>", ((void *)0) };

char *ctc_cli_port_mapping_show_port2phy_cmd_help[] = { "Show running system information", "port to phy", "mapping table", ((void *)0) };

char *ctc_cli_port_mapping_show_port2mdio_cmd_help[] = { "Show running system information", "port to mdio", "mapping table", ((void *)0) };

char *ctc_cli_mpls_reset_out_sqn_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Egress direction", "Sequence counter", "Sequence counter index, <0-255>", ((void *)0) };

char *ctc_cli_port_set_use_outer_ttl_cmd_help[] = { "Port module", "<0-0x1DFF>", "Use outer ttl in case of tunnel", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_DynamicDs_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show DynamicDs statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_scan_phy_cmd_help[] = { "Scan cmd", "PHY cmd", "Enable functions", "Interval Value <0-0xFFFF>", "Disable functions", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_vlan_set_member_port_cmd_help[] = { "VLAN module", "Add member port to vlan", "Delete member port from vlan", "Global port", "<0-0x1DFF>", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_chip_show_sensor_value_cmd_help[] = { "show", "chip module", "sensor interface", "temperature sensor", "voltage sensor", "local chip id", ((void *)0) };

char *ctc_cli_dbg_tool_write_sfp_cmd_help[] = { "Write cmd", "I2C master cmd", "Slave id", "<0-31>", "Address", "Address value", "Slave internal memory offset", "<0-0xFFFF>", "Set value", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_linkagg_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Linkagg module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_dbg_tool_write_i2c_local_cmd_help[] = { "Write cmd", "I2C local cmd", "Address", "Address value", "Set value", "<0-0xFFFFFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipv6_member_info_cmd_help[] = { "Show running system information", "IPMC module", "IPv6", "Members of a given multicast group", "Information, including gport, vlan, l3 type.", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route", "VRF id of route <0-8191>", "Do IP based L2 mcast", ((void *)0) };

char *ctc_cli_oam_cfm_update_rmep_sf_state_cmd_help[] = { "Remote MEP", "Set signal fail state", "<0-1>", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_ptp_set_adjust_delay_cmd_help[] = { "PTP module", "Delay value used for adjust PTP message pass the device", "<0-0x1DFF>", "Latency from phy to mac on RX", "<0-0xFFFF>", "Latency from mac to phy on TX", "<0-0xFFFF>", "Asymmetry on ingress", "high 4bits value <0-0xF>", "low 32bits value <0-0xFFFFFFFF>", "The value is negative", "Asymmetry on egress", "high 4bits value <0-0xF>", "low 32bits value <0-0xFFFFFFFF>", "The value is negative", "Path delay", "high 4bits value <0-0xF>", "low 32bits value <0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_stats_debug_on_cmd_help[] = { "Debugging functions", "Statistics", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_run_inter_laken_prbs_cmd_help[] = { "Run cmd", "Inter-laken prbs cmd", "Lane id", "<0-8>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l2_flush_fdb_cmd_help[] = { "Layer 2 module", "FDB table", "Flush fdb", "Query condition", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Forwarding ID", "<0-16383>", "Port+FID", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "<0-16383>", "All condition", "Static fdb table", "All dynamic fdb table", "Local chip dynamic fdb table", "Static and dynamic", ((void *)0) };

char *ctc_cli_dbg_tool_scan_xgphy_para_cmd_help[] = { "Scan cmd", "Set xgphy scan parameter cmd", "Bitmap0", "bitmap0", "Bitmap1", "bitmap1", "Link mask <bit0: PMA/PMD; bit1: PCS; bit2: PHY XS>", "<0-0x7>", "Twice enable", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_acl_destroy_group_cmd_help[] = { "ACL module", "Destroy ACL group", "ACL group id", "<0xFFFF0000>", ((void *)0) };

char *ctc_cli_scl_add_ipv4_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Ipv4 hash entry", "IPv4 source address", "IPv4 address in A.B.C.D format", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_gb_ipuc_show_ipv6_cmd_help[] = { "Show running system information", "IPUC module", "IPV6", ((void *)0) };

char *ctc_cli_mpls_add_l3vpn_ilm_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Incoming label mapping", "Add ilm", "L3vpn vc label", "Label space id, <0-255>, space 0 is platform space", "Label, <0-1048575>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "VRF id of route <0-8191>", "Uniform model", "Short pipe model", "Pipe model", "Qos use packet outer info", "Use outer label's exp", "Inner packet type is ipv6", "OAM Enable", ((void *)0) };

char *ctc_cli_gb_ipmc_show_count_cmd_help[] = { "Show running system information", "IPMC module", "Show ipmc route number", ((void *)0) };

char *ctc_cli_acl_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "ACL module", "Ctc layer", "Sys layer", "ACL Label", "ACL entry", ((void *)0) };

char *show_cli_drv_list_tbl_cmd_help[] = { "List tables", "Substring of table name", ((void *)0) };

char *ctc_cli_gb_ipuc_show_count_cmd_help[] = { "Show running system information", "IPUC module", "Show ipuc route number", ((void *)0) };

char *ctc_cli_chip_write_i2c_cmd_help[] = { "chip module", "write operate", "i2c device", "device address", "address value", "slave id", "slave id value", "offset", "offset value", "data", "data value", ((void *)0) };

char *ctc_cli_nh_add_rspan_cmd_help[] = { "Nexthop module", "Add nexthop", "RSPAN nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "VLAN", "<1-4095>" "CrossChip Rspan", ((void *)0) };

char *ctc_cli_gb_vlan_remove_egress_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Use egress vlan mapping", "Remove vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "802.1ad svlan", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Stag priority or cos", "<0-7>", "802.1ad cvlan", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Ctag priority or cos", "<0-7>", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, ((void *)0) };

char *ctc_cli_vlan_debug_on_cmd_help[] = { "Debugging functions", "VLAN module", "Basic Vlan sub module", "Vlan class sub mdoule", "Vlan mapping sub module", "Protocol vlan sub module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_ipmc_update_ipv6_rpf_cmd_help[] = { "IPMC module", "IPv6", "Update", "Reverse path forward", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route", "VRF id of route <0-8191>", "L3if ID", "<0-1023>, 0:disable rpf check", "L3if ID", "<0-1023>, 0:disable rpf check", "L3if ID", "<0-1023>, 0:disable rpf check", "L3if ID", "<0-1023>, 0:disable rpf check", "More than 4 rpf interfaces, send to CPU for checking", ((void *)0) };

char *ctc_cli_nh_remove_aps_xlate_cmd_help[] = { "Nexthop module", "Delete nexthop", "APS Egress Vlan Editing nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_set_freq_cmd_help[] = { "Hss12g cmd", "Hss12g set frequency cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "HSS12G link num", "<0-8>, 8 means broadcast", "Frequency", "1.25G", "5G", "10.3125G", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_nh_clear_stats_cmd_help[] = { "Clear functions", "Nexthop module", "Statistics", "Nexthop id", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "Portection path", ((void *)0) };

char *ctc_cli_learning_aging_show_logic_port_mapping_srcport_cmd_help[] = { "Show running system information", "Learning module", "Virtual port(VPLS  port)", "0-0x1FFF", ((void *)0) };

char *ctc_cli_dbg_tool_oam_autogen_pkt_disable_cmd_help[] = { "Oam cmd", "Autogen pkt cmd" "Disable functions", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l2_clear_stp_state_cmd_help[] = { "Clear functions", "Spanning Tree Protocol (STP)", "Stp state", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_pdu_debug_on_cmd_help[] = { "Debugging functions", "Pdu modue", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_gb_ftm_opf_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Offset pool freelist", "Fast priority arrangement", ((void *)0) };

char *ctc_cli_dbg_tool_bsr_flow_chart_cmd_help[] = { "show bus", "debug tool", "bsr", "flow chart", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_parallel_loopback_cmd_help[] = { "Hss12g cmd", "Parallel-loopback cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "HSS12G link num", "<0-8>, 8 means broadcast", "Enable functions", "Disable functions", "Chip id on linecard", "<0-1>", ((void *)0) };

char *show_cli_drv_list_tbl_all_cmd_help[] = { "List tables", ((void *)0) };

char *ctc_cli_ipuc_add_default_cmd_help[] = { "IPUC module", "IPV6", "Default route", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_gb_vlan_remove_egress_vlan_mapping_default_entry_cmd_help[] = { "VLAN module", "Vlan mapping", "Use egress vlan mapping", "Remove vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Default entry", ((void *)0) };

char *ctc_cli_port_set_port_check_enable_cmd_help[] = { "Port module", "<0-0x1DFF>", "Src port match check", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_oam_tp_y1731_add_remove_maid_cmd_help[] = { "OAM module", "MPLS TP Y.1731", "MA ID", "Add MAID", "Remove MAID", "MEP group id", "MEG id string", ((void *)0) };

char *ctc_cli_port_set_vlan_filtering_cmd_help[] = { "Port module", "<0-0x1DFF>", "Vlan filtering", "Filtering direction", "Ingress filtering", "Egress filtering", "Both igs and egs filtering", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeOam_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeOam statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_show_mac_CpuMac_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "mac status", "show CpuMac statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_check_sgmac_prbs_xauipcs_cmd_help[] = { "Check cmd", "Sgmac-prbs cmd", "Pcsmode xauipcs", "Port id", "<4-12>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_run_capture_cmd_help[] = { "Debug cmd", "Start to capture lpmtcam intcam data", "LPM tcam", "Internal tcam", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_show_drop_log_cmd_help[] = { "Show running system information", "Statistics", "Global port", "<0-0x1DFF>", "drop paket", "log", "flow", ((void *)0) };

char *ctc_cli_common_enter_sdk_mode_cmd_help[] = { "Enter", "Ctc SDK mode", "Mode", ((void *)0) };

char *ctc_cli_port_set_blocking_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port based blocking", "Ucast unkown flooding to this port", "Mcast unkown flooding to this port", "Mcast kown flooding to this port", "Broadcast flooding to this port", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_ipuc_arrange_fragment_cmd_help[] = { "IPUC module", "Arrange lpm fragment", "Arrange lpm fragment", ((void *)0) };

char *ctc_cli_gb_nh_set_reflective_bridge_cmd_help[] = { "Nexthop module", "Mcast packet will support reflective bridge", "Enable", "Disable", ((void *)0) };

char *ctc_cli_security_set_port_security_cmd_help[] = { "Security module", "Port based mac security", "Global port", "<0-0x1DFF>", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_vlan_set_transmit_cmd_help[] = { "VLAN module", "<1-4095>", "If set, transmit is enable on the vlan", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_acl_install_group_cmd_help[] = { "ACL module", "Install to hardware", "ACL group id", "<0xFFFF0002>", "Direction", "Ingress", "Egress", "Both direction", "Lchip", "<0-1>", "Port bitmap (52 ports supported)", "<0-0xFFFFF> each bit stands a port valid. 20 high ports: port32~ port51", "<0-0xFFFFFFFF>, 32 low ports: port0~ port31", "Global", "Vlan class", "<1-1023> 0 is reserved", "Port class", "<1-1023> 0 is reserved", "Service ACL", "<1-0xFFFF>", "Pbr class", "<0-63>", ((void *)0) };

char *ctc_cli_stats_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Statistics", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_ipuc_offset_show_cmd_help[] = { "Show running system information", "IPUC module", "IPUC offset", "IPV6", "Block index", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_loopback_cmd_help[] = { "Hss12g cmd", "Loopback cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "HSS12G link num", "<0-8>, 8 means broadcast", "Enable functions", "Disable functions", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_qos_bind_service_cmd_help[] = { "QoS module", "Service", "Binding queue", "Unbinding queue", "Serive id", "Value <0-0xFFFF>", "dest port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_port_set_dir_property_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port property with direction", "Acl enable (bitmap 0~3)", "Port acl port classid", "Port acl port classid0", "Port acl port classid1", "Port acl ipv4-key force to mac-key", "Port acl ipv6-key force to mac-key", "Port acl force use ipv6", "Port acl use classid", "Port service acl enable", "Port qos domain", "Port policer valid", "Svlan tpid index, the corresponding svlan tpid is in EpeL2TpidCtl", "Rate of random threshold,0-0x7fff", "Flow direction", "Ingress", "Egress", "Both direction", "Property value", "Value", ((void *)0) };

char *ctc_cli_dbg_tool_read_i2c_cmd_help[] = { "Read cmd", "I2C raw cmd", "read address", "Address value", "Data len", "<0-32767>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_sf_state_cmd_help[] = { "Local MEP", "Set signal fail state", "<0-1>", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_oam_cfm_set_port_lm_enable_cmd_help[] = { "OAM module", "CFM", "Frame loss Measurement", "Global port", "<0-0x1DFF>", "Enable LM on this port", "Disable LM on this port", ((void *)0) };

char *ctc_cli_gb_port_set_scl_key_type_cmd_help[] = { "Port module", "<0-0x1DFF>", "SCL key type on port", "SCL id", "<0-1> There are 2 scl lookup. Each has its own feature.", "Direction", "Ingress", "Egress", "Type", "Cvid mapping", "Svid mapping", "Cvid+ccos mapping", "Svid+scos mapping", "Port mapping", "Double vlan mapping", "Mac SA based on vlan class", "Mac DA based on vlan class", "Ipv4 based on vlan class", "Ipv6 based on vlan class", "IP source guard based on port and mac", "IP source guard based on port and ip", "IP source guard based on ipv6", "IP tunnel for IPv4/IPv6 in IPv4, 6to4, ISATAP, GRE with/without  GRE key tunnel", "Rpf check for outer header, if set, scl-id parameter must be 1, scl-id 0 for tunnel scl-id 1 for rpf", "IPv6 based IP tunnel", "Rpf check for outer header, if set, scl-id parameter must be 1, scl-id 0 for tunnel-v6 scl-id 1 for rpf", "Disable scl key", ((void *)0) };

char *ctc_cli_common_enter_app_mode_cmd_help[] = { "Enter", "App mode", "Mode", ((void *)0) };

char *ctc_cli_dma_debug_on_cmd_help[] = { "Debugging functions", "dma module", "CTC layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_nh_add_aps_mpls_pop_nexthop_cmd_help[] = { "Nexthop module", "Add nexthop", "Aps mpls nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Aps bridge id", "<0-1023>", "Mpls pop nexthop", "APS protection path parameter", "Mpls pop nexthop payload operation type", "POP PHP operation", "PHP's Pipe mode", "PHP's Short Pipe mode", "PHP's Uniform mode", "Enable sequence order", "Sequence counter index, <0-255>", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "APS protection path parameter", "Mpls pop nexthop payload operation type", "POP PHP operation", "PHP's Pipe mode", "PHP's Short Pipe mode", "PHP's Uniform mode", "Enable sequence order", "Sequence counter index, <0-255>", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", ((void *)0) };

char *ctc_cli_gb_ftm_show_opf_type_info_cmd_help[] = { "Show running system information", "Offset pool freelist", "OPF type", "OPF type value", "All types", ((void *)0) };

char *ctc_cli_oam_cfm_clear_rmep_seq_num_fail_cmd_help[] = { "Remote MEP", "Sequence number check fail counter", "Clear the counter", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_IpeIntfMap_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show IpeIntfMap statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_oam_pf_interval_cmd_help[] = { "BFD", "Pf", "Interval", "Interval value, us", ((void *)0) };

char *ctc_cli_oam_cfm_clear_rmep_mac_status_fail_cmd_help[] = { "Remote MEP", "Mac status mismatch", "Clear Mac status mismatch", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_oam_cfm_clear_lmep_rdi_cmd_help[] = { "Local MEP", "RDI status", "Clear the defect status", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_gb_vlan_show_vlan_info_cmd_help[] = { "Show running system information", "VLAN", "<1-4095>", "Gchip id", "Global chip id", "Vlan member port", "Vlan receive state", "Vlan transmit state", "Vlan bridge state", "Vlan learning state", "Vrfid of vlan", "Stp instance in vlan", "IGMP snooping", "ARP exception action", "DHCP exception action", "Vlan tagged port", "Ipv4 based l2mc", "Ipv6 based l2mc", "Ptp", "Drop unknown ucast", "Drop unknown mcast", ((void *)0) };

char *ctc_cli_gb_chip_agent_debug_io_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Chip Agent module", "Driver IO", "Socket", "Encode and Decode", "All", ((void *)0) };

char *ctc_cli_oam_cfm_set_vlan_enable_cmd_help[] = { "OAM module", "CFM", "VLAN", "<1-4095>", "Ingress direction", "Egress direction", "Both direction", "Enable OAM on this vlan", "Disable OAM on this vlan", ((void *)0) };

char *ctc_cli_vlan_remove_ipv4_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Add one vlan classification entry", "IPv4 based vlan", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 source mask", "IPv4 mask in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "IPv4 destination mask", "IPv4 mask in A.B.C.D format", "MAC SA address", "MAC address in HHHH.HHHH.HHHH format", "MAC DA address", "MAC address in HHHH.HHHH.HHHH format", "Layer3 type", "Layer3 type ARP", "Layer3 type RARP", "Layer3 type IPv4", "Layer3 type id: 0~15", "Layer4 type", "Layer4 type UDP", "Layer4 type TCP", "Layer4 type id: 0~15", "Layer4 source port", "<0-65535>", "Layer4 destination port", "<0-65535>", ((void *)0) };

char *ctc_cli_port_set_reflective_bridge_cmd_help[] = { "Port module", "<0-0x1DFF>", "Reflective bridge", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_parser_show_l2_l2flex_cmd_help[] = { "Show running system information", "Parser module", "Layer 2 header of packet", "Flexible config", ((void *)0) };

char *ctc_cli_port_set_transmit_cmd_help[] = { "Port module", "<0-0x1DFF>", "Transmission port state", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_acl_add_mac_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", " Mac entry ", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Source CoS", "<0-7>", "<0-7>", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "Ether type", "<0-0xFFFF>", "<0-0xFFFF>", "Stag valid", "<0-1>", "Ctag valid", "<0-1>", "Arp operation code", "<0-0xFFFF>", "<0-0xFFFF>", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *ctc_cli_ipuc_add_ipv6_cmd_help[] = { "IPUC module", "IPV6", "Add ipv6 route", "VRF id of route <0-8191>", "IPv6 address in X:X::X:X format", "The length of Ipv6 mask <0~128>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "ECMP group", "TTL check enable", "RPF check enable", "ICMP redirect check", "ICMP error message check", "Copy to cpu", "Protocol-entry", "Self address, used for oam or ptp", "This route is a Neighbor", "This route is a Connect route", "L3 interface of this connect route", "L3 interface ID", "Stats id", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_gb_mpls_show_ilm__cmd_help[] = { "Show running system information", "Multi-Protocol Label Switching (MPLS)", "Ilm entries", ((void *)0) };

char *ctc_cli_pdu_l2_set_global_action_by_layer3_type_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Global action", "Based on layer3 type", "Layer3 type index", "<0-15>", "Entry valid, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "By pass all, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "Per-port action index", "<0-14>", "Copy to cpu", "<0-0xFF>,when great than 1,as 1 to process", ((void *)0) };

char *ctc_cli_oam_common_set_defect_to_cpu_cmd_help[] = { "OAM module", "OAM Common", "OAM defect to CPU", "Rx First PDU", "Mac status changed defect", "Src Mac mismatch defect", "Rx RDI defect", "Maid mismatch defect", "Low CCM defect", "ErrorCcmDefect RMEP not found", "ErrorCcmDefect CCM interval mismatch", "DLoc defect", "dCSF defect", "BFD init defect", "BFD up defect", "BFD down defect", "All defect to RDI", ((void *)0) };

char *ctc_cli_gb_vlan_show_vlan_profile_cmd_help[] = { "Do show", "Vlan profile", "All", ((void *)0) };

char *ctc_cli_parser_l3_mapping_l4type_cmd_help[] = { "Parser module", "Layer 3 header of packet", "Mapping layer4 type", "Mapping l4 type entry", "Entry index", "Index value <0,1,2,3,6>", "Layer3 header protocol", "Protocol value <0x0-0xFF>", "Layer3 header protocol mask", "Layer3 header protocol mask value <0x0-0xFF>", "Layer3 type", "Layer3 type value <0-15>", "Layer3 type mask", "Layer3 type mask value <0x0-0xFF>,when greater than 0xF,as 0xF to process", "Layer4 type", "Layer4 type value <0-15>", "Addition offset to be added", "Offset value<0-15>", ((void *)0) };

char *ctc_cli_gb_acl_show_status_cmd_help[] = { "Show", "ACL module", "Status", ((void *)0) };

char *ctc_cli_ipuc_set_global_property_cmd_help[] = { "IPUC module", "Global config for ip module", "Martian address check for ipv4", "Enable functions", "Disable functions", "Martian address check for ipv6", "Enable functions", "Disable functions", "Unmatched muticast MAC/IP address check", "Enable functions", "Disable functions", "IPSA lookup for IPv4 (only support in HUMBER)", "Enable functions", "Disable functions", "IPSA lookup for IPv6 (only support in HUMBER)", "Enable functions", "Disable functions", "Threshold for TTL check", "TTL threshold value <0~0xFF> ", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeHdrAdj_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeHdrAdj statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_set_syncup_internal_cmd_help[] = { "Statistics", "Set", "Sync up", "Internal", "Time", ((void *)0) };

char *ctc_cli_gb_port_set_scl_property_cmd_help[] = { "Port module", "<0-0x1DFF>", "SCL property on port", "SCL id", "<0-1> There are 2 scl lookup. Each has its own feature.", "Direction", "Ingress", "Egress", "Hash Type", "Hash 2vlan", "Hash svlan", "Hash cvlan", "Hash svlan cos", "Hash cvlan cos", "Hash mac sa", "Hash mac da", "Hash port mac sa", "Hash port mac da", "Hash ip", "Hash port ip", "Hash port", "Hash l2", "Hash disable", "Tcam Type", "Tcam mac", "Tcam ip", "Tcam vlan", "Tcam disable", "Class id", "1-61", ((void *)0) };

char *ctc_cli_parser_show_l4_flex_cmd_help[] = { "Show running system information", "Parser module", "Layer 4 header of packet", "Flexible operation", ((void *)0) };

char *ctc_cli_oam_cfm_add_remove_maid_cmd_help[] = { "OAM module", "CFM", "MA ID", "Add MAID", "Remove MAID", "MEP group id", "MEG id string", "MD Name", "MD name string", "MA Name", "MA name string", "Master chip id", "<0-29>", ((void *)0) };

char *ctc_cli_gb_oam_trpt_set_session_cmd_help[] = { "OAM module", "ThroughPut", "set", "ThroughPut session", "Session id: <0~3>", "Dest port", "<0-0x1DFF>", "Transmit mode", "Mode:0-packetnum, 1-continuely", "packet number", "<1-0xFFFF>", "pattern type", "Repeat pattern type", "<0-0xFFFFFFFF>", "Random pattern type", "Increment byte pattern type", "Decrement byte pattern type", "Increment word pattern type", "Decrement word pattern type", "Transmit rate", "<128-speed of dest port>(unit:Kbps)", "Packet size", "<64-16128>", "Enable tx seqnum insert", "<14-31>", "Loopback message", "Testing message", "File store packet header", "File name", ((void *)0) };

char *ctc_cli_acl_add_pbr_ipv4_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", "Pbr Ipv4 entry ", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Dscp", "<0-0x3F>", "<0-0x3F>", "Ip precedence", "<0-0x7>", "<0-0x7>", "Ip fragment information", "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment", "<0-1>", "Vrfid", "VRF id of route <0-8191>", "<0-0x3FFF>", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "<0-0xFF>", "Layer4 source port", "Use mask.", "Use range. ", "Use mask:Src Port data. Use range: Src port min. ", "<0-0xFFFF>", "Use mask:Src Port mask. Use range: Src port max. ", "<0-0xFFFF>", "Layer4 destination port", "Use mask.", "Use range. ", "Use mask:Dst Port data. Use range: Dst port min. ", "<0-0xFFFF>", "Use mask:Dst Port mask. Use range: Dst port max. ", "<0-0xFFFF>", "Icmp type", "<0-0xFF>", "<0-0xFF>", "Icmp code", "<0-0xFF>", "<0-0xFF>", "Tcp flags", "Match any tcp flag", "Match all tcp flags", "<0-0x3F>", "Permit", "Deny", "Forward to nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Ecmp nexthop", "TTL check", "ICMP redirect check", "Copy to cpu", ((void *)0) };

char *ctc_cli_dbg_tool_read_i2c_local_cmd_help[] = { "Read cmd", "I2C local cmd", "Address", "Address value", "Data len", "<0-32767>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_linkagg_remove_cmd_help[] = { "Linkagg module", "Remove linkagg group", "Linkagg", "<0-127>", ((void *)0) };

char *ctc_cli_dbg_tool_run_sgmac_prbs_xfipcs_cmd_help[] = { "Run cmd", "Sgmac-prbs cmd", "Pcsmode xfipcs", "Port id", "<0-12>", "Prbs-pattern", "0:Squre-wave; 1:Pseduo-Random; 2:PRBS31", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_scl_update_action_cmd_help[] = { "SCL module", "Update action", "SCL entry id", "<0~0x7FFFFFFF>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_ptp_set_clock_drift_cmd_help[] = { "PTP module", "<0-1>", "Clock drift", "The drift is negative", "Nanoseconds of drift", "<0-(10^9/quanta-1)>", ((void *)0) };

char *ctc_cli_dbg_tool_read_pcie_cfg_cmd_help[] = { "Read cmd", "PCI cfg cmd", "Address", "Address value", "<0-32767>", ((void *)0) };

char *ctc_cli_gb_vlan_add_protocol_vlan_cmd_help[] = { "VLAN module", "Vlan classification", "Add one vlan classification entry", "Protocol based vlan", "Ethertype", "<0x0000-0xFFFF>", "VLAN", "<1-4095>", "Cos", "<0-7>", ((void *)0) };

char *ctc_cli_gb_dma_tbl_rd_cmd_help[] = { "Dma module", "read table", "Asic table", "Table address", "Length of entry, unit:bytes", "Value <1-65535>", "Number of Entry", "Value <1-65535>", ((void *)0) };

char *ctc_cli_ipmc_debug_on_cmd_help[] = { "Debugging functions", "MCAST module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_bus_cmd_help[] = { "show chip info cmd", "bus info", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "ipe bus", "bus name", "bsr bus", "bus name", "bsr bus", "bus name", "bsr bus", "bus name", "bsr bus", "bus name", "bsr bus", "bus name", "bsr bus", "bus name", "bsr bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "epe bus", "bus name", "fib bus", "bus name", "fib bus", "bus name", "fib bus", "bus name", "fib bus", "bus name", "lpm bus", "bus name", "lpm bus", "bus name", "lpm bus", "bus name", "lpm bus", "bus name", "lpm bus", "bus name", "lpm bus", "bus name", "lpm bus", "bus name", "tcam bus", "bus name", "oam bus", "bus name", "oam bus", "bus name", "bus index", "index num", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_ipuc_show_l3_hash_state_cmd_help[] = { "Show running system information", "Show ipuc hash state", "Show ipuc hash state", ((void *)0) };

char *ctc_cli_scl_remove_entry_cmd_help[] = { "SCL module", "Remove one entry from software table", "SCL entry id", "<0~0x7FFFFFFF>", ((void *)0) };

char *ctc_cli_gb_acl_show_tcp_flags_cmd_help[] = { "Show", "ACL module", "Tcp flags", "Layer4 port range", ((void *)0) };

char *ctc_cli_l2_show_fdb_count_cmd_help[] = { "Show running system information", "Layer 2 module", "FDB table", "Count FDB entries", "Query condition", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Logic port", "Reference to global config", "Forwarding ID", "<0-16383>", "Port+Fid", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "<0-16383>", "All condition", "Static fdb table", "All dynamic fdb table", "Local chip dynamic fdb table", "Static and dynamic", ((void *)0) };

char *ctc_cli_scl_add_vlan_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0~0xFFFF0000>", "SCL entry id", "<0~0x7FFFFFFF>", "SCL entry priority", "<0~0xFFFFFFFF> :0 is lowest", "Vlan tcam entry", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "Stag valid", "0-1", "Ctag valid", "0-1", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_qos_clear_port_queue_stats_cmd_help[] = { "Clear functions", "QoS module", "QoS statstics", "Service id", "Value <1-0xFFFF", "Global port", "<0-0x1DFF>", "Queue id", "<0-7>", "Cpu reason", "Cpu reason id", ((void *)0) };

char *ctc_cli_nh_add_aps_xlate_cmd_help[] = { "Nexthop module", "Add nexthop", "APS Egress Vlan Editing nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Aps bridge id", "<0-1023>", "APS working path parameter", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "APS protection path parameter", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", ((void *)0) };

char *ctc_cli_parser_show_l3_trill_header_cmd_help[] = { "Show running system information", "Parser module", "Layer 3 header of packet", "Layer3 trill", "Header", "Inner tpid", "Rbridge channel ether type", ((void *)0) };

char *ctc_cli_aps_show_select_cmd_help[] = { "Show running system information", "Aps Module", "APS select", "APS select group", "<0-2047>", ((void *)0) };

char *ctc_cli_mirror_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Mirror module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_ftm_show_ftm_info_cmd_help[] = { "Show running system information", "Memory Allocation module", "Information of TCAM and Sram", ((void *)0) };

char *ctc_cli_nh_remove_ipuc_cmd_help[] = { "Nexthop module", "Delete nexthop", "IP unicast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_gb_oam_cfm_dump_ma_cmd_help[] = { "OAM module", "CFM", "Dump registers or tables", "MA table", "Chip id on linecard", "<0-1>", "MA table index", "<0-2047>", ((void *)0) };

char *ctc_cli_port_set_random_threshold_cmd_help[] = { "Port module", "<0-0x1DFF>", "The percentage of received packet for random log", "Flow direction ", "Ingress port random log percent", "Egress port random log percent", "Both direction", "Percent value, <0-100>", ((void *)0) };

char *ctc_cli_oam_cfm_rmep_enable_cmd_help[] = { "Remote MEP", "Enable remote MEP", "Disable remote MEP", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_dbg_tool_show_discard_cmd_help[] = { "Show cmd", "ipe/epe discard info", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_acl_add_ipv4_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", " Ipv4 entry ", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Source CoS", "<0-7>", "<0-7>", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "Ether type", "<0-0xFFFF>", "<0-0xFFFF>", "Stag valid", "<0-1>", "Ctag valid", "<0-1>", "Dscp", "<0-0x3F>", "<0-0x3F>", "Ip precedence", "<0-0x7>", "<0-0x7>", "Ip fragment information", "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment", "Ip option", "<0-1>", "Routed packet", "<0-1>", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "<0-0xFF>", "Layer4 source port", "Use mask.", "Use range. ", "Use mask:Src Port data. Use range: Src port min. ", "<0-0xFFFF>", "Use mask:Src Port mask. Use range: Src port max. ", "<0-0xFFFF>", "Layer4 destination port", "Use mask.", "Use range. ", "Use mask:Dst Port data. Use range: Dst port min. ", "<0-0xFFFF>", "Use mask:Dst Port mask. Use range: Dst port max. ", "<0-0xFFFF>", "Icmp type", "<0-0xFF>", "<0-0xFF>", "Icmp code", "<0-0xFF>", "<0-0xFF>", "Tcp flags", "Match any tcp flag", "Match all tcp flags", "<0-0x3F>", "Igmp type", "<0-0xFF>", "<0-0xFF>", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Arp packet", "<0-1>", "Arp operation code", "<0-0xFFFF>", "<0-0xFFFF>", "Sender Ip address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Target Ip address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Ipv4 packet", "<0-1>", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *ctc_cli_l2mcast_add_member_cmd_help[] = { "Layer 2 module", "L2 multicast", "Add operation", "The member of multicast group", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Forwarding ID", "<0-16383>", "Global port /Remote chip id", "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)", "Next hop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Indicate the member is remote chip entry", "Enable transport by service queue", ((void *)0) };

char *ctc_cli_ptp_show_sync_intf_rx_code_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Sync interface input time code", ((void *)0) };

char *ctc_cli_dbg_tool_show_mac_QuadMac_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "mac status", "show QuadMac statistic", "QuadMac index", "<0-11>", "QuadMac internal sub index", "<0-3>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_linkagg_debug_on_cmd_help[] = { "Debugging functions", "Linkagg module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_stats_destroy_statsid_cmd_help[] = { "Statistics", "Destroy", "Stats id", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_gb_oam_cfm_dump_mep_index_cmd_help[] = { "OAM module", "CFM", "Dump registers or tables", "MEP table", "MEP table index", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_security_mac_security_show_port_security_cmd_help[] = { "Show running system information", "Security module", "Port based mac security", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_chip_show_serdes_mode_cmd_help[] = { "show", "chip module", "serdes", "<0~5>", "3.125G/1.25G", ((void *)0) };

char *ctc_cli_learning_aging_set_learning_action_cmd_help[] = { "Learning module", "Learning action", "Learning action always cpu", "Learning action full to cpu", "Cache threshod", "0-15", "Learning action cache only", "Cache threshod", "0-15", "Learning action dont learn", "Learning mac table full", "Learning mac table full enable", "Learning mac table full disable", ((void *)0) };

char *ctc_cli_ptp_debug_on_cmd_help[] = { "Debugging functions", "PTP module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_port_set_untag_dft_vid_enable_cmd_help[] = { "Port module", "<0-0x1DFF>", "Untag vlan id when transmit packet vid equal to PVID", "Enable functions", "Untag vlan id on stag", "Untag vlan id on ctag", ((void *)0) };

char *ctc_cli_acl_uninstall_group_cmd_help[] = { "ACL module", "Uninstall from hardware", "ACL group id", "<0xFFFF0002>", ((void *)0) };

char *ctc_cli_nh_cfg_max_ecmp_cmd_help[] = { "Nexthop module", "Max_ecmp the maximum ECMP paths allowed for a route.<1-8>", "Max_ecmp the maximum ECMP paths allowed for a route.<1-8>", ((void *)0) };

char *ctc_cli_security_mac_security_set_system_mac_limit_cmd_help[] = { "Security module", "Mac limit", "System based mac limit", "The action of mac limit", "Do nothing", "Forwarding packet", "Discard packet", "Redirect to cpu", ((void *)0) };

char *humber_cli_aps_set_bridge_cmd_help[] = { "Aps Module", "Setting", "Aps bridge", "Aps bridge group", "<0-2047>", "Aps bridge to protection path", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_stats_show_mac_cfg_cmd_help[] = { "Show running system information", "Statistics", "MTU1 packet length,packet length larger than this consider as oversized packet.Default is 1518B", "MTU2 packet length,packet length larger than this consider as jumbo packet.Default is 1536B,must greater than MTU1", "Show stats dot1q-subtract-byte port GPHYPORT_ID", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_scl_get_multi_entry_cmd_help[] = { "Show", "SCL module", "SCL group id", "<0xFFFF000C>", "Entry size", "0: show entry count in this group. 1 ~  : show entry IDs in that number", ((void *)0) };

char *ctc_cli_dbg_tool_write_pcie_mem_cmd_help[] = { "Write cmd", "PCI mem cmd", "Address", "Address value", "Set value", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_vlan_remove_all_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Flush all entries", "Mac based vlan", "Ipv4 based vlan", "Ipv6 based vlan", "Protocol based vlan", ((void *)0) };

char *ctc_cli_scl_debug_show_cmd_help[] = { "Show running system information", "Debugging functions", "SCL module", ((void *)0) };

char *ctc_cli_l2_set_default_entry_feature_cmd_help[] = { "Layer 2 module", "FDB table", "Vlan default entry", "Forwarding ID", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Use logic port check", "Unknown ucast mac to drop", "Unknown mcast mac to drop", "Protocol exception to cpu", "Learning disable", "Recover the default unknown mac action", ((void *)0) };

char *ctc_cli_ipuc_remove_ipv6_tunnel_cmd_help[] = { "IPUC module", "Remove ipv6 route", "This route is ip tunnel", "Remove ip tunnel route", "IPv6 address in X:X::X:X format", "VRF id of route <0-8191>", "VRF id of route <0-8191>", "Tunnel payload layer 4 type (default is IPv4)", "Tunnel payload layer 4 type: \r\n0 for GRE \r\n1 for IPv4 \r\n2 for IPv6", "Set Gre key value", "Gre key value <1-4294967295>", "Set IP Sa", "IPv6 address in X:X::X:X format", ((void *)0) };

char *ctc_cli_gb_vlan_add_egress_vlan_mapping_default_entry_cmd_help[] = { "VLAN module", "Vlan mapping", "Use egress vlan mapping", "Add vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Mismatch action", "Dicard packet", "Normal forwaring", ((void *)0) };

char *ctc_cli_dbg_tool_check_packet_capture_cmd_help[] = { "Check cmd", "Packet buffer capture cmd", "Port id", "<0-59>", "Packet length", "<0-4096B>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_l2_show_master_cmd_help[] = { "Show running system information", "Layer 2 module", "master data", ((void *)0) };

char *ctc_cli_scl_add_port_svlan_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port svlan hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "Direction", "Ingress direction", "Egress direction", "Service VLAN", "<1-4095>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_remove_ip_tunnel_cmd_help[] = { "Nexthop module", "Delete nexthop", "IP Tunnel nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_pdu_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Pdu modue", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_port_set_vlan_domain_cmd_help[] = { "Port module", "<0-0x1DFF>", "Set vlan domain when the cvlan TPID is the same as svlan TPID", "Cvlan domain", "Svlan domain", ((void *)0) };

char *ctc_cli_vlan_set_drop_unknown_pkt_cmd_help[] = { "VLAN module", "<1-4095>", "Drop unknown ucast packet", "Drop unknown mcast packet", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_ptp_show_tod_intf_tx_code_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Tod interface output time code config", ((void *)0) };

char *ctc_cli_gb_oam_set_mep_index_by_sw_cmd_help[] = { "OAM module", "OAM MEP index alloc mode", "OAM MEP index alloc mode by system", ((void *)0) };

char *ctc_cli_parser_l3_set_trill_header_cmd_help[] = { "Parser module", "Layer 3 header of packet", "Layer3 trill", "Header", "Inner-tpid", "Inner-tpid value <0x0 - 0xFFFF>", "Rbridge channel ether type", "Type value <0x0 - 0xFFFF>", ((void *)0) };

char *ctc_cli_gb_port_show_port_info_cmd_help[] = { "Show running system information", "Port module", "<0-0x1DFF>", "SCL key type", "SCL id", "<0-1> There are 2 scl lookup. Each has its own feature.", "Vlan Range", "Direction", "Ingress", "Egress ", ((void *)0) };

char *ctc_cli_stats_set_threshold_cfg_cmd_help[] = { "Statistics", "deal with general options pkt cnt threshold", "deal with general options byte cnt threshold", "deal with general options fifo depth threshold", "packet-threshold or byte-threshold maxsize: 4095, fifo-depth-threshold maxsize: 0~31", ((void *)0) };

char *ctc_cli_nh_add_aps_mpls_push_nexthop_cmd_help[] = { "Nexthop module", "Add nexthop", "APS MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Aps bridge id", "<0-1023>", "Mpls push nexthop", "Enable H-VPLS in VPLS network", "Destination logic port in VPLS network", "MPLS vpls port value <0-8191>", "APS working path parameter", "Mpls push nexthop payload operation type", "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)", "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "First label is martini control word", "Martini sequence type is none", "Martini sequence type is per-pw", "Martini per-pw sequence index", "Sequence index value", "Martini sequence type is global sequence type0", "Martini sequence type is global sequence type1", "Control word", "Control word value", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "Statistics", "Stats id", "APS protection path parameter", "Mpls push nexthop payload operation type", "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)", "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "First label is martini control word", "Martini sequence type is none", "Martini sequence type is per-pw", "Martini per-pw sequence index", "Sequence index value", "Martini sequence type is global sequence type0", "Martini sequence type is global sequence type1", "Control word", "Control word value", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "Statistics", "Stats id", ((void *)0) };

char *ctc_cli_security_storm_ctl_set_global_cfg_cmd_help[] = { "Security module", "Storm control", "Ipg", "Enable functions", "Disable functions", "Ucast control mode", "Enable functions", "Disable functions", "Mcast control mode", "Enable functions", "Disable functions", "Granularity", "0:100, 1:1000, 2:10000, default and other:100, pps or bps", ((void *)0) };

char *ctc_cli_scl_add_port_dvlan_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port double vlan hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "Direction", "Ingress direction", "Egress direction", "Service VLAN", "<1-4095>", "Customer VLAN", "<1-4095>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_dbg_tool_ipe_flow_chart_cmd_help[] = { "show bus", "debug tool", "ipe", "flow chart", ((void *)0) };

char *ctc_cli_ipuc_remove_ipv4_tunnel_cmd_help[] = { "IPUC module", "This route is ip tunnel", "Remove ip tunnel route", "IPv4 address in A.B.C.D format", "VRF id of route <0-8191>", "VRF id of route <0-8191>", "Tunnel payload layer 4 type (default is IPv6)", "Tunnel payload layer 4 type: \r\n0 for GRE \r\n1 for IPv4 \r\n2 for IPv6", "Set Gre key value", "Gre key value <1-4294967295>", "Set IP Sa", "IPv4 address in A.B.C.D format", "Use flex key", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeAclQos_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeAclQos statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_fast_enter_internal_debug_mode_cmd_help[] = { "Enter Internal Debug mode", ((void *)0) };

char *ctc_cli_chip_set_gpio_mode_cmd_help[] = { "chip module", "set operate", "gpio", "gpio id", "<0~15>", "gpio mode", "0:input 1:output 2:special function", ((void *)0) };

char *ctc_cli_common_help_cmd_help[] = { "Description of the interactive help system", ((void *)0) };

char *ctc_cli_oam_tp_remove_chan_cmd_help[] = { "OAM module", "MPLS TP Y.1731", "MEP lookup channel", "Remove MEP lookup channel", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", "Master chip id for OAM", "<0-29>", ((void *)0) };

char *ctc_cli_dbg_tool_show_oam_OamAutoGenPkt_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "oam status", "show OamAutoGenPkt statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_scl_show_group_info_cmd_help[] = { "Show", "SCL module", "Group info", "SCL group id", "<0xFFFF000C>", ((void *)0) };

char *ctc_cli_qos_set_port_shape_cmd_help[] = { "QoS module", "QoS shaping", "Global port", "<0-0x1DFF>", "Cancel port shaping", "Config PIR (Peak Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", "Config PBS (Peak Burst Size)", "PBS value unit bytes", ((void *)0) };

char *ctc_cli_nh_add_iloop_cmd_help[] = { "Nexthop module", "Add nexthop", "IPE Loopback nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Loopback local phy port", "<0-255>", "CustomerId is valid(eg. VC label)", "Map priority and color from exp of VC label", "Specify inner packet type", "Inner packet type, if set, indicates that the packet is from a VPLS tunnel port", "VPLS port, If set, indicates that the packet is from a VPLS tunnel port", "Control word using sequence order", "Sequence counter index, <0-255>", "Words to be removed from loopback packet header, not include bytes of customerId", "One word stands for 4 bytes, not include customerId 4 bytes, the ranges is 0~31", "If set, indicates that the packets will do service queue processing", ((void *)0) };

char *ctc_cli_ipmc_add_ipv4_member_cmd_help[] = { "IPMC module", "Add", "Member of the ipmc group", "Group address, class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN interface", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan port", "Physical interface/remote chip id", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Add a member port for a IP based l2 mcast", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Indicate the member is remote chip entry", "Add member by nexthop, can be used in ipmc and ip based l2mc", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_ptp_show_adjust_delay_cmd_help[] = { "Show running system information", "PTP module", "Delay value used for adjust PTP message pass the device", "<0-0x1DFF>", "Delay type", "Latency from phy to mac on RX", "Latency from mac to phy on TX", "Asymmetry on ingress", "Asymmetry on egress", "Path delay", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_nhop_cmd_help[] = { "Local MEP", "NextHop ID for MEP out", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_security_port_isolation_set_route_obey_cmd_help[] = { "Security module", "Port isolation", "Route obey isolate", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_common_fast_enter_app_mode_cmd_help[] = { "Enter app mode", ((void *)0) };

char *ctc_cli_gb_interrupt_clear_status_cmd_help[] = { "Interrupt module", "<0-1>", "Interrupt sup-level type", "<0-0xFFFFFFFF>", "Interrupt sub-level type", "<0-0xFFFFFFFF>", "Clear", ((void *)0) };

char *ctc_cli_common_quit_cmd_help[] = { "Exit current mode and down to previous mode", ((void *)0) };

char *ctc_cli_acl_debug_show_cmd_help[] = { "Show running system information", "Debugging functions", "ACL module", ((void *)0) };

char *ctc_cli_mirror_add_session_cmd_help[] = { "Mirror module", "Mirror destination", "Add destination for mirror session", "Configure a mirror session", "Session_id range <0-3>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", "Mirror received and transmitted traffic", "Port session", "Vlan session", "Acl session", "Acl priority id", "GB range <0-3>, HB range <0-1>", "Cpu session", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_chip_show_chip_info_cmd_help[] = { "show", "chip module", "chip information", ((void *)0) };

char *ctc_cli_l3if_set_vrf_stats_en_cmd_help[] = { "L3 interface module", "Statistics", "VRF id of route", "VRF id of route <0-8191>", "Enable", "Disable", ((void *)0) };

char *ctc_cli_dbg_tool_show_interrupt_cmd_help[] = { "Show cmd", "interrupt info", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_qos_set_queue_wdrr_weight_cmd_help[] = { "QoS module", "QoS schedule", "Queue wdrr", "Service identification", "<0-0xFFFF>", "Global port", "<0-0x1DFF>", "Confirm green class weight", "Exceed yellow class weight", "Config WDRR weight", "Weight of queue 0, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 1, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 2, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 3, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 4, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 5, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 6, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Weight of queue 7, ""<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", "Queue id", "<0-7>", "Config WDRR weight", "<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", ((void *)0) };

char *ctc_cli_mirror_show_port_mirror_state_cmd_help[] = { "Show running system information", "Mirror module", "Global port", "<0-0x1DFF>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_BufStore_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show BufStore statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_l2_show_fdb_hw_cmd_help[] = { "Show running system information", "Layer 2 module", "FDB table", "HW table", "FDB entry information", "FDB detail information", "Query condition", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "MAC+Fid", "MAC address in HHHH.HHHH.HHHH format", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Forwarding ID", "<0-16383>", "Port+Fid", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "<0-16383>", "All condition", "Static fdb table", "All dynamic fdb table", "Local chip dynamic fdb table", "Static and dynamic", "File store fdb entry", "File name", ((void *)0) };

char *ctc_cli_parser_l4_type_cmd_help[] = { "Parser module", "Layer 4 header of packet", "Tcp ", "Udp ", "Gre", "IpinIp", "Ipv6inIp", "Icmp", "Igmp", "enable l4 type parser", "disable l4 type parser", ((void *)0) };

char *ctc_cli_nh_add_ecmp_cmd_help[] = { "Nexthop module", "Add nexthop", "ECMP group", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Member nexthop ID 0", "Member nexthop ID 1", "Member nexthop ID 2", "Member nexthop ID 3", "Member nexthop ID 4", "Member nexthop ID 5", "Member nexthop ID 6", "Member nexthop ID 7", ((void *)0) };

char *ctc_cli_pdu_show_l2_port_action_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "Per-port l2pdu action", "Port", "<0-0x1DFF>", "Per-port action index", "<0-14>", "Action type", ((void *)0) };

char *ctc_cli_mirror_set_mirror_discard_cmd_help[] = { "Mirror module", "Packets dropped", "Port session", "Vlan session", "Acl session", "All port and vlan and acl session", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", "Mirror received and transmitted traffic", "Packet enable to log when dropped", "Packet disable to log when dropped", ((void *)0) };

char *ctc_cli_mpls_ac_remove_port_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "L2vpn attachment circuit entry", "Remove l2vpn ac", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_dbg_tool_monitor_sgmac_prbs_xauipcs_cmd_help[] = { "Monitor cmd", "Sgmac-prbs cmd", "Pcsmode xauipcs", "Port id", "<4-12>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_csf_type_cmd_help[] = { "Local MEP", "Client signal fail type", "Client loss of signal", "Client forward defect indication", "Client remote defect indication", "Client defect clear indication", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_learning_aging_debug_on_cmd_help[] = { "Debugging functions", "Learning and Aging module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeHdrEdit_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeHdrEdit statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_FwdMsg_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show FwdMsg statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_common_set_defect_to_rdi_cmd_help[] = { "OAM module", "OAM Common", "OAM defect to RDI", "Mac status changed defect", "Src Mac mismatch defect", "Maid mismatch defect", "Low CCM defect", "ErrorCcmDefect RMEP not found", "ErrorCcmDefect CCM interval mismatch", "DLoc defect", "dCSF defect", "All defect to RDI", ((void *)0) };

char *ctc_cli_learning_aging_show_config_cmd_help[] = { "Show running system information", "Learning and Aging module", "Config param", "Aging table", "MAC table", "SCL table", ((void *)0) };

char *ctc_cli_gb_mpls_show_count_cmd_help[] = { "Show running system information", "Multi-Protocol Label Switching (MPLS)", "Show mpls ilm count", ((void *)0) };

char *ctc_cli_scl_remove_all_entry_cmd_help[] = { "SCL module", "Remove all entries of a specified group from software table", "SCL group id", "<0xFFFF000C>", ((void *)0) };

char *ctc_cli_acl_copy_entry_cmd_help[] = { "ACL module", "ACL entry id", "<0~0xFFFFFFFF>", "Copy to", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_ptp_clear_sync_intf_rx_code_cmd_help[] = { "PTP module", "<0-1>", "Clear sync interface input time code", ((void *)0) };

char *ctc_cli_port_set_bridge_cmd_help[] = { "Port module", "<0-0x1DFF>", "L2 bridge", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_scl_add_port_mac_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port mac hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>" "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Mac is destination address or not ", "0-1", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en_cmd_help[] = { "Local MEP", "CCM use unicast Mac Da in P2P mode", "Enable CCM use unicast Mac Da in P2P mode", "Disable CCM use unicast Mac Da in P2P mode", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_show_oam_OamFwd_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "oam status", "show OamFwd statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_set_lbm_proc_in_asic_cmd_help[] = { "OAM module", "CFM", "Set LBM process in ASIC", "Enable LBM process in ASIC", "Disable LBM process in ASIC", ((void *)0) };

char *ctc_cli_dbg_tool_read_reg_cmd_help[] = { "read address value cmd", "address 0xXXXXXXXX", ((void *)0) };

char *ctc_cli_qos_set_cpu_reason_map_cmd_help[] = { "QoS module", "Cpu Resaon Id", "<0-MAX> refer to cli show qos cpu-reason>", "Mapping to", "Queue id", "<0-7>", "Reason group", "<0-15>", "Cos", "<0-7>", ((void *)0) };

char *ctc_cli_nh_add_stats_cmd_help[] = { "Nexthop module", "Add action", "Statistics", "Nexthop id", ((void *)0) };

char *ctc_cli_scl_add_ipv6_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0~0xFFFF0000>", "SCL entry id", "<0~0x7FFFFFFF>", "SCL entry priority", "<0~0xFFFFFFFF> :0 is lowest", "Ipv6 tcam entry", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "IPv6 source address", "IPv6 address in X:X::X:X format", "IPv6 mask in X:X::X:X format", "IPv6 destination address", "IPv6 address in X:X::X:X format", "IPv6 mask in X:X::X:X format", "Ipv6 Flow label", "<0-0xFFFFF>", "<0-0xFFFFF>", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "<0-0xFF>", "Layer4 source port", "<0-0xFFFF>", "<0-0xFFFF>", "Layer4 destination port", "<0-0xFFFF>", "<0-0xFFFF>", "Icmp type", "<0-0xFF>", "<0-0xFF>", "Icmp code", "<0-0xFF>", "<0-0xFF>", "Dscp", "<0-0x3F>", "<0-0x3F>", "Ether type", "<0-0xFFFF>", "<0-0xFFFF>", "Layer 4 Type", "<0-15>", "<0-0xF>", "Ip option", "<0-1>", "Ip fragment information", "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment", "Ip precedence", "<0-0x7>", "<0-0x7>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_gb_ipuc_show_state_cmd_help[] = { "Show running system information", "IPUC module", "IPUC configure and state", ((void *)0) };

char *ctc_cli_parser_set_ecmp_hash_cmd_help[] = { "Parser module", "Ecmp hash field", "Layer3 ip", "Add l3protocol into ecmp hash", "Add dscp into ecmp hash", "Add ipv6 flow label into ecmp hash", "Add ipsa into ecmp hash", "Add ipda into ecmp hash", "Layer4", "Add src port into ecmp hash", "Add dst port into ecmp hash", "Add gre key into ecmp hash", "Mpls", "Add mpls l3protocol into ecmp hash", "Add mpls dscp into ecmp hash", "Add mpls ipv6 flow label into ecmp hash", "Add mpls ipsa into ecmp hash", "Add mpls ipda into ecmp hash", "Fcoe", "Add sid into ecmp hash", "Add did into ecmp hash", "Trill", "Add inner vlan id into ecmp hash", "Add ingress nickname into ecmp hash", "Add egress nickname into ecmp hash", "Layer2 ethernet", "Add cos into ecmp hash", "Add macsa into ecmp hash", "Add macda into ecmp hash", ((void *)0) };

char *ctc_cli_linkagg_create_cmd_help[] = { "Linkagg module", "Create linkagg group", "Linkagg", "<0-127>", "static linkagg group", "dlb linkagg group", "hw-sync group", ((void *)0) };

char *ctc_cli_acl_set_entry_priority_cmd_help[] = { "ACL module", "ACL entry id", "<0~0xFFFFFFFF>", "ACL entry priority", "<0~0xFFFFFFFF> :0 is lowest", ((void *)0) };

char *ctc_cli_gb_l2_show_fdb_detail_cmd_help[] = { "Show running system information", "Layer 2 module", "FDB table", "FDB detail information", "Query condition", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "MAC+Fid", "MAC address in HHHH.HHHH.HHHH format", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Forwarding ID", "<0-16383>", "Port+Fid", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "<0-16383>", "All condition", "Static fdb table", "All dynamic fdb table", "Local chip dynamic fdb table", "Static and dynamic", "File store fdb entry", "File name", ((void *)0) };

char *ctc_cli_scl_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "SCL module", "Ctc layer", "Sys layer", "SCL Label", "SCL entry", ((void *)0) };

char *ctc_cli_ipmc_add_ipv4_default_entry_cmd_help[] = { "IPMC module", "Add", "Default action", "Send the packet that hit the default entry to cpu", "Drop the packet that hit the default entry", ((void *)0) };

char *ctc_cli_ipuc_add_ipv4_tunnel_cmd_help[] = { "IPUC module", "This route is ip tunnel", "Add ip tunnel route", "IPv4 address in A.B.C.D format", "VRF id of route <0-8191>" " used to do route lookup (HUMBER support), Inner route will lookup by this VRFID (HUMBER not support)", "VRF id of route <0-8191>" " used to do route lookup (HUMBER support), Inner route will lookup by this VRFID (HUMBER not support)", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Tunnel payload layer 4 type (default is IPv6)", "Tunnel payload layer 4 type: \r\n0 for GRE \r\n1 for IPv4 \r\n2 for IPv6", "Set Gre key value", "Gre key value <1-4294967295>", "Set IP Sa", "IPv4 address in A.B.C.D format", "ECMP group", "Do ttl check for tunnel outer header (HUMBER not support)", "Use outer ttl for later process , or use inner ttl (HUMBER not support)", "Packet will do router by tunnel outer header , or use payload header (HUMBER not support)", "ISATAP source adderss check (HUMBER not support)", "Copy to CPU (Only HUMBER support)", "RPF check enable (HUMBER not support)", "L3 interface of this tunnel route, only need when do RPF check and packet do router by inner header", "The 1st L3 interface ID", "The 1st L3 interface ID <0-1022>, 0 means invalid", "The 2nd L3 interface ID", "The 2nd L3 interface ID <0-1022>, 0 means invalid", "The 3rd L3 interface ID", "The 3rd L3 interface ID <0-1022>, 0 means invalid", "The 4th L3 interface ID", "The 4th L3 interface ID <0-1022>, 0 means invalid", "RPF check fail packet will send to cpu", "Use outer header info do acl lookup, default use the inner", "Service Id", "<1-0xFFFF>", "Logic port", "<0-0x1DFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Enable stats", "Stats ID", "Discard", "Use flex key", ((void *)0) };

char *ctc_cli_qos_set_reason_shape_cmd_help[] = { "QoS module", "QoS shaping", "Cpu reason id", "<0-MAX> refer to cli [show qos cpu-reason-id]", "Cancel cpu reason shaping", "Config PIR (Peak Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", ((void *)0) };

char *ctc_cli_qos_set_port_queue_class_cmd_help[] = { "QoS module", "QoS schedule", "Queue class", "Service identification", "<0-0xFFFF>", "Global port", "<0-0x1DFF>", "Queue Id", "<0-7>", "Confirm green class", "Exceed yellow class", "<0-3>", ((void *)0) };

char *ctc_cli_aps_update_raps_member_cmd_help[] = { "Aps Module", "Aps bridge", "Raps", "Aps bridge group", "<0-2047>", "Add", "Remove", "Member port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Remote chip", ((void *)0) };

char *ctc_cli_qos_set_group_wdrr_weight_cmd_help[] = { "QoS module", "QoS schedule", "Group", "Service identification", "<0-0xFFFF>", "Global port", "<0-0x1DFF>", "Config WDRR weight", "<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)", ((void *)0) };

char *ctc_cli_dbg_tool_write_gephy_cmd_help[] = { "Write cmd", "GEPHY cmd", "Mdio bus", "<0-1>", "Phy address", "<0-0x1F>", "Reg address", "<0-0x1F>", "Chip id on linecard", "<0-1>", "Set value", "value", ((void *)0) };

char *ctc_cli_gb_mpls_show_mpls_status_cmd_help[] = { "Show running system information", "Multi-Protocol Label Switching (MPLS)", "MPLS Status", ((void *)0) };

char *ctc_cli_security_ip_src_guard_add_entry_cmd_help[] = { "Security module", "Ip source guard", "Add ip source guard entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "IPv4 source address", "IPv4 address in A.B.C.D format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Vlan", "<1-4095>", "Vlan id is svlan id", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_OobFc_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show OobFc statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_aps_set_aps_bridge_group_cmd_help[] = { "Aps Module", "setting", "Aps bridge", "Aps bridge group", "<0-2047>", "Aps type", "Used for normal l2 aps protection", "Used for mpls aps protection", "Working path gport", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Working path L3 interface id", "INTERFACE ID", "Next working aps group id", "0-1023", "Protection path gport", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Protection path L3 interface id", "INTERFACE ID", "Next protection aps group id", "0-1023", "Port aps use physical isolated path", "Raps enable", "Raps groupid", "Raps Group ID,it is allocated in platform adaption Layer", "Hw-based-aps-en,hardware based aps for link down event", ((void *)0) };

char *ctc_cli_ipmc_mcast_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "MCAST module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_EpeNextHop_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show EpeNextHop statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *common_cli_show_memory_module_detaill_cmd_help[] = { "Show running system information", "Memory information", "Memory pool", "Memory statistics", "FDB module", "Mpls module", "Queue module", "IPUC module", "IPMC module", "Nexthop module", "SCL module", "L3 interface module", "Port module", "VLAN module", "OAM module", "Sort-key module", "Linklist module", "Avl Tree ", "CTCCLI", "LIBCTCCLI", "Vector", "Hash", "Share pool", "Ptp", "Ftm", "Stacking", "Dma", "Storm control", "Mirror", "SAL", "OPF", "L3_HASH", "FPA", "ACL", "Sync ether", "Rpf", "Aps", "All", "Detail", ((void *)0) };

char *ctc_cli_stacking_set_break_en_cmd_help[] = { "Stacking", "Break if exist break point", "Full mesh topo", "Enable", "Disable", ((void *)0) };

char *ctc_cli_vlan_add_ipv4_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Add one vlan classification entry", "IPv4 based vlan", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 source mask", "IPv4 mask in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "IPv4 destination mask", "IPv4 mask in A.B.C.D format", "MAC SA address", "MAC address in HHHH.HHHH.HHHH format", "MAC DA address", "MAC address in HHHH.HHHH.HHHH format", "Layer3 type", "Layer3 type ARP", "Layer3 type RARP", "Layer3 type IPv4", "Layer3 type id: 0~15", "Layer4 type", "Layer4 type UDP", "Layer4 type TCP", "Layer4 type id: 0~15", "Layer4 source port", "<0-65535>", "Layer4 destination port", "<0-65535>", "Vlan", "<1-4095>", "Cos", "<0-7>", ((void *)0) };

char *ctc_cli_vlan_set_learning_cmd_help[] = { "VLAN module", "<1-4095>", "Mac learning on the vlan", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_oam_bfd_update_rmep_cmd_help[] = { "Remote MEP", "BFD", "Update remote BFD MEP", "IP BFD OAM option", "My Discriminator", "My Discriminator value, <0~4294967295>", "MPLS BFD OAM option", "My Discriminator", "My Discriminator value, <0~4294967295>", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", "MEP is active or not", "Enable remote MEP", "Disable remote MEP", "Set signal fail state", "<0-1>", "BFD remote diag", "<0~31>", "BFD remote state", "Admin down :0 ; Down: 1; Init : 2; Up :3", "Your Discriminator", "Your Discriminator value, <0~4294967295>", "Rx timer", "Required min rx interval", ((void *)0) };

char *show_l3if_vmac_low_8bits_cmd_help[] = { "Show running system information", "L3 interface module", "VRRP mac", "Low 8 bits", "L3if ID", "<0-1022>", "The index of vmac entry", "Index range <0-3>", ((void *)0) };

char *ctc_cli_qos_set_port_queue_shape_cmd_help[] = { "QoS module", "QoS shaping", "Queue shaping", "Service id", "Value <1-0xFFFF", "Global port", "<0-0x1DFF>", "Queue id", "<0-7>", "Cpu reason", "Cpu reason id", "Cancel queue shaping", "Config CIR (Commit Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", "Config PIR (Peak Information Rate)", "<0-10000000>, rate in Kbps (1000/sec)", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_ELoop_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show ELoop statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_packet_debug_show_cmd_help[] = { "Show running system information", "Debugging functions", "Packet module", "Ctc layer", "System layer", ((void *)0) };

char *ctc_cli_mpls_debug_show_cmd_help[] = { "Show running system information", "Debugging functions", "Multi-Protocol Label Switching (MPLS)", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_parser_l2_set_l2flex_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Flexible config", "Mac da basic offset", "Offset value <0-26>", "Header protocol basic offset", "Offset value <0-30>", "Minium length", "Length value <0x0-0xFF>", "Layer2 basic offset", "Offset value <0x0-0xFF>", ((void *)0) };

char *ctc_cli_dbg_tool_scan_xgphy_sp_dis_cmd_help[] = { "Scan cmd", "Set xgphy special scan disable cmd", "Disable functions", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l3if_show_mpls_property_cmd_help[] = { "Show running system information", "L3 interface module", "L3if ID", "<0-1022>", "MPLS property", "MPLS is enabled", "MPLS label space id", ((void *)0) };

char *ctc_cli_port_mapping_set_mapping_mode_cmd_help[] = { "Port module", "Port mapping", "Globally enable mapping", "Globally disable mapping", ((void *)0) };

char *ctc_cli_parser_global_set_hash_type_cmd_help[] = { "Parser module", "Layer2 3 4", "Hash", "Ecmp hash type, if set 0 use xor, else use crc", "Type value <0x0 - 0xFF>,when greater than 1,as 1 to process", "Linkagg hash type, if set 0 use xor, else use crc", "Type value <0x0 - 0xFF>,when greater than 1,as 1 to process", ((void *)0) };

char *ctc_cli_qos_set_resrc_mgr_enable_cmd_help[] = { "QoS module", "Resource manage", "Globally enable", "Globally disable", ((void *)0) };

char *ctc_cli_qos_show_port_queue_stats_cmd_help[] = { "Show running system information", "QoS module", "QoS statstics", "Service id", "Value <1-0xFFFF", "Global port", "<0-0x1DFF>", "Queue id", "<0-7>", "Cpu reason", "Cpu reason id", ((void *)0) };

char *ctc_cli_dbg_tool_packet_flow_diag_start_cmd_help[] = { "packet", "flow", "diagnose", "function start", "<0-1>", ((void *)0) };

char *ctc_cli_gb_oam_show_bfd_session_state_cmd_help[] = { "Show running system information", "BFD", "Session", "State", "Session number", "Session number value", ((void *)0) };

char *ctc_cli_l3if_debug_on_cmd_help[] = { "Debugging functions", "L3 interface module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_mpls_remove_pw_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "L2vpn pw mapping", "Remove l2vpn pw mapping", "Mpls label space", "Label space id, <0-255>, space 0 is platform space", "L2vpn vc incoming label, <0-1048575>", ((void *)0) };

char *ctc_cli_pdu_show_l3if_action_action_cmd_help[] = { "Show running system information", "Pdu modue", "Layer3 PDU", "Per-l3if l3pdu action ", "Layer3 interface", "<0-1022>", "Per-l3if l3pdu action index", "<0-14>", "Action type", ((void *)0) };

char *ctc_cli_port_set_vlan_mapping_cmd_help[] = { "Port module", "<0-0x1DFF>", "Vlan mapping on port", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_chip_set_datapath_cmd_help[] = { "chip module", "user configuration", "mode", "set", "48G+4TenG mode", "36G+6TenG mode", "chip", "<0~1>", ((void *)0) };

char *ctc_cli_l2_stp_set_state_cmd_help[] = { "Spanning Tree Protocol (STP)", "Configure stp state", "Global port", "<0-0x1DFF>", "MST instance", "MST instance ID <0-127>", "Stp statue", "Forwarding state", "Blocking state", "Learing state", ((void *)0) };

char *ctc_cli_l3if_remove_vmac_low_8bits_cmd_help[] = { "L3 interface module", "VRRP mac", "L3if ID", "<0-1022>", "Remove low 8bits of router mac", "The index of vmac entry", "Index range <0-3>", ((void *)0) };

char *ctc_cli_gb_nh_set_ecmp_mode_cmd_help[] = { "Nexthop module", "Ecmp mode", "Enable means use exact member", "Disable means use max member count", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_IpeFib_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show IpeFib statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_exec_cfg_cmd_help[] = { "execute cfg cmd", "chip", "chip id value", "cfg File name", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_GlobalStats_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show GlobalStats statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_enter_debug_mode_cmd_help[] = { "Enter", "Ctc Debug mode", "Mode", ((void *)0) };

char *ctc_cli_qos_set_port_queue_stats_en_cmd_help[] = { "QoS module", "QoS statstics", "Service id", "Value <1-0xFFFF", "Global port", "<0-0x1DFF>", "Queue id", "<0-7>", "Cpu reason", "Cpu reason id", "Enable", "Disable", ((void *)0) };

char *ctc_cli_acl_show_group_info_cmd_help[] = { "Show", "ACL module", "Group info", "ACL group id", "<0xFFFF0000>", ((void *)0) };

char *ctc_cli_dma_thread_cmd_help[] = { "dma", "thread", "monitor", "start", ((void *)0) };

char *ctc_cli_nh_update_mpls_nexthop_to_unrov_cmd_help[] = { "Nexthop module", "Update nexthop", "MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Forward nexthop to unresolved nexthop", ((void *)0) };

char *ctc_cli_pdu_l3_classify_l3pdu_by_l4port_cmd_help[] = { "Pdu modue", "Layer3 PDU", "Classify pdu based on layer3 protocol", "Layer4 port", "Entry index", "<0-15>", "Udp packet", "<0-0xFF>,when great than 1,as 1 to process", "Tcp packet", "<0-0xFF>,when great than 1,as 1 to process", "Layer4 dest port", "<0-0xFFFF>", ((void *)0) };

char *ctc_cli_qos_clear_policer_stats_cmd_help[] = { "Clear functions", "QoS module", "QoS policer", "Statistics", "Global port", "<0-0x1DFF>", "Ingress direction", "Egress direction", "QoS flow policer", "<1-0xFFFF>", "Service policer", "Service id <1-0xFFFF>", "Hbwp policer", "Specify the cos index", "Value <0-3>", ((void *)0) };

char *ctc_cli_dbg_tool_scan_sfp_para_cmd_help[] = { "Scan cmd", "Set scan I2C master parameter cmd", "Select which slaves to access", "<0-0xFFFFFFFF>", "Address", "Address value", "Slave internal memory offset", "<0-0xFFFF>", "Data len", "<0-0xFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_bfd_add_lmep_cmd_help[] = { "Local MEP", "BFD MEP", "Add local BFD MEP", "My Discriminator", "My Discriminator value, <0~4294967295>", "MEP is active or not", "MEP on cpu", "BFD desired min tx interval", "<1-1023> ms", "BFD local state", "Admin down :0 ; Down: 1; Init : 2; Up :3", "BFD local diag", "<0~31>", "BFD local detect mult", "<1-15>", "Enable APS message to CPU", "IP BFD OAM option", "Single Hop BFD", "UDP source port", "<49152-65535>", "NextHop ID for MEP out", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "MPLS BFD OAM option", "MPLS PW VCCV BFD", "MPLS PW VCCV BFD with IPv4, GAL 0x21", "MEP in mpls label", "Label, <0-1048575>", "APS PROTECTION PATH", "NextHop ID for MEP out", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "UDP source port", "<49152-65535>", "IPv4 sa", "IPv4 address in A.B.C.D format", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "MEP in mpls label", "Label, <0-1048575>", "APS PROTECTION PATH", "NextHop ID for MEP out", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "MPLS TTL", "TTL, <0-255>", "MPLS TP BFD OAM without GAL", "Source MEP ID TLV", "Source MEP ID TLV value", "MEP enable LM", "LM Cos type: 0 All cos data counter together; 1 Only count specified cos data; 2 Per cos date to count", "LM Cos <0-7>", "MEP enable CSF", "CSF type: 0 LOS; 1 FDI; 2 RDI; 3 DCI", "MEP index allocated by system", "2~max MEP index", "Master chip id for link agg", "<0-29>", ((void *)0) };

char *ctc_cli_ptp_show_global_property_cmd_help[] = { "Show running system information", "PTP module", "Global property", "Show all property", "PTP ucast will be processed", "TC will copy sync and follow_up message to CPU", "TC will copy signaling message to CPU", "TC will copy management message to CPU", "P2P transparent clock will process Delay_Request message", "PTP will be enabled by port or vlan, default by port", "Aging time for timestamp", "Select Sync Interface or ToD Interface to synchronize from master clock", ((void *)0) };

char *ctc_cli_scl_add_port_ipv4_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port ipv4 hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "IPv4 source address", "IPv4 address in A.B.C.D format", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_qos_set_igs_prec_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "IP precedence", "<0-0x7>", "To", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", ((void *)0) };

char *ctc_cli_gb_acl_show_entry_info_cmd_help[] = { "Show running system information", "ACL module", "Entry info", "All entries", "All entrise divide by group", "By entry", "<0~0xFFFFFFFF>", "By group", "<0xFFFF0002>", "By group priority", "<0~3> :0 is highest", "By type", "ACL Mac-entry", "ACL Ipv4-entry", "ACL Ipv6-entry", "ACL Mpls-entry", "Detail", ((void *)0) };

char *ctc_cli_mirror_debug_on_cmd_help[] = { "Debugging functions", "Mirror module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_nh_update_ecmp_cmd_help[] = { "Nexthop module", "Update nexthop", "ECMP group", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Member nexthop ID", "Add member", "Remove member", ((void *)0) };

char *ctc_cli_parser_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Parser module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_nh_show_nexthop_by_nhid_type_cmd_help[] = { "Show running system information", "Nexthop module", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Display detail information", ((void *)0) };

char *ctc_cli_gb_chip_agent_debug_sim_chip_model_on_cmd_help[] = { "Debugging functions", "Chip Agent module", "Simulator", "Chip model", "All debug", "Forward path", "Parser", "Process Key", "Lookup Key", "Table pointer", "Discard reason", "Queue manager message", "Queue manager packet header", "Queue manager link aggregation", "Aging Timer", ((void *)0) };

char *ctc_cli_acl_add_hash_ipv4_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", " Hash ipv4 entry ", "Global port", "PhyPort or logic-port, decided by is-logic-port. PhyPort including linkagg", "Is logic port", "<0-1>", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "Layer4 source port", "<0-0xFFFF>", "Layer4 destination port", "<0-0xFFFF>", "Dscp", "<0-0x3F>", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "Arp packet", "<0-1>", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *ctc_cli_chip_set_mdio_clock_frequency_cmd_help[] = { "chip module", "set", "mdio interface", "gephy mdio bus", "xgphy mdio bus", "mdio clock frequency", "frequency value(K)", ((void *)0) };

char *ctc_cli_dbg_tool_show_mac_Sgmac_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "mac status", "show Sgmac statistic", "Index", "<0-11>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_check_inter_laken_prbs_cmd_help[] = { "Check cmd", "Inter-laken prbs cmd", "Lane id", "<0-8>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_scl_uninstall_group_cmd_help[] = { "SCL module", "Uninstall from hardware", "SCL group id", "<0xFFFF000C>", ((void *)0) };

char *ctc_cli_stats_set_dot1q_subtract_bytes_cmd_help[] = { "Statistics", "Byte to subtract for Dot1Q packet", "Global port", "<0-0x1DFF>", "Packet Bytes", "<0,4,8,12 (default:4)>", ((void *)0) };

char *ctc_cli_ipmc_rpf_debug_on_cmd_help[] = { "Debugging functions", "RPF", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_ipmc_remove_ipv6_member_cmd_help[] = { "IPMC module", "IPv6", "Remove", "Member of the ipmc group", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route <0-8191>", "VRF id of route", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN interface", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan port", "Physical interface/remote chip id", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Remove a member port for a IP based l2 mcast", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Indicate the member is remote chip entry", "Add member by nexthop, can be used in ipmc and ip based l2mc", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_chip_set_hss12g_enable_cmd_help[] = { "chip module", "set operate", "hss12g macro", "hss id", "xfi mode", "sgmii mode", "qsgmii mode", "disable hss12g", "local chip", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_tx_cos_cmd_help[] = { "Local MEP", "COS for tx OAM PDU", "Set tx cos", "<0-7>", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_write_hss6g_cmd_help[] = { "Write cmd", "HSS6G cmd", "Address", "Address value", "Set value", "<0-0xFFFFFFFF>", "Mask", "<0-0xFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_protocol_vlan_cmd_help[] = { "Port module", "<0-0x1DFF>", "Protocol vlan on port", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_pdu_show_all_cmd_help[] = { "Show running system information", "Pdu modue", "All", ((void *)0) };

char *ctc_cli_mpls_add_vpws_ilm_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Incoming label mapping", "Add ilm", "Vpws vc label", "Label space id, <0-255>, space 0 is platform space", "Label, <0-1048575>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Control word used", "OAM Enable", "Don't check inner packet for vpws", "Service id", "Statistics", "0~0xFFFFFFFF", ((void *)0) };

char *ctc_cli_port_set_random_log_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port random log", "Flow direction", "Ingress port log", "Egress port log", "Both direction", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_packet_debug_on_cmd_help[] = { "Debugging functions", "Packet module", "Ctc layer", "System layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_scl_install_group_cmd_help[] = { "SCL module", "Install to hardware", "SCL group id", "<0xFFFF000C>", "Group type", "Port scl", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Global", "Port class", "<0~63>", "Lchip", "<0-1>", "Logic port", "Logic port value", ((void *)0) };

char *ctc_cli_pdu_l3_set_l3if_action_cmd_help[] = { "Pdu modue", "Layer3 PDU", "Per-l3if l3pdu action ", "Layer3 interface", "<0-1022>", "Per-l3if l3pdu action index", "<0-14>", "Action type", "Forward", "Copy to cpu", ((void *)0) };

char *ctc_cli_gb_chip_agent_dump_drvio_cmd_help[] = { "Show", "Chip Agent module", "Cache", "Driver IO", "Both direction", "TX direction", "RX direction", "Last count", "Detail information", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_TcamDsMem_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show TcamDsMem statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_chip_set_i2c_scan_en_cmd_help[] = { "chip module", "set operate", "i2c scan", "scan enable", "scan disable", ((void *)0) };

char *ctc_cli_nh_add_misc_l2hdr_cmd_help[] = { "Nexthop module", "Add nexthop", "Misc nexthop", "Replace L2 Header", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "If set,packet will be reflective to ingress port", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Swap macda and macsa", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_gb_ftm_debug_on_cmd_help[] = { "Debugging functions", "Memory Allocation module", "CTC layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_scl_add_mac_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Mac hash entry", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Mac is destination address or not ", "0-1", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_ptp_show_tod_intf_rx_code_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Tod interface input time code", ((void *)0) };

char *ctc_cli_port_mapping_show_mapping_mode_cmd_help[] = { "Show running system information", "Port module", "Port mapping", ((void *)0) };

char *ctc_cli_qos_set_egs_dscp_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", "To", "DSCP", "<0-0x3F>", ((void *)0) };

char *ctc_cli_port_set_isolation_id_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port isolation", "Flow direction ", "Ingress", "Egress", "Both direction", "Unknown ucast packet will be isolated", "Unknown mcast packet will be isolated", "Known mcast packet will be isolated", "Broadcast packet will be isolated", "Isolation ID value", "VALUE", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_lm_type_cmd_help[] = { "Local MEP", "Loss measurement type", "Loss measurement dual", "Loss measurement single", "Loss measurement none", ((void *)0) };

char *ctc_cli_qos_set_igs_dscp_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "DSCP", "<0-0x3F>", "To", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", ((void *)0) };

char *ctc_cli_nh_add_remove_remote_member_cmd_help[] = { "Nexthop module", "Mcast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Add operation", "Remove operation", "The member of multicast nexthop", "Global port/Remote chip id", "remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)", ((void *)0) };

char *ctc_cli_oam_cfm_update_port_status_cmd_help[] = { "OAM module", "CFM", "Update local MEP port status", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "Port status", "<1-2>", ((void *)0) };

char *ctc_cli_l2_show_stp_state_by_port_cmd_help[] = { "Show running system information", "Spanning Tree Protocol (STP)", "Stp state", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_dbg_tool_debug_on_cmd_help[] = { "diag tool", "debug information", "debug is on", "debug is off", ((void *)0) };

char *ctc_cli_ipuc_remove_ipv4_cmd_help[] = { "IPUC module", "Remove route", "VRF id of route <0-8191>", "IPv4 address in A.B.C.D format", "The length of Ipv4 mask <0~32>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "ECMP group", "Layer4 destination port for NAPT", "<0-0xFFFF>", "If set, indicate it is a tcp port, or is a udp port", ((void *)0) };

char *ctc_cli_mpls_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Multi-Protocol Label Switching (MPLS)", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_dbg_tool_read_voltage_cmd_help[] = { "Read cmd", "Read voltage cmd", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_ipmc_set_ipv6_force_route_cmd_help[] = { "IPMC module", "Ipv6", "Force route", "IPv6 address0", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "IPv6 address1", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Force bridge", "Force unicast", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_IpeForward_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show IpeForward statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_ipmc_add_ipv4_group_cmd_help[] = { "IPMC module", "Add", "IP multicast group", "Mcast Group ID,it is allocated in platform adaption Layer", "Group address class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "TTL check enable", "RPF check enable", "If RPF check fail, sent to cpu", "Statistic supported", "0~0xFFFFFFFF", "Do IP based L2 mcast", "Ptp-entry", "Redirect to cpu", ((void *)0) };

char *ctc_cli_oam_cfm_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "OAM module", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_acl_update_pbr_action_cmd_help[] = { "ACL module", "Update pbr action", "ACL entry id", "<0~0xFFFFFFFF>", "Permit", "Deny", "Forward to nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Ecmp nexthop", "TTL check", "ICMP redirect check", "Copy to cpu", ((void *)0) };

char *ctc_cli_common_fast_enter_debug_mode_cmd_help[] = { "Enter Debug mode", ((void *)0) };

char *ctc_cli_dbg_tool_integrity_golden_cmd_help[] = { "integrity-compare generate cmd", "template file name", "golden file name", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_interrupt_get_en_cmd_help[] = { "Show running system information", "Interrupt module", "<0-1>", "Interrupt sup-level type", "<0-0xFFFFFFFF>", "Interrupt sub-level type", "<0-0xFFFFFFFF>", "Enable functions", ((void *)0) };

char *ctc_cli_scl_add_port_cvlan_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port cvlan hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "Direction", "Ingress direction", "Egress direction", "Customer VLAN", "<1-4095>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_parser_l3_unmapping_l4type_cmd_help[] = { "Parser module", "Layer 3 header of packet", "Mapping layer4 type", "Unmapping l4 type entry", "Entry index", "Index value <0,1,2,3,6>", ((void *)0) };

char *ctc_cli_vlan_set_dir_property_cmd_help[] = { "VLAN module", "<1-4095>", "Vlan property with direction", "Acl enable (bitmap 0~3)", "Vlan acl vlan classid", "Vlan acl against routed-packet only", "Flow direction", "Ingress", "Egress", "Both direction", "Property value", "Value", ((void *)0) };

char *ctc_cli_ptp_show_clock_time_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Local clock time", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_GreatbeltSup_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show GreatbeltSup statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_mirror_show_mirror_info_cmd_help[] = { "Show running system information", "Mirror module", "Information", "Global port", "<0-0x1DFF>", "VLAN module", "<1-4095>", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipmc_status_cmd_help[] = { "Show running system information", "IPMC module", "Ipmc and L2mc based ip Status", ((void *)0) };

char *ctc_cli_acl_add_hash_mac_entry_cmd_help[] = { "ACL module", "Add one entry to software table", "ACL group id", "<0xFFFF0002>", "ACL entry id", "<0~0xFFFFFFFF>", " Hash mac entry ", "Global port", "PhyPort or logic-port, decided by is-logic-port. PhyPort including linkagg", "Is logic port", "<0-1>", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "Ether type", "<0-0xFFFF>", "Vlan id", "<1-4095>", "Cos", "<0-7>", "Permit", "Deny", "Statistics", "0-0xFFFFFFFF", "Deny bridge", "Deny learning", "Deny route", "Valid priority and color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Valid trust", "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence", "Valid micro flow policer", "<0-0xFFFF>", "Valid macro flow policer", "<0-0xFFFF>", "Log to", "<0-3>", "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% ", "Copy to cpu", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Valid dscp", "<0-0x3F>", "Copy to cpu with timestamp", "Valid qos domain", "<1-7> 0 is invalid", "Valid aging", "Valid vlan edit ", "Stag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Svid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag vlan id", "<1-4095>", "Scos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cos", "<0-7>", "Scfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)", "Cvid select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cos", "<0-7>", "Ccfi select", "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)", "New ctag cfi", "0-1", ((void *)0) };

char *ctc_cli_qos_set_shape_ipg_en_cmd_help[] = { "QoS module", "QoS shaping", "Ipg", "Enable", "Disable", ((void *)0) };

char *ctc_cli_mirror_add_rspan_session_cmd_help[] = { "Mirror module", "Remote mirror destination", "Add destination for mirror session", "Configure a mirror session", "Session_id range <0-3>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", "Mirror received and transmitted traffic", "Port session", "Vlan session", "Acl session", "Acl priority id", "GB range <0-3>, HB range <0-1>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Add vlan tagged for remote mirror", "<1-4095>", "Nexthop for remote mirror", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_dbg_tool_read_xgphy_cmd_help[] = { "Read cmd", "XGPHY cmd", "Mdio bus", "<0-1>", "Phy address", "<0-0x1F>", "Dev type", "<0-0x1F>", "XGPHY reg address", "<0-0xFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_vlan_remove_vlan_class_default_entry_cmd_help[] = { "VLAN module", "Vlan classification", "Remove vlan classify default entry", "Mac based vlan", "IPv4 based vlan", "IPv6 based vlan", ((void *)0) };

char *ctc_cli_stats_show_port_cfg_cmd_help[] = { "Show running system information", "Statistics", "Ingress port statistics option type", "Egress port statistics option type", ((void *)0) };

char *ctc_cli_mpls_remove_stats_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Remove action", "Statistics", "Label space id, <0-255>, space 0 is platform space", "Label, <0-1048575>", ((void *)0) };

char *ctc_cli_chip_show_datapath_infor_cmd_help[] = { "Chip module", "CTC_CLI_SHOW_STR", "Datapath", "Information", "Pll", "Hss", "SerDes", "Mac", "Synce", "Calender", "Misc", "Channel", "Internal port", ((void *)0) };

char *ctc_cli_dbg_tool_scan_gephy_para_cmd_help[] = { "Scan cmd", "Set gephy scan parameter cmd", "Bitmap0", "bitmap0", "Bitmap1", "bitmap1", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_ipuc_reinit_cmd_help[] = { "IPUC module", "Reinit ipv4 module with pipline mode (test usage only)", "Pipline3 enable means do lpm with hash8 and 3 level pipline else means do lpm with hash16 with 2 level pipline", "Enable : do hash8 and 3 level pipline lookup", "Disable : do hash16 and 2 level pipline lookup", ((void *)0) };

char *ctc_cli_dbg_tool_read_pcie_io_cmd_help[] = { "Read cmd", "PCI io cmd", "Address", "Address value", "<0-32767>", ((void *)0) };

char *ctc_cli_sync_ether_debug_on_cmd_help[] = { "Debugging functions", "SyncE module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_scan_gephy_sp_para_cmd_help[] = { "Scan cmd", "Set gephy special scan parameter cmd", "GEPHY special register index", "<0-1>", "Reg address", "<0-0x1F>", "Bit ctrl", "<0-0xF>", "Interrupt enable", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_dot1q_type_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port's type descript in 802.1q", "Packet transmit with untag", "Packet transmit with ctag", "Packet transmit with stag", "Packet transmit with both stag and ctag", ((void *)0) };

char *ctc_cli_gb_vlan_create_vlan_vrange_cmd_help[] = { "VLAN module", "Vlan range", "Create", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, "Direction", "Ingress", "Egress", "Is svlan range", "Is cvlan range", ((void *)0) };

char *ctc_cli_learning_aging_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Learning and Aging module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_pdu_l3_classify_l3pdu_by_l3proto_cmd_help[] = { "Pdu modue", "Layer3 PDU", "Classify pdu base on layer3 protocol", "Base on layer3 protocol", "Entry index", "<0-15>", "Layer3 header protocol", "<0-255>", ((void *)0) };

char *ctc_cli_port_set_untag_dft_vid_disable_cmd_help[] = { "Port module", "<0-0x1DFF>", "Untag vlan id when transmit packet vid equal to PVID", "Disable functions", ((void *)0) };

char *ctc_cli_oam_cfm_set_port_enable_port_tunnel_cmd_help[] = { "OAM module", "CFM", "Global port", "<0-0x1DFF>", "OAM tunnel", "Enable OAM tunnel on this port", "Disable OAM tunnel on this port", ((void *)0) };

char *ctc_cli_gb_oam_update_update_timer_enable_cmd_help[] = { "OAM module", "CFM", "OAM update timer", "Enable update timer", "Disable update timer", ((void *)0) };

char *ctc_cli_dbg_tool_write_chip_cmd_help[] = { "Write cmd", "Address", "Address value", "Set value", "<0-0xFFFFFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_clear_stats_cmd_help[] = { "Clear functions", "Statistics", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_ipmc_add_ipv6_member_cmd_help[] = { "IPMC module", "IPv6", "Add", "Member of the ipmc group", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route <0-8191>", "VRF id of route", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN interface", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan port", "Physical interface/remote chip id", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Add a member port for a IP based l2 mcast", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Indicate the member is remote chip entry", "Add member by nexthop, can be used in ipmc and ip based l2mc", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_mirror_rspan_escape_en_cmd_help[] = { "Mirror module", "Remote mirror", "Discard some special mirrored packet", "Enable", "Disable", ((void *)0) };

char *ctc_cli_ipuc_show_ipv4_info_cmd_help[] = { "IPUC module", "Show ipuc ipv4 route", "VRF id of route <0-8191>", "IPv4 address in A.B.C.D format", "The length of Ipv4 mask <0~32>", "show the detail of this entry", ((void *)0) };

char *ctc_cli_ipmc_add_ipv6_default_entry_cmd_help[] = { "IPMC module", "IPV6", "Add", "Default action", "Send the packet that hit the default entry to cpu", "Drop the packet that hit the default entry", ((void *)0) };

char *ctc_cli_port_set_property_cmd_help[] = { "Port module", "<0-0x1DFF>", "Property", "Route enable", "Packet tag take precedence over all", "Hareware learning", "Scl ipv6 lookup disable", "Scl ipv4 lookup disable", "Scl default lookup", "Scl MAC key is used for lookup instead of ipv6 key", "Scl MAC key is used for lookup instead of ipv4 key", "Set trill enable", "Set NON-TRILL packet discard", "Set TRILL packet discard", "Bridge to the same port enable", "FCoE enable", "FCoE RPF enable", "The STAG COS field is replaced by the classified COS result", "The CTAG COS field is replaced by the classified COS result", "The Dscp field is replace by qos mapping", "ARP packet processing type", "DHCP packet processing type", "RPF check for outer tunnel IP header is enabled", "For untagged PTP packet and routedPort", "Strict or loose", "This port connect with a leaf node", "Priority tagged packet will be sent out", "Default cos of vlan tag", "Default dei of vlan tag", "Port support linkscan function", "Aps failover enable", "Linkagg failover enable", "Timestamp", "Property value", "Value", "Packet type", "Raw packet type disable", "Port parser ethernet raw packet", "Port only parser ipv4 raw packet", "Port only parser ipv6 raw packet", "Pvlan port type", "Port is none port", "Port is promiscuous port", "Port is isolated port", "Port is community port", "Community port isolated id", "0~15", "Qos trust policy", "Classifies ingress packets with the port default CoS value", "Classifies ingress packets with the outer priority value", "Classifies ingress packets with the packet CoS values", "Classifies ingress packets with the packet DSCP values", "Classifies ingress packets with the packet IP-Precedence values", "Classifies ingress packets with the packet Stag CoS values", "Classifies ingress packets with the packet Ctag CoS values", ((void *)0) };

char *ctc_cli_oam_cfm_add_rmep_cmd_help[] = { "Remote MEP", "Add remote MEP on oam channel", "MEP id", "<1-8191>", "MEP is active or not", "Enable CSF", "Ethernet OAM option", "MEP level", "<0-7>", "Remote MEP MAC SA", "MAC address in HHHH.HHHH.HHHH format", "Enable sequence number update", "Enable MAC update", "MPLS TP Y.1731 OAM option", "MEP index allocated by system", "2~max MEP index", ((void *)0) };

char *ctc_cli_oam_cfm_set_vlan_max_md_level_cmd_help[] = { "OAM module", "CFM", "VLAN", "<1-4095>", "Ingress direction", "Egress direction", "Both direction", "Set max MD level on this vlan", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_check_net_tx_packet_capture_cmd_help[] = { "Check cmd", "Net-tx packet buffer capture cmd", "Port id", "<0-59>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_ipmc_rpf_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "RPF", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_oam_update_error_cache_timer_enable_cmd_help[] = { "OAM module", "CFM", "OAM error cache timer", "Enable timer", "Disable timer", ((void *)0) };

char *ctc_cli_ptp_show_sync_intf_toggle_time_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Sync interface toggle time", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_MetFifo_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show MetFifo statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_chip_read_gephy_interface_cmd_help[] = { "chip module", "read operate", "ge phy", "index of port", "<0-0x1DFF>", "phy register", "<0~31>", ((void *)0) };

char *ctc_cli_port_set_flow_ctl_cmd_help[] = { "Port module", "<0-0x1DFF>", "Flow control", "Cos used for PFC", "<0-7>", "Receive pause frame", "Transmit pause frame", "Both receive and transmit", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_oam_show_bfd_up_time_cmd_help[] = { "Show running system information" "BFD", "UP state", "Time", ((void *)0) };

char *ctc_cli_qos_set_cpu_reason_dest_cmd_help[] = { "QoS module", "Cpu Resaon Id", "<0-MAX> refer to cli [show qos cpu-reason-id]", "Destination", "Local cpu", "Remote cpu port", "<0-0x1DFF>", "Local port", "<0-0x1DFF>", "Drop", ((void *)0) };

char *ctc_cli_qos_set_queue_wdrr_weight_mtu_cmd_help[] = { "QoS module", "QoS schedule", "Set queue wdrr weight mtu", "<1-0x3FFFFFF>", ((void *)0) };

char *ctc_cli_gb_chip_agent_serverip_cmd_help[] = { "Chip Agent module", "Configured server IP address", "Server IP address", "Configured TCP port", "TCP port number", ((void *)0) };

char *ctc_cli_dbg_tool_run_bist_cmd_help[] = { "Debug cmd", "Run bist", "Run lpmtcam bist", "Run inttcam bist", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_nh_update_mcast_service_queue_cmd_help[] = { "Nexthop module", "Multicast Group", "Mcast Group ID,it is allocated in platform adaption Layer", "Service queue", "Update service queue", "Member nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_vlan_remove_all_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Flush all vlan mapping entries by port", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_stats_get_threshold_cfg_cmd_help[] = { "Show running system information", "Statistics", "deal with general options pkt cnt threshold", "deal with general options byte cnt threshold", "deal with general options fifo depth threshold", ((void *)0) };

char *ctc_cli_chip_read_i2c_buffer_cmd_help[] = { "chip module", "read operate", "i2c device buffer", "local chip id", "chip id value", "buffer length", "length value", ((void *)0) };

char *ctc_cli_parser_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Parser module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_interrupt_get_status_cmd_help[] = { "Show running system information", "Interrupt module", "<0-1>", "Interrupt sup-level type", "<0-0xFFFFFFFF>", "Interrupt sub-level type", "<0-0xFFFFFFFF>", "Status", ((void *)0) };

char *ctc_cli_mpls_update_normal_ilm_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Incoming label mapping", "Update ilm", "Mpls label space", "Label space id, <0-255>, space 0 is platform space", "Mpls label", "Label, <0-1048575>", "Uniform model", "Short pipe model", "Pipe model", "OAM Enable", ((void *)0) };

char *ctc_cli_nh_remove_stats_cmd_help[] = { "Nexthop module", "Delete action", "Statistics", "Nexthop id", ((void *)0) };

char *ctc_cli_vlan_remove_ipv6_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Add one vlan classification entry", "IPv6 based vlan", "IPv6 source address", "IPv6 address in X:X::X:X format", "IPv6 source mask", "IPv6 mask in X:X::X:X format", "IPv6 destination address", "IPv6 address in X:X::X:X format", "IPv6 destination mask", "IPv6 mask in X:X::X:X format", "MAC SA address", "MAC address in HHHH.HHHH.HHHH format", "MAC DA address", "MAC address in HHHH.HHHH.HHHH format", "Layer4 source port", "<0-65535>", "Layer4 destination port", "<0-65535>", "Layer4 type", "Layer4 type UDP", "Layer4 type TCP", "Layer4 type id: 0~15", ((void *)0) };

char *ctc_cli_gb_qos_show_qos_domain_cmd_help[] = { "Show", "QoS module", "QoS domain", "domain <0-7>", "Maping type", "Cos map to priority color", "Ip-prec map to priority color", "Dscp map to priority color", "Exp map to priority color", "Priority color map to cos", "Priority color map to dscp", "Priority color map to exp", ((void *)0) };

char *ctc_cli_oam_cfm_remove_chan_cmd_help[] = { "OAM module", "CFM", "MEP lookup channel", "Remove MEP lookup channel", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "MEP direction", "Up MEP", "Down MEP", "Is link level oam", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_port_set_max_frame_size_cmd_help[] = { "Port module", "<0-0x1DFF>", "Max frame size per port", "0:size0, 1:size1. (default:0)", ((void *)0) };

char *ctc_cli_stats_reset_mac_cmd_help[] = { "Clear functions", "Statistics", "Mac reception statistics", "Mac transmission statistics", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_chip_set_phy_scan_en_cmd_help[] = { "chip module", "set operate", "phy scan", "scan enable", "scan disable", ((void *)0) };

char *ctc_cli_gb_oam_tp_section_oam_cmd_help[] = { "OAM module", "MPLS TP", "Section OAM", "Section OAM use port", "Section OAM use interface", ((void *)0) };

char *ctc_cli_gb_mpls_show_space_cmd_help[] = { "Show running system information", "Multi-Protocol Label Switching (MPLS)", "Label space", ((void *)0) };

char *ctc_cli_oam_cfm_debug_on_cmd_help[] = { "Debugging functions", "OAM module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_DynamicDsMem_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show DynamicDsMem statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_set_all_pdu_to_cpu_cmd_help[] = { "OAM module", "Enable or disable all OAM pdu relay to cpu", "Enable all pkt to cpu", "Disable all pkt to cpu", ((void *)0) };

char *ctc_cli_oam_set_lbr_use_share_mac_cmd_help[] = { "OAM module", "CFM", "Set reply LBR MAC SA mode", "Enable LBR MAC SA use share Mac", "Disable LBR MAC SA use share Mac", ((void *)0) };

char *ctc_cli_oam_cfm_update_rmep_seq_num_en_cmd_help[] = { "Remote MEP", "Sequence number", "Enable Sequence number", "Disable Sequence number", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_pdu_l2_classify_l2pdu_by_macda_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Classify pdu based on type", "Base on 48bit macda", "Entry index", "<0-3>", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Macda mask", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_port_set_min_frame_size_cmd_help[] = { "Port module", "<0-0x1DFF>", "Min frame size", "<33-64>", ((void *)0) };

char *ctc_cli_dma_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "DMA Module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_nh_add_remove_l2mc_member_cmd_help[] = { "Nexthop module", "Mcast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Add operation", "Remove operation", "The bridge member of multicast nexthop", "Global port/Remote chip id", "Local member  gport:gchip(8bit) +local phy port(8bit)", ((void *)0) };

char *ctc_cli_parser_show_l4_app_data_cmd_help[] = { "Show running system information", "Parser module", "Layer 4 header of packet", "Application entry", "Entry index", "Index Value <0 - 3>", ((void *)0) };

char *ctc_cli_l3if_show_property_cmd_help[] = { "Show running system information", "L3 interface module", "L3if ID", "<0-1022>", "IP property", "Transmission enable, 1:enable, 0:disable (only support in HUMBER)", "Reception enable, 1:enable, 0:disable (only support in HUMBER)", "Routing enable, 1:enable, 0:disable", "PBR enable, 1:enable,0:disable", "The type of nat interface,0- external,1- internal", "All packets are routed, 1:enable, 0:disable", "IPv4 unicast enable, 1:enable, 0:disable", "IPv4 multicast enable, 1:enable,0:disable", "IPv4 multicast RPF check is enabled, 1:enable, 0:disable (only support in HUMBER)", "IPv4 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR", "IPv6 unicast enable, 1:enable, 0:disable", "IPv6 multicast enable, 1:enable, 0:disable", "IPv6 multicast RPF check enable, 1:enable, 0:disable (only support in HUMBER)", "IPv6 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR", "VRF id of route <0-8191>", "Enable route lookup with vrfid", "Mtu check enable, 1:enable, 0:disable", "Mtu exception to cpu enable, 1:enable, 0:disable", "Mtu size of the interface, <0-16383>", "Route mac prefix type, range<0-2> (HUMBER not support)", "Route mac low 8bits, in 0xHH format (HUMBER not support)", "Egress macsa prefix type, range<0-2>", "Egress macsa low 8bits, in 0xHH format", "Multicast TTL threshold, range<0-255>", "ARP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard (only support in HUMBER)", "DHCP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard (only support in HUMBER)", "RPF check type, 0:strick mode, 1:loose mode", "RPF permit default, 0:disable, 1:enable", "Ingress vlan acl enable, bitmap<0-3>", "Ingress acl0 vlan classid, range<0-127>", "Ingress acl1 vlan classid, range<0-127>", "Engress vlan acl enable, bitmap<0-3>", "Engress acl0 vlan classid, range<0-127>", "Engress acl1 vlan classid, range<0-127>", ((void *)0) };

char *ctc_cli_nh_add_xlate_cmd_help[] = { "Nexthop module", "Add nexthop", "Egress Vlan Editing nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "Service queue", "Enable adjust length for shaping", "Enable horizon split in VPLS networks", "Loopback nexthop id", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Statistics", "0~0xFFFFFFFF", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", ((void *)0) };

char *ctc_cli_parser_show_l2_vlan_pas_num_cmd_help[] = { "Show running system information", "Parser module", "Layer 2 header of packet", "Ethernet l2", "Vlan parser num", ((void *)0) };

char *ctc_cli_dbg_tool_check_reg_cmd_help[] = { "check address value cmd", "address 0xXXXXXXXX", "value to expect", "mask", "rand", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipv4_group_info_cmd_help[] = { "Show running system information", "IPMC module", "IP multicast group", "Information, including source and group IP adress, etc.", ((void *)0) };

char *ctc_cli_security_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Security module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_ipuc_add_ipv6_tunnel_cmd_help[] = { "IPUC module", "Add ipv6 route", "This route is ip tunnel", "Add ip tunnel route", "IPv6 address in X:X::X:X format", "VRF id of route <0-8191>" "(Only HUMBER support) used to do route lookup, Inner route will lookup by this VRFID (HUMBER not support)", "VRF id of route <0-8191>" "(Only HUMBER support) used to do route lookup, Inner route will lookup by this VRFID (HUMBER not support)", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Tunnel payload layer 4 type (default is IPv4)", "Tunnel payload layer 4 type: \r\n0 for GRE \r\n1 for IPv4 \r\n2 for IPv6", "Set Gre key value", "Gre key value <1-4294967295>", "Set IP Sa", "IPv6 address in X:X::X:X format", "ECMP group (Only HUMBER support)", "Do ttl check for tunnel outer header (HUMBER not support)", "Use outer ttl for later process , or use inner ttl (HUMBER not support)", "Packet will do router by tunnel outer header , or use payload header (HUMBER not support)", "Copy to CPU (Only HUMBER support)", "Use outer header info do acl lookup, default use the inner", "Service Id", "<1-0xFFFF>", "Logic port", "<0-0x1DFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Enable stats", "Stats ID", "Discard", ((void *)0) };

char *ctc_cli_learning_aging_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Learning and Aging module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_dbg_tool_driver_init_cmd_help[] = { "Driver cmd", "Initialize driver information cmd", "Initialize chip num <1-2>", "Chip ID start base", ((void *)0) };

char *ctc_cli_gb_oam_clear_error_cache_cmd_help[] = { "OAM module", "OAM event", "Clear OAM event", ((void *)0) };

char *ctc_cli_gb_chip_agent_send_pkt_cmd_help[] = { "Chip Agent module", "Packet", "Configure port", "Port number", "Configure packet length", "Packet length", "Header of packet with HEX format", "Configure payload mode", "Fixed mode", "Fixed value", "Increment mode", "Increment base", "Increment repeat", "Random mode", ((void *)0) };

char *ctc_cli_dbg_tool_mdioxg_clock_cmd_help[] = { "Mdioxg cmd", "Core clk", "Core Clock value (MHZ)", "Mdio clock", "Mdioxg Clock value (MHZ)", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_l2_show_mcast_cmd_help[] = { "Show running system information", "Layer 2 module", "Mcast entry", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "VLAN", "<0-16383>", ((void *)0) };

char *ctc_cli_parser_debug_on_cmd_help[] = { "Debugging functions", "Parser module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_stacking_set_keeplive_cmd_help[] = { "Stacking", "KeepLive group for cpu-to-cpu", "Add group member", "Remove group member", "Trunk id", "Trunk id <1-63>", "Member port(Cpu port)", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_qos_remove_policer_cmd_help[] = { "QoS module", "QoS policer", "Detach policer", "Global port", "<0-0x1DFF>", "Ingress direction", "Egress direction", "QoS flow policer", "<1-0xFFFF>", "Service policer", "Service id <1-0xFFFF>", "Hbwp policer", "Specify the cos index", "Value <0-3>", ((void *)0) };

char *ctc_cli_l2_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Layer 2 module", "FDB table", "Spanning Tree Protocol (STP)", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_stacking_add_remove_trunk_port_cmd_help[] = { "Stacking", "Trunk", "Trunk id <1-63>", "Add", "Remove", "Trunk port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_l2_add_or_remove_port_to_default_entry_cmd_help[] = { "Layer 2 module", "FDB table", "Vlan default entry", "Forwarding ID", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Add operation", "Remove operation", "Global port /Remote chip id", "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)", "Nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Indicate the member is remote chip", "Enable transport by service queue", ((void *)0) };

char *ctc_cli_security_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Security module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_security_storm_ctl_show_global_cfg_cmd_help[] = { "Show running system information", "Security module", "Storm control", "Global configure", ((void *)0) };

char *ctc_cli_stats_show_cpu_mac_stats_cmd_help[] = { "Show running system information", "Statistics", "CPU Statistics", "Global port", "<0-0x1DXX>, low 8bit value is ignored", ((void *)0) };

char *ctc_cli_l3if_create_or_destroy_l3_phy_if_cmd_help[] = { "L3 interface module", "Create l3 interface", "Destroy l3 interface", "L3if ID", "<0-1022>", "L3 interface type", "Physical interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_vlan_remove_vlan_cmd_help[] = { "VLAN module", "Remove one vlan", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_gb_qos_use_default_domain_cmd_help[] = { "QoS module", "QoS domain", "Default value", ((void *)0) };

char *ctc_cli_scl_add_port_cvlan_cos_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port cvlan cos hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "Direction", "Ingress direction", "Egress direction", "Customer VLAN", "<1-4095>", "Customer VLAN CoS", "<0-7>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_dm_en_cmd_help[] = { "Local MEP", "Delay measurment", "Enable delay measurment", "Disable delay measurment", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_nh_remove_mcast_cmd_help[] = { "Nexthop module", "Delete nexthop", "Mcast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_gb_ipmc_show_offset_cmd_help[] = { "Show running system information", "IPMC module", "IPMC offset", "IPV6", "Block index", ((void *)0) };

char *ctc_cli_l2_add_ucast_nexthop_cmd_help[] = { "Layer 2 module", "FDB table", "Add operation", "Ucast bridge nexthop entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Type of nexthop (if undefine any subtype, all will be add)", "Basic brgu nexthop", "Untagged brgu nexthop", "Bypass brgu nexthop", "Raw_pkt brgu nexthop", "Service queue brgu nexthop", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_Policing_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show Policing statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_mac_cmd_help[] = { "Mac ctl cmd", "Mac id <0-59>", "Enable functions", "Disable functions", "Net tx mode", "0: EPE chan mode; 1: netTx chan mode", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_chip_debug_on_cmd_help[] = { "Debugging functions", "chip module", "CTC layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_l3if_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "L3 interface module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_nh_update_ipuc_cmd_help[] = { "Nexthop module", "Update nexthop", "IP unicast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Forward attribute(include port, vid and mac)", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "Forward nexthop to unresolved nexthop", "Unresolved nexthop to forward nexthop", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", ((void *)0) };

char *ctc_cli_oam_cfm_remove_rmep_cmd_help[] = { "Remote MEP", "Remove remote MEP on oam channel", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_port_set_phy_if_cmd_help[] = { "Port module", "<0-0x1DFF>", "Physical interface", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_ftm_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Memory Allocation module", "CTC layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_ftm_opf_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Offset pool freelist", "Fast priority arrangement", ((void *)0) };

char *ctc_cli_security_mac_security_show_mac_limit_cmd_help[] = { "Show running system information", "Security module", "Mac limit", "Port based mac limit", "Vlan based mac limit", "The value of gport Id or vlan Id", ((void *)0) };

char *ctc_cli_aps_create_aps_bridge_group_cmd_help[] = { "Aps Module", "Create", "Aps bridge", "Aps bridge group", "<0-2047>", "Aps type", "Used for normal l2 aps protection", "Used for mpls aps protection", "Working path gport", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Working path L3 interface id", "INTERFACE ID", "Next working aps group id", "0-1023", "Protection path gport", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Protection path L3 interface id", "INTERFACE ID", "Next protection aps group id", "0-1023", "Port aps use physical isolated path", "Raps enable", "Raps groupid", "Raps Group ID,it is allocated in platform adaption Layer", "Hw-based-aps-en,hardware based aps for link down event", ((void *)0) };

char *ctc_cli_pdu_show_l3_global_action_cmd_help[] = { "Show running system information", "Pdu modue", "Layer3 PDU", "Global action", "OSPF, not need entry index", "PIM,  not need entry index", "VRRP, not need entry index", "RSVP, not need entry index", "RIP,  not need entry index", "BGP,  not need entry index", "MSDP, not need entry index", "LDP,  not need entry index", "Base on layer3 protocol", "Base on layer4 port", "Base on layer4 type", "Entry index", "<0-15>", ((void *)0) };

char *ctc_cli_qos_debug_on_cmd_help[] = { "Debugging functions", "QoS module", "Ctc layer", "Sys layer", "QoS policer", "QoS classification", "QoS queue", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_acl_create_group_cmd_help[] = { "ACL module", "Create ACL group", "ACL group id", "<0xFFFF0000>", "ACL group priority", "<0~3> :0 is highest", "Direction", "Ingress", "Egress", "Both direction", "Lchip", "<0-1>", "Port bitmap (52 ports supported)", "<0-0xFFFFF> each bit stands a port valid. 20 high ports: port32~ port51", "<0-0xFFFFFFFF>, 32 low ports: port0~ port31", "Global", "Vlan class", "<1-1023> 0 is reserved", "Port class", "<1-1023> 0 is reserved", "Service ACL", "<1-0xFFFF>", "Pbr class", "<0-63>", ((void *)0) };

char *ctc_cli_qos_add_del_service_cmd_help[] = { "QoS module", "Service", "Binding queue", "Unbinding queue", "Serive id", "Value <0-0xFFFF>", "dest port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_qos_set_egs_exp_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", "To", "MPLS EXP", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_scan_sfp_cmd_help[] = { "Scan cmd", "I2C master cmd", "Enable functions", "Disable functions", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_ptp_show_device_type_cmd_help[] = { "Show running system information", "PTP module", "PTP device type", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_lm_cos_cmd_help[] = { "Local MEP", "Loss measurement cos", "<0-7>", ((void *)0) };

char *ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Global action", "Base on layer2 header protocol", "Entry index", "<0-15>", "Entry valid", "<0-0xFF>,when great than 1,as 1 to process", "By pass all, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "Per-port action index", "<0-14>", ((void *)0) };

char *ctc_cli_mirror_show_mirror_discard_cmd_help[] = { "Show running system information", "Mirror module", "Packets dropped", "Port session", "Vlan session", "Acl session", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", ((void *)0) };

char *ctc_cli_port_show_monitor_rlt_cmd_help[] = { "Show running system information", "Chip module", "Device", "Information", "Local chip Id", "<0~1>", ((void *)0) };

char *ctc_cli_ipuc_write_lpm_by_dma_cmd_help[] = { "IPUC module", "Use", "DMA", ((void *)0) };

char *ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Classify pdu based on type", "Base on layer2 header protocol", "Entry index", "<0-15>", "Layer2 header protocol", "<0x0 - 0xFFFF>", ((void *)0) };

char *ctc_cli_port_set_system_max_frame_size_cmd_help[] = { "System attribution", "Max frame size to apply port select", "Max frame size0", "Max Frame size1", "Max size value", ((void *)0) };

char *ctc_cli_vlan_add_ipv6_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Add one vlan classification entry", "IPv6 based vlan", "IPv6 source address", "IPv6 address in X:X::X:X format", "IPv6 source mask", "IPv6 mask in X:X::X:X format", "IPv6 destination address", "IPv6 address in X:X::X:X format", "IPv6 destination mask", "IPv6 mask in X:X::X:X format", "MAC SA address", "MAC address in HHHH.HHHH.HHHH format", "MAC DA address", "MAC address in HHHH.HHHH.HHHH format", "Layer4 source port", "<0-65535>", "Layer4 destination port", "<0-65535>", "Layer4 type", "Layer4 type UDP", "Layer4 type TCP", "Layer4 type id: 0~15", "Vlan", "<1-4095>", "Cos", "<0-7>", ((void *)0) };

char *ctc_cli_parser_l2_set_max_len_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Ethernet l2", "Max length", "Max length value <0x0 - 0xFFFF>", ((void *)0) };

char *ctc_cli_vlan_add_mac_vlan_class_cmd_help[] = { "VLAN module", "Vlan classification", "Add one entry", "Mac based vlan", "MAC SA address", "MAC address in HHHH.HHHH.HHHH format", "MAC DA address", "MAC address in HHHH.HHHH.HHHH format", "Vlan", "<1-4095>", "Cos", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_prbs_cmd_help[] = { "Hss12g cmd", "Prbs cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "HSS12G link num", "<0-8>, 8 means broadcast", "Internal loopback enable", "Test pattern", "0:PRBS7+ 1:PRBS7- 2:PRBS15+ 3:PRBS15- 4:PRBS23+ 5.PRBS23- 6.PRBS31+ 7.PRBS31-", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_chip_agent_pkt_rx_print_off_cmd_help[] = { "Negate a command or set its defaults", "Chip Agent module", "Packet", "Print CPU RX packet", ((void *)0) };

char *ctc_cli_ipuc_remove_ipv4_nat_sa_cmd_help[] = { "IPUC module", "Nat sa", "Add", "VRF id of route <0-8191>", "IPv4 address in A.B.C.D format", "Layer4 source port for NAPT", "<0-0xFFFF>", "If set, indicate it is a tcp port, or is a udp port", ((void *)0) };

char *ctc_cli_common_fast_enter_sdk_mode_cmd_help[] = { "Enter SDK Mode", ((void *)0) };

char *ctc_cli_stats_set_mtu2_packet_length_cmd_help[] = { "Statistics", "MTU2 packet length,packet length larger than this consider as jumbo packet.Default is 1536B", "Global port", "<0-0x1DFF>", "Packet length", "<1024 - 8191> (default:1536)", ((void *)0) };

char *ctc_cli_qos_set_port_queue_drop_cmd_help[] = { "QoS module", "Config queue drop", "Global port", "<0-0x1DFF>", "Queue id", "<0-7>", "Weighted tail drop", "Drop threshold", "Drop threshold for red-colored packets, ""<0-0xFFFF>", "Drop threshold for yellow-colored packets, ""<0-0xFFFF>", "Drop threshold for green-colored packets, ""<0-0xFFFF>", "Weighted random early detection", "Minimum threshold", "Minimum threshold for red-colored packets, ""<0-0xFFFF>", "Minimum threshold for yellow-colored packets, ""<0-0xFFFF>", "Minimum threshold for green-colored packets, ""<0-0xFFFF>", "Maximum threshold", "Maximum threshold for red-colored packets, ""<0-0xFFFF>", "Maximum threshold for yellow-colored packets, ""<0-0xFFFF>", "Maximum threshold for green-colored packets, ""<0-0xFFFF>", "Drop probability", "Drop probability for red-colored packets, ""<0-15>", "Drop probability for yellow-colored packets, ""<0-15>", "Drop probability for green-colored packets, ""<0-15>", "Weight [GB not support]", "WRED weight for computing average queue size, <0-15>", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_lm_en_cmd_help[] = { "Local MEP", "Loss measurement", "Enable LM", "Disable LM", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_nh_add_mcast_cmd_help[] = { "Nexthop module", "Add nexthop", "Mcast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Multicast Group", "Mcast Group ID,it is allocated in platform adaption Layer", "If set,the mcast group will be applied to mirror", "Statistics", "0~0xFFFFFFFF", ((void *)0) };

char *ctc_cli_dbg_tool_epe_flow_chart_cmd_help[] = { "show bus", "debug tool", "epe", "flow chart", ((void *)0) };

char *ctc_cli_nh_remove_mpls_nexthop_cmd_help[] = { "Nexthop module", "Delete nexthop", "Mpls or aps-mpls nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_IpePktProc_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show IpePktProc statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l2_debug_on_cmd_help[] = { "Debugging functions", "Layer 2 module", "FDB table", "Spanning Tree Protocol (STP)", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_l3if_set_mpls_property_cmd_help[] = { "L3 interface module", "L3if ID", "<0-1022>", "MPLS property", "MPLS is enabled", "Value", "1: enalbe, 0: disable, other value process as 1", "MPLS label space", "Value", "<0-255>, space 0 is platform space", ((void *)0) };

char *ctc_cli_vlan_set_igmp_snooping_cmd_help[] = { "VLAN module", "<1-4095>", "IGMP snooping on vlan", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_l3if_set_router_mac_cmd_help[] = { "L3 interface module", "Router mac", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_mirror_set_vlan_enable_cmd_help[] = { "Mirror module", "Mirror source, port or vlan", "VLAN module", "<1-4095>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", "Mirror received and transmitted traffic", "Mirror enable", "Configure a mirror session", "Session_id range <0-3>", "Mirror disable", ((void *)0) };

char *ctc_cli_nh_update_mpls_push_nexthop_cmd_help[] = { "Nexthop module", "Update nexthop", "MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Mpls push nexthop", "Update forward nexthop", "Unresolved nexthop to forward nexthop", "Enable H-VPLS in VPLS network", "Destination logic port in VPLS network", "MPLS vpls port value <0-8191>", "Mpls push nexthop payload operation type", "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)", "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "First label is martini control word", "Martini sequence type is none", "Martini sequence type is per-pw", "Martini per-pw sequence index", "Sequence index value", "Martini sequence type is global sequence type0", "Martini sequence type is global sequence type1", "Control word", "Control word value", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "Statistics", "Stats id", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_Mdio_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show Mdio statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_set_sender_id_tlv_cmd_help[] = { "OAM module", "CFM", "Set sender id", "Sender id", "Sender id string", ((void *)0) };

char *ctc_cli_gb_packet_show_packet_info_cmd_help[] = { "Show running system information", "Packet module", ((void *)0) };

char *ctc_cli_dbg_tool_write_pcie_cfg_cmd_help[] = { "Write cmd", "PCI cfg cmd", "Address", "Address value", "Set value", "<0-0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_common_show_memory_summary_total_cmd_help[] = { "Show running system information", "Memory information", "Summary of memory statistics", "Total", ((void *)0) };

char *ctc_cli_nh_update_aps_mpls_push_nexthop_cmd_help[] = { "Nexthop module", "Update nexthop", "APS MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Aps bridge id", "<0-1023>", "Mpls push nexthop", "Update forward nexthop", "Unresolved nexthop to forward nexthop", "Enable H-VPLS in VPLS network", "Destination logic port in VPLS network", "MPLS vpls port value <0-8191>", "APS working path parameter", "Mpls push nexthop payload operation type", "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)", "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "First label is martini control word", "Martini sequence type is none", "Martini sequence type is per-pw", "Martini per-pw sequence index", "Sequence index value", "Martini sequence type is global sequence type0", "Martini sequence type is global sequence type1", "Control word", "Control word value", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "Statistics", "Stats id", "APS protection path parameter", "Mpls push nexthop payload operation type", "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)", "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "First label is martini control word", "Martini sequence type is none", "Martini sequence type is per-pw", "Martini per-pw sequence index", "Sequence index value", "Martini sequence type is global sequence type0", "Martini sequence type is global sequence type1", "Control word", "Control word value", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "Statistics", "Stats id", ((void *)0) };

char *ctc_cli_gb_drvio_show_cmd_help[] = { "Show running system information", "Driver IO", "Cache", "Lastest count number in cache", ((void *)0) };

char *ctc_cli_gb_vlan_flush_all_egress_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Egress", "Flush all vlan mapping entries by port", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_common_hostname_cmd_help[] = { "Set system's network name", "System's network name", ((void *)0) };

char *ctc_cli_vlan_set_stpid_cmd_help[] = { "VLAN module", "<1-4095>", "Stp instance id", "<0-127>", ((void *)0) };

char *ctc_cli_l2_remove_addr_cmd_help[] = { "Layer 2 module", "FDB table", "Remove operation", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Forwarding ID", "<0-16383>", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_BufRetrv_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show BufRetrv statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_write_xgphy_cmd_help[] = { "Write cmd", "XGPHY cmd", "Mdio bus", "<0-1>", "Phy address", "<0-0x1F>", "Dev type", "<0-0x1F>", "XGPHY reg address", "<0-0xFFFF>", "Set value", "Value", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_chip_agent_dump_cmd_help[] = { "Show", "Chip Agent module", ((void *)0) };

char *ctc_cli_stats_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Statistics", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_gb_scl_show_status_cmd_help[] = { "Show", "SCL module", "Status", ((void *)0) };

char *ctc_cli_mirror_show_vlan_mirror_state_cmd_help[] = { "Show running system information", "Mirror module", "VLAN module", "<1-4095>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", ((void *)0) };

char *ctc_cli_vlan_set_ipv6_l2mc_cmd_help[] = { "VLAN module", "<1-4095>", "Ipv6 based l2mc", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_read_chip_cmd_help[] = { "Read cmd", "Address", "Address value", "Data len", "<0-32767>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_oam_trpt_clear_stats_cmd_help[] = { "clear", "stats", "OAM module", "ThroughPut", "Throughput sessopn", "Session id: <0~3>", "Global chip id", "<0-29>", ((void *)0) };

char *ctc_cli_chip_show_mdio_clock_frequency_cmd_help[] = { "show", "chip module", "mdio interface", "gephy mdio bus", "xgphy mdio bus", "mdio clock frequency", "frequency value(K)", ((void *)0) };

char *ctc_cli_nh_remove_iloop_cmd_help[] = { "Nexthop module", "Delete nexthop", "IPE Loopback nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_pdu_show_l3_classify_l3pdu_cmd_help[] = { "Show running system information", "Pdu modue", "Layer3 PDU", "Classify pdu based on layer3 protocol", "Layer3 protocol", "Layer4 port", "Entry index", "<0-15>", ((void *)0) };

char *ctc_cli_l3if_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "L3 interface module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_oam_cfm_remove_edge_port_cmd_help[] = { "OAM module", "CFM", "OAM edge port", "Remove edge port", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_stats_reset_global_fwd_cmd_help[] = { "Clear functions", "Statistics", "Ingress global forwarding statistics", "Egress global forwarding statistics", ((void *)0) };

char *ctc_cli_oam_cfm_set_rx_vlan_tpid_cmd_help[] = { "OAM module", "Vlan tpid for OAM pdu rx", "Set tpid", "Tpid index", "<0-1>", "Tpid", "0xXXXX", ((void *)0) };

char *ctc_cli_stacking_set_end_rchip_bitmap_cmd_help[] = { "Stacking", "Add", "Remove", "End the remote chip id", "ChipId Value<0-29>", "Break point for ring", "Trunk id <1-63>", ((void *)0) };

char *ctc_cli_port_debug_on_cmd_help[] = { "Debugging functions", "Port module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_MacLedDriver_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show MacLedDriver statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_ipmc_mcast_debug_on_cmd_help[] = { "Debugging functions", "MCAST module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_scl_set_entry_priority_cmd_help[] = { "SCL module", "SCL entry id", "<0~0x7FFFFFFF>", "SCL entry priority", "<0~0xFFFFFFFF> :0 is lowest", ((void *)0) };

char *ctc_cli_port_release_internal_port_cmd_help[] = { "Port module", "Release", "Internal port", "64-255", "Global chip id", "<0-29>", "Type", "Iloop", "Eloop", "Discard", "Fwd", "Port module", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_gb_chip_agent_debug_sim_chip_model_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Chip Agent module", "Simulator", "Chip model", "All debug", "Forward path", "Parser", "Process Key", "Lookup Key", "Table pointer", "Discard reason", "Queue manager message", "Queue manager packet header", "Queue manager link aggregation", "Aging Timer", ((void *)0) };

char *ctc_cli_security_mac_security_set_port_learn_limit_cmd_help[] = { "Security module", "Mac limit", "Port based mac limit", "<0-0x1DFF>", "The action of mac limit", "Do nothing", "Forwarding packet", "Discard packet", "Redirect to cpu", ((void *)0) };

char *ctc_cli_port_show_port_mac_link_cmd_help[] = { "Show running system information", "Port module", "Port mac link status", ((void *)0) };

char *ctc_cli_dbg_tool_drain_on_off_cmd_help[] = { "Drain on off ctl cmd", "Module name", "1:ipe 2:bsr 3:epe 4: oam 5: all", "Block name", "0:IpeHdrAdj 1:IpeIntfMap 2:IpeLkupMgr 3:IpePktProc 4:IpeForward 5:BufStore 6:MetFifo 7:QMgrEnq 8:QMgrDeq 9:BufRetrv 10:NetRx 11:Tcam 12:Eloop 13:EpeHdrAdj 14:EpeNextHop 15 : EpeHdrProc 16:EpeAclQos 17 :EpeCla 18: EpeOam 19:EpeHdrEdit 20:OamHdrAdj 21: OamHdrEdit", "Repeat time", "Repeat time value", "Drain on off mode", "0: parallel 1:serial", "Off time ", "Off time vlaue ms", "On time ", "On time vlaue ms", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_qos_show_qos_metfifo_excp_cmd_help[] = { "Show", "QoS module", "Metfifo exception info", "Destmap and nexthop info", ((void *)0) };

char *ctc_cli_vlan_set_ptp_cmd_help[] = { "VLAN module", "<1-4095>", "Ptp", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_common_interrupt_show_debug_cmd_help[] = { "Show running system information", "Debugging functions", "Interrupt module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_chip_set_gpio_output_cmd_help[] = { "chip module", "set operate", "gpio", "gpio id", "<0~15>", "output value", "0-1", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_QMgrDeq_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show QMgrDeq statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_bfd_remove_rmep_cmd_help[] = { "Remote MEP", "BFD MEP", "Remove remote BFD MEP", "My Discriminator", "My Discriminator value, <0~4294967295>", "IP BFD OAM option", "MPLS BFD OAM option", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", ((void *)0) };

char *ctc_cli_gb_oam_cfm_show_port_oam_info_cmd_help[] = { "Show OAM Information", "OAM Information", "OAM information in Port", "Global port", "<0-0x1DFF>", ((void *)0) };

char *ctc_cli_ipuc_debug_show_cmd_help[] = { "Show running system information", "Debugging functions", "IPUC module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_stats_show_global_fwd_cmd_help[] = { "Show running system information", "Statistics", "Ingress global forwarding statistics", "Egress global forwarding statistics", ((void *)0) };

char *ctc_cli_acl_remove_entry_cmd_help[] = { "ACL module", "Remove one entry from software table", "ACL entry id", "<0~0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_port_set_vlan_ctl_cmd_help[] = { "Port module", "<0-0x1DFF>", "Vlan control", "Allow all packet", "Drop all untagged packet", "Drop all tagged packet", "Drop all packet", "Drop packet without 2 tag", "Drop packet with 2 tag", "Drop stagged packet", "Drop packet without stag", "Drop packet only stagged", "Permit packet only with stag", "Drop all packet with ctag", "Drop packet without ctag", "Drop packet only ctagged", "Permit packet only with ctag", ((void *)0) };

char *ctc_cli_qos_set_igs_cos_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "Class of Service", "<0-7>", "To", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", ((void *)0) };

char *ctc_cli_gb_nh_show_nexthop_all_by_type_cmd_help[] = { "Show running system information", "Nexthop module", "All nexthop", "Multicast nexthop", "Bridge unicast nexthop", "Ipuc nexthop", "ECMP nexthop", "MPLS nexthop", "ILoop nexthop", "RSPAN nexthop", "Ip tunnel nexthop", "Misc nexthop", "Display detail information", ((void *)0) };

char *ctc_cli_gb_chip_agent_dump_error_cmd_help[] = { "Show", "Chip Agent module", "Error information", ((void *)0) };

char *ctc_cli_bpe_interlaken_en_cmd_help[] = { "bpe module", "interlaken interface", "enable", "disable", ((void *)0) };

char *ctc_cli_dbg_tool_write_hss12g_cmd_help[] = { "Write cmd", "Hss12g cmd", "HSS12G Macro Cores", "HSS12G Macro ID <0-3>", "Transmitter section", "Receiver section", "HS PLL section", "HSS12G link num", "<0-8>, 8 means broadcast", "HSS12G addr offset", "<0-0x3F>", "Set value", "<0-0xFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_interrupt_debug_on_cmd_help[] = { "Debugging functions", "Interrupt module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_oam_OamProc_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "oam status", "show OamProc statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_linkagg_show_detail_member_port_cmd_help[] = { "Show running system information", "Linkagg module", "<0-127>", "Member ports of linkagg group", "all member include pad member", ((void *)0) };

char *ctc_cli_chip_set_mac_led_en_cmd_help[] = { "chip module", "set operate", "mac led", "enable", "disable", ((void *)0) };

char *ctc_cli_nh_remove_ecmp_cmd_help[] = { "Nexthop module", "Delete nexthop", "ECMP group", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_dbg_tool_read_pcie_mem_cmd_help[] = { "Read cmd", "PCI mem cmd", "Address", "Address value", "<0-32767>", ((void *)0) };

char *ctc_cli_port_set_learning_cmd_help[] = { "Port module", "<0-0x1DFF>", "Mac learning", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_trpt_en_cmd_help[] = { "Local MEP", "ThroughPut", "TroughPut session", "Session id<0~3>", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_aps_map_mepindex_cmd_help[] = { "APS Module", "Map aps group id to mep index", "APS bridge group", "Aps bridge group id", "Mep index", "Index", ((void *)0) };

char *ctc_cli_vlan_update_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Add vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Stag vlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Ctag vlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Vlan range group id", "<0~63>", "Stag priority or cos", "<0-7>", "Ctag priority or cos", "<0-7>", "Mapping to", "Disable vsi learning", "Enable vsi mac security", "Enable aging", ((void *)0) };

char *ctc_cli_scl_add_ipv6_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Ipv6 hash entry", "IPv6 source address", "IPv6 address in X:X::X:X format", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_gb_oam_cfm_dump_lmep_cmd_help[] = { "OAM module", "CFM", "Dump registers or tables", "Local MEP table", "Chip id on linecard", "<0-1>", "Local MEP table index", "<0-2047>", ((void *)0) };

char *ctc_cli_pdu_l2_set_global_action_by_bpdu_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Global action", "Bpdu", "Slow protocol", "Eapol", "Lldp", "Entry valid", "<0-0xFF>,when great than 1,as 1 to process", "Copy to cpu, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "By pass all, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "Per-port action index", "<0-14>", ((void *)0) };

char *ctc_cli_dbg_tool_run_net_tx_packet_capture_cmd_help[] = { "Run cmd", "Net-tx packet buffer capture cmd", "Port id", "<0-59>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_tp_add_chan_cmd_help[] = { "OAM module", "MPLS TP Y.1731", "MEP lookup channel", "Add MEP lookup channel", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", "Master chip id for OAM", "<0-29>", ((void *)0) };

char *ctc_cli_gb_vlan_remove_vlan_vrange_cmd_help[] = { "VLAN module", "Vlan range", "Remove", "<1-4095>", "To", "<1-4095>", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, "Direction", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_gb_interrupt_show_interrupt_chip_cmd_help[] = { "Show running system information", "Interrupt module", "<0-1>", "Interrupt register", "Group/PIN level", "Chip fatal", "Chip normal", "Function", "DMA function", "DMA normal", "All", ((void *)0) };

char *ctc_cli_vlan_set_fid_cmd_help[] = { "VLAN module", "<1-4095>", "Fid", "<0-16383>", ((void *)0) };

char *ctc_cli_oam_cfm_enter_chan_cmd_help[] = { "OAM module", "CFM", "Enter OAM channel mode", "MEP lookup channel", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "MEP direction", "Up MEP", "Down MEP", "Is link level oam", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_sync_ether_set_cfg_cmd_help[] = { "SyncE module", "<0-1>", "SyncE clock", "SyncE clock ID", "The port used for recover clock", "Local port ID", "Clock divider", "<0-63>", "Clock output enable or disable", "Clock output enable", "Clock output disable", "Link status detect enable or disable", "Link status detect enable", "Link status detect disable", ((void *)0) };

char *ctc_cli_port_set_system_ipg_size_cmd_help[] = { "System attribution", "Set", "Ipg index", "Value <0-3>", "Size", "Value <0-255>", ((void *)0) };

char *ctc_cli_gb_drvio_action_cmd_help[] = { "Driver IO", "Action", "Print", "Cache", "Print and Cache", "Disable", ((void *)0) };

char *ctc_cli_gb_port_set_vlan_range_cmd_help[] = { "Port module", "<0-0x1DFF>", "Vlan range on port", "Direction", "Ingress", "Egress", "Vlan range group id", "<0~63>", "Enable vlan range", "Disable vlan range", ((void *)0) };

char *ctc_cli_oam_cfm_add_remove_mip_cmd_help[] = { "MIP", "Add MIP", "Remove MIP", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "MEP level", "<0-7>", "MPLS label", "Label, <0-1048575>", "Master chip id", "<0-29>", ((void *)0) };

char *ctc_cli_l2mcast_remove_member_cmd_help[] = { "Layer 2 module", "L2 multicast", "Remove operation", "The member of multicast group", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Forwarding ID", "<0-16383>", "Global port /Remote chip id", "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)", "Next hop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Indicate the member is remote chip entry", ((void *)0) };

char *show_cli_tbl_reg_cmd_help[] = { "Show running system information", "Table or register id", "Table id value <0-MaxTblId_t>", ((void *)0) };

char *ctc_cli_gb_security_ip_src_guard_remove_entry_cmd_help[] = { "Security module", "Ip source guard", "Remove ip source guard entry", "IPv6 source address", "IPv6 address in X:X::X:X format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", ((void *)0) };

char *ctc_cli_chip_get_access_mode_cmd_help[] = { "chip module", "get operate", "access", "mode", ((void *)0) };

char *ctc_cli_parser_l3_set_ip_frag_cmd_help[] = { "Parser module", "Layer 3 header of packet", "Layer3 ip", "Small fragment offset", "Offset value<0-3>", ((void *)0) };

char *ctc_cli_update_ip_tunnel_to_unrov_cmd_help[] = { "Nexthop module", "Update nexthop", "IP Tunnel nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Forward nexthop to unresolved nexthop", ((void *)0) };

char *ctc_cli_gb_oam_trpt_set_session_en_cmd_help[] = { "OAM module", "ThroughPut", "Set", "Throughput sessopn", "Session id: <0~3>", "Global chip id", "<0-29>", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_parser_l4_set_app_cmd_help[] = { "Parser module", "Layer 4 header of packet", "Udp application operation", "Layer4 ptp enable", "Enable functions", "Disable functions", "Layer4 ntp enable", "Enable functions", "Disable functions", "Layer4 bfd enable", "Enable functions", "Disable functions", "Layer4 capwap enable", "Enable functions", "Disable functions", "Capwap roaming state bits", "State bits value<0-3>", ((void *)0) };

char *ctc_cli_gb_acl_set_global_cfg_cmd_help[] = { "ACL module", "Global configuration", "Merge mac to ipv4 key", "Ingress use mapped vlan instead of packet vlan", "Trill packet use ipv6 key", "Arp packet use ipv6 key", "Hash acl enable", "Non ipv4 mpls packet use ipv6 key", "Ingress service acl enable on port (bitmap 0~15)", "Ingress service acl enable on vlan (bitmap 0~15)", "Egress service acl enable on port (bitmap 0~15)", "Egress service acl enable on vlan (bitmap 0~15)", "Hash mac key flag", "Hash ipv4 key flag", "Priority bitmap of stats", "Property value", "Value", ((void *)0) };

char *ctc_cli_pdu_set_default_global_action_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Layer3 PDU", "set default global action", ((void *)0) };

char *ctc_cli_mpls_debug_on_cmd_help[] = { "Debugging functions", "Multi-Protocol Label Switching (MPLS)", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_show_packet_flow_diag_rst_cmd_help[] = { "show", "packet", "flow", "diagnose", "function start", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_emulation_load_config_cmd_help[] = { "debug cmd", "load", "cfg", "asic master config file name", ((void *)0) };

char *ctc_cli_security_ip_src_guard_remove_entry_cmd_help[] = { "Security module", "Ip source guard", "Remove ip source guard entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "IPv4 source address", "IPv4 address in A.B.C.D format", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Vlan", "<1-4095>", "Vlan id is svlan id", ((void *)0) };

char *ctc_cli_ptp_show_tod_intf_cmd_help[] = { "Show running system information", "PTP module", "<0-1>", "Tod interface config", ((void *)0) };

char *ctc_cli_l2_add_addr_cmd_help[] = { "Layer 2 module", "FDB table", "Add operation", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Forwarding ID", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Src port after vpls loopback", "Global port", "Pbb", "Virtual port, use for vpls/pbb/qinq service queue", "Virtual port value, range 0-0x1FFF, and 0x1FFF is invalid", "Static fdb table", "Remote chip learing dynamic fdb", "Destination discard fdb table", "Source discard fdb table", "Source discard and send to cpu fdb table", "Forward and copy to cpu fdb table", "Port bind fdb table", "Send raw packet to cpu,using for terminating protocol pakcet(LBM/DMM/LMM) to cpu", "Packet will enqueue by service id", "Output packet will has no vlan tag", "Protocol entry enable exception flag", "Indicate the entry is system mac, it can't be deleted by flush api, using for MAC DA", "PTP mac address for ptp process", ((void *)0) };

char *ctc_cli_nh_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Nexthop module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_l3if_set_vmac_prefix_cmd_help[] = { "L3 interface module", "VRRP mac", "Vmac prefix type", "Prefix type, 0 or 1", "Vmac prefix value", "MAC address in HHHH.HHHH.HHHH format, the high 40 bits is used to prefix", ((void *)0) };

char *ctc_cli_l2_add_default_entry_cmd_help[] = { "Layer 2 module", "FDB table", "Add operation", "Vlan default entry", "Forwarding ID", "<0-16383>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Multicast Group", "Mcast Group ID,it is allocated in platform adaption Layer", "Use logic port check", "Unknown ucast mac to drop", "Unknown mcast mac to drop", "Protocol exception to cpu", ((void *)0) };

char *ctc_cli_dbg_tool_check_sgmac_prbs_xfipcs_cmd_help[] = { "Check cmd", "Sgmac-prbs cmd", "Pcsmode xfipcs", "Port id", "<0-12>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_auto_neg_cmd_help[] = { "Port module", "<0-0x1DFF>", "Auto-neg", "Enable", "Disable", ((void *)0) };

char *ctc_cli_qos_set_egs_cos_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", "To", "Class of Service", "<0-7>", ((void *)0) };

char *ctc_cli_vlan_add_vlan_class_default_entry_cmd_help[] = { "VLAN module", "Vlan classification", "Add vlan classify default entry", "Mac based vlan", "Ipv4 based vlan", "Ipv6 based vlan", "Discard packet", "Normal forwarding", "Redirect to CPU", ((void *)0) };

char *ctc_cli_dbg_tool_cpu_access_dyn_ram_cmd_help[] = { "Dynamic ram access cmd", "Repeat time", "Repeat time value", "Access step ", "Access step len(per index)", "Access mode", "1:4w; 0: 8w", "Operation mode ", "0:none 1:RW 2:RO", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_stats_set_mtu1_packet_length_cmd_help[] = { "Statistics", "MTU1 packet length,packet length larger than this consider as oversized packet.Default is 1518B", "Global port", "<0-0x1DFF>", "Packet length", "<1024 - 16383> (default:1518)", ((void *)0) };

char *ctc_cli_aps_set_aps_select_cmd_help[] = { "Aps Module", "Setting", "Aps select", "Aps select group", "<0-2047>", "Aps select from protection path", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_PtpEngine_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show PtpEngine statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_hss12g_show_status_cmd_help[] = { "Hss12g cmd", "Status cmd", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_nh_cfg_global_param_cmd_help[] = { "Nexthop module", "IPMC-logic_replication", "Value", "The Value of paramer configuration", ((void *)0) };

char *ctc_cli_port_show_port_info_cmd_help[] = { "Show running system information", "Port module", "<0-0x1DFF>", "Show all property", "Port state", "Reception port state", "Transmission port state", "L2 bridge", "Default vlan id", "Port cross connect", "Mac learning enable", "Keep vlan tag", "Routed port l3 only", "Route enable on port", "Vlan tag control", "Ingress vlan filtering", "Egress vlan filtering", "Vlan domain when STPID is the same as CTPID", "Ingress stag tpid index", "Egress stag tpid index", "Untag-defalt-vlan", "Port's type descript in 802.1q", "Use outer ttl in case of tuunel", "Port based blocking", "Ucast flooding to this port", "Mcast flooding to this port", "Broadcast flooding to this port", "Known multicast flooding to this port", "Protocol vlan", "Vlan mapping", "Vlan classify", "Ip source guard", "Mac enable", "Speed mode", "Preamble value", "Min frame size", "Max frame size", "Stretch mode", "L2 pading", "Ingress random log", "Egress random log", "The percentage of received packet for Ingress random log", "The percentage of received packet for egress random log", "Src discard", "Enable to check src port match", "Mac link status", "Ipg index", "Ingress isolation ID", "Egress isolation ID", "Isolation Id", "Reflective bridge enable", "Mux", "Demux", ((void *)0) };

char *ctc_cli_scl_set_default_action_cmd_help[] = { "SCL module", "Set default entry", "Global port", "<0-0x1D3F>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_oam_cfm_enter_other_chan_cmd_help[] = { "Enter other OAM channel", "MEP lookup channel", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "MEP direction", "Up MEP", "Down MEP", "Is link level oam", "VLAN", "<1-4095>", ((void *)0) };

char *ctc_cli_oam_cfm_add_edge_port_cmd_help[] = { "OAM module", "CFM", "OAM edge port", "Configure edge port", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "MD level", "<0-7>", ((void *)0) };

char *ctc_cli_oam_bfd_update_lmep_cmd_help[] = { "Local MEP", "BFD", "Update Local BFD MEP", "IP BFD OAM option", "My Discriminator", "My Discriminator value, <0~4294967295>", "MPLS BFD OAM option", "My Discriminator", "My Discriminator value, <0~4294967295>", "MPLS TP BFD OAM option", "Is MPLS TP section oam", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "L3if ID", "<0-1022>", "Mpls label", "Label, <0-1048575>", "MEP is active or not", "BFD CC Enable", "BFD CV Enable(for TP BFD)", "Client signal fail(for TP BFD)", "CSF Tx Enable(for TP BFD)", "LM Enable(for TP BFD)", "Enable", "Disable", "Set tx cos", "<0-7>", "Set signal fail state", "<0-1>", "Client signal fail type(for TP BFD)", "LOS: 0, FDI: 1, RDI: 2, DCI: 3", "CSF defect(for TP BFD)", "Clear CSF defect", "Loss measurement cos type(for TP BFD)", "All cos : 0 ; Specified-cos : 1; Per-cos : 2", "Loss measurement cos(for TP BFD)", "<0-7>", "BFD local diag", "<0~31>", "BFD local state", "Admin down :0 ; Down: 1; Init : 2; Up :3", "My Discriminator", "My Discriminator value, <0~4294967295>", "NextHop ID for MEP out", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Tx timer", "Desired min tx interval", "Local detection interval multiplier", ((void *)0) };

char *ctc_cli_dbg_tool_read_tcam_cmd_help[] = { "Read cmd", "Read tcam", "Tcam index", "Tcam index value", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_oam_trpt_get_stats_cmd_help[] = { "Show running system information", "stats", "OAM module", "ThroughPut", "Throughput sessopn", "Session id: <0~3>", "Global chip id", "<0-29>", ((void *)0) };

char *ctc_cli_security_storm_ctl_set_config_cmd_help[] = { "Security module", "Storm control", "Port based Storm control", "<0-0x1DFF>", "Vlan based Storm control", "<1-4095>", "Enable functions", "Disable functions", "The mode of storm control", "Packet per second", "Bytes per second", "Threshold", "Bps <0-PortMaxSpeed/8>, must be in multiples of 1000, pps:(<0-PortMaxSpeed/(8*84)>) ", "The type of storm control", "Broadcast", "Known multicast", "Unknown multicast", "All multicast, known multicast + unknown multicast", "Known unicast", "Unknown unicast", "All unicast, known unicast + unknown unicast", "Discarded packet by storm control and redirect to cpu", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_csf_use_uc_da_cmd_help[] = { "Local MEP", "Client signal fail Mac Da", "Enable CSF Mac Da", "Disable CSF Mac Da", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_nh_add_ipuc_cmd_help[] = { "Nexthop module", "Add nexthop", "IP unicast nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Unresolved nexthop", "Forward Nexthop", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "Loopback nexthop id", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Protection path for aps", "Aps bridge id", "<0-1023>", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", ((void *)0) };

char *ctc_cli_stacking_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "stacking", "CTC Layer", "SYS Layer", ((void *)0) };

char *ctc_cli_oam_cfm_update_intf_status_cmd_help[] = { "OAM module", "CFM", "Update interface status", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Interface status", "<1-7>", ((void *)0) };

char *ctc_cli_parser_show_l2_pbb_header_cmd_help[] = { "Show running system information", "Parser module", "Layer 2 header of packet", "Provider backbone", "Header", ((void *)0) };

char *ctc_cli_chip_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Chip Module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_mirror_set_port_enable_cmd_help[] = { "Mirror module", "Mirror source, port or vlan", "Global port", "<0-0x1DFF>", "The direction of monitored traffic", "Mirror received traffic", "Mirror transmitted traffic", "Mirror received and transmitted traffic", "Mirror enable", "Configure a mirror session", "Session_id range <0-3>, cpu mirror don't care session id", "Mirror disable", ((void *)0) };

char *ctc_cli_parser_show_l2_max_len_cmd_help[] = { "Show running system information", "Parser module", "Layer 2 header of packet", "Ethernet l2", "Max length", ((void *)0) };

char *ctc_cli_scl_add_l2_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "L2 hash entry", " MAC source address", "MAC address in HHHH.HHHH.HHHH format", " MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Cos", "<0-7>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_port_set_sub_if_cmd_help[] = { "Port module", "<0-0x1DFF>", "Sub interface", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_l3if_show_vmac_prefix_cmd_help[] = { "Show running system information", "L3 interface module", "VRRP mac", "Vmac prefix type", "Prefix type, 0 or 1", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_IpeLkupMgr_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show IpeLkupMgr statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_write_tbl_reg_cmd_help[] = { "Write cmd", "Table or register", "Table/register id name", "Table/register entry index<0>", "Entry index num", "Table/register fileld id name", "Table/register field name(string)", "Table offset", "Table offset value", "Set value", "<0-0xFFFFFFFF>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_vlan_remove_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Remove vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "802.1ad svlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "802.1ad cvlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Vlan range group id", "<0~63>", "Stag priority or cos", "<0-7>", "Ctag priority or cos", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_list_tbl_reg_tips_cmd_help[] = { "List table or register info ", "Table or register", "Table/register include string NAME", ((void *)0) };

char *ctc_cli_scl_add_port_hash_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0xFFFF0000~0xFFFF000C>", "SCL entry id", "<0~0x7FFFFFFF>", "Port hash entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Class id", "<0~63>", "Direction", "Ingress direction", "Egress direction", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_common_interrupt_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Interrupt module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_parser_l2_unmapping_l3type_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Mapping Layer3 type for Layer3 parsing", "Unmapping l3 type entry", "Entry index", "Index Value <0-2>", ((void *)0) };

char *ctc_cli_nh_swap_mpls_tunnel_cmd_help[] = { "Nexthop module", "mpls tunnel", "swap mpls tunnel", "Source Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "Destination Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", ((void *)0) };

char *ctc_cli_mirror_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Mirror module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_dbg_tool_show_oam_OamParser_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "oam status", "show OamParser statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_common_enter_cmodel_mode_cmd_help[] = { "Enter", "Ctc Cmodel mode", "Mode", ((void *)0) };

char *ctc_cli_oam_cfm_update_lmep_ccm_enable_cmd_help[] = { "Local MEP", "Tx ccm configure", "Enable ccm for local MEP", "Disable ccm for local MEP", "MEP level", "<0-7>", ((void *)0) };

char *ctc_cli_port_set_port_en_cmd_help[] = { "Port module", "<0-0x1DFF>", "Set port state", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_vlan_get_vlan_vrange_cmd_help[] = { "Do show", "Vlan range", "Vlan range group id", CTC_CLI_VLAN_RANGE_GRP_ID_VALUE, "Direction", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_packet_tx_uc_cmd_help[] = { "Sample Packet module", "Send packet", "Unicast", "Destination gport", "<0-0x1DFF>", "Linkagg", "<0-127>", "Has hash", "Hash Value", "Bypass nexthop", "Bridge nexthop", "VLAN of packet", "VLAN ID", "Packet length", "length value", "DMA", ((void *)0) };

char *ctc_cli_scl_add_ipv4_entry_cmd_help[] = { "SCL module", "Add one entry to software table", "SCL group id", "<0~0xFFFF0000>", "SCL entry id", "<0~0x7FFFFFFF>", "SCL entry priority", "<0~0xFFFFFFFF> :0 is lowest", "Ipv4 tcam entry", "Customer VLAN", "<1-4095>", "<0-0xFFF>", "Customer VLAN CoS", "<0-7>", "<0-7>", "Customer VLAN CFI", "<0-1>", "Service VLAN", "<1-4095>", "<0-0xFFF>", "Service VLAN CoS", "<0-7>", "<0-7>", "Service VLAN CFI", "<0-1>", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "MAC destination address", "MAC address in HHHH.HHHH.HHHH format", "MAC address in HHHH.HHHH.HHHH format", "Layer 2 Type", "<0-15>", "<0-0xF>", "Layer 3 Type", "<0-15>", "<0-0xF>", "IPv4 source address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "IPv4 destination address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Sender Ip address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "Target Ip address", "IPv4 address in A.B.C.D format", "IPv4 mask in A.B.C.D format", "L4 protocol ", "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17", "<0-0xFF>", "Layer4 source port", "<0-0xFFFF>", "<0-0xFFFF>", "Layer4 destination port", "<0-0xFFFF>", "<0-0xFFFF>", "Icmp type", "<0-0xFF>", "<0-0xFF>", "Icmp code", "<0-0xFF>", "<0-0xFF>", "Igmp type", "<0-0xFF>", "<0-0xFF>", "Dscp", "<0-0x3F>", "<0-0x3F>", "Ether type", "<0-0xFFFF>", "<0-0xFFFF>", "Layer 4 Type", "<0-15>", "<0-0xF>", "Ip option", "<0-1>", "Ip fragment information", "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment", "Ip precedence", "<0-0x7>", "<0-0x7>", "Ingress service queue", "Priority color", "<0-0x3F>", "<1-3> 0 is invalid, 1:red 2:yellow 3:green", "Service Id", "<1-0xFFFF>", "Enable service acl", "Enable service queue", "Enable service policer", "Copy to cpu", "Forwarding ID", "<0-16383>", "Redirect to", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpls", "Enable vsi learning", "Enable vsi mac security", "Logic port", "<0-0x1DFF>", "Logic-port-type", "It is leaf node in E-Tree networks", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Bind", "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", "MAC source address", "MAC address in HHHH.HHHH.HHHH format", "IPv4 source address", "IPv4 address in A.B.C.D format", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan id", "<1-4095>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", "Egress service queue", "Vlan Edit", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag vlan id", "<1-4095>", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cos", "<0-7>", "Scfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New stag cfi", "0-1", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag vlan id", "<1-4095>", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cos", "<0-7>", "Ccfi select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "New ctag cfi", "0-1", "Discard the packet", "Statistic", "0~0xFFFFFFFF", "Aging", ((void *)0) };

char *ctc_cli_aps_create_and_remove_raps_group_cmd_help[] = { "Aps Module", "Aps bridge", "Raps", "Create", "Remove", "Raps mcast group id", "<0-2047>", ((void *)0) };

char *ctc_cli_parser_l3_type_cmd_help[] = { "Parser module", "Layer 3 header of packet", "ipv4", "ipv6", "mpls", "mpls mcast", "arp", "rarp", "eapo", "ethoam", "slow-proto", "cmac", "ptp", "enable l3 type parser", "disable l3 type parser", ((void *)0) };

char *ctc_cli_nh_remove_rspan_cmd_help[] = { "Nexthop module", "Delete nexthop", "RSPAN nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_learning_aging_logic_port_mapping_srcport_cmd_help[] = { "Learning module", "Logic port", "0-2240", "Logic port mapping", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_gb_chip_agent_pkt_rx_print_cmd_help[] = { "Chip Agent module", "Packet", "Print CPU RX packet", ((void *)0) };

char *ctc_cli_l2mcast_add_addr_cmd_help[] = { "Layer 2 module", "Multicast", "Add operation", "Multicast Group", "Mcast Group ID,it is allocated in platform adaption Layer", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Forwarding ID", "<0-16383>", "Protocol Entry flag", "When removing group,  only delete all member", "PTP mac address for ptp process", ((void *)0) };

char *ctc_cli_gb_chip_agent_set_sim_action_cmd_help[] = { "Chip Agent module", "Simulator log action", "Start log", "Log server print", "Log client print", "Log client file", "Stop log", ((void *)0) };

char *ctc_cli_parser_show_ecmp_hash_cmd_help[] = { "Show running system information", "Parser module", "Ecmp hash field", "Layer3 ip", "l3protocol in ecmp hash", "dscp in ecmp hash", "ipv6 flow label in ecmp hash", "ipsa in ecmp hash", "ipda in ecmp hash", "Layer4", "src port in ecmp hash", "dst port in ecmp hash", "gre key in ecmp hash", "Mpls", "mpls l3protocol in ecmp hash", "mpls dscp in ecmp hash", "mpls ipv6 flow label in ecmp hash", "mpls ipsa in ecmp hash", "mpls ipda in ecmp hash", "Fcoe", "sid in ecmp hash", "did in ecmp hash", "Trill", "inner vlan id in ecmp hash", "ingress nickname in ecmp hash", "egress nickname in ecmp hash", "Layer2 ethernet", "Add cos into ecmp hash", "Add macsa into ecmp hash", "Add macda into ecmp hash", ((void *)0) };

char *ctc_cli_port_set_cpumac_en_cmd_help[] = { "Port module", "Cpu mac", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_qos_set_igs_exp_map_cmd_help[] = { "QoS module", "QoS domain maping", "<1-7> 0 is invalid", "Mapping", "MPLS EXP", "<0-7>", "To", "Priority", "<0-0x3F>", "Color", "Green", "Yellow", "Red", ((void *)0) };

char *ctc_cli_port_set_all_mac_en_cmd_help[] = { "Port module", "All port", "Port mac", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_stacking_show_trunk_info_cmd_help[] = { "Show", "Stacking", "Trunk", "Trunk id", "Value <1-63>", "Information", ((void *)0) };

char *ctc_cli_chip_set_gephy_scan_special_reg_cmd_help[] = { "chip module", "set operate", "auto scan operate", "special reg", "ge phy", "phy register", "<0~31>", "enable", "bit control", "<0~15>", "interrupt", "interrupt enable", "interrupt disable", "scan disable", ((void *)0) };

char *ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "Classify pdu based on type", "Base on 24bit macda", "Entry index", "<0-7>", ((void *)0) };

char *ctc_cli_dbg_tool_debug_show_cmd_help[] = { "debug tool", "debug information", "debug status", ((void *)0) };

char *ctc_cli_nh_add_mpls_pop_nexthop_cmd_help[] = { "Nexthop module", "Add nexthop", "MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Mpls pop nexthop", "Unresolved nexthop", "Forward Nexthop", "Mpls pop nexthop payload operation type", "POP PHP operation", "PHP's Pipe mode", "PHP's Short Pipe mode", "PHP's Uniform mode", "Enable sequence order", "Sequence counter index, <0-255>", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "Service queue", ((void *)0) };

char *ctc_cli_ptp_set_device_type_cmd_help[] = { "PTP module", "PTP device type", "Not PTP device,process the PTP massage as normal packet", "Ordinary clock", "Boundary clock", "End-to-end transparent clock or E2ETC+OC/BC", "Peer-to-peer transparent clock or P2PTC+OC/BC", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_PbCtl_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show PbCtl statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_chip_set_access_mode_cmd_help[] = { "chip module", "set operate", "access", "mode", "PCI access", "I2C access", ((void *)0) };

char *ctc_cli_nh_remove_misc_l2hdr_cmd_help[] = { "Nexthop module", "Delete nexthop", "Misc nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_chip_write_gephy_interface_cmd_help[] = { "chip module", "write operate", "ge phy", "index of port", "<0-0x1DFF>", "phy register", "<0~31>", "value", "reg value", ((void *)0) };

char *ctc_cli_qos_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "QoS module", "Ctc layer", "Sys layer", "QoS policer", "QoS classification", "QoS queue", ((void *)0) };

char *ctc_cli_port_set_srcdiscard_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port src_discard", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_ipmc_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Ipmc", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "Classify pdu based on type", "Base on 48bit macda", "Entry index", "<0-3>", ((void *)0) };

char *ctc_cli_ipuc_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "IPUC module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_bpe_debug_on_cmd_help[] = { "Debugging functions", "bpe module", "CTC Layer", "SYS Layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_port_show_system_max_frame_size_cmd_help[] = { "Show running system information", "System attribution", "Max frame size to apply port select", "Max frame size control by buffer", ((void *)0) };

char *ctc_cli_gb_oam_cfm_dump_maid_cmd_help[] = { "OAM module", "CFM", "Dump registers or tables", "MAID table", "Chip id on linecard", "<0-1>", "MAID table index", "<0- max 1023 for 16bytes, 340 for 48byts>", ((void *)0) };

char *ctc_cli_stacking_show_header_cmd_help[] = { "Show", "Stacking", "Header", "Infomation", ((void *)0) };

char *ctc_cli_gb_ipmc_show_ipv6_group_info_cmd_help[] = { "Show running system information", "IPv6", "IPMC module", "IP multicast group", "Information, including source and group IP adress, etc.", ((void *)0) };

char *ctc_cli_gb_l2_show_fdb_status_cmd_help[] = { "Show running system information", "Layer 2 module", "FDB table", "FDB Status", ((void *)0) };

char *ctc_cli_port_set_stretch_mode_cmd_help[] = { "Port module", "<0-0x1DFF>", "Stretch mode(WAN mode)", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_gb_chip_agent_debug_simulator_on_cmd_help[] = { "Debugging functions", "Chip Agent module", "Simulator", "Packet IO", ((void *)0) };

char *ctc_cli_scl_uninstall_entry_cmd_help[] = { "SCL module", "Uninstall from hardware", "SCL entry id", "<0~0x7FFFFFFF>", ((void *)0) };

char *ctc_cli_dbg_tool_scan_xgphy_sp_para_cmd_help[] = { "Scan cmd", "Set xgphy special scan parameter cmd", "Dev type", "<0-0x1F>", "Reg address", "<0-0x1F>", "Bit Ctrl", "<0-0xF>", "Interrupt enable", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_l3if_show_router_mac_cmd_help[] = { "Show running system information", "L3 interface module", "Router mac", ((void *)0) };

char *ctc_cli_chip_read_xgphy_interface_cmd_help[] = { "chip module", "read operate", "xg phy", "index of port", "<1~52>", "device eg:pmd/wis/phy xgxs/dte xgxs", "device no", "phy register", "<0~31>", ((void *)0) };

char *ctc_cli_acl_debug_on_cmd_help[] = { "Debugging functions", "ACL module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_dbg_tool_run_sgmac_prbs_xauipcs_cmd_help[] = { "Run cmd", "Sgmac-prbs cmd", "Pcsmode xauipcs", "Port id", "<4-12>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_parser_set_linkagg_hash_cmd_help[] = { "Parser module", "Linkagg hash field", "Layer2 ethernet", "Add vlan into linkagg hash", "Add cos into linkagg hash", "Add ethertype into linkagg hash", "Add double vlan into linkagg hash", "Add macsa into linkagg hash", "Add macda into linkagg hash", "Layer3 ip", "Add l3protocol into linkagg hash", "Add ipsa into linkagg hash", "Add ipda into linkagg hash", "Layer4", "Add src port into linkagg hash", "Add dst port into linkagg hash", "Add gre key into linkagg hash", "Provider backbone", "Add bvlan into linkagg hash", "Add bcos into linkagg hash", "Add bmacsa into linkagg hash", "Add bmacda into linkagg hash", "Mpls", "Add mpls l3protocol into linkagg hash", "Add mpls ipsa into linkagg hash", "Add mpls ipda into linkagg hash", "Fcoe", "Add sid into linkagg hash", "Add did into linkagg hash", "Trill", "Add ingress nickname into linkagg hash", "Add egress nickname into linkagg hash", ((void *)0) };

char *ctc_cli_l3if_set_property_cmd_help[] = { "L3 interface module", "L3if ID", "<0-1022>", "IP property", "Transmission enable, 1:enable, 0:disable (only support in HUMBER)", "Receive enable, 1:enable, 0:disable (only support in HUMBER)", "Routing enable, 1:enable, 0:disable", "PBR label, <0-63>", "The type of nat interface,0- external,1- internal", "All packets are routed, 1:enable, 0:disable", "IPv4 unicast enable, 1:enable, 0:disable", "IPv4 multicast enable, 1:enable,0:disable", "IPv4 multicast RPF check is enabled, 1:enable, 0:disable (only support in HUMBER)", "IPv4 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR", "IPv6 unicast enable, 1:enable, 0:disable", "IPv6 multicast enable, 1:enable, 0:disable", "IPv6 multicast RPF check enable, 1:enable, 0:disable (only support in HUMBER)", "IPv6 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR", "VRF id of route <0-8191>", "Enable route lookup with vrfid", "Mtu check enable, 1:enable, 0:disable", "Mtu exception to cpu enable, 1:enable, 0:disable", "Mtu size of the interface, <0-16383>", "Route mac prefix type, range<0-2> (HUMBER not support)", "Route mac low 8bits, in 0xHH format (HUMBER not support)", "Egress macsa prefix type, range<0-2>", "Egress macsa low 8bits, in 0xHH format", "Multicast TTL threshold, range<0-255>", "ARP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard (only support in HUMBER)", "DHCP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard (only support in HUMBER)", "RPF check type, 0:strick mode, 1:loose mode", "RPF permit default, 0:disable, 1:enable", "Ingress vlan acl enable, bitmap<0-3>", "Ingress acl0 vlan classid, range<0-127>", "Ingress acl1 vlan classid, range<0-127>", "Engress vlan acl enable, bitmap<0-3>", "Engress acl0 vlan classid, range<0-127>", "Engress acl1 vlan classid, range<0-127>", "Statistic supported", "Value", "Value of the property", ((void *)0) };

char *ctc_cli_dbg_tool_read_hss6g_cmd_help[] = { "Read cmd", "HSS6G cmd", "Address", "Address value", "Data len", "<0-32767>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_nh_add_misc_cpu_reason_cmd_help[] = { "Nexthop module", "Add nexthop", "Misc nexthop", "TO CPU with custom reason", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "CPU Reason", "CPU Reason ID", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_HashDsMem_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show HashDsMem statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_mpls_add_vpls_ilm_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Incoming label mapping", "Add ilm", "Vpls vc label", "Label, <0-1048575>", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "PW ID", "Vpls port type, means this vpls PW is not a H-VPLS tunnel", "Control word used", "OAM Enable", "Service id", "Statistics", "0~0xFFFFFFFF", ((void *)0) };

char *ctc_cli_ipmc_remove_ipv4_group_cmd_help[] = { "IPMC module", "Remove", "IP multicast group", "Group address, class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "Do IP based L2 mcast", ((void *)0) };

char *ctc_cli_update_ip_tunnel_cmd_help[] = { "Nexthop module", "Update nexthop", "IP Tunnel nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Update forward nexthop", "Unresolved nexthop to forward nexthop", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "Tunnel type", "Tunnel over ipv4", "Ipv6inIpv4", "Ipv4inIpv4", "IP-BFD enable the nexthop", "6to4", "Use Manual IPSA", "6rd", "6rd IPv6 SP prefix length (default is 32)", "6rd IPv6 SP prefix length (0:16 1:24 2:32 3:40 4:48 5:56)", "IPv4 source address mask length (default is 32) ", "IPv4 source address mask length ", "IPv4 destination address mask length (default is 32)", "IPv4 destination address mask length ", "ISATAP", "GRE in Ipv4", "Protocol type of GRE Header", "Protocol type of GRE Header(Ipv4:0x0800,ipv6:0x86DD,ethernet:0x6558)", "GRE header with GRE key", "GRE key,if mtu-check-en is set,GRE Key only 16 bit, else 32 bit", "Mirror over GRE tunnel", "Ip sa", "IPv4 address in A.B.C.D format", "Ip da", "IPv4 address in A.B.C.D format", "If set , means new dont frag will copy payload ip dont frag ", "Use defined dont frag", "Dont frag <0~1>", "Tunnel over ipv6", "Ipv4inIpv6", "IP-BFD enable the nexthop", "Ipv6inIpv6", "GRE in Ipv6", "Protocol type of GRE Header", "Protocol type of GRE Header(Ipv4:0x0800,ipv6:0x86DD,ethernet:0x6558)", "GRE header with GRE key", "GRE key 32 bit", "Mirror over GRE tunnel", "Ip sa", "IPv6 address in X:X::X:X format", "Ip da", "IPv6 address in X:X::X:X format", "IVI", "IPv6 translate to IPv4", "IPv4 translate to IPv6", "IPv6 prefix address", "IPv6 address in X:X::X:X format", "Prefix length", "Length value 0-32, 1-40, 2-48, 3-56, 4-64, 5-72, 6-80, 7-96", "Prefix obey rfc6052, if set, IPv6 address's 64 to 71 bits are set to zero", "NAT", "New ipda", "IPv4 address in A.B.C.D format", "New layer4 dest port", "Value <0-65535>", "If set,encap packet will re-route with new tunnel header", "Dscp if ipv4 tunnel, TOS if ipv6 tunnel", "Dscp value: <0~0x3F>, Tos value: <0~0xFF>", "Dscp select mode <0~2>", "Dscp select mode:0,Use user-define DSCP to outer header,1-Use DSCP value from DSCP map,2-Copy packet DSCP to outer header", "TTL", "TTL value: <0~0xFF>", "TTL mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "IPv6 flow label mode (default is do not set flow label value)", "IPv6 flow label mode:0-Do not set flow label valu, 1-Use (user-define flow label + header hash)to outer header , 2- Use user-define flow label to outer header", "IPv6 flow label value (default is 0)", "IPv6 flow label value <0-0xFFFFF>", ((void *)0) };

char *ctc_cli_ipuc_add_ipv4_nat_sa_cmd_help[] = { "IPUC module", "Nat sa", "Add", "VRF id of route <0-8191>", "IPv4 address in A.B.C.D format", "Layer4 source port for NAPT", "<0-0xFFFF>", "Indicate l4-src-port is tcp port or not. If set is TCP port else is UDP port", "New ipsa", "IPv4 address in A.B.C.D format", "Layer4 destination port for NAPT", "<0-0xFFFF>", ((void *)0) };

char *ctc_cli_port_show_port_property_cmd_help[] = { "Show running system information", "Port module", "<0-0x1DFF>", "Property", "Show all property", "Route enable", "Packet tag take precedence over all", "Hareware learning", "Scl ipv6 lookup disable", "Scl ipv4 lookup disable", "Scl default lookup", "Scl MAC key is used for lookup instead of ipv6 key", "Scl MAC key is used for lookup instead of ipv4 key", "Trill enable", "NON-TRILL packet discard", "TRILL packet discard", "Bridge to the same port enable", "FCoE enable", "FCoE RPF enable", "The STAG COS field is replaced by the classified COS result", "The CTAG COS field is replaced by the classified COS result", "The Dscp field is replaced by the qos mapping result", "ARP packet processing type", "DHCP packet processing type", "RPF check for outer tunnel IP header is enabled", "For untagged PTP packet and routedPort", "Strict or loose", "This port connect with a leaf node", "Priority tagged packet will be sent out", "Default cos of vlantag", "Default dei of vlantag", "Raw packet type", "Pvlan port type", "Qos trust policy", "auto neg", "link interrupt", "Timestamp", "Port linkscan function enable", "Aps failover enable", "Linkagg failover enable", ((void *)0) };

char *ctc_cli_parser_l2_set_pbb_header_cmd_help[] = { "Parser module", "Layer 2 header of packet", "Provider backbone", "Header", "Nca enable", "Enable functions", "Disable functions", "Outer vlan is cvlan", "Enable functions", "Disable functions", "Vlan parsing num", "Vlan parsing num value<0-3>", ((void *)0) };

char *ctc_cli_common_exit_config_cmd_help[] = { "Exit current mode and down to previous mode", ((void *)0) };

char *ctc_cli_nh_show_stats_cmd_help[] = { "Show running system information", "Nexthop module", "Statistics", "Nexthop id", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "Portection path", ((void *)0) };

char *ctc_cli_dbg_tool_read_temp_cmd_help[] = { "Read cmd", "Read temp cmd", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_dbg_tool_show_bridge_header_cmd_help[] = { "show cmd", "ipe/bsr/epe/oam packet header info", "ipe forward packet header", "epe header adjust packet header", "epe packet header index", "<0, 31>", "cpu to buffer store packet header", "iloop to buffer store packet header", "eloop to buffer store packet header", "oam to buffer store packet header", "buffer retrieve packet header", "channel id of packet header", "<0, 63>", "start capture oam-engine packet header", "oam engine packet header", "oam engine packet header index", "<0, 8>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_clear_rmep_dcsf_cmd_help[] = { "Remote MEP", "CSF defect", "Clear CSF defect", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_ipmc_add_ipv6_group_cmd_help[] = { "IPMC module", "IPv6", "Add", "IP multicast group", "Mcast Group ID,it is allocated in platform adaption Layer", "Group address, IPv6 address begin with FF", "IPv6 address in X:X::X:X format", "The length of group address mask <128>", "Source address", "IPv6 address in X:X::X:X format", "The length of source address mask <0 or 128>", "VRF id of route", "VRF id of route <0-8191>", "TTL check enable", "RPF check enable", "If RPF check fail, sent to cpu", "Statistic supported", "0~0xFFFFFFFF", "Do IP based L2 mcast", "Ptp-entry", "Redirect to cpu", ((void *)0) };

char *ctc_cli_gb_ftm_show_opf_alloc_info_cmd_help[] = { "Show running system information", "Offset pool freelist", "OPF alloc info", "OPF type", "OPF type value", "Index of opf pool", "<0-0xFF>", ((void *)0) };

char *ctc_cli_vlan_set_arp_dhcp_excp_cmd_help[] = { "VLAN module", "<1-4095>", "ARP packet of vlan", "DHCP packet of vlan", "Exception action", "Forwarding and to cpu", "Normal forwarding", "Redirect to cpu", "Discard", ((void *)0) };

char *ctc_cli_mpls_add_normal_ilm_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Incoming label mapping", "Add ilm", "Mpls label space", "Label space id, <0-255>, space 0 is platform space", "Mpls label", "Label, reference to show space", "Nexthop id, humber must set", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Uniform model", "Short pipe model", "Pipe model", "Pop operation label", "Decap Inner Packet", "Output mpls interface label space", "Interface label space id, <1-255>", "OAM Enable", "APS select ", "<0-2047>", "Indicate working path", "Indicate protection path", "Statistics", "0~0xFFFFFFFF", "Use outer label's exp", "Inner packet type is ipv6", ((void *)0) };

char *ctc_cli_oam_cfm_update_rmep_csf_en_cmd_help[] = { "Remote MEP", "Client signal fail", "Enable client signal fail", "Disable client signal fail", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_oam_cfm_set_tx_vlan_tpid_cmd_help[] = { "OAM module", "Vlan tpid for OAM pdu tx", "Set tpid", "Tpid index", "<0-1>", "Tpid", "0xXXXX", ((void *)0) };

char *ctc_cli_port_set_pading_cmd_help[] = { "Port module", "<0-0x1DFF>", "L2 pading", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_vlan_add_vlan_mapping_cmd_help[] = { "VLAN module", "Vlan mapping", "Add vlan mapping entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Stag vlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Ctag vlan, or start vlan of vlan range", "<1-4095>", "Vlan range, end vlan", "<1-4095>", "Vlan range group id", "<0~63>", "Stag priority or cos", "<0-7>", "Ctag priority or cos", "<0-7>", "Mapping to", "Mapped cvlan", "<1-4095>", "New stag cos", "<0-7>", "New ctag cos", "<0-7>", "Service queue id", "Service id", "Ingress service ", "Egress service ", "Enable service acl", "Enable service policer", "Enable service queue", "Mapped svlan", "<1-4095>", "Logic port", "Logic-port-type", "<0 -0x1FFF>", "Aps selector", "<0-2047>", "Protected vlan, maybe working vlan", "<1-4095>", "This is working path", "This is protection path", "Forwarding ID", "<0-16383>", "Vpls fid", "Disable vsi learning", "Enable vsi mac security", "It is leaf node in E-Tree networks", "Nexthop instance to forward", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Vpws nexthop", "It is leaf node in E-Tree networks", "Vlan switching", "Stag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Svid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Scos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Ctag operation type", "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)", "Cvid select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "Ccos select", "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "Statistic", "0~0xFFFFFFFF", "Enable aging", ((void *)0) };

char *ctc_cli_acl_uninstall_entry_cmd_help[] = { "ACL module", "Uninstall from hardware", "ACL entry id", "<0~0xFFFFFFFF>", ((void *)0) };

char *ctc_cli_gb_vlan_set_port_tag_cmd_help[] = { "VLAN module", "<1-4095>", "Gport id", "<0-0x1DFF>", "Vlan tagged on this port", "Vlan untagged on this port", ((void *)0) };

char *ctc_cli_ipmc_update_ipv4_rpf_cmd_help[] = { "IPMC module", "Update", "Reverse path forward", "Group address, class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "L3if ID", "<0-1023>, 0:disable rpf check", "L3if ID", "<0-1023>, 0:disable rpf check", "L3if ID", "<0-1023>, 0:disable rpf check", "L3if ID", "<0-1023>, 0:disable rpf check", "More than 4 rpf interfaces, send to CPU for checking", ((void *)0) };

char *ctc_cli_dbg_tool_integrity_check_cmd_help[] = { "integrity-check generate cmd", "Chip id on linecard", "chip id value", "golden file name", "template file name", ((void *)0) };

char *ctc_cli_common_fast_enter_cmodel_mode_cmd_help[] = { "Enter cmodel mode", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_TcamDs_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show TcamDs statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_gb_ftm_opf_debug_on_cmd_help[] = { "Debugging functions", "Offset pool freelist", "Fast priority arrangement", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_gb_interrupt_show_interrupt_sdk_cmd_help[] = { "Show running system information", "Interrupt module", "Infomation in SDK", ((void *)0) };

char *ctc_cli_vlan_set_replace_dscp_cmd_help[] = { "VLAN module", "<1-4095>", "Replace packet dscp by classified dscp", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Classify pdu based on type", "Base on 24bit macda", "Entry index", "<0-7>", "MAC address", "MAC address in HHHH.HHHH.HHHH format,0180.C2XX.XXXX", "Macda mask, the mask works only if entry index set 0 or 1", "MAC address in HHHH.HHHH.HHHH format,FFFF.FFXX.XXXX", ((void *)0) };

char *ctc_cli_dbg_tool_show_bsr_QMgrEnq_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "bsr status", "show QMgrEnq statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_vlan_show_dir_property_cmd_help[] = { "Show running system information", "VLAN module", "<1-4095>", "Vlan property with direction", "All property", "Acl enable (bitmap 0~3)", "Vlan acl classid", "Vlan acl against routed-packet only", "Flow direction", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_vlan_set_receive_cmd_help[] = { "VLAN module", "<1-4095>", "If set, receive on the vlan is enable", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_learning_aging_show_port_nhid_cmd_help[] = { "Show running system information", "Learning module", "Port to nexthop mapping", "Port to nexthop mapping", ((void *)0) };

char *ctc_cli_scl_debug_on_cmd_help[] = { "Debugging functions", "SCL module", "Ctc layer", "Sys layer", "Set the debugging output level ", "Output enter the functions with parameters ", "Output the parameters information when enter the functions or invoke other functions ", "Output the general information ", "Output the error  information ", ((void *)0) };

char *ctc_cli_nh_add_mpls_tunnel_label_cmd_help[] = { "Nexthop module", "Add nexthop", "Update nexthop", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "Aps bridge id, effective when tunnel has protection path", "<0-1023>", "LSP+SPME 2 level aps", "This tunnel is working lsp", "This tunnel is protecting lsp", "APS working path parameter", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value,0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "MPLS label2", "MPLS label2 valule", "MPLS label2's ttl", "MPLS label2's ttl value(0~255)", "MPLS EXP2", "MPLS EXP2 value(0-7)", "MPLS EXP2 type(0-2)", "MPLS EXP2 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label2's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label2 is mcast label", "MPLS label stats enable", "Stats id", "APS protection path parameter", "MAC address", "MAC address in HHHH.HHHH.HHHH format", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Routed Port interface", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "outer Vlan is svlan", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value,0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "MPLS label2", "MPLS label2 valule", "MPLS label2's ttl", "MPLS label2's ttl value(0~255)", "MPLS EXP2", "MPLS EXP2 value(0-7)", "MPLS EXP2 type(0-2)", "MPLS EXP2 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label2's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label2 is mcast label", "MPLS label stats enable", "Stats id", ((void *)0) };

char *ctc_cli_pdu_l2_set_port_action_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Per-port l2pdu action", "Port", "<0-0x1DFF>", "Per-port action index", "<0-14>", "Action type", "Forward packet", "Forward and send to cpu", "Direct to cpu", "Discard packet", ((void *)0) };

char *ctc_cli_acl_get_multi_entry_cmd_help[] = { "Show", "ACL module", "ACL group id", "<0xFFFF0002>", "Entry size", "0: show entry count in this group. 1 ~  : show entry IDs in that number", ((void *)0) };

char *ctc_cli_pdu_show_l2_global_action_cmd_help[] = { "Show running system information", "Pdu modue", "Layer2 PDU", "Global action", "Base on 48bit Macda", "Entry index", "<0-3>", "Base on 24bit Macda", "Entry index", "<0-7>", "Base on layer2 header protocol", "Entry index", "<0-15>", "Bpdu", "Slow protocol", "Eapol", "Lldp", "Layer3 type", "Entry index", "<0-15>", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_Qsgmii_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show Qsgmii statistic", "Index", "<0-11>", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_chip_show_chip_clock_cmd_help[] = { "show", "chip", "clock", "frequence", ((void *)0) };

char *ctc_cli_oam_set_big_interval_to_cpu_cmd_help[] = { "OAM module", "CFM", "Big interval to CPU", "The big interval <1~7>", ((void *)0) };

char *ctc_cli_ptp_set_sync_intf_cmd_help[] = { "PTP module", "<0-1>", "Sync interface config", "Sync interface work mode", "Input mode", "Out mode", "Set lock", "Enable lock", "Disable lock", "Timer accuracy", "<32-49,or 0xFE>", "SyncClock signal frequency", "<0-25000000>", "SyncPulse signal frequency", "<0-250000>", "SyncPulse signal duty", "<1-99>", "Bit clock cycles before time code", "<0-63>", ((void *)0) };

char *ctc_cli_dbg_tool_show_misc_PciExpCore_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "misc status", "show PciExpCore statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch_cmd_help[] = { "Remote MEP", "Remote MEP MAC mismatch", "Clear the mismatch status", "MEP level", "<0-7>", "MEP id", "<1-8191>", ((void *)0) };

char *ctc_cli_gb_dma_mpool_dbg_cmd_help[] = { "Show running system information", "Dma module", "mpool", "state", ((void *)0) };

char *ctc_cli_gb_vlan_remove_protocol_vlan_cmd_help[] = { "VLAN module", "Vlan classification", "Remove one vlan classification entry", "Protocol based vlan", "Ethertype", "<0x0000-0xFFFF>", ((void *)0) };

char *ctc_cli_stacking_remove_trunk_cmd_help[] = { "Stacking", "Destroy", "Trunk", "Trunk id <1-63>", ((void *)0) };

char *ctc_cli_port_show_port_dir_property_cmd_help[] = { "Show running system information", "Port module", "<0-0x1DFF>", "Property with direction", "All property", "Acl enable (bitmap 0~3)", "Port acl port classid", "Port acl port classid0", "Port acl port classid1", "Port acl ipv4-key force to mac-key", "Port acl ipv6-key force to mac-key", "Port acl force use ipv6", "Port acl use classid", "Port service acl enable", "Port qos domain", "Port policer valid", "Svlan tpid index, the corresponding svlan tpid is in EpeL2TpidCtl", "Rate of random threshold,0-0x7fff", "Direction", "Ingress", "Egress", ((void *)0) };

char *ctc_cli_qos_set_phb_configure_cmd_help[] = { "QoS module", "Map", "Priority", "Value <0-63>", "To", "PHB", "Value <0-3>", ((void *)0) };

char *ctc_cli_dbg_tool_show_epe_NetTx_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "epe status", "show NetTx statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_acl_remove_all_entry_cmd_help[] = { "ACL module", "Remove all entries of a specified group from software table", "ACL group id", "<0xFFFF0002>", ((void *)0) };

char *ctc_cli_gb_dma_show_stats_cmd_help[] = { "Show running system information", "Dma module", "packet DMA RX", "statistics", ((void *)0) };

char *ctc_cli_stats_set_global_cfg_cmd_help[] = { "Statistics", "Saturate when at max value", "StatsPtrFifo on hold", "Clear after read", "Ingress port", "Egress port", "Ingress global fwd", "Egress global fwd", "Forward", "Gmac", "Sgmac", "Xgmac", "Cpumac", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_ipmc_remove_ipv4_member_cmd_help[] = { "IPMC module", "Remove", "Member of the ipmc group", "Group address, class D IP address", "IPv4 address in A.B.C.D format", "The length of group address mask <32>", "Source address", "IPv4 address in A.B.C.D format", "The length of source address mask <0 or 32>", "VRF id of route", "VRF id of route <0-8191>", "Sub interface", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "VLAN", "<1-4095>", "VLAN interface", "VLAN", "<1-4095>", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Vlan port", "Physical interface/remote chip id", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Remove a member port for a IP based l2 mcast", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Indicate the member is remote chip entry", "Add member by nexthop, can be used in ipmc and ip based l2mc", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", ((void *)0) };

char *ctc_cli_ptp_set_global_property_cmd_help[] = { "PTP module", "Global property", "PTP ucast will be processed", "TC will copy sync and follow_up message to CPU", "Copy signaling message to CPU", "Copy management message to CPU", "P2P transparent clock will process Delay_Request message", "PTP will be enabled by port or vlan, default by port", "Aging time for timestamp", "Select Sync Interface or ToD Interface to synchronize from master clock", "Property value", "Value", ((void *)0) };

char *ctc_cli_dbg_tool_show_share_DsAging_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "share status", "show DsAging statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_port_set_mac_en_cmd_help[] = { "Port module", "<0-0x1DFF>", "Port mac", "Enable functions", "Disable functions", ((void *)0) };

char *ctc_cli_dbg_tool_show_ipe_IpeHdrAdj_cmd_help[] = { "show chip statistic info cmd", "ipe/bsr/mac/misc/oam/share status info", "ipe status", "show IpeHdrAdj statistic", "Chip id on linecard", "<0-1>", ((void *)0) };

char *ctc_cli_ptp_adjust_clock_offset_cmd_help[] = { "PTP module", "<0-1>", "Adjust clock offset", "The offset is negative", "Seconds of offset", "<0-0xFFFFFFFF>", "Nanoseconds of offset", "<0-(10^9-1)>", "Nano nanoseconds of offset", "<0-(10^9-1)>", ((void *)0) };

char *ctc_cli_common_enter_internal_debug_mode_cmd_help[] = { "Enter", "Internal Debug mode", "Mode", ((void *)0) };

char *ctc_cli_pdu_l2_set_global_action_by_macda_cmd_help[] = { "Pdu modue", "Layer2 PDU", "Global action", "Base on 48bit Macda", "Entry index", "<0-3>", "Entry valid", "<0-0xFF>,when great than 1,as 1 to process", "Bypass all, only humber support", "<0-0xFF>,when great than 1,as 1 to process", "Per-port action index", "<0-14>", ((void *)0) };

char *ctc_cli_mpls_ac_add_vlan_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "L2vpn attachment circuit entry", "Add l2vpn ac", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", "Port binding to l2vpn with svlan", "Port binding to l2vpn with cvlan", "<1-4095>", "Virtual private wire services ", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "Virtual private lan services", "<0-16383>", ((void *)0) };

char *ctc_cli_aps_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Aps Module", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_common_show_memory_detail_cmd_help[] = { "Show running system information", "Memory information", "Memory pool", "Detail of memory statistics", ((void *)0) };

char *ctc_cli_l2_remove_ucast_nexthop_cmd_help[] = { "Layer 2 module", "FDB table", "Remove operation", "Ucast bridge nexthop entry", "Global port", "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>", ((void *)0) };

char *ctc_cli_l2_show_stp_state_by_port_and_stpid_cmd_help[] = { "Show running system information", "Spanning Tree Protocol (STP)", "Show stp state", "Global port", "<0-0x1DFF>", "MST instance", "MST instance ID <0-127>", ((void *)0) };

char *ctc_cli_common_debug_show_cmd_help[] = { "Show running system information", "Debugging functions", "Enable debugging information", ((void *)0) };

char *ctc_cli_oam_cfm_add_lmep_cmd_help[] = { "Local MEP", "Add local MEP on oam channel", "MEP id", "<1-8191>", "MEP group id", "MEG id string", "MD Name", "MD name string", "MA Name", "MA name string", "MEP is active or not", "CCM interval", "<1-7>", "Delay measurement", "MEP on cpu", "Enable APS message to CPU", "MEP enable LM", "LM type: 0 means not count; 1 means dual-end LM; 2 means single-end LM", "LM Cos type: 0 All cos data counter together; 1 Only count specified cos data; 2 Per cos date to count", "LM Cos <0-7>", "MEP enable CSF", "CSF type: 0 LOS; 1 FDI; 2 RDI; 3 DCI", "Ethernet OAM option", "MEP level", "<0-7>", "MEP is P2P mode", "MEP sequence number enable", "MEP is VPLS OAM", "MEP Shared address mode", "Tx interface status TLV", "Tx port status TLV", "Tx sender id TLV", "CCM tx tpid type index", "<0-1>", "Ccm gport id[HB]", "<0-0x1DFF>", "Tx mcast group id for up MEP[HB]", "Mcast Group ID,it is allocated in platform adaption Layer", "MPLS TP Y.1731 OAM option", "APS Protection PATH", "NextHop ID for MEP out", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "MPLS TTL", "TTL, <0-255>", "MPLS TP Y.1731 OAM without GAL", "MEP index allocated by system", "2~max MEP index", ((void *)0) };

char *ctc_cli_nh_add_mpls_push_nexthop_cmd_help[] = { "Nexthop module", "Add nexthop", "MPLS nexthop", "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151", "If it isn't be selected,the dsnh_offset will be internally allocated in SDK", "DsNexthop Table's Offset", "Mpls push nexthop", "Unresolved nexthop", "Forward Nexthop", "Enable H-VPLS in VPLS network", "Destination logic port in VPLS network", "MPLS vpls port value <0-8191>", "Mpls push nexthop payload operation type", "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)", "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)", "CVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "SVLAN Editing Type", "0 for no egress editing, keep ingress editing; 1 for keep unchange, " "will recover ingress editing although ingress editing have done; " "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan", "Output CVLANID", "<1-4095>", "Output SVLANID", "<1-4095>", "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", "Enable swaping SVLAN's TPID with CVLAN's TPID", "Enable swaping SVLAN's Cos with CVLAN's Cos", "Replace SVLAN's Cos", "0-7", "Map SVLAN's Cos", "Set svlan tpid index", "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl", "Output CVLANID is valid", "Output SVLANID is valid", "User vlanptr", "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3.", "The nexthop is leaf node in E-Tree networks", "MPLS Tunnel", "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023", "MPLS label1", "MPLS label1 valule", "MPLS label1's ttl", "MPLS label1's ttl value(0~255)", "MPLS EXP1", "MPLS EXP1 value(0-7)", "MPLS EXP1 type(0-2)", "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", "MPLS label1 is mcast label", "First label is martini control word", "Martini sequence type is none", "Martini sequence type is per-pw", "Martini per-pw sequence index", "Sequence index value", "Martini sequence type is global sequence type0", "Martini sequence type is global sequence type1", "Control word", "Control word value", "Mtu check enable", "Mtu size", "Mtu size value <0-0x3fff>", "Statistics", "Stats id", ((void *)0) };

char *ctc_cli_port_show_port_mac_cmd_help[] = { "Show running system information", "Port module", "<0-0x1DFF>", "Port MAC", ((void *)0) };

char *ctc_cli_mpls_add_stats_cmd_help[] = { "Multi-Protocol Label Switching (MPLS)", "Add action", "Statistics", "0~0xFFFFFFFF", "Label space id, <0-255>, space 0 is platform space", "Label, <0-1048575>", ((void *)0) };

char *ctc_cli_stacking_show_property_cmd_help[] = { "Show", "Stacking", "Break enable", "End chip bitmap", "Full mesh topo enable", ((void *)0) };

char *ctc_cli_l2_debug_off_cmd_help[] = { "Negate a command or set its defaults", "Debugging functions", "Layer 2 module", "FDB table", "Spanning Tree Protocol (STP)", "Ctc layer", "Sys layer", ((void *)0) };

char *ctc_cli_security_storm_ctl_show_cfg_cmd_help[] = { "Show running system information", "Security module", "Storm control", "Port configure", "Global port", "Vlan based Storm control", "<1-4095>", "Broadcast", "Known multicast", "Unknown multicast", "All multicast, known multicast + unknown multicast", "Known unicast", "Unknown unicast", "All unicast, known unicast + unknown unicast", ((void *)0) };

char *ctc_cli_gb_oam_cfm_dump_rmep_cmd_help[] = { "OAM module", "CFM", "Dump registers or tables", "Remote MEP table", "Chip id on linecard", "<0-1>", "Remote MEP table index", "<0-2047>", ((void *)0) };

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_port_nhid_mapping_cmd_vtysh,  "learning port-nhid (port GPORT_ID | logic-port LOGIC_PORT_ID) fid FID nexthop NH_ID",  ctc_cli_learning_aging_port_nhid_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipv4_member_info_cmd_vtysh,  "show ipmc member info group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (ip-based-l2mc|)",  ctc_cli_gb_ipmc_show_ipv4_member_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_bpe_get_interlaken_en_cmd_vtysh,  "show bpe interlaken",  ctc_cli_bpe_get_interlaken_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_add_rmep_cmd_vtysh,  "remote-mep bfd add my-discr MY_DISCR your-discr YOUR_DISCR ({mep-en  | required-rx-interval REQUIRED_RX_INTERVAL | state STATE | diag DIAG | detect-mult DETECT_MULT }|)    (ip-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL)}) (mep-index IDX|)",  ctc_cli_oam_bfd_add_rmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_bridge_cmd_vtysh,  "vlan VLAN_ID bridge (enable | disable)",  ctc_cli_vlan_set_bridge_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_preamble_cmd_vtysh,  "port GPHYPORT_ID preamble BYTES",  ctc_cli_port_set_preamble_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l3_l3flex_cmd_vtysh,  "show parser l3 flex",  ctc_cli_parser_show_l3_l3flex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_add_vlan_mapping_default_entry_cmd_vtysh,  "vlan mapping add port GPORT_ID mismatch (discard|do-nothing|send-to-cpu|service-id SERVICEID (ingress|egress)) (user-vlanptr VLAN_PTR|)",  ctc_cli_vlan_add_vlan_mapping_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_read_i2c_cmd_vtysh,  "chip read i2c dev-add ADDR bitmap BITMAP offset OFFSET length LENGTH",  ctc_cli_chip_read_i2c_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_remove_ipv6_group_cmd_vtysh,  "ipmc ipv6 remove group group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (ip-based-l2mc|)",  ctc_cli_ipmc_remove_ipv6_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_allocate_internal_port_cmd_vtysh,  "port allocate internal-port gchip GCHIP type (iloop|eloop|discard|fwd port GPHYPORT_ID)",  ctc_cli_port_allocate_internal_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_debug_cmd_vtysh,  "show debug chip (ctc | sys)",  ctc_cli_chip_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_parity_error_show_mapping_memory_cmd_vtysh,  "show parity-err-map-mem table TABLE_ID index INDEX",  ctc_cli_common_parity_error_show_mapping_memory_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_queue_pkt_len_cmd_vtysh,  "qos shape pkt-len-adjust" "((service-id SERVICE|) (port GPHYPORT_ID) (queue-id QUEUE_ID) | reason-id REASON)" "(pkt-adjust-len LEN)",  ctc_cli_qos_set_queue_pkt_len_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_dma_show_state_cmd_vtysh,  "show dma state (detail|)",  ctc_cli_gb_dma_show_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_set_aging_property_cmd_vtysh,  "aging ({fifo-threshold THRESHOLD} | { interval INTERVAL } | { fifo-scan (enable | disable) }) ((aging-tbl (mac|scl)) |)",  ctc_cli_learning_aging_set_aging_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_set_vlan_pas_num_cmd_vtysh,  "parser l2 ethernet vlan-pas-num VLAN_NUM",  ctc_cli_parser_l2_set_vlan_pas_num_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_port_group_class_priority_cmd_vtysh,  "qos schedule pri-propagation (service-id SERVICE |) (port GPHYPORT_ID) (queue-class CLASS) (priority PRIORITY)",  ctc_cli_qos_set_port_group_class_priority_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_show_error_desc_cmd_vtysh,  "show error ERROR_ENUM",  ctc_cli_common_show_error_desc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_sync_ether_show_cfg_cmd_vtysh,  "show sync-ehter CHIP clock CLOCK_ID",  ctc_cli_sync_ether_show_cfg_cmd_help)

DEFSH (VTYSH_SDK, show_cli_drv_write_reg_or_tbl_field_cmd_vtysh,  "write tbl-name TBL_NAME tbl-index INDEX (fld-id) FLD_ID (value) VALUE (lchip CHIP_ID|)",  show_cli_drv_write_reg_or_tbl_field_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2mcast_remove_addr_cmd_vtysh,  "l2 mcast remove group mac MAC fid FID",  ctc_cli_l2mcast_remove_addr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_clear_log_cmd_vtysh,  "clear stats port GPHYPORT_ID ( ingress | egress ) log",  ctc_cli_stats_clear_log_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_policer_stats_enable_cmd_vtysh,  "qos policer stats (enable | disable)",  ctc_cli_qos_set_policer_stats_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_debug_on_cmd_vtysh,  "debug stacking (ctc|sys)  (debug-level {func|param|info|error} |)",  ctc_cli_stacking_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_Parser_cmd_vtysh,  "show statistic (share) (parser) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_Parser_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_cpu_reason_gourp_mode_cmd_vtysh,  "qos cpu-group (reason-group GROUP | fwd-group)  dest-to (local-cpu | dma)",  ctc_cli_qos_set_cpu_reason_gourp_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_gephy_sp_dis_cmd_vtysh,  "scan gephy-special disable SPREG_INDEX (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_gephy_sp_dis_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_debug_off_cmd_vtysh,  "no debug on",  ctc_cli_common_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dma_show_debug_cmd_vtysh,  "show debug dma (ctc | sys)",  ctc_cli_dma_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_create_policer_cmd_vtysh,  "qos policer attach (port GPORT (in|out) |flow POLICER_ID|service SEVICE_ID)          (mode (rfc2697|rfc2698|rfc4115|bwp)) ((color-blind|color-aware)|)           (cir CIR) (cbs CBS |) ((pir PIR|eir EIR)|) ((pbs PBS|ebs EBS)|) (cf-en|)(drop-color (red|yellow)|) (use-l3-length|) (stats-en|)          ((hbwp-en schedule-mode (sp|wdrr weight WEIGHT) cos-index INDEX max-cir CIR_MAX (max-eir EIR_MAX|)(sf-en|) (cf-total-dis|))|)",  ctc_cli_qos_create_policer_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_show_ipv6_info_cmd_vtysh,  "show ipuc ipv6 VRFID X:X::X:X MASK_LEN (detail |)",  ctc_cli_ipuc_show_ipv6_info_cmd_help)

DEFSH (VTYSH_SDK, humber_cli_aps_remvoe_aps_bridge_group_cmd_vtysh,  "aps remove aps-bridge group GROUP_ID",  humber_cli_aps_remvoe_aps_bridge_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_debug_on_cmd_vtysh,  "debug ipuc (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_ipuc_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_access_mode_cmd_vtysh,  "access-mode (pcie|i2c)",  ctc_cli_dbg_tool_access_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_loopback_cmd_vtysh,  "port GPHYPORT_ID loopback-to dst-port DST_GPORT (enable | disable) (swap-mac|)",  ctc_cli_port_set_loopback_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_error_debug_cmd_vtysh,  "debug error (on|off)",  ctc_cli_common_error_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_enable_cmd_vtysh,  "local-mep (enable| disable) (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_bpe_debug_off_cmd_vtysh,  "no debug bpe (ctc|sys)",  ctc_cli_bpe_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_global_hash_type_cmd_vtysh,  "show parser global hash-type",  ctc_cli_parser_show_global_hash_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_gephy_cmd_vtysh,  "read gephy mdio-bus BUS_NUM phy-addr PHY_ADDR reg REG (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_gephy_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l4_set_flex_cmd_vtysh,  "parser l4 flex dest-port-basic-offset OFFSET min-len LEN",  ctc_cli_parser_l4_set_flex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_get_mep_stats_cmd_vtysh,  "local-mep stats level LEVEL",  ctc_cli_oam_get_mep_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_aps_show_bridge_cmd_vtysh,  "show aps bridge group GROUP_ID",  ctc_cli_gb_aps_show_bridge_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_drvio_clear_cache_cmd_vtysh,  "driver clear (cache | stats | all)",  ctc_cli_gb_drvio_clear_cache_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_remove_ilm_cmd_vtysh,  "mpls ilm remove space SPACEID label LABEL ( nexthop-id NHID |)",  ctc_cli_mpls_remove_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_show_port_cpu_mac_en_cmd_vtysh,  "show port cpumac enable",  ctc_cli_show_port_cpu_mac_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_linkagg_hash_cmd_vtysh,  "show parser linkagg-hash{l2 {vlan|cos|ethertype|double-vlan|macsa|macda}|ip {use-ip|l3protocol|ipsa|ipda}|l4 {use-l4|src-port|dst-port|gre-key}|    pbb{b-vlan|b-cos|b-macsa|b-macda }|mpls {use-mpls|m-l3protocol|m-ipsa|m-ipda}|fcoe{use-fcoe|sid|did}|trill{use-trill|ingress-nickname|egress-nickname}}",  ctc_cli_parser_show_linkagg_hash_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_tod_intf_tx_code_cmd_vtysh,  "ptp CHIP tod-tx-code (class CLASS) (id ID) (length LEN) (leap LEAP) (1pps-status PPSTATUS) (accuracy ACCURACY)     (clock-src SRC) (src-status SRCSTATUS) (alarm ALARM)",  ctc_cli_ptp_set_tod_intf_tx_code_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_show_l3if_id_cmd_vtysh,  "show l3if type (phy-if gport GPORT_ID | vlan-if vlan VLAN_ID)",  ctc_cli_l3if_show_l3if_id_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_xgphy_scan_special_reg_cmd_vtysh,  "chip set phy-scan special-reg xgphy dev DEV reg REG ((enable bit BIT_CTL intr (enable | disable))|disable)",  ctc_cli_chip_set_xgphy_scan_special_reg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_speed_cmd_vtysh,  "port GPHYPORT_ID speed-mode (eth|fe|ge|sg)",  ctc_cli_port_set_speed_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l4_set_app_data_cmd_vtysh,  "parser l4 app-data set index INDEX dst-port-mask DST_PORT_MASK dst-port-val DST_PORT_VAL    is-tcp-mask IS_TCP_MASK is-tcp-val IS_TCP_VAL is-udp-mask IS_UDP_MASK is-udp-val IS_UDP_VAL",  ctc_cli_parser_l4_set_app_data_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_show_ilm_cmd_vtysh,  "show mpls ilm SPACEID LABEL",  ctc_cli_mpls_show_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_wrapback_cmd_vtysh,  "hss12g wrapback hss-id HSS_ID link-num LINK_NUM (tx-wrap|rx-wrap-sync|rx-wrap-async) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_wrapback_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_mac_security_show_system_mac_limit_cmd_vtysh,  "show security mac-limit system",  ctc_cli_security_mac_security_show_system_mac_limit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_mac_vlan_class_cmd_vtysh,  "vlan classifier remove mac (macsa MAC|macda MAC)",  ctc_cli_vlan_remove_mac_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_queue_depth_cmd_vtysh,  "show queue queue-depth (lchip CHIP_ID|)",  ctc_cli_dbg_tool_queue_depth_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_cross_connect_cmd_vtysh,  "port GPHYPORT_ID port-cross-connect nhid NHID",  ctc_cli_port_set_cross_connect_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_remove_session_cmd_vtysh,  "mirror destination remove session SESSION_ID direction (ingress|egress|both) (port-session | vlan-session | acl-session (priority-id PRIORITY_ID|) | cpu-session)",  ctc_cli_mirror_remove_session_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_serdes_param_cmd_vtysh,  "chip set serdes SERDES ffe board-type (m4 |tr4) trace-len (4inch|7inch|10inch) (lchip LCHIP |)",  ctc_cli_chip_set_serdes_param_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_bist_cmd_vtysh,  "check bist (lpmtcam|intcam) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_check_bist_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_get_nhid_by_logic_port_cmd_vtysh,  "show l2 fdb logic-nhid by logic-port LOGIC_PORT_ID",  ctc_cli_l2_get_nhid_by_logic_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_efm_lb_mode_enable_cmd_vtysh,  "port GPHYPORT_ID efm-loop-back enable (redirect-to-cpu INDEX|)",  ctc_cli_port_efm_lb_mode_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_mac_cmd_vtysh,  "show stats ( mac-rx | mac-tx ) port GPHYPORT_ID (mode (plus | detail) |)",  ctc_cli_stats_show_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_show_fdb_cmd_vtysh,  "show l2 fdb entry by (mac MAC|mac-fid MAC FID|port GPORT_ID|logic-port LOGIC_PORT_ID |fid FID|port-fid GPORT_ID FID|all ) (static|) (dynamic|) (local-dynamic|) (all|) (entry-file FILE_NAME|)",  ctc_cli_l2_show_fdb_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto_cmd_vtysh,  "show pdu l2pdu classify l2hdr-proto index INDEX",  ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_show_policer_stats_cmd_vtysh,  "show qos policer stats (port GPORT (in|out) |flow POLICER_ID|service SEVICE_ID)         (hbwp-en cos-index INDEX |)",  ctc_cli_qos_show_policer_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_clear_cmd_vtysh,  "clear chip-agent (stats (driver-io|packet-io|) | cache)",  ctc_cli_gb_chip_agent_clear_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_reason_shp_base_pkt_cmd_vtysh,  "qos shape reason-shape-pkt (enable | disable)",  ctc_cli_qos_set_reason_shp_base_pkt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_TcamCtlInt_cmd_vtysh,  "show statistic (share) (tcamctlint) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_TcamCtlInt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_default_vlan_cmd_vtysh,  "port GPHYPORT_ID default vlan VLAN_ID",  ctc_cli_port_set_default_vlan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_global_cfg_cmd_vtysh,  "show stats ( saturate | hold | clear-after-read ) (igs-port|egs-port|igs-global-fwd|egs-global-fwd|fwd|gmac|sgmac|xgmac|cpumac)",  ctc_cli_stats_show_global_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_mac_security_set_vlan_learn_limit_cmd_vtysh,  "security mac-limit vlan VLAN_ID action (none|fwd|discard|redirect-to-cpu)",  ctc_cli_security_mac_security_set_vlan_learn_limit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_set_header_cmd_vtysh,  "stacking header {layer2 vlan-tpid TPID cos COS ether-type ETHTYPE (mac-da-check-en|) (ethtype-check-en|) |                       layer3 (ipv4 ip-sa A.B.C.D| ipv6 ip-sa X:X::X:X) ttl TTL dscp DSCP ip-protocol PROTOCOL|                       layer4 (udp src-port SRCPORT dest-port DESTPORT )}",  ctc_cli_stacking_set_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_sync_intf_toggle_time_cmd_vtysh,  "ptp CHIP sync-toggle (seconds SECOND) (nanoseconds NS) (nano-nanoseconds NNS)",  ctc_cli_ptp_set_sync_intf_toggle_time_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_destroy_group_cmd_vtysh,  "scl destroy group GROUP_ID",  ctc_cli_scl_destroy_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_create_or_destroy_l3_sub_if_cmd_vtysh,  "l3if (create | destroy) ifid IFID type sub-if gport GPORT_ID vlan VLAN_ID",  ctc_cli_l3if_create_or_destroy_l3_sub_if_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_remove_nh_member_cmd_vtysh,  "nexthop mcast NHID (add | remove) nh-member NH_ID (reflective |)",  ctc_cli_nh_add_remove_nh_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_debug_io_on_cmd_vtysh,  "debug chip-agent driver-io (socket|code|all)",  ctc_cli_gb_chip_agent_debug_io_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_QuadPcs_cmd_vtysh,  "show statistic (misc) (quadpcs) (index INDEX) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_QuadPcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_dump_mpls_tunnel_cmd_vtysh,  "show nexthop mpls-tunnel TUNNEL_ID (detail|)",  ctc_cli_gb_nh_dump_mpls_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_captured_ts_cmd_vtysh,  "show ptp CHIP captured-ts type (pkt-tx lport LPORT seq-id ID | (sync-pulse-rx | tod-1pps-rx) (seq-id ID)) ",  ctc_cli_ptp_show_captured_ts_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_Ds_TCAM_cmd_vtysh,  "show statistic (share) (ds&tcam) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_Ds_TCAM_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_mac_entry_cmd_vtysh,  "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|)" " mac-entry ({" " cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI |       svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI " " | mac-sa MAC MASK | mac-da MAC MASK |       layer2-type L2_TYPE MASK | layer3-type L3_TYPE MASK " " }|)" " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_mac_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_rspan_escape_mac_cmd_vtysh,  "mirror rspan escape {mac1 MAC (mask1 MASK |) | mac2 MAC (mask2 MASK |)}",  ctc_cli_mirror_rspan_escape_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_set_nhid_by_logic_port_cmd_vtysh,  "l2 fdb logic-nhid logic-port LOGIC_PORT_ID (gport GPORT_ID |nexthop NH_ID )",  ctc_cli_l2_set_nhid_by_logic_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_internal_port_cmd_vtysh,  "port set internal-port LOCAL_PORT gchip GCHIP type (iloop|eloop|discard|fwd port GPHYPORT_ID)",  ctc_cli_port_set_internal_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_show_ipv4_cmd_vtysh,  "show ipuc",  ctc_cli_gb_ipuc_show_ipv4_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_system_port_mac_prefix_cmd_vtysh,  "system set port-mac { prefix0 MAC | prefix1 MAC }",  ctc_cli_port_set_system_port_mac_prefix_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_cpu_access_static_ram_file_cmd_vtysh,  "static-ram-access file-name FILE repeat-time TIMES mode MODE op-code OP_CODE all-entry ALL_ENTRY (lchip CHIP_ID|)",  ctc_cli_dbg_tool_cpu_access_static_ram_file_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_show_member_port_cmd_vtysh,  "show linkagg AGG_ID member-ports",  ctc_cli_linkagg_show_member_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_update_action_cmd_vtysh,  "acl update entry ENTRY_ID " "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_update_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_create_statsid_cmd_vtysh,  "stats create statsid STATS_ID (vlan VLAN_ID | vrf VRF_ID | ipmc | mpls | scl | acl | tunnel | nexthop | l3if) (ingress|egress|)",  ctc_cli_stats_create_statsid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_set_global_action_by_macda_low24_cmd_vtysh,  "pdu l2pdu global-action macda-24bit index INDEX entry-valid ENTRY_VALID (bypass-all BYPASS_ALL|) action-index ACTION_INDEX",  ctc_cli_pdu_l2_set_global_action_by_macda_low24_cmd_help)

DEFSH (VTYSH_SDK, cli_oam_cfm_update_fs_cmd_vtysh,  "oam cfm update-mep local-chipid CHIP_ID mep-index MIN_MEP_INDEX MAX_MEP_INDEX",  cli_oam_cfm_update_fs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_debug_on_cmd_vtysh,  "debug aps (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_aps_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_mdio1g_clock_cmd_vtysh,  "mdio1g core-clk CORE_CLK mdio-clk MDIO_CLOCK (lchip CHIP_ID|)",  ctc_cli_dbg_tool_mdio1g_clock_cmd_help)

DEFSH (VTYSH_SDK, show_cli_linkread_cmd_vtysh,  "linkread tbl-name TBL_NAME tbl-index INDEX (lchip CHIP_ID|)",  show_cli_linkread_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_SharedDs_cmd_vtysh,  "show statistic (share) (sharedds) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_SharedDs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_calc_eye_height_cmd_vtysh,  "hss12g calc-eye-height hss-id HSS_ID link-num LINK_NUM (eye-file EYE_FILE|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_calc_eye_height_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_remove_lmep_cmd_vtysh,  "local-mep bfd remove my-discr MY_DISCR         (ip-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL)})",  ctc_cli_oam_bfd_remove_lmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_show_resource_usage_cmd_vtysh,  "show nexthop status",  ctc_cli_gb_nh_show_resource_usage_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_default_cfg_cmd_vtysh,  "port GPHYPORT_ID default-config",  ctc_cli_port_set_default_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_mac_switch_sys_cmd_vtysh,  "mac switch S_MAC_ID E_MAC_ID times TIMES mode MODE (net_tx_mode NET_TX_MODE|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_mac_switch_sys_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_port_oam_enable_cmd_vtysh,  "oam cfm port GPORT_ID (ingress| egress| both-direction) (enable| disable)",  ctc_cli_oam_cfm_set_port_oam_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_net_rx_packet_capture_cmd_vtysh,  "check net-rx-packet-capture lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_check_net_rx_packet_capture_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_bpe_show_mux_demux_cmd_vtysh,  "show bpe mux-demux port GPHYPORT_ID",  ctc_cli_bpe_show_mux_demux_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_ipv6_entry_cmd_vtysh,  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) " " ipv6-entry ({" " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK                          | cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI                    | svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI                    | layer2-type L2_TYPE MASK " "| layer3-type L3_TYPE MASK |eth-type ETH_TYPE MASK                         | stag-valid VALID | ctag-valid VALID                         " "| dscp DSCP MASK |prec IP_PREC MASK                                        | ip-frag IP_FRAG                          | ip-option IP_OPTION | routed-packet ROUTED_PACKET         " "|l4-protocol L4_PROTOCOL MASK (l4-src-port (use-mask|use-range) port0 PORT port1 PORT|)                         (l4-dst-port (use-mask|use-range) port0 PORT port1 PORT|)                         (icmp-type ICMP_TYPE MASK |) (icmp-code ICMP_CODE MASK|)                            (tcp-flags (match-any|match-all) FLAGS|)                  " "| ip-sa X:X::X:X  (host|X:X::X:X) | ip-da X:X::X:X (host|X:X::X:X) | ext-hdr EXT_HDR EXT_HDR_MASK | flow-label FLOW_LABEL MASK " " }|)" "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_add_ipv6_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_i2c_scan_para_cmd_vtysh,  "chip set i2c-scan para {(address ADDRESS offset OFFSET length LENGTH) | (bitmap BITMAP0) (interval INTERVAL)}",  ctc_cli_chip_set_i2c_scan_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_clear_tod_intf_rx_code_cmd_vtysh,  "ptp CHIP tod-rx-code-clear",  ctc_cli_ptp_clear_tod_intf_rx_code_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_add_or_remove_port_cmd_vtysh,  "linkagg AGG_ID (add | remove) member-port GPHYPORT_ID",  ctc_cli_linkagg_add_or_remove_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_policer_first_cmd_vtysh,  "qos policer (in | out | both) (flow-first | port-first)",  ctc_cli_qos_set_policer_first_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_show_vlan_mapping_default_entry_cmd_vtysh,  "show vlan mapping default info port GPORT_ID",  ctc_cli_gb_vlan_show_vlan_mapping_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dma_mem_dump_cmd_vtysh,  "dma mem dump addr ADDR len LEN",  ctc_cli_dma_mem_dump_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_remove_default_entry_cmd_vtysh,  "l2 fdb remove vlan-default-entry (fid FID) (port GPORT_ID |)",  ctc_cli_l2_remove_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_drain_on_off_file_cmd_vtysh,  "drain-onoff file-name FILE repeat-time TIMES mode MODE ({off-time OFF_MS | on-time ON_MS}|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_drain_on_off_file_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_group_shape_cmd_vtysh,  "qos shape group (service-id SERVICE|) (port GPHYPORT_ID) (none | pir PIR)",  ctc_cli_qos_set_group_shape_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_clock_drift_cmd_vtysh,  "show ptp CHIP drift",  ctc_cli_ptp_show_clock_drift_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_reg_cmd_vtysh,  "reg_write ADDR VALUE ({rand}|)",  ctc_cli_dbg_tool_write_reg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_qos_show_element_info_cmd_vtysh,  "show qos (port-policer (in|out) |flow-policer| cpu-reason | queue | group | channel) (range START END) (detail|)",  ctc_cli_gb_qos_show_element_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_show_ipv4_nat_sa_cmd_vtysh,  "show ipuc nat-sa",  ctc_cli_gb_ipuc_show_ipv4_nat_sa_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_set_lbr_use_lbm_da_cmd_vtysh,  "oam cfm lbr-sa-use-lbm-da (enable| disable)",  ctc_cli_oam_set_lbr_use_lbm_da_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_port_cmd_vtysh,  "show stats ( igs-port | egs-port ) port GPHYPORT_ID",  ctc_cli_stats_show_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_discard_same_mac_or_ip_cmd_vtysh,  "show chip (discard-same-mac|discard-same-ip)",  ctc_cli_chip_show_discard_same_mac_or_ip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_create_or_destroy_l3_vlan_if_cmd_vtysh,  "l3if (create | destroy) ifid IFID type vlan-if vlan VLAN_ID",  ctc_cli_l3if_create_or_destroy_l3_vlan_if_cmd_help)

DEFSH (VTYSH_SDK, show_cli_drv_read_reg_or_tbl_field_cmd_vtysh,  "read tbl-name TBL_NAME tbl-index INDEX (count COUNT (step STEP_COUNT |)|) (lchip CHIP_ID|)",  show_cli_drv_read_reg_or_tbl_field_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l2_tpid_cmd_vtysh,  "show parser l2 ethernet tpid (cvlan|itag|bvlan|svlan0|svlan1|svlan2|svlan3|cntag) ",  ctc_cli_parser_show_l2_tpid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_pdu_show_global_config_cmd_vtysh,  "show pdu (l2pdu|l3pdu) global-config",  ctc_cli_gb_pdu_show_global_config_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_debug_off_cmd_vtysh,  "no debug vlan (vlan|vlan-class|vlan-mapping|protocol-vlan) (ctc|sys)",  ctc_cli_vlan_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_tod_intf_cmd_vtysh,  "ptp CHIP tod (mode ( input | output)) (pulse-duty DUTY) (pulse-delay DELAY) (stop-bit NUMBER) (epoch EPOCH)",  ctc_cli_ptp_set_tod_intf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_add_ip_tunnel_cmd_vtysh,  "nexthop add ip-tunnel NHID (dsnh-offset OFFSET|) (unrov |fwd (""mac MAC ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" "      type (tunnel-v4 (ip6in4|ip4in4 (ip-bfd|)|ip6to4 (manual-ipsa|) | 6rd (sp-prefix-len SPPREFIXLEN |) (v4-sa-masklen SAMASKLEN|) (v4-da-masklen DAMASKLEN|) |isatap|grein4 protocol PROTOCOL (gre-key GREKEY|) (mirror|))     ({ipsa A.B.C.D|ipda A.B.C.D}|) ((copy-dont-frag|dont-frag DONT_FRAG)|) | tunnel-v6 (ip4in6 (ip-bfd|)|ip6in6|grein6 protocol PROTOCOL (gre-key GREKEY|) (mirror|))      ipsa X:X::X:X ipda X:X::X:X | ivi (6to4 | 4to6 v6-prefix-addr X:X::X:X) prefix-len LENGTH (prefix-obey-rfc6052|) |      nat new-ipda A.B.C.D (new-l4dstport L4PORT|))      (re-route|) (dscp DSCP dscp-select SEL_MODE ttl TTL (map-ttl|)|) (mtu-check-en mtu-size MTU |) (flow-label-mode MODE (flow-label-value VALUE | ) |)" "))",  ctc_cli_add_ip_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_seq_num_en_cmd_vtysh,  "local-mep seq-num (enable | disable) level LEVEL",  ctc_cli_oam_cfm_update_lmep_seq_num_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_mel_cmd_vtysh,  "local-mep level LEVEL",  ctc_cli_oam_cfm_update_lmep_mel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_write_xgphy_interface_cmd_vtysh,  "chip write xgphy port GPHYPORT_ID dev DEV_NO reg REG val VAL",  ctc_cli_chip_write_xgphy_interface_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_create_trunk_cmd_vtysh,  "stacking create trunk TRUNK_ID (encap {layer2 mac-da MACDA mac-sa MACSA (vlan-id VLAN|) | (ipv4 ip-da A.B.C.D| ipv6 ip-da X:X::X:X) } |)",  ctc_cli_stacking_create_trunk_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_ipg_index_cmd_vtysh,  "port GPHYPORT_ID ipg-index INDEX",  ctc_cli_port_set_ipg_index_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_link_intr_cmd_vtysh,  "port GPHYPORT_ID mac link-intr (enable|disble)",  ctc_cli_port_set_link_intr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_lm_cos_type_cmd_vtysh,  "local-mep lm-cos-type (all-cos | specified-cos | per-cos)",  ctc_cli_oam_cfm_update_lmep_lm_cos_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_monitor_sgmac_prbs_xfipcs_cmd_vtysh,  "monitor sgmac-prbs xfipcs lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_monitor_sgmac_prbs_xfipcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_hss12g_cmd_vtysh,  "read hss12g hss-id HSS_ID (link-tx|link-rx|pll) (link-num LINK_NUM|) addr ADDR_OFFSET (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_hss12g_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_PtpFrc_cmd_vtysh,  "show statistic (misc) (ptpfrc) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_PtpFrc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_efm_lb_mode_disable_cmd_vtysh,  "port GPHYPORT_ID efm-loop-back disable",  ctc_cli_port_efm_lb_mode_disable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_add_egress_vlan_mapping_cmd_vtysh,  "vlan mapping egress add port GPORT_ID ( { svid VLAN_ID (to-svid VLAN_ID| ) (stag-cos COS|) | cvid VLAN_ID (to-cvid VLAN_ID | ) (ctag-cos COS | )}  ( vrange-grp VRANGE_GROUP_ID | ) | ) " "mapping-to {new-cvid VLAN_ID|new-svid VLAN_ID|new-scos COS|new-ccos COS|stag-op TAG_OP  (sl-svid TAG_SL|) (sl-scos TAG_SL|) |" "ctag-op TAG_OP  (sl-cvid TAG_SL|) (sl-ccos TAG_SL|) } (aging|)",  ctc_cli_gb_vlan_add_egress_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_create_vlan_cmd_vtysh,  "vlan create vlan VLAN_ID",  ctc_cli_vlan_create_vlan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_pbr_ipv6_entry_cmd_vtysh,  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) " " pbr-ipv6-entry ({" " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK                          | cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI                    | svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI                    | layer2-type L2_TYPE MASK " "| layer3-type L3_TYPE MASK |eth-type ETH_TYPE MASK " "| dscp DSCP MASK |prec IP_PREC MASK                 | ip-frag IP_FRAG                         | vrfid VRFID MASK" "|l4-protocol L4_PROTOCOL MASK (l4-src-port (use-mask|use-range) port0 PORT port1 PORT|)                         (l4-dst-port (use-mask|use-range) port0 PORT port1 PORT|)                         (icmp-type ICMP_TYPE MASK |) (icmp-code ICMP_CODE MASK|)                            (tcp-flags (match-any|match-all) FLAGS|)                  " "| ip-sa X:X::X:X  (host|X:X::X:X) | ip-da X:X::X:X (host|X:X::X:X) | ext-hdr EXT_HDR EXT_HDR_MASK | flow-label FLOW_LABEL MASK " " }|)" "(permit|deny) ({forward NH_ID | ecmp | ttl-check | icmp-check | copy-to-cpu}|)",  ctc_cli_acl_add_pbr_ipv6_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_ipg_size_cmd_vtysh,  "show system ipg-index INDEX size",  ctc_cli_port_show_ipg_size_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_sync_intf_cmd_vtysh,  "show ptp CHIP sync",  ctc_cli_ptp_show_sync_intf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l3_set_global_action_cmd_vtysh,  "pdu l3pdu global-action (ospf|pim|vrrp|rsvp|rip|bgp|msdp|ldp|layer3-proto|layer4-port|layer4-type)     index INDEX entry-valid ENTRY_VALID action-index ACTION_INDEX",  ctc_cli_pdu_l3_set_global_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_reset_port_cmd_vtysh,  "clear stats ( igs-port | egs-port ) port GPHYPORT_ID",  ctc_cli_stats_reset_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_datapath_mode_cmd_vtysh,  "show chip datapath mode (chip LOCAL_CHIPID |)",  ctc_cli_chip_show_datapath_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_add_chan_cmd_vtysh,  "oam cfm lookup-chan add gport GPORT_ID direction (up |down ) (link-oam |) (vlan VLAN_ID (ccm-vlan VLAN_ID|)|) (master-gchip GCHIP_ID|)",  ctc_cli_oam_cfm_add_chan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_qos_show_cpu_reason_cmd_vtysh,  "show qos cpu-reason-id",  ctc_cli_gb_qos_show_cpu_reason_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_show_debug_cmd_vtysh,  "show debug chip-agent",  ctc_cli_gb_chip_agent_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l3_ip_frag_cmd_vtysh,  "show parser l3 ip small-frag-offset",  ctc_cli_parser_show_l3_ip_frag_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_oam_autogen_pkt_cmd_vtysh,  "oam autogen-pkt lport PORTID (pkt-size SIZE|) (band-width BAND-WIDTH|) (tx-mode TX_MODE|) (tx-pattern TX_PATTERN|) (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_oam_autogen_pkt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_show_mep_info_with_key_cmd_vtysh,  "show bfd info my-discr MY_DISCR         (ip-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL)})",  ctc_cli_oam_bfd_show_mep_info_with_key_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_set_service_queue_cmd_vtysh,  "nexthop service-queue NHID (enable|disable)",  ctc_cli_gb_nh_set_service_queue_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_debug_off_cmd_vtysh,  "no debug pdu (ctc | sys)",  ctc_cli_pdu_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_bpe_set_mux_demux_cmd_vtysh,  "bpe mux-demux port GPHYPORT_ID vlan-base VLAN_ID port-base PORT_ID extender-num NUM (enable|disable)",  ctc_cli_bpe_set_mux_demux_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_show_vlan_l3if_id_cmd_vtysh,  "show vlan VLAN_ID l3if-id",  ctc_cli_gb_vlan_show_vlan_l3if_id_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_HashDs_cmd_vtysh,  "show statistic (share) (hashds) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_HashDs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_dma_tbl_wr_cmd_vtysh,  "dma write table TBL_ADDR entry-len LEN entry-num NUM",  ctc_cli_gb_dma_tbl_wr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_remove_lmep_cmd_vtysh,  "local-mep remove (level LEVEL|)",  ctc_cli_oam_cfm_remove_lmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_get_l2uc_cmd_vtysh,  "show nexthop brguc port GPORT (bypass |)",  ctc_cli_nh_get_l2uc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_phy_scan_para_cmd_vtysh,  "chip set phy-scan para {(gebitmap0 GE_BITMAP0 gebitmap1 GE_BITMAP1) | (xgbitmap0 XG_BITMAP0 xgbitmap1 XG_BITMAP1) |           (interval INTERVAL) | (xglinkmask LINK_MASK) | (twince (enable | disable))}",  ctc_cli_chip_set_phy_scan_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_port_max_md_level_cmd_vtysh,  "oam cfm port GPORT_ID (ingress| egress| both-direction) (max-md-level MD_LEVEL)",  ctc_cli_oam_cfm_set_port_max_md_level_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_interrupt_set_en_cmd_vtysh,  "interrupt LCHIP type TYPE sub-type TYPE (enable|disable)",  ctc_cli_gb_interrupt_set_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_common_set_exception_cmd_vtysh,  "oam common exception-to-cpu ({ invalid-oam-pdu | rdi-defect | mac-status | high-ver-oam |     no-rmep | xcon-ccm | seq-num-error | reg-cfg-error | ccm-with-option-tlv | slow-oam | src-mac-mismatch |     aps-pdu-to-cpu | interval-mismatch | dm-to-cpu | lbr-to-cpu | lm-to-cpu | test-to-cpu | invalid-dm-dlm-to-cpu |     big-interval-to-cpu | tp-dlm-to-cpu | tp-dlmdm-to-cpu | csf-to-cpu | mip-no-process-pdu | mcc-to-cpu | lt-to-cpu |     lbm-mac-mep-id-fail | learning-bfd-to-cpu | error-bfd | bfd-timer-negotiation | scc-to-cpu | all } | )",  ctc_cli_oam_common_set_exception_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeCla_cmd_vtysh,  "show statistic (epe) (epecla) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeCla_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_emulation_dump_config_cmd_vtysh,  "emulation dump cfg SRC_FILE DST_FILE",  ctc_cli_dbg_tool_emulation_dump_config_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_sfp_cmd_vtysh,  "read sfp bit-map BIT_MAP address ADDR offset OFFSET (len LEN|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_sfp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_pcie_io_cmd_vtysh,  "write pcie-io address ADDR_OFFSET value VALUE",  ctc_cli_dbg_tool_write_pcie_io_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_queueid_mon_cmd_vtysh,  "show queue queueid-mon (lchip CHIP_ID|)",  ctc_cli_dbg_tool_queueid_mon_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_mapping_l3type_cmd_vtysh,  "parser l2 mapping-l3-type mapping index INDEX (l2-type-none | l2-type-eth-v2 | l2-type-eth-sap | l2-type-eth-snap|     l2-type-ppp-1b | l2-type-ppp-2b ) (l2-hdr-ptl L2_HDR_PTL mask MASK l3-type L3-TYPE (additional-offset OFFSET|))",  ctc_cli_parser_l2_mapping_l3type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_remove_ipv6_cmd_vtysh,  "ipuc ipv6 remove VRFID X:X::X:X MASK_LEN NHID ( ecmp | )",  ctc_cli_ipuc_remove_ipv6_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_l2_show_dflt_cmd_vtysh,  "show l2 vlan-default-entry fid FID",  ctc_cli_gb_l2_show_dflt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_set_mep_maid_len_cmd_vtysh,  "oam maid-len ( byte-16 | byte-32 | byte-48 )",  ctc_cli_gb_oam_set_mep_maid_len_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_stats_cmd_vtysh,  "show stats stats-id STATS_ID",  ctc_cli_stats_show_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_I2CMaster_cmd_vtysh,  "show statistic (misc) (i2cmaster) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_I2CMaster_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_mux_demux_cmd_vtysh,  "port GPHYPORT_ID (mux|demux) (enable|disable)",  ctc_cli_port_set_mux_demux_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_security_ip_src_guard_add_entry_cmd_vtysh,  "security ip-src-guard add ipv6 X:X::X:X mac MAC",  ctc_cli_gb_security_ip_src_guard_add_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_policer_configure_cmd_vtysh,  "qos policer (sequential | ipg | update |hbwp-share) (enable | disable)",  ctc_cli_qos_set_policer_configure_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_overlay_vlan_mapping_cmd_vtysh,  "vlan mapping overlay port GPORT_ID ( ({ svid VLAN_ID (to-svid VLAN_ID | ) |cvid VLAN_ID(to-cvid VLAN_ID | ) } ( vrange-grp VRANGE_GROUP_ID | )|) ({ stag-cos COS | ctag-cos COS }|) | ) " "mapping-to {new-cvid VLAN_ID|new-scos COS|new-ccos COS|service-id SERVICEID ((ingress|egress)|) (service-acl-en|) (service-policer-en|) (service-queue-en|)|new-svid VLAN_ID|logic-port GPHYPORT_ID (logic-port-type |)" "|aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path)|(fid FID (vpls-fid ({vpls-learning-dis | maclimit-en | is-leaf}|)|)|nexthop NHID (vpws-nh (is-leaf |)|vlan-switching|))" "|stag-op TAG_OP  (sl-svid TAG_SL|) (sl-scos TAG_SL|) |ctag-op TAG_OP  (sl-cvid TAG_SL |) (sl-ccos TAG_SL|) |user-vlanptr VLAN_PTR} (stats STATS_ID|) (aging|)",  ctc_cli_gb_vlan_overlay_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_mac_MacMux_cmd_vtysh,  "show statistic (mac) (macmux) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_mac_MacMux_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_copy_entry_cmd_vtysh,  "scl entry ENTRY_ID copy-to group GROUP_ID entry ENTRY_ID ",  ctc_cli_scl_copy_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_packet_debug_off_cmd_vtysh,  "no debug packet (ctc | sys)",  ctc_cli_packet_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_log_cmd_vtysh,  "show stats port GPHYPORT_ID ( ingress | egress ) log",  ctc_cli_stats_show_log_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_reset_in_sqn_cmd_vtysh,  "mpls ingress sequence-number SQN_INDEX",  ctc_cli_mpls_reset_in_sqn_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_add_pw_cmd_vtysh,  "mpls l2vpn-pw add (space SPACEID |) LABEL (vpws NHID (pwid PWID|)| vpls FID (pwid PWID|)         (vpls-port-type|) (igmp-snooping-en|) (vsi-learning-disable|) (mac-security-vsi-discard |)         (aps-select GROUP_ID (working-path|protection-path) |)) (service-aclqos-en | )         (encapsulation-mode (raw|tagged) |) (control-word|)(oam |) (qos-use-outer-info | )         (service-id SERVICE_ID |) (service-policer-en | ) (service-queue-en | ) (stats STATS_ID | )",  ctc_cli_mpls_add_pw_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_debug_simulator_off_cmd_vtysh,  "no debug chip-agent simulator packet-io",  ctc_cli_gb_chip_agent_debug_simulator_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_NetRx_cmd_vtysh,  "show statistic (ipe) (netrx) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_NetRx_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipv6_group_param_cmd_vtysh,  "show ipmc ipv6 group info group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (ip-based-l2mc|)",  ctc_cli_gb_ipmc_show_ipv6_group_param_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_set_ipv4_force_route_cmd_vtysh,  "ipmc force-route {ip-addr0 A.B.C.D MASK_LEN | ip-addr1 A.B.C.D MASK_LEN} (force-bridge|force-unicast)",  ctc_cli_ipmc_set_ipv4_force_route_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_update_mpls_pop_nexthop_cmd_vtysh,  "nexthop update mpls NHID pop (fwd-attr|unrov2fwd) (""payload-op op-route    ((pipe|short-pipe|uniform|)|) (sequence-number SQN_INDEX |) (mac MAC|) ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" ")",  ctc_cli_nh_update_mpls_pop_nexthop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipv4_group_param_cmd_vtysh,  "show ipmc group info group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (ip-based-l2mc|)",  ctc_cli_gb_ipmc_show_ipv4_group_param_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_reset_cpu_mac_cmd_vtysh,  "clear stats cpu-stats port GPHYPORT_ID",  ctc_cli_stats_reset_cpu_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_resrc_mgr_cmd_vtysh,  "show resr-mgr (ingress|egress) (port PORT) (cos COS)",  ctc_cli_dbg_tool_show_resrc_mgr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_monitor_inter_laken_prbs_cmd_vtysh,  "monitor inter-laken-prbs lane LANEID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_monitor_inter_laken_prbs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_drop_log_cmd_vtysh,  "stats drop-packet {log|flow} (enable | disable)",  ctc_cli_stats_drop_log_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_vlan_mapping_default_entry_cmd_vtysh,  "vlan mapping remove port GPORT_ID mismatch",  ctc_cli_vlan_remove_vlan_mapping_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_show_dma_desc_cmd_vtysh,  "show dma desc (stats | packet-rx chan CHAN | packet-tx | table-r | table-w)",  ctc_cli_show_dma_desc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_csf_en_cmd_vtysh,  "local-mep csf-tx (enable | disable) (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_csf_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_serdes_mode_cmd_vtysh,  "chip set serdes SERDES mode (xfi|sgmii|xsgmii|qsgmii|xaui) (lchip LCHIP |)",  ctc_cli_chip_set_serdes_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_capture_cmd_vtysh,  "check capture (lpmtcam|intcam) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_check_capture_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_add_remove_trunk_rchip_cmd_vtysh,  "stacking trunk TRUNK_ID (add|remove) remote-chip RCHIP",  ctc_cli_stacking_add_remove_trunk_rchip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_debug_on_cmd_vtysh,  "debug on",  ctc_cli_common_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_add_ipv4_cmd_vtysh,  "ipuc add VRFID A.B.C.D MASK_LEN  NHID (  ecmp | )   ({ttl-check | rpf-check | icmp-check | icmp-err-check | copy-to-cpu | protocol-entry | self-address |is-neighbor | is-connect l3if L3IFID } |) (l4-dst-port L4PORT (tcp-port|)|) (stats-id STATISID |)",  ctc_cli_ipuc_add_ipv4_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_remove_port_nhid_mapping_cmd_vtysh,  "learning port-nhid remove (port GPORT_ID | logic-port VPLS_PORT_ID) fid FID",  ctc_cli_learning_aging_remove_port_nhid_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_debug_off_cmd_vtysh,  "no debug port (ctc|sys)",  ctc_cli_port_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_port_mac_postfix_cmd_vtysh,  "port GPHYPORT_ID prefix-index INDEX low-8bit-mac MAC",  ctc_cli_port_set_port_mac_postfix_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_trpt_show_state_cmd_vtysh,  "show oam trpt state (gchip GCHIP|)",  ctc_cli_gb_oam_trpt_show_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_DsFib_cmd_vtysh,  "show statistic (share) (dsfib) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_DsFib_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_add_vmac_low_8bits_cmd_vtysh,  "l3if vmac ifid IFID add prefix-type PRE_TYPE low-8bits-mac-index MAC_INDEX low-8bits-mac LOW_8BIT_MAC",  ctc_cli_l3if_add_vmac_low_8bits_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_pdu_show_l2pdu_port_action_cmd_vtysh,  "show pdu l2pdu port-action port GPORT_ID",  ctc_cli_gb_pdu_show_l2pdu_port_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_sync_ether_debug_off_cmd_vtysh,  "no debug sync-ether (ctc|sys)",  ctc_cli_sync_ether_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_svlan_cos_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-svlan-cos-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) direction (ingress|egress) " " (svlan VLAN_ID) " " (stag-cos COS) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_svlan_cos_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_show_ver_cmd_vtysh,  "show version",  ctc_cli_common_show_ver_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_i2c_cmd_vtysh,  "write i2c-raw address ADDR value VALUE",  ctc_cli_dbg_tool_write_i2c_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_ipv4_l2mc_cmd_vtysh,  "vlan VLAN_ID ipv4-l2mc (enable |disable)",  ctc_cli_vlan_set_ipv4_l2mc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_destroy_vlan_vrange_cmd_vtysh,  "vlan range destroy vrange-grp GROUP_ID direction (ingress|egress) ",  ctc_cli_gb_vlan_destroy_vlan_vrange_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_install_entry_cmd_vtysh,  "scl install entry ENTRY_ID",  ctc_cli_scl_install_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_QMgrQueEntry_cmd_vtysh,  "show statistic (bsr) (qmgrqueentry) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_QMgrQueEntry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_debug_on_cmd_vtysh,  "debug security (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_security_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_stag_tpid_index_cmd_vtysh,  "port GPHYPORT_ID stag-tpid-index direction (ingress|egress|both) INDEX",  ctc_cli_port_set_stag_tpid_index_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_port_cfg_cmd_vtysh,  "stats ( igs-port-opt-type | egs-port-opt-type ) type (none|basic-port|mac-based-port|protocol-based-port|phb|per-port-phb)",  ctc_cli_stats_set_port_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l3_set_l3flex_cmd_vtysh,  "parser l3 flex ipda-basic-offset OFFSET min-len LEN l3-basic-offset OFFSET ptl-byte-select VAL",  ctc_cli_parser_l3_set_l3flex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_install_entry_cmd_vtysh,  "acl install entry ENTRY_ID",  ctc_cli_acl_install_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_remove_ipmc_member_cmd_vtysh,  "nexthop mcast NHID (add | remove) ip-member ""(sub-if gport GPORT_ID vlanId VLAN_ID | vlan-if vlanId VLAN_ID gport GPORT_ID ({vlan-port}|) | phy-if gport GPORT_ID )",  ctc_cli_nh_add_remove_ipmc_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_set_tpid_cmd_vtysh,  "parser l2 ethernet tpid (cvlan|itag|bvlan|svlan0|svlan1|svlan2|svlan3|cntag) TPID",  ctc_cli_parser_l2_set_tpid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_debug_on_cmd_vtysh,  "debug nexthop (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_nh_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_bridge_mac_cmd_vtysh,  "oam cfm set bridge-mac BRIDGE_MAC",  ctc_cli_oam_cfm_set_bridge_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_port_isolation_show_route_obey_cmd_vtysh,  "show security port-isolation route-obey",  ctc_cli_security_port_isolation_show_route_obey_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_create_group_cmd_vtysh,  "scl create group GROUP_ID (type        (port GPORT| global | port-class CLASS_ID lchip LCHIP | logic-port LOGIC_PORT)|)",  ctc_cli_scl_create_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_debug_off_cmd_vtysh,  "no debug ptp (ctc|sys)",  ctc_cli_ptp_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_mpls_entry_cmd_vtysh,  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) " " mpls-entry ({" " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK                          | cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI                    | svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI                    | layer2-type L2_TYPE MASK " "| routed-packet ROUTED_PACKET                    | mpls-label0 MPLS_LABEL MASK | mpls-label1 MPLS_LABEL MASK                | mpls-label2 MPLS_LABEL MASK | mpls-label3 MPLS_LABEL MASK " " }|)" "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_add_mpls_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_vlan_classify_cmd_vtysh,  "port GPHYPORT_ID vlan-classify (mac|ipv4|ipv6|protocol) (enable policy POLICY_ID |disable)",  ctc_cli_port_set_vlan_classify_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_mac_led_mode_cmd_vtysh,  "chip set mac-led para gport GPHYPORT_ID mode MODE { (polarity POLARITY) |         ((first-mode FISRT_MODE) (sec-mode SEC_MODE |))}",  ctc_cli_chip_set_mac_led_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_receive_cmd_vtysh,  "port GPHYPORT_ID receive (enable | disable)",  ctc_cli_port_set_receive_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_show_vlan_class_default_entry_cmd_vtysh,  "show vlan classifier default info",  ctc_cli_gb_vlan_show_vlan_class_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeHdrProc_cmd_vtysh,  "show statistic (epe) (epehdrproc) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeHdrProc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_misc_l2l3hdr_cmd_vtysh,  "nexthop add misc replace-l2-l3hdr NHID (dsnh-offset OFFSET|) (reflective |port GPORT_ID) " "(""(swap-mac | {replace-mac-da MAC | replace-mac-sa MAC})"" |) (vlan-edit ""cvlan-edit-type VALUE svlan-edit-type VALUE output-cvid VALUE         output-svid VALUE ((""((swap-vlan (swap-tpid|) (swap-cos|)) | (replace-svlan-cos VALUE ) | (map-svlan-cos ))"")|) (svlan-tpid-index TPID_INDEX|) (cvlan-valid|) (svlan-valid|) (user-vlanptr VLAN_PTR|) (is-leaf|)" "|)" "(replace-ipda A.B.C.D |) (replace-l4dest-port L4PORT|)",  ctc_cli_nh_add_misc_l2l3hdr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_discard_same_mac_or_ip_cmd_vtysh,  "chip (discard-same-mac|discard-same-ip) (enable|disable)",  ctc_cli_chip_set_discard_same_mac_or_ip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l4_app_cmd_vtysh,  "show parser l4 app",  ctc_cli_parser_show_l4_app_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_add_vlan_vrange_cmd_vtysh,  "vlan range add  MINVID to MAXVID vrange-grp GROUP_ID direction (ingress|egress)",  ctc_cli_gb_vlan_add_vlan_vrange_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_xlate_cmd_vtysh,  "nexthop remove egs-vlan-edit NHID",  ctc_cli_nh_remove_xlate_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_IntLkIntf_cmd_vtysh,  "show statistic (misc) (intlkintf) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_IntLkIntf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_scl_show_entry_info_cmd_vtysh,  "show scl entry-info     (all   | entry ENTRY_ID |group GROUP_ID |priority)     (type (tcam-mac|tcam-ipv4|tcam-ipv6|tcam-vlan|hash-port|hash-port-cvlan|hash-port-svlan|hash-port-2vlan|hash-port-cvlan-cos|     hash-port-svlan-cos|hash-mac|hash-port-mac|hash-ipv4|hash-port-ipv4|hash-ipv6|hash-l2|     hash-tunnel-ipv4|hash-tunnel-ipv4-gre|hash-tunnel-ipv4-rpf)|) (detail|)",  ctc_cli_gb_scl_show_entry_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_show_max_ecmp_cmd_vtysh,  "show nexthop max-ecmp",  ctc_cli_nh_show_max_ecmp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_mapping_show_port2mdio_cmd_vtysh,  "show mdio mapping",  ctc_cli_port_mapping_show_port2mdio_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_mapping_show_port2phy_cmd_vtysh,  "show phy mapping",  ctc_cli_port_mapping_show_port2phy_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_reset_out_sqn_cmd_vtysh,  "mpls egress sequence-number SQN_INDEX",  ctc_cli_mpls_reset_out_sqn_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_use_outer_ttl_cmd_vtysh,  "port GPHYPORT_ID use-outer-ttl (enable|disable)",  ctc_cli_port_set_use_outer_ttl_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_DynamicDs_cmd_vtysh,  "show statistic (share) (dynamicds) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_DynamicDs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_phy_cmd_vtysh,  "scan phy (enable INTERVAL|disable) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_phy_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_member_port_cmd_vtysh,  "vlan (add |del) port GPHYPORT_ID vlan VLAN_ID ",  ctc_cli_vlan_set_member_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_sensor_value_cmd_vtysh,  "show chip sensor (temp|voltage) (lchip LOCAL_CHIPID |)",  ctc_cli_chip_show_sensor_value_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_sfp_cmd_vtysh,  "write sfp slave-id SLAVE_ID address ADDR offset OFFSET value VALUE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_sfp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_debug_off_cmd_vtysh,  "no debug linkagg (ctc|sys)",  ctc_cli_linkagg_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_i2c_local_cmd_vtysh,  "write i2c-local address ADDR_OFFSET value VALUE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_i2c_local_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_rmep_sf_state_cmd_vtysh,  "remote-mep sf-state SF_STATE (level LEVEL|) (rmep-id MEP_ID)",  ctc_cli_oam_cfm_update_rmep_sf_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipv6_member_info_cmd_vtysh,  "show ipmc ipv6 member info group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (ip-based-l2mc|)",  ctc_cli_gb_ipmc_show_ipv6_member_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_adjust_delay_cmd_vtysh,  "ptp adjust-delay GPHYPORT_ID (ingress-latency VALUE|) (egress-latency VALUE|)                                        (ingress-asymmetry HIGH4_VALUE LOW32_VALUE (negative|)|)                                       (egress-asymmetry HIGH4_VALUE LOW32_VALUE (negative|)|)                                        (path-delay HIGH4_VALUE LOW32_VALUE|)",  ctc_cli_ptp_set_adjust_delay_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_debug_on_cmd_vtysh,  "debug stats (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_stats_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_run_inter_laken_prbs_cmd_vtysh,  "run inter-laken-prbs lane LANEID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_run_inter_laken_prbs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_flush_fdb_cmd_vtysh,  "l2 fdb flush by (mac MAC|port GPORT_ID|fid FID|port-fid GPORT_ID FID|all) (static|dynamic|local-dynamic|all) ",  ctc_cli_l2_flush_fdb_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_xgphy_para_cmd_vtysh,  "scan xgphy-para bit-map0 BIT_MAP0 bit-map1 BIT_MAP1 (link-mask LINK_MASK|) (twice-en|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_xgphy_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_destroy_group_cmd_vtysh,  "acl destroy group GROUP_ID",  ctc_cli_acl_destroy_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_ipv4_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " ipv4-hash-entry " " ip-sa A.B.C.D " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_ipv4_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_add_l3vpn_ilm_cmd_vtysh,  "mpls ilm add l3vpn-vc SPACEID LABEL NHID VRFID (uniform | short-pipe | pipe |) (qos-use-outer-info | ) (trust-outer-exp |)(ipv6 |) (oam |)",  ctc_cli_mpls_add_l3vpn_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_show_ipv6_cmd_vtysh,  "show ipuc ipv6",  ctc_cli_gb_ipuc_show_ipv6_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_count_cmd_vtysh,  "show ipmc count",  ctc_cli_gb_ipmc_show_count_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_debug_off_cmd_vtysh,  "no debug acl (ctc | sys)",  ctc_cli_acl_debug_off_cmd_help)

DEFSH (VTYSH_SDK, show_cli_drv_list_tbl_cmd_vtysh,  "listtbl TBL_NAME",  show_cli_drv_list_tbl_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_write_i2c_cmd_vtysh,  "chip write i2c dev-add ADDR slave-id SLAVE_ID offset OFFSET data DATA",  ctc_cli_chip_write_i2c_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_show_count_cmd_vtysh,  "show ipuc count",  ctc_cli_gb_ipuc_show_count_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_rspan_cmd_vtysh,  "nexthop add rspan NHID (dsnh-offset OFFSET|) (vlan VLAN_ID) (remote-chip|)",  ctc_cli_nh_add_rspan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_remove_egress_vlan_mapping_cmd_vtysh,  "vlan mapping egress remove port GPORT_ID ( { svid VLAN_ID (to-svid VLAN_ID| ) (stag-cos COS|) | cvid VLAN_ID (to-cvid VLAN_ID | ) (ctag-cos COS | )}  ( vrange-grp VRANGE_GROUP_ID | ) | )",  ctc_cli_gb_vlan_remove_egress_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_debug_on_cmd_vtysh,  "debug vlan (vlan|vlan-class|vlan-mapping|protocol-vlan)(ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_vlan_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_aps_xlate_cmd_vtysh,  "nexthop remove aps-egs-vlan-edit NHID",  ctc_cli_nh_remove_aps_xlate_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_update_ipv6_rpf_cmd_vtysh,  "ipmc ipv6 update rpf group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID {rpf-if0 IFID0|rpf-if1 IFID1|rpf-if2 IFID2|rpf-if3 IFID3}(more-rpf-to-cpu|)",  ctc_cli_ipmc_update_ipv6_rpf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_set_freq_cmd_vtysh,  "hss12g set hss-id HSS_ID link-num LINK_NUM frequency (1.25|5|10.3125) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_set_freq_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_clear_stats_cmd_vtysh,  "clear nexthop stats (nhid NHID|mpls-tunnel TUNNEL_ID) (protection-path|)",  ctc_cli_nh_clear_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_show_logic_port_mapping_srcport_cmd_vtysh,  "show learning virtual-port PORT ",  ctc_cli_learning_aging_show_logic_port_mapping_srcport_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_oam_autogen_pkt_disable_cmd_vtysh,  "oam autogen-pkt disable (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_oam_autogen_pkt_disable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_clear_stp_state_cmd_vtysh,  "clear stp state port GPHYPORT_ID",  ctc_cli_l2_clear_stp_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_debug_on_cmd_vtysh,  "debug pdu (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_pdu_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_opf_show_debug_cmd_vtysh,  "show debug (opf|fpa)",  ctc_cli_gb_ftm_opf_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_bsr_flow_chart_cmd_vtysh,  "show bus bsr diagram",  ctc_cli_dbg_tool_bsr_flow_chart_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_parallel_loopback_cmd_vtysh,  "hss12g parallel-loopback hss-id HSS_ID link-num LINK_NUM (enable|disable) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_parallel_loopback_cmd_help)

DEFSH (VTYSH_SDK, show_cli_drv_list_tbl_all_cmd_vtysh,  "listtbl",  show_cli_drv_list_tbl_all_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_add_default_cmd_vtysh,  "ipuc (ipv6 | ) default NHID",  ctc_cli_ipuc_add_default_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_tp_y1731_add_remove_maid_cmd_vtysh,  "oam tp-y1731 maid (add | remove) meg-id MEG_ID",  ctc_cli_oam_tp_y1731_add_remove_maid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_port_check_enable_cmd_vtysh,  "port GPHYPORT_ID port-check-enable (enable | disable)",  ctc_cli_port_set_port_check_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_remove_egress_vlan_mapping_default_entry_cmd_vtysh,  "vlan mapping egress remove port GPORT_ID mismatch",  ctc_cli_gb_vlan_remove_egress_vlan_mapping_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_vlan_filtering_cmd_vtysh,  "port GPHYPORT_ID vlan-filtering direction (ingress | egress | both)(enable | disable)",  ctc_cli_port_set_vlan_filtering_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeOam_cmd_vtysh,  "show statistic (epe) (epeoam) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeOam_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_mac_CpuMac_cmd_vtysh,  "show statistic (mac) (cpumac) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_mac_CpuMac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_sgmac_prbs_xauipcs_cmd_vtysh,  "check sgmac-prbs xauipcs lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_check_sgmac_prbs_xauipcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_drop_log_cmd_vtysh,  "show stats port GPHYPORT_ID drop-packet (log|flow)",  ctc_cli_stats_show_drop_log_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_run_capture_cmd_vtysh,  "run capture (lpmtcam|intcam) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_run_capture_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_blocking_cmd_vtysh,  "port GPHYPORT_ID port-blocking {ucast-flooding | mcast-flooding | known-mcast-flooding | bcast-flooding}     (enable|disable)",  ctc_cli_port_set_blocking_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_arrange_fragment_cmd_vtysh,  "ipuc arrange fragment",  ctc_cli_gb_ipuc_arrange_fragment_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_set_reflective_bridge_cmd_vtysh,  "nexthop mcast-reflective-bridge (enable|disable)",  ctc_cli_gb_nh_set_reflective_bridge_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_set_port_security_cmd_vtysh,  "security port-security port GPHYPORT_ID (enable|disable)",  ctc_cli_security_set_port_security_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_transmit_cmd_vtysh,  "vlan VLAN_ID transmit (enable | disable)",  ctc_cli_vlan_set_transmit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_install_group_cmd_vtysh,  "acl install group GROUP_ID (direction (ingress|egress|both) (lchip LCHIP|)      (port-bitmap PORT_BITMAP_HIGH PORT_BITMAP_LOW|      global| vlan-class CLASS_ID | port-class CLASS_ID |service-id SERVICE_ID|      pbr-class CLASS_ID)|)",  ctc_cli_acl_install_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_debug_off_cmd_vtysh,  "no debug stats (ctc | sys)",  ctc_cli_stats_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_bind_service_cmd_vtysh,  "qos service  (bind | unbind) (service-id SERVICE) (dest-port GPORT)",  ctc_cli_qos_bind_service_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_loopback_cmd_vtysh,  "hss12g loopback hss-id HSS_ID link-num LINK_NUM (enable|disable) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_loopback_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_dir_property_cmd_vtysh,  "port GPHYPORT_ID dir-property      ( acl-en | acl-port-classid | acl-port-classid-0 | acl-port-classid-1 | acl-ipv4-force-mac     | acl-ipv6-force-mac | acl-force-use-ipv6 | acl-use-class | service-acl-en | qos-domain | policer-valid     | stag-tpid-index | random-log-rate) direction (ingress|egress|both) value VALUE ",  ctc_cli_port_set_dir_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_sf_state_cmd_vtysh,  "local-mep sf-state SF_STATE (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_sf_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_port_lm_enable_cmd_vtysh,  "oam cfm lm port GPORT_ID (enable| disable)",  ctc_cli_oam_cfm_set_port_lm_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_i2c_cmd_vtysh,  "read i2c-raw address ADDR (len LEN|)",  ctc_cli_dbg_tool_read_i2c_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dma_debug_on_cmd_vtysh,  "debug dma (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_dma_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_port_set_scl_key_type_cmd_vtysh,  "port GPHYPORT_ID scl-key-type scl-id SCL_ID direction (ingress|egress) " "type (cvid|svid|cvid-ccos|svid-scos|port|dvid|mac-sa|mac-da|ipv4|ipv6|ipsg-port-mac|ipsg-port-ip|ipsg-ipv6|tunnel ( rpf | )| tunnel-v6 ( rpf | )|disable)",  ctc_cli_gb_port_set_scl_key_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_aps_mpls_pop_nexthop_cmd_vtysh,  "nexthop add aps-mpls NHID (dsnh-offset OFFSET|) (aps-bridge-id APS_BRIDGE_ID) pop (working-path ""payload-op op-route    ((pipe|short-pipe|uniform|)|) (sequence-number SQN_INDEX |) (mac MAC|) ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" "protection-path ""payload-op op-route    ((pipe|short-pipe|uniform|)|) (sequence-number SQN_INDEX |) (mac MAC|) ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" ")",  ctc_cli_nh_add_aps_mpls_pop_nexthop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_enter_app_mode_cmd_vtysh,  "enter app mode",  ctc_cli_common_enter_app_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_show_opf_type_info_cmd_vtysh,  "show opf type (TYPE|all)",  ctc_cli_gb_ftm_show_opf_type_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_clear_rmep_seq_num_fail_cmd_vtysh,  "remote-mep seq-num-fail-counter clear level LEVEL rmep-id MEP_ID",  ctc_cli_oam_cfm_clear_rmep_seq_num_fail_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_IpeIntfMap_cmd_vtysh,  "show statistic (ipe) (ipeintfmap) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_IpeIntfMap_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_clear_rmep_mac_status_fail_cmd_vtysh,  "remote-mep mac-status clear level LEVEL rmep-id MEP_ID",  ctc_cli_oam_cfm_clear_rmep_mac_status_fail_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_pf_interval_cmd_vtysh,  "bfd pf interval INTERVAL",  ctc_cli_gb_oam_pf_interval_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_clear_lmep_rdi_cmd_vtysh,  "local-mep rdi clear (level LEVEL|)",  ctc_cli_oam_cfm_clear_lmep_rdi_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_show_vlan_info_cmd_vtysh,  "show vlan VLAN_ID (gchip GCHIP_ID|) {(member-port | receive | transmit | bridge | mac-learning | vrfid | stpid| igmp-snooping     | arp | dhcp | tagged-port |ipv4-l2mc |ipv6-l2mc | ptp |drop-unknown-ucast | drop-unknown-mcast)}",  ctc_cli_gb_vlan_show_vlan_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_debug_io_off_cmd_vtysh,  "no debug chip-agent driver-io (socket|code|all)",  ctc_cli_gb_chip_agent_debug_io_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_vlan_enable_cmd_vtysh,  "oam cfm vlan VLAN_ID (ingress| egress| both-direction) (enable| disable)",  ctc_cli_oam_cfm_set_vlan_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l2_l2flex_cmd_vtysh,  "show parser l2 flex",  ctc_cli_parser_show_l2_l2flex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_ipv4_vlan_class_cmd_vtysh,  "vlan classifier remove ipv4 {ip-sa A.B.C.D (mask A.B.C.D|)|ip-da A.B.C.D (mask A.B.C.D|)|mac-sa MAC|mac-da MAC|l3-type (arp|rarp|ipv4|L3TYPE)|l4-type (udp|tcp|L4TYPE)     |l4-srcport L4PORT |l4-destport L4PORT}",  ctc_cli_vlan_remove_ipv4_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_reflective_bridge_cmd_vtysh,  "port GPHYPORT_ID reflective-bridge (enable|disable)",  ctc_cli_port_set_reflective_bridge_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_transmit_cmd_vtysh,  "port GPHYPORT_ID transmit (enable | disable)",  ctc_cli_port_set_transmit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_mac_entry_cmd_vtysh,  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) " " mac-entry ({ " " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK                          | cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI                    | svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI                    | layer2-type L2_TYPE MASK " "| layer3-type L3_TYPE MASK |eth-type ETH_TYPE MASK                         | stag-valid VALID | ctag-valid VALID                         " "| arp-op-code ARP_OP_CODE MASK                                             | ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D               " " }|)" "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_add_mac_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_add_ipv6_cmd_vtysh,  "ipuc ipv6 add VRFID X:X::X:X MASK_LEN NHID ( ecmp  |) ({ttl-check | rpf-check | icmp-check | icmp-err-check | copy-to-cpu | protocol-entry | self-address | is-neighbor | is-connect l3if L3IFID } |)(stats-id STATISID |)",  ctc_cli_ipuc_add_ipv6_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_mpls_show_ilm__cmd_vtysh,  "show mpls ilm",  ctc_cli_gb_mpls_show_ilm__cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_set_global_action_by_layer3_type_cmd_vtysh,  "pdu l2pdu global-action l3type index INDEX (entry-valid ENTRY_VALID bypass-all BYPASS_ALL|) action-index ACTION_INDEX copy-to-cpu COPY_TO_CPU",  ctc_cli_pdu_l2_set_global_action_by_layer3_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_common_set_defect_to_cpu_cmd_vtysh,  "oam common defect-to-cpu ({ rx-first-pdu | mac-status-change | src-mac-mismatch | rx-rdi  | maid-mismatch |     low-ccm | no-rmep  | interval-mismatch  |     dloc | dcsf | bfd-init | bfd-up | bfd-down | all } | )",  ctc_cli_oam_common_set_defect_to_cpu_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l3_mapping_l4type_cmd_vtysh,  "parser l3 mapping-l4-type mapping index INDEX hdr-ptl HDR_PTL hdr-mask HDR_MASK l3-type L3-TYPE type-mask TYPE_MASK     l4-type L4-TYPE (additional-offset OFFSET|)",  ctc_cli_parser_l3_mapping_l4type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_show_vlan_profile_cmd_vtysh,  "show vprofile all",  ctc_cli_gb_vlan_show_vlan_profile_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_acl_show_status_cmd_vtysh,  "show acl status",  ctc_cli_gb_acl_show_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_set_global_property_cmd_vtysh,  "ipuc global-config {ipv4-martian-check (enable | disable) | ipv6-martian-check (enable | disable) | mcast-match-check (enable | disable) | ipv4-sa-search (enable | disable) | ipv6-sa-search (enable | disable) | ip-ttl-threshold THRESHOLD }",  ctc_cli_ipuc_set_global_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeHdrAdj_cmd_vtysh,  "show statistic (epe) (epehdradj) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeHdrAdj_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_syncup_internal_cmd_vtysh,  "stats set syncup internal TIME",  ctc_cli_stats_set_syncup_internal_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_port_set_scl_property_cmd_vtysh,  "port GPHYPORT_ID scl-property scl-id SCL_ID direction (ingress|egress) " "hash-type (port-2vlan|port-svlan|port-cvlan|port-svlan-cos|port-cvlan-cos|mac-sa|mac-da|port-mac-sa|port-mac-da|" "ip-sa|port-ip-sa|port|l2|disable) (tcam-type (mac|ip|vlan|disable)|)" "(class-id CLASS_ID|)",  ctc_cli_gb_port_set_scl_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l4_flex_cmd_vtysh,  "show parser l4 flex",  ctc_cli_parser_show_l4_flex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_add_remove_maid_cmd_vtysh,  "oam cfm maid (add|remove) (meg-id MEG_ID | md-name MD_NAME ma-name NA_NAME) (master-gchip GCHIP_ID|)",  ctc_cli_oam_cfm_add_remove_maid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_pbr_ipv4_entry_cmd_vtysh,  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) " " pbr-ipv4-entry ({ " "ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D                | dscp DSCP MASK |prec IP_PREC MASK                                        | ip-frag IP_FRAG                          | vrfid VRFID MASK" "|l4-protocol L4_PROTOCOL MASK (l4-src-port (use-mask|use-range) port0 PORT port1 PORT|)                         (l4-dst-port (use-mask|use-range) port0 PORT port1 PORT|)                         (icmp-type ICMP_TYPE MASK |) (icmp-code ICMP_CODE MASK|)                            (tcp-flags (match-any|match-all) FLAGS|)                  " " }|)" "(permit|deny) ({forward NH_ID | ecmp | ttl-check | icmp-check | copy-to-cpu}|)",  ctc_cli_acl_add_pbr_ipv4_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_trpt_set_session_cmd_vtysh,  "oam trpt set session SESSION dest-port GPORT tx-mode MODE (pkt-num NUMBER |)  pattern-type         {(repeat VALUE | random | increment-byte | decrement-byte | increment-word | decrement-word )}          rate RATE pkt-size SIZE (seqnum-en OFFSET |)  (lbm | tst | pkt-file FILE_NAME)",  ctc_cli_gb_oam_trpt_set_session_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_remove_cmd_vtysh,  "linkagg remove linkagg AGG_ID",  ctc_cli_linkagg_remove_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_i2c_local_cmd_vtysh,  "read i2c-local address ADDR_OFFSET (len LEN|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_i2c_local_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_run_sgmac_prbs_xfipcs_cmd_vtysh,  "run sgmac-prbs xfipcs lport PORTID prbs-pattern PRBS_PATTERN (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_run_sgmac_prbs_xfipcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_update_action_cmd_vtysh,  "scl update entry ENTRY_ID " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_update_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_clock_drift_cmd_vtysh,  "ptp CHIP drift (negative | ) (nanoseconds NS)",  ctc_cli_ptp_set_clock_drift_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_pcie_cfg_cmd_vtysh,  "read pcie-cfg address ADDR_OFFSET (LEN|)",  ctc_cli_dbg_tool_read_pcie_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_add_protocol_vlan_cmd_vtysh,  "vlan classifier add protocol ether-type ETHER_TYPE vlan VLAN_ID ((cos COS)|)",  ctc_cli_gb_vlan_add_protocol_vlan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_dma_tbl_rd_cmd_vtysh,  "dma read table TBL_ADDR entry-len LEN entry-num NUM",  ctc_cli_gb_dma_tbl_rd_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_debug_on_cmd_vtysh,  "debug ipmc (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_ipmc_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bus_cmd_vtysh,  "show bus ((ipe learning2fwd) |(ipe pkt-process2fwd) |(ipe hdr-adjust2inf-mapper) |(ipe parser2inf-mapper) |(ipe inf-mapper2lkp-mgr) |(ipe parser2lkp-mgr) |(ipe pkt-process-ack2oam) |(ipe inf-mapper2oam) |(ipe lkp-mgr2oam) |(ipe inf-mapper2pkt-process) |(ipe scl2pkt-process) |(ipe lkp-mgr2pkt-process) |(bsr bufretrieve2pkt-buffer-track) |(bsr ipe-hdr2metfifo) |(bsr bufstore2metfifo) |(bsr bufretrieve2metfifo) |(bsr qmgr2metfifo) |(bsr enqueue-msg) |(bsr dequeue-msg2enqueue-msg) |(epe hdr-process2acl-qos) |(epe tcam-ack2acl) |(epe classfiy2policer) |(epe bufretrieve2hdr-adjust) |(epe hdr-process2hdr-edit) |(epe oam2hdr-edit) |(epe hdr-adjust2hdr-process) |(epe nh-mapper2hdr-process) |(epe payload-process2layer3-edit) |(epe hdr-adjust2nh-mapper) |(epe parser2nh-mapper) |(epe hdr-process2oam) |(epe classfiy2oam) |(fib lkpmgr-lkp-key-input) |(fib pkt-process-lkp-key-output) |(fib fib-ad-output) |(fib track-fifo) |(lpm lkp-mgr-key-input) |(lpm conflict-tcam-key-input) |(lpm hash-key-output) |(lpm conflict-tcam-key-output) |(lpm pipeline-request-fifo) |(lpm ad-output) |(lpm track) |(tcam fifo) |(oam hdr-edit) | (oam rx-proc)) index INDEX (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bus_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_remove_entry_cmd_vtysh,  "scl remove entry ENTRY_ID",  ctc_cli_scl_remove_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_acl_show_tcp_flags_cmd_vtysh,  "show acl (tcp-flags|port-range)",  ctc_cli_gb_acl_show_tcp_flags_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_show_fdb_count_cmd_vtysh,  "show l2 fdb count by (mac MAC|port GPORT_ID |logic-port LOGIC_PORT_ID |fid FID|port-fid GPORT_ID FID |all ) (static|) (dynamic|) (local-dynamic|) (all|)",  ctc_cli_l2_show_fdb_count_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_vlan_entry_cmd_vtysh,  "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|)" " vlan-entry ({" " cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI |       svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI " "| stag-valid STAG_VALID | ctag-valid CTAG_VALID " " }|)" " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_vlan_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_clear_port_queue_stats_cmd_vtysh,  "clear qos stats" "((service-id SERVICE|) (port GPHYPORT_ID) (queue-id QUEUE_ID) | reason-id REASON)",  ctc_cli_qos_clear_port_queue_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_aps_xlate_cmd_vtysh,  "nexthop add aps-egs-vlan-edit NHID (dsnh-offset OFFSET|) (aps-bridge-id APS_BRIDGE_ID) " "(working-path ""cvlan-edit-type VALUE svlan-edit-type VALUE output-cvid VALUE         output-svid VALUE ((""((swap-vlan (swap-tpid|) (swap-cos|)) | (replace-svlan-cos VALUE ) | (map-svlan-cos ))"")|) (svlan-tpid-index TPID_INDEX|) (cvlan-valid|) (svlan-valid|) (user-vlanptr VLAN_PTR|) (is-leaf|)" " )" "((protection-path ""cvlan-edit-type VALUE svlan-edit-type VALUE output-cvid VALUE         output-svid VALUE ((""((swap-vlan (swap-tpid|) (swap-cos|)) | (replace-svlan-cos VALUE ) | (map-svlan-cos ))"")|) (svlan-tpid-index TPID_INDEX|) (cvlan-valid|) (svlan-valid|) (user-vlanptr VLAN_PTR|) (is-leaf|)" ")|)",  ctc_cli_nh_add_aps_xlate_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l3_trill_header_cmd_vtysh,  "show parser l3 trill header inner-tpid rbridge-ethertype",  ctc_cli_parser_show_l3_trill_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_show_select_cmd_vtysh,  "show aps select group GROUP_ID",  ctc_cli_aps_show_select_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_show_debug_cmd_vtysh,  "show debug mirror (ctc | sys)",  ctc_cli_mirror_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_show_ftm_info_cmd_vtysh,  "show ftm info",  ctc_cli_gb_ftm_show_ftm_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_ipuc_cmd_vtysh,  "nexthop remove ipuc NHID",  ctc_cli_nh_remove_ipuc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_cfm_dump_ma_cmd_vtysh,  "oam cfm dump ma local-chipid CHIP_ID ma-index MA_INDEX",  ctc_cli_gb_oam_cfm_dump_ma_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_rmep_enable_cmd_vtysh,  "remote-mep (enable| disable) level LEVEL rmep-id MEP_ID",  ctc_cli_oam_cfm_rmep_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_random_threshold_cmd_vtysh,  "port GPHYPORT_ID random-threshold direction (ingress|egress|both) THRESHOLD",  ctc_cli_port_set_random_threshold_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2mcast_add_member_cmd_vtysh,  "l2 mcast add member mac MAC fid FID (port GPORT_ID|nexthop NH_ID) (remote-chip|) (service-queue|)",  ctc_cli_l2mcast_add_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_ipv4_entry_cmd_vtysh,  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) " " ipv4-entry ({ " " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK                          | cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI                    | svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI                    | layer2-type L2_TYPE MASK " "| layer3-type L3_TYPE MASK |eth-type ETH_TYPE MASK                         | stag-valid VALID | ctag-valid VALID                         " "| dscp DSCP MASK |prec IP_PREC MASK                                        | ip-frag IP_FRAG                          | ip-option IP_OPTION | routed-packet ROUTED_PACKET         " "|l4-protocol L4_PROTOCOL MASK (l4-src-port (use-mask|use-range) port0 PORT port1 PORT|)                         (l4-dst-port (use-mask|use-range) port0 PORT port1 PORT|)                         (icmp-type ICMP_TYPE MASK |) (icmp-code ICMP_CODE MASK|)                            (tcp-flags (match-any|match-all) FLAGS|)                  " "(igmp-type IGMP_TYPE  MASK|)" "| ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D | arp-packet ARP_PACKET (arp-op-code ARP_OP_CODE MASK|) (sender-ip A.B.C.D A.B.C.D|) (target-ip A.B.C.D A.B.C.D|) | ipv4-packet IPV4_PACKET  " " }|)" "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_add_ipv4_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_discard_cmd_vtysh,  "show discard (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_discard_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_sync_intf_rx_code_cmd_vtysh,  "show ptp CHIP sync-rx-code",  ctc_cli_ptp_show_sync_intf_rx_code_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_debug_on_cmd_vtysh,  "debug linkagg (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_linkagg_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_mac_QuadMac_cmd_vtysh,  "show statistic (mac) (quadmac) (index INDEX) (sub-index SUB_INDEX|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_mac_QuadMac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_destroy_statsid_cmd_vtysh,  "stats destroy statsid STATS_ID",  ctc_cli_stats_destroy_statsid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_cfm_dump_mep_index_cmd_vtysh,  "oam cfm dump mep index local-chipid CHIP_ID",  ctc_cli_gb_oam_cfm_dump_mep_index_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_mac_security_show_port_security_cmd_vtysh,  "show security port-security port GPHYPORT_ID",  ctc_cli_security_mac_security_show_port_security_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_serdes_mode_cmd_vtysh,  "show chip serdes SERDES_ID mode",  ctc_cli_chip_show_serdes_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_set_learning_action_cmd_vtysh,  "learning action (always-cpu|cache-full-to-cpu threshold THRESHOLD | cache-only  threshold THRESHOLD |dont-learn |(mac-table-full (enable|disable)))",  ctc_cli_learning_aging_set_learning_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_debug_on_cmd_vtysh,  "debug ptp (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_ptp_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_untag_dft_vid_enable_cmd_vtysh,  "port GPHYPORT_ID untag-default-vlan enable (untag-svlan | untag-cvlan)",  ctc_cli_port_set_untag_dft_vid_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_cfg_max_ecmp_cmd_vtysh,  "nexthop max-ecmp MAX_ECMP",  ctc_cli_nh_cfg_max_ecmp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_uninstall_group_cmd_vtysh,  "acl uninstall group GROUP_ID",  ctc_cli_acl_uninstall_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_mac_security_set_system_mac_limit_cmd_vtysh,  "security mac-limit system action (none|fwd|discard|redirect-to-cpu)",  ctc_cli_security_mac_security_set_system_mac_limit_cmd_help)

DEFSH (VTYSH_SDK, humber_cli_aps_set_bridge_cmd_vtysh,  "aps set aps-bridge group GROUP_ID protect (enable|disable)",  humber_cli_aps_set_bridge_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_mac_cfg_cmd_vtysh,  "show stats ( mtu1-pkt-length | mtu2-pkt-length | dot1q-subtract-byte ) port GPHYPORT_ID",  ctc_cli_stats_show_mac_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_get_multi_entry_cmd_vtysh,  "show scl group GROUP_ID size ENTRY_SIZE ",  ctc_cli_scl_get_multi_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_debug_show_cmd_vtysh,  "show debug scl",  ctc_cli_scl_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_set_default_entry_feature_cmd_vtysh,  "l2 fdb vlan-default-entry (fid FID) (port GPORT_ID |) ({use-logic-port|unknown-ucast-drop|unknown-mcast-drop|proto-to-cpu|learn-dis} | default|)",  ctc_cli_l2_set_default_entry_feature_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_pcie_mem_cmd_vtysh,  "write pcie-mem address ADDR_OFFSET value VALUE",  ctc_cli_dbg_tool_write_pcie_mem_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_all_vlan_class_cmd_vtysh,  "vlan classifier flush-all (mac-vlan|ipv4-vlan|ipv6-vlan|protocol-vlan)",  ctc_cli_vlan_remove_all_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_remove_ipv6_tunnel_cmd_vtysh,  "ipuc ipv6 tunnel remove X:X::X:X ({ vrf VRFID | tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY  | ip-sa X:X::X:X } | )",  ctc_cli_ipuc_remove_ipv6_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_add_egress_vlan_mapping_default_entry_cmd_vtysh,  "vlan mapping egress add port GPORT_ID mismatch (discard|do-nothing)",  ctc_cli_gb_vlan_add_egress_vlan_mapping_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_packet_capture_cmd_vtysh,  "check packet-capture lport PORTID (pkt-len PKT_LEN|) (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_check_packet_capture_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_l2_show_master_cmd_vtysh,  "show l2 fdb-master",  ctc_cli_gb_l2_show_master_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_svlan_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-svlan-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) direction (ingress|egress) " " (svlan VLAN_ID) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_svlan_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_remove_ip_tunnel_cmd_vtysh,  "nexthop remove ip-tunnel NHID",  ctc_cli_remove_ip_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_debug_cmd_vtysh,  "show debug pdu (ctc | sys)",  ctc_cli_pdu_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_vlan_domain_cmd_vtysh,  "port GPHYPORT_ID vlan-domain (cvlan | svlan)",  ctc_cli_port_set_vlan_domain_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_drop_unknown_pkt_cmd_vtysh,  "vlan VLAN_ID (drop-unknown-ucast|drop-unknown-mcast) (enable|disable)",  ctc_cli_vlan_set_drop_unknown_pkt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_tod_intf_tx_code_cmd_vtysh,  "show ptp CHIP tod-tx-code",  ctc_cli_ptp_show_tod_intf_tx_code_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_set_mep_index_by_sw_cmd_vtysh,  "oam mep-index-alloc system",  ctc_cli_gb_oam_set_mep_index_by_sw_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l3_set_trill_header_cmd_vtysh,  "parser l3 trill header inner-tpid TPID rbridge-ethertype TPYE",  ctc_cli_parser_l3_set_trill_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_threshold_cfg_cmd_vtysh,  "stats ( packet-threshold | byte-threshold | fifo-depth-threshold ) THRESHOLD",  ctc_cli_stats_set_threshold_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_port_show_port_info_cmd_vtysh,  "show port GPHYPORT_ID ((scl-key-type scl-id SCL_ID|vlan-range) direction (ingress|egress)  )",  ctc_cli_gb_port_show_port_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_dvlan_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-double-vlan-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) direction (ingress|egress) " " (svlan VLAN_ID) " " (cvlan VLAN_ID) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_dvlan_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_storm_ctl_set_global_cfg_cmd_vtysh,  "security storm-ctl ipg (enable|disable) ucast-ctl-mode (enable|disable)  mcast-ctl-mode (enable|disable) (granu VALUE | )",  ctc_cli_security_storm_ctl_set_global_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_ipe_flow_chart_cmd_vtysh,  "show bus ipe diagram",  ctc_cli_dbg_tool_ipe_flow_chart_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_remove_ipv4_tunnel_cmd_vtysh,  "ipuc tunnel remove A.B.C.D ({ vrf VRFID | tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY  | ip-sa A.B.C.D | use-flex} | ) ",  ctc_cli_ipuc_remove_ipv4_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeAclQos_cmd_vtysh,  "show statistic (epe) (epeaclqos) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeAclQos_cmd_help)

DEFSH (VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK, ctc_cli_common_fast_enter_internal_debug_mode_cmd_vtysh,  "internal",  ctc_cli_common_fast_enter_internal_debug_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_gpio_mode_cmd_vtysh,  "chip set gpio gpio-id GPIO_ID mode MODE",  ctc_cli_chip_set_gpio_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_help_cmd_vtysh,  "help",  ctc_cli_common_help_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_tp_remove_chan_cmd_vtysh,  "oam (tp-y1731) lookup-chan remove (section-oam (gport GPORT_ID |ifid IFID) | mpls-label LABEL) (master-gchip GCHIP_ID|)",  ctc_cli_oam_tp_remove_chan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_oam_OamAutoGenPkt_cmd_vtysh,  "show statistic (oam) (oamautogenpkt) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_oam_OamAutoGenPkt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_show_group_info_cmd_vtysh,  "show scl group-info group GROUP_ID",  ctc_cli_scl_show_group_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_port_shape_cmd_vtysh,  "qos shape port GPHYPORT_ID (none | pir PIR (pbs PBS|)) ",  ctc_cli_qos_set_port_shape_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_iloop_cmd_vtysh,  "nexthop add iloop NHID lpbk-lport LPORT_ID (customerid-valid (map-exp|)|) (inner-pkt-type TYPE |) (vpls-port|) (sequence-number SQN_INDEX |) (words-remove VALUE|) (service-queue-en |)",  ctc_cli_nh_add_iloop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_add_ipv4_member_cmd_vtysh,  "ipmc add member group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID ((""(sub-if gport GPORT_ID vlanid VLAN_ID | vlan-if vlanid VLAN_ID gport GPORT_ID ({vlan-port}|) | phy-if gport GPORT_ID )" "  | (ip-based-l2mc gport GPORT_ID)) (remote-chip|) | ""nexthop-id NHID" ")",  ctc_cli_ipmc_add_ipv4_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_adjust_delay_cmd_vtysh,  "show ptp adjust-delay GPHYPORT_ID delay-type ( ingress-latency | egress-latency | ingress-asymmetry | egress-asymmetry | path-delay)",  ctc_cli_ptp_show_adjust_delay_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_nhop_cmd_vtysh,  "local-mep nhop NHOP",  ctc_cli_oam_cfm_update_lmep_nhop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_port_isolation_set_route_obey_cmd_vtysh,  "security port-isolation route-obey (enable|disable)",  ctc_cli_security_port_isolation_set_route_obey_cmd_help)

DEFSH (VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK, ctc_cli_common_fast_enter_app_mode_cmd_vtysh,  "app",  ctc_cli_common_fast_enter_app_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_interrupt_clear_status_cmd_vtysh,  "interrupt LCHIP type TYPE sub-type TYPE clear",  ctc_cli_gb_interrupt_clear_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_add_session_cmd_vtysh,  "mirror destination add session SESSION_ID direction (ingress|egress|both) (port-session | vlan-session | acl-session (priority-id PRIORITY_ID|) | cpu-session)  (port GPORT_ID) ",  ctc_cli_mirror_add_session_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_debug_show_cmd_vtysh,  "show debug acl",  ctc_cli_acl_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_set_vrf_stats_en_cmd_vtysh,  "l3if stats vrfid VRFID (enable|disable)",  ctc_cli_l3if_set_vrf_stats_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_chip_info_cmd_vtysh,  "show chip info",  ctc_cli_chip_show_chip_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_queue_wdrr_weight_cmd_vtysh,  "qos schedule queue-wdrr (service-id SERVICE |) (port GPHYPORT_ID) (confirm-class| exceed-class)     (wdrr-weight WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT |queue-id QUEUE_ID weight WEIGHT)",  ctc_cli_qos_set_queue_wdrr_weight_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_show_port_mirror_state_cmd_vtysh,  "show mirror port GPHYPORT_ID direction (ingress|egress) ",  ctc_cli_mirror_show_port_mirror_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_interrupt_cmd_vtysh,  "show interrupt (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_interrupt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_ecmp_cmd_vtysh,  "nexthop add ecmp ECMPID NHID0 NHID1 (NHID2 (NHID3 (NHID4 (NHID5 (NHID6 (NHID7 |) |) |) |) |) |)",  ctc_cli_nh_add_ecmp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_BufStore_cmd_vtysh,  "show statistic (bsr) (bufstore) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_BufStore_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_l2_show_fdb_hw_cmd_vtysh,  "show l2 fdb hw (entry|detail) by (mac MAC|mac-fid MAC FID|port GPORT_ID|fid FID|port-fid GPORT_ID FID|all ) (static|) (dynamic|) (local-dynamic|) (all|)(entry-file FILE_NAME|)",  ctc_cli_gb_l2_show_fdb_hw_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l4_type_cmd_vtysh,  "parser l4-type (tcp | udp | gre | ipinip | v6inip | icmp | igmp ) (enable | disable)",  ctc_cli_parser_l4_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l2_port_action_cmd_vtysh,  "show pdu l2pdu port-action port PORT action-index ACTION_INDEX action-type",  ctc_cli_pdu_show_l2_port_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_set_mirror_discard_cmd_vtysh,  "mirror dropped-pkt (port-session|vlan-session|acl-session |all) direction (ingress|egress|both) (enable | disable)",  ctc_cli_mirror_set_mirror_discard_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_monitor_sgmac_prbs_xauipcs_cmd_vtysh,  "monitor sgmac-prbs xauipcs lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_monitor_sgmac_prbs_xauipcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_debug_on_cmd_vtysh,  "debug learning-aging (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_learning_aging_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_csf_type_cmd_vtysh,  "local-mep csf-type ( los | fdi | rdi | dci) (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_csf_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeHdrEdit_cmd_vtysh,  "show statistic (epe) (epehdredit) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeHdrEdit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_common_set_defect_to_rdi_cmd_vtysh,  "oam common defect-to-rdi ({ mac-status-change | src-mac-mismatch | maid-mismatch |     low-ccm | no-rmep | interval-mismatch | dloc | dcsf | all } | )",  ctc_cli_oam_common_set_defect_to_rdi_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_show_config_cmd_vtysh,  "show learning-aging config (aging-tbl (mac|scl) |)",  ctc_cli_learning_aging_show_config_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_FwdMsg_cmd_vtysh,  "show statistic (bsr) (fwdmsg) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_FwdMsg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_mpls_show_count_cmd_vtysh,  "show mpls count",  ctc_cli_gb_mpls_show_count_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_remove_all_entry_cmd_vtysh,  "scl remove-all group GROUP_ID",  ctc_cli_scl_remove_all_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_copy_entry_cmd_vtysh,  "acl entry ENTRY_ID copy-to group GROUP_ID entry ENTRY_ID ",  ctc_cli_acl_copy_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_clear_sync_intf_rx_code_cmd_vtysh,  "ptp CHIP sync-rx-code-clear",  ctc_cli_ptp_clear_sync_intf_rx_code_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_bridge_cmd_vtysh,  "port GPHYPORT_ID bridge (enable | disable)",  ctc_cli_port_set_bridge_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_mac_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-mac-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) " " (mac MAC) " " (mac-is-da MAC_IS_DA) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_mac_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en_cmd_vtysh,  "local-mep p2p-ccm-tx-use-uc-da (enable | disable) level LEVEL",  ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_oam_OamFwd_cmd_vtysh,  "show statistic (oam) (oamfwd) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_oam_OamFwd_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_set_lbm_proc_in_asic_cmd_vtysh,  "oam cfm lbm-proc-in-asic (enable| disable)",  ctc_cli_oam_set_lbm_proc_in_asic_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_reg_cmd_vtysh,  "reg_read ADDR",  ctc_cli_dbg_tool_read_reg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_cpu_reason_map_cmd_vtysh,  "qos (cpu-reason REASON) map-to (queue-id QUEUE reason-group GROUP|cos COS)",  ctc_cli_qos_set_cpu_reason_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_stats_cmd_vtysh,  "nexthop add stats NHID",  ctc_cli_nh_add_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_ipv6_entry_cmd_vtysh,  "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|)" " ipv6-entry ({" " cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI |       svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI " " | mac-sa MAC MASK | mac-da MAC MASK |       layer2-type L2_TYPE MASK | layer3-type L3_TYPE MASK " "|ip-sa X:X::X:X X:X::X:X | ip-da X:X::X:X X:X::X:X | flow-label FLOW_LABEL MASK " "|l4-protocol L4_PROTOCOL MASK {l4-src-port L4_SRC_PORT MASK|l4-dst-port L4_DST_PORT MASK|     icmp-type ICMP_TYPE MASK| icmp-code ICMP_CODE MASK } " "|dscp DSCP MASK | eth-type ETH_TYPE MASK | l4-type L4_TYPE MASK | ip-option IP_OPTION | ip-frag IP_FRAG | prec IP_PREC MASK " " }|)" " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_ipv6_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_show_state_cmd_vtysh,  "show ipuc state",  ctc_cli_gb_ipuc_show_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_create_cmd_vtysh,  "linkagg create linkagg AGG_ID ((static|dynamic|hw-sync)|)",  ctc_cli_linkagg_create_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_set_ecmp_hash_cmd_vtysh,  "parser ecmp-hash{ip ({l3protocol|dscp|flow-label|ipsa|ipda}|)|l4 ({src-port|dst-port|gre-key}|)|        mpls ({m-l3protocol|m-dscp|m-flow-label|m-ipsa|m-ipda}|)|fcoe ({sid|did}|)|trill         ({inner-vlan|ingress-nickname|egress-nickname}|) | l2 ({cos|macsa|macda}|) }",  ctc_cli_parser_set_ecmp_hash_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_set_entry_priority_cmd_vtysh,  "acl entry ENTRY_ID priority PRIORITY",  ctc_cli_acl_set_entry_priority_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_l2_show_fdb_detail_cmd_vtysh,  "show l2 fdb detail by (mac MAC|mac-fid MAC FID|port GPORT_ID|fid FID|port-fid GPORT_ID FID|all ) (static|) (dynamic|) (local-dynamic|) (all|)(entry-file FILE_NAME|)",  ctc_cli_gb_l2_show_fdb_detail_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_debug_off_cmd_vtysh,  "no debug scl (ctc | sys)",  ctc_cli_scl_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_add_ipv4_default_entry_cmd_vtysh,  "ipmc add default (to-cpu | drop)",  ctc_cli_ipmc_add_ipv4_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_add_ipv4_tunnel_cmd_vtysh,  "ipuc tunnel add A.B.C.D { vrf VRFID | nhid NHID | tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY         | ip-sa A.B.C.D | ecmp | ttl-check | use-outer-ttl | lookup-by-outer | isatap-source-check | copy-to-cpu         | rpf-check (l3if l3ifid0 VALUE  (l3ifid1 VALUE | ) (l3ifid2 VALUE | ) (l3ifid3 VALUE | ) (more-rpf |) | )         | use-outer-info | service-id SERVICE_ID (logic-port GPHYPORT_ID|) (service-acl-en|) (service-queue-en|)         (service-policer-en|) | stats STATS_ID | discard | use-flex |}",  ctc_cli_ipuc_add_ipv4_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_update_raps_member_cmd_vtysh,  "aps aps-bridge raps group GROUP_ID (add|remove) (member-port) GPORT_ID (remote-chip|)",  ctc_cli_aps_update_raps_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_reason_shape_cmd_vtysh,  "qos shape cpu-reason REASON_ID (none | pir PIR)",  ctc_cli_qos_set_reason_shape_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_port_queue_class_cmd_vtysh,  "qos schedule queue-class (service-id SERVICE_ID |) (port GPHYPORT_ID) (queue-id  QUEUE_ID) (confirm-class CFMCLASS | exceed-class ECDCLASS)",  ctc_cli_qos_set_port_queue_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_group_wdrr_weight_cmd_vtysh,  "qos schedule group-wdrr (service-id SERVICE |) (port GPHYPORT_ID) (weight WEIGHT)",  ctc_cli_qos_set_group_wdrr_weight_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_gephy_cmd_vtysh,  "write gephy mdio-bus BUS_NUM phy-addr PHY_ADDR reg REG value VALUE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_gephy_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_mpls_show_mpls_status_cmd_vtysh,  "show mpls status",  ctc_cli_gb_mpls_show_mpls_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_ip_src_guard_add_entry_cmd_vtysh,  "security ip-src-guard add port GPORT_ID {ipv4 A.B.C.D |mac MAC|vlan VLAN_ID (is-svlan |)}",  ctc_cli_security_ip_src_guard_add_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_OobFc_cmd_vtysh,  "show statistic (misc) (oobfc) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_OobFc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_mcast_debug_off_cmd_vtysh,  "no debug mcast (ctc|sys)",  ctc_cli_ipmc_mcast_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_set_aps_bridge_group_cmd_vtysh,  "aps set aps-bridge group GROUP_ID (aps-type (l2|mpls) |) (working-port GPORT_ID (w-l3if-id INTERFACE_ID|)|next-w-aps  ID) (protection-port GPORT_ID (p-l3if-id INTERFACE_ID|) (physical-isolated|) | next-p-aps ID | raps-en raps-groupid GROUP_ID) (hw-based-aps-en|)",  ctc_cli_aps_set_aps_bridge_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_set_break_en_cmd_vtysh,  "stacking (break|full-mesh) (enable|disable)",  ctc_cli_stacking_set_break_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_EpeNextHop_cmd_vtysh,  "show statistic (epe) (epenexthop) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_EpeNextHop_cmd_help)

DEFSH (VTYSH_SDK, common_cli_show_memory_module_detaill_cmd_vtysh,  "show memory pool stats (fdb|mpls|queue|ipuc|ipmc|nexthop|scl" "|l3if|port|vlan|oam|sort-key|linklist|avl-tree|ctc-cli|lib-ctccli|vector|hash" "|spool |ptp | ftm |stacking |dma|storm-control |mirror|sal" "|opf|l3hash | fpa" "|acl | sync_ether| rpf| aps" "|all) (detail|)",  common_cli_show_memory_module_detaill_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_add_ipv4_vlan_class_cmd_vtysh,  "vlan classifier add ipv4 {ip-sa A.B.C.D (mask A.B.C.D|)|ip-da A.B.C.D (mask A.B.C.D|)|mac-sa MAC|mac-da MAC|l3-type (arp|rarp|ipv4|L3TYPE)|l4-type (udp|tcp|L4TYPE)     |l4-srcport L4PORT |l4-destport L4PORT} vlan VLAN_ID ((cos COS)|)",  ctc_cli_vlan_add_ipv4_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_update_rmep_cmd_vtysh,  "remote-mep bfd update (ip-bfd my-discr MY_DISCR | mpls-bfd my-discr MY_DISCR | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL)})          (mep-en (enable| disable) | sf-state SF_STATE | diag DIAG | state STATE | your-discr YOUR_DISCR | rx-timer MIN_INTERVAL)",  ctc_cli_oam_bfd_update_rmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_learning_cmd_vtysh,  "vlan VLAN_ID mac-learning (enable|disable)",  ctc_cli_vlan_set_learning_cmd_help)

DEFSH (VTYSH_SDK, show_l3if_vmac_low_8bits_cmd_vtysh,  "show l3if vmac low-8bits ifid IFID low-8bits-mac-index MAC_INDEX",  show_l3if_vmac_low_8bits_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_port_queue_shape_cmd_vtysh,  "qos shape queue" "((service-id SERVICE|) (port GPHYPORT_ID) (queue-id QUEUE_ID) | reason-id REASON)" "(none | cir CIR (pir PIR|))",  ctc_cli_qos_set_port_queue_shape_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_ELoop_cmd_vtysh,  "show statistic (epe) (eloop) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_ELoop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_packet_debug_show_cmd_vtysh,  "show debug packet (ctc | sys)",  ctc_cli_packet_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_debug_show_cmd_vtysh,  "show debug mpls (ctc | sys)",  ctc_cli_mpls_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_set_l2flex_cmd_vtysh,  "parser l2 flex mac-da-basic-offset VAL protocol-basic-offset VAL min-len LEN l2-basic-offset VAL",  ctc_cli_parser_l2_set_l2flex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_mapping_set_mapping_mode_cmd_vtysh,  "port mapping (on | off)",  ctc_cli_port_mapping_set_mapping_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_xgphy_sp_dis_cmd_vtysh,  "scan xgphy-special disable (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_xgphy_sp_dis_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_show_mpls_property_cmd_vtysh,  "show l3if ifid IFID mpls-property (mpls-en | mpls-label-space)",  ctc_cli_l3if_show_mpls_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_global_set_hash_type_cmd_vtysh,  "parser global hash ecmp-hash-type TYPE linkagg-hash-type TYPE",  ctc_cli_parser_global_set_hash_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_resrc_mgr_enable_cmd_vtysh,  "qos resrc-mgr (enable | disable)",  ctc_cli_qos_set_resrc_mgr_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_show_port_queue_stats_cmd_vtysh,  "show qos stats" "((service-id SERVICE|) (port GPHYPORT_ID) (queue-id QUEUE_ID) | reason-id REASON)",  ctc_cli_qos_show_port_queue_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_packet_flow_diag_start_cmd_vtysh,  "packet flow diagnose start (lchip CHIP_ID|)",  ctc_cli_dbg_tool_packet_flow_diag_start_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_show_bfd_session_state_cmd_vtysh,  "show bfd session state num NUM",  ctc_cli_gb_oam_show_bfd_session_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_remove_pw_cmd_vtysh,  "mpls l2vpn-pw remove (space SPACEID |) LABEL",  ctc_cli_mpls_remove_pw_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_debug_on_cmd_vtysh,  "debug l3if (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_l3if_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l3if_action_action_cmd_vtysh,  "show pdu l3pdu l3if-action l3if L3IFID action-index ACTION_INDEX action-type",  ctc_cli_pdu_show_l3if_action_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_vlan_mapping_cmd_vtysh,  "port GPHYPORT_ID vlan-mapping (enable|disable)",  ctc_cli_port_set_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_datapath_cmd_vtysh,  "chip datapath mode set (48-4 | 36-6) (chip LOCAL_CHIPID |)",  ctc_cli_chip_set_datapath_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_stp_set_state_cmd_vtysh,  "stp state port GPHYPORT_ID stpid STP_ID state (forwarding|blocking|learning) ",  ctc_cli_l2_stp_set_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_remove_vmac_low_8bits_cmd_vtysh,  "l3if vmac ifid IFID remove low-8bits-mac-index MAC_INDEX",  ctc_cli_l3if_remove_vmac_low_8bits_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_set_ecmp_mode_cmd_vtysh,  "nexthop ecmp-mode (enable|disable)",  ctc_cli_gb_nh_set_ecmp_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_IpeFib_cmd_vtysh,  "show statistic (ipe) (ipefib) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_IpeFib_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_GlobalStats_cmd_vtysh,  "show statistic (misc) (globalstats) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_GlobalStats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_exec_cfg_cmd_vtysh,  "execute chip CHIP_ID FILE",  ctc_cli_dbg_tool_exec_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_port_queue_stats_en_cmd_vtysh,  "qos stats" "((service-id SERVICE|) (port GPHYPORT_ID) (queue-id QUEUE_ID) | reason-id REASON)" "(enable|disable)",  ctc_cli_qos_set_port_queue_stats_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_show_group_info_cmd_vtysh,  "show acl group-info group GROUP_ID",  ctc_cli_acl_show_group_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dma_thread_cmd_vtysh,  "dma thread monitor start",  ctc_cli_dma_thread_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l3_classify_l3pdu_by_l4port_cmd_vtysh,  "pdu l3pdu classify layer4-port index INDEX is-udp UDP_EN_VAL is-tcp TCP_EN_VAL destport DESTPORT",  ctc_cli_pdu_l3_classify_l3pdu_by_l4port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_update_mpls_nexthop_to_unrov_cmd_vtysh,  "nexthop update mpls NHID fwd2unrov",  ctc_cli_nh_update_mpls_nexthop_to_unrov_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_clear_policer_stats_cmd_vtysh,  "clear qos policer stats (port GPORT (in|out) |flow POLICER_ID|service SEVICE_ID)         (hbwp-en cos-index INDEX |)",  ctc_cli_qos_clear_policer_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_sfp_para_cmd_vtysh,  "scan sfp-para bit-map BIT_MAP address ADDR offset OFFSET (len LEN|) (interval INTERVAL|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_sfp_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_add_lmep_cmd_vtysh,  "local-mep bfd add my-discr MY_DISCR ({mep-en | mep-on-cpu | desired-tx-interval DESIRED_TX_INTERVAL | state STATE | diag DIAG | detect-mult DETECT_MULT | aps-enable }|)        (ip-bfd { is-single-hop | src-udp-port SRC_PORT | nhid NHID} |        mpls-bfd { pw-vccv | vccv-with-ipv4 | mpls-in-label LABEL | protection-path | nhid NHID | src-udp-port SRC_PORT | ipv4-sa A.B.C.D }|         tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL) | protection-path | nhid NHID | mpls-ttl MPLS_TTL | without-gal | mep-id MEP_ID | lm-en LM_COS_TYPE LM_COS | csf-en CSF_TYPE} )         ({mep-index IDX | master-gchip GCHIP_ID} | )",  ctc_cli_oam_bfd_add_lmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_global_property_cmd_vtysh,  "show ptp global-property (all |ucast-en | sync-cpu-en | signaling-cpu-en | management-cpu-en |      delay-request-en | port-based-en | cache-aging-time | interface-selected)",  ctc_cli_ptp_show_global_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_igs_prec_map_cmd_vtysh,  "qos domain DOMAIN map ip-prec IP_PREC to priority PRIORITY color (green |yellow | red)",  ctc_cli_qos_set_igs_prec_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_ipv4_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-ipv4-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) " " (ip-sa A.B.C.D) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_ipv4_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_acl_show_entry_info_cmd_vtysh,  "show acl entry-info     (all (divide-by-group|)  | entry ENTRY_ID |group GROUP_ID |priority PRIORITY )     (type (mac|ipv4|ipv6|mpls)|) (detail|)",  ctc_cli_gb_acl_show_entry_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_update_ecmp_cmd_vtysh,  "nexthop update ecmp ECMPID NHID (add | remove)",  ctc_cli_nh_update_ecmp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_debug_on_cmd_vtysh,  "debug mirror (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_mirror_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_debug_cmd_vtysh,  "show debug parser (ctc | sys)",  ctc_cli_parser_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_show_nexthop_by_nhid_type_cmd_vtysh,  "show nexthop NHID (detail |)",  ctc_cli_gb_nh_show_nexthop_by_nhid_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_hash_ipv4_entry_cmd_vtysh,  "acl add (group GROUP_ID|) (entry ENTRY_ID)" " hash-ipv4-entry " "(gport GPHYPORT_ID|) (is-logic-port IS_LOGIC_PORT|) (ip-sa A.B.C.D|) (ip-da A.B.C.D|) (l4-src-port L4_SRC_PORT|) (l4-dst-port L4_DST_PORT|) (dscp DSCP|)     (l4-protocol L4_PROTOCOL|) (arp-packet ARP_PACKET|) " "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_add_hash_ipv4_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_debug_sim_chip_model_on_cmd_vtysh,  "debug chip-agent simulator chip-model (all | {fwd-path | parser-info | key-info | lookup-key | tbl-ptr | discard-type | qmgr-msg | qmgr-hdr | qmgr-linkagg | aging})",  ctc_cli_gb_chip_agent_debug_sim_chip_model_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_mdio_clock_frequency_cmd_vtysh,  "chip set mdio (gephy|xgphy) freq FREQ",  ctc_cli_chip_set_mdio_clock_frequency_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_mac_Sgmac_cmd_vtysh,  "show statistic (mac) (sgmac) (index INDEX) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_mac_Sgmac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_inter_laken_prbs_cmd_vtysh,  "check inter-laken-prbs lane LANEID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_check_inter_laken_prbs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_uninstall_group_cmd_vtysh,  "scl uninstall group GROUP_ID",  ctc_cli_scl_uninstall_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_dot1q_subtract_bytes_cmd_vtysh,  "stats dot1q-subtract-byte port GPHYPORT_ID bytes PKT_BYTES",  ctc_cli_stats_set_dot1q_subtract_bytes_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_rpf_debug_on_cmd_vtysh,  "debug rpf (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_ipmc_rpf_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_remove_ipv6_member_cmd_vtysh,  "ipmc ipv6 remove member group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID ((""(sub-if gport GPORT_ID vlanid VLAN_ID | vlan-if vlanid VLAN_ID gport GPORT_ID ({vlan-port}|) | phy-if gport GPORT_ID )" "  | (ip-based-l2mc gport GPORT_ID)) (remote-chip|) | ""nexthop-id NHID" ")",  ctc_cli_ipmc_remove_ipv6_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_hss12g_enable_cmd_vtysh,  "chip set hss12g HSS_ID (xfi|sgmii|qsgmii|disable) (lchip LCHIP|)",  ctc_cli_chip_set_hss12g_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_tx_cos_cmd_vtysh,  "local-mep tx-cos set COS (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_tx_cos_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_hss6g_cmd_vtysh,  "write hss6g address ADDR value VALUE mask MASK (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_hss6g_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_protocol_vlan_cmd_vtysh,  "port GPHYPORT_ID protocol-vlan (enable|disable)",  ctc_cli_port_set_protocol_vlan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_all_cmd_vtysh,  "show pdu all",  ctc_cli_pdu_show_all_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_add_vpws_ilm_cmd_vtysh,  "mpls ilm add vpws-vc SPACEID LABEL NHID (control-word | ) (oam |) (inner-pkt-raw |) (SERVICE_ID | stats STATS_ID |)",  ctc_cli_mpls_add_vpws_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_random_log_cmd_vtysh,  "port GPHYPORT_ID random-log direction (ingress|egress|both)(enable|disable)",  ctc_cli_port_set_random_log_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_packet_debug_on_cmd_vtysh,  "debug packet (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_packet_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l3_set_l3if_action_cmd_vtysh,  "pdu l3pdu l3if-action l3if L3IFID action-index     ACTION_INDEX action-type(forward|copy-to-cpu)",  ctc_cli_pdu_l3_set_l3if_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_install_group_cmd_vtysh,  "scl install group GROUP_ID (type (port GPORT| global | port-class CLASS_ID (lchip LCHIP) |" "logic-port LOGIC_PORT)|)",  ctc_cli_scl_install_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_dump_drvio_cmd_vtysh,  "show chip-agent cache driver-io (both|tx|rx) (<1-64>|) (detail|)",  ctc_cli_gb_chip_agent_dump_drvio_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_TcamDsMem_cmd_vtysh,  "show statistic (share) (tcamdsmem) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_TcamDsMem_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_i2c_scan_en_cmd_vtysh,  "chip set i2c-scan (enable | disable)",  ctc_cli_chip_set_i2c_scan_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_misc_l2hdr_cmd_vtysh,  "nexthop add misc replace-l2hdr NHID (dsnh-offset OFFSET|) (reflective |port GPORT_ID) " "(""(swap-mac | {replace-mac-da MAC | replace-mac-sa MAC})"" |)",  ctc_cli_nh_add_misc_l2hdr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_debug_on_cmd_vtysh,  "debug alloc (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_gb_ftm_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_mac_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " mac-hash-entry " " mac MAC " " mac-is-da MAC_IS_DA " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_mac_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_tod_intf_rx_code_cmd_vtysh,  "show ptp CHIP tod-rx-code",  ctc_cli_ptp_show_tod_intf_rx_code_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_mapping_show_mapping_mode_cmd_vtysh,  "show port mapping",  ctc_cli_port_mapping_show_mapping_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_egs_dscp_map_cmd_vtysh,  "qos domain DOMAIN map priority PRIORITY color (green |yellow | red) to dscp DSCP",  ctc_cli_qos_set_egs_dscp_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_isolation_id_cmd_vtysh,  "port GPHYPORT_ID isolation-id direction (ingress|egress|both)     ({ucast-flooding | mcast-flooding | known-mcast-flooding | bcast-flooding}|)     value VALUE",  ctc_cli_port_set_isolation_id_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_lm_type_cmd_vtysh,  "local-mep lm-type (dual | single | none)",  ctc_cli_oam_cfm_update_lmep_lm_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_igs_dscp_map_cmd_vtysh,  "qos domain DOMAIN map dscp DSCP to priority PRIORITY color (green |yellow | red)",  ctc_cli_qos_set_igs_dscp_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_port_status_cmd_vtysh,  "oam cfm update-port-status port GPORT_ID vlan VLAN_ID port-status PORT_STATUS",  ctc_cli_oam_cfm_update_port_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_remove_remote_member_cmd_vtysh,  "nexthop mcast NHID (add | remove) remote-member port GPORT_ID",  ctc_cli_nh_add_remove_remote_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_show_stp_state_by_port_cmd_vtysh,  "show stp state port GPHYPORT_ID ",  ctc_cli_l2_show_stp_state_by_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_debug_on_cmd_vtysh,  "diag debug  (on|off)",  ctc_cli_dbg_tool_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_remove_ipv4_cmd_vtysh,  "ipuc remove VRFID A.B.C.D MASK_LEN NHID ( ecmp  |) (l4-dst-port L4PORT (tcp-port|)|)",  ctc_cli_ipuc_remove_ipv4_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_debug_off_cmd_vtysh,  "no debug mpls (ctc | sys)",  ctc_cli_mpls_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_voltage_cmd_vtysh,  "read voltage (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_voltage_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_set_ipv6_force_route_cmd_vtysh,  "ipmc ipv6 force-route {ip-addr0 X:X::X:X MASK_LEN | ip-addr1 X:X::X:X MASK_LEN} (force-bridge|force-unicast)",  ctc_cli_ipmc_set_ipv6_force_route_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_add_ipv4_group_cmd_vtysh,  "ipmc add group GROUPID group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID ({ttl-check |rpf-check |rpf-fail-to-cpu |stats STATS_ID | ip-based-l2mc | ptp-entry | redirect-tocpu } | )",  ctc_cli_ipmc_add_ipv4_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_IpeForward_cmd_vtysh,  "show statistic (ipe) (ipeforward) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_IpeForward_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_debug_off_cmd_vtysh,  "no debug oam (ctc|sys)",  ctc_cli_oam_cfm_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_update_pbr_action_cmd_vtysh,  "acl update pbr-entry ENTRY_ID " "(permit|deny) ({forward NH_ID | ecmp | ttl-check | icmp-check | copy-to-cpu}|)",  ctc_cli_acl_update_pbr_action_cmd_help)

DEFSH (VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK, ctc_cli_common_fast_enter_debug_mode_cmd_vtysh,  "debug",  ctc_cli_common_fast_enter_debug_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_cvlan_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-cvlan-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) direction (ingress|egress) " " (cvlan VLAN_ID) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_cvlan_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_interrupt_get_en_cmd_vtysh,  "show interrupt LCHIP type TYPE sub-type TYPE enable",  ctc_cli_gb_interrupt_get_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l3_unmapping_l4type_cmd_vtysh,  "parser l3 mapping-l4-type unmapping index INDEX ",  ctc_cli_parser_l3_unmapping_l4type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_integrity_golden_cmd_vtysh,  "integrity-compare SRC_FILE DEST_FILE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_integrity_golden_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_dir_property_cmd_vtysh,  "vlan VLAN_ID dir-property      ( acl-en | acl-vlan-classid | acl-routed-pkt-only)     direction (ingress|egress|both) value VALUE ",  ctc_cli_vlan_set_dir_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_clock_time_cmd_vtysh,  "show ptp CHIP time",  ctc_cli_ptp_show_clock_time_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_GreatbeltSup_cmd_vtysh,  "show statistic (misc) (greatbeltsup) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_GreatbeltSup_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_add_hash_mac_entry_cmd_vtysh,  "acl add (group GROUP_ID|) (entry ENTRY_ID)" " hash-mac-entry " "(gport GPHYPORT_ID|) (is-logic-port IS_LOGIC_PORT|) (mac-da MAC|) (eth-type ETH_TYPE|) (vlan VLAN_ID|) (cos COS|) " "(permit|deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | priority-and-color PRIORITY COLOR    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp     | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)      (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )    }|)",  ctc_cli_acl_add_hash_mac_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_mirror_show_mirror_info_cmd_vtysh,  "show mirror info (port GPHYPORT_ID | vlan VLAN_ID) ",  ctc_cli_gb_mirror_show_mirror_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipmc_status_cmd_vtysh,  "show ipmc status",  ctc_cli_gb_ipmc_show_ipmc_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_shape_ipg_en_cmd_vtysh,  "qos shape ipg (enable|disable)",  ctc_cli_qos_set_shape_ipg_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_add_rspan_session_cmd_vtysh,  "mirror remote-destination add session SESSION_ID direction (ingress|egress|both) (port-session | vlan-session | acl-session (priority-id PRIORITY_ID|)) (port GPORT_ID) (vlan VLAN_ID | nexthop NH_ID) ",  ctc_cli_mirror_add_rspan_session_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_xgphy_cmd_vtysh,  "read xgphy mdio-bus BUS_NUM phy-addr PHY_ADDR dev DEV reg REG (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_xgphy_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_vlan_class_default_entry_cmd_vtysh,  "vlan classifier remove-mismatch (mac|ipv4|ipv6)",  ctc_cli_vlan_remove_vlan_class_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_remove_stats_cmd_vtysh,  "mpls remove stats SPACEID LABEL",  ctc_cli_mpls_remove_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_port_cfg_cmd_vtysh,  "show stats ( igs-port-opt-type | egs-port-opt-type )",  ctc_cli_stats_show_port_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_datapath_infor_cmd_vtysh,  "show chip datapath information (pll|hss|serdes|mac|synce|calender|misc|channel|inter-port)",  ctc_cli_chip_show_datapath_infor_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipuc_reinit_cmd_vtysh,  "ipuc reinit pipline3 (enable | disable )",  ctc_cli_gb_ipuc_reinit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_gephy_para_cmd_vtysh,  "scan gephy-para bit-map0 BIT_MAP0 bit-map1 BIT_MAP1 (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_scan_gephy_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_sync_ether_debug_on_cmd_vtysh,  "debug sync-ether (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_sync_ether_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_pcie_io_cmd_vtysh,  "read pcie-io address ADDR_OFFSET (LEN|)",  ctc_cli_dbg_tool_read_pcie_io_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_gephy_sp_para_cmd_vtysh,  "scan gephy-special-para spreg-index SPREG_INDEX reg REG bit-ctrl BIT_CTRL (intr-en|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_gephy_sp_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_dot1q_type_cmd_vtysh,  "port GPHYPORT_ID dot1q-type (untagged | cvlan-tagged | svlan-tagged | double-tagged)",  ctc_cli_port_set_dot1q_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_create_vlan_vrange_cmd_vtysh,  "vlan range create vrange-grp GROUP_ID direction (ingress|egress) (is-svlan|is-cvlan)",  ctc_cli_gb_vlan_create_vlan_vrange_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_debug_off_cmd_vtysh,  "no debug learning-aging (ctc | sys)",  ctc_cli_learning_aging_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l3_classify_l3pdu_by_l3proto_cmd_vtysh,  "pdu l3pdu classify layer3-proto index INDEX layer3-proto LAYER3_PROTO",  ctc_cli_pdu_l3_classify_l3pdu_by_l3proto_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_untag_dft_vid_disable_cmd_vtysh,  "port GPHYPORT_ID untag-default-vlan disable",  ctc_cli_port_set_untag_dft_vid_disable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_port_enable_port_tunnel_cmd_vtysh,  "oam cfm port GPORT_ID tunnel (enable| disable)",  ctc_cli_oam_cfm_set_port_enable_port_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_update_update_timer_enable_cmd_vtysh,  "oam cfm update-timer (enable| disable)",  ctc_cli_gb_oam_update_update_timer_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_clear_stats_cmd_vtysh,  "clear stats STATS_ID",  ctc_cli_stats_clear_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_chip_cmd_vtysh,  "write address ADDR_OFFSET value VALUE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_chip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_rspan_escape_en_cmd_vtysh,  "mirror rspan escape (enable | disable)",  ctc_cli_mirror_rspan_escape_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_add_ipv6_member_cmd_vtysh,  "ipmc ipv6 add member group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID ((""(sub-if gport GPORT_ID vlanid VLAN_ID | vlan-if vlanid VLAN_ID gport GPORT_ID ({vlan-port}|) | phy-if gport GPORT_ID )" "  | (ip-based-l2mc gport GPORT_ID)) (remote-chip|) | ""nexthop-id NHID" ")",  ctc_cli_ipmc_add_ipv6_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_add_ipv6_default_entry_cmd_vtysh,  "ipmc ipv6 add default (to-cpu | drop)",  ctc_cli_ipmc_add_ipv6_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_show_ipv4_info_cmd_vtysh,  "show ipuc VRFID A.B.C.D MASK_LEN (detail |)",  ctc_cli_ipuc_show_ipv4_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_property_cmd_vtysh,  "port GPHYPORT_ID property (((route-en| pkt-tag-high-priority | hw-learn  |     ipv6-lookup | ipv4-lookup | use-default-lookup | ipv6-to-mac-key | ipv4-to-mac-key | trill | discard-non-tril | discard-tril |     reflective-bridge | fcoe | fcoe-rpf | replace-stag-cos| replace-ctag-cos | replace-dscp |    l3pdu-arp-action | l3pdu-dhcp-action | tunnel-rpf-check | ptp | rpf-type | is-leaf | priority-tag | default-pcp | default-dei|     | linkscan-en |aps-failover | linkagg-failover | mac-ts)     value VALUE)| (raw-packet-type ( none | ethernet | ipv4 | ipv6 ))| (pvlan-type ( none | promiscuous | isolated | community )     (isolation-id VALUE|)) | (qos-trust (port | outer | cos | dscp | ip-prec | stag-cos | ctag-cos)) )",  ctc_cli_port_set_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_add_rmep_cmd_vtysh,  "remote-mep add rmep-id MEP_ID ({mep-en | csf-en}|)    (eth-oam level LEVEL rmep-sa-mac MAC_ADDR ({seq-num-update-enable|mac-update-enable}|) | tp-y1731 ) (mep-index IDX|)",  ctc_cli_oam_cfm_add_rmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_vlan_max_md_level_cmd_vtysh,  "oam cfm vlan VLAN_ID (ingress| egress| both-direction) (max-md-level MD_LEVEL)",  ctc_cli_oam_cfm_set_vlan_max_md_level_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_net_tx_packet_capture_cmd_vtysh,  "check net-tx-packet-capture lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_check_net_tx_packet_capture_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_rpf_debug_off_cmd_vtysh,  "no debug rpf (ctc|sys)",  ctc_cli_ipmc_rpf_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_update_error_cache_timer_enable_cmd_vtysh,  "oam cfm error-cache-timer (enable| disable)",  ctc_cli_gb_oam_update_error_cache_timer_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_sync_intf_toggle_time_cmd_vtysh,  "show ptp CHIP sync-toggle",  ctc_cli_ptp_show_sync_intf_toggle_time_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_MetFifo_cmd_vtysh,  "show statistic (bsr) (metfifo) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_MetFifo_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_read_gephy_interface_cmd_vtysh,  "chip read gephy port GPHYPORT_ID reg REG",  ctc_cli_chip_read_gephy_interface_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_flow_ctl_cmd_vtysh,  "port GPHYPORT_ID flow-ctl (cos COS|)(ingress|egress|both)(enable|disable)",  ctc_cli_port_set_flow_ctl_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_cpu_reason_dest_cmd_vtysh,  "qos cpu-reason REASON dest-to (local-cpu|remote-cpu-port GPORT|local-port GPORT|drop)",  ctc_cli_qos_set_cpu_reason_dest_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_queue_wdrr_weight_mtu_cmd_vtysh,  "qos schedule wdrr-weight-mtu MTU",  ctc_cli_qos_set_queue_wdrr_weight_mtu_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_show_bfd_up_time_cmd_vtysh,  "show bfd up time",  ctc_cli_gb_oam_show_bfd_up_time_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_serverip_cmd_vtysh,  "chip-agent server-ip A.B.C.D (port <1024-65535>|)",  ctc_cli_gb_chip_agent_serverip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_run_bist_cmd_vtysh,  "run bist (lpmtcam|intcam) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_run_bist_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_update_mcast_service_queue_cmd_vtysh,  "nexthop mcast-group GROUP_ID service-queue update nexthop NHID",  ctc_cli_gb_nh_update_mcast_service_queue_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_all_vlan_mapping_cmd_vtysh,  "vlan mapping flush-all port GPORT_ID",  ctc_cli_vlan_remove_all_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_get_threshold_cfg_cmd_vtysh,  "show stats ( packet-threshold | byte-threshold | fifo-depth-threshold )",  ctc_cli_stats_get_threshold_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_debug_off_cmd_vtysh,  "no debug parser (ctc | sys)",  ctc_cli_parser_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_read_i2c_buffer_cmd_vtysh,  "chip read i2c-buffer lchip CHIP_ID length LENGTH",  ctc_cli_chip_read_i2c_buffer_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_stats_cmd_vtysh,  "nexthop remove stats NHID",  ctc_cli_nh_remove_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_update_normal_ilm_cmd_vtysh,  "mpls ilm update space SPACEID label LABEL (uniform | short-pipe | pipe) (oam | )",  ctc_cli_mpls_update_normal_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_interrupt_get_status_cmd_vtysh,  "show interrupt LCHIP type TYPE sub-type TYPE status",  ctc_cli_gb_interrupt_get_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_ipv6_vlan_class_cmd_vtysh,  "vlan classifier remove ipv6 {ip-sa X:X::X:X (mask X:X::X:X|)|ip-da X:X::X:X (mask X:X::X:X|)|mac-sa MAC|mac-da MAC|l4-srcport L4PORT |l4-destport L4PORT|     l4-type (udp|tcp|L4TYPE)}",  ctc_cli_vlan_remove_ipv6_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_qos_show_qos_domain_cmd_vtysh,  "show qos domain DOAMIN type (cos-map-pri|ip-prec-map-pri|dscp-map-pri|exp-map-pri|pri-map-cos|pri-map-dscp|pri-map-exp)",  ctc_cli_gb_qos_show_qos_domain_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_remove_chan_cmd_vtysh,  "oam cfm lookup-chan remove gport GPORT_ID direction (up |down ) (link-oam|) (vlan VLAN_ID|)",  ctc_cli_oam_cfm_remove_chan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_max_frame_size_cmd_vtysh,  "port GPHYPORT_ID max-frame-size SIZE",  ctc_cli_port_set_max_frame_size_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_phy_scan_en_cmd_vtysh,  "chip set phy-scan (enable | disable)",  ctc_cli_chip_set_phy_scan_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_reset_mac_cmd_vtysh,  "clear stats ( mac-rx | mac-tx ) port GPHYPORT_ID",  ctc_cli_stats_reset_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_debug_on_cmd_vtysh,  "debug oam (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_oam_cfm_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_mpls_show_space_cmd_vtysh,  "show mpls space",  ctc_cli_gb_mpls_show_space_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_tp_section_oam_cmd_vtysh,  "oam tp section (port | interface)",  ctc_cli_gb_oam_tp_section_oam_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_DynamicDsMem_cmd_vtysh,  "show statistic (share) (dynamicdsmem) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_DynamicDsMem_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_set_all_pdu_to_cpu_cmd_vtysh,  "oam relay-all-pkt-to-cpu (enable| disable)",  ctc_cli_oam_set_all_pdu_to_cpu_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_set_lbr_use_share_mac_cmd_vtysh,  "oam cfm lbr-sa-use-share-mac (enable| disable)",  ctc_cli_oam_set_lbr_use_share_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_rmep_seq_num_en_cmd_vtysh,  "remote-mep seq-num (enable| disable) level LEVEL rmep-id MEP_ID",  ctc_cli_oam_cfm_update_rmep_seq_num_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_classify_l2pdu_by_macda_cmd_vtysh,  "pdu l2pdu classify macda-48bit index INDEX macda MACDA (mask MASK|) ",  ctc_cli_pdu_l2_classify_l2pdu_by_macda_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_min_frame_size_cmd_vtysh,  "port GPHYPORT_ID min-frame-size SIZE",  ctc_cli_port_set_min_frame_size_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dma_debug_off_cmd_vtysh,  "no debug dma (ctc | sys)",  ctc_cli_dma_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_remove_l2mc_member_cmd_vtysh,  "nexthop mcast NHID (add | remove) bridge-member port GPORT_ID",  ctc_cli_nh_add_remove_l2mc_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l4_app_data_cmd_vtysh,  "show parser l4 app-data index VAL",  ctc_cli_parser_show_l4_app_data_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_show_property_cmd_vtysh,  "show l3if ifid IFID ip-property (transmit-en|receive-en|route-en|pbr-label|nat-iftype|route-all-pkt|ipv4-ucast-en|ipv4-mcast-en|ipv4-mcast-rpf-en|    ipv4-sa-type|ipv6-ucast-en|ipv6-mcast-en|ipv6-mcast-rpf-en|ipv6-sa-type|vrfid|vrf-en|mtu-en|mtu-exception-en|mtu-size|route-mac-prefix-type|    route-mac-low-8bits|egs-macsa-prefix-type|egs-macsa-low-8bits|egs-mcast-ttl-threshold|arp-excp|dhcp-excp|rpf-check-type|rpf-permit-default|    igs-acl-en|igs-acl-vlan-classid-0|igs-acl-vlan-classid-1|egs-acl-en|egs-acl-vlan-classid-0|egs-acl-vlan-classid-1)",  ctc_cli_l3if_show_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_xlate_cmd_vtysh,  "nexthop add egs-vlan-edit NHID (dsnh-offset OFFSET|) (port GPORT_ID) (""cvlan-edit-type VALUE svlan-edit-type VALUE output-cvid VALUE         output-svid VALUE ((""((swap-vlan (swap-tpid|) (swap-cos|)) | (replace-svlan-cos VALUE ) | (map-svlan-cos ))"")|) (svlan-tpid-index TPID_INDEX|) (cvlan-valid|) (svlan-valid|) (user-vlanptr VLAN_PTR|) (is-leaf|)" ") (service-queue |) (length-adjust-en|) ( horizon-split|) (loop-nhid NHID|) (stats STATS_ID|) (mtu-check-en mtu-size MTU |)",  ctc_cli_nh_add_xlate_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l2_vlan_pas_num_cmd_vtysh,  "show parser l2 ethernet vlan-pas-num",  ctc_cli_parser_show_l2_vlan_pas_num_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_reg_cmd_vtysh,  "reg_check ADDR VALUE MASK ({rand}|)",  ctc_cli_dbg_tool_check_reg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipv4_group_info_cmd_vtysh,  "show ipmc group info",  ctc_cli_gb_ipmc_show_ipv4_group_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_show_debug_cmd_vtysh,  "show debug security (ctc | sys)",  ctc_cli_security_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_add_ipv6_tunnel_cmd_vtysh,  "ipuc ipv6 tunnel add X:X::X:X { vrf VRFID |nhid NHID | tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY         | ip-sa X:X::X:X |  ecmp  | ttl-check | use-outer-ttl | lookup-by-outer | copy-to-cpu | use-outer-info         | service-id SERVICE_ID (logic-port GPHYPORT_ID|) (service-acl-en|) (service-queue-en|) (service-policer-en|)         | stats STATS_ID | discard |}",  ctc_cli_ipuc_add_ipv6_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_show_debug_cmd_vtysh,  "show debug learning-aging (ctc | sys)",  ctc_cli_learning_aging_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_clear_error_cache_cmd_vtysh,  "oam event-cache clear",  ctc_cli_gb_oam_clear_error_cache_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_debug_on_cmd_vtysh,  "debug parser (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_parser_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_mdioxg_clock_cmd_vtysh,  "mdioxg core-clk CORE_CLK mdio-clk MDIO_CLOCK (lchip CHIP_ID|)",  ctc_cli_dbg_tool_mdioxg_clock_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_l2_show_mcast_cmd_vtysh,  "show l2 mcast mac MAC fid FID",  ctc_cli_gb_l2_show_mcast_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_set_keeplive_cmd_vtysh,  "stacking keeplive-group (add|remove) (trunk TRUNK_ID|cpu-port GPORT)",  ctc_cli_stacking_set_keeplive_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_remove_policer_cmd_vtysh,  "qos policer detach (port GPORT (in|out) |flow POLICER_ID|service SEVICE_ID)         (hbwp-en cos-index INDEX |)",  ctc_cli_qos_remove_policer_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_show_debug_cmd_vtysh,  "show debug l2 (fdb|stp) (ctc|sys)",  ctc_cli_l2_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_add_remove_trunk_port_cmd_vtysh,  "stacking trunk TRUNK_ID (add|remove) port GPORT",  ctc_cli_stacking_add_remove_trunk_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_add_or_remove_port_to_default_entry_cmd_vtysh,  "l2 fdb vlan-default-entry (fid FID) (port GPORT_ID |) (add |remove) (port GPORT_ID |nexthop NHPID) (remote-chip|) (service-queue|)",  ctc_cli_l2_add_or_remove_port_to_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_debug_off_cmd_vtysh,  "no debug security (ctc | sys)",  ctc_cli_security_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_storm_ctl_show_global_cfg_cmd_vtysh,  "show security storm-ctl glb-cfg",  ctc_cli_security_storm_ctl_show_global_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_create_or_destroy_l3_phy_if_cmd_vtysh,  "l3if (create | destroy) ifid IFID type phy-if gport GPORT_ID",  ctc_cli_l3if_create_or_destroy_l3_phy_if_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_cpu_mac_stats_cmd_vtysh,  "show stats cpu-mac-stats port GPHYPORT_ID",  ctc_cli_stats_show_cpu_mac_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_vlan_cmd_vtysh,  "vlan remove vlan VLAN_ID",  ctc_cli_vlan_remove_vlan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_cvlan_cos_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-cvlan-cos-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) direction (ingress|egress) " " (cvlan VLAN_ID) " " (ctag-cos COS) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_cvlan_cos_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_qos_use_default_domain_cmd_vtysh,  "qos domain default",  ctc_cli_gb_qos_use_default_domain_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_dm_en_cmd_vtysh,  "local-mep dm (enable | disable) (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_dm_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_mcast_cmd_vtysh,  "nexthop remove mcast NHID",  ctc_cli_nh_remove_mcast_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_offset_cmd_vtysh,  "show ipmc offset (ipv6 | ) INDEX",  ctc_cli_gb_ipmc_show_offset_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_add_ucast_nexthop_cmd_vtysh,  "l2 fdb add ucast-nexthop port GPORT_ID ( (type { basic-nh | untag-nh | bypass-nh | raw-nh | service-queue-nh } ) | )",  ctc_cli_l2_add_ucast_nexthop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_Policing_cmd_vtysh,  "show statistic (share) (policing) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_Policing_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_mac_cmd_vtysh,  "mac MAC_ID (enable|disable) (net_tx_mode NET_TX_MODE|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_debug_on_cmd_vtysh,  "debug chip (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_chip_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_debug_off_cmd_vtysh,  "no debug l3if (ctc | sys)",  ctc_cli_l3if_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_update_ipuc_cmd_vtysh,  "nexthop update ipuc NHID (fwd-attr mac MAC ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" " |fwd2unrov|unrov2fwd mac MAC ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" ")",  ctc_cli_nh_update_ipuc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_remove_rmep_cmd_vtysh,  "remote-mep remove (level LEVEL|) (rmep-id MEP_ID)",  ctc_cli_oam_cfm_remove_rmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_phy_if_cmd_vtysh,  "port GPHYPORT_ID phy-if (enable | disable)",  ctc_cli_port_set_phy_if_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_debug_off_cmd_vtysh,  "no debug alloc (ctc|sys)",  ctc_cli_gb_ftm_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_create_aps_bridge_group_cmd_vtysh,  "aps create aps-bridge group GROUP_ID (aps-type (l2|mpls) |) (working-port GPORT_ID (w-l3if-id INTERFACE_ID|)|next-w-aps  ID) (protection-port GPORT_ID (p-l3if-id INTERFACE_ID|) (physical-isolated|) | next-p-aps ID | raps-en raps-groupid GROUP_ID) (hw-based-aps-en|)",  ctc_cli_aps_create_aps_bridge_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_opf_debug_off_cmd_vtysh,  "no debug (opf|fpa)",  ctc_cli_gb_ftm_opf_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_mac_security_show_mac_limit_cmd_vtysh,  "show security mac-limit (port |vlan) VALUE",  ctc_cli_security_mac_security_show_mac_limit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_debug_on_cmd_vtysh,  "debug qos (ctc | sys) (policer | class| queue) (debug-level {func|param|info|error} |)",  ctc_cli_qos_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l3_global_action_cmd_vtysh,  "show pdu l3pdu global-action (ospf|pim|vrrp|rsvp|rip|bgp|msdp|ldp|layer3-proto|layer4-port|layer4-type)  index INDEX",  ctc_cli_pdu_show_l3_global_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_create_group_cmd_vtysh,  "acl create group GROUP_ID (priority PRIORITY |) direction (ingress|egress|both) (lchip LCHIP|)         (port-bitmap PORT_BITMAP_HIGH PORT_BITMAP_LOW|         global| vlan-class CLASS_ID | port-class CLASS_ID |service-id SERVICE_ID |pbr-class CLASS_ID)",  ctc_cli_acl_create_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_add_del_service_cmd_vtysh,  "qos service (create | destroy) (service-id SERVICE) ",  ctc_cli_qos_add_del_service_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_egs_exp_map_cmd_vtysh,  "qos domain DOMAIN map priority PRIORITY color (green |yellow | red) to exp EXP",  ctc_cli_qos_set_egs_exp_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_sfp_cmd_vtysh,  "scan sfp (enable|disable) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_sfp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_device_type_cmd_vtysh,  "show ptp device-type",  ctc_cli_ptp_show_device_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_lm_cos_cmd_vtysh,  "local-mep lm-cos COS",  ctc_cli_oam_cfm_update_lmep_lm_cos_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto_cmd_vtysh,  "pdu l2pdu global-action l2hdr-proto index INDEX entry-valid ENTRY_VALID (bypass-all BYPASS_ALL|) action-index ACTION_INDEX",  ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_show_mirror_discard_cmd_vtysh,  "show mirror dropped-pkt (port-session|vlan-session|acl-session) direction (ingress|egress)",  ctc_cli_mirror_show_mirror_discard_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_monitor_rlt_cmd_vtysh,  "show port link monitor  (lchip LCHIP|)",  ctc_cli_port_show_monitor_rlt_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto_cmd_vtysh,  "pdu l2pdu classify l2hdr-proto index  INDEX  l2hdr-proto L2_HDR_PROTO",  ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_system_max_frame_size_cmd_vtysh,  "system max-frame-size (size0|size1) BYTES",  ctc_cli_port_set_system_max_frame_size_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_add_ipv6_vlan_class_cmd_vtysh,  "vlan classifier add ipv6 {ip-sa X:X::X:X (mask X:X::X:X|)|ip-da X:X::X:X (mask X:X::X:X|)|mac-sa MAC|mac-da MAC|l4-srcport L4PORT |l4-destport L4PORT|     l4-type (udp|tcp|L4TYPE)} vlan VLAN_ID ((cos COS)|)",  ctc_cli_vlan_add_ipv6_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_set_max_len_cmd_vtysh,  "parser l2 ethernet max-length LEN",  ctc_cli_parser_l2_set_max_len_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_add_mac_vlan_class_cmd_vtysh,  "vlan classifier add mac (macsa MAC| macda MAC)     vlan VLAN_ID ((cos COS)|)",  ctc_cli_vlan_add_mac_vlan_class_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_prbs_cmd_vtysh,  "hss12g prbs hss-id HSS_ID link-num LINK_NUM (internal-en|) (test-pattern TEST_PATTERN|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_prbs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_remove_ipv4_nat_sa_cmd_vtysh,  "ipuc nat-sa remove VRFID A.B.C.D (l4-src-port L4PORT (tcp-port|)|)",  ctc_cli_ipuc_remove_ipv4_nat_sa_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_pkt_rx_print_off_cmd_vtysh,  "no chip-agent packet cpu-rx-print",  ctc_cli_gb_chip_agent_pkt_rx_print_off_cmd_help)

DEFSH (VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK, ctc_cli_common_fast_enter_sdk_mode_cmd_vtysh,  "sdk",  ctc_cli_common_fast_enter_sdk_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_port_queue_drop_cmd_vtysh,  "qos drop port GPHYPORT_ID queue QUEUE_ID     (wtd threshold THRESH1 THRESH2 THRESH3 |     wred min-threshold THRESH1 THRESH2 THRESH3     max-threshold THRESH1 THRESH2 THRESH3     drop-probability PROB1 PROB2 PROB3     (weight WEIGHT|))",  ctc_cli_qos_set_port_queue_drop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_lm_en_cmd_vtysh,  "local-mep lm (enable | disable) (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_lm_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_mtu2_packet_length_cmd_vtysh,  "stats mtu2-pkt-length port GPHYPORT_ID length PKT_LENGTH",  ctc_cli_stats_set_mtu2_packet_length_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_mcast_cmd_vtysh,  "nexthop add mcast NHID group GROUP_ID (mirror |) (stats STATS_ID |)",  ctc_cli_nh_add_mcast_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_epe_flow_chart_cmd_vtysh,  "show bus epe diagram",  ctc_cli_dbg_tool_epe_flow_chart_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_mpls_nexthop_cmd_vtysh,  "nexthop remove mpls NHID",  ctc_cli_nh_remove_mpls_nexthop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_debug_on_cmd_vtysh,  "debug l2 (fdb|stp) (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_l2_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_IpePktProc_cmd_vtysh,  "show statistic (ipe) (ipepktproc) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_IpePktProc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_set_mpls_property_cmd_vtysh,  "l3if ifid IFID mpls-property (mpls-en value VALUE | mpls-label-space value VALUE) ",  ctc_cli_l3if_set_mpls_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_igmp_snooping_cmd_vtysh,  "vlan VLAN_ID igmp-snooping (enable|disable)",  ctc_cli_vlan_set_igmp_snooping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_set_router_mac_cmd_vtysh,  "l3if router-mac MAC",  ctc_cli_l3if_set_router_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_set_vlan_enable_cmd_vtysh,  "mirror source vlan VLAN_ID direction (ingress|egress|both) (enable session SESSION_ID | disable)",  ctc_cli_mirror_set_vlan_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_Mdio_cmd_vtysh,  "show statistic (misc) (mdio) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_Mdio_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_sender_id_tlv_cmd_vtysh,  "oam cfm set sender-id SENDER_ID",  ctc_cli_oam_cfm_set_sender_id_tlv_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_packet_show_packet_info_cmd_vtysh,  "show packet",  ctc_cli_gb_packet_show_packet_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_pcie_cfg_cmd_vtysh,  "write pcie-cfg address ADDR_OFFSET value VALUE",  ctc_cli_dbg_tool_write_pcie_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_show_memory_summary_total_cmd_vtysh,  "show memory summary total",  ctc_cli_common_show_memory_summary_total_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_drvio_show_cmd_vtysh,  "show driver (cache (<1-256>|) |)",  ctc_cli_gb_drvio_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_flush_all_egress_vlan_mapping_cmd_vtysh,  "vlan mapping egress flush-all port GPORT_ID",  ctc_cli_gb_vlan_flush_all_egress_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_hostname_cmd_vtysh,  "hostname NAME",  ctc_cli_common_hostname_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_stpid_cmd_vtysh,  "vlan VLAN_ID stpid STPID",  ctc_cli_vlan_set_stpid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_remove_addr_cmd_vtysh,  "l2 fdb remove mac MAC fid FID",  ctc_cli_l2_remove_addr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_BufRetrv_cmd_vtysh,  "show statistic (bsr) (bufretrv) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_BufRetrv_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_dump_cmd_vtysh,  "show chip-agent",  ctc_cli_gb_chip_agent_dump_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_xgphy_cmd_vtysh,  "write xgphy mdio-bus BUS_NUM phy-addr PHY_ADDR dev DEV reg REG value VALUE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_xgphy_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_debug_cmd_vtysh,  "show debug stats (ctc | sys)",  ctc_cli_stats_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_scl_show_status_cmd_vtysh,  "show scl status",  ctc_cli_gb_scl_show_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_show_vlan_mirror_state_cmd_vtysh,  "show mirror vlan VLAN_ID direction (ingress|egress)",  ctc_cli_mirror_show_vlan_mirror_state_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_ipv6_l2mc_cmd_vtysh,  "vlan VLAN_ID ipv6-l2mc (enable |disable)",  ctc_cli_vlan_set_ipv6_l2mc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_mdio_clock_frequency_cmd_vtysh,  "show chip mdio (gephy|xgphy) freq",  ctc_cli_chip_show_mdio_clock_frequency_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_chip_cmd_vtysh,  "read address ADDR_OFFSET (len LEN|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_chip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_trpt_clear_stats_cmd_vtysh,  "clear stats oam trpt session SESSION gchip GCHIP",  ctc_cli_gb_oam_trpt_clear_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l3_classify_l3pdu_cmd_vtysh,  "show pdu l3pdu classify (layer3-proto|layer4-port) index INDEX",  ctc_cli_pdu_show_l3_classify_l3pdu_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_iloop_cmd_vtysh,  "nexthop remove iloop NHID",  ctc_cli_nh_remove_iloop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_show_debug_cmd_vtysh,  "show debug l3if (ctc | sys)",  ctc_cli_l3if_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_remove_edge_port_cmd_vtysh,  "oam cfm edge-port remove port GPORT_ID",  ctc_cli_oam_cfm_remove_edge_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_reset_global_fwd_cmd_vtysh,  "clear stats ( igs-global-fwd | egs-global-fwd )",  ctc_cli_stats_reset_global_fwd_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_rx_vlan_tpid_cmd_vtysh,  "oam rx-vlan-tpid set tpid-index TPID_INDEX tpid TPID",  ctc_cli_oam_cfm_set_rx_vlan_tpid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_debug_on_cmd_vtysh,  "debug port (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_port_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_set_end_rchip_bitmap_cmd_vtysh,  "stacking (add|remove) (end-chip-point GCHIP | break-point TRUNK_ID)",  ctc_cli_stacking_set_end_rchip_bitmap_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_MacLedDriver_cmd_vtysh,  "show statistic (misc) (macleddriver) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_MacLedDriver_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_mcast_debug_on_cmd_vtysh,  "debug mcast (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_ipmc_mcast_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_set_entry_priority_cmd_vtysh,  "scl entry ENTRY_ID priority PRIORITY",  ctc_cli_scl_set_entry_priority_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_release_internal_port_cmd_vtysh,  "port release internal-port LOCAL_PORT gchip GCHIP type (iloop|eloop|discard|fwd port GPHYPORT_ID)",  ctc_cli_port_release_internal_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_mac_security_set_port_learn_limit_cmd_vtysh,  "security mac-limit port GPHYPORT_ID action (none|fwd|discard|redirect-to-cpu)",  ctc_cli_security_mac_security_set_port_learn_limit_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_debug_sim_chip_model_off_cmd_vtysh,  "no debug chip-agent simulator chip-model (all | {fwd-path | parser-info | key-info | lookup-key | tbl-ptr | discard-type | qmgr-msg | qmgr-hdr | qmgr-linkagg | aging})",  ctc_cli_gb_chip_agent_debug_sim_chip_model_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_port_mac_link_cmd_vtysh,  "show port mac-link",  ctc_cli_port_show_port_mac_link_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_ptp_cmd_vtysh,  "vlan VLAN_ID ptp (enable |disable)",  ctc_cli_vlan_set_ptp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_drain_on_off_cmd_vtysh,  "drain-onoff module MODULE (block BLOCK |) repeat-time TIMES mode MODE ({off-time OFF_MS | on-time ON_MS}|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_drain_on_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_qos_show_qos_metfifo_excp_cmd_vtysh,  "show qos metifo-exception status",  ctc_cli_gb_qos_show_qos_metfifo_excp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_interrupt_show_debug_cmd_vtysh,  "show debug interrupt (ctc | sys)",  ctc_cli_common_interrupt_show_debug_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_gpio_output_cmd_vtysh,  "chip set gpio gpio-id GPIO_ID output VALUE",  ctc_cli_chip_set_gpio_output_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_QMgrDeq_cmd_vtysh,  "show statistic (bsr) (qmgrdeq) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_QMgrDeq_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_remove_rmep_cmd_vtysh,  "remote-mep bfd remove my-discr MY_DISCR         (ip-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL)})",  ctc_cli_oam_bfd_remove_rmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_cfm_show_port_oam_info_cmd_vtysh,  "show oam port-info port GPORT_ID",  ctc_cli_gb_oam_cfm_show_port_oam_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_debug_show_cmd_vtysh,  "show debug ipuc (ctc | sys)",  ctc_cli_ipuc_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_show_global_fwd_cmd_vtysh,  "show stats ( igs-global-fwd | egs-global-fwd )",  ctc_cli_stats_show_global_fwd_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_remove_entry_cmd_vtysh,  "acl remove entry ENTRY_ID",  ctc_cli_acl_remove_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_vlan_ctl_cmd_vtysh,  "port GPHYPORT_ID vlan-ctl (allow-all | drop-all-untagged | drop-all-tagged | drop-all | drop-wo-2tag     | drop-w-2tag | drop-stag | drop-non-stag | drop-only-stag | permit-only-stag |drop-all-ctag |drop-non-ctag     | drop-only-ctag | permit-only-ctag)",  ctc_cli_port_set_vlan_ctl_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_igs_cos_map_cmd_vtysh,  "qos domain DOMAIN map cos COS to priority PRIORITY color (green |yellow | red)",  ctc_cli_qos_set_igs_cos_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_show_nexthop_all_by_type_cmd_vtysh,  "show nexthop all ((mcast|brguc|ipuc|ecmp|mpls|iloop|rspan|ip-tunnel|misc)|) (detail |)",  ctc_cli_gb_nh_show_nexthop_all_by_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_dump_error_cmd_vtysh,  "show chip-agent error",  ctc_cli_gb_chip_agent_dump_error_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_bpe_interlaken_en_cmd_vtysh,  "bpe interlaken (enable|disable)",  ctc_cli_bpe_interlaken_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_write_hss12g_cmd_vtysh,  "write hss12g hss-id HSS_ID (link-tx|link-rx|pll) (link-num LINK_NUM|) addr ADDR_OFFSET value VALUE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_write_hss12g_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_interrupt_debug_on_cmd_vtysh,  "debug interrupt (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_common_interrupt_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_linkagg_show_detail_member_port_cmd_vtysh,  "show linkagg AGG_ID member-ports detail",  ctc_cli_linkagg_show_detail_member_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_mac_led_en_cmd_vtysh,  "chip set mac-led (enable | disable)",  ctc_cli_chip_set_mac_led_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_oam_OamProc_cmd_vtysh,  "show statistic (oam) (oamproc) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_oam_OamProc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_ecmp_cmd_vtysh,  "nexthop remove ecmp ECMPID",  ctc_cli_nh_remove_ecmp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_pcie_mem_cmd_vtysh,  "read pcie-mem address ADDR_OFFSET (LEN|)",  ctc_cli_dbg_tool_read_pcie_mem_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_learning_cmd_vtysh,  "port GPHYPORT_ID learning (enable |disable)",  ctc_cli_port_set_learning_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_trpt_en_cmd_vtysh,  "local-mep trpt (session SESSION) (enable | disable)",  ctc_cli_oam_cfm_update_lmep_trpt_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_map_mepindex_cmd_vtysh,  "aps map aps-brigde GROUP_ID mepindex INDEX",  ctc_cli_aps_map_mepindex_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_update_vlan_mapping_cmd_vtysh,  "vlan mapping update port GPORT_ID ( ({ svid VLAN_ID (to-svid VLAN_ID | ) |cvid VLAN_ID(to-cvid VLAN_ID | ) } ( vrange-grp VRANGE_GROUP_ID | )|) ({ stag-cos COS | ctag-cos COS }|) | ) " "mapping-to ({vpls-learning-dis | maclimit-en}|) (aging|)",  ctc_cli_vlan_update_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_ipv6_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " ipv6-hash-entry " " ip-sa X:X::X:X " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_ipv6_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_cfm_dump_lmep_cmd_vtysh,  "oam cfm dump lmep local-chipid CHIP_ID lmep-index MEP_INDEX",  ctc_cli_gb_oam_cfm_dump_lmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_set_global_action_by_bpdu_cmd_vtysh,  "pdu l2pdu global-action (bpdu|slow-proto|eapol|lldp) (entry-valid ENTRY_VALID) (copy-to-cpu COPY_TO_CPU bypass-all BYPASS_ALL|)  (action-index ACTION_INDEX)",  ctc_cli_pdu_l2_set_global_action_by_bpdu_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_tp_add_chan_cmd_vtysh,  "oam (tp-y1731) lookup-chan add (section-oam (gport GPORT_ID |ifid IFID) | mpls-label LABEL) (master-gchip GCHIP_ID|)",  ctc_cli_oam_tp_add_chan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_run_net_tx_packet_capture_cmd_vtysh,  "run net-tx-packet-capture lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_run_net_tx_packet_capture_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_remove_vlan_vrange_cmd_vtysh,  "vlan range remove MINVID to MAXVID vrange-grp GROUP_ID direction (ingress|egress)",  ctc_cli_gb_vlan_remove_vlan_vrange_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_fid_cmd_vtysh,  "vlan VLAN_ID vrfid VRF_ID",  ctc_cli_vlan_set_fid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_interrupt_show_interrupt_chip_cmd_vtysh,  "show interrupt LCHIP register (group | chip-fatal | chip-normal | func | dma-func | dma-normal | all)",  ctc_cli_gb_interrupt_show_interrupt_chip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_sync_ether_set_cfg_cmd_vtysh,  "sync-ether CHIP clock CLOCK_ID recovered-port LPORT divider DIVIDER output (enable | disable) link-status-detect     (enable | disable)",  ctc_cli_sync_ether_set_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_system_ipg_size_cmd_vtysh,  "system set ipg-index INDEX size SIZE",  ctc_cli_port_set_system_ipg_size_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_add_remove_mip_cmd_vtysh,  "mip (add | remove) (gport GPORT_ID vlan VLAN_ID level LEVEL | mpls-label LABEL) (master-gchip GCHIP_ID|)",  ctc_cli_oam_cfm_add_remove_mip_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_port_set_vlan_range_cmd_vtysh,  "port GPHYPORT_ID vlan-range direction (ingress|egress)  vrange-grp VRANGE_GROUP_ID (enable|disable)",  ctc_cli_gb_port_set_vlan_range_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_drvio_action_cmd_vtysh,  "driver action (print|cache|print-and-cache|disable)",  ctc_cli_gb_drvio_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2mcast_remove_member_cmd_vtysh,  "l2 mcast remove member mac MAC fid FID (port GPORT_ID|nexthop NH_ID) (remote-chip|) ",  ctc_cli_l2mcast_remove_member_cmd_help)

DEFSH (VTYSH_SDK, show_cli_tbl_reg_cmd_vtysh,  "show tbl-id TBL_ID",  show_cli_tbl_reg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_security_ip_src_guard_remove_entry_cmd_vtysh,  "security ip-src-guard remove ipv6 X:X::X:X mac MAC",  ctc_cli_gb_security_ip_src_guard_remove_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_get_access_mode_cmd_vtysh,  "chip get access mode",  ctc_cli_chip_get_access_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l3_set_ip_frag_cmd_vtysh,  "parser l3 ip small-frag-offset OFFSET",  ctc_cli_parser_l3_set_ip_frag_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_update_ip_tunnel_to_unrov_cmd_vtysh,  "nexthop update ip-tunnel NHID fwd2unrov",  ctc_cli_update_ip_tunnel_to_unrov_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_trpt_set_session_en_cmd_vtysh,  "oam trpt set session SESSION gchip GCHIP (enable| disable)",  ctc_cli_gb_oam_trpt_set_session_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l4_set_app_cmd_vtysh,  "parser l4 app ptp-en (enable|disable) ntp-en (enable|disable) bfd-en (enable|disable) capwap-en (enable|disable) state-bits VAL",  ctc_cli_parser_l4_set_app_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_set_default_global_action_cmd_vtysh,  "pdu (l2pdu|l3pdu) default-global-action",  ctc_cli_pdu_set_default_global_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_acl_set_global_cfg_cmd_vtysh,  "acl global-cfg (merge | in-use-mapped-vlan | trill-use-ipv6| arp-use-ipv6 | hash-acl-en |non-ipv4-mpls-use-ipv6|    in-port-service-acl-en | in-vlan-service-acl-en | out-port-service-acl-en| out-vlan-service-acl-en |    hash-mac-key-flag | hash-ipv4-key-flag | priority-bitmap-of-stats) value VALUE ",  ctc_cli_gb_acl_set_global_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_debug_on_cmd_vtysh,  "debug mpls (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_mpls_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_packet_flow_diag_rst_cmd_vtysh,  "show packet flow diagnose result (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_packet_flow_diag_rst_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_emulation_load_config_cmd_vtysh,  "emulation load cfg CONFIG_FILE_NAME",  ctc_cli_dbg_tool_emulation_load_config_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_ip_src_guard_remove_entry_cmd_vtysh,  "security ip-src-guard remove port GPORT_ID {ipv4 A.B.C.D |mac MAC|vlan VLAN_ID (is-svlan |)} ",  ctc_cli_security_ip_src_guard_remove_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_show_tod_intf_cmd_vtysh,  "show ptp CHIP tod",  ctc_cli_ptp_show_tod_intf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_add_addr_cmd_vtysh,  "l2 fdb add mac MAC fid FID (port GPORT_ID|nexthop NHPID)        ((vpls src-port GPORT_ID|pbb) virtual-port VIRTUAL_PORT|)       ({static|remote-dynamic|discard|src-discard|src-discard-tocpu|copy-tocpu|bind-port|raw-pkt-elog-cpu |service-queue | untagged | protocol-entry | system-mac | ptp-entry}|)",  ctc_cli_l2_add_addr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_add_default_entry_cmd_vtysh,  "l2 fdb add vlan-default-entry (fid FID) (port GPORT_ID|) (group GROUP_ID)(use-logic-port|)(unknown-ucast-drop|)(unknown-mcast-drop|)(proto-to-cpu|) ",  ctc_cli_l2_add_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_debug_off_cmd_vtysh,  "no debug nexthop (ctc | sys)",  ctc_cli_nh_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_set_vmac_prefix_cmd_vtysh,  "l3if vmac prefix-type PRE_TYPE prefix-value MAC",  ctc_cli_l3if_set_vmac_prefix_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_check_sgmac_prbs_xfipcs_cmd_vtysh,  "check sgmac-prbs xfipcs lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_check_sgmac_prbs_xfipcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_egs_cos_map_cmd_vtysh,  "qos domain DOMAIN map priority PRIORITY color (green |yellow | red) to cos COS",  ctc_cli_qos_set_egs_cos_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_auto_neg_cmd_vtysh,  "port GPHYPORT_ID auto-neg (enable|disble)",  ctc_cli_port_set_auto_neg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_add_vlan_class_default_entry_cmd_vtysh,  "vlan classifier add-mismatch (mac|ipv4|ipv6) (discard|do-nothing|send-to-cpu)",  ctc_cli_vlan_add_vlan_class_default_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_mtu1_packet_length_cmd_vtysh,  "stats mtu1-pkt-length port GPHYPORT_ID length PKT_LENGTH",  ctc_cli_stats_set_mtu1_packet_length_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_cpu_access_dyn_ram_cmd_vtysh,  "dyn-ram-access repeat-time TIMES step STEP word-mode WORD_MODE op-code OP_CODE (lchip CHIP_ID|)",  ctc_cli_dbg_tool_cpu_access_dyn_ram_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_set_aps_select_cmd_vtysh,  "aps set aps-select group GROUP_ID protect (enable|disable)",  ctc_cli_aps_set_aps_select_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_PtpEngine_cmd_vtysh,  "show statistic (misc) (ptpengine) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_PtpEngine_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_port_info_cmd_vtysh,  "show port GPHYPORT_ID (all | port-en | receive | transmit | bridge | default-vlan | port-cross-connect     | learning-enable |keep-vlan-tag| sub-if | phy-if | vlan-ctl | ingress-vlan-filtering | egress-vlan-filtering |     vlan-domain |ingress-stpid-index|egress-stpid-index | untag-defalt-vlan| dot1q-type|     use-outer-ttl|port-blocking (ucast-flooding|mcast-flooding|bcast-flooding | known-mcast-flooding) |protocol-vlan|vlan-mapping|vlan-classify|ip-src-guard|     mac-en|speed-mode|preamble|min-frame-size|max-frame-size|stretch-mode|pading|ingress-random-log|egress-random-log|     ingress-random-threshold|egress-random-threshold|src-discard|port-check-enable|mac-link-status|ipg | ingress-isolation-id |    egress-isolation-id | isolation-id | reflective-bridge| mux| demux)",  ctc_cli_port_show_port_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_nh_cfg_global_param_cmd_vtysh,  "nexthop (ipmc-logic-replication) value VALUE",  ctc_cli_gb_nh_cfg_global_param_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_hss12g_show_status_cmd_vtysh,  "hss12g status (lchip CHIP_ID|)",  ctc_cli_dbg_tool_hss12g_show_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_set_default_action_cmd_vtysh,  "scl default-action (gport GPORT_ID) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_set_default_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_add_edge_port_cmd_vtysh,  "oam cfm edge-port config port GPORT_ID level LEVEL",  ctc_cli_oam_cfm_add_edge_port_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_bfd_update_lmep_cmd_vtysh,  "local-mep bfd update (ip-bfd my-discr MY_DISCR | mpls-bfd my-discr MY_DISCR | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | mpls-in-label IN_LABEL)})         ((mep-en | cc | cv | csf-en | csf-tx | lm-en) (enable| disable) | tx-cos COS | sf-state SF_STATE | csf-type CSF_TYPE | d-csf clear | lm-cos-type LM_COS_TYPE |lm-cos LM_COS |         diag DIAG | state STATE | my-discr MY_DISCR | nhop NHOP | tx-timer MIN_INTERVAL DETECT_MULT)",  ctc_cli_oam_bfd_update_lmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_storm_ctl_set_config_cmd_vtysh,  "security storm-ctl (port GPHYPORT_ID| vlan VLAN_ID) (enable|disable) mode (pps |bps) (threshold THRESHOLD_NUM)     type(broadcast|known-multicast|unknown-multicast|all-multicast|known-unicast|unknown-unicast|all-unicast)    (discard-to-cpu|)",  ctc_cli_security_storm_ctl_set_config_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_tcam_cmd_vtysh,  "read tcam index INDEX (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_tcam_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_trpt_get_stats_cmd_vtysh,  "show stats oam trpt session SESSION gchip GCHIP",  ctc_cli_gb_oam_trpt_get_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_csf_use_uc_da_cmd_vtysh,  "local-mep csf-use-uc-da (enable | disable) level LEVEL",  ctc_cli_oam_cfm_update_lmep_csf_use_uc_da_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_ipuc_cmd_vtysh,  "nexthop add ipuc NHID (dsnh-offset OFFSET|) (unrov | fwd mac MAC)""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" "(loop-nhid NHID |)" "((protection-path (aps-bridge-id APS_BRIDGE_ID) mac MAC ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" ")|)",  ctc_cli_nh_add_ipuc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_debug_off_cmd_vtysh,  "no debug stacking (ctc|sys)",  ctc_cli_stacking_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_intf_status_cmd_vtysh,  "oam cfm update-intf-status port GPORT_ID intf-status INTF_STATUS",  ctc_cli_oam_cfm_update_intf_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l2_pbb_header_cmd_vtysh,  "show parser l2 pbb header",  ctc_cli_parser_show_l2_pbb_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_debug_off_cmd_vtysh,  "no debug chip (ctc | sys)",  ctc_cli_chip_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_l2_max_len_cmd_vtysh,  "show parser l2 ethernet max-length",  ctc_cli_parser_show_l2_max_len_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_set_port_enable_cmd_vtysh,  "mirror source port GPHYPORT_ID direction (ingress | egress | both) (enable session SESSION_ID | disable)",  ctc_cli_mirror_set_port_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_l2_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " l2-hash-entry " " mac-sa MAC mac-da MAC " " gport GPORT_ID " " vlan-id VLAN_ID " " cos COS" " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_l2_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_sub_if_cmd_vtysh,  "port GPHYPORT_ID sub-if (enable | disable)",  ctc_cli_port_set_sub_if_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_show_vmac_prefix_cmd_vtysh,  "show l3if vmac prefix-type PRE_TYPE",  ctc_cli_l3if_show_vmac_prefix_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_IpeLkupMgr_cmd_vtysh,  "show statistic (ipe) (ipelkupmgr) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_IpeLkupMgr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_remove_vlan_mapping_cmd_vtysh,  "vlan mapping remove port GPORT_ID ( ({ svid VLAN_ID (to-svid VLAN_ID | ) |cvid VLAN_ID(to-cvid VLAN_ID | ) } ( vrange-grp VRANGE_GROUP_ID | )|) ({ stag-cos COS | ctag-cos COS }|) | )",  ctc_cli_vlan_remove_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_port_hash_entry_cmd_vtysh,  "scl add (group GROUP_ID|) (entry ENTRY_ID)" " port-hash-entry " "(gport GPORT_ID | class-id CLASS_ID) direction (ingress|egress) " " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_port_hash_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_interrupt_debug_off_cmd_vtysh,  "no debug interrupt (ctc | sys)",  ctc_cli_common_interrupt_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_unmapping_l3type_cmd_vtysh,  "parser l2 mapping-l3-type unmapping index INDEX ",  ctc_cli_parser_l2_unmapping_l3type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mirror_debug_off_cmd_vtysh,  "no debug mirror (ctc | sys)",  ctc_cli_mirror_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_swap_mpls_tunnel_cmd_vtysh,  "nexthop mpls-tunnel swap (source TUNNEL_ID) (destination TUNNEL_ID)",  ctc_cli_nh_swap_mpls_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_oam_OamParser_cmd_vtysh,  "show statistic (oam) (oamparser) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_oam_OamParser_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_lmep_ccm_enable_cmd_vtysh,  "local-mep ccm (enable| disable) (level LEVEL|)",  ctc_cli_oam_cfm_update_lmep_ccm_enable_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_enter_cmodel_mode_cmd_vtysh,  "enter cmodle mode",  ctc_cli_common_enter_cmodel_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_port_en_cmd_vtysh,  "port GPHYPORT_ID port-en (enable | disable)",  ctc_cli_port_set_port_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_packet_tx_uc_cmd_vtysh,  "packet tx ucast (dest-gport GPHYPORT_ID | linkagg AGG_ID (hash HASH|)) (bypass|bridge vlan VLANID) (length LEN) (dma)",  ctc_cli_packet_tx_uc_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_create_and_remove_raps_group_cmd_vtysh,  "aps aps-bridge raps (create|remove) (group-id) GROUP_ID",  ctc_cli_aps_create_and_remove_raps_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_add_ipv4_entry_cmd_vtysh,  "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|)" " ipv4-entry ({" " cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI |       svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI " " | mac-sa MAC MASK | mac-da MAC MASK |       layer2-type L2_TYPE MASK | layer3-type L3_TYPE MASK " "|ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D|      sender-ip A.B.C.D A.B.C.D | target-ip A.B.C.D A.B.C.D " "|l4-protocol L4_PROTOCOL MASK {l4-src-port L4_SRC_PORT MASK|l4-dst-port L4_DST_PORT MASK|     icmp-type ICMP_TYPE MASK| icmp-code ICMP_CODE MASK | igmp-type IGMP_TYPE  MASK} " "|dscp DSCP MASK | eth-type ETH_TYPE MASK | l4-type L4_TYPE MASK | ip-option IP_OPTION | ip-frag IP_FRAG | prec IP_PREC MASK " " }|)" " ( " "ingress-action ({ priority-and-color PRIORITY COLOR | service-id SERVICE_ID (service-acl-en|)     (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | redirect NHID | vpls (learning-en |) (mac-limit-en |) |     logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf |     user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) |     aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " | " "egress-action ({ " " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|)                 (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|)     | discard | stats STATS_ID | aging " "}|)" " ) ",  ctc_cli_scl_add_ipv4_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l3_type_cmd_vtysh,  "parser l3-type (ipv4 |ipv6 |mpls |mpls-mcast |arp |    rarp |eapol | ethoam | slow-proto | cmac | ptp) (enable | disable)",  ctc_cli_parser_l3_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_get_vlan_vrange_cmd_vtysh,  "show vrange vrange-grp GROUP_ID direction (ingress|egress)",  ctc_cli_gb_vlan_get_vlan_vrange_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_rspan_cmd_vtysh,  "nexthop remove rspan NHID",  ctc_cli_nh_remove_rspan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_logic_port_mapping_srcport_cmd_vtysh,  "learning virtual-port PORT mapping port GPORT_ID ",  ctc_cli_learning_aging_logic_port_mapping_srcport_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_pkt_rx_print_cmd_vtysh,  "chip-agent packet cpu-rx-print",  ctc_cli_gb_chip_agent_pkt_rx_print_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2mcast_add_addr_cmd_vtysh,  "l2 mcast add group GROUP_ID mac MAC fid FID ({protocol-exp|keep-empty|ptp-entry}|)",  ctc_cli_l2mcast_add_addr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_set_sim_action_cmd_vtysh,  "chip-agent simulator-log (start (server-print|client-print|client-file) | stop)",  ctc_cli_gb_chip_agent_set_sim_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_show_ecmp_hash_cmd_vtysh,  "show parser ecmp-hash{ip {l3protocol|dscp|flow-label|ipsa|ipda}|l4 {src-port|dst-port|gre-key}|        mpls {m-l3protocol|m-dscp|m-flow-label|m-ipsa|m-ipda}|fcoe {sid|did}|trill         {inner-vlan|ingress-nickname|egress-nickname} | l2 ({cos|macsa|macda}|) }",  ctc_cli_parser_show_ecmp_hash_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_cpumac_en_cmd_vtysh,  "port cpumac (enable | disable)",  ctc_cli_port_set_cpumac_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_igs_exp_map_cmd_vtysh,  "qos domain DOMAIN map exp EXP to priority PRIORITY color (green |yellow | red)",  ctc_cli_qos_set_igs_exp_map_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_all_mac_en_cmd_vtysh,  "port all mac (enable |disable)",  ctc_cli_port_set_all_mac_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_gephy_scan_special_reg_cmd_vtysh,  "chip set phy-scan special-reg gephy reg REG ((enable bit BIT_CTL intr (enable | disable))|disable)",  ctc_cli_chip_set_gephy_scan_special_reg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_stacking_show_trunk_info_cmd_vtysh,  "show stacking trunk trunk-id TRUNID info",  ctc_cli_gb_stacking_show_trunk_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24_cmd_vtysh,  "show pdu l2pdu classify macda-24bit index INDEX",  ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_debug_show_cmd_vtysh,  "dbg debug status",  ctc_cli_dbg_tool_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_device_type_cmd_vtysh,  "ptp device-type ( none | oc | bc | e2e-tc | p2p-tc) ",  ctc_cli_ptp_set_device_type_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_mpls_pop_nexthop_cmd_vtysh,  "nexthop add mpls NHID (dsnh-offset OFFSET|) pop (unrov|fwd (""payload-op op-route    ((pipe|short-pipe|uniform|)|) (sequence-number SQN_INDEX |) (mac MAC|) ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" ") (service-queue|))",  ctc_cli_nh_add_mpls_pop_nexthop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_PbCtl_cmd_vtysh,  "show statistic (misc) (pbctl) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_PbCtl_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_set_access_mode_cmd_vtysh,  "chip set access mode (PCI|I2C)",  ctc_cli_chip_set_access_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_remove_misc_l2hdr_cmd_vtysh,  "nexthop remove misc NHID",  ctc_cli_nh_remove_misc_l2hdr_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_debug_off_cmd_vtysh,  "no debug qos (ctc | sys) (policer | class| queue)",  ctc_cli_qos_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_write_gephy_interface_cmd_vtysh,  "chip write gephy port GPHYPORT_ID reg REG val VAL",  ctc_cli_chip_write_gephy_interface_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_srcdiscard_cmd_vtysh,  "port GPHYPORT_ID src-discard (enable | disable)",  ctc_cli_port_set_srcdiscard_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_debug_off_cmd_vtysh,  "no debug ipmc (ctc|sys)",  ctc_cli_ipmc_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_cmd_vtysh,  "show pdu l2pdu classify macda-48bit index INDEX",  ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_debug_off_cmd_vtysh,  "no debug ipuc (ctc | sys)",  ctc_cli_ipuc_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_system_max_frame_size_cmd_vtysh,  "show system (max-frame-size|none)",  ctc_cli_port_show_system_max_frame_size_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_bpe_debug_on_cmd_vtysh,  "debug bpe (ctc|sys) (debug-level {func|param|info|error} |)",  ctc_cli_bpe_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_show_header_cmd_vtysh,  "show stacking header info",  ctc_cli_stacking_show_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_cfm_dump_maid_cmd_vtysh,  "oam cfm dump maid local-chipid CHIP_ID maid-index MAID_INDEX",  ctc_cli_gb_oam_cfm_dump_maid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ipmc_show_ipv6_group_info_cmd_vtysh,  "show ipmc ipv6 group info",  ctc_cli_gb_ipmc_show_ipv6_group_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_stretch_mode_cmd_vtysh,  "port GPHYPORT_ID stretch-mode (enable|disable)",  ctc_cli_port_set_stretch_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_l2_show_fdb_status_cmd_vtysh,  "show l2 fdb status",  ctc_cli_gb_l2_show_fdb_status_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_uninstall_entry_cmd_vtysh,  "scl uninstall entry ENTRY_ID",  ctc_cli_scl_uninstall_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_chip_agent_debug_simulator_on_cmd_vtysh,  "debug chip-agent simulator packet-io",  ctc_cli_gb_chip_agent_debug_simulator_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_show_router_mac_cmd_vtysh,  "show l3if router-mac",  ctc_cli_l3if_show_router_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_scan_xgphy_sp_para_cmd_vtysh,  "scan xgphy-special-para dev DEV reg REG bit-ctrl BIT_CTRL (intr-en|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_scan_xgphy_sp_para_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_read_xgphy_interface_cmd_vtysh,  "chip read xgphy port GPHYPORT_ID dev DEV_NO reg REG",  ctc_cli_chip_read_xgphy_interface_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_debug_on_cmd_vtysh,  "debug acl (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_acl_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_set_linkagg_hash_cmd_vtysh,  "parser linkagg-hash{l2 ({vlan|cos|ethertype|double-vlan|macsa|macda}|) | ip ({l3protocol|ipsa|ipda}|)|l4 ({src-port|dst-port|gre-key}|) |    pbb ({b-vlan|b-cos|b-macsa|b-macda }|) |mpls ({m-l3protocol|m-ipsa|m-ipda}|)|fcoe ({sid|did}|)|trill ({ingress-nickname|egress-nickname}|)}",  ctc_cli_parser_set_linkagg_hash_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_run_sgmac_prbs_xauipcs_cmd_vtysh,  "run sgmac-prbs xauipcs lport PORTID (lchip CHIP_ID|) ",  ctc_cli_dbg_tool_run_sgmac_prbs_xauipcs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l3if_set_property_cmd_vtysh,  "l3if ifid IFID ip-property (transmit-en|receive-en|route-en|pbr-label|nat-iftype|route-all-pkt|ipv4-ucast-en|ipv4-mcast-en|ipv4-mcast-rpf-en|    ipv4-sa-type|ipv6-ucast-en|ipv6-mcast-en|ipv6-mcast-rpf-en|ipv6-sa-type|vrfid|vrf-en|mtu-en|mtu-exception-en|mtu-size|route-mac-prefix-type|    route-mac-low-8bits|egs-macsa-prefix-type|egs-macsa-low-8bits|egs-mcast-ttl-threshold|arp-excp|dhcp-excp|rpf-check-type|rpf-permit-default|    igs-acl-en|igs-acl-vlan-classid-0|igs-acl-vlan-classid-1|egs-acl-en|egs-acl-vlan-classid-0|egs-acl-vlan-classid-1|stats) value VALUE",  ctc_cli_l3if_set_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_hss6g_cmd_vtysh,  "read hss6g address ADDR_OFFSET (len LEN|) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_hss6g_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_add_misc_cpu_reason_cmd_vtysh,  "nexthop add misc cpu-reason NHID reason-id ID",  ctc_cli_nh_add_misc_cpu_reason_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_HashDsMem_cmd_vtysh,  "show statistic (share) (hashdsmem) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_HashDsMem_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_remove_ipv4_group_cmd_vtysh,  "ipmc remove group group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (ip-based-l2mc|)",  ctc_cli_ipmc_remove_ipv4_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_add_vpls_ilm_cmd_vtysh,  "mpls ilm add vpls-vc LABEL NHID PWID (vpls-port-type | ) (control-word | ) (oam |) (SERVICE_ID | stats STATS_ID |)",  ctc_cli_mpls_add_vpls_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_update_ip_tunnel_cmd_vtysh,  "nexthop update ip-tunnel NHID ( fwd-attr|unrov2fwd) (""mac MAC ""(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)" "      type (tunnel-v4 (ip6in4|ip4in4 (ip-bfd|)|ip6to4 (manual-ipsa|) | 6rd (sp-prefix-len SPPREFIXLEN |) (v4-sa-masklen SAMASKLEN|) (v4-da-masklen DAMASKLEN|) |isatap|grein4 protocol PROTOCOL (gre-key GREKEY|) (mirror|))     ({ipsa A.B.C.D|ipda A.B.C.D}|) ((copy-dont-frag|dont-frag DONT_FRAG)|) | tunnel-v6 (ip4in6 (ip-bfd|)|ip6in6|grein6 protocol PROTOCOL (gre-key GREKEY|) (mirror|))      ipsa X:X::X:X ipda X:X::X:X | ivi (6to4 | 4to6 v6-prefix-addr X:X::X:X) prefix-len LENGTH (prefix-obey-rfc6052|) |      nat new-ipda A.B.C.D (new-l4dstport L4PORT|))      (re-route|) (dscp DSCP dscp-select SEL_MODE ttl TTL (map-ttl|)|) (mtu-check-en mtu-size MTU |) (flow-label-mode MODE (flow-label-value VALUE | ) |)" ")",  ctc_cli_update_ip_tunnel_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipuc_add_ipv4_nat_sa_cmd_vtysh,  "ipuc nat-sa add VRFID A.B.C.D (l4-src-port L4PORT (tcp-port|)|) (new-ipsa A.B.C.D) (new-l4-src-port L4PORT|)",  ctc_cli_ipuc_add_ipv4_nat_sa_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_port_property_cmd_vtysh,  "show port GPHYPORT_ID property (all | route-en |pkt-tag-high-priority | hw-learn |    ipv6-lookup | ipv4-lookup | use-default-lookup | ipv6-to-mac-key | ipv4-to-mac-key | trill | discard-non-tril | discard-tril |     reflective-bridge | fcoe | fcoe-rpf | replace-stag-cos | replace-ctag-cos | replace-dscp |    l3pdu-arp-action | l3pdu-dhcp-action | tunnel-rpf-check | ptp | rpf-type | is-leaf | priority-tag | default-pcp |     default-dei | raw-packet-type | pvlan-type |qos-trust|auto-neg|link-intr|mac-ts|linkscan-en|aps-failover|linkagg-failover)",  ctc_cli_port_show_port_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_parser_l2_set_pbb_header_cmd_vtysh,  "parser l2 pbb header nca-check-en (enable|disable) outer-vlan-is-cvlan (enable|disable) vlan-pas-num VAL",  ctc_cli_parser_l2_set_pbb_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_nh_show_stats_cmd_vtysh,  "show nexthop stats (nhid NHID|mpls-tunnel TUNNEL_ID) (protection-path|)",  ctc_cli_nh_show_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_clear_rmep_dcsf_cmd_vtysh,  "remote-mep d-csf clear (level LEVEL|) (rmep-id MEP_ID)",  ctc_cli_oam_cfm_clear_rmep_dcsf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_read_temp_cmd_vtysh,  "read temp (lchip CHIP_ID|)",  ctc_cli_dbg_tool_read_temp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bridge_header_cmd_vtysh,  "show pkthdr (ipe-fwd|(epe-hdr-adjust (index INDEX|))|cpu-bufstore|iloop-bufstore|eloop-bufstore|oam-bufstore|(bufretrieve (chanid CHAD_ID|))|start-capture-oam-engine|(oam-engine (index INDEX|))) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bridge_header_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_add_ipv6_group_cmd_vtysh,  "ipmc ipv6 add group GROUPID group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID ({ttl-check |rpf-check |rpf-fail-to-cpu |stats STATS_ID | ip-based-l2mc | ptp-entry | redirect-tocpu } | )",  ctc_cli_ipmc_add_ipv6_group_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_show_opf_alloc_info_cmd_vtysh,  "show opf alloc type TYPE index INDEX",  ctc_cli_gb_ftm_show_opf_alloc_info_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_arp_dhcp_excp_cmd_vtysh,  "vlan VLAN_ID (arp|dhcp) action (copy-to-cpu|forward|redirect-to-cpu|discard)",  ctc_cli_vlan_set_arp_dhcp_excp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_update_rmep_csf_en_cmd_vtysh,  "remote-mep csf (enable| disable) (level LEVEL|) (rmep-id MEP_ID)",  ctc_cli_oam_cfm_update_rmep_csf_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_add_normal_ilm_cmd_vtysh,  "mpls ilm add space SPACEID label LABEL ( nexthop-id NHID|)  ((uniform | short-pipe | pipe )|) ((pop (decap |) (interface-label-space SPACEID|))|) (oam | ) ((aps-select GROUP_ID (working-path|protection-path) | stats STATS_ID)|) (trust-outer-exp |)(ipv6 | )",  ctc_cli_mpls_add_normal_ilm_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_set_tx_vlan_tpid_cmd_vtysh,  "oam tx-vlan-tpid set tpid-index TPID_INDEX tpid TPID",  ctc_cli_oam_cfm_set_tx_vlan_tpid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_pading_cmd_vtysh,  "port GPHYPORT_ID pading (enable | disable)",  ctc_cli_port_set_pading_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_add_vlan_mapping_cmd_vtysh,  "vlan mapping add port GPORT_ID ( ({ svid VLAN_ID (to-svid VLAN_ID | ) |cvid VLAN_ID(to-cvid VLAN_ID | ) } ( vrange-grp VRANGE_GROUP_ID | )|) ({ stag-cos COS | ctag-cos COS }|) | ) " "mapping-to {new-cvid VLAN_ID|new-scos COS|new-ccos COS|service-id SERVICEID ((ingress|egress)|) (service-acl-en|) (service-policer-en|) (service-queue-en|)|new-svid VLAN_ID|logic-port GPHYPORT_ID (logic-port-type |)" "|aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path)|(fid FID (vpls-fid ({vpls-learning-dis | maclimit-en | is-leaf}|)|)|nexthop NHID (vpws-nh (is-leaf |)|vlan-switching|))" "|stag-op TAG_OP  (sl-svid TAG_SL|) (sl-scos TAG_SL|) |ctag-op TAG_OP  (sl-cvid TAG_SL |) (sl-ccos TAG_SL|) |user-vlanptr VLAN_PTR} (stats STATS_ID|) (aging|)",  ctc_cli_vlan_add_vlan_mapping_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_uninstall_entry_cmd_vtysh,  "acl uninstall entry ENTRY_ID",  ctc_cli_acl_uninstall_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_update_ipv4_rpf_cmd_vtysh,  "ipmc update rpf group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID {rpf-if0 IFID0|rpf-if1 IFID1|rpf-if2 IFID2|rpf-if3 IFID3}(more-rpf-to-cpu|)",  ctc_cli_ipmc_update_ipv4_rpf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_set_port_tag_cmd_vtysh,  "vlan VLAN_ID port GPHYPORT_ID (tagged|untagged)",  ctc_cli_gb_vlan_set_port_tag_cmd_help)

DEFSH (VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK|VTYSH_SDK, ctc_cli_common_fast_enter_cmodel_mode_cmd_vtysh,  "cmodel",  ctc_cli_common_fast_enter_cmodel_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_integrity_check_cmd_vtysh,  "integrity-check (chip CHIP_ID|) SRC_FILE DEST_FILE ",  ctc_cli_dbg_tool_integrity_check_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_ftm_opf_debug_on_cmd_vtysh,  "debug (opf|fpa) (debug-level {func|param|info|error} |)",  ctc_cli_gb_ftm_opf_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_TcamDs_cmd_vtysh,  "show statistic (share) (tcamds) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_TcamDs_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_interrupt_show_interrupt_sdk_cmd_vtysh,  "show interrupt sdk",  ctc_cli_gb_interrupt_show_interrupt_sdk_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_replace_dscp_cmd_vtysh,  "vlan VLAN_ID replace-dscp (enable|disable)",  ctc_cli_vlan_set_replace_dscp_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24_cmd_vtysh,  "pdu l2pdu classify macda-24bit index INDEX macda MACDA (mask MASK|)",  ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_set_receive_cmd_vtysh,  "vlan VLAN_ID receive (enable |disable)",  ctc_cli_vlan_set_receive_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_bsr_QMgrEnq_cmd_vtysh,  "show statistic (bsr) (qmgrenq) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_bsr_QMgrEnq_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_vlan_show_dir_property_cmd_vtysh,  "show vlan VLAN_ID dir-property     ( all | acl-en | acl-vlan-classid | acl-routed-pkt-only )     direction ( ingress | egress)",  ctc_cli_vlan_show_dir_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_learning_aging_show_port_nhid_cmd_vtysh,  "show learning port-nhid mapping",  ctc_cli_learning_aging_show_port_nhid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_scl_debug_on_cmd_vtysh,  "debug scl (ctc | sys) (debug-level {func|param|info|error} |)",  ctc_cli_scl_debug_on_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_set_port_action_cmd_vtysh,  "pdu l2pdu port-action port PORT action-index ACTION_INDEX action-type (forward|copy-to-cpu|redirect-to-cpu|discard)",  ctc_cli_pdu_l2_set_port_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_get_multi_entry_cmd_vtysh,  "show acl group GROUP_ID size ENTRY_SIZE ",  ctc_cli_acl_get_multi_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_show_l2_global_action_cmd_vtysh,  "show pdu l2pdu global-action     (macda-48bit index INDEX|macda-24bit index INDEX|l2hdr-proto index INDEX|bpdu|slow-proto|eapol|lldp|l3type index INDEX)",  ctc_cli_pdu_show_l2_global_action_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_Qsgmii_cmd_vtysh,  "show statistic (misc) (qsgmii) (index INDEX) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_Qsgmii_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_chip_show_chip_clock_cmd_vtysh,  "show chip clock frequence",  ctc_cli_chip_show_chip_clock_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_set_big_interval_to_cpu_cmd_vtysh,  "oam cfm big-ccm-interval INTERVAL",  ctc_cli_oam_set_big_interval_to_cpu_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_sync_intf_cmd_vtysh,  "ptp CHIP sync (mode ( input | output)) (lock (enable | disable)) (accuracy ACCURACY) (sync-clock-hz SYNC_CLOCK_HZ)     (sync-pulse-hz SYNC_PULSE_HZ) (sync-pulse-duty SYNC_PULSE_DUTY) (epoch EPOCH)",  ctc_cli_ptp_set_sync_intf_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch_cmd_vtysh,  "remote-mep src-mac-mismatch clear level LEVEL rmep-id MEP_ID",  ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_misc_PciExpCore_cmd_vtysh,  "show statistic (misc) (pciexpcore) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_misc_PciExpCore_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_dma_mpool_dbg_cmd_vtysh,  "show dma mpool state",  ctc_cli_gb_dma_mpool_dbg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_qos_set_phb_configure_cmd_vtysh,  "qos map priority PRIORITY to phb PHB",  ctc_cli_qos_set_phb_configure_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_remove_trunk_cmd_vtysh,  "stacking destroy trunk TRUNK_ID",  ctc_cli_stacking_remove_trunk_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_vlan_remove_protocol_vlan_cmd_vtysh,  "vlan classifier remove protocol ether-type ETHER_TYPE",  ctc_cli_gb_vlan_remove_protocol_vlan_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_port_dir_property_cmd_vtysh,  "show port GPHYPORT_ID dir-property (all     | acl-en | acl-port-classid | acl-port-classid-0 | acl-port-classid-1 | acl-ipv4-force-mac     | acl-ipv6-force-mac | acl-force-use-ipv6 | acl-use-class | service-acl-en      | qos-domain | policer-valid | stag-tpid-index | random-log-rate) direction ( ingress | egress)",  ctc_cli_port_show_port_dir_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_epe_NetTx_cmd_vtysh,  "show statistic (epe) (nettx) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_epe_NetTx_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_acl_remove_all_entry_cmd_vtysh,  "acl remove-all group GROUP_ID",  ctc_cli_acl_remove_all_entry_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stats_set_global_cfg_cmd_vtysh,  "stats ( saturate | hold | clear-after-read ) (igs-port|egs-port|igs-global-fwd|egs-global-fwd|fwd|gmac|sgmac|xgmac|cpumac) (enable|disable)",  ctc_cli_stats_set_global_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_dma_show_stats_cmd_vtysh,  "show dma rx stats",  ctc_cli_gb_dma_show_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_set_global_property_cmd_vtysh,  "ptp global-property (ucast-en | sync-cpu-en | signaling-cpu-en | management-cpu-en |      delay-request-en | port-based-en | cache-aging-time | interface-selected) value VALUE",  ctc_cli_ptp_set_global_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ipmc_remove_ipv4_member_cmd_vtysh,  "ipmc remove member group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID ((""(sub-if gport GPORT_ID vlanid VLAN_ID | vlan-if vlanid VLAN_ID gport GPORT_ID ({vlan-port}|) | phy-if gport GPORT_ID )" "  | (ip-based-l2mc gport GPORT_ID)) (remote-chip|) | ""nexthop-id NHID" ")",  ctc_cli_ipmc_remove_ipv4_member_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_set_mac_en_cmd_vtysh,  "port GPHYPORT_ID mac (enable | disable)",  ctc_cli_port_set_mac_en_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_share_DsAging_cmd_vtysh,  "show statistic (share) (dsaging) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_share_DsAging_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_dbg_tool_show_ipe_IpeHdrAdj_cmd_vtysh,  "show statistic (ipe) (ipehdradj) (lchip CHIP_ID|)",  ctc_cli_dbg_tool_show_ipe_IpeHdrAdj_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_ptp_adjust_clock_offset_cmd_vtysh,  "ptp CHIP adjust-offset (negative |) (seconds SECOND) (nanoseconds NS) (nano-nanoseconds NNS)",  ctc_cli_ptp_adjust_clock_offset_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_pdu_l2_set_global_action_by_macda_cmd_vtysh,  "pdu l2pdu global-action macda-48bit index INDEX entry-valid ENTRY_VALID (bypass-all BYPASS_ALL|) action-index ACTION_INDEX",  ctc_cli_pdu_l2_set_global_action_by_macda_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_enter_internal_debug_mode_cmd_vtysh,  "enter internal mode",  ctc_cli_common_enter_internal_debug_mode_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_aps_debug_off_cmd_vtysh,  "no debug aps (ctc | sys)",  ctc_cli_aps_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_remove_ucast_nexthop_cmd_vtysh,  "l2 fdb remove ucast-nexthop port GPORT_ID",  ctc_cli_l2_remove_ucast_nexthop_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_show_memory_detail_cmd_vtysh,  "show memory pool detail",  ctc_cli_common_show_memory_detail_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_show_stp_state_by_port_and_stpid_cmd_vtysh,  "show stp state port GPHYPORT_ID stpid STP_ID",  ctc_cli_l2_show_stp_state_by_port_and_stpid_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_common_debug_show_cmd_vtysh,  "show debug on",  ctc_cli_common_debug_show_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_oam_cfm_add_lmep_cmd_vtysh,  "local-mep add (mep-id MEP_ID) (meg-id MEG_ID | md-name MD_NAME ma-name NA_NAME) ({mep-en | ccm-interval CCM_INTERVAL | dm-enable| mep-on-cpu|aps-enable | lm-en LM_TYPE LM_COS_TYPE LM_COS | csf-en CSF_TYPE}|)    (eth-oam { level LEVEL | is-p2p | seq-num-en | is-vpls-oam | addr-share-mode | tx-if-status | tx-port-status | tx-sender-id| tpid-index TPID_INDEX| ccm-gport-id CCM_GPORT_ID | up-mep-tx-group-id TX_GROUP_ID } |    tp-y1731-oam { protection-path | nhid NHID | mpls-ttl MPLS_TTL | without-gal } ) (mep-index IDX|)",  ctc_cli_oam_cfm_add_lmep_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_port_show_port_mac_cmd_vtysh,  "show port GPHYPORT_ID port-mac",  ctc_cli_port_show_port_mac_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_mpls_add_stats_cmd_vtysh,  "mpls add stats STATS_ID SPACEID LABEL",  ctc_cli_mpls_add_stats_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_l2_debug_off_cmd_vtysh,  "no debug l2 (fdb|stp) (ctc|sys)",  ctc_cli_l2_debug_off_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_stacking_show_property_cmd_vtysh,  "show stacking (break-en | end-chip-bitmap| full-mesh-en)",  ctc_cli_stacking_show_property_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_security_storm_ctl_show_cfg_cmd_vtysh,  "show security storm-ctl (port GPHYPORT_ID| vlan VLAN_ID)     (broadcast|known-multicast|unknown-multicast|all-multicast|known-unicast|unknown-unicast|all-unicast)",  ctc_cli_security_storm_ctl_show_cfg_cmd_help)

DEFSH (VTYSH_SDK, ctc_cli_gb_oam_cfm_dump_rmep_cmd_vtysh,  "oam cfm dump rmep local-chipid CHIP_ID rmep-index MEP_INDEX",  ctc_cli_gb_oam_cfm_dump_rmep_cmd_help)

void
vtysh_init_cmd ()
{
  install_element (1, &ctc_cli_gb_oam_update_update_timer_enable_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_misc_l2hdr_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l4_type_cmd_vtysh);
  install_element (1, &ctc_cli_stats_reset_global_fwd_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_gpio_mode_cmd_vtysh);
  install_element (1, &humber_cli_aps_set_bridge_cmd_vtysh);
  install_element (1, &ctc_cli_chip_write_gephy_interface_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l4_flex_cmd_vtysh);
  install_element (1, &ctc_cli_packet_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_port_group_class_priority_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_i2c_scan_en_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_run_net_tx_packet_capture_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_gb_pdu_show_l2pdu_port_action_cmd_vtysh);
  install_element (1, &ctc_cli_add_ip_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_global_property_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_set_header_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_port_queue_class_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_tod_intf_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_clear_cmd_vtysh);
  install_element (1, &ctc_cli_aps_create_aps_bridge_group_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_security_set_port_security_cmd_vtysh);
  install_element (1, &ctc_cli_parser_global_set_hash_type_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_show_port_nhid_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_Ds_TCAM_cmd_vtysh);
  install_element (1, &ctc_cli_security_mac_security_set_vlan_learn_limit_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_set_vmac_prefix_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_run_capture_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_pcie_io_cmd_vtysh);
  install_element (1, &ctc_cli_show_dma_desc_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_logic_port_mapping_srcport_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_vlan_class_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_bpe_get_interlaken_en_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_drain_on_off_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_clear_sync_intf_rx_code_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l2_l2flex_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_add_ipv6_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_l2_get_nhid_by_logic_port_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_lm_cos_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_mtu1_packet_length_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_egs_dscp_map_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_set_service_queue_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_QMgrEnq_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_trpt_clear_stats_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l3if_action_action_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_discard_same_mac_or_ip_cmd_vtysh);
  install_element (1, &ctc_cli_acl_copy_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_integrity_check_cmd_vtysh);
  install_element (1, &ctc_cli_l2_flush_fdb_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_add_default_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_learning_cmd_vtysh);
  install_element (1, &ctc_cli_bpe_set_mux_demux_cmd_vtysh);
  install_element (1, &ctc_cli_aps_set_aps_bridge_group_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_remove_lmep_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_auto_neg_cmd_vtysh);
  install_element (1, &ctc_cli_chip_get_access_mode_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_set_ipv4_force_route_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_adjust_delay_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_xgphy_cmd_vtysh);
  install_element (1, &ctc_cli_gb_acl_show_tcp_flags_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_property_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_log_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_clear_lmep_rdi_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_add_edge_port_cmd_vtysh);
  install_element (1, &ctc_cli_gb_port_show_port_info_cmd_vtysh);
  install_element (1, &ctc_cli_chip_read_xgphy_interface_cmd_vtysh);
  install_element (1, &ctc_cli_dma_mem_dump_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_rmep_sf_state_cmd_vtysh);
  install_element (1, &ctc_cli_aps_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_gb_security_ip_src_guard_add_entry_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_set_global_action_by_macda_low24_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_opf_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_ipv4_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_l2_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_common_enter_cmodel_mode_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_captured_ts_cmd_vtysh);
  install_element (1, &ctc_cli_acl_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_ipv4_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_mux_demux_cmd_vtysh);
  install_element (1, &ctc_cli_chip_read_i2c_buffer_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_OobFc_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_svlan_cos_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_add_ipv6_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_set_max_len_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_create_vlan_vrange_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l4_set_flex_cmd_vtysh);
  install_element (1, &ctc_cli_l2_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_add_ipv4_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_gpio_output_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_mac_Sgmac_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_NetTx_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_clear_rmep_mac_status_fail_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_phy_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_show_ipv4_info_cmd_vtysh);
  install_element (1, &ctc_cli_scl_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_add_protocol_vlan_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_i2c_local_cmd_vtysh);
  install_element (1, &ctc_cli_gb_aps_show_bridge_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_cfm_show_port_oam_info_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_show_dir_property_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_policer_stats_enable_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_dump_mpls_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_cpumac_en_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_remove_ipv6_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_inter_laken_prbs_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_remove_egress_vlan_mapping_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_acl_show_status_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_system_ipg_size_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_TcamCtlInt_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_tod_intf_rx_code_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_rspan_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_fid_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_rspan_escape_mac_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_port_lm_enable_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_dir_property_cmd_vtysh);
  install_element (1, &ctc_cli_security_mac_security_show_port_security_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_port_shape_cmd_vtysh);
  install_element (1, &ctc_cli_acl_create_group_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_reflective_bridge_cmd_vtysh);
  install_element (1, &ctc_cli_acl_destroy_group_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_set_mep_index_by_sw_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_pkt_rx_print_off_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_mac_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_misc_cpu_reason_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_HashDsMem_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_NetRx_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_debug_sim_chip_model_off_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_remove_ipv4_cmd_vtysh);
  install_element (1, &ctc_cli_nh_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_security_storm_ctl_show_global_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_port_queue_shape_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipv4_group_info_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_misc_l2l3hdr_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeOam_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_ptp_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_show_ipv4_nat_sa_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_ecmp_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_show_opf_type_info_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_DsFib_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_flow_ctl_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_offset_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_l2mcast_remove_member_cmd_vtysh);
  install_element (1, &show_cli_drv_write_reg_or_tbl_field_cmd_vtysh);
  install_element (1, &ctc_cli_qos_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_show_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_IpePktProc_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_i2c_local_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_show_vlan_profile_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_chip_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_transmit_cmd_vtysh);
  install_element (1, &ctc_cli_l2_stp_set_state_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_drain_on_off_file_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_pf_interval_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_chip_clock_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_ipuc_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_add_remove_maid_cmd_vtysh);
  install_element (1, &ctc_cli_nh_clear_stats_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_ipv4_entry_cmd_vtysh);
  install_element (1, &ctc_cli_common_show_memory_summary_total_cmd_vtysh);
  install_element (1, &ctc_cli_acl_update_pbr_action_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_discard_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_port_dir_property_cmd_vtysh);
  install_element (1, &ctc_cli_acl_show_group_info_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l4_app_data_cmd_vtysh);
  install_element (1, &ctc_cli_acl_install_group_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_learning_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_add_lmep_cmd_vtysh);
  install_element (1, &cli_oam_cfm_update_fs_cmd_vtysh);
  install_element (1, &ctc_cli_acl_uninstall_group_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_i2c_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_bist_cmd_vtysh);
  install_element (1, &ctc_cli_gb_packet_show_packet_info_cmd_vtysh);
  install_element (1, &ctc_cli_l2mcast_add_addr_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_set_global_action_by_bpdu_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_set_global_action_by_layer3_type_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_chip_cmd_vtysh);
  install_element (1, &ctc_cli_security_ip_src_guard_add_entry_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_add_vlan_class_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_IpeForward_cmd_vtysh);
  install_element (1, &ctc_cli_gb_qos_show_qos_domain_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_mdioxg_clock_cmd_vtysh);
  install_element (1, &ctc_cli_gb_l2_show_fdb_status_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipv6_member_info_cmd_vtysh);
  install_element (1, &ctc_cli_common_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_common_show_memory_detail_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_capture_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_remove_nh_member_cmd_vtysh);
  install_element (1, &ctc_cli_common_interrupt_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_resrc_mgr_enable_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_mtu2_packet_length_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_port_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l3_classify_l3pdu_by_l3proto_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_set_tpid_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_show_header_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_ccm_enable_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_queue_depth_cmd_vtysh);
  install_element (1, &ctc_cli_sync_ether_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l3_global_action_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_port_en_cmd_vtysh);
  install_element (1, &ctc_cli_oam_set_lbm_proc_in_asic_cmd_vtysh);
  install_element (1, &ctc_cli_gb_dma_show_stats_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_member_port_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_clear_tod_intf_rx_code_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_show_nexthop_all_by_type_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_dump_drvio_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_net_tx_packet_capture_cmd_vtysh);
  install_element (1, &ctc_cli_l2_set_nhid_by_logic_port_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_trpt_show_state_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_ipv4_l2mc_cmd_vtysh);
  install_element (1, &ctc_cli_acl_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dma_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_interrupt_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_cfm_dump_mep_index_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_set_mirror_discard_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_remove_protocol_vlan_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_vlan_mapping_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_port_allocate_internal_port_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_common_fast_enter_cmodel_mode_cmd_vtysh);
  install_element (1, &ctc_cli_scl_uninstall_group_cmd_vtysh);
  install_element (1, &ctc_cli_chip_write_xgphy_interface_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_trpt_en_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_i2c_scan_para_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_debug_simulator_off_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_syncup_internal_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_rspan_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_dvlan_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_tx_vlan_tpid_cmd_vtysh);
  install_element (1, &ctc_cli_gb_stacking_show_trunk_info_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_sync_intf_cmd_vtysh);
  install_element (1, &ctc_cli_acl_get_multi_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_run_sgmac_prbs_xauipcs_cmd_vtysh);
  install_element (1, &ctc_cli_common_enter_app_mode_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_untag_dft_vid_disable_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_opf_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_common_error_debug_cmd_vtysh);
  install_element (1, &ctc_cli_nh_swap_mpls_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_rx_vlan_tpid_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_pcie_io_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_voltage_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_show_vlan_mirror_state_cmd_vtysh);
  install_element (1, &ctc_cli_nh_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_mac_QuadMac_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_BufRetrv_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_iloop_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_set_vlan_pas_num_cmd_vtysh);
  install_element (1, &ctc_cli_security_storm_ctl_set_global_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_pcie_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_gb_interrupt_clear_status_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_clear_rmep_seq_num_fail_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_remove_ipv4_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l4_set_app_data_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_monitor_sgmac_prbs_xauipcs_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_stats_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_mcast_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_run_bist_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_vlan_classify_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_show_ipv6_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_dump_error_cmd_vtysh);
  install_element (1, &ctc_cli_chip_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_datapath_mode_cmd_vtysh);
  install_element (1, &ctc_cli_gb_qos_show_element_info_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_add_session_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_port_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_mcast_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_l2mcast_add_member_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_remove_lmep_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeHdrProc_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_show_mep_info_with_key_cmd_vtysh);
  install_element (1, &ctc_cli_oam_set_lbr_use_share_mac_cmd_vtysh);
  install_element (1, &ctc_cli_l2_show_stp_state_by_port_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_add_pw_cmd_vtysh);
  install_element (1, &ctc_cli_gb_l2_show_master_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_lm_type_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_tx_cos_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_emulation_load_config_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_update_ipv6_rpf_cmd_vtysh);
  install_element (1, &ctc_cli_nh_show_max_ecmp_cmd_vtysh);
  install_element (1, &ctc_cli_common_fast_enter_debug_mode_cmd_vtysh);
  install_element (1, &ctc_cli_oam_common_set_exception_cmd_vtysh);
  install_element (1, &ctc_cli_acl_set_entry_priority_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_add_ipv4_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_show_bfd_up_time_cmd_vtysh);
  install_element (1, &ctc_cli_scl_remove_entry_cmd_vtysh);
  install_element (1, &ctc_cli_security_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_set_mep_maid_len_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_add_vpws_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_gephy_sp_para_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_stats_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_cpu_access_dyn_ram_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_port_info_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l3_set_l3flex_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_gb_port_set_vlan_range_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_min_frame_size_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_reset_in_sqn_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_remove_rmep_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_preamble_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_mac_entry_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l3_set_ip_frag_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_misc_l2hdr_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_bridge_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_show_property_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_show_bfd_session_state_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_show_vlan_class_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_create_or_destroy_l3_phy_if_cmd_vtysh);
  install_element (1, &ctc_cli_bpe_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_tod_intf_tx_code_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_speed_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_set_port_action_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_global_property_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_stats_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_gb_dma_tbl_wr_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_temp_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_xgphy_sp_dis_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_trpt_set_session_en_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_datapath_infor_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_cpu_mac_stats_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_random_threshold_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_ipv6_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_arp_dhcp_excp_cmd_vtysh);
  install_element (1, &ctc_cli_stats_clear_log_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_drop_log_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_add_rspan_session_cmd_vtysh);
  install_element (1, &ctc_cli_common_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_security_storm_ctl_set_config_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_tcam_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_mac_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_vlan_enable_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_set_property_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_global_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_cpu_access_static_ram_file_cmd_vtysh);
  install_element (1, &ctc_cli_gb_interrupt_show_interrupt_chip_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_set_vlan_enable_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_add_vlan_mapping_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_aps_debug_off_cmd_vtysh);
  install_element (1, &common_cli_show_memory_module_detaill_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_blocking_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_add_ipv4_member_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_ipuc_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l2_tpid_cmd_vtysh);
  install_element (1, &ctc_cli_oam_set_all_pdu_to_cpu_cmd_vtysh);
  install_element (1, &ctc_cli_port_mapping_show_mapping_mode_cmd_vtysh);
  install_element (1, &ctc_cli_security_mac_security_set_system_mac_limit_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_DynamicDsMem_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_set_keeplive_cmd_vtysh);
  install_element (1, &show_cli_drv_list_tbl_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_set_port_enable_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_l2_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l2_port_action_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_trpt_get_stats_cmd_vtysh);
  install_element (1, &ctc_cli_qos_show_policer_stats_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_set_learning_action_cmd_vtysh);
  install_element (1, &ctc_cli_acl_remove_all_entry_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_aps_xlate_cmd_vtysh);
  install_element (1, &ctc_cli_common_fast_enter_sdk_mode_cmd_vtysh);
  install_element (1, &ctc_cli_qos_clear_port_queue_stats_cmd_vtysh);
  install_element (1, &ctc_cli_chip_read_i2c_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bus_cmd_vtysh);
  install_element (1, &ctc_cli_scl_show_group_info_cmd_vtysh);
  install_element (1, &ctc_cli_port_release_internal_port_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_run_inter_laken_prbs_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_replace_dscp_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_reset_out_sqn_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_remove_vlan_vrange_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_SharedDs_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_port_mac_postfix_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_PciExpCore_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_debug_io_on_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_link_intr_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_aps_mpls_pop_nexthop_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_threshold_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_ipg_index_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_remove_ipv6_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_gb_scl_show_status_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_vlan_ctl_cmd_vtysh);
  install_element (1, &show_cli_drv_read_reg_or_tbl_field_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_policer_configure_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_DsAging_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_rpf_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipmc_status_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_remove_ipv6_member_cmd_vtysh);
  install_element (1, &ctc_cli_gb_security_ip_src_guard_remove_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_cfm_dump_lmep_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_sync_intf_toggle_time_cmd_vtysh);
  install_element (1, &ctc_cli_remove_ip_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_PtpFrc_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_set_freq_cmd_vtysh);
  install_element (1, &ctc_cli_gb_l2_show_dflt_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_serdes_param_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_remove_remote_member_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_unmapping_l3type_cmd_vtysh);
  install_element (1, &ctc_cli_gb_drvio_action_cmd_vtysh);
  install_element (1, &ctc_cli_stats_get_threshold_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_add_egress_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_trpt_set_session_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_ipg_size_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_mel_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_IntLkIntf_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_sync_intf_toggle_time_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_MacLedDriver_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_intf_status_cmd_vtysh);
  install_element (1, &ctc_cli_scl_copy_entry_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_show_logic_port_mapping_srcport_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_port_enable_port_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_Qsgmii_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_cfg_global_param_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_remove_ipv4_member_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_show_vlan_mapping_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_egs_exp_map_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_nh_update_ipuc_cmd_vtysh);
  install_element (1, &ctc_cli_l2_add_or_remove_port_to_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_FwdMsg_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_sub_if_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_seq_num_en_cmd_vtysh);
  install_element (1, &ctc_cli_scl_get_multi_entry_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_remove_ipmc_member_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l3_classify_l3pdu_by_l4port_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_show_config_cmd_vtysh);
  install_element (1, &ctc_cli_gb_interrupt_get_status_cmd_vtysh);
  install_element (1, &ctc_cli_security_mac_security_show_mac_limit_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_ipv4_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_port_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_show_status_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_show_opf_alloc_info_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_reason_shape_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_port_max_md_level_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_create_or_destroy_l3_sub_if_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bridge_header_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_mac_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_xgphy_sp_para_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_arrange_fragment_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_phy_if_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_tp_section_oam_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_pbr_ipv6_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_HashDs_cmd_vtysh);
  install_element (1, &show_l3if_vmac_low_8bits_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_rspan_escape_en_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_get_vlan_vrange_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_remove_edge_port_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_isolation_id_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_port_nhid_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_max_frame_size_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_Mdio_cmd_vtysh);
  install_element (1, &ctc_cli_scl_remove_all_entry_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_mac_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_serdes_mode_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_cfm_dump_maid_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_group_wdrr_weight_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l2_global_action_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_show_vlan_l3if_id_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_mpls_pop_nexthop_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_QMgrQueEntry_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_mapping_l3type_cmd_vtysh);
  install_element (1, &ctc_cli_l2_clear_stp_state_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_update_lmep_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_dir_property_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_csf_use_uc_da_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_integrity_golden_cmd_vtysh);
  install_element (1, &ctc_cli_nh_show_stats_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_set_port_tag_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_Parser_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_sfp_para_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_net_rx_packet_capture_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_classify_l2pdu_by_macda_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_overlay_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_vlan_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_l2_show_fdb_detail_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_show_count_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l3_set_trill_header_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_ipv6_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_set_sim_action_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_add_ipv6_member_cmd_vtysh);
  install_element (1, &ctc_cli_gb_dma_show_state_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_show_port_mirror_state_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_rmep_seq_num_en_cmd_vtysh);
  install_element (1, &ctc_cli_gb_qos_show_qos_metfifo_excp_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_bridge_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_I2CMaster_cmd_vtysh);
  install_element (1, &ctc_cli_gb_l2_show_mcast_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_chip_info_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_mpls_nexthop_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_create_or_destroy_l3_vlan_if_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_show_mpls_property_cmd_vtysh);
  install_element (1, &ctc_cli_nh_update_mpls_nexthop_to_unrov_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_clear_error_cache_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_gephy_cmd_vtysh);
  install_element (1, &ctc_cli_acl_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_bpe_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_show_l3if_id_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_reg_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_use_outer_ttl_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_xgphy_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_show_ftm_info_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_debug_simulator_on_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_datapath_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_access_mode_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_ipv6_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_cfm_dump_rmep_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_system_max_frame_size_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_reg_cmd_vtysh);
  install_element (1, &ctc_cli_port_efm_lb_mode_enable_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_QMgrDeq_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_emulation_dump_config_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_remove_l2mc_member_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_port_property_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_update_mcast_service_queue_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_pkt_rx_print_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l2_vlan_pas_num_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_igmp_snooping_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_enable_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_queue_wdrr_weight_mtu_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_parallel_loopback_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_add_vlan_vrange_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_shape_ipg_en_cmd_vtysh);
  install_element (1, &ctc_cli_l2_show_fdb_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_update_normal_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_hss6g_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipv4_group_param_cmd_vtysh);
  install_element (1, &ctc_cli_dma_thread_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_debug_io_off_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_iloop_cmd_vtysh);
  install_element (1, &ctc_cli_common_fast_enter_internal_debug_mode_cmd_vtysh);
  install_element (1, &ctc_cli_chip_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_cpu_reason_gourp_mode_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_srcdiscard_cmd_vtysh);
  install_element (1, &ctc_cli_gb_acl_show_entry_info_cmd_vtysh);
  install_element (1, &ctc_cli_parser_set_ecmp_hash_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_debug_sim_chip_model_on_cmd_vtysh);
  install_element (1, &ctc_cli_gb_port_set_scl_key_type_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_sgmac_prbs_xfipcs_cmd_vtysh);
  install_element (1, &ctc_cli_stats_create_statsid_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_mac_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l4_app_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_stag_tpid_index_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_hss12g_cmd_vtysh);
  install_element (1, &ctc_cli_sync_ether_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_mac_led_mode_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_queue_pkt_len_cmd_vtysh);
  install_element (1, &ctc_cli_aps_map_mepindex_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l3_unmapping_l4type_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_device_type_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_show_resource_usage_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_add_remove_trunk_port_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_parser_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_xgphy_para_cmd_vtysh);
  install_element (1, &ctc_cli_port_mapping_show_port2phy_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_i2c_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_lm_cos_type_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_oam_autogen_pkt_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l2_pbb_header_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_gephy_scan_special_reg_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_oam_OamProc_cmd_vtysh);
  install_element (1, &ctc_cli_scl_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_update_ipv4_rpf_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_port_queue_stats_en_cmd_vtysh);
  install_element (1, &ctc_cli_oam_set_lbr_use_lbm_da_cmd_vtysh);
  install_element (1, &ctc_cli_common_help_cmd_vtysh);
  install_element (1, &ctc_cli_scl_install_entry_cmd_vtysh);
  install_element (1, &ctc_cli_aps_set_aps_select_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_protocol_vlan_cmd_vtysh);
  install_element (1, &ctc_cli_gb_mirror_show_mirror_info_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l3_type_cmd_vtysh);
  install_element (1, &ctc_cli_gb_l2_show_fdb_hw_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_mpls_entry_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_nhop_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_BufStore_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_mac_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_create_vlan_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_egs_cos_map_cmd_vtysh);
  install_element (1, &ctc_cli_security_mac_security_show_system_mac_limit_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_set_reflective_bridge_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_security_mac_security_set_port_learn_limit_cmd_vtysh);
  install_element (1, &ctc_cli_l2_add_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_count_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_set_aging_property_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_reg_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_add_ipv4_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_acl_uninstall_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_IpeLkupMgr_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_mac_switch_sys_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipv6_group_info_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeHdrAdj_cmd_vtysh);
  install_element (1, &ctc_cli_dma_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_remove_pw_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_hash_ipv4_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_hss12g_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_add_ipv6_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_epe_flow_chart_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_gb_scl_show_entry_info_cmd_vtysh);
  install_element (1, &ctc_cli_gb_mpls_show_space_cmd_vtysh);
  install_element (1, &ctc_cli_sync_ether_show_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_ipe_flow_chart_cmd_vtysh);
  install_element (1, &ctc_cli_port_mapping_show_port2mdio_cmd_vtysh);
  install_element (1, &ctc_cli_nh_cfg_max_ecmp_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_all_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_igs_dscp_map_cmd_vtysh);
  install_element (1, &ctc_cli_l2_remove_ucast_nexthop_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_add_ipv4_nat_sa_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_system_max_frame_size_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_remove_trunk_cmd_vtysh);
  install_element (1, &ctc_cli_common_enter_internal_debug_mode_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_random_log_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_clock_drift_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_policer_first_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_igs_prec_map_cmd_vtysh);
  install_element (1, &ctc_cli_dma_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_tod_intf_tx_code_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_global_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_common_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_nh_get_l2uc_cmd_vtysh);
  install_element (1, &ctc_cli_common_parity_error_show_mapping_memory_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_gephy_para_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_ecmp_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_gephy_sp_dis_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_clear_rmep_dcsf_cmd_vtysh);
  install_element (1, &ctc_cli_scl_set_entry_priority_cmd_vtysh);
  install_element (1, &ctc_cli_oam_tp_add_chan_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_add_l3vpn_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_clock_time_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_xlate_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_svlan_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_mpls_show_ilm__cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_oam_autogen_pkt_disable_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_ipv6_entry_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_add_normal_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_scl_install_group_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_rmep_csf_en_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_clock_drift_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_sf_state_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_pcie_mem_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_port_mac_link_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipv6_group_param_cmd_vtysh);
  install_element (1, &ctc_cli_stats_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_sgmac_prbs_xauipcs_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_update_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_monitor_rlt_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_add_ipv6_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_remove_chan_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_monitor_sgmac_prbs_xfipcs_cmd_vtysh);
  install_element (1, &ctc_cli_stats_clear_stats_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_DynamicDs_cmd_vtysh);
  install_element (1, &ctc_cli_gb_mpls_show_count_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_set_global_action_by_macda_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_adjust_clock_offset_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_calc_eye_height_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_untag_dft_vid_enable_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_exec_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_l2_set_default_entry_feature_cmd_vtysh);
  install_element (1, &ctc_cli_common_hostname_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_group_shape_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_flush_all_egress_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_gb_drvio_clear_cache_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_mcast_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_cross_connect_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_oam_OamFwd_cmd_vtysh);
  install_element (1, &ctc_cli_gb_qos_show_cpu_reason_cmd_vtysh);
  install_element (1, &ctc_cli_gb_dma_tbl_rd_cmd_vtysh);
  install_element (1, &ctc_cli_gb_mpls_show_mpls_status_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_dm_en_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_set_global_property_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_mdio_clock_frequency_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_sfp_cmd_vtysh);
  install_element (1, &ctc_cli_common_interrupt_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_ipv4_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_port_set_scl_property_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_igs_cos_map_cmd_vtysh);
  install_element (1, &ctc_cli_bpe_show_mux_demux_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_TcamDs_cmd_vtysh);
  install_element (1, &ctc_cli_update_ip_tunnel_to_unrov_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_set_end_rchip_bitmap_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_wrapback_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l3_set_l3if_action_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_mac_led_en_cmd_vtysh);
  install_element (1, &ctc_cli_qos_create_policer_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_add_or_remove_port_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l3_trill_header_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_set_l2flex_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_sync_intf_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeNextHop_cmd_vtysh);
  install_element (1, &ctc_cli_common_fast_enter_app_mode_cmd_vtysh);
  install_element (1, &ctc_cli_aps_create_and_remove_raps_group_cmd_vtysh);
  install_element (1, &ctc_cli_scl_update_action_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_igs_exp_map_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_loopback_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_remove_stats_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_set_mpls_property_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l3_mapping_l4type_cmd_vtysh);
  install_element (1, &ctc_cli_nh_update_mpls_pop_nexthop_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_QuadPcs_cmd_vtysh);
  install_element (1, &ctc_cli_scl_destroy_group_cmd_vtysh);
  install_element (1, &ctc_cli_stats_reset_mac_cmd_vtysh);
  install_element (1, &ctc_cli_oam_get_mep_stats_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_remove_egress_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_mcast_cmd_vtysh);
  install_element (1, &ctc_cli_scl_set_default_action_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_pcie_mem_cmd_vtysh);
  install_element (1, &ctc_cli_security_ip_src_guard_remove_entry_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_show_detail_member_port_cmd_vtysh);
  install_element (1, &ctc_cli_gb_interrupt_show_interrupt_sdk_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_vlan_filtering_cmd_vtysh);
  install_element (1, &ctc_cli_port_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_drop_unknown_pkt_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_set_ipv6_force_route_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_add_egress_vlan_mapping_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_interrupt_get_en_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_add_remove_trunk_rchip_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l3_l3flex_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_pcie_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_show_ipv4_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipmc_show_ipv4_member_info_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_oam_OamAutoGenPkt_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_linkagg_hash_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_ecmp_hash_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_stats_cmd_vtysh);
  install_element (1, &ctc_cli_l2_add_addr_cmd_vtysh);
  install_element (1, &ctc_cli_l2_show_fdb_count_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_remove_rmep_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_monitor_inter_laken_prbs_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_vlan_max_md_level_cmd_vtysh);
  install_element (1, &ctc_cli_port_show_port_mac_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeCla_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_create_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_add_vpls_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_dump_cmd_vtysh);
  install_element (1, &ctc_cli_qos_add_del_service_cmd_vtysh);
  install_element (1, &ctc_cli_common_show_ver_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_mac_entry_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_mdio_clock_frequency_cmd_vtysh);
  install_element (1, &ctc_cli_bpe_interlaken_en_cmd_vtysh);
  install_element (1, &ctc_cli_security_port_isolation_set_route_obey_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_check_packet_capture_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_add_rmep_cmd_vtysh);
  install_element (1, &ctc_cli_security_port_isolation_show_route_obey_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l2_set_pbb_header_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_pbr_ipv4_entry_cmd_vtysh);
  install_element (1, &ctc_cli_gb_acl_set_global_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_set_ecmp_mode_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_l3_classify_l3pdu_cmd_vtysh);
  install_element (1, &ctc_cli_gb_chip_agent_serverip_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_receive_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_add_ipv6_group_cmd_vtysh);
  install_element (1, &ctc_cli_parser_set_linkagg_hash_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_oam_OamParser_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_add_vmac_low_8bits_cmd_vtysh);
  install_element (1, &ctc_cli_chip_write_i2c_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_cpu_reason_map_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_csf_type_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_GlobalStats_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_phy_scan_en_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_mac_en_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_serdes_mode_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_show_port_cpu_mac_en_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ftm_opf_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_stats_drop_log_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_show_ipv6_info_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_qos_remove_policer_cmd_vtysh);
  install_element (1, &ctc_cli_nh_update_ecmp_cmd_vtysh);
  install_element (1, &ctc_cli_nh_add_xlate_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_remove_ipv4_nat_sa_cmd_vtysh);
  install_element (1, &ctc_cli_stats_reset_port_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_pading_cmd_vtysh);
  install_element (1, &humber_cli_aps_remvoe_aps_bridge_group_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_gephy_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_add_rmep_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en_cmd_vtysh);
  install_element (1, &ctc_cli_parser_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_write_sfp_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_port_status_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_mdio1g_clock_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_ELoop_cmd_vtysh);
  install_element (1, &ctc_cli_gb_qos_use_default_domain_cmd_vtysh);
  install_element (1, &ctc_cli_chip_read_gephy_interface_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_bsr_MetFifo_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_phb_configure_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_mac_MacMux_cmd_vtysh);
  install_element (1, &ctc_cli_scl_uninstall_entry_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_scl_create_group_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_vlan_cmd_vtysh);
  install_element (1, &ctc_cli_l2mcast_remove_addr_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_update_rmep_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_GreatbeltSup_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_show_state_cmd_vtysh);
  install_element (1, &ctc_cli_ipuc_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_xgphy_scan_special_reg_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_add_ipv4_group_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_l3_set_global_action_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_read_hss6g_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_resrc_mgr_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_ipv6_l2mc_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_IpeFib_cmd_vtysh);
  install_element (1, &show_cli_drv_list_tbl_all_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_remove_ipv6_group_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_port_check_enable_cmd_vtysh);
  install_element (1, &ctc_cli_acl_add_hash_mac_entry_cmd_vtysh);
  install_element (1, &ctc_cli_oam_tp_y1731_add_remove_maid_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_csf_en_cmd_vtysh);
  install_element (1, &ctc_cli_common_show_error_desc_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_transmit_cmd_vtysh);
  install_element (1, &ctc_cli_security_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_tod_intf_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_scan_sfp_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_cvlan_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_add_stats_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_hss12g_enable_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_show_property_cmd_vtysh);
  install_element (1, &ctc_cli_scl_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_l2_remove_addr_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_loopback_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_rmep_enable_cmd_vtysh);
  install_element (1, &ctc_cli_stats_reset_cpu_mac_cmd_vtysh);
  install_element (1, &ctc_cli_l2_show_stp_state_by_port_and_stpid_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_add_remove_mip_cmd_vtysh);
  install_element (1, &ctc_cli_acl_remove_entry_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_remove_session_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l2_max_len_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_show_sync_intf_rx_code_cmd_vtysh);
  install_element (1, &ctc_cli_parser_l4_set_app_cmd_vtysh);
  install_element (1, &ctc_cli_packet_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_add_chan_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_show_vmac_prefix_cmd_vtysh);
  install_element (1, &ctc_cli_port_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_bridge_mac_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_port_oam_enable_cmd_vtysh);
  install_element (1, &ctc_cli_sync_ether_set_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_IpeHdrAdj_cmd_vtysh);
  install_element (1, &ctc_cli_gb_drvio_show_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_set_default_global_action_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_set_vrf_stats_en_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_remove_all_vlan_mapping_cmd_vtysh);
  install_element (1, &show_cli_linkread_cmd_vtysh);
  install_element (1, &ctc_cli_qos_clear_policer_stats_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_PtpEngine_cmd_vtysh);
  install_element (1, &ctc_cli_acl_update_action_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_vlan_domain_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeHdrEdit_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_dot1q_type_cmd_vtysh);
  install_element (1, &ctc_cli_nh_remove_aps_xlate_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_phy_scan_para_cmd_vtysh);
  install_element (1, &ctc_cli_aps_show_select_cmd_vtysh);
  install_element (1, &ctc_cli_oam_set_big_interval_to_cpu_cmd_vtysh);
  install_element (1, &ctc_cli_pdu_show_all_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_add_mac_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_debug_off_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_queueid_mon_cmd_vtysh);
  install_element (1, &ctc_cli_gb_dma_mpool_dbg_cmd_vtysh);
  install_element (1, &ctc_cli_oam_bfd_add_lmep_cmd_vtysh);
  install_element (1, &ctc_cli_packet_tx_uc_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_receive_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_queue_wdrr_weight_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_reason_shp_base_pkt_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_global_hash_type_cmd_vtysh);
  install_element (1, &ctc_cli_port_mapping_set_mapping_mode_cmd_vtysh);
  install_element (1, &ctc_cli_scl_add_port_cvlan_cos_hash_entry_cmd_vtysh);
  install_element (1, &ctc_cli_security_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_epe_EpeAclQos_cmd_vtysh);
  install_element (1, &ctc_cli_l2_remove_default_entry_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_remove_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_set_break_en_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_default_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_stats_show_global_fwd_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_update_error_cache_timer_enable_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_packet_flow_diag_start_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_set_stpid_cmd_vtysh);
  install_element (1, &ctc_cli_acl_install_entry_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_update_lmep_lm_en_cmd_vtysh);
  install_element (1, &ctc_cli_mpls_remove_ilm_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_remove_vmac_low_8bits_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_run_sgmac_prbs_xfipcs_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_TcamDsMem_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_add_ipv4_vlan_class_cmd_vtysh);
  install_element (1, &ctc_cli_parser_show_l3_ip_frag_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_share_Policing_cmd_vtysh);
  install_element (1, &ctc_cli_gb_pdu_show_global_config_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_default_vlan_cmd_vtysh);
  install_element (1, &ctc_cli_mirror_show_mirror_discard_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_discard_same_mac_or_ip_cmd_vtysh);
  install_element (1, &ctc_cli_l2_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_misc_PbCtl_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_adjust_delay_cmd_vtysh);
  install_element (1, &ctc_cli_gb_interrupt_set_en_cmd_vtysh);
  install_element (1, &ctc_cli_learning_aging_remove_port_nhid_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_chip_set_access_mode_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_cpu_reason_dest_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_mac_CpuMac_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_show_vlan_info_cmd_vtysh);
  install_element (1, &ctc_cli_common_interrupt_show_debug_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_system_port_mac_prefix_cmd_vtysh);
  install_element (1, &ctc_cli_update_ip_tunnel_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_rpf_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_stats_set_dot1q_subtract_bytes_cmd_vtysh);
  install_element (1, &ctc_cli_chip_show_sensor_value_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_bsr_flow_chart_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_packet_flow_diag_rst_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_stretch_mode_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_show_router_mac_cmd_vtysh);
  install_element (1, &ctc_cli_port_efm_lb_mode_disable_cmd_vtysh);
  install_element (1, &ctc_cli_qos_bind_service_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_hss12g_prbs_cmd_vtysh);
  install_element (1, &ctc_cli_gb_oam_cfm_dump_ma_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_all_mac_en_cmd_vtysh);
  install_element (1, &ctc_cli_l3if_set_router_mac_cmd_vtysh);
  install_element (1, &ctc_cli_security_storm_ctl_show_cfg_cmd_vtysh);
  install_element (1, &ctc_cli_gb_nh_show_nexthop_by_nhid_type_cmd_vtysh);
  install_element (1, &ctc_cli_gb_vlan_destroy_vlan_vrange_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_set_sender_id_tlv_cmd_vtysh);
  install_element (1, &ctc_cli_gb_ipuc_reinit_cmd_vtysh);
  install_element (1, &ctc_cli_stats_destroy_statsid_cmd_vtysh);
  install_element (1, &ctc_cli_oam_tp_remove_chan_cmd_vtysh);
  install_element (1, &ctc_cli_oam_common_set_defect_to_cpu_cmd_vtysh);
  install_element (1, &ctc_cli_packet_debug_show_cmd_vtysh);
  install_element (1, &ctc_cli_linkagg_show_member_port_cmd_vtysh);
  install_element (1, &show_cli_tbl_reg_cmd_vtysh);
  install_element (1, &ctc_cli_vlan_add_vlan_mapping_cmd_vtysh);
  install_element (1, &ctc_cli_stacking_create_trunk_cmd_vtysh);
  install_element (1, &ctc_cli_oam_cfm_debug_on_cmd_vtysh);
  install_element (1, &ctc_cli_dbg_tool_show_ipe_IpeIntfMap_cmd_vtysh);
  install_element (1, &ctc_cli_aps_update_raps_member_cmd_vtysh);
  install_element (1, &ctc_cli_ipmc_remove_ipv4_group_cmd_vtysh);
  install_element (1, &ctc_cli_l2_add_ucast_nexthop_cmd_vtysh);
  install_element (1, &ctc_cli_oam_common_set_defect_to_rdi_cmd_vtysh);
  install_element (1, &ctc_cli_qos_show_port_queue_stats_cmd_vtysh);
  install_element (1, &ctc_cli_qos_set_port_queue_drop_cmd_vtysh);
  install_element (1, &ctc_cli_port_set_internal_port_cmd_vtysh);
  install_element (1, &ctc_cli_ptp_set_device_type_cmd_vtysh);
  install_element (1, &ctc_cli_qos_debug_off_cmd_vtysh);
}
