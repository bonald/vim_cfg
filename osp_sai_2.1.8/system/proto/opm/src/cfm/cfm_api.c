
#include "proto.h"
#include "glb_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"
#include "gen/tbl_cfm_maid_define.h"
#include "gen/tbl_cfm_maid.h"
#include "cfmd.h"
#include "g8032.h"

int32
cfm_md_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, uint16 *p_level, uint8 *p_name_type, tbl_cfm_md_t  *p_md)
{
    int32                   rc = PM_E_NONE;
    uint16                 level = 0;
    uint8                   name_type = GLB_CFM_MD_CHAR_STRING;
    tbl_cfm_global_t  *p_db_glb = NULL;
    uint8                   para_num = 0;
    uint32                 part_first = 0;
    uint32                 part_second = 0;
    uint32                 part_third = 0;
    uint32                 part_fourth = 0;
    
    /* check exist */
    p_db_glb = tbl_cfm_global_get_cfm_global();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_GLOBAL not found");
    }

    /* check paramter number */
    if (argc < 2)
    {
        CFG_INVALID_PARAM_RET("MD Parameters must include MD name and level");
    }

    /* get md level */
    PM_CFG_GET_UINT(level, argv[1], para->p_rs);
    *p_level = level;

    if (level >= GLB_CFM_INVALID_LEVEL)
    {
        CFG_INVALID_PARAM_RET("Please define MD level when create md");    
    }

    /* get name type */
    if (argc >= 4)
    {
        if (0 == sal_strcmp(argv[2], "name_type"))
        {
            PM_CFG_GET_UINT(name_type, argv[3], para->p_rs);
            *p_name_type = name_type;
        }
    }
    
    /* name type not no-md-name */
    if (GLB_CFM_NO_MD_NAME != name_type)
    {
        /* check for y1731 mode */
        if (GLB_CFM_MODE_Y1731 == p_db_glb->cfm_mode)
        {
            CFG_INVALID_PARAM_RET("For y1731 mode, md name format should be no-md-name");  
        }

        if (5 == argc)
        {
            /* mac-address */
            if (GLB_CFM_MAC_ADDR_TWO_OCTET_INTEGER == name_type) 
            {
                para_num = sal_sscanf(argv[4], "%x-%x-%x:%d", &part_first, &part_second, &part_third, &part_fourth);
                if (4 != para_num)
                {
                    CFG_INVALID_PARAM_RET("For format mac-address, the input string is wrong");
                }
                
                if ((part_first > 0xffff) || (part_second > 0xffff) || (part_third > 0xffff) || (part_fourth > 0xffff))
                {
                    CFG_INVALID_PARAM_RET("For format mac-address, the input string is wrong");
                }

                p_md->pkt_name[0] = part_first >> 8;
                p_md->pkt_name[1] = part_first & 0xff;
                p_md->pkt_name[2] = part_second >> 8;
                p_md->pkt_name[3] = part_second & 0xff;
                p_md->pkt_name[4] = part_third >> 8;
                p_md->pkt_name[5] = part_third & 0xff;
                p_md->pkt_name[6] = part_fourth >> 8;
                p_md->pkt_name[7] = part_fourth & 0xff;
                p_md->pkt_name_length = 8;
            }
            else
            {
                p_md->pkt_name_length = sal_strlen(argv[4]);
                if (p_md->pkt_name_length > (GLB_CFM_MD_MA_NAME_LENGTH_8021AG - 1))
                {
                    CFG_INVALID_PARAM_RET("MD format name length should not exceed 43 characters");
                }
                
                sal_memcpy(p_md->pkt_name, argv[4], sal_strlen(argv[4]));
            }

            sal_memcpy(p_md->pkt_name_input, argv[4], sal_strlen(argv[4]));
        }
    }
    
    return rc;
}

