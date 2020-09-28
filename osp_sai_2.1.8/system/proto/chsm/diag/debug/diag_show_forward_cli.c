
/****************************************************************************
 *diag_show_forward.c     Provides debug command for cmodel
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liul
 * Date:         2010-11-19.
 * Reason:       First Create.
 ****************************************************************************/

   
 /******************************************************************
       * include header files
  ******************************************************************/
#include <unistd.h>
#include "pal.h"
#include "cli.h"
#include "lib.h"
#include "pal_string.h"
#include "thread.h"
#include "chsm_mgt.h"
#include "chsm_port.h"
#include "chsm_error.h"
#include "cli.h"
#include "chsm.h"
#include "chsm_debug.h"
#include "diag_show_forward.h"
#include "diag_show_forward_util_pkt.h"
#include "diag_show_forward_util_rand.h"

/****************************************************************************
*  
* Defines and Macros
*
*****************************************************************************/
static diag_ctrl_var_t g_diag_show_forward_ctrl_var[DIAG_PAYLOAD];
static diag_config_t g_diag_show_forward_config[DIAG_PAYLOAD];

uint64 g_diag_show_forward_module_enable;  
uint64 g_diag_show_forward_out_pkt_enable; 
uint64 g_diag_show_forward_debug = FALSE;
bool g_is_show_forward_logging_file_open = FALSE;
bool g_is_show_forward_cmd_runing = FALSE;
#define BUFFER_MAX 200

/****************************************************************************
*  
* Function
*
*****************************************************************************/

/****************************************************************************
 * Name    : _diag_show_forward_get_common_header
 * Purpose : get common header
 * Input    : argv : command line parameter
 * Output  : lc: line card . common_header: common header
 * Return   :  CHSM_E_XX
 * Note     :  N/A.
 *****************************************************************************/
