/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_brgif.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "glb_tempfile_define.h"
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "ctc_sai_port.h"

extern hsrv_fdb_t g_hsrv_fdb;

/***************************************************************************************************
 * Name         : hsrv_fdb_hw_get_count_number
 * Purpose      : count fdb number in ASIC
 * Input        : N/A
 * Output       : static_count, dynamic_count, total_count                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_hw_get_count_number(uint32* static_count, uint32* dynamic_count, uint32* total_count)
{
    int32 ret = 0;
    uint32 index = 0;
    ctc_l2_fdb_query_rst_t query_rst;
    ctc_l2_fdb_query_t query;
    uint32 num_static = 0;
    uint32 num_dynamic = 0;
    uint32 num_total = 0;
    
    sal_memset(&query_rst, 0, sizeof(query_rst));
    sal_memset(&query, 0, sizeof(query));

    query.fid = 0;
    query.gport = 0;
    query.query_flag = GLB_L2_FDB_ENTRY_ALL;
    query.query_type = GLB_L2_FDB_ENTRY_OP_GET_COUNT;
    query.query_hw = TRUE;
    
    query_rst.buffer_len = g_hsrv_fdb.hw_dump_buffer_len;
    query_rst.buffer = g_hsrv_fdb.hw_dump_buffer;
    if (NULL == query_rst.buffer)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Hardware dump buffer not exist\n");
        return HSRV_E_INVALID_PARAM;
    }
    
    query.query_type = GLB_L2_FDB_ENTRY_OP_ALL;
    do
    {
        query_rst.start_index = query_rst.next_query_index;
        ret = ctc_l2_get_fdb_entry(&query, &query_rst);
        if (ret < 0)
        {
            return HSRV_E_INVALID_PARAM;
        }
        for (index = 0; index < query.count; index++)
        {
            /*skip mcast fdb entry*/
            if(query_rst.buffer[index].mac[0] & 0x1)
            {
                continue;
            } 
            if(query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC)
            {
                num_static++;
            }
            else
            {
                num_dynamic++;
            }
            sal_memset(&query_rst.buffer[index], 0, sizeof(ctc_l2_addr_t));
            num_total++;
        }
    } while (query_rst.is_end == 0);

    *static_count = num_static;
    *dynamic_count = num_dynamic;
    *total_count = num_total;

    return HSRV_E_NONE;
}