int32
cfm_ma_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_cfm_md_t  *p_md, uint16 *p_vlan_id, uint8 *p_name_type, uint8 *p_index)
{
    int32                   rc = PM_E_NONE;
    uint16                 vlan_id = 0;
    uint8                   name_type = 0;
    tbl_cfm_global_t  *p_db_glb = NULL;
    uint8                   index = 2;

    /* check exist */
    p_db_glb = tbl_cfm_global_get_cfm_global();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_GLOBAL not found");
    }

    /* check paramter number */
    if (argc < 2)
    {
        CFG_INVALID_PARAM_RET("MA Parameters must include MA name and MD name");
    }

    /* check ma length */
    if ((sal_strlen(argv[1]) < 1)
        || (sal_strlen(argv[1]) > (GLB_CFM_MD_MA_NAME_LENGTH_8021AG - 1)))
    {
        CFG_INVALID_PARAM_RET("MA String length should not exceed 43 characters");
    }

    /* get vlan */
    if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "vlan"))
        {
            PM_CFG_GET_UINT(vlan_id, argv[index+1], para->p_rs);
            *p_vlan_id = vlan_id;
            index = index + 2;
        }
    }

    /* get name type */
    if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "name_type"))
        {
            PM_CFG_GET_UINT(name_type, argv[index+1], para->p_rs);
            *p_name_type = name_type;
            index = index + 2;
        }

        /* get format string */
        if (argc > index)
        {
             *p_index = index;
        }
    }

    if (GLB_CFM_MODE_Y1731 == p_db_glb->cfm_mode)
    {
        /* for icc based, check ma length isn't greater than 13 bytes */
        if (GLB_CFM_MA_ICC_BASED_FORMAT == name_type)
        {
            if (sal_strlen(argv[index]) > GLB_CFM_MD_MA_NAME_LENGTH_Y1731)
            {
                CFG_INVALID_PARAM_RET("Icc-based string length shouldn't be greater than 13 characters");      
            }    
        }
    }
    else
    {
        if (GLB_CFM_MA_ICC_BASED_FORMAT == name_type)
        {
            CFG_INVALID_PARAM_RET("For 802.1ag mode, icc-based can't be used"); 
        }  
    }
    
    if (0 == name_type)
    {
        /* md pkt name length and ma name length should not greater than 44 bytes */
        if (p_md->pkt_name_length)
        {
            if ((p_md->pkt_name_length + sal_strlen(argv[1])) > GLB_CFM_MD_MA_NAME_LENGTH_8021AG)
            {
                CFG_INVALID_PARAM_RET("MD format name + MA name length should not exceed %d characters\n", 
                    GLB_CFM_MD_MA_NAME_LENGTH_8021AG); 
            }
        }
        else if (GLB_CFM_NO_MD_NAME != p_md->name_type)
        {
            /* md name length and ma name length should not greater than 44 bytes */
            if ((sal_strlen(argv[0]) + sal_strlen(argv[1])) > GLB_CFM_MD_MA_NAME_LENGTH_8021AG)
            {
                CFG_INVALID_PARAM_RET("MD name + MA name length should not exceed %d characters\n", 
                    GLB_CFM_MD_MA_NAME_LENGTH_8021AG);
            }
        }
    }
    else
    {
        /* format name length should not be greater than 43 bytes */
        if (sal_strlen(argv[index]) >  (GLB_CFM_MD_MA_NAME_LENGTH_8021AG - 1))
        {
            CFG_INVALID_PARAM_RET("String length shouldn't be greater than 43 characters");      
        }
    
        if (p_md->pkt_name_length)
        {
            if ((p_md->pkt_name_length + sal_strlen(argv[index])) > GLB_CFM_MD_MA_NAME_LENGTH_8021AG)
            {
                CFG_INVALID_PARAM_RET("MD format name + MA format name length should not exceed %d characters\n", 
                    GLB_CFM_MD_MA_NAME_LENGTH_8021AG); 
            }
        }
        else if (GLB_CFM_NO_MD_NAME != p_md->name_type)
        {
            if ((sal_strlen(argv[0])  + sal_strlen(argv[index])) > GLB_CFM_MD_MA_NAME_LENGTH_8021AG)
            {
                CFG_INVALID_PARAM_RET("MD name + MA format name length should not exceed %d characters\n", 
                    GLB_CFM_MD_MA_NAME_LENGTH_8021AG); 
            }
        }
    }
    
    return rc;
}