static int32
_diag_show_forward_get_common_header(char **argv, tbl_card_t **lc, int32 *argi, uint8* pvid, 
                                                                    diag_show_forward_common_header_t * common_header)
{
    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name         : _diag_show_forward_get_l2_header
 * Purpose      : get l2 header
 * Input        :  argv : command line parameter.
 * Output       : N/A..
 * Return       : CHSM_E_XX
 * Note         :   N/A.
 *****************************************************************************/
static int32
_diag_show_forward_get_l2_header(struct cli *cli, int32 argc, char **argv, int32 *argi, 
                                                                            uint8 pvid, diag_ethernet_config_t * l2_config)
{
    int32 temp = 0;
    int32 flag = 0;
    /* 1, SRC MAC*/
    if (sal_sscanf (argv[*argi], "%4hx.%4hx.%4hx",
                  (unsigned short *)&l2_config->mac_sa[0],
                  (unsigned short *)&l2_config->mac_sa[2],
                  (unsigned short *)&l2_config->mac_sa[4]) != 3)
    {
        cli_out (cli, "%% Unable to translate src mac address %s\n", argv[1]);
        return CHSM_E_INVALID_PARAM;
    }
      /* Convert to network order */
    *(unsigned short *)&l2_config->mac_sa[0] =
      sal_hton16(*(unsigned short *)&l2_config->mac_sa[0]);
    *(unsigned short *)&l2_config->mac_sa[2] =
      sal_hton16(*(unsigned short *)&l2_config->mac_sa[2]);
    *(unsigned short *)&l2_config->mac_sa[4] =
      sal_hton16(*(unsigned short *)&l2_config->mac_sa[4]);
    (*argi)++;

    /*2, DST MAC*/
    if (sal_sscanf (argv[*argi], "%4hx.%4hx.%4hx",
                  (unsigned short *)&l2_config->mac_da[0],
                  (unsigned short *)&l2_config->mac_da[2],
                  (unsigned short *)&l2_config->mac_da[4]) != 3)
    {
        cli_out (cli, "%% Unable to translate dst mac address %s\n", argv[1]);
        return CHSM_E_INVALID_PARAM;
    }
      /* Convert to network order */
    *(unsigned short *)&l2_config->mac_da[0] =
      sal_hton16(*(unsigned short *)&l2_config->mac_da[0]);
    *(unsigned short *)&l2_config->mac_da[2] =
      sal_hton16(*(unsigned short *)&l2_config->mac_da[2]);
    *(unsigned short *)&l2_config->mac_da[4] =
      sal_hton16(*(unsigned short *)&l2_config->mac_da[4]);
    (*argi)++;

    /*3, VLAN ID*/
    while( ( *argi < argc )  && ((!sal_strcmp(argv[*argi], "l3-type")) ||(!sal_strcmp(argv[*argi], "cos")) ||(!sal_strcmp(argv[*argi], "vlan"))))
    {
        if (!sal_strcmp(argv[*argi], "vlan"))  
        {
            (*argi)++;
            CLI_GET_INTEGER_RANGE ("vlan", temp, argv[*argi], 1, 4094);
            l2_config->vid1 = temp;
            (*argi)++;
            flag = 1;
        }

        /*4, COS*/
        else if (!sal_strcmp(argv[*argi], "cos"))  
        {
            (*argi)++;
            CLI_GET_INTEGER_RANGE ("cos", temp, argv[*argi], 1, 255);
            l2_config->cos1= temp;
            (*argi)++;
            flag = 1;
        }

        /*5, L3 TYPE*/
        else if (!sal_strcmp(argv[*argi], "l3-type"))  
        {
            (*argi)++;
            CLI_GET_INTEGER_RANGE ("l3 type", temp, argv[*argi], 1, 65535);
            l2_config->type= temp;
            (*argi)++;
        }
    }
    if ( flag && pvid)
    {
        l2_config->num_vlan = 2;
        l2_config->vid2 = l2_config->vid1;
        l2_config->vid1 = pvid; 
        l2_config->cos2 = l2_config->cos1;
        l2_config->vlan_tag1 = 0x8100;
        l2_config->vlan_tag2 = 0x8100;
    }
    else if ( flag)
    {
        l2_config->num_vlan = 1;
        l2_config->vlan_tag1 = 0x8100;
    }
    else if (pvid)
    {
        l2_config->num_vlan = 1;
        l2_config->vid1 = pvid;
        l2_config->vlan_tag1 = 0x8100;
    }

    l2_config->l2_type = L2_TYPE_ETH_V2;
    return CHSM_E_SUCCESS;
}  

/****************************************************************************
 * Name         : _diag_show_forward_get_ipv4_header(struct cli *cli, int32 argc, uint8 **argv, int32 *i, diag_ipv4_config_t * ipv4_config)
 * Purpose      : get ipv4 header
 * Input        :  argv : command line parameter.
 * Output       : ipv4_config:save ipv4 header
 * Return       : CHSM_E_XX
 * Note         :  N/A..
 *****************************************************************************/
static int32
_diag_show_forward_get_ipv4_header(struct cli *cli, int32 argc, char **argv, int32 *i, diag_ipv4_config_t * ipv4_config)
{
    int32 temp = 0;
    struct prefix_ipv4 addr4;

    /*1, IP SA*/
    CLI_GET_IPV4_PREFIX ( "ipv4 source address ", addr4, argv[*i]);
    ipv4_config->ipv4_sa = sal_ntohl( addr4.prefix.s_addr );
    (*i)++;

    /*2, IP DA */
    CLI_GET_IPV4_PREFIX ( "ipv4 dst address ", addr4, argv[*i]);
    ipv4_config->ipv4_da = sal_ntohl( addr4.prefix.s_addr );
    (*i)++;

    ipv4_config->ipv4_ttl = 64;

    /*3, protocol num*/
    while( ( *i < argc ) && ( (!sal_strcmp(argv[*i], "protocol")) || 
    (!sal_strcmp(argv[*i], "flag")) || (!sal_strcmp(argv[*i], "frag-offset")) ||
    (!sal_strcmp(argv[*i], "ttl")) || (!sal_strcmp(argv[*i], "tos")) || (!sal_strcmp(argv[*i], "length"))))
    {
        if (!sal_strcmp(argv[*i], "protocol"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE (" protocol", temp, argv[*i], 1, 255);
            ipv4_config->ipv4_proto = temp;
            (*i)++;
        }

        /*4, flag num*/
        else if (!sal_strcmp(argv[*i], "flag"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("Flag", temp, argv[*i], 1, 7);
            ipv4_config->ipv4_flags= temp;
            (*i)++;
        }

        /*5, frag-offset num*/
        else if (!sal_strcmp(argv[*i], "frag-offset"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("The frag-offset", temp, argv[*i], 1, 8191);
            ipv4_config->ipv4_frag_offset= temp;
            (*i)++;
        }

        /*6, ttl*/
        else if (!sal_strcmp(argv[*i], "ttl"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("The ttl", temp, argv[*i], 1, 255);
            ipv4_config->ipv4_ttl= temp;
            (*i)++;
        }

        /*7, tos*/
        else if (!sal_strcmp(argv[*i], "tos"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("The tos", temp, argv[*i], 1, 255);
            ipv4_config->ipv4_type_serv= temp;
            (*i)++;
        }

        /*8,total length*/
        else if (!sal_strcmp(argv[*i], "length"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("ipv4 total length", temp, argv[*i], 1, 65535);
            ipv4_config->ipv4_total_len = temp;
            (*i)++;
        }
    }

    return CHSM_E_SUCCESS;
}  
#ifdef HAVE_IPV6
/****************************************************************************
 * Name:       _diag_show_forward_get_ipv6_header(struct cli *cli, int32 argc, uint8 **argv, int32 *i, diag_ipv6_config_t * ipv6_config)
 * Purpose:    get ipv6 header
 * Input:       argv : command line parameter.
 * Output:     ipv6_config:save ipv6 header
 * Return       : CHSM_E_XX
 * Note         :  N/A
 *****************************************************************************/
static int32
_diag_show_forward_get_ipv6_header(struct cli *cli, int32 argc, char **argv, int32 *i, diag_ipv6_config_t * ipv6_config)
{
    int32 temp = 0;
    int32 n = 0;
    struct prefix_ipv6 addr6;

    CLI_GET_IPV6_PREFIX("ipv6 address classifier", addr6, argv[*i]);
    ipv6_config->ipv6_sa[0] = addr6.prefix.s6_addr32[0];
    ipv6_config->ipv6_sa[1] = addr6.prefix.s6_addr32[1];
    ipv6_config->ipv6_sa[2] = addr6.prefix.s6_addr32[2];
    ipv6_config->ipv6_sa[3] = addr6.prefix.s6_addr32[3];
    for(n = 0; n < 4; n++)
    {
        ipv6_config->ipv6_sa[n]= sal_ntohl(ipv6_config->ipv6_sa[n]);
    }
    (*i)++;

    CLI_GET_IPV6_PREFIX("ipv6 address classifier", addr6, argv[*i]);
    ipv6_config->ipv6_da[0] = addr6.prefix.s6_addr32[0];
    ipv6_config->ipv6_da[1] = addr6.prefix.s6_addr32[1];
    ipv6_config->ipv6_da[2] = addr6.prefix.s6_addr32[2];
    ipv6_config->ipv6_da[3] = addr6.prefix.s6_addr32[3];
    for(n = 0; n < 4; n++)
    {
        ipv6_config->ipv6_da[n]= sal_ntohl(ipv6_config->ipv6_da[n]);
    }
    (*i)++;

    while( ( *i < argc ) && ((!sal_strcmp(argv[*i], "priority")) ||(!sal_strcmp(argv[*i], "flow-label")) ||
    (!sal_strcmp(argv[*i], "next-header")) ||(!sal_strcmp(argv[*i], "hop-limit")) ||(!sal_strcmp(argv[*i], "length")) ))
    { 
        if (!sal_strcmp(argv[*i], "priority"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("priority", temp, argv[*i], 1, 65535);
            ipv6_config->ipv6_traf_class = temp;
            (*i)++;
        }
        else if (!sal_strcmp(argv[*i], "flow-label"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("flow-label", temp, argv[*i], 1, 1048575);
            ipv6_config->ipv6_flow_label= temp;
            (*i)++;
        }
        else if (!sal_strcmp(argv[*i], "next-header"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("next-header", temp, argv[*i], 1, 255);
            ipv6_config->ipv6_nxt_hdr= temp;
            (*i)++;
        }
        else if (!sal_strcmp(argv[*i], "hop-limit"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("hop-limit", temp, argv[*i], 1, 255);
            ipv6_config->ipv6_hp_limit= temp;
            (*i)++;
        }
        else if (!sal_strcmp(argv[*i], "length"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE ("length", temp, argv[*i], 1, 65535);
            ipv6_config->ipv6_pld_len = temp;
            (*i)++;
        }
    }

    return CHSM_E_SUCCESS;
}  
#endif
/****************************************************************************
 * Name:       diag_show_forward_get_arp_header(struct cli *cli, int32 argc, uint8 **argv, int32 *i, diag_arp_config_t * arp_config)
 * Purpose:    get arp header
 * Input:       argv : command line parameter.
 * Output:     N/A.
 * Return       : CHSM_E_XX
 * Note         :  N/A
 *****************************************************************************/
int32
diag_show_forward_get_arp_header(struct cli *cli, int32 argc, char **argv, int32 *i, diag_arp_config_t * arp_config)
{
    uint32 temp = 0;
    struct prefix_ipv4 addr4;

    arp_config->arp_hdw_type = 0x0001;
    arp_config->arp_protocol_type = 0x0800;
    arp_config->arp_hdw_addr_len = 12;
    arp_config->arp_protocol_addr_len = 4;
    arp_config->arp_operation_code = 1;

    /*3, protocol num*/
    while( ( *i < argc ) && ( (!sal_strcmp(argv[*i], "hw-type")) ||(!sal_strcmp(argv[*i], "protocol")) ||  
        (!sal_strcmp(argv[*i], "hw-len")) || (!sal_strcmp(argv[*i], "pro-len")) ||(!sal_strcmp(argv[*i], "opcode")) || 
        (!sal_strcmp(argv[*i], "snd-mac")) || (!sal_strcmp(argv[*i], "snd-ip")) ||
        (!sal_strcmp(argv[*i], "tgt-mac"))||(!sal_strcmp(argv[*i], "tgt-ip"))))
    {
        if (!sal_strcmp(argv[*i], "hw-type"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE (" hw-type", temp, argv[*i], 1, 65535);
            arp_config->arp_hdw_type = temp;
            (*i)++;
        }
        if (!sal_strcmp(argv[*i], "protocol"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE (" protocol", temp, argv[*i], 1, 65535);
            arp_config->arp_protocol_type= temp;
            (*i)++;
        }
        if (!sal_strcmp(argv[*i], "hw-len"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE (" hw-len", temp, argv[*i], 1, 255);
            arp_config->arp_hdw_addr_len = temp;
            (*i)++;
        }
        if (!sal_strcmp(argv[*i], "pro-len"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE (" pro-len", temp, argv[*i], 1, 255);
            arp_config->arp_protocol_addr_len = temp;
            (*i)++;
        }
        if (!sal_strcmp(argv[*i], "opcode"))  
        {
            (*i)++;
            CLI_GET_INTEGER_RANGE (" opcode", temp, argv[*i], 1, 255);
            arp_config->arp_operation_code = temp;
            (*i)++;
        }

        if (!sal_strcmp(argv[*i], "snd-mac"))  
        {
            (*i)++;
            if (sal_sscanf (argv[*i], "%4hx.%4hx.%4hx",
                          (unsigned short *)&arp_config->arp_sender_hdw_add[0],
                          (unsigned short *)&arp_config->arp_sender_hdw_add[2],
                          (unsigned short *)&arp_config->arp_sender_hdw_add[4]) != 3)
            {
                cli_out (cli, "%% Unable to translate sender mac address %s\n", argv[1]);
                return CHSM_E_INVALID_PARAM;
            }
              /* Convert to network order */
            *(unsigned short *)&arp_config->arp_sender_hdw_add[0] =
              sal_hton16(*(unsigned short *)&arp_config->arp_sender_hdw_add[0]);
            *(unsigned short *)&arp_config->arp_sender_hdw_add[2] =
              sal_hton16(*(unsigned short *)&arp_config->arp_sender_hdw_add[2]);
            *(unsigned short *)&arp_config->arp_sender_hdw_add[4] =
              sal_hton16(*(unsigned short *)&arp_config->arp_sender_hdw_add[4]);
            (*i)++;

        }
        if (!sal_strcmp(argv[*i], "snd-ip"))  
        {
           (*i)++;
           CLI_GET_IPV4_PREFIX ( "ipv4 source address ", addr4, argv[*i]);
           arp_config->arp_sender_protocol_add=  addr4.prefix.s_addr;
           arp_config->arp_sender_protocol_add = sal_ntohl(arp_config->arp_sender_protocol_add );
           (*i)++;
        }
        if (!sal_strcmp(argv[*i], "tgt-mac"))  
        {
          (*i)++;
         if (sal_sscanf (argv[*i], "%4hx.%4hx.%4hx",
                          (unsigned short *)&arp_config->arp_dst_hdw_add[0],
                          (unsigned short *)&arp_config->arp_dst_hdw_add[2],
                          (unsigned short *)&arp_config->arp_dst_hdw_add[4]) != 3)
            {
                cli_out (cli, "%% Unable to translate sender mac address %s\n", argv[1]);
                return CHSM_E_INVALID_PARAM;
            }
              /* Convert to network order */
            *(unsigned short *)&arp_config->arp_dst_hdw_add[0] =
              sal_hton16(*(unsigned short *)&arp_config->arp_dst_hdw_add[0]);
            *(unsigned short *)&arp_config->arp_dst_hdw_add[2] =
              sal_hton16(*(unsigned short *)&arp_config->arp_dst_hdw_add[2]);
            *(unsigned short *)&arp_config->arp_dst_hdw_add[4] =
              sal_hton16(*(unsigned short *)&arp_config->arp_dst_hdw_add[4]);
            (*i)++;
        }
        if (!sal_strcmp(argv[*i], "tgt-ip"))  
        {
            (*i)++;
            CLI_GET_IPV4_PREFIX ( "ipv4 source address ", addr4, argv[*i]);
            arp_config->arp_dst_protocol_add=  addr4.prefix.s_addr;
            arp_config->arp_dst_protocol_add = sal_ntohl(arp_config->arp_dst_protocol_add );
            (*i)++;
        }
    }

    return CHSM_E_SUCCESS;
}  

/****************************************************************************
 * Name:       _diag_show_forward_get_tcp_header(struct cli *cli, int32 argc, uint8 **argv, int32 *i, diag_tcp_config_t * tcp_config)
 * Purpose:    get tcp header
 * Input:       argv : command line parameter.
 * Output:     none.
 * Return       : CHSM_E_XX
 * Note         :  N/A
 *****************************************************************************/
static int32
_diag_show_forward_get_tcp_header(struct cli *cli, int32 argc, char **argv, int32 *i, diag_tcp_config_t * tcp_config)
{
    int32 temp = 0;

    CLI_GET_INTEGER_RANGE (" tcp src port", temp, argv[*i], 1, 65535);   
    (*i)++;
    tcp_config->sport = temp;

    CLI_GET_INTEGER_RANGE (" tcp dst port", temp, argv[*i], 1, 65535);   
    (*i)++;
    tcp_config->dport = temp;

    return CHSM_E_SUCCESS;
}  

/****************************************************************************
 * Name:       _diag_show_forward_get_udp_header(struct cli *cli, int32 argc, uint8 **argv, int32 *i, diag_udp_config_t * udp_config)
 * Purpose:    get udp header
 * Input:       argv : command line parameter.
 * Output:     none.
 * Return       : CHSM_E_XX
 * Note         :  N/A
 *****************************************************************************/
static int32
_diag_show_forward_get_udp_header(struct cli *cli, int32 argc, char **argv, int32 *i, diag_udp_config_t * udp_config)
{
    int32 temp = 0;

    CLI_GET_INTEGER_RANGE (" udp src port", temp, argv[*i], 1, 65535);   
    (*i)++;
    udp_config->sport = temp;

    CLI_GET_INTEGER_RANGE (" udp dst port", temp, argv[*i], 1, 65535);   
    (*i)++;
    udp_config->dport = temp;

    return CHSM_E_SUCCESS;
}  

/****************************************************************************
 * Name    :diag_show_forward_recv_ack(  diag_pkt_t  * ipeout_pkt)
 * Purpose : do show forwrd recv ack   
 * Input    : ipeout_pkt.
 * Output:  N/A.
 * Return   : CHSM_E_XX
 * Note     :  N/A
 *****************************************************************************/
int32
diag_show_forward_recv_ack(diag_pkt_t  * ipeout_pkt)
{
    tbl_card_t *lc_dst = NULL;
    uint8 dest_chipid = 0;

    /*4 bytes bayheader exception header, then follows bayheader, in the offset 0, byte1(network order) is dest chipid*/
    /*uint32 *tmp_pkt = (uint32*)ipeout_pkt->pkt;
    dest_chipid = (ntohl(tmp_pkt[1])>> 16)&0x1F*/
    dest_chipid = ipeout_pkt->chip_id; 
    if (dest_chipid == 0x1F)
    {
        lc_dst = NULL; /*send to all linecard, multicast*/
    }
    else
    {
        lc_dst = tbl_card_get_default_card();

        if (!lc_dst || BOARD_PRESENT != lc_dst->card_state)
        {
            CHSM_LOG_ERR("get dest chipid failed, dest_chipid=%d", dest_chipid);  
            goto FINISH;
        }
    }
    
    if (chsm_lcm_do_show_forward_epe(lc_dst, ipeout_pkt) < 0)
    {   
        CHSM_LOG_ERR("chsm_lcm_do_show_forward_epe error");        
    }

FINISH:
    
    if (NULL != ipeout_pkt->pkt)
    {
        XFREE(MTYPE_TMP, ipeout_pkt->pkt);
        ipeout_pkt->pkt = NULL;
    }

    return CHSM_E_SUCCESS;    
}

/****************************************************************************
 * Name    : _diag_show_forward_do_ipe(  struct cli* cli, diag_pkt_t  *cpkt, tbl_card_t *lc)
 * Purpose:  send msg to lcm do ipe process  
 * Input    : cpkt:input pkt. lc:line card
 * Output  :N/A.
 * Return  :CHSM_E_XX
 * Note     : N/A
 *****************************************************************************/
static int32
_diag_show_forward_do_ipe(  struct cli* cli, diag_pkt_t  *cpkt, tbl_card_t *lc)
{
    /*3. do show forward in ipe and fwd in the ingress direction*/
    if (chsm_lcm_do_show_forward_ipe(lc, cpkt) < 0)
    {
        cli_out(cli, "show forward unexpected error happen");        
    } 

    /*free the pkt info memory*/     
    if (NULL != cpkt->pkt)
    {
        XFREE(MTYPE_TMP, cpkt->pkt);
        cpkt->pkt = NULL;
    }  

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name    :  _diag_show_forward_get_one_pkt_from_file(FILE* p_file, diag_pkt_t* ipkt)
 * Purpose :  get a pkt from input file  
 * Input    : p_file:pointer of input file.
 * Output : ipkt:pointer of input pkt.
 * Return  : CHSM_E_XX
 * Note     : N/A
 *****************************************************************************/
static int32
_diag_show_forward_get_one_pkt_from_file(FILE* p_file, diag_pkt_t* ipkt)
{
    char pkt_line[PKT_LINE_LEN] = {0};
    uint32 line_num = 0;
    int32 lidx = 0; 
    int32 cidx = 0;
    int32 tmp_len = 0;
    int32 tmp_chan = 0;
    int32 tmp_chip = 0; 
    int32 tmp_val = 0;
    
    if (NULL == p_file || NULL == ipkt)
    {
        return CHSM_E_INVALID_PTR;
    }

    ipkt->pkt = (uint8 *)XCALLOC(MTYPE_TMP, SHOW_FORWARD_MTU );
    ipkt->pkt_len = 0;

    sal_memset(ipkt->pkt, 0, SHOW_FORWARD_MTU);
    sal_memset(pkt_line, 0, PKT_LINE_LEN);
    while (NULL != sal_fgets(pkt_line, PKT_LINE_LEN, p_file))
    {
        if (' ' == pkt_line[0])
        {
            continue;
        }
        if (3 == sal_sscanf(pkt_line, "pkt %d %d %d", &tmp_len, &tmp_chan, &tmp_chip))
        {
            ipkt->pkt_len = tmp_len;
            ipkt->chan_id = tmp_chan;
            ipkt->chip_id = tmp_chip;
            line_num = ipkt->pkt_len /PKT_LINE_MAX_BYTES +1;
            for (lidx = 0; lidx < line_num; ++lidx)
            {
                if (NULL == sal_fgets(pkt_line, PKT_LINE_LEN, p_file))
                {
                    return CHSM_E_SUCCESS;
                }

                for (cidx= 0; cidx < PKT_LINE_MAX_BYTES; cidx++)
                {
                    if (1 == sal_sscanf(pkt_line + cidx*2, "%02x", &tmp_val))
                    {
                        ipkt->pkt[lidx * PKT_LINE_MAX_BYTES + cidx] = tmp_val;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            return ipkt->pkt_len;
        }
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name      :_diag_show_forward_info(struct cli *cli)
 * Purpose   :  show forward debug info  
 * Input:      cli:command line interface pointer
 * Output:     N/A.
 * Return       : CHSM_E_XX
 * Note         :  N/A
 *****************************************************************************/
static int32
_diag_show_forward_info(struct cli *cli)
{
    FILE *fp = NULL;
    char buffer[BUFFER_MAX]; /*output buffer*/
    int8 flag = 0; 

    if (TRUE == g_is_show_forward_logging_file_open)
    {
        cli_out(cli,"err:show forward logging file in use!");
        return CHSM_E_SUCCESS;
    }
    
    fp = sal_fopen(SHOW_FORWARD_LOGGING_FILE, "r");
    if (fp == NULL)
    {
        cli_out(cli, "Chsm can not open logging file:%s\n", sal_strerror(errno));
        g_is_show_forward_logging_file_open = FALSE;
        return CHSM_E_SUCCESS;
    }
    g_is_show_forward_logging_file_open = TRUE;

    while (sal_fgets(buffer, BUFFER_MAX, fp) != NULL)
    {
        cli_out(cli, "%s", buffer);
        flag = 1;
    }

    if (!flag)
    {
        cli_out(cli, "Read logging file failed!\n");
    }

    sal_fclose(fp);
    g_is_show_forward_logging_file_open = FALSE;
    
    return CHSM_E_SUCCESS;
}
/****************************************************************************
 * Name     :  _diag_show_forward_send_flag(uint32 module)
 * Purpose  :   send flags to lcm 
 * Input     : module_id - ipe or epe module.
 * Output  : N/A.
 * Return   : CHSM_E_XX
 * Note    :  N/A
 *****************************************************************************/
static int32 
_diag_show_forward_send_flag(uint32 module)
{
    if ( DIAG_SHOW_FORWARD_MODULE == module)
    {
        return chsm_lcm_send_show_forward_flags(module, g_diag_show_forward_module_enable);
    }
    else if ( DIAG_SHOW_FORWARD_PKT == module)
    {
        return chsm_lcm_send_show_forward_flags(module, g_diag_show_forward_out_pkt_enable);
    }
    else if ( DIAG_SHOW_FORWARD_DEBUG == module)
    {
        return chsm_lcm_send_show_forward_flags(module, g_diag_show_forward_debug);
    }
    
    return CHSM_E_SUCCESS;
}

/****************************************************************************
* Name      :  _diag_show_forward_module_off(uint8 module_id)
* Purpose   : close the module for info print
* Input     :  module_id - ipe or epe module.
* Output   :  CLI_SUCCESS is success.
 * Return   : CHSM_E_XX
* Note      :   N/A
****************************************************************************/
static int32
_diag_show_forward_module_off(uint8 module_id)
{    
    if (module_id >= DIAG_SHOW_FORWARD_OAM_MAX_MODULE)
    {
        return CHSM_E_INVALID_INDEX;
    }
    else
    {
        g_diag_show_forward_module_enable &= ~( 1 << module_id  );
    }
     
    return CHSM_E_SUCCESS;
}
 
/****************************************************************************
* Name      : _diag_show_forward_module_on(u8 module_id)
* Purpose   : open the module for info print
* Input      : module_id - ipe or epe module.
* Output    :N/A
 * Return    : CHSM_E_XX
* Note      : N/A
****************************************************************************/
static int32
_diag_show_forward_module_on(uint8 module_id)
{    
    if (module_id >= DIAG_SHOW_FORWARD_OAM_MAX_MODULE)
    {
        return CHSM_E_INVALID_INDEX;
    }
    else
    {
       g_diag_show_forward_module_enable |=  (1<<module_id );
    }

    return CHSM_E_SUCCESS;
}

 /****************************************************************************
 * Name     : chsm_diag_debug_cmodel_on(uint8 module_id)
 * Purpose  : open the module for info print
 * Input     : module_id - ipe or epe module.
 * Output   : N/A
 * Return   : CHSM_E_XX
* Note      :  N/A
****************************************************************************/
static int32
_diag_show_forward_debug_on(uint8 module_id)
{    
    if (module_id >= DIAG_SHOW_FORWARD_OAM_MAX_MODULE)
    {
        return CHSM_E_INVALID_INDEX;
    }
    else
    {
        g_diag_show_forward_out_pkt_enable |=  (1 << module_id );
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_debug_off(uint8 module_id)
 * Purpose  : open the module for info print
 * Input     : module_id - ipe or epe module.
 * Output   : N/A
 * Return    : CHSM_E_XX
* Note      :  N/A
****************************************************************************/
static int32
_diag_show_forward_debug_off(uint8 module_id)
{    
    if (module_id >= DIAG_SHOW_FORWARD_OAM_MAX_MODULE)
    {
        return CHSM_E_INVALID_INDEX;
    }
    else
    {
        g_diag_show_forward_out_pkt_enable &= ~ (1<<module_id );
    }
    
    return CHSM_E_SUCCESS;
}

 /*cmodel ipe debug command*/
 CLI (debug_show_forward_ipe,
     debug_show_forward_ipe_cmd,
     "debug show forward ipe (all|hdr-adjust|parser|userid|ifmap|lookup|"
     "aclqos|routing|bridge|mpls|learning|classification|forwarding)",
     CLI_DEBUG_STR,
     "Show information",  
     "Show forward debug",
     "Show forward IPE debug",
     "All the sub modules",
     "Header adjust sub module",
     "Parser sub module",
     "Userid sub module",
     "Interface map sub module",
     "Lookup sub module",
     "Acl qos sub module",
     "Routing sub module",
     "Bridge sub module",
     "Mpls sub module",
     "Learning sub module",
     "Classification sub module",
     "Forwarding sub module")
{
    int32 ret = 0;
    int32 argi = 0;

    if (!sal_strncmp(argv[0], "hdr", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_HDR_ADJUST));
    }
    else if (!sal_strncmp(argv[0], "par", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_PARSER));
    }
    else if (!sal_strncmp(argv[0], "use", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_USERID));
    }
    else if (!sal_strncmp(argv[0], "ifm", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_IF_MAPPER));
    }
    else if (!sal_strncmp(argv[0], "loo", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on( DIAG_SHOW_FORWARD_IPE_LOOKUP));
    }
    else if(!sal_strncmp(argv[0], "acl", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_ACL_QOS));
    }
    else if (!sal_strncmp(argv[0], "rou", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_ROUTING));
    }
    else if (!sal_strncmp(argv[0], "bri", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_BRIDGE));
    }
    else if (!sal_strncmp(argv[0], "mpl", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_MPLS));
    }
    else if (!sal_strncmp(argv[0], "lea", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_LEARNING));
    }
    else if (!sal_strncmp(argv[0], "cla", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_CLASSIFICATION));
    }
    else if (!sal_strncmp(argv[0], "for", 3))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_IPE_FORWARDING));
    }
    else if (!sal_strncmp(argv[0], "all", 3))
    {  
        for( argi=DIAG_SHOW_FORWARD_IPE_HDR_ADJUST; argi< DIAG_SHOW_FORWARD_IPE_MAX_MODULE; argi++)
        {
            SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(argi));
        }
    }
    else 
    {
        cli_out(cli,"%% Error! Invalid ipe sub module name!\n");
        ret = CLI_ERROR;
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   

    return CLI_SUCCESS;   

}

 /*show_forward ipe no debug command*/
CLI (no_debug_show_forward_ipe,
    no_debug_show_forward_ipe_cmd,
    "no debug show forward ipe (all|hdr-adjust|parser|userid|ifmap|lookup|"
    "aclqos|routing|bridge|mpls|learning|classification|forwarding)",
    CLI_NO_STR,
    CLI_DEBUG_STR,
    "Show information",  
    "Show forward debug",
    "Show forward IPE debug",
    "All the sub modules",
    "Header adjust sub module",
    "Parser sub module",
    "Userid sub module",
    "Interface map sub module",
    "Lookup sub module",
    "Acl qos sub module",
    "Routing sub module",
    "Bridge sub module",
    "Mpls sub module",
    "Learning sub module",
    "Classification sub module",
    "Forwarding sub module")
{
   int32 argi = 0;  
 
   if (!sal_strncmp(argv[0], "hdr", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_HDR_ADJUST));
    }
   else if (!sal_strncmp(argv[0], "par", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_PARSER));
    }
   else if (!sal_strncmp(argv[0], "use", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_USERID));
    }
   else if (!sal_strncmp(argv[0], "ifm", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_IF_MAPPER));
    }
   else if (!sal_strncmp(argv[0], "loo", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_LOOKUP));
    }
   else if (!sal_strncmp(argv[0], "acl", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_ACL_QOS));
    }
   else if (!sal_strncmp(argv[0], "rou", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_ROUTING));
    }
   else if (!sal_strncmp(argv[0], "bri", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_BRIDGE));
    }
   else if (!sal_strncmp(argv[0], "mpl", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_MPLS));
    }
   else if (!sal_strncmp(argv[0], "lea", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_LEARNING));
    }
   else if (!sal_strncmp(argv[0], "cla", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_CLASSIFICATION));
    }
   else if (!sal_strncmp(argv[0], "for", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_IPE_FORWARDING));
    }
   else if (!sal_strncmp(argv[0], "all", 3))
    {  
        for( argi=DIAG_SHOW_FORWARD_IPE_HDR_ADJUST; argi< DIAG_SHOW_FORWARD_IPE_MAX_MODULE; argi++)
        {
            SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(argi));
        }
    }
   else 
    {
        cli_out(cli,"%% Error! Invalid ipe sub module name!\n");
        return CLI_ERROR;
    }
    
   SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   
     
   return CLI_SUCCESS;   
 
 }