int32
hsrv_fdb_hw_dump_fdb(vid_t vid, sai_object_id_t port_oid, uint8* mac_addr, glb_l2_fdb_query_type_t query_type,
                             glb_l2_fdb_query_flag_t query_flag, cfg_cmd_para_t *para)
{
    int32 ret = 0;
    uint32 index = 0;
    uint32 total_count = 0;
    uint32 query_type_old = MAX_GLB_L2_FDB_ENTRY_OP;
    FILE* fp = NULL;
    ctc_l2_fdb_query_rst_t query_rst;
    sai_object_id_t    gport_obj = 0;
    char char_buffer[128];
    ctc_l2_fdb_query_t query;
    char ifname[IFNAME_SIZE];
    char ifname_ext[GLB_IFNAME_SZ];  
    uint32 tid = 0;
    int i = 0;
    uint32  pgport = 0;
    tbl_fea_port_if_t* f_port_if = NULL;
    tbl_fea_port_if_key_t  fea_port_if_key;

    sal_memset(&query_rst, 0, sizeof(query_rst));
    sal_memset(&query, 0, sizeof(query));
    sal_memset(&fea_port_if_key, 0, sizeof(fea_port_if_key));
    
    query.fid = vid;
    ctc_sai_port_objectid_to_gport( port_oid, &pgport);
    query.gport = pgport;
    sal_memcpy(query.mac, mac_addr, sizeof(mac_addr_t));
    query.query_flag = query_flag;
    query.query_type = query_type;
    query.query_hw = TRUE;
    
    /*GG unsupport MAC*/
    if (GLB_L2_FDB_ENTRY_OP_BY_MAC == query.query_type)
    {
        query_type_old = query.query_type;
        query.query_type = GLB_L2_FDB_ENTRY_OP_ALL;
    }
    query_rst.buffer_len = g_hsrv_fdb.hw_dump_buffer_len;
    query_rst.buffer = g_hsrv_fdb.hw_dump_buffer;
    if (NULL == query_rst.buffer)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Hardware dump buffer not exist\n");
        return HSRV_E_INVALID_PARAM;
    }
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if ((NULL == fp) || sal_feof(fp))
    {
        return HSRV_E_INVALID_PARAM;
    }
    if(GLB_L2_FDB_ENTRY_OP_GET_COUNT == query.query_type)
    {
        uint32 total_count = 0, dynamic_count= 0, static_count = 0;
        query.query_type = GLB_L2_FDB_ENTRY_OP_ALL;
        do
        {
            query_rst.start_index = query_rst.next_query_index;
            ret = ctc_l2_get_fdb_entry(&query, &query_rst);
            if (ret < 0)
            {
                sal_fclose(fp);
                return HSRV_E_INVALID_PARAM;
            }
            for (index = 0; index < query.count; index++)
            {
                /*skip mcast fdb entry*/
                if(query_rst.buffer[index].mac[0] & 0x1)
                {
                    continue;
                } 
                if (CTC_IS_LINKAGG_PORT(query_rst.buffer[index].gport))
                {    
                     tid = CTC_GPORT_LINKAGG_ID( query_rst.buffer[index].gport );
                     gport_obj = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
                     fea_port_if_key.portid = gport_obj;
                     f_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key);  
                     if (NULL != f_port_if)
                     {
                          for(i = 0; i < IFNAME_SIZE ; i++)
                          {
                              ifname[i] = f_port_if->name[i];
                          }
                     }       
                }
                else
                {
                     gport_obj = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, query_rst.buffer[index].gport);
                     fea_port_if_key.portid = gport_obj;
                     f_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key);  
                     if (NULL != f_port_if)
                     {
                          for(i = 0; i < IFNAME_SIZE ; i++)
                          {
                              ifname[i] = f_port_if->name[i];
                          }
                     }       
                }
                if(query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC)
                {
                    static_count++;
                }
                else
                {
                    dynamic_count++;
                }
                sal_memset(&query_rst.buffer[index], 0, sizeof(ctc_l2_addr_t));
                total_count++;
            }
        }while (query_rst.is_end == 0);

        sal_fprintf(fp, "MAC address count in hardware table:\n");
        sal_fprintf(fp, "(Include MAC security entry)\n");
        sal_fprintf(fp, "-------------------------------------------\n");
        sal_fprintf(fp, "Dynamic Address Count  :%5d\n", dynamic_count);
        sal_fprintf(fp, "Static  Address Count  :%5d\n", static_count);
        sal_fprintf(fp, "Total Mac Addresses    :%5d\n", total_count);
    }
    else
    {
        sal_fprintf(fp, "            Mac Address HW Table\n");
        sal_fprintf(fp, "----------------------------------------------\n");
        sal_fprintf(fp, "%-7s %-16s %-12s %-8s\n", "VLAN ID", "MAC Address", "Type", "Port");
        sal_fprintf(fp, "-------+----------------+------------+--------\n");
            
        do
        {
            query_rst.start_index = query_rst.next_query_index;
            ret = ctc_l2_get_fdb_entry(&query, &query_rst);
            if (ret < 0)
            {
                sal_fclose(fp);
                return HSRV_E_INVALID_PARAM;
            } 
            for (index = 0; index < query.count; index++)
            {
                /*skip mcast fdb entry*/
                if(query_rst.buffer[index].mac[0] & 0x1)
                {
                    continue;
                }
                if (CTC_IS_LINKAGG_PORT(query_rst.buffer[index].gport))
                {
                    tid = CTC_GPORT_LINKAGG_ID( query_rst.buffer[index].gport );
                    gport_obj = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
                    fea_port_if_key.portid = gport_obj;
                    f_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key);  
                    if (NULL != f_port_if)
                    {
                        for(i = 0; i < IFNAME_SIZE ; i++)
                        {
                            ifname[i] = f_port_if->name[i];
                        }
                    }       
                }
                else
                {
                    gport_obj = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, query_rst.buffer[index].gport);
                    fea_port_if_key.portid = gport_obj;
                    f_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key);  
                    if (NULL != f_port_if)
                    {
                        for(i = 0; i < IFNAME_SIZE ; i++)
                        {
                            ifname[i] = f_port_if->name[i];
                        }

                    }       
                }
                /*GG unsupport MAC*/
                if (query_type_old == GLB_L2_FDB_ENTRY_OP_BY_MAC)
                {
                    if (sal_memcmp(query_rst.buffer[index].mac, query.mac, sizeof(mac_addr_t)))
                    {
                        continue;
                    }
                }
                sal_sprintf(char_buffer, "%-4d    %.04hx.%.04hx.%.04hx   %-11s  %s\n",
                    query_rst.buffer[index].fid, 
                    sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[0]),
                    sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[2]),
                    sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[4]),
                    (query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC) ?  
                    "static":"dynamic",  IFNAME_ETH2FULL(ifname, ifname_ext));
                sal_fprintf(fp, "%s", char_buffer);
                sal_memset(&query_rst.buffer[index], 0, sizeof(ctc_l2_addr_t));
                total_count ++;
            }

        }while (query_rst.is_end == 0);
        sal_fprintf(fp, "Total Entry Num: %d\n", total_count);
    }
    sal_fclose(fp);
    fp = NULL;

    return HSRV_E_NONE;
}