int32
cfm_lmep_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, uint16 *p_vlan_id, uint8 *p_interval, uint8  *p_direction, uint16  *p_mep_id)
{
    int32                   rc = PM_E_NONE;
    uint8                   index = 4;
    uint16                 vlan_id = 0;
    uint8                   cci_interval = 0;
    uint16                 mep_id = 0;

    /* check md length */
    if ((sal_strlen(argv[3]) < 1)
        || (sal_strlen(argv[3]) > (GLB_CFM_MD_MA_NAME_LENGTH_8021AG - 1)))
    {
        CFG_INVALID_PARAM_RET("MD String length should not exceed 43 characters");
    }

   /* get vlan */
    if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "vlan"))
        {
            PM_CFG_GET_UINT(vlan_id, argv[index+1], para->p_rs);
            *p_vlan_id = vlan_id;
            index = index + 2;
        }
    }

    /* get interval */
    if (argc >= index)
    {
        if (0 == sal_strcmp(argv[index], "interval"))
        {
            PM_CFG_GET_UINT(cci_interval, argv[index+1], para->p_rs);
            *p_interval = cci_interval;
            index = index + 2;
        }
    }

    /* get direction */
    if (argc >= index)
    {
        if (0 == sal_strcmp(argv[index], "down"))
        {
            *p_direction = GLB_CFM_DOWN_MEP;
        }
        else
        {
            *p_direction = GLB_CFM_UP_MEP;
        }
        index++;
    }

    /* get mep id */
    if (argc >= index)
    {
        PM_CFG_GET_UINT(mep_id, argv[index], para->p_rs);
        *p_mep_id = mep_id;
    }
 
    return rc;
}

int32
cfm_rmep_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, uint16 *p_vlan_id, uint16  *p_rmep_id, uint8 *mac)
{
    int32                   rc = PM_E_NONE;
    uint8                   index = 4;
    uint16                 vlan_id = 0;
    uint16                 rmep_id = 0;

    /* check md length */
    if ((sal_strlen(argv[3]) < 1)
        || (sal_strlen(argv[3]) > (GLB_CFM_MD_MA_NAME_LENGTH_8021AG - 1)))
    {
        CFG_INVALID_PARAM_RET("MD String length should not exceed 43 characters");
    }

    /* get vlan */
    if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "vlan"))
        {
            PM_CFG_GET_UINT(vlan_id, argv[index+1], para->p_rs);
            *p_vlan_id = vlan_id;
            index = index + 2;
        }
    }

    /* get mac */
    if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "mac"))
        {
            rc = cdb_fdb_mac_address_is_valid(argv[index+1]);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[index+1]);
                return rc;
            }
    
            if (3 != sal_sscanf(argv[index+1], "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
            {
                CFG_INVALID_PARAM_RET("Invalid MAC address %s", argv[index+1]);
                return PM_E_CFG_INVALID_PARAM;
            }
            *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
            *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
            *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);

            index = index + 2;
        }
    }

    /* get mep id */
    if (argc >= index)
    {
        PM_CFG_GET_UINT(rmep_id, argv[index], para->p_rs);
        *p_rmep_id = rmep_id;
    }
 
    return rc;
}

char * cfm_get_cc_interval_desc(uint8 interval)
{
    switch(interval)
    {
    case  1:
        return "3.3ms";
        break;
    case  2:
        return "10ms";
        break;
    case  3:
        return "100ms";
        break;
    case  4 :
        return "1s";
        break;  
    case  5 :
        return "10s";
        break;
    case  6 :
        return "60s";
        break;
    case  7 :
        return "600s";
        break;
    default:
        return "error";   
    }
  
    return "error"; 
}

char *cfm_format_mac_address (uint8 *mac_addr)
{
  static char mac_desc[15 + 1] =  { 0 };
  if (mac_addr)
    {     
      sal_sprintf(mac_desc, "%02x%02x.%02x%02x.%02x%02x",
                  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3],   
                  mac_addr[4], mac_addr[5]);
     
      return mac_desc;
    }
  else
    {
      return " ";
    }
}