/*debug cmodel epe command*/
CLI (debug_show_forward_epe,
    debug_show_forward_epe_cmd,
    "debug show forward epe (all|hdr-adjust|nexthop|pld-proc|l3edit|l2edit|aclqos|classification|header-edit)",
    CLI_DEBUG_STR,
    "Show information",  
    "Show forward debug",
    "Show forward EPE debug",
    "All the sub modules",
    "Header adjust sub module",
    "Next hop sub module",
    "Payload process sub module",
    "Layer3 edit sub module",
    "Layer2 edit sub module",
    "Acl qos sub module",    
    "Classification sub module",
    "Header edit sub module")
{
    int32 argi = 0;

    if (!sal_strncmp(argv[0], "hdr", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_HDR_ADJUST));
    }
    else if (!sal_strncmp(argv[0], "nex", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_NEXT_HOP));
    }
    else if (!sal_strncmp(argv[0], "pld", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_PAYLOAD_PROCESS));
    }
    else if (!sal_strncmp(argv[0], "l3e", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_L3_EDITING));
    }
    else if (!sal_strncmp(argv[0], "l2e", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_L2_EDITING));
    }
    else if (!sal_strncmp(argv[0], "acl", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_ACL_QOS));
    }
    else if (!sal_strncmp(argv[0], "cla", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_CLASSIFICATION));
    }
    else if (!sal_strncmp(argv[0], "hea", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_EPE_HEADER_EDIT));
    }
    else if (!sal_strncmp(argv[0], "all", 3))
    {  
        for ( argi=DIAG_SHOW_FORWARD_EPE_HDR_ADJUST; argi< DIAG_SHOW_FORWARD_EPE_MAX_MODULE; argi++)
        {
            SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(argi));
        }
    }
    else 
    {
        cli_out(cli,"%% Error! Invalid epe sub module name!\n");
        return CLI_ERROR;
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   

    return CLI_SUCCESS;
}

 /*no debug cmodel epe command*/
