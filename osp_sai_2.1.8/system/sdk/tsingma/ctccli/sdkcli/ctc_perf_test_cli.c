/**
 @file ctc_perf_test_cli.c

 @date 2016-03-05

 @version v1.0

 The file applies cli to test sdk module of SDK
*/

#include "ctc_perf_test_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"

extern uint8 g_ctcs_api_en;

extern int32 ctcs_sdk_deinit(uint8 lchip);
extern int32 ctc_sdk_deinit();

enum cli_perf_op_type_e
{
    CLI_PERF_OP_TYPE_ADD,
    CLI_PERF_OP_TYPE_REMOVE,
    CLI_PERF_OP_TYPE_UPDATE,
    CLI_PERF_OP_TYPE_SWITCH,
    CLI_PERF_OP_TYPE_FLUSH,
    CLI_PERF_OP_TYPE_MAX
};
typedef enum cli_perf_op_type_e cli_perf_op_type_t;

enum cli_perf_nh_type_e
{
    CLI_PERF_NH_TYPE_IPUC,
    CLI_PERF_NH_TYPE_MPLS_TUNNEL,
    CLI_PERF_NH_TYPE_MPLS_NH,
    CLI_PERF_NH_TYPE_XLATE,
    CLI_PERF_NH_TYPE_IP_TUNNEL,
    CLI_PERF_NH_TYPE_MAX
};
typedef enum cli_perf_nh_type_e cli_perf_nh_type_t;
struct cli_perf_output_s
{
    char* buf;
    cli_perf_op_type_t op;
    sal_systime_t  tv_start;
    sal_systime_t  tv_end;
};
typedef struct cli_perf_output_s cli_perf_output_t;

#define CTC_CLI_PERF_CHECK_RETURN(ret)                                    \
    {                                                                      \
        if (ret < 0) {                                                     \
            ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));  \
            if((ret != CTC_E_HASH_CONFLICT) && (ret != CTC_E_HASH_CONFLICT) \
                && (ret != CTC_E_HASH_CONFLICT) && (ret != CTC_E_HASH_CONFLICT) \
                 && (ret != CTC_E_HASH_CONFLICT)) {                   \
            return CLI_ERROR; }}                                            \
    }


int32
_ctc_cli_perf_output(cli_perf_output_t* output)
{
    sal_systime_t* tv1;
    sal_systime_t* tv2;
    char* perf_op[CLI_PERF_OP_TYPE_MAX] = {"Add", "Remove", "Update", "Switch","Flush"};


    tv1 = &(output->tv_start);
    tv2 = &(output->tv_end);

    ctc_cli_out("%s, OP:%s\n", output->buf, perf_op[output->op]);
    ctc_cli_out("START TIME:[%10u s:%3u ms]\n", tv1->tv_sec, tv1->tv_usec/1000);
    ctc_cli_out("END   TIME:[%10u s:%3u ms]\n", tv2->tv_sec, tv2->tv_usec/1000);
    if(tv2->tv_usec >= tv1->tv_usec)
    {
        ctc_cli_out("USED  TIME:[%10u s:%3u ms]\n", tv2->tv_sec - tv1->tv_sec, (tv2->tv_usec - tv1->tv_usec)/1000);
    }
    else
    {
        ctc_cli_out("USED  TIME:[%10u s:%3u ms]\n", tv2->tv_sec - tv1->tv_sec - 1, (tv2->tv_usec + 1000000 - tv1->tv_usec)/1000);
    }

    return 0;
}