int32
cfm_set_service_cci_enable(tbl_cfm_ma_t   *p_ma, uint8   cci_enabled)
{
    int32                              rc = PM_E_NONE;
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;

    if (p_ma->cci_enabled == cci_enabled)
    {
        return rc;    
    }

     /* check exist */
    p_db_glb = tbl_cfm_lmep_get_master();
    if (!p_db_glb)
    {
        return PM_E_FAIL;
    }

    /* update local CDB */
    p_ma->cci_enabled = cci_enabled;
    rc = tbl_cfm_ma_set_cfm_ma_field(p_ma, TBL_CFM_MA_FLD_CCI_ENABLED);

    /* set local mep cci enabled */
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_lmep_list, p_lmep, listnode)
    {
        if ((p_ma->key.vlan_id == p_lmep->key.vlan_id)
            && (0 == sal_strcmp((char*)p_ma->key.name, p_lmep->md_name)))
        {
            p_lmep->cci_enabled = cci_enabled;
            tbl_cfm_lmep_set_cfm_lmep_field(p_lmep, TBL_CFM_LMEP_FLD_CCI_ENABLED);
        }
    }

    return rc;
}

int32
cfm_set_service_ccm_vlan_cos(tbl_cfm_ma_t   *p_ma, uint8   ccm_vlan_cos)
{
    int32                              rc = PM_E_NONE;
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;

    if (p_ma->ccm_vlan_cos == ccm_vlan_cos)
    {
        return rc;    
    }

     /* check exist */
    p_db_glb = tbl_cfm_lmep_get_master();
    if (!p_db_glb)
    {
        return PM_E_FAIL;
    }

    /* update local CDB */
    p_ma->ccm_vlan_cos = ccm_vlan_cos;
    rc = tbl_cfm_ma_set_cfm_ma_field(p_ma, TBL_CFM_MA_FLD_CCM_VLAN_COS);

    /* set local mep cci enabled */
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_lmep_list, p_lmep, listnode)
    {
        if ((p_ma->key.vlan_id == p_lmep->key.vlan_id)
            && (0 == sal_strcmp((char*)p_ma->key.name, p_lmep->md_name)))
        {
            p_lmep->ccm_vlan_cos = ccm_vlan_cos;
            tbl_cfm_lmep_set_cfm_lmep_field(p_lmep, TBL_CFM_LMEP_FLD_CCM_VLAN_COS);
        }
    }

    return rc;
}

int32
cfm_api_delete_meps_for_ma(tbl_cfm_ma_t   *p_ma)
{
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    ctclib_slistnode_t             *listnode_next = NULL;
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;
    tbl_interface_t                 *p_db_if = NULL;

    p_db_glb_rmep = tbl_cfm_rmep_get_master();
    if (p_db_glb_rmep)
    {
        CTCLIB_SLIST_LOOP_DEL(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode, listnode_next)
        {
            if ((0 == sal_strcmp(p_rmep->md_name, (char*)p_ma->key.name))
                && (p_rmep->key.vlan_id == p_ma->key.vlan_id))
            {
                p_db_if = tbl_interface_get_interface_by_ifindex(p_rmep->ifindex);
                if (p_db_if)
                {
                    p_db_if->cfm_rmep_num--;
                }
                else
                {
                    CFM_LOG_ERR("%s %d ifindex %d is invalid\n", __func__, __LINE__, p_rmep->ifindex);       
                }
                
                tbl_cfm_rmep_del_cfm_rmep(&p_rmep->key);
            }
        }
    }
    
    p_db_glb = tbl_cfm_lmep_get_master();
    if (p_db_glb)
    {
        CTCLIB_SLIST_LOOP_DEL(p_db_glb->cfm_lmep_list, p_lmep, listnode, listnode_next)
        {
            if ((0 == sal_strcmp(p_lmep->md_name, (char*)p_ma->key.name))
                && (p_lmep->key.vlan_id == p_ma->key.vlan_id))
            {
                /* delete g8032 cfm association */
                if (p_lmep->ring_id)
                {
                    g8032_delete_association_by_cfm(p_lmep->ring_id);
                }
                
                p_db_if = tbl_interface_get_interface_by_ifindex(p_lmep->key.ifindex);
                if (p_db_if)
                {
                    p_db_if->cfm_lmep_num--;
                }
                else
                {
                    CFM_LOG_ERR("%s %d ifindex %d is invalid\n", __func__, __LINE__, p_rmep->ifindex);       
                }
                
                tbl_cfm_lmep_del_cfm_lmep(&p_lmep->key);
            }
        }
    }
    
    return PM_E_NONE;
}