CLI (no_debug_show_forward_epe,
    no_debug_show_forward_epe_cmd,
    "no debug show forward epe (all|hdr-adjust|nexthop|pld-proc|l3edit|l2edit|aclqos|classification|header-edit)",   
    CLI_NO_STR,
    CLI_DEBUG_STR,
    "Show information",  
    "Show forward debug",
    "Show forward EPE debug",
    "All the sub modules",
    "Header adjust sub module",
    "Next hop sub module",
    "Payload process sub module",
    "Layer3 edit sub module",
    "Layer2 edit sub module",
    "Acl qos sub module",    
    "Classification sub module",
    "Header edit sub module")
{
    int32 argi = 0;

    if(!sal_strncmp(argv[0], "hdr", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_HDR_ADJUST));
    }
    else if(!sal_strncmp(argv[0], "nex", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_NEXT_HOP));
    }
    else if(!sal_strncmp(argv[0], "pld", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_PAYLOAD_PROCESS));
    }
    else if(!sal_strncmp(argv[0], "l3e", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_L3_EDITING));
    }
    else if(!sal_strncmp(argv[0], "l2e", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_L2_EDITING));
    }
    else if(!sal_strncmp(argv[0], "acl", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_ACL_QOS));
    }
    else if(!sal_strncmp(argv[0], "cla", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_CLASSIFICATION));
    }
    else if(!sal_strncmp(argv[0], "hea", 3))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_EPE_HEADER_EDIT));
    }
    else if(!sal_strncmp(argv[0], "all", 3))
    {  
        for( argi=DIAG_SHOW_FORWARD_EPE_HDR_ADJUST; argi< DIAG_SHOW_FORWARD_EPE_MAX_MODULE; argi++)
        {
            SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off( argi ));
        }
    }
    else 
    {
        cli_out(cli,"%% Error! Invalid epe sub module name!\n");
        return CLI_ERROR;
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   

    return CLI_SUCCESS;
}