int32
_ctc_cli_perf_vlan_create_vlan(uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    ctc_vlan_uservlan_t user_vlan;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};


    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));

        for (i = 0; i < num; i++)
        {
            user_vlan.vlan_id = i+1;
            user_vlan.fid = user_vlan.vlan_id;
            user_vlan.user_vlanptr = user_vlan.vlan_id;

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_create_uservlan(g_api_lchip, &user_vlan);
            }
            else
            {
                ret = ctc_vlan_create_uservlan(&user_vlan);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_destroy_vlan(g_api_lchip, i+1);
            }
            else
            {
                ret = ctc_vlan_destroy_vlan(i+1);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"VLAN Entry:%5d", num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_vlan_create_vlan_default(uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    ctc_vlan_uservlan_t user_vlan;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    ctc_l2dflt_addr_t def;

    sal_memset(&def, 0, sizeof(def));

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));

        for (i = 0; i < num; i++)
        {
            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_create_vlan(g_api_lchip, i+1);
            }
            else
            {
                ret = ctc_vlan_create_vlan(i+1);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);

            def.fid = i+1;
            def.l2mc_grp_id = i+1;
            if(g_ctcs_api_en)
            {
                ret = ctcs_l2_add_default_entry(g_api_lchip, &def);
            }
            else
            {
                ret = ctc_l2_add_default_entry(&def);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_destroy_vlan(g_api_lchip, i+1);
            }
            else
            {
                ret = ctc_vlan_destroy_vlan(i+1);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);

            def.fid = i+1;
            if(g_ctcs_api_en)
            {
                ret = ctcs_l2_remove_default_entry(g_api_lchip, &def);
            }
            else
            {
                ret = ctc_l2_remove_default_entry(&def);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"VLAN And Default Entry:%5d", num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_vlan_add_port(uint32 vlan_num, uint32 port_num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    uint32 j = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[64] = {0};
    uint16 vlan_id = 0;
    uint16 gport = 0;
    ctc_l2dflt_addr_t l2dflt_addr;
    uint8 chip_type = 0;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    if(g_ctcs_api_en)
    {
        chip_type = ctcs_get_chip_type(g_api_lchip);
    }
    else
    {
        chip_type = ctc_get_chip_type();
    }

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (j = 0; j < port_num; j++)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(0, (j > 63) ? (256+j-64) : j);

            for (i = 0; i < vlan_num; i++)
            {
                vlan_id = i+1;
                if(g_ctcs_api_en)
                {
                    ret = ctcs_vlan_add_port(g_api_lchip, vlan_id, gport);
                }
                else
                {
                    ret = ctc_vlan_add_port(vlan_id, gport);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);

                if (CTC_CHIP_GREATBELT == chip_type)
                {
                    l2dflt_addr.fid = i+1;
                    l2dflt_addr.member.mem_port = gport;
                    if(g_ctcs_api_en)
                    {
                        ret = ctcs_l2_add_port_to_default_entry(g_api_lchip, &l2dflt_addr);
                    }
                    else
                    {
                        ret = ctc_l2_add_port_to_default_entry(&l2dflt_addr);
                    }
                    CTC_CLI_PERF_CHECK_RETURN(ret);
                }
            }
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (j = 0; j < port_num; j++)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(0, (j > 63) ? (256+j-64) : j);

            for (i = 0; i < vlan_num; i++)
            {
                vlan_id = i+1;
                if(g_ctcs_api_en)
                {
                    ret = ctcs_vlan_remove_port(g_api_lchip, vlan_id, gport);
                }
                else
                {
                    ret = ctc_vlan_remove_port(vlan_id, gport);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);

                if (CTC_CHIP_GREATBELT == chip_type)
                {
                    l2dflt_addr.fid = i+1;
                    l2dflt_addr.member.mem_port = gport;
                    if(g_ctcs_api_en)
                    {
                        ret = ctcs_l2_remove_port_from_default_entry(g_api_lchip, &l2dflt_addr);
                    }
                    else
                    {
                        ret = ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
                    }
                    CTC_CLI_PERF_CHECK_RETURN(ret);
                }
            }
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"VLAN PORT Entry: vlan:%5d  port:%5d", vlan_num, port_num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_vlan_class(ctc_vlan_class_t* vlan_class, uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    uint32 mac_tmp = 0;
    char* key_type[CTC_VLAN_CLASS_MAX] = {"MAC", "IPV4", "IPV6", "PROTOCOL"};

    mac_tmp = (vlan_class->vlan_class.mac[2] << 24) + (vlan_class->vlan_class.mac[3] << 16)  \
               + (vlan_class->vlan_class.mac[4] << 8) + (vlan_class->vlan_class.mac[5]);

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < num; i++)
        {
            vlan_class->vlan_id = ((vlan_class->vlan_id + 1)%4095) + 1;

            if(vlan_class->type == CTC_VLAN_CLASS_MAC)
            {
                mac_tmp = mac_tmp + 1;
                vlan_class->vlan_class.mac[5] = (mac_tmp & 0xFF);
                vlan_class->vlan_class.mac[4] = ((mac_tmp>>8) & 0xFF);
                vlan_class->vlan_class.mac[3] = ((mac_tmp>>16) & 0xFF);
                vlan_class->vlan_class.mac[2] = ((mac_tmp>>24) & 0xFF);
            }
            else if(vlan_class->type == CTC_VLAN_CLASS_IPV4)
            {
                if(CTC_FLAG_ISSET(vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    vlan_class->vlan_class.flex_ipv4.ipv4_sa = vlan_class->vlan_class.flex_ipv4.ipv4_sa + 1;
                }
                else
                {
                    vlan_class->vlan_class.ipv4_sa = vlan_class->vlan_class.ipv4_sa + 1;
                }
            }
            else if(vlan_class->type == CTC_VLAN_CLASS_IPV6)
            {
                if(CTC_FLAG_ISSET(vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    vlan_class->vlan_class.flex_ipv6.ipv6_sa[0] = vlan_class->vlan_class.flex_ipv6.ipv6_sa[0] + 1;
                }
                else
                {
                    vlan_class->vlan_class.ipv6_sa[0] = vlan_class->vlan_class.ipv6_sa[0] + 1;
                }
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_add_vlan_class(g_api_lchip, vlan_class);
            }
            else
            {
                ret = ctc_vlan_add_vlan_class(vlan_class);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            vlan_class->vlan_id = ((vlan_class->vlan_id + 1)%4095) + 1;

            if(vlan_class->type == CTC_VLAN_CLASS_MAC)
            {
                mac_tmp = mac_tmp + 1;
                vlan_class->vlan_class.mac[5] = (mac_tmp & 0xFF);
                vlan_class->vlan_class.mac[4] = ((mac_tmp>>8) & 0xFF);
                vlan_class->vlan_class.mac[3] = ((mac_tmp>>16) & 0xFF);
                vlan_class->vlan_class.mac[2] = ((mac_tmp>>24) & 0xFF);
            }
            else if(vlan_class->type == CTC_VLAN_CLASS_IPV4)
            {
                if(CTC_FLAG_ISSET(vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    vlan_class->vlan_class.flex_ipv4.ipv4_sa = vlan_class->vlan_class.flex_ipv4.ipv4_sa + 1;
                }
                else
                {
                    vlan_class->vlan_class.ipv4_sa = vlan_class->vlan_class.ipv4_sa + 1;
                }
            }
            else if(vlan_class->type == CTC_VLAN_CLASS_IPV6)
            {
                if(CTC_FLAG_ISSET(vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    vlan_class->vlan_class.flex_ipv6.ipv6_sa[0] = vlan_class->vlan_class.flex_ipv6.ipv6_sa[0] + 1;
                }
                else
                {
                    vlan_class->vlan_class.ipv6_sa[0] = vlan_class->vlan_class.ipv6_sa[0] + 1;
                }
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_remove_vlan_class(g_api_lchip, vlan_class);
            }
            else
            {
                ret = ctc_vlan_remove_vlan_class(vlan_class);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"VLAN CLASS %s Entry:%5d", key_type[vlan_class->type], num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_vlan_mapping(uint16 gport, ctc_vlan_mapping_t* vlan_map, uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    char* key_type[4] = {"NONE", "CVLAN", "SVLAN", "DOUBLE VLAN"};

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < num; i++)
        {
            vlan_map->new_svid = ((vlan_map->new_svid + 1)%4095) + 1;

            if(vlan_map->key & CTC_VLAN_MAPPING_KEY_SVID)
            {
                vlan_map->old_svid = ((vlan_map->old_svid + 1)%4095) + 1;
            }

            if(vlan_map->key & CTC_VLAN_MAPPING_KEY_CVID)
            {
                vlan_map->old_cvid = ((vlan_map->old_cvid + 1)%4095) + 1;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_add_vlan_mapping(g_api_lchip, gport, vlan_map);
            }
            else
            {
                ret = ctc_vlan_add_vlan_mapping(gport, vlan_map);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_UPDATE == op)
    {
        for (i = 0; i < num; i++)
        {
            vlan_map->new_svid = ((vlan_map->new_svid + 1)%4095) + 1;

            if(vlan_map->key & CTC_VLAN_MAPPING_KEY_SVID)
            {
                vlan_map->old_svid = ((vlan_map->old_svid + 1)%4095) + 1;
            }

            if(vlan_map->key & CTC_VLAN_MAPPING_KEY_CVID)
            {
                vlan_map->old_cvid = ((vlan_map->old_cvid + 1)%4095) + 1;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_update_vlan_mapping(g_api_lchip, gport, vlan_map);
            }
            else
            {
                ret = ctc_vlan_update_vlan_mapping(gport, vlan_map);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            vlan_map->new_svid = ((vlan_map->new_svid + 1)%4095) + 1;

            if(vlan_map->key & CTC_VLAN_MAPPING_KEY_SVID)
            {
                vlan_map->old_svid = ((vlan_map->old_svid + 1)%4095) + 1;
            }

            if(vlan_map->key & CTC_VLAN_MAPPING_KEY_CVID)
            {
                vlan_map->old_cvid = ((vlan_map->old_cvid + 1)%4095) + 1;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_remove_vlan_mapping(g_api_lchip, gport, vlan_map);
            }
            else
            {
                ret = ctc_vlan_remove_vlan_mapping(gport, vlan_map);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"VLAN MAPPING %s Entry:%5d", key_type[vlan_map->key], num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_l2_fdb(ctc_l2_addr_t* l2_addr, uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    uint32 mac_tmp = 0;
    uint32 max_phy_port = 0;
    uint16 lport = 0;
    uint16 lport_buf[CTC_MAX_PHY_PORT] = {0};
    uint8 start_index = 0xFF;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint8 chip_type = 0;

    mac_tmp = (l2_addr->mac[2] << 24) + (l2_addr->mac[3] << 16)  \
               + (l2_addr->mac[4] << 8) + (l2_addr->mac[5]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
        chip_type = ctcs_get_chip_type(g_api_lchip);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
        chip_type = ctc_get_chip_type();
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];

    if (CTC_CHIP_GOLDENGATE == chip_type)
    {
        lport = CTC_MAP_GPORT_TO_LPORT(l2_addr->gport);

        for (i = 0; i < max_phy_port; i++)
        {
            if (i < max_phy_port/2)
            {
                lport_buf[i] = i;
            }
            else
            {
                lport_buf[i] = i - max_phy_port/2 + 256;
            }

            if (lport == lport_buf[i])
            {
                start_index = i;
            }
        }

        if (0xFF == start_index)
        {
            ret = CTC_E_INVALID_PARAM;
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }


    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < num; i++)
        {
            /* l2_addr->fid = ((l2_addr->fid+1)%4095)+1;*/
            l2_addr->fid = l2_addr->fid;
            mac_tmp = mac_tmp + 1;
            l2_addr->mac[5] = (mac_tmp & 0xFF);
            l2_addr->mac[4] = ((mac_tmp>>8) & 0xFF);
            l2_addr->mac[3] = ((mac_tmp>>16) & 0xFF);
            l2_addr->mac[2] = ((mac_tmp>>24) & 0xFF);

            if (CTC_CHIP_GOLDENGATE == chip_type)
            {
                start_index = start_index + 1;
                lport = lport_buf[start_index % max_phy_port];
                l2_addr->gport = CTC_MAP_LPORT_TO_GPORT(0, lport);
            }
            else
            {
                /*l2_addr->gport = (l2_addr->gport + 1) % max_phy_port;*/
                l2_addr->gport = l2_addr->gport ;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_l2_add_fdb(g_api_lchip, l2_addr);
            }
            else
            {
                ret = ctc_l2_add_fdb(l2_addr);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            l2_addr->fid = ((l2_addr->fid+1)%4095)+1;

            mac_tmp = mac_tmp + 1;
            l2_addr->mac[5] = (mac_tmp & 0xFF);
            l2_addr->mac[4] = ((mac_tmp>>8) & 0xFF);
            l2_addr->mac[3] = ((mac_tmp>>16) & 0xFF);
            l2_addr->mac[2] = ((mac_tmp>>24) & 0xFF);

            if(g_ctcs_api_en)
            {
                ret = ctcs_l2_remove_fdb(g_api_lchip, l2_addr);
            }
            else
            {
                ret = ctc_l2_remove_fdb(l2_addr);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"L2 FDB Entry:%5d", num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;
}

int32
_ctc_cli_perf_l2_mcast(ctc_l2_mcast_addr_t* l2_mc,uint32 grp_num, uint32 mem_num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    uint32 j = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[64] = {0};
    uint32 mac_tmp = 0;

    mac_tmp = (l2_mc->mac[2] << 24) + (l2_mc->mac[3] << 16)  \
               + (l2_mc->mac[4] << 8) + (l2_mc->mac[5]);

    if(mem_num > 48)
    {
        ret = CTC_E_INVALID_PARAM;
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < grp_num; i++)
        {
            l2_mc->l2mc_grp_id = i+1;
            l2_mc->fid = ((l2_mc->fid+1)%4095)+1;

            mac_tmp = mac_tmp + 1;
            l2_mc->mac[5] = (mac_tmp & 0xFF);
            l2_mc->mac[4] = ((mac_tmp>>8) & 0xFF);
            l2_mc->mac[3] = ((mac_tmp>>16) & 0xFF);
            l2_mc->mac[2] = ((mac_tmp>>24) & 0xFF);

            if(g_ctcs_api_en)
            {
                ret = ctcs_l2mcast_add_addr(g_api_lchip, l2_mc);
            }
            else
            {
                ret = ctc_l2mcast_add_addr(l2_mc);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);

            for (j = 0; j < mem_num; j++)
            {
                l2_mc->member.mem_port = j;

                if(g_ctcs_api_en)
                {
                    ret = ctcs_l2mcast_add_member(g_api_lchip, l2_mc);
                }
                else
                {
                    ret = ctc_l2mcast_add_member(l2_mc);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);
            }
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < grp_num; i++)
        {
            l2_mc->fid = ((l2_mc->fid+1)%4095)+1;

            mac_tmp = mac_tmp + 1;
            l2_mc->mac[5] = (mac_tmp & 0xFF);
            l2_mc->mac[4] = ((mac_tmp>>8) & 0xFF);
            l2_mc->mac[3] = ((mac_tmp>>16) & 0xFF);
            l2_mc->mac[2] = ((mac_tmp>>24) & 0xFF);

            if(g_ctcs_api_en)
            {
                ret = ctcs_l2mcast_remove_addr(g_api_lchip, l2_mc);
            }
            else
            {
                ret = ctc_l2mcast_remove_addr(l2_mc);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"L2 MCAST Entry: group:%5d member:%5d", grp_num, mem_num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;
}

int32
_ctc_cli_perf_acl_add_install(ctc_acl_entry_t* acl_entry, uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    ctc_acl_group_info_t ginfo;


    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ginfo.dir = CTC_INGRESS;
        ginfo.type  = CTC_ACL_GROUP_TYPE_GLOBAL;
        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_create_group(g_api_lchip, 0, &ginfo);
        }
        else
        {
            ret = ctc_acl_create_group(0, &ginfo);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);

        for (i = 0; i < num; i++)
        {
            acl_entry->priority = sal_rand();
            acl_entry->entry_id = i;
            acl_entry->key.u.ipv4_key.ip_sa = acl_entry->key.u.ipv4_key.ip_sa + 1;

            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_add_entry(g_api_lchip, 0, acl_entry);
            }
            else
            {
                ret = ctc_acl_add_entry(0, acl_entry);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_install_entry(g_api_lchip, i);
            }
            else
            {
                ret = ctc_acl_install_entry(i);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            acl_entry->entry_id = i;
            acl_entry->key.u.ipv4_key.ip_sa = acl_entry->key.u.ipv4_key.ip_sa + 1;

            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_uninstall_entry(g_api_lchip, i);
            }
            else
            {
                ret = ctc_acl_uninstall_entry(i);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_remove_entry(g_api_lchip, i);
            }
            else
            {
                ret = ctc_acl_remove_entry(i);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_destroy_group(g_api_lchip, 0);
        }
        else
        {
            ret = ctc_acl_destroy_group(0);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"ACL ADD INSTALL Entry:%5d", num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_acl_add_all(ctc_acl_entry_t* acl_entry, uint32 num, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    ctc_acl_group_info_t ginfo;


    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));

    sal_gettime(&tv1);

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ginfo.dir = CTC_INGRESS;
        ginfo.type  = CTC_ACL_GROUP_TYPE_GLOBAL;
        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_create_group(g_api_lchip, 0, &ginfo);
        }
        else
        {
            ret = ctc_acl_create_group(0, &ginfo);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);

        for (i = 0; i < num; i++)
        {
            acl_entry->priority = sal_rand();
            acl_entry->entry_id = i;
            acl_entry->key.u.ipv4_key.ip_sa = acl_entry->key.u.ipv4_key.ip_sa + 1;

            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_add_entry(g_api_lchip, 0, acl_entry);
            }
            else
            {
                ret = ctc_acl_add_entry(0, acl_entry);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_install_group(g_api_lchip, 0, NULL);
        }
        else
        {
            ret = ctc_acl_install_group(0, NULL);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_uninstall_group(g_api_lchip, 0);
        }
        else
        {
            ret = ctc_acl_uninstall_group(0);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);
        for (i = 0; i < num; i++)
        {
            acl_entry->entry_id = i;
            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_remove_entry(g_api_lchip, i);
            }
            else
            {
                ret = ctc_acl_remove_entry(i);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_destroy_group(g_api_lchip, 0);
        }
        else
        {
            ret = ctc_acl_destroy_group(0);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_gettime(&tv2);

    sal_sprintf(str_buf,"ACL ADD ALL Entry:%5d", num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_aps_switch(cli_perf_op_type_t op, uint32 num)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    uint16 aps_group_id = 1;
    ctc_aps_bridge_group_t aps_group;

    /*creat aps group*/
    sal_memset(&aps_group, 0, sizeof(ctc_aps_bridge_group_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_create_aps_bridge_group(g_api_lchip, aps_group_id, &aps_group);
    }
    else
    {
        ret = ctc_aps_create_aps_bridge_group(aps_group_id, &aps_group);
    }
    CTC_CLI_PERF_CHECK_RETURN(ret);

    sal_gettime(&tv1);
    for (i = 0; i < num; i++)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_aps_set_aps_bridge(g_api_lchip, aps_group_id, 1);
        }
        else
        {
            ret = ctc_aps_set_aps_bridge(aps_group_id, 1);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }
    sal_gettime(&tv2);

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_destroy_aps_bridge_group(g_api_lchip, aps_group_id);
    }
    else
    {
        ret = ctc_aps_destroy_aps_bridge_group(aps_group_id);
    }
    CTC_CLI_PERF_CHECK_RETURN(ret);

    sal_sprintf(str_buf,"APS switch times:%d", num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}


int32
_ctc_cli_perf_nexthop_com(cli_perf_nh_type_t nh_type, uint32 nh_id_base, cli_perf_op_type_t op, uint32 num)
{
    int32 ret = CTC_E_NONE;
    uint32 nhid = 0;
    uint16 l3if_id = 0;
    ctc_l3if_t l3if;

    /*creat l3if*/
    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
    l3if_id = 1;
    l3if.gport = 1;
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

    if(CLI_PERF_OP_TYPE_ADD == op)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_create(l3if_id, &l3if);
        }
        if(ret < 0)
        {
            return ret;
        }
    }
    else if(CLI_PERF_OP_TYPE_REMOVE == op)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_destory(l3if_id, &l3if);
        }
        if(ret < 0)
        {
            return ret;
        }
    }

    /*nexthop*/
    if (CLI_PERF_NH_TYPE_IPUC == nh_type)
    {
        ctc_ip_nh_param_t ip_nh_param;
        sal_memset(&ip_nh_param, 0, sizeof(ctc_ip_nh_param_t));

        ip_nh_param.oif.gport = 1;

        ip_nh_param.mac[0] = 0x01;
        ip_nh_param.mac[1] = 0x02;
        ip_nh_param.mac[2] = 0x03;
        ip_nh_param.mac[3] = 0x04;
        ip_nh_param.mac[4] = 0x05;
        ip_nh_param.mac[5] = 0x00;

        for (nhid = nh_id_base; nhid < (nh_id_base + num); nhid++)
        {
            ip_nh_param.dsnh_offset = nhid;
            ip_nh_param.mac[4] =  ((nhid>>8) & 0xFF);
            ip_nh_param.mac[5]  = nhid & 0xFF;

            if (CLI_PERF_OP_TYPE_ADD == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_add_ipuc(g_api_lchip, nhid, &ip_nh_param);
                }
                else
                {
                    ret = ctc_nh_add_ipuc(nhid, &ip_nh_param);
                }
            }
            else if (CLI_PERF_OP_TYPE_REMOVE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_remove_ipuc(g_api_lchip, nhid);
                }
                else
                {
                    ret = ctc_nh_remove_ipuc(nhid);
                }
            }
            else if (CLI_PERF_OP_TYPE_UPDATE == op)
            {
                ip_nh_param.mac[0] = 0x08;
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_update_ipuc(g_api_lchip, nhid, &ip_nh_param);
                }
                else
                {
                    ret = ctc_nh_update_ipuc(nhid, &ip_nh_param);
                }
            }

            if (ret < 0)
            {
                goto Err;
            }
        }
    }
    else if (CLI_PERF_NH_TYPE_MPLS_TUNNEL == nh_type)
    {
        ctc_mpls_nexthop_tunnel_param_t mpls_tunnel_param;
        sal_memset(&mpls_tunnel_param, 0, sizeof(ctc_mpls_nexthop_tunnel_param_t));

        mpls_tunnel_param.nh_param.label_num = 1;
        mpls_tunnel_param.nh_param.mac[0] = 0x01;
        mpls_tunnel_param.nh_param.mac[1] = 0x02;
        mpls_tunnel_param.nh_param.mac[2] = 0x03;
        mpls_tunnel_param.nh_param.mac[3] = 0x04;
        mpls_tunnel_param.nh_param.mac[4] = 0x05;
        mpls_tunnel_param.nh_param.mac[5] = 0x00;

        mpls_tunnel_param.nh_param.oif.gport                = 1;
        mpls_tunnel_param.nh_param.tunnel_label[0].label    = 100;
        mpls_tunnel_param.nh_param.tunnel_label[0].ttl      = 255;

        if (CLI_PERF_OP_TYPE_UPDATE == op)
        {
            mpls_tunnel_param.nh_param.tunnel_label[0].label    = 1000;
            mpls_tunnel_param.nh_param.mac[0] = 0x08;
        }

        for (nhid = nh_id_base; nhid < (nh_id_base + num); nhid++)
        {
            mpls_tunnel_param.nh_param.tunnel_label[0].label++;

            if (CLI_PERF_OP_TYPE_ADD == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_add_mpls_tunnel_label(g_api_lchip, nhid, &mpls_tunnel_param);
                }
                else
                {
                    ret = ctc_nh_add_mpls_tunnel_label(nhid, &mpls_tunnel_param);
                }
            }
            else if (CLI_PERF_OP_TYPE_REMOVE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_remove_mpls_tunnel_label(g_api_lchip, nhid);
                }
                else
                {
                    ret = ctc_nh_remove_mpls_tunnel_label(nhid);
                }
            }
            else if (CLI_PERF_OP_TYPE_UPDATE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_update_mpls_tunnel_label(g_api_lchip, nhid, &mpls_tunnel_param);
                }
                else
                {
                    ret = ctc_nh_update_mpls_tunnel_label(nhid, &mpls_tunnel_param);
                }
            }

            if (ret < 0)
            {
                goto Err;
            }
        }

    }
    else if (CLI_PERF_NH_TYPE_MPLS_NH == nh_type)
    {
        ctc_mpls_nexthop_tunnel_param_t mpls_tunnel_param;
        ctc_mpls_nexthop_param_t  mpls_nh_param;
        /*creat tunnel*/
        if (CLI_PERF_OP_TYPE_ADD == op)
        {
            mpls_tunnel_param.nh_param.label_num = 1;
            mpls_tunnel_param.nh_param.mac[0] = 0x01;
            mpls_tunnel_param.nh_param.mac[1] = 0x02;
            mpls_tunnel_param.nh_param.mac[2] = 0x03;
            mpls_tunnel_param.nh_param.mac[3] = 0x04;
            mpls_tunnel_param.nh_param.mac[4] = 0x05;
            mpls_tunnel_param.nh_param.mac[5] = 0x00;

            mpls_tunnel_param.nh_param.oif.gport                = 1;
            mpls_tunnel_param.nh_param.tunnel_label[0].label    = 100;
            mpls_tunnel_param.nh_param.tunnel_label[0].ttl      = 255;

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_mpls_tunnel_label(g_api_lchip, 100, &mpls_tunnel_param);
            }
            else
            {
                ret = ctc_nh_add_mpls_tunnel_label(100, &mpls_tunnel_param);
            }
            if(ret < 0)
            {
                goto Err;
            }
        }

        /*op mpls nexthop*/
        mpls_nh_param.flag= 0;
        mpls_nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
        mpls_nh_param.logic_port = 300;
        mpls_nh_param.logic_port_valid = 1;

        mpls_nh_param.nh_para.nh_param_push.tunnel_id = 100;

        mpls_nh_param.nh_para.nh_param_push.label_num = 1;
        mpls_nh_param.nh_para.nh_param_push.push_label[0].ttl   = 64;
        mpls_nh_param.nh_para.nh_param_push.push_label[0].exp   = 1;
        mpls_nh_param.nh_para.nh_param_push.push_label[0].exp_type = 0;

        mpls_nh_param.nh_para.nh_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;
        mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;

        mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
        mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.output_svid = 1;

        CTC_SET_FLAG(mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);

        if(CLI_PERF_OP_TYPE_ADD == op)
        {
            mpls_nh_param.nh_para.nh_param_push.push_label[0].label = 100;
            mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.output_svid = 1;
        }

        if(CLI_PERF_OP_TYPE_UPDATE == op)
        {
            mpls_nh_param.nh_para.nh_param_push.push_label[0].label = 200;
            mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.output_svid = 3;
            mpls_nh_param.upd_type = CTC_NH_UPD_FWD_ATTR;
        }

        for (nhid = nh_id_base; nhid < (nh_id_base + num); nhid++)
        {
            mpls_nh_param.nh_para.nh_param_push.push_label[0].label ++;
            mpls_nh_param.nh_para.nh_param_push.nh_com.vlan_info.output_svid ++;

            if (CLI_PERF_OP_TYPE_ADD == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_add_mpls(g_api_lchip, nhid, &mpls_nh_param);
                }
                else
                {
                    ret = ctc_nh_add_mpls(nhid, &mpls_nh_param);
                }
            }
            else if (CLI_PERF_OP_TYPE_REMOVE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_remove_mpls(g_api_lchip, nhid);
                }
                else
                {
                    ret = ctc_nh_remove_mpls(nhid);
                }
            }
            else if (CLI_PERF_OP_TYPE_UPDATE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_update_mpls(g_api_lchip, nhid, &mpls_nh_param);
                }
                else
                {
                    ret = ctc_nh_update_mpls(nhid, &mpls_nh_param);
                }
            }

            if (ret < 0)
            {
                goto Err;
            }
        }


        /*destroy tunnel*/
        if (CLI_PERF_OP_TYPE_REMOVE == op)
        {
            ctc_nh_remove_mpls_tunnel_label(100);
        }
    }
    else if (CLI_PERF_NH_TYPE_XLATE == nh_type)
    {
        ctc_vlan_edit_nh_param_t  vlan_edit_param;
        sal_memset(&vlan_edit_param, 0, sizeof(ctc_vlan_edit_nh_param_t));

        vlan_edit_param.gport_or_aps_bridge_id = 1;
        vlan_edit_param.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
        vlan_edit_param.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;

        CTC_SET_FLAG(vlan_edit_param.vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
        vlan_edit_param.vlan_edit_info.output_svid    = 1;

        for (nhid = nh_id_base; nhid < (nh_id_base + num); nhid++)
        {
            vlan_edit_param.dsnh_offset = nhid;
            vlan_edit_param.vlan_edit_info.output_svid ++;
            if (CLI_PERF_OP_TYPE_ADD == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_add_xlate(g_api_lchip, nhid, &vlan_edit_param);
                }
                else
                {
                    ret = ctc_nh_add_xlate(nhid, &vlan_edit_param);
                }
            }
            else if (CLI_PERF_OP_TYPE_REMOVE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_remove_xlate(g_api_lchip, nhid);
                }
                else
                {
                    ret = ctc_nh_remove_xlate(nhid);
                }
            }
            else
            {
                ret = CTC_E_NOT_SUPPORT;
            }

            if (ret < 0)
            {
                goto Err;
            }
        }
    }
    else if (CLI_PERF_NH_TYPE_IP_TUNNEL == nh_type)
    {
        ctc_ip_tunnel_nh_param_t  ip_tunnel_param;
        sal_memset(&ip_tunnel_param, 0, sizeof(ctc_ip_tunnel_nh_param_t));

        ip_tunnel_param.upd_type = CTC_NH_UPD_FWD_ATTR;
        ip_tunnel_param.flag = 0;

        ip_tunnel_param.oif.gport = 1;

        ip_tunnel_param.mac[0] = 0x01;
        ip_tunnel_param.mac[1] = 0x02;
        ip_tunnel_param.mac[2] = 0x03;
        ip_tunnel_param.mac[3] = 0x04;
        ip_tunnel_param.mac[4] = 0x05;
        ip_tunnel_param.mac[5] = 0x06;

        ip_tunnel_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV6_IN4;
        ip_tunnel_param.tunnel_info.dscp_or_tos = 10;
        ip_tunnel_param.tunnel_info.dscp_select = CTC_NH_DSCP_SELECT_ASSIGN;
        ip_tunnel_param.tunnel_info.ttl         = 64;

        ip_tunnel_param.tunnel_info.ip_sa.ipv4  = 0x01010100;
        ip_tunnel_param.tunnel_info.ip_da.ipv4  = 0x02010100;

        if (CLI_PERF_OP_TYPE_UPDATE == op)
        {
            ip_tunnel_param.tunnel_info.ip_da.ipv4  = 0x03010100;
            ip_tunnel_param.mac[0] = 0x8;
        }

        for (nhid = nh_id_base; nhid < (nh_id_base + num); nhid++)
        {
            ip_tunnel_param.tunnel_info.ip_da.ipv4 ++;

            if (CLI_PERF_OP_TYPE_ADD == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_add_ip_tunnel(g_api_lchip, nhid, &ip_tunnel_param);
                }
                else
                {
                    ret = ctc_nh_add_ip_tunnel(nhid, &ip_tunnel_param);
                }
            }
            else if (CLI_PERF_OP_TYPE_REMOVE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_remove_ip_tunnel(g_api_lchip, nhid);
                }
                else
                {
                    ret = ctc_nh_remove_ip_tunnel(nhid);
                }
            }
            else if (CLI_PERF_OP_TYPE_UPDATE == op)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_update_ip_tunnel(g_api_lchip, nhid, &ip_tunnel_param);
                }
                else
                {
                    ret = ctc_nh_update_ip_tunnel(nhid, &ip_tunnel_param);
                }
            }

            if (ret < 0)
            {
                goto Err;
            }
        }
    }
Err:

    /*destroy l3if*/
    if(CLI_PERF_OP_TYPE_REMOVE == op)
    {
        ctc_l3if_destory(l3if_id, &l3if);
    }

    return ret;

}


