
#include "ctc_cli.h"
#if 0 
CTC_CLI(cli_option_show0,
    option_show_cmd0,
    "show0 (a1 A1) b (a2 A2|) (a3 A3|)",
    "Option show0",
    "Keyword a1",
    "Var A1",
    "Key b",
    "Keyword a2",
    "Keyword A2",
    "Keyword a3",
    "Var A3")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_option_show1,
    option_show_cmd1,
    "show1 (a1|a2) b",
    "Option show1",
    "Keyword a1",
    "Keyword a2",
    "Key b")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_option_show2,
    option_show_cmd2,
    "show2 (a1 A11 A12 A13 |a2 A2 (a3 A3|)) x X",
    "Option show2",
    "Keyword a1",
    "Var A11",
    "Var A12",
    "Var A13",
    "Keyword a2",
    "Var A2",
    "Keyword a3",
    "Var A3",
    "Keyword x",
    "Var X")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_option_test,
    option_test_cmd,
    "show3 (a1 A11 A12 A13 |) (a2 A2|)  (a3 A3|) x X",
    "Show3",
    "Keyword a1",
    "Var A11",
    "Var A12",
    "Var A13",
    "Keyword a2",
    "Var A2",
    "Keyword a3",
    "Var A3",
    "Keyword x",
    "Var X")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_option_test4,
    option_test_cmd4,
    "show4 (a1 A11 A12 A13 |) (a2 A2|)  (a3 A3|)",
    "Show3",
    "Keyword a1",
    "Var A11",
    "Var A12",
    "Var A13",
    "Keyword a2",
    "Var A2",
    "Keyword a3",
    "Var A3")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_multi_option_test,
    multi_option_test_cmd,
    "multi-option-test (bb BB_VAR | dd (ee EE_VAR| ff FF_VAR) )",
    "Multi Option test",
    "Key bb",
    "Var BB",
    "Key dd",
    "Key ee",
    "Var EE",
    "Key ff",
    "Var FF"
    )
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_multi_option_test1,
    multi_option_test1_cmd,
    "multi1 {option1| option2|option3} hello XXX",
    "Multi Option test1",
    "If option1 exist or not",
    "If option2 exist or not",
    "If option3 exist or not",
    "Hello",
    "XXX var")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}


CTC_CLI(cli_multi_option_test2,
    multi_option_test2_cmd,
    "multi2 {option1| (option2_1 | option2_2) |option3} hello XXX",
    "Multi Option test2",
    "If option1 exist or not",
    "If option2_1 exist or not",
    "If option2_2 exist or not",
    "If option3 exist or not",
    "Hello",
    "XXX var")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_multi_option_test5,
    multi_option_test5_cmd,
    "multi-option-test5 {aa AA_VAR ((bb BB_VAR| cc CC_VAR) | dd (ee EE_VAR| ff FF_VAR) ) |gg GG_VAR} hello XXX",
    "Multi Option test5",
    "If aa exist or not",
    "aa VAR",
    "bb exist or not",
    "BB VAR",
    "cc exist or not",
    "cc VAR",
    "dd exist or not",
    "ee exist or not",
    "ee VAR",
    "ff exist or not",
    "ff VAR",
    "gg exist or not",
    "gg AR",
    "Hello",
    "XXX var")
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}


CTC_CLI(cli_exit,
        cli_exit_cmd,
        "exit",
        "Exit"
        )
{
  switch(vty->node)
  {
    case EXEC_MODE: 
        restore_terminal_mode();
        exit(0);
        break;
    case CTC_SDK_MODE:
        vty->node = EXEC_MODE;
        break;
    default:
        break;
  }
    return 0;
}

extern void enable_cmd_debug(int enable);
CTC_CLI (cli_cmd_debug,
    cmd_debug_cmd,
    "debug (on|off)",
    "Debug command",
    "Debug on",
    "Debug off")
{
    if(strcmp("on", argv[0])==0)
    {
        enable_cmd_debug(1);
    }
    else
    {
        enable_cmd_debug(0);
    }

  return CLI_SUCCESS;
}