/*debug cmodel fwd command*/
CLI (debug_show_forward_fwd,
    debug_show_forward_fwd_cmd,
    "debug show forward fwd (all|bufstore|metfifo|qmgr|bufrev)",
    CLI_DEBUG_STR,
    "Show information",  
    "Show forward debug",
    "Show forward qmgt debug",
    "All the sub modules",
    "Bufstore process",
    "Metfifo process",
    "Qmgr process",
    "Bufrev process")
{
    int32 argi = 0;

    if (0 == sal_strncmp(argv[0], "all", sal_strlen("all")))
    {
        for (argi = DIAG_SHOW_FORWARD_FWD_BUFF_STORE; argi < DIAG_SHOW_FORWARD_FWD_MAX_MODULE; ++argi)
        {
            SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(argi));
        }
    }
    else if (0 == sal_strncmp(argv[0], "bufstore", sal_strlen("bufstore")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_FWD_BUFF_STORE));
    }
    else if (0 == sal_strncmp(argv[0], "metfifo", sal_strlen("metfifo")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_FWD_GEN_ENQUE));
    }
    else if (0 == sal_strncmp(argv[0], "qmgr", sal_strlen("qmgr")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_FWD_GEN_QID));
    }
    else if (0 == sal_strncmp(argv[0], "bufrev", sal_strlen("bufrev")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(DIAG_SHOW_FORWARD_FWD_BUFF_RETRIEVE));
    }
    else
    {
        cli_out(cli,"%% Error! Invalid fwd sub module name!\n");
        return CLI_ERROR;
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   

    return CLI_SUCCESS;
    
}

 /*debug cmodel fwd command*/