int32
_ctc_cli_perf_nexthop_id(cli_perf_nh_type_t nexthop_type, uint32 nexthop_id_base, cli_perf_op_type_t op, uint32 num)
{
    int32 ret = CTC_E_NONE;
    sal_systime_t tv1,tv2;
    char str_buf[64] = {0};
    cli_perf_output_t output;
    char* nh_type_str[CLI_PERF_NH_TYPE_MAX] = {"IPUC", "MPLS tunnel", "MPLS", "Xlate", "IP tunnel"};

    sal_gettime(&tv1);
    ret = _ctc_cli_perf_nexthop_com(nexthop_type, nexthop_id_base, op, num);
    CTC_CLI_PERF_CHECK_RETURN(ret);
    sal_gettime(&tv2);

    sal_sprintf(str_buf,"NH type %s NH number:%d", nh_type_str[nexthop_type], num);

    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

int32
_ctc_cli_perf_random_data(uint32 num, uint32 range, uint32 **p_data, cli_perf_op_type_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 i  = 0;
    uint32 j  = 0;
    uint32 rand_data = 0;

    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        *p_data = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * num);
        if (NULL == *p_data)
        {
            ctc_cli_out("Alloc memory failed \n");
            return -1;
        }

        for(i=0; i<num; i++)
        {
            while(1)
            {
                /* get random data, range is 0-range */
                rand_data = sal_rand()%range;

                /* check, keep random data not in the same */
                for(j=0; j<i; j++)
                {
                    if((*(*p_data+j)) == rand_data)
                    {
                        break;
                    }
                }

                /* save random data in the p_data */
                if(j == i)
                {
                    (*(*p_data+i)) = rand_data;
                    break;
                }
            }
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        mem_free(*p_data);
    }

    return ret;
}