#define CTC_CLI_NH_EGS_VLANTRANS_STR "Egress Vlan Editing nexthop (external nexthop)"
#define CTC_CLI_NH_APS_EGS_VLANTRANS_STR "APS Egress Vlan Editing nexthop (external nexthop)"
#define CTC_CLI_NH_BRGUC_STR  "Bridge unicast nexthop (internal nexthop)"
#define CTC_CLI_NH_BRGMC_STR  "Bridge multicast nexthop (internal nexthop)"
#define CTC_CLI_NH_IPUC_STR  "IP unicast nexthop (external nexthop)"
#define CTC_CLI_NH_IPMC_STR  "IP multicast nexthop (internal nexthop)"
#define CTC_CLI_NH_MPLS_STR  "MPLS nexthop (external nexthop)"
#define CTC_CLI_NH_ILOOP_STR  "IPE Loopback nexthop (external nexthop)"
#define CTC_CLI_NH_RSPAN_STR  "RSPAN nexthop (external nexthop)"
#define CTC_CLI_NH_UPMEP_STR  "UP MEP nexthop(internal nexthop)"



#define CTC_CLI_NH_ADD_STR "Add nexthop"
#define CTC_CLI_NH_DEL_STR "Delete nexthop"
#define CTC_CLI_NH_UPDATE_STR "Update nexthop"
#define CTC_CLI_NH_DSNH_OFFSET_STR "Ds nexthop offset in global dynamic sram"
#define CTC_CLI_NH_DSNH_OFFSET_VALUE_STR  CTC_CLI_GLOBAL_MCASTGRP_NEXTHOP_ID_DESC
#define CTC_CLI_L3IF_ALL_STR "(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) (outervlan-is-svlan|)"
#define CTC_CLI_L3IF_ALL_DESC "Sub interface", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, "VLAN port/interface,if vlan port,the vlan_id is equal to 4095 ,indicate the vid is invalid", CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, CTC_CLI_GPORT_DESC,  \
    CTC_CLI_GPORT_ID_DESC, "Routed Port interface", CTC_CLI_GPORT_ID_DESC,"outer Vlan is svlan"

#define CTC_CLI_NH_EGS_VLAN_EDIT_STR "cvlan-edit-type VALUE svlan-edit-type VALUE output-cvid VALUE \
        output-svid VALUE (swap-vlan (swap-tpid|) (swap-cos|)|) (cvlan-valid|) (svlan-valid|) (change-macda MAC|)"
#define CTC_CLI_NH_EGS_VLAN_EDIT_DESC "CVLAN Editing Type",                     \
        "0 for no egress editing, keep ingress editing; 1 for keep unchange, "  \
        "will recover ingress editing although ingress editing have done; "     \
        "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan",            \
        "SVLAN Editing Type",                                                   \
        "0 for no egress editing, keep ingress editing; 1 for keep unchange, "  \
        "will recover ingress editing although ingress editing have done; "     \
        "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan",            \
        "Output CVLANID",                                                       \
        CTC_CLI_VLAN_RANGE_DESC,                                                \
        "Output SVLANID",                                                       \
        CTC_CLI_VLAN_RANGE_DESC,                                                \
        "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit",\
        "Enable swaping SVLAN's TPID with CVLAN's TPID",                        \
        "Enable swaping SVLAN's Cos with CVLAN's Cos",                          \
        "Output CVLANID is valid",                                              \
        "Output SVLANID is valid",                                               \
        "Replace output packet's mac da",                                    \
        CTC_CLI_MAC_FORMAT