CLI (no_debug_show_forward_fwd,
    no_debug_show_forward_fwd_cmd,
    "no debug show forward fwd (all|bufstore|metfifo|qmgr|bufrev)",  
    CLI_NO_STR,
    CLI_DEBUG_STR,
    "Show information",  
    "Show forward debug",
    "Show forward qmgt debug",
    "All process",
    "Bufstore process",
    "Metfifo process",
    "Qmgr process",
    "Bufrev process")
{
    int32 argi = 0;
 
    if (0 == sal_strncmp(argv[0], "all", sal_strlen("all")))
    {
        for (argi = DIAG_SHOW_FORWARD_FWD_BUFF_STORE; argi < DIAG_SHOW_FORWARD_FWD_MAX_MODULE; ++argi)
        {
            SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(argi));
        }
    }
    else if (0 == sal_strncmp(argv[0], "bufstore", sal_strlen("bufstore")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_FWD_BUFF_STORE));
    }
    else if (0 == sal_strncmp(argv[0], "metfifo", sal_strlen("metfifo")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_FWD_GEN_ENQUE));
    }
    else if (0 == sal_strncmp(argv[0], "qmgr", sal_strlen("qmgr")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_FWD_GEN_QID));
    }
    else if (0 == sal_strncmp(argv[0], "bufrev", sal_strlen("bufrev")))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(DIAG_SHOW_FORWARD_FWD_BUFF_RETRIEVE));
    }
    else
    {
        cli_out(cli,"%% Error! Invalid fwd sub module name!\n");
        return CLI_ERROR;
    }
 
   SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   
     
    return CLI_SUCCESS;
     
 } 


 /*cmodel ipe debug command*/
 CLI (debug_show_forward_oam,
     debug_show_forward_oam_cmd,
     "debug show forward oam all",
     CLI_DEBUG_STR,
     "Show information",  
     "Show forward debug",
     "Show forward OAM debug",
     "All the sub modules")
{
    int32 argi = 0;

    for (argi = DIAG_SHOW_FORWARD_OAM_HDR_ADJUST; argi < DIAG_SHOW_FORWARD_OAM_MAX_MODULE; argi++)
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_on(argi));
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   

    return CLI_SUCCESS;   

}

 /*show_forward ipe no debug command*/
CLI (no_debug_show_forward_oam,
    no_debug_show_forward_oam_cmd,
    "no debug show forward oam all",
    CLI_NO_STR,
    CLI_DEBUG_STR,
    "Show information",  
    "Show forward debug",
    "Show forward OAM debug",
    "All the sub modules")
{
    int32 argi = 0;  

    for (argi = DIAG_SHOW_FORWARD_OAM_HDR_ADJUST; argi < DIAG_SHOW_FORWARD_OAM_MAX_MODULE; argi++)
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_module_off(argi));
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_MODULE));   
     
    return CLI_SUCCESS;   
 
 }

/*these 2 cmds are only for emu*/
/*debug cmodel fwd command*/
CLI (show_forward_cmodel_pkt,
     show_forward_cmodel_pkt_cmd,
    "debug show forward  pkt (all|input|output|qmgt|oam)",
    CLI_DEBUG_STR,    
    "Show information",  
    "Show forward debug",
    "Show forward packet debug",
    "All the sub modules",
    "Show input packet",
    "Show output packet",
    "Show qmgt packet",
    "Show oam packet")
{
    if (!sal_strncmp(argv[0], "all", 3))
    {  
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_INPUT_PKT));
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_QMGT_PKT));
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_OUTPUT_PKT));
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_OAM_PKT));
    }
    else if (!sal_strncmp(argv[0], "input", 5))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_INPUT_PKT));
    }
    else if (!sal_strncmp(argv[0], "output", 6))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_OUTPUT_PKT));
    }
    else if (!sal_strncmp(argv[0], "qmgt", 4))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_QMGT_PKT));
    }
    else if (!sal_strncmp(argv[0], "oam", 3))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_on(DIAG_SHOW_FORWARD_OAM_PKT));
    }

    SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_PKT));   
    
    return CLI_SUCCESS;
    
}
    
 /*debug cmodel fwd command*/
CLI (no_show_fwd_cmodel_pkt,
    no_show_fwd_cmodel_pkt_cmd,
    "no debug show forward pkt (all|input|output|qmgt)", 
    CLI_NO_STR,
    CLI_DEBUG_STR, 
    "Show information",  
    "Show forward debug",
    "Show forwrd packet debug",
    "All the sub modules",
    "Show input packet",
    "Show output packet",
    "Show qmgt packet")
{
   if (!sal_strncmp(argv[0], "all", 3))
    {  
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_off(DIAG_SHOW_FORWARD_INPUT_PKT));
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_off(DIAG_SHOW_FORWARD_QMGT_PKT));
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_off(DIAG_SHOW_FORWARD_OUTPUT_PKT));
    }
   else if (!sal_strncmp(argv[0], "input", 5))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_off(DIAG_SHOW_FORWARD_INPUT_PKT));
    }
   else if (!sal_strncmp(argv[0], "output", 6))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_off(DIAG_SHOW_FORWARD_OUTPUT_PKT));
    }
   else if (!sal_strncmp(argv[0], "qmgt", 4))
    {
       SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_debug_off(DIAG_SHOW_FORWARD_QMGT_PKT));
    }
 
   SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_send_flag(DIAG_SHOW_FORWARD_PKT));   
     
    return CLI_SUCCESS;
     
 }

 /*show the  logging information from file*/
CLI (show_forward_info,
    show_forward_info_cmd,
    "show forward info",  
    "Show information",  
    "Show forward debug information",  
    "Forward information")
{

    SHOW_FORWARD_IF_ERROR_RETURN( _diag_show_forward_info(cli)); 

    return CLI_SUCCESS;
    
}