int32
_ctc_cli_perf_mpls_ilm(uint32 label_base, cli_perf_op_type_t op, uint32 num)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    char str_buf[32] = {0};
    ctc_mpls_ilm_t mpls_ilm;
    uint32 nh_base = 3;
    uint32 nh_num = 1000;

    typedef int32 (*CTC_MPLS_FN_t)(ctc_mpls_ilm_t* p_mpls_ilm);
    typedef int32 (*CTCS_MPLS_FN_t)(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);
    CTC_MPLS_FN_t ctc_mpls_fn[] =
    {
        ctc_mpls_add_ilm,
        ctc_mpls_del_ilm,
        ctc_mpls_update_ilm,
    };
    CTCS_MPLS_FN_t ctcs_mpls_fn[] =
    {
        ctcs_mpls_add_ilm,
        ctcs_mpls_del_ilm,
        ctcs_mpls_update_ilm,
    };
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
    mpls_ilm.type = CTC_MPLS_LABEL_TYPE_NORMAL;
    mpls_ilm.inner_pkt_type = CTC_MPLS_INNER_IPV4;
    if (CLI_PERF_OP_TYPE_UPDATE == op)
    {
        mpls_ilm.trust_outer_exp = 1;
    }

    sal_gettime(&tv1);
    for (i = 0; i < num; i++)
    {
        mpls_ilm.nh_id = (i % nh_num) + nh_base;
        mpls_ilm.label = label_base + i;
        mpls_ilm.pwid = 0;
        if(g_ctcs_api_en)
        {
            ret = ctcs_mpls_fn[op](g_api_lchip, &mpls_ilm);
        }
        else
        {
            ret = ctc_mpls_fn[op](&mpls_ilm);
        }
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }
    sal_gettime(&tv2);

    if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_sprintf(str_buf, "MPLS lable base:%d, number:%d", label_base, num);
    output.buf = str_buf;
    output.op = op;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    return ret;

}