#define CTC_CLI_NH_MPLS_POP_NH_PARAM_STR "payload-op (op-none|op-route|op-mpls|l2vpn-no-op|l2vpn-op "CTC_CLI_NH_EGS_VLAN_EDIT_STR")\
    (sequence-number SQN-INDEX |) (mac MAC|) "CTC_CLI_L3IF_ALL_STR
#define CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC "Mpls pop nexthop payload operation type",                            \
    "Payload could be ip, mpls or ethernet, and the operation type is none, will append new layer2 header",     \
    "Payload could be ip, will decreace ip header's ttl and will append new layer2 header",                     \
    "Payload could be mpls, will operation mpls label's ttl, and will append new layer2 header",                \
    "Payload could be ethernet, will not do any endit on the payload, and will not append new layer2 header",   \
    "Payload could be ethernet, will edit ethernet's vlan tag, and will not append new layer2 header",          \
    CTC_CLI_NH_EGS_VLAN_EDIT_DESC,                                                                              \
    "Enable sequence order",                                                                                    \
    "Sequence counter index, <0-255>",                                                                          \
    CTC_CLI_MAC_DESC,                                                                                           \
    CTC_CLI_MAC_FORMAT,                                                                                         \
    CTC_CLI_L3IF_ALL_DESC

#define CTC_CLI_NH_MPLS_PUSH_NH_PARAM_STR  "mac MAC "CTC_CLI_L3IF_ALL_STR" payload-op (op-none|op-route|op-l2vpn (vpls-type|) "CTC_CLI_NH_EGS_VLAN_EDIT_STR")\
    ((martini-cw (no-seq|per-pw-seq seq-index INDEX|glb-seq0|glb-seq1))|label1 LABLE ttl1 TTL exp1 EXP exp1-type EXP_TYPE  (map-ttl1|) (is-mcast1|) )\
    (label2 LABLE ttl2 TTL exp2 EXP exp2-type EXP_TYPE (map-ttl2|) (is-mcast2|)|)\
    (label3 LABLE ttl3 TTL exp3 EXP exp3-type EXP_TYPE (map-ttl3|) (is-mcast3|)|)\
    (label4 LABLE ttl4 TTL exp4 EXP exp4-type EXP_TYPE (map-ttl4|) (is-mcast4|)|)\
    (vpls-port VPLS_PORT|)"
