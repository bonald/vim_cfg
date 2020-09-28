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
#include "glb_tempfile_define.h"
#include "hsrv_fdb.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/
extern hsrv_fdb_t g_hsrv_fdb;


/***************************************************************************************************
 * Name         : _hsrv_port_security_interface_status_dump
 * Purpose      : get the special port security information
 * Input        : p_db_if, table interface; para, cfg_cmd_para_t;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
_hsrv_port_security_interface_status_dump( tbl_interface_t * p_db_if, FILE *fp)
{
    ds_brgif_t* p_brgif = NULL;
    tbl_fea_brg_if_t *p_db_bif = NULL;
    tbl_fea_brg_if_key_t fea_brg_if_key;
    
    uint32 dynamic_security_cnt = 0;
    uint32 static_security_cnt = 0;
    
    sal_memset(&fea_brg_if_key, 0, sizeof(fea_brg_if_key));
    fea_brg_if_key.ifindex = p_db_if->ifindex;
    p_db_bif = tbl_fea_brg_if_get_fea_brg_if(&fea_brg_if_key);
    if (NULL == p_db_if)
    {
        return HSRV_E_PORT_NOT_EXIST;
    }
    p_brgif = ds_brgif_get_brgif(p_db_if);
    if (NULL == p_brgif)
    {
        return HSRV_E_PORT_NOT_EXIST;
    } 
    
    dynamic_security_cnt = p_db_bif->curr_mac;
    static_security_cnt = p_db_if->static_security_num;

    sal_fprintf(fp,"Port security                    : %s\n", (p_brgif->port_security_en)?"enabled":"disabled");
    if (p_brgif->violate_mode == GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT)
         sal_fprintf(fp,"Violation mode                   : %5s\n", "discard packet silence" );
    else if (p_brgif->violate_mode == GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT)
         sal_fprintf(fp,"Violation mode                   : %5s\n", "discard packet and log" );
    else if (p_brgif->violate_mode == GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN)
         sal_fprintf(fp,"Violation mode                   : %5s\n", "discard packet, log and set interface error-disabled" );

    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        sal_fprintf(fp,"Maximum MAC addresses            : %d \n",  p_brgif->max_mac);
    }
    else
    {
        sal_fprintf(fp,"Maximum Dynamic MAC addresses    : %d \n",  p_brgif->max_mac);
    }
    sal_fprintf(fp,"Total MAC addresses              : %d \n",  dynamic_security_cnt + static_security_cnt);
    sal_fprintf(fp,"Static configured MAC addresses  : %d \n",  static_security_cnt);
    sal_fprintf(fp,"Dynamic learned MAC addresses    : %d \n",  dynamic_security_cnt);
    
    return HSRV_E_NONE;
}


static int32
_hsrv_port_security_show_all_status_banner(FILE *fp)
{
    sal_fprintf(fp,"Secure Port  MaxSecureAddr  CurrentAddr  SecurityViolationMode\n");
    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        sal_fprintf(fp,"               (Count)       (All)   \n");
    }
    else
    {
        sal_fprintf(fp,"               (Count)       (Dynamic)   \n");
    }
    sal_fprintf(fp,"------------+--------------+------------+-------------------------\n");
    return HSRV_E_NONE;
}

static int32
_hsrv_port_security_address_show_banner(FILE *fp)
{
    sal_fprintf(fp, "            Security Mac Address Table\n");
    sal_fprintf(fp, "--------------------------------------------------\n");
    sal_fprintf(fp, "%-7s %-16s %-16s %-8s\n", "VLAN ID", "MAC Address", "Type", "Port");
    sal_fprintf(fp, "-------+----------------+----------------+--------\n");
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : _hsrv_port_security_get_all_status
 * Purpose      : get the port_security information of one port
 * Input        : p_db_if, table interface; para, cfg_cmd_para_t;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
_hsrv_port_security_get_all_status(tbl_interface_t *p_db_if, FILE* fp)
{
      ds_brgif_t* p_brgif = NULL; 
      tbl_fea_brg_if_t *p_db_bif = NULL;
      tbl_fea_brg_if_key_t fea_brg_if_key;
          
      char type_str[CMD_BUF_128];
      char ifname_ext[GLB_IFNAME_SZ]; 
      uint32 MaxSecureAddr = 0;
      uint32 mode = 0;
      uint32 dynamic_security_cnt = 0;

      sal_memset(&fea_brg_if_key, 0, sizeof(fea_brg_if_key));
      fea_brg_if_key.ifindex = p_db_if->ifindex;
      p_db_bif = tbl_fea_brg_if_get_fea_brg_if(&fea_brg_if_key);
      if (NULL == p_db_if)
      {
          return HSRV_E_PORT_NOT_EXIST;
      }
      p_brgif = ds_brgif_get_brgif(p_db_if);
      if (NULL == p_brgif)
      {
          return HSRV_E_PORT_NOT_EXIST;
      }
      MaxSecureAddr = p_db_bif->max_mac;
      dynamic_security_cnt = p_db_bif->curr_mac;
      mode = p_brgif->violate_mode;

      type_str[0] = CMD_ZERO_CHAR;
      if(GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT == mode)
      {
           sal_strcat(type_str, "protect");
      }
      else if(GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == mode)
      {
           sal_strcat(type_str, "restrict");
            
      }
      else if (GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == mode)
      {
           sal_strcat(type_str, "shutdown");
      }
      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext); 
      
      sal_fprintf(fp, "%-12s %-14d %-12d %-15s\n",
      ifname_ext, MaxSecureAddr, dynamic_security_cnt, type_str);
      
      return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_port_security_get_interface_status
 * Purpose      : show the port_security information on one port
 * Input        : p_db_if, table interface; para, cfg_cmd_para_t;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_security_get_interface_status(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
   
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_db_if)
    {
        _hsrv_port_security_interface_status_dump(p_db_if, fp);
    }
    
    sal_fclose(fp);  
    fp = NULL;
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_all_status
 * Purpose      : show the port_security information on all ports that port_security enable
 * Input        : para, cfg_cmd_para_t;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_security_get_all_status(cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    ds_brgif_t* p_brgif = NULL; 
    tbl_interface_t * p_db_if = NULL;
    bool port_security_enable = false;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_MODE_L2 != p_db_if->mode)
        {
            continue;
        }
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        p_brgif = ds_brgif_get_brgif(p_db_if);
        if(p_brgif == NULL)
        {
            continue;
        }
        if (p_brgif->port_security_en)
        {
            if (port_security_enable == false)
            {
                _hsrv_port_security_show_all_status_banner(fp);
            }
            _hsrv_port_security_get_all_status(p_db_if, fp);
            port_security_enable = true;
        }
    }
    if (port_security_enable == false)
    {
        sal_fprintf(fp, "No port security in system");
        sal_fclose(fp);  
        fp = NULL;
        return HSRV_E_NONE;
    }
    
    sal_fclose(fp);  
    fp = NULL;
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_address_one
 * Purpose      : get one dynamic or static port-security fdb    
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
hsrv_port_security_get_address_one(tbl_fea_fdb_t *p_db_fdb, tbl_psfdb_t *p_db_psfdb, hsrv_mac_addr_type_t type, FILE *fp)
{
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char mac_str[CMD_BUF_32];
    char type_str[CMD_BUF_32];
    uint32 is_static = FALSE;
    uint32 is_security = FALSE;

    if (p_db_fdb)
    {
        is_static = GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC);
        is_security = GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
    }
    else if (p_db_psfdb)
    {
        is_static = GLB_FLAG_ISSET(p_db_psfdb->flags, GLB_FDB_FLAG_STATIC);
        is_security = GLB_FLAG_ISSET(p_db_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
    }

    if ((!is_security) &&(type == HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY ||
        type == HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY || type == HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY))
    {
        return HSRV_E_NONE;
    }

    switch (type)
    {
    case HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY:
        if (!is_static || !is_security)
        {
            return PM_E_NONE;
        }
        break;
    case HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY:
        if (is_static || !is_security)
        {
            return PM_E_NONE;
        }
        break;
    case HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY:
        if (!is_security)
        {
            return PM_E_NONE;
        }
        break;

    default:
        break;
    }
    
    type_str[0] = CMD_ZERO_CHAR;
    if (is_static)
    {
        sal_strcat(type_str, "SecureConfigured");
    }
    else
    {
        sal_strcat(type_str, "SecureLearned");
    }
    if (p_db_fdb)
    {
        tbl_interface_get_name_by_ifindex(p_db_fdb->ifindex, ifname, IFNAME_SIZE);
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_fprintf(fp, "%-7u %-16s %-16s %-8s\n",
            p_db_fdb->key.fid,
            cdb_mac_addr_val2str(mac_str, CMD_BUF_32, p_db_fdb->key.mac),
            type_str,
            ifname_ext);
    }
    else if (p_db_psfdb)
    {
        tbl_interface_get_name_by_ifindex(p_db_psfdb->ifindex, ifname, IFNAME_SIZE);
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_fprintf(fp, "%-7u %-16s %-16s %-8s\n",
            p_db_psfdb->key.fid,
            cdb_mac_addr_val2str(mac_str, CMD_BUF_32, p_db_psfdb->key.mac),
            type_str,
            ifname_ext);
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_dynamic
 * Purpose      : get dynamic port-security fdb form tbl_fea_fdb_t    
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
hsrv_port_security_get_dynamic(tbl_fea_fdb_t *p_db_fdb, tbl_iter_args_t *pargs)
{
    vid_t vid = *((vid_t *)pargs->argv[0]);
    uint32 ifindex = *((uint32 *)pargs->argv[1]);
    mac_addr_t *mac = (mac_addr_t *)pargs->argv[2];
    hsrv_mac_addr_type_t type = *((hsrv_mac_addr_type_t *)pargs->argv[3]);
    FILE *fp = pargs->argv[4];
    
    if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC) && GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        if (vid > 0 && p_db_fdb->key.fid == vid)
        {
            if ((NULL == mac) || (mac != NULL && (0 == sal_memcmp(mac, p_db_fdb->key.mac, MAC_ADDR_LEN))))
            {
                hsrv_port_security_get_address_one(p_db_fdb, NULL, type, fp);
            }
        }
        else if (ifindex > 0 && p_db_fdb->ifindex == ifindex)
        {
            hsrv_port_security_get_address_one(p_db_fdb, NULL, type, fp);
        }
        else
        {
            return HSRV_E_NONE;
        }
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_static
 * Purpose      : get static port-security fdb orm tbl_psfdb_t       
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
hsrv_port_security_get_static(tbl_psfdb_t *p_db_psfdb, tbl_iter_args_t *pargs)
{
    vid_t vid = *((vid_t *)pargs->argv[0]);
    uint32 ifindex = *((uint32 *)pargs->argv[1]);
    mac_addr_t *mac = (mac_addr_t *)pargs->argv[2];
    hsrv_mac_addr_type_t type = *((hsrv_mac_addr_type_t *)pargs->argv[3]);
    FILE *fp = pargs->argv[4];
    
    if (GLB_FLAG_ISSET(p_db_psfdb->flags, GLB_FDB_FLAG_STATIC) && GLB_FLAG_ISSET(p_db_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        if (vid > 0 && p_db_psfdb->key.fid == vid)
        {
            if ((NULL == mac) || (mac != NULL && (0 == sal_memcmp(mac, p_db_psfdb->key.mac, MAC_ADDR_LEN))))
            {
                hsrv_port_security_get_address_one(NULL, p_db_psfdb, type, fp);
            }
        }
        else if (ifindex > 0 && p_db_psfdb->ifindex == ifindex)
        {
            hsrv_port_security_get_address_one(NULL, p_db_psfdb, type, fp);
        }
        else
        {
            return HSRV_E_NONE;
        }
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_psfdb
 * Purpose      : get the fdb form hash by port-security type      
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
hsrv_port_security_get_psfdb(vid_t vid, uint32 ifindex, uint8 *mac, hsrv_mac_addr_type_t type, FILE *fp)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &vid;
    args.argv[1] = &ifindex;
    args.argv[2] = mac;
    args.argv[3] = &type;
    args.argv[4] = fp;

    if (HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY == type)
    {
        tbl_psfdb_iterate((TBL_ITER_CB_FUNC)hsrv_port_security_get_static, &args);
    }
    else if (HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY == type)
    {
        tbl_fea_fdb_iterate((TBL_ITER_CB_FUNC)hsrv_port_security_get_dynamic, &args);
    }
    else if (HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY == type)
    {
        tbl_fea_fdb_iterate((TBL_ITER_CB_FUNC)hsrv_port_security_get_dynamic, &args);
        tbl_psfdb_iterate((TBL_ITER_CB_FUNC)hsrv_port_security_get_static, &args);
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_address
 * Purpose      : show the port-security fdb by mac or all      
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_security_get_address(hsrv_mac_addr_type_t type, uint8* mac, cfg_cmd_para_t* para)
{
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    vid_t vid = 0;
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    _hsrv_port_security_address_show_banner(fp);

    for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
    {
        sal_memset(&key, 0, sizeof(tbl_vlan_key_t));
        key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (NULL == p_db_vlan)
        {
            continue;
        }
        hsrv_port_security_get_psfdb(vid, 0, mac, type, fp);
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);

    sal_fclose(fp);

    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_port_security_get_address_vlan_port
 * Purpose      : show the port-security fdb by port or vlan      
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_security_get_address_vlan_port(vid_t vid, uint32 ifindex, hsrv_mac_addr_type_t type, cfg_cmd_para_t* para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    _hsrv_port_security_address_show_banner(fp);

    hsrv_port_security_get_psfdb(vid, ifindex, NULL, type, fp);  

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);

    sal_fclose(fp);

    return rc;
}


/***************************************************************************************************
 * Name         : hsrv_port_security_cmd_process_psfdb
 * Purpose      : show the psfdb          
 * Input        : argv; argc; para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_security_cmd_process_psfdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    hsrv_mac_addr_type_t type = HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t  if_key;
    vid_t vid = 0;
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
            if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_memset(&if_key, 0, sizeof(if_key));
                sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&if_key);
                IFNAME_ETH2FULL(argv[1], ifname_ext);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                } 
                if (GLB_IF_MODE_L2 != p_db_if->mode)
                {
                    CFG_INVALID_PARAM_RET("Interface %s is not switch port", ifname_ext);
                }
                rc = hsrv_port_security_get_interface_status(p_db_if, para);
                return rc;
            }
            else if(0 == sal_strcmp(argv[0], "status"))
            {
                 rc = hsrv_port_security_get_all_status(para);
                 return rc;
            }
            else if(0 == sal_strcmp(argv[0], "address-table"))
            {
                if (0 == sal_strcmp(argv[1], "static"))
                {
                    type = HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY;
                }
                else if (0 == sal_strcmp(argv[1], "dynamic"))
                {
                    type = HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY;
                }
                else if (0 == sal_strcmp(argv[1], "all"))
                {
                    type = HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid show type %s", argv[1]);
                }

                if (2 == argc)
                {
                    rc = hsrv_port_security_get_address(type, NULL, para);
                }
                else if (4 == argc)
                {
                    if (0 == sal_strcmp(argv[2], "interface"))
                    {
                        sal_memset(&if_key, 0, sizeof(if_key));
                        sal_strncpy(if_key.name, argv[3], IFNAME_SIZE);
                        p_db_if = tbl_interface_get_interface(&if_key);
                        if (NULL == p_db_if)
                        {
                            IFNAME_ETH2FULL(argv[3], ifname_ext);
                            CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                        } 
                        rc = hsrv_port_security_get_address_vlan_port(0, p_db_if->ifindex, type, para);
                    }
                    else if (0 == sal_strcmp(argv[2], "vlan"))
                    {
                        PM_CFG_GET_UINT(vid, argv[3], para->p_rs);
                        rc = hsrv_port_security_get_address_vlan_port(vid, 0, type, para);
                    }
                    else if (0 == sal_strcmp(argv[2], "address"))
                    {
                        sal_memset(mac, 0, sizeof(mac));
                        rc = cdb_mac_str2val(mac, argv[3]);
                        if (rc < 0)
                        {
                            CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[3]);
                        }
                        rc = hsrv_port_security_get_address(type, mac, para);
                    }
                    else
                    {
                        CFG_INVALID_PARAM_RET("Invalid type %s", argv[2]);
                    }
                }
                else
                {
                    CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
                }
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
hsrv_port_security_start()
{   
    cdb_register_cfg_act_cb(ACT_SHOW_PSFDB, hsrv_port_security_cmd_process_psfdb);
    return HSRV_E_NONE;
}