static uint32 *p_rand_ipv4_data = NULL;
static uint32 rand_ipv4_entry_num = 0;
int32
_ctc_cli_perf_ipuc_ipv4_route_random(uint32 num, cli_perf_op_type_t op, ctc_ipuc_param_t *p_ipuc_info)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    char str_buf[32] = {0};
    cli_perf_output_t output;
    sal_systime_t tv1,tv2;
    uint32 nh_base = 10;
    uint32 range = 0;

    if(CLI_PERF_OP_TYPE_UPDATE == op)
    {
        ctc_cli_out("invalid parameter\n");
        return -1;
    }

    /* (remove num) = (add num) */
    if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        num = rand_ipv4_entry_num;
    }

    if(p_ipuc_info->masklen <= 8)
    {
        ctc_cli_out("random, only care mask 8-32bit\n");
        return -1;
    }
    else
    {
        range = (1<<(p_ipuc_info->masklen-8))-1;
        if(num>range)
        {
            ctc_cli_out("invalid parameter\n");
            return -1;
        }
    }

    /* 1. creat nexthop */
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    /* 2. get random data */
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ret = _ctc_cli_perf_random_data(num, range, &p_rand_ipv4_data, op);
        CTC_CLI_PERF_CHECK_RETURN(ret);

        /* recode add num */
        rand_ipv4_entry_num = num;
    }
    if (NULL == p_rand_ipv4_data)
    {
        return -1;
    }

    /* 3 test ipuc */
    sal_gettime(&tv1);
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < num; i++)
        {
            p_ipuc_info->nh_id = i + nh_base;
            p_ipuc_info->ip.ipv4 = ((*(p_rand_ipv4_data+i)) << (32 - p_ipuc_info->masklen));
            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_add(g_api_lchip, p_ipuc_info);
            }
            else
            {
                ret = ctc_ipuc_add(p_ipuc_info);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);

        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            p_ipuc_info->nh_id = i + nh_base;
            p_ipuc_info->ip.ipv4 = ((*(p_rand_ipv4_data+i)) << (32 - p_ipuc_info->masklen));
            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_remove(g_api_lchip, p_ipuc_info);
            }
            else
            {
                ret = ctc_ipuc_remove(p_ipuc_info);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    sal_gettime(&tv2);

    /* 4. free random data && remove nexthop */
    if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        _ctc_cli_perf_random_data(num, range, &p_rand_ipv4_data, op);
        /* clear add num */
        rand_ipv4_entry_num = 0;

        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_sprintf(str_buf, "Ipv4 Route Entry:%5d", num);
    output.buf = str_buf;
    output.tv_start = tv1;
    output.tv_end = tv2;
    output.op = op;
    _ctc_cli_perf_output(&output);

    return ret;
}

int32
_ctc_cli_perf_ipuc_ipv4_route(uint32 num, cli_perf_op_type_t op, ctc_ipuc_param_t *p_ipuc_info, uint8 random_cfg)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    ip_addr_t ipv4 = 0;
    char str_buf[32] = {0};
    cli_perf_output_t output;
    sal_systime_t tv1,tv2;
    uint32 nh_base = 10;

    if(1 == random_cfg)
    {
        ret = _ctc_cli_perf_ipuc_ipv4_route_random(num, op, p_ipuc_info);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }
    else
    {
        /* 1. creat nexthop */
        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);

        /* 2. test ipuc */
        ipv4 = p_ipuc_info->ip.ipv4;
        sal_gettime(&tv1);
        if ((CLI_PERF_OP_TYPE_ADD == op)||(CLI_PERF_OP_TYPE_UPDATE == op))
        {
            for (i = 0; i < num; i++)
            {
                p_ipuc_info->nh_id = i + nh_base;
                p_ipuc_info->ip.ipv4 = ipv4  + (i << (32 - p_ipuc_info->masklen));
                if(g_ctcs_api_en)
                {
                    ret = ctcs_ipuc_add(g_api_lchip, p_ipuc_info);
                }
                else
                {
                    ret = ctc_ipuc_add(p_ipuc_info);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);
            }
        }
        else if (CLI_PERF_OP_TYPE_REMOVE == op)
        {
            for (i = 0; i < num; i++)
            {
                p_ipuc_info->nh_id = i + nh_base;
                p_ipuc_info->ip.ipv4 = ipv4  + (i << (32 - p_ipuc_info->masklen));
                if(g_ctcs_api_en)
                {
                    ret = ctcs_ipuc_remove(g_api_lchip, p_ipuc_info);
                }
                else
                {
                    ret = ctc_ipuc_remove(p_ipuc_info);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);
            }
        }
        sal_gettime(&tv2);

        /* 3. remove nexthop */
        if (CLI_PERF_OP_TYPE_REMOVE == op)
        {
            ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }

        sal_sprintf(str_buf, "Ipv4 Route Entry:%5d", num);
        output.buf = str_buf;
        output.tv_start = tv1;
        output.tv_end = tv2;
        output.op = op;
        _ctc_cli_perf_output(&output);
    }

    return ret;
}

static uint32 *p_rand_ipv6_data = NULL;
static uint32 rand_ipv6_entry_num = 0;
/* ipv6 random, only care mask 48-64bit */
int32
_ctc_cli_perf_ipuc_ipv6_route_random(uint32 num, cli_perf_op_type_t op, ctc_ipuc_param_t *p_ipuc_info)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    ipv6_addr_t ipv6_add = {0};
    char str_buf[32] = {0};
    cli_perf_output_t output;
    sal_systime_t tv1,tv2;
    uint8 mask_len = 0;
    uint32 nh_base = 10;
    uint32 range = 0;

    mask_len = p_ipuc_info->masklen;
    ipv6_add[0] = p_ipuc_info->ip.ipv6[0];
    ipv6_add[1] = p_ipuc_info->ip.ipv6[1];
    ipv6_add[2] = p_ipuc_info->ip.ipv6[2];
    ipv6_add[3] = p_ipuc_info->ip.ipv6[3];

    if(CLI_PERF_OP_TYPE_UPDATE == op)
    {
        ctc_cli_out("invalid parameter\n");
        return -1;
    }

    /* (remove num) = (add num) */
    if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        num = rand_ipv6_entry_num;
    }

    if((mask_len<=48)||(mask_len>64))
    {
        ctc_cli_out("random, only care mask 49-64bit\n");
        return -1;
    }

    /* get random data range, only care mask 48-64bit */
    if((mask_len>32)&&(mask_len<=64))
    {
        if(32 == (mask_len-32))
        {
            range = 0xffffffff;
        }
        else
        {
            range = (1<<(mask_len-32))-1;
        }
    }

    /* 1. creat nexthop */
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    /* 2. get random data */
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        ret = _ctc_cli_perf_random_data(num, range, &p_rand_ipv6_data, op);
        CTC_CLI_PERF_CHECK_RETURN(ret);
        rand_ipv6_entry_num = num;
    }
    if (NULL == p_rand_ipv6_data)
    {
        return -1;
    }

    /* 3. test ipuc*/
    sal_gettime(&tv1);
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < num; i++)
        {
            p_ipuc_info->nh_id = i + nh_base;
            if(mask_len<=32)
            {
                p_ipuc_info->ip.ipv6[0] = ipv6_add[0] + (i<<(32-mask_len));
            }
            else if((mask_len>32)&&(mask_len<=64))
            {
                p_ipuc_info->ip.ipv6[1] = ((*(p_rand_ipv6_data+i)) << (64-mask_len));
            }
            else if((mask_len>64)&&(mask_len<=96))
            {
                p_ipuc_info->ip.ipv6[2] = ipv6_add[2] + (i<<(96-mask_len));
            }
            else if((mask_len>96)&&(mask_len<=128))
            {
                p_ipuc_info->ip.ipv6[3] = ipv6_add[3] + (i<<(128-mask_len));
            }
            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_add(g_api_lchip, p_ipuc_info);
            }
            else
            {
                ret = ctc_ipuc_add(p_ipuc_info);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < num; i++)
        {
            p_ipuc_info->nh_id = i + nh_base;
            if(mask_len<=32)
            {
                p_ipuc_info->ip.ipv6[0] = ipv6_add[0] + (i<<(32-mask_len));
            }
            else if((mask_len>32)&&(mask_len<=64))
            {
                p_ipuc_info->ip.ipv6[1] = ((*(p_rand_ipv6_data+i)) << (64-mask_len));
            }
            else if((mask_len>64)&&(mask_len<=96))
            {
                p_ipuc_info->ip.ipv6[2] = ipv6_add[2] + (i<<(96-mask_len));
            }
            else if((mask_len>96)&&(mask_len<=128))
            {
                p_ipuc_info->ip.ipv6[3] = ipv6_add[3] + (i<<(128-mask_len));
            }
            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_remove(g_api_lchip, p_ipuc_info);
            }
            else
            {
                ret = ctc_ipuc_remove(p_ipuc_info);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    sal_gettime(&tv2);

    /* 4. free random data && remove nexthop */
    if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        _ctc_cli_perf_random_data(num, range, &p_rand_ipv6_data, op);
        rand_ipv6_entry_num = 0;

        ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
        CTC_CLI_PERF_CHECK_RETURN(ret);
    }

    sal_sprintf(str_buf, "Ipv6 Route Entry:%5d", num);
    output.buf = str_buf;
    output.tv_start = tv1;
    output.tv_end = tv2;
    output.op = op;
    _ctc_cli_perf_output(&output);

    return ret;
}