#define CTC_CLI_NH_MPLS_PUSH_NH_PARAM_DESC     CTC_CLI_MAC_DESC,    \
    CTC_CLI_MAC_FORMAT,                                             \
    CTC_CLI_L3IF_ALL_DESC,                                          \
    "Mpls push nexthop payload operation type",                     \
    "Payload could be ip, mpls or ethernet, and the operation type is none",\
    "Payload could be ip, will decreace ip header's ttl",           \
    "Payload could be ethernet, will edit ethernet's vlan tag",     \
    "Nexthop is vpls type, will do vpls split horizon",             \
    CTC_CLI_NH_EGS_VLAN_EDIT_DESC,                                  \
    "First label is martini control word",                          \
    "Martini sequence type is none",                                \
    "Martini sequence type is per-pw",                              \
    "Martini per-pw sequence index",                                \
    "Sequence index value",                                         \
    "Martini sequence type is global sequence type0",               \
    "Martini sequence type is global sequence type1",               \
    "MPLS label1",                                                  \
    "MPLS label1 valule",                                           \
    "MPLS label1's ttl",                                            \
    "MPLS label1's ttl value(0~255)",                                      \
    "MPLS EXP1",                                                    \
    "MPLS EXP1 value(0-7)",                                              \
    "MPLS EXP1 type(0-2)",                                               \
    "MPLS EXP1 type value, 0 stands for new EXP1 will use the specified EXP1 value; 1 stands for new EXP1 will use ingress mapped exp; 2 stands for new EXP1 will be mapped from packet",\
    "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL",\
    "MPLS label1 is mcast label",                                   \
    "MPLS label2",                                                  \
    "MPLS label2 valule",                                           \
    "MPLS label2's ttl",                                            \
    "MPLS label2's ttl value(0~255)",                                      \
    "MPLS EXP2",                                                    \
    "MPLS EXP2 value(0-7)",                                              \
    "MPLS EXP2 type(0-2)",                                              \
    "MPLS EXP2 type value, 0 stands for new EXP2 will use the specified EXP1 value; 1 stands for new EXP2 will use ingress mapped exp; 2 stands for new EXP2 will be mapped from packet",\
    "MPLS label2's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL",\
    "MPLS label2 is mcast label",                                   \
    "MPLS label3",                                                  \
    "MPLS label3 valule",                                           \
    "MPLS label3's ttl",                                            \
    "MPLS label3's ttl value(0~255)",                                      \
    "MPLS EXP3",                                                    \
    "MPLS EXP3 value(0-7)",                                              \
    "MPLS EXP3 type(0-2)",                                               \
    "MPLS EXP3 type value, 0 stands for new EXP3 will use the specified EXP3 value; 1 stands for new EXP3 will use ingress mapped exp; 2 stands for new EXP3 will be mapped from packet",\
    "MPLS label3's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL",\
    "MPLS label3 is mcast label",                                   \
    "MPLS label4",                                                  \
    "MPLS label4 valule",                                           \
    "MPLS label4's ttl",                                            \
    "MPLS label4's ttl value(0~255)",                                      \
    "MPLS EXP4",                                                    \
    "MPLS EXP4 value(0-7)",                                              \
    "MPLS EXP4 type(0-2)",                                               \
    "MPLS EXP4 type value, 0 stands for new EXP4 will use the specified EXP4 value; 1 stands for new EXP4 will use ingress mapped exp; 2 stands for new EXP4 will be mapped from packet",\
    "MPLS label4's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL",\
    "MPLS label4 is mcast label",                                   \
    "MPLS vpls port check enable",                                  \
    "MPLS vpls port value <0-8191>"                                 \

#define CTC_CLI_NH_APS_WORKING_PATH_DESC     "APS working path parameter"
#define CTC_CLI_NH_APS_PROTECTION_PATH_DESC  "APS protection path parameter"
#define CTC_CLI_NH_APS_BRIDGE_ID_STR         "Aps bridge id"
#define CTC_CLI_NH_APS_BRIDGE_ID_DESC        "Aps bridge group id"