int32
hsrv_fdb_hw_get_common(glb_l2_fdb_query_type_t query_type, hsrv_mac_addr_type_t type, 
                             uint32 ifindex, vid_t vid, uint8 *mac_addr, cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_glb = tbl_brg_global_get_brg_global();
    FILE *fp = NULL;
    glb_l2_fdb_query_flag_t   query_flag = 0;
    sai_object_id_t   port_oid = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_SIZE];
    int rc = 0;
    
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    switch (type)
    {
    case HSRV_MAC_ADDR_TYPE_STATIC:
        query_flag = GLB_L2_FDB_ENTRY_STATIC;
        break;
    case HSRV_MAC_ADDR_TYPE_DYNAMIC:
        query_flag = GLB_L2_FDB_ENTRY_DYNAMIC;
        break;
    case HSRV_MAC_ADDR_TYPE_ALL:
        query_flag = GLB_L2_FDB_ENTRY_ALL;        
        break;
    default:
        CFG_CONFLICT_RET("Invalid query type");
        break;
    }
    
    /*ifindex to objectid*/  
    if (GLB_L2_FDB_ENTRY_OP_BY_PORT == query_type)
    { 
        tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
        rc = hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
        if (HSRV_E_ENTRY_NOT_EXIST == rc)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        }
    }    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+"); 
    if (p_glb->hw_learning_enable)
    {
        hsrv_fdb_hw_dump_fdb(vid, port_oid, mac_addr, query_type, query_flag, para);
    }
    else
    {
        sal_fprintf(fp, "Hardware learning isn't enabled\n");
    }
    
    sal_fclose(fp);
    fp = NULL;
    return HSRV_E_NONE;
}

int32
hsrv_fdb_hw_get_interface(int32 ifindex, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
    mac_addr_t mac_addr = {0};

    return hsrv_fdb_hw_get_common(GLB_L2_FDB_ENTRY_OP_BY_PORT, type, ifindex, 0, mac_addr, para);
}