int32
_ctc_cli_perf_ipuc_ipv6_route(uint32 num, cli_perf_op_type_t op, ctc_ipuc_param_t *p_ipuc_info, uint8 random_cfg)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    ipv6_addr_t ipv6_add = {0};
    char str_buf[32] = {0};
    cli_perf_output_t output;
    sal_systime_t tv1,tv2;
    uint8 mask_len = 0;
    uint32 nh_base = 10;

    /* 2. test */
    if(1 == random_cfg)
    {
        _ctc_cli_perf_ipuc_ipv6_route_random(num, op, p_ipuc_info);
    }
    else
    {
        mask_len = p_ipuc_info->masklen;
        ipv6_add[0] = p_ipuc_info->ip.ipv6[0];
        ipv6_add[1] = p_ipuc_info->ip.ipv6[1];
        ipv6_add[2] = p_ipuc_info->ip.ipv6[2];
        ipv6_add[3] = p_ipuc_info->ip.ipv6[3];

        /* 1. creat nexthop */
        if ((CLI_PERF_OP_TYPE_ADD == op)||(CLI_PERF_OP_TYPE_UPDATE == op))
        {
            ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }

        /* 2. test ipuc */
        sal_gettime(&tv1);
        if ((CLI_PERF_OP_TYPE_ADD == op)||(CLI_PERF_OP_TYPE_UPDATE == op))
        {
            for (i = 0; i < num; i++)
            {
                p_ipuc_info->nh_id = i + nh_base;
                if(mask_len<=32)
                {
                    p_ipuc_info->ip.ipv6[0] = ipv6_add[0] + (i<<(32-mask_len));
                }
                else if((mask_len>32)&&(mask_len<=64))
                {
                    p_ipuc_info->ip.ipv6[1] = ipv6_add[1] + (i<<(64-mask_len));
                }
                else if((mask_len>64)&&(mask_len<=96))
                {
                    p_ipuc_info->ip.ipv6[2] = ipv6_add[2] + (i<<(96-mask_len));
                }
                else if((mask_len>96)&&(mask_len<=128))
                {
                    p_ipuc_info->ip.ipv6[3] = ipv6_add[3] + (i<<(128-mask_len));
                }
                if(g_ctcs_api_en)
                {
                    ret = ctcs_ipuc_add(g_api_lchip, p_ipuc_info);
                }
                else
                {
                    ret = ctc_ipuc_add(p_ipuc_info);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);
            }
        }
        else if (CLI_PERF_OP_TYPE_REMOVE == op)
        {
            for (i = 0; i < num; i++)
            {
                p_ipuc_info->nh_id = i + nh_base;
                if(mask_len<=32)
                {
                    p_ipuc_info->ip.ipv6[0] = ipv6_add[0] + (i<<(32-mask_len));
                }
                else if((mask_len>32)&&(mask_len<=64))
                {
                    p_ipuc_info->ip.ipv6[1] = ipv6_add[1] + (i<<(64-mask_len));
                }
                else if((mask_len>64)&&(mask_len<=96))
                {
                    p_ipuc_info->ip.ipv6[2] = ipv6_add[2] + (i<<(96-mask_len));
                }
                else if((mask_len>96)&&(mask_len<=128))
                {
                    p_ipuc_info->ip.ipv6[3] = ipv6_add[3] + (i<<(128-mask_len));
                }
                if(g_ctcs_api_en)
                {
                    ret = ctcs_ipuc_remove(g_api_lchip, p_ipuc_info);
                }
                else
                {
                    ret = ctc_ipuc_remove(p_ipuc_info);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);
            }
        }
        sal_gettime(&tv2);

        /* 3. remove nexthop */
        if (CLI_PERF_OP_TYPE_REMOVE == op)
        {
            ret = _ctc_cli_perf_nexthop_com(CLI_PERF_NH_TYPE_IPUC, nh_base, op, num);
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }

        sal_sprintf(str_buf, "Ipv6 Route Entry:%5d", num);
        output.buf = str_buf;
        output.tv_start = tv1;
        output.tv_end = tv2;
        output.op = op;
        _ctc_cli_perf_output(&output);
    }

    return ret;
}