int32
cfm_api_delete_ma_for_md(tbl_cfm_md_t   *p_md)
{
    tbl_cfm_ma_master_t  *g_p_master_ma = tbl_cfm_ma_get_master();
    tbl_cfm_ma_t              *p_ma = NULL;
    ctclib_slistnode_t         *listnode_ma = NULL;
    ctclib_slistnode_t         *listnode_ma_next = NULL;
    tbl_cfm_maid_t            maid;
    
    CTCLIB_SLIST_LOOP_DEL(g_p_master_ma->cfm_ma_list, p_ma, listnode_ma, listnode_ma_next)
    {
        if (0 == sal_strcmp((char*)p_ma->key.name, p_md->key.name))
        {
            /* delete all meps for ma */
            cfm_api_delete_meps_for_ma(p_ma);
        
            /* count ma number for md */
            p_md->service_number--;
        
            /* del maid */
            sal_memset(&maid, 0, sizeof(tbl_cfm_maid_t));
            sal_memcpy(&maid.key.name, p_ma->maid, CFM_MAID_MAX_LENGTH);
            tbl_cfm_maid_del_cfm_maid(&maid.key);
        
            /* del ma */
            tbl_cfm_ma_del_cfm_ma(&p_ma->key);    
        }
    }
    
    return PM_E_NONE;
}

int32 
cfm_api_clear_cfm_errors_for_md(tbl_cfm_md_t   *p_md)
{
    ds_cfm_error_t             *p_db_cfm_error = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    ctclib_slistnode_t          *listnode_next = NULL;

    if (!p_md->cfm_error.obj_list)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_md->cfm_error.obj_list, p_db_cfm_error, listnode, listnode_next)
    {
        ds_cfm_error_del_cfm_error(p_md, p_db_cfm_error);
    }

    return PM_E_NONE;
}

int32
cfm_api_delete_ma_for_vlan(vid_t   vid)
{
    tbl_cfm_ma_master_t  *g_p_master_ma = tbl_cfm_ma_get_master();
    tbl_cfm_ma_t              *p_ma = NULL;
    ctclib_slistnode_t         *listnode_ma = NULL;
    ctclib_slistnode_t         *listnode_ma_next = NULL;
    tbl_cfm_maid_t            maid;
    tbl_cfm_md_key_t        md_key;
    tbl_cfm_md_t               *p_md;
    
    CTCLIB_SLIST_LOOP_DEL(g_p_master_ma->cfm_ma_list, p_ma, listnode_ma, listnode_ma_next)
    {
        if (p_ma->key.vlan_id == vid)
        {
            /* delete all meps for ma */
            cfm_api_delete_meps_for_ma(p_ma);

            sal_memcpy(md_key.name, p_ma->key.name, GLB_CFM_MD_MA_NAME_LENGTH_8021AG);
            p_md = tbl_cfm_md_get_cfm_md(&md_key);
            if (p_md)
            {
                /* count ma number for md */
                p_md->service_number--;
            }
        
            /* del maid */
            sal_memset(&maid, 0, sizeof(tbl_cfm_maid_t));
            sal_memcpy(&maid.key.name, p_ma->maid, CFM_MAID_MAX_LENGTH);
            tbl_cfm_maid_del_cfm_maid(&maid.key);
        
            /* del ma */
            tbl_cfm_ma_del_cfm_ma(&p_ma->key);    
        }
    }
    
    return PM_E_NONE;
}

int32
cfm_api_get_cfm_state_for_g8032(tbl_cfm_lmep_t        *p_lmep)
{
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;

    p_db_glb_rmep = tbl_cfm_rmep_get_master();
    if (!p_db_glb_rmep)
    {
        return GLB_CFM_RMEP_INITIAL;
    }

    CTCLIB_SLIST_LOOP(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode)
    {
        if ((p_rmep->key.level == p_lmep->key.level) && (p_rmep->key.vlan_id == p_lmep->key.vlan_id)
            && (p_rmep->ifindex == p_lmep->key.ifindex))
        {
            if (!p_rmep->first_pkt_rx)
            {
                return GLB_CFM_RMEP_INITIAL;    
            }
            else
            {
                if (p_rmep->d_loc)
                {
                    return GLB_CFM_RMEP_DOWN;    
                }
                else
                {
                    return GLB_CFM_RMEP_UP;    
                }
            }
        }
    }

    return GLB_CFM_RMEP_INITIAL;
}