int32
hsrv_fdb_hw_get_vlan(vid_t vid, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
    mac_addr_t mac_addr = {0};
    
    return hsrv_fdb_hw_get_common(GLB_L2_FDB_ENTRY_OP_BY_VID, type, 0, vid, mac_addr, para);
}

int32
hsrv_fdb_hw_get_mac(uint8 *mac, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
    return hsrv_fdb_hw_get_common(GLB_L2_FDB_ENTRY_OP_BY_MAC, type, 0, 0, mac, para);
}

int32
hsrv_fdb_hw_get_all(hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
    mac_addr_t mac_addr = {0};
    
    return hsrv_fdb_hw_get_common(GLB_L2_FDB_ENTRY_OP_ALL, type, 0, 0, mac_addr, para);
}

int32
hsrv_fdb_hw_get_count(cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_glb = tbl_brg_global_get_brg_global();
    FILE *fp = NULL;
    mac_addr_t mac_addr = {0};

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_glb->hw_learning_enable)
    {
        hsrv_fdb_hw_dump_fdb(0, 0, mac_addr, GLB_L2_FDB_ENTRY_OP_GET_COUNT, GLB_L2_FDB_ENTRY_ALL, para);
    }
    else
    {
        sal_fprintf(fp, "Hardware learning isn't enabled\n");
    }
    sal_fclose(fp);
    fp = NULL;
    return HSRV_E_NONE;
}

int32
hsrv_fdb_hw_cmd_process_fdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    hsrv_mac_addr_type_t type = HSRV_MAC_ADDR_TYPE_ALL;
    vid_t vid = 0;
    int32 ifindex = 0;
    mac_addr_t mac;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Need show type parameter");
        }
        else if (1 <= argc)
        {
            if (0 == sal_strcmp(argv[0], "count"))
            {
                rc = hsrv_fdb_hw_get_count(para);
                return rc;
            }   
            if (0 == sal_strcmp(argv[0], "static"))
            {
                type = HSRV_MAC_ADDR_TYPE_STATIC;
            }
            else if (0 == sal_strcmp(argv[0], "dynamic"))
            {
                type = HSRV_MAC_ADDR_TYPE_DYNAMIC;
            }
            else if (0 == sal_strcmp(argv[0], "all"))
            {
                type = HSRV_MAC_ADDR_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid show type %s", argv[0]);
            }
            
            if (1 == argc)
            {
                rc = hsrv_fdb_hw_get_all(type, para);
            }
            else if (3 == argc)
            {
                if (0 == sal_strcmp(argv[1], "interface"))
                {
                    ifindex = tbl_interface_get_ifindex_by_name(argv[2]);
                    if (ifindex < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[2]);
                    }
                    rc = hsrv_fdb_hw_get_interface(ifindex, type, para);    
                }
                else if (0 == sal_strcmp(argv[1], "vlan"))
                {
                    PM_CFG_GET_UINT(vid, argv[2], para->p_rs);
                    rc = hsrv_fdb_hw_get_vlan(vid, type, para);
                }
                else if (0 == sal_strcmp(argv[1], "address"))
                {
                    sal_memset(mac, 0, sizeof(mac));
                    rc = cdb_mac_str2val(mac, argv[2]);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[2]);
                    }
                    rc = hsrv_fdb_hw_get_mac(mac, type, para);
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid type %s", argv[1]);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
            }
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;   
}

int32
hsrv_fdb_hw_init()
{
    g_hsrv_fdb.hw_dump_buffer_len = HSRV_FDB_HW_DUMP_BUFFER_LEN * sizeof(ctc_l2_addr_t);
    g_hsrv_fdb.hw_dump_buffer = HSRV_MALLOC(MEM_FEA_HSRV_MODULE, g_hsrv_fdb.hw_dump_buffer_len); 
    cdb_register_cfg_act_cb(ACT_SHOW_HARDWARE_FDB, hsrv_fdb_hw_cmd_process_fdb);

    return HSRV_E_NONE;
}