CTC_CLI(cli_ctc_nh_add_mpls_aps_pop_nexthop,
    cli_ctc_nh_add_mpls_aps_pop_nexthop_cmd,
    "nexthop add aps-mpls NHID dsnh-offset OFFSET (dsnh8w|) (aps-bridge-id APS_BRIDGE_ID) pop (working-path "CTC_CLI_NH_MPLS_POP_NH_PARAM_STR
    "protection-path "CTC_CLI_NH_MPLS_POP_NH_PARAM_STR")",
    CTC_CLI_NH_M_STR,
    CTC_CLI_NH_ADD_STR,
    CTC_CLI_NH_MPLS_STR,
    CTC_CLI_NH_ID_STR, 
    CTC_CLI_NH_DSNH_OFFSET_STR,
    CTC_CLI_NH_DSNH_OFFSET_VALUE_STR, 
    "Use DsNexthop8W table, default use DsNexthop4W table",
    CTC_CLI_NH_APS_BRIDGE_ID_STR,
    CTC_CLI_NH_APS_BRIDGE_ID_DESC,
    "Mpls pop nexthop",
    CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
    CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC,
    CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
    CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC)
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI(cli_ctc_nh_add_aps_mpls_push_nexthop,
    cli_ctc_nh_add_aps_mpls_push_nexthop_cmd,
    "nexthop add aps-mpls NHID dsnh-offset OFFSET (dsnh8w|) (aps-bridge-id APS_BRIDGE_ID) push (working-path "CTC_CLI_NH_MPLS_PUSH_NH_PARAM_STR")"
    "(protection-path "CTC_CLI_NH_MPLS_PUSH_NH_PARAM_STR")",
    CTC_CLI_NH_M_STR,
    CTC_CLI_NH_ADD_STR,
    "APS MPLS nexthop",
    CTC_CLI_NH_ID_STR, 
    CTC_CLI_NH_DSNH_OFFSET_STR,
    CTC_CLI_NH_DSNH_OFFSET_VALUE_STR, 
    "Use DsNexthop8W table, default use DsNexthop4W table",
    CTC_CLI_NH_APS_BRIDGE_ID_STR,
    CTC_CLI_NH_APS_BRIDGE_ID_DESC,
    "Mpls push nexthop",
    CTC_CLI_NH_APS_WORKING_PATH_DESC,
    CTC_CLI_NH_MPLS_PUSH_NH_PARAM_DESC,
    CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
    CTC_CLI_NH_MPLS_PUSH_NH_PARAM_DESC)
{
    int i =0;
    printf("argc=%d, argv= \n\r", argc);
    for(i=0; i<argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
    return CLI_SUCCESS;
}

CTC_CLI (show_history,
       show_history_cmd,
       "show history",
       SHOW_STR,
       "Display the session command history")
{
    int index;
    for (index = vty->hindex + 1; index != vty->hindex;)
    {
        if (index == VTY_MAXHIST)
        {
        index = 0;
        continue;
        } 
        if (vty->hist[index] != NULL)
            printf ("  %s%s", vty->hist[index], VTY_NEWLINE); 
        index++;    
    } 
    return CLI_SUCCESS;
}

void install_all_cmd(void)
{
    install_element (EXEC_MODE, &cli_exit_cmd);
    install_element (EXEC_MODE, &show_history_cmd);

    install_element (CTC_SDK_MODE, &cli_exit_cmd);
    install_element (CTC_SDK_MODE, &show_history_cmd);
    install_element (CTC_SDK_MODE, &cmd_debug_cmd);
    install_element (CTC_SDK_MODE, &option_show_cmd0);
    install_element (CTC_SDK_MODE, &option_show_cmd1);
    install_element (CTC_SDK_MODE, &option_show_cmd2);
    install_element (CTC_SDK_MODE, &option_test_cmd);
    install_element (CTC_SDK_MODE, &option_test_cmd4);
    install_element (CTC_SDK_MODE, &multi_option_test1_cmd);
    install_element (CTC_SDK_MODE, &multi_option_test2_cmd);

    install_element (CTC_SDK_MODE, &cli_ctc_nh_add_mpls_aps_pop_nexthop_cmd);
    install_element (CTC_SDK_MODE, &cli_ctc_nh_add_aps_mpls_push_nexthop_cmd);
}


struct cmd_node exec_node =
{
  EXEC_MODE,
  "\rCTC_CLI#",
};

struct cmd_node sdk_node =
{
  CTC_SDK_MODE,
  "\rCTC_CLI(ctc-sdk)#",
};

struct cmd_node cmodel_node =
{
  CTC_CMODEL_MODE,
  "\r(CTC_CLI(ctc-cmodel)#",
};

struct cmd_node oam_chan_node =
{
  CTC_SDK_OAM_CHAN_MODE,
  "\rCTC_CLI((oam_chan)#",
};

/* change main to other name for vxworks */
int main()
{
    ctc_cmd_init (0);
    /* Install top nodes. */
    ctc_install_node (&exec_node, NULL);
    ctc_install_node (&sdk_node, NULL);
    ctc_install_node (&cmodel_node, NULL);
    ctc_install_node (&oam_chan_node, NULL); 
    vty_init();

    install_all_cmd();

    ctc_sort_node();
    set_terminal_raw_mode();

    while(1)
    {
        vty_read();
    }

    restore_terminal_mode();

    return 0;
}
#endif