int32
_ctc_cli_perf_ipmc(uint32 group_num, uint32 mem_num, cli_perf_op_type_t op, ctc_ipmc_group_info_t *p_ipmc_info)
{
    int32 ret = CLI_SUCCESS;
    uint32 i = 0;
    uint32 j = 0;
    ip_addr_t group_addr = 0;
    ip_addr_t src_addr = 0;
    ctc_l3if_t l3if;
    uint16 l3if_id = 0;
    char str_buf[32] = {0};
    cli_perf_output_t output;
    sal_systime_t tv1,tv2;

    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));

    /* 1.cread l3if */
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        for (i = 0; i < mem_num; i++)
        {
            l3if.gport = i;
            l3if_id    = i+1;
            if(g_ctcs_api_en)
            {
                ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_create(l3if_id, &l3if);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    /* 2.test */
    group_addr = p_ipmc_info->address.ipv4.group_addr;
    src_addr   = p_ipmc_info->address.ipv4.src_addr;

    sal_gettime(&tv1);
    if (CLI_PERF_OP_TYPE_ADD == op)
    {
        /* add group */
        for (i = 0; i < group_num; i++)
        {
            p_ipmc_info->group_id = i+1;
            p_ipmc_info->address.ipv4.group_addr = group_addr  + (i << (32 - p_ipmc_info->group_ip_mask_len));
            p_ipmc_info->address.ipv4.src_addr   = src_addr    + (i << (32 - p_ipmc_info->src_ip_mask_len));
            if(g_ctcs_api_en)
            {
                ret = ctcs_ipmc_add_group(g_api_lchip, p_ipmc_info);
            }
            else
            {
                ret = ctc_ipmc_add_group(p_ipmc_info);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);

            /* add nember */
            p_ipmc_info->member_number = 1;
            for (j = 0; j < mem_num; j++)
            {
                p_ipmc_info->ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF;
                p_ipmc_info->ipmc_member[0].global_port = j;
                if(g_ctcs_api_en)
                {
                    ret = ctcs_ipmc_add_member(g_api_lchip, p_ipmc_info);
                }
                else
                {
                    ret = ctc_ipmc_add_member(p_ipmc_info);
                }
                CTC_CLI_PERF_CHECK_RETURN(ret);
            }
        }
    }
    else if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        /* remove group */
        for (i = 0; i < group_num; i++)
        {
            p_ipmc_info->group_id = i+1;
            p_ipmc_info->address.ipv4.group_addr = group_addr  + (i << (32 - p_ipmc_info->group_ip_mask_len));
            p_ipmc_info->address.ipv4.src_addr   = src_addr    + (i << (32 - p_ipmc_info->src_ip_mask_len));
            if(g_ctcs_api_en)
            {
                ret = ctcs_ipmc_remove_group(g_api_lchip, p_ipmc_info);
            }
            else
            {
                ret = ctc_ipmc_remove_group(p_ipmc_info);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }
    sal_gettime(&tv2);

    /* 3. remove l3if */
    if (CLI_PERF_OP_TYPE_REMOVE == op)
    {
        for (i = 0; i < mem_num; i++)
        {
            l3if.gport = i;
            l3if_id    = i+1;
            if(g_ctcs_api_en)
            {
                ret = ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_destory(l3if_id, &l3if);
            }
            CTC_CLI_PERF_CHECK_RETURN(ret);
        }
    }

    sal_sprintf(str_buf, "Ipmc Group_num:%d  Mem_num:%d", group_num, mem_num);
    output.buf = str_buf;
    output.tv_start = tv1;
    output.tv_end = tv2;
    output.op = op;
    _ctc_cli_perf_output(&output);

    return ret;
}

CTC_CLI(ctc_cli_perf_vlan_create_vlan,
        ctc_cli_perf_vlan_create_vlan_cmd,
        "perf-test vlan (create|remove) default-entry vlan-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_VLAN_M_STR,
        "Create",
        "Remove",
        "Default entry",
        "Vlan number",
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;


    CTC_CLI_GET_UINT32_RANGE("vlan-num", number, argv[1], 0, CTC_MAX_UINT32_VALUE);

    if (0 == sal_memcmp("c", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    ret = _ctc_cli_perf_vlan_create_vlan(number, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_vlan_create_vlan_add_default,
        ctc_cli_perf_vlan_create_vlan_add_default_cmd,
        "perf-test vlan (create|remove) vlan-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_VLAN_M_STR,
        "Create",
        "Remove",
        "Vlan number",
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;


    CTC_CLI_GET_UINT32_RANGE("vlan-num", number, argv[1], 0, CTC_MAX_UINT32_VALUE);

    if (0 == sal_memcmp("c", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    ret = _ctc_cli_perf_vlan_create_vlan_default(number, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_vlan_add_port,
        ctc_cli_perf_vlan_add_port_cmd,
        "perf-test vlan vlan-num NUMBER (add|del) port-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan number",
        CTC_CLI_VLAN_RANGE_DESC,
        "Add",
        "Delete",
        "Port number",
        "Number")
{
    int32 ret = CLI_SUCCESS;
    uint32 vlan_num = 0;
    uint8 port_num = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;


    CTC_CLI_GET_UINT32_RANGE("vlan-num", vlan_num, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT8_RANGE("port-num", port_num, argv[2], 0, CTC_MAX_UINT8_VALUE);

    if (0 == sal_memcmp("a", argv[1], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("d", argv[1], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    ret = _ctc_cli_perf_vlan_add_port(vlan_num, port_num, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_vlan_class,
        ctc_cli_perf_vlan_class_cmd,
        "perf-test vlan classifier (add|remove) (mac macsa MAC |ipv4 ip-sa A.B.C.D | ipv6 ip-sa X:X::X:X) vlan VLAN_ID entry-num NUMBER (use-flex|)",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add",
        "Remove",
        "Mac based vlan class",
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        "IPv4 based vlan class",
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 based vlan class",
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Use tcam key")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    ctc_vlan_class_t vlan_class;
    uint8 index = 0xFF;
    ipv6_addr_t ipv6_address;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));


    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != index)
    {
        vlan_class.type = CTC_VLAN_CLASS_MAC;
        CTC_CLI_GET_MAC_ADDRESS("source mac address", vlan_class.vlan_class.mac, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (0xFF != index)
    {
        vlan_class.type = CTC_VLAN_CLASS_IPV4;
        if(CTC_FLAG_ISSET(vlan_class.flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 2]);
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
        }
        else
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.ipv4_sa, argv[index + 2]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        vlan_class.type = CTC_VLAN_CLASS_IPV6;
        if(CTC_FLAG_ISSET(vlan_class.flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
        }
        else
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 2]);
            vlan_class.vlan_class.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
        }
    }

    CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_class.vlan_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[5], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_perf_vlan_class(&vlan_class, number, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_vlan_mapping_add,
        ctc_cli_perf_vlan_mapping_add_cmd,
        "perf-test vlan mapping (add|update) port GPORT_ID {svid VLAN_ID | cvid VLAN_ID} mapping-to new-svid VLAN_ID entry-num NUMBER (use-flex|)",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Add",
        "Update",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Mapping to",
        "Mapped svlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Use tcam key")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    ctc_vlan_mapping_t vlan_mapping;
    uint8 index = 0xFF;
    uint16 gport = 0;
    uint8 num_index = 3;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));


    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("u", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_UPDATE;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[1], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
        num_index = num_index + 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        num_index = num_index + 2;
    }


    CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mapping.new_svid, argv[num_index - 1], 0, CTC_MAX_UINT16_VALUE);
    vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;

    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[num_index], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_perf_vlan_mapping(gport, &vlan_mapping, number, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_vlan_mapping_remove,
        ctc_cli_perf_vlan_mapping_remove_cmd,
        "perf-test vlan mapping remove port GPORT_ID {svid VLAN_ID | cvid VLAN_ID} entry-num NUMBER (use-flex|)",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Remove",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Use tcam key")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    ctc_vlan_mapping_t vlan_mapping;
    uint8 index = 0xFF;
    uint16 gport = 0;
    uint8 num_index = 1;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    operation = CLI_PERF_OP_TYPE_REMOVE;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
        num_index = num_index + 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        num_index = num_index + 2;
    }

    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[num_index], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_perf_vlan_mapping(gport, &vlan_mapping, number, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_l2_fdb_add,
        ctc_cli_perf_l2_fdb_add_cmd,
        "perf-test l2 fdb add mac MAC fid FID port GPORT_ID entry-num NUMBER (static  |)",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Add",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Static fdb table")
{
    int32 ret = CLI_SUCCESS;
    ctc_l2_addr_t l2_addr;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    uint8 index = 0xFF;
    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

    operation = CLI_PERF_OP_TYPE_ADD;
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);
    CTC_CLI_GET_UINT16_RANGE("fid", l2_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("src-port", l2_addr.gport, argv[2], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT32_RANGE("number", number, argv[3], 0, CTC_MAX_UINT32_VALUE);
     index = CTC_CLI_GET_ARGC_INDEX("static");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_IS_STATIC;
    }

    ret = _ctc_cli_perf_l2_fdb(&l2_addr, number, operation);

    return ret;

}

CTC_CLI(ctc_cli_perf_l2_fdb_remove,
        ctc_cli_perf_l2_fdb_remove_cmd,
        "perf-test l2 fdb remove mac MAC fid FID entry-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Remove",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Entry number",
        "<0-0xFFFFFFFF>")
{
    int32 ret = CLI_SUCCESS;
    ctc_l2_addr_t l2_addr;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;

    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

    operation = CLI_PERF_OP_TYPE_REMOVE;
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);
    CTC_CLI_GET_UINT16_RANGE("fid", l2_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT32_RANGE("number", number, argv[2], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_perf_l2_fdb(&l2_addr, number, operation);

    return ret;

}
CTC_CLI(ctc_cli_perf_l2_fdb_flush,
        ctc_cli_perf_l2_fdb_flush_cmd,
        "perf-test l2 fdb flush by (mac MAC | mac-fid MAC FID  | port GPORT_ID | fid FID | port-fid GPORT_ID FID | all) (is-logic-port|) (static | dynamic | local-dynamic | all)",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Flush fdb",
        "Query condition",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Mac+FID",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Port+FID",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_ID_DESC,
        "All condition",
        "Is logic port",
        "Static fdb table",
        "All dynamic fdb table",
        "Local chip dynamic fdb table",
        "Static and dynamic")
{
    uint8 index   = 0;
    int32    ret  = CLI_SUCCESS;
    sal_systime_t tv1,tv2;
    cli_perf_output_t output;
    ctc_l2_flush_fdb_t Flush;
    char str_buf[32] = {0};
    sal_memset(&Flush, 0, sizeof(ctc_l2_flush_fdb_t));

    Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Flush.flush_flag = CTC_L2_FDB_ENTRY_ALL;

    /*1) parse MAC address */

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;
        index++;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Flush.mac, argv[index]);

    }

    /*2) mac+fid */
    index = CTC_CLI_GET_ARGC_INDEX("mac-fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
        index++;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Flush.mac, argv[index]);
        index++;
        CTC_CLI_GET_UINT16_RANGE("vlan id",  Flush.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3) parse PORT address */

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        index++;
        CTC_CLI_GET_UINT16_RANGE("gport", Flush.gport, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*4)parse fid */

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        index++;
        CTC_CLI_GET_UINT16_RANGE("vlan id",  Flush.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*5)port+fid*/
    index = CTC_CLI_GET_ARGC_INDEX("port-fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        CTC_CLI_GET_UINT16_RANGE("gport", Flush.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("fid", Flush.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /* is logic port*/
    index = CTC_CLI_GET_ARGC_INDEX("is-logic-port");
    if (index != 0xFF)
    {
        Flush.use_logic_port = 1;
    }

    /*6) parse static|dynamic */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-dynamic");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC;
    }
    sal_gettime(&tv1);
    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_flush_fdb(g_api_lchip, &Flush);
    }
    else
    {
        ret = ctc_l2_flush_fdb(&Flush);
    }
    sal_gettime(&tv2);
    sal_sprintf(str_buf,"Flush L2 FDB Entry");
    output.buf = str_buf;
    output.op = CLI_PERF_OP_TYPE_FLUSH;
    output.tv_start = tv1;
    output.tv_end = tv2;
    _ctc_cli_perf_output(&output);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_perf_l2_mcast,
        ctc_cli_perf_l2_mcast_cmd,
        "perf-test l2 mcast (add|remove) member mac MAC fid FID group-num NUMBER mem-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_L2_M_STR,
        "Multicast",
        "Add",
        "Remove",
        "The member of multicast group",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Group number",
        "<0-512>",
        "Member number",
        "<0-48>")
{
    int32 ret = CLI_SUCCESS;
    ctc_l2_mcast_addr_t l2mc_addr;
    uint32 group_number = 0;
    uint32 member_number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;

    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));

    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    CTC_CLI_GET_MAC_ADDRESS("mac address", l2mc_addr.mac, argv[1]);
    CTC_CLI_GET_UINT16_RANGE("fid", l2mc_addr.fid, argv[2], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT32_RANGE("group number", group_number, argv[3], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("member number", member_number, argv[4], 0, CTC_MAX_UINT32_VALUE);


    ret = _ctc_cli_perf_l2_mcast(&l2mc_addr, group_number, member_number, operation);

    return ret;

}

CTC_CLI(ctc_cli_perf_acl_add_install,
        ctc_cli_perf_acl_add_install_cmd,
        "perf-test acl (add-install|unstall-remove) ipv4-entry ip-sa A.B.C.D entry-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_ACL_STR,
        "Add and install",
        "Unstall and remove",
        "Ipv4 double entry",
        "Ipv4 Sa",
        CTC_CLI_IPV4_FORMAT,
        "Entry number",
        "<0-0xFFFFFFFF>")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    ctc_acl_entry_t acl_entry;
    ctc_acl_ipv4_key_t* p_key = &acl_entry.key.u.ipv4_key;
    ctc_acl_action_t* p_action = &acl_entry.action;


    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));

    acl_entry.key.type = CTC_ACL_KEY_IPV4;
    acl_entry.entry_id = 0;
    acl_entry.priority_valid = 1;
    acl_entry.priority = 0xFFFFFFFF;
    p_key->key_size = CTC_ACL_KEY_SIZE_SINGLE;

    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("u", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    p_key->eth_type = 0x0800;
    p_key->eth_type_mask = 0xFFFF;
    CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);

    CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[1]);                                \
    sal_memset(&p_key->ip_sa_mask, 0xFF, sizeof(ip_addr_t));                     \
    CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);

    CTC_SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DISCARD);

    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[2], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_perf_acl_add_install(&acl_entry, number, operation);

    return ret;
}

CTC_CLI(ctc_cli_perf_acl_add_all,
        ctc_cli_perf_acl_add_all_cmd,
        "perf-test acl (add-all|remove-all) ipv4-entry ip-sa A.B.C.D entry-num NUMBER ",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_ACL_STR,
        "Add and install",
        "Unstall and remove",
        "Ipv4 double entry",
        "Ipv4 Sa",
        CTC_CLI_IPV4_FORMAT,
        "Entry number",
        "<0-0xFFFFFFFF>")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    ctc_acl_entry_t acl_entry;
    ctc_acl_ipv4_key_t* p_key = &acl_entry.key.u.ipv4_key;
    ctc_acl_action_t* p_action = &acl_entry.action;


    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));

    acl_entry.key.type = CTC_ACL_KEY_IPV4;
    acl_entry.entry_id = 0;
    acl_entry.priority_valid = 1;
    acl_entry.priority = 0xFFFFFFFF;
    p_key->key_size = CTC_ACL_KEY_SIZE_SINGLE;

    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    p_key->eth_type = 0x0800;
    p_key->eth_type_mask = 0xFFFF;
    CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);

    CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[1]);                                \
    sal_memset(&p_key->ip_sa_mask, 0xFF, sizeof(ip_addr_t));                     \
    CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);

    CTC_SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DISCARD);

    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[2], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_perf_acl_add_all(&acl_entry, number, operation);

    return ret;
}