CLI (show_forward_set_pkt,
     show_forward_set_pkt_cmd,
     "show forward INTERFACE SRC-MAC DST-MAC ({vlan <1-4094> |cos <0-1> |l3-type TYPE }|)"
     "((ipv4 IP-SA IP-DA ({protocol PROT| flag FLAG|frag-offset OFFSET|ttl TTL|tos TOS |length LENGTH }|)) |"
     "(ipv6 IPV6-SA IPV6-DA ({priority PRI|flow-label LABEL|next-header HEADER|hop-limit LIMIT | length PAY-LEN}|)) |"
     "(arp )|"
     " mpls |bpdu|eapol |ospf|)"
     "(tcp SRC-PORT DST-PORT |udp SRC-PORT DST-PORT|igmp|icmp |icmpv6|pim-sm|"
     "l2vpn <16-65536> SRC-MAC DST-MAC (vlan <1-4094>|)| ) (payload LEN|)",
     "Show information",
     "Debug cmodel forward",
     "Interface name as eth-0-1",
     "MAC address as xxxx.xxxx.xxxx",
     "MAC address as xxxx.xxxx.xxxx",
     "VLAN ID",
     "VLAN ID 1-4095",
     "COS",
     "COS value",
     "L3 type",
     "L3 type such as 0x0800",
     "IPV4 frame",
     "IPV4  source address as A.B.C.D",
     "IPV4 destination address as A.B.C.D",
     "Protocol num",
     "Protocol Number",
     "IPV4 header frag",
     "Frag value",
     "IPV4 header fragment offset",
     "Offset value",
     "IPV4 header time to live",
     "TTL value",
     "IPV4 header type of service",
     "TOS value",
     "IPV4 total length",
     "IPV4 total length value",
     "IPV6 frame",
     "IPV6 source address as XXXX:XXXX::XXXX:XXXX",
     "IPV6 destination address XXXX:XXXX::XXXX:XXXX",
     "IPV6 Priority",
     "IPV6 Priority,little this value, lower the priority",
     "IPV6 Flow Label",
     "IPV6 flow label, used as virtual connection",
     "IPV6 Next header",
     "IPV6 next header, indecate the extend header or L4 protocol",
     "IPV6 Hop limit",
     "IPV6 hop limit, as time to live",
     "IPV6 payload ",
     "IPV6 payload length",
     "ARP frame",
     "MPLS frame",
     "BPDU frame",
     "EAPOL frame",
     "OSPF frame",
     "TCP frame",
     "Source port num <1-65535>",
     "Destination port num <1-65535>",
     "UDP frame",
     "Source port num <1-65535>",
     "Destination port num <1-65535>",
     "IGMP  frame",
     "ICMP  frame",
     "ICMPV6 frame",
     "PIM-SM frame",
     "Layer2 vpn frame",
     "Layer2 vpn label 16-65536",
     "MAC address as xxxx.xxxx.xxxx",
     "MAC address as xxxx.xxxx.xxxx",
     "VLAN ID",
     "VLAN ID 1-4095",
     "Pay load",
     "Pay load length")
{
    int32 argi = 0;
    int32 argj = 0;
    int32 temp = 0;
    int32 ret = 0;
    diag_pkt_t in_pkt ;
    diag_ethernet_config_t *ethv2_config = NULL ;
    diag_show_forward_common_header_t common_header;
    char *emp_char = "nomeaning";
    uint8 pvid = 0;
    tbl_card_t *lc = NULL;

    if( TRUE == g_is_show_forward_cmd_runing)
    {
        return CLI_SUCCESS;
    }
    g_is_show_forward_cmd_runing = TRUE;
    

    if( TRUE == g_is_show_forward_logging_file_open)
    {
        return CLI_SUCCESS;
    }

    /*delete the logging file first*/ 
    remove(SHOW_FORWARD_LOGGING_FILE);
    g_is_show_forward_logging_file_open = FALSE;

    sal_memset(&in_pkt, 0, sizeof(diag_pkt_t));
    sal_memset(g_diag_show_forward_ctrl_var, 0, DIAG_PAYLOAD*sizeof(diag_ctrl_var_t));
    sal_memset(g_diag_show_forward_config, 0, DIAG_PAYLOAD*sizeof(diag_config_t)); 

    /* 1, get chip-id and channel-id according to ifname */
    ret = _diag_show_forward_get_common_header(argv, &lc, &argi, &pvid, &common_header);
    if ( CHSM_E_SUCCESS != ret)
    {
        g_is_show_forward_cmd_runing = FALSE;
        return ret;
    }

    /* 2, get l2 header*/
    g_diag_show_forward_config[argj].header_type = DIAG_ETHERNET_HDR;
    ethv2_config = &g_diag_show_forward_config[argj].ethernet;
    ret = _diag_show_forward_get_l2_header(cli, argc, argv, &argi, pvid, ethv2_config);
    if ( CHSM_E_SUCCESS != ret)
    {
        g_is_show_forward_cmd_runing = FALSE;
        return ret;
    }

    /*3, get l3 header*/
    if( argi >= argc ) 
    {
        goto construct_packet;
    }
    if (!sal_strcmp (argv[argi], "ipv4") )
    {
        argj++;
        g_diag_show_forward_config[argj].header_type = DIAG_IPV4_HDR;
        argi++;
        ret = _diag_show_forward_get_ipv4_header(cli, argc, argv, &argi, &g_diag_show_forward_config[argj].ipv4);
        if ( CHSM_E_SUCCESS != ret)
        {
            g_is_show_forward_cmd_runing = FALSE;
            return ret;
        }
        g_diag_show_forward_config[argj-1].ethernet.type = 0x0800;
    }
#ifdef HAVE_IPV6
    else if (!sal_strcmp (argv[argi], "ipv6") )
    {
        argj++;
        g_diag_show_forward_config[argj].header_type = DIAG_IPV6_HDR;
        argi++;
        ret = _diag_show_forward_get_ipv6_header(cli, argc, argv, &argi, &g_diag_show_forward_config[argj].ipv6);
        if ( CHSM_E_SUCCESS != ret)
        {
            g_is_show_forward_cmd_runing = FALSE;
            return ret;
        }
        g_diag_show_forward_config[argj-1].ethernet.type = 0x86DD;
    }
#endif
    else if (!sal_strcmp (argv[argi], "arp") )
    {
        argj++;
        g_diag_show_forward_config[argj].header_type = DIAG_ARP_HDR;
        argi++; 
        /*  SHOW_FORWARD_IF_ERROR_RETURN(diag_show_forward_get_arp_header(cli, argc, argv, &i, &g_diag_show_forward_config[j].arp));*/
        g_diag_show_forward_config[argj-1].ethernet.type = 0x0806;
    }

    else if (!sal_strcmp (argv[argi], "mpls") )
    {
        argj++;
        g_diag_show_forward_config[argj].header_type = DIAG_MPLS_HDR;
        argi++; 
        g_diag_show_forward_config[argj-1].ethernet.type = 0x8847;
    }

    else if (!sal_strcmp (argv[argi], "bpdu") )
    {
        argi++; 
        /*MACDA = 01:80:c2:00:00:00 or 01:80:ce:00:00:08*/
        g_diag_show_forward_config[0].ethernet.mac_da[0] = 0x01;
        g_diag_show_forward_config[0].ethernet.mac_da[1] = 0x80;
        g_diag_show_forward_config[0].ethernet.mac_da[2] = 0xc2;
        g_diag_show_forward_config[0].ethernet.mac_da[3] = 0x00;
        g_diag_show_forward_config[0].ethernet.mac_da[4] = 0x00;
        g_diag_show_forward_config[0].ethernet.mac_da[5] = 0x00;
    }

    else if (!sal_strcmp (argv[argi], "eapol") )
    {
        argi++; 
        g_diag_show_forward_config[0].ethernet.type = 0x888e;
    }
    else if (!sal_strcmp (argv[argi], "ospf") )
    {
        argi++; 
        argj++;
        g_diag_show_forward_config[1].header_type = DIAG_IPV4_HDR;
        g_diag_show_forward_config[1].ipv4.ipv4_da = 0xe0000005; /*224.0.0.5*/
        g_diag_show_forward_config[1].ipv4.ipv4_proto = 89;
    }

    /*4, get l4 header*/
    if ( argi >= argc ) 
    {
        goto construct_packet;
    }
    if (!sal_strcmp (argv[argi], "tcp") )
    {
        argj++;
        g_diag_show_forward_config[argj].header_type = DIAG_TCP_HDR;
        argi++;
        ret = _diag_show_forward_get_tcp_header(cli, argc, argv, &argi,&g_diag_show_forward_config[argj].tcp);
        if ( CHSM_E_SUCCESS != ret)
        {
            g_is_show_forward_cmd_runing = FALSE;
            return ret;
        }
        if (  g_diag_show_forward_config[argj-1].header_type == DIAG_IPV4_HDR)
        {
            g_diag_show_forward_config[argj-1].ipv4.ipv4_proto = 6;
        }
        else 
        {
            g_diag_show_forward_config[argj-1].ipv6.ipv6_nxt_hdr = 6;
        }
    }
    else if (!sal_strcmp (argv[argi], "udp") )
    {
        argj++;
        g_diag_show_forward_config[argj].header_type = DIAG_UDP_HDR;
        argi++;
        ret = _diag_show_forward_get_udp_header(cli, argc, argv, &argi,&g_diag_show_forward_config[argj].udp);
        if ( CHSM_E_SUCCESS != ret)
        {
            g_is_show_forward_cmd_runing = FALSE;
            return ret;
        }
        if(  g_diag_show_forward_config[argj-1].header_type == DIAG_IPV4_HDR)
        {
            g_diag_show_forward_config[argj-1].ipv4.ipv4_proto = 17;
        }
        else 
        {
            g_diag_show_forward_config[argj-1].ipv6.ipv6_nxt_hdr = 17;
        }
    }
    else if (!sal_strcmp (argv[argi], "igmp") )
    {
        argj++;
        argi++;
        if(  g_diag_show_forward_config[argj-1].header_type == DIAG_IPV4_HDR)
        {
            g_diag_show_forward_config[argj-1].ipv4.ipv4_proto = 2;
        }
        else 
        {
            g_diag_show_forward_config[argj-1].ipv6.ipv6_nxt_hdr = 2;
        }
    }

    else if (!sal_strcmp (argv[argi], "icmp") )
    {
        argj++;
        argi++;
        if(  g_diag_show_forward_config[argj-1].header_type == DIAG_IPV4_HDR)
        {
            g_diag_show_forward_config[argj-1].ipv4.ipv4_proto = 1;
        }
        else 
        {
            g_diag_show_forward_config[argj-1].ipv6.ipv6_nxt_hdr = 1;
        }
    }

    else if (!sal_strcmp (argv[argi], "icmpv6") )
    {
        argj++;
        argi++;
        if(  g_diag_show_forward_config[argj-1].header_type == DIAG_IPV4_HDR)
        {
            g_diag_show_forward_config[argj-1].ipv4.ipv4_proto = 58;
        }
        else 
        {
            g_diag_show_forward_config[argj-1].ipv6.ipv6_nxt_hdr = 58;
        }
    }
    else if (!sal_strcmp (argv[argi], "pim-sm") )
    {
        argj++;
        argi++;
        if(  g_diag_show_forward_config[argj-1].header_type == DIAG_IPV4_HDR)
        {
            g_diag_show_forward_config[argj-1].ipv4.ipv4_proto = 103;
        }
        else 
        g_diag_show_forward_config[argj-1].ipv6.ipv6_nxt_hdr = 103;
        g_diag_show_forward_config[0].ethernet.mac_da[0] = 0x01; 
    }
    construct_packet:

    if( argi < argc ) 
    {
        if (!sal_strcmp (argv[argi], "payload") )
        {
            argi++; 
            CLI_GET_INTEGER_RANGE ("payload", temp, argv[argi], 1, 1500);
            ++argj;  
            g_diag_show_forward_config[argj].header_type = DIAG_PAYLOAD;
            g_diag_show_forward_config[argj].payload.pld_len = temp;
        }
        else
        {
            ++argj;  
            ++argi;
            g_diag_show_forward_config[argj].header_type = DIAG_PAYLOAD;
            g_diag_show_forward_config[argj].payload.pld_len = 100;
        }
    }
    else
    {
        ++argj;  
        ++argi;
        g_diag_show_forward_config[argj].header_type = DIAG_PAYLOAD;
        g_diag_show_forward_config[argj].payload.pld_len = 100;
    }

    /*5, construct packet from CLI */
    in_pkt.pkt =(uint8 *)XCALLOC(MTYPE_TMP, SHOW_FORWARD_MTU );
    if ( CHSM_E_SUCCESS != ret)
    {
        g_is_show_forward_cmd_runing = FALSE;
        return ret;
    }

    if ( NULL == in_pkt.pkt )
    {
        cli_out(cli, "\n%%Can not allocate memory\n");
        return CLI_ERROR;
    } 

    ret = diag_show_forward_construct_pkt(NULL, (uint8 *)emp_char, &in_pkt, 
                                    (diag_ctrl_var_t*)(&g_diag_show_forward_ctrl_var),(diag_config_t *)(&g_diag_show_forward_config));

    if ( CHSM_E_SUCCESS != ret)
    {
        g_is_show_forward_cmd_runing = FALSE;
        return ret;
    }
    in_pkt.chip_id = common_header.chip_id;
    in_pkt.chan_id = common_header.chan_id;  

    /*7, cast pkt to fwd*/
    ret = _diag_show_forward_do_ipe(cli, &in_pkt, lc);
    if ( CHSM_E_SUCCESS != ret)
    {
        g_is_show_forward_cmd_runing = FALSE;
        return ret;
    }
    g_is_show_forward_cmd_runing = FALSE;

    return CLI_SUCCESS; 
}