CTC_CLI(ctc_cli_perf_mpls,
        ctc_cli_perf_mpls_cmd,
        "perf-test mpls (add|update|remove) ilm LABEL label-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_MPLS_M_STR,
        "Add",
        "Update",
        "Remove",
        "Incoming label mapping",
        "Mpls label base",
        "Mpls label number",
        "Mpls label number")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    uint32 label = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;

    CTC_CLI_GET_UINT32_RANGE("label", label, argv[1], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("label-num", number, argv[2], 0, CTC_MAX_UINT32_VALUE);

    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("u", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_UPDATE;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    ret = _ctc_cli_perf_mpls_ilm(label, operation, number);

    return ret;
}

CTC_CLI(ctc_cli_perf_aps,
        ctc_cli_perf_aps_cmd,
        "perf-test aps switch-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_APS_M_STR,
        "APS switch times",
        "APS switch times")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_SWITCH;

    CTC_CLI_GET_UINT32_RANGE("switch-num", number, argv[0], 0, CTC_MAX_UINT32_VALUE);
    ret = _ctc_cli_perf_aps_switch(operation, number);

    return ret;
}

CTC_CLI(ctc_cli_perf_nexthop,
        ctc_cli_perf_nexthop_cmd,
        "perf-test nexthop (add|update|remove) (ipuc | mpls-tunnel | mpls-nexthop | xlate | ip-tunnel) entry-num NUMBER",
        CTC_CLI_PERF_TEST_STR,
        CTC_CLI_NH_M_STR,
        "Add",
        "Update",
        "Remove",
        "IP unicast nexthop",
        "MPLS Tunnel",
        "MPLS nexthop",
        "Egress Vlan Editing nexthop",
        "IP Tunnel nexthop",
        "Nexthop entry number",
        "Nexthop entry number")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;
    cli_perf_nh_type_t nexthop_type = 0;

    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[2], 0, CTC_MAX_UINT32_VALUE);

    /*get action op type*/
    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("u", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_UPDATE;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    /*get nh type*/
    if (0 == sal_strcmp("ipuc", argv[1]))
    {
        nexthop_type = CLI_PERF_NH_TYPE_IPUC;
    }
    else if (0 == sal_strcmp("mpls-tunnel", argv[1]))
    {
        nexthop_type = CLI_PERF_NH_TYPE_MPLS_TUNNEL;
    }
    else if (0 == sal_strcmp("mpls-nexthop", argv[1]))
    {
        nexthop_type = CLI_PERF_NH_TYPE_MPLS_NH;
    }
    else if (0 == sal_strcmp("xlate", argv[1]))
    {
        nexthop_type = CLI_PERF_NH_TYPE_XLATE;
    }
    else if (0 == sal_strcmp("ip-tunnel", argv[1]))
    {
        nexthop_type = CLI_PERF_NH_TYPE_IP_TUNNEL;
    }

    ret = _ctc_cli_perf_nexthop_id(nexthop_type, 10, operation, number);

    return ret;
}


CTC_CLI(ctc_cli_perf_ipuc_ipv4,
        ctc_cli_perf_ipuc_ipv4_cmd,
        "perf-test ipuc ipv4 (add|remove|update) A.B.C.D MASK_LEN (host-route |) (random |) (entry-num NUMBER)",
        "Performance test",
        "IPUC",
        CTC_CLI_IPV4_STR,
        "Add ipv4 route",
        "Remove ipv4 route",
        "Update ipv4 route",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        "Is a host route",
        "Random",
        "Entry number",
        "Number")
{
    int32  ret     = CLI_SUCCESS;
    uint8  index   = 0;
    uint32 number  = 0;
    uint8  random_cfg = 0;
    ctc_ipuc_param_t ipuc_info = {0};
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;

    /* 1.parser cli param */
    if (0 == sal_memcmp("add", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("remove", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }
    else if (0 == sal_memcmp("update", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_UPDATE;
        ipuc_info.route_flag = CTC_IPUC_FLAG_SELF_ADDRESS;
    }

    CTC_CLI_GET_IPV4_ADDRESS("ipv4", ipuc_info.ip.ipv4, argv[1]);
    CTC_CLI_GET_UINT8_RANGE("mask", ipuc_info.masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("host-route");
    if (index != 0xFF)
    {
        ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (index != 0xFF)
    {
        random_cfg = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("entry-num", number, argv[index + 1]);
    }

    /* 2.test operation */
    ret = _ctc_cli_perf_ipuc_ipv4_route(number, operation, &ipuc_info, random_cfg);

    return ret;
}


CTC_CLI(ctc_cli_perf_ipuc_ipv6,
        ctc_cli_perf_ipuc_ipv6_cmd,
        "perf-test ipuc ipv6 (add|remove|update) X:X::X:X MASK_LEN (host-route |) (random |) (entry-num NUMBER)",
        "Performance test",
        "IPUC",
        CTC_CLI_IPV6_STR,
        "Add ipv6 route",
        "Remove ipv6 route",
        "Updata ipv6 route",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        "Is a host route",
        "Random",
        "Entry number",
        "Number")
{
    int32  ret     = CLI_SUCCESS;
    uint8  index   = 0;
    uint32 number  = 0;
    uint8  random_cfg = 0;
    ipv6_addr_t ipv6_address = {0};
    ctc_ipuc_param_t ipuc_info = {0};
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;

    ipuc_info.ip_ver = CTC_IP_VER_6;
    /* 1.parser cli param */
    if (0 == sal_memcmp("add", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("remove", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }
    else if (0 == sal_memcmp("update", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_UPDATE;
        ipuc_info.route_flag = CTC_IPUC_FLAG_SELF_ADDRESS;
    }

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[1]);
    /* adjust endian */
    ipuc_info.ip.ipv6[0] = sal_htonl(ipv6_address[0]);
    ipuc_info.ip.ipv6[1] = sal_htonl(ipv6_address[1]);
    ipuc_info.ip.ipv6[2] = sal_htonl(ipv6_address[2]);
    ipuc_info.ip.ipv6[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", ipuc_info.masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("host-route");
    if (index != 0xFF)
    {
        ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (index != 0xFF)
    {
        random_cfg = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("entry-num", number, argv[index + 1]);
    }

    /* 2.test operation */
    ret = _ctc_cli_perf_ipuc_ipv6_route(number, operation, &ipuc_info, random_cfg);

    return ret;
}

CTC_CLI(ctc_cli_perf_ipmc,
        ctc_cli_perf_ipmc_cmd,
        "perf-test ipmc (add|remove) group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN (group-num NUMBER) (mem-num NUMBER)",
        "Performance test",
        "IPMC",
        "Add",
        "Remove",
        "Group address class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        "Group number",
        "Number",
        "Member number",
        "Number")
{
    int32  ret     = CLI_SUCCESS;
    uint8  index   = 0;
    uint8  grp_masklen = 0;
    uint8  src_masklen = 0;
    uint32 group_num  = 0;
    uint32 mem_num    = 0;
    ip_addr_t grp_ipv4 = 0;
    ip_addr_t src_ipv4 = 0;
    ctc_ipmc_group_info_t* p_ipmc_group_info = NULL;
    cli_perf_op_type_t operation = CLI_PERF_OP_TYPE_MAX;

    /* 1.parser cli param */
    if (0 == sal_memcmp("add", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("remove", argv[0], 1))
    {
        operation = CLI_PERF_OP_TYPE_REMOVE;
    }

    /* group info */
    CTC_CLI_GET_IPV4_ADDRESS("ip", grp_ipv4, argv[1]);
    CTC_CLI_GET_UINT8_RANGE("mask", grp_masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);

    /* member info */
    CTC_CLI_GET_IPV4_ADDRESS("ip", src_ipv4, argv[3]);
    CTC_CLI_GET_UINT8_RANGE("mask", src_masklen, argv[4], 0, CTC_MAX_UINT8_VALUE);

    /* number info */
    index = CTC_CLI_GET_ARGC_INDEX("group-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("group-num", group_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mem-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("mem-num", mem_num, argv[index + 1]);
    }

    p_ipmc_group_info = mem_malloc(MEM_CLI_MODULE, sizeof(ctc_ipmc_group_info_t));
    if (NULL == p_ipmc_group_info)
    {
        return CLI_ERROR;
    }
    sal_memset(p_ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));
    p_ipmc_group_info->ip_version = CTC_IP_VER_4;
    p_ipmc_group_info->address.ipv4.group_addr = grp_ipv4;
    p_ipmc_group_info->group_ip_mask_len = grp_masklen;
    p_ipmc_group_info->address.ipv4.src_addr   = src_ipv4;
    p_ipmc_group_info->src_ip_mask_len  = src_masklen;

    /* 2.test operation */
    ret = _ctc_cli_perf_ipmc(group_num, mem_num, operation, p_ipmc_group_info);
    mem_free(p_ipmc_group_info);

    return ret;
}

int32
ctc_perf_test_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_perf_vlan_create_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_vlan_create_vlan_add_default_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_vlan_add_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_vlan_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_vlan_mapping_add_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_vlan_mapping_remove_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_l2_fdb_add_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_l2_fdb_remove_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_l2_mcast_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_acl_add_install_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_acl_add_all_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_mpls_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_aps_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_ipuc_ipv4_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_ipuc_ipv6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_ipmc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_perf_l2_fdb_flush_cmd);

    return CLI_SUCCESS;
}