CLI(show_forward_set_pkt_from_file,
     show_forward_set_pkt_from_file_cmd,
    "show forward from INPUT_FILE_NAME",
    "Show information",
    "Forward process",
    "From file ",
    "Store input pkt file name")
{
    char* input = NULL;
    tbl_card_t *lc = tbl_card_get_default_card();
    FILE *ipe_inpkt = NULL;
    char   file_name[64] = {0};
    diag_pkt_t  cpkt;

    /*delete the logging file first*/ 
    remove(SHOW_FORWARD_LOGGING_FILE);

    sal_memset(&cpkt, 0, sizeof(diag_pkt_t));
    input = argv[0];

    sal_snprintf(file_name, 63, "%s/%s", ROOT_DIR_NAME, input);  

    ipe_inpkt = sal_fopen(file_name, "r");
    if (NULL == ipe_inpkt)
    {
        cli_out(cli,"open input file failed!\n");
        return CLI_SUCCESS;
    }

    /* cycle for packet processing */
    while(_diag_show_forward_get_one_pkt_from_file(ipe_inpkt, &cpkt))
    {
        SHOW_FORWARD_IF_ERROR_RETURN(_diag_show_forward_do_ipe(cli, &cpkt, lc));        
    }
    sal_fclose(ipe_inpkt);
    return CLI_SUCCESS;

}

void
chsm_show_forward_cli_init_debug (struct cli_tree *ctree)
{

    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &debug_show_forward_ipe_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &no_debug_show_forward_ipe_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &debug_show_forward_epe_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &no_debug_show_forward_epe_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &debug_show_forward_fwd_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &no_debug_show_forward_fwd_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &show_forward_cmodel_pkt_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &no_show_fwd_cmodel_pkt_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &debug_show_forward_oam_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &no_debug_show_forward_oam_cmd);
    cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN, &show_forward_info_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &show_forward_info_cmd);
    cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN, &show_forward_set_pkt_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &show_forward_set_pkt_cmd);
    cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN, &show_forward_set_pkt_from_file_cmd);
    cli_install_gen(ctree, INTERNAL_DEBUG_MODE, PRIVILEGE_NORMAL, 0, &show_forward_set_pkt_from_file_cmd);

}


