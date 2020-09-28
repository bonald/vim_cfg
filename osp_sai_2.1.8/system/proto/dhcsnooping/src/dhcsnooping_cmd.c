
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "proto.h"
#include "dhcsnooping.h"
#include "dhcsnooping_func.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_ipsg_s_mac_define.h"
#include "gen/tbl_ipsg_s_mac.h"
#include "param_check.h"
#include "lib_fs.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"

extern void reset_dhcsnooping_statistics();
extern int is_dhcsnooping_service_enabled(void);
extern int dhcsnooping_modify_database_saver(u_int16_t nNewInterval);
extern int dhcsnooping_binding_execute(int bIPv6, int cmpType, 
                int type, void *pArg, int delOrShow, void *pCbArg);
extern int dhcsnooping_binding_exist_check(tbl_dhcbinding_t *pItemNew, 
                char *pszErrMsg, size_t nErrBufsz);
extern void dhcsnooping_binding_hash_add(ctclib_hash_t *pHash,
                ctclib_hash_t *pHashIp, tbl_dhcbinding_t *pItem);
extern void dhcsnooping_binding_hash_del(tbl_dhcbinding_t *pItem);
extern int dhcp_snooping_write_file(u_int8_t bV4, u_int8_t bV6);
extern int dhcsnooping_reload_database(FILE *pFile, u_int8_t bIPv6, 
                int bSyslog, char *pszErrMsg, size_t nErrBufsz);
extern void output_dhcsnooping_statistics(FILE *fp);
extern int dhcsnooping_binding_queue_count(int bIPv6, u_int32_t *learn, u_int32_t *config);

DEBUG_DECLAR(dhcp_debug);

int32
check_dhcbinding_vlan_exist(uint32 vlanid)
{
    vid_t vid = 0;
    tbl_vlan_t tbl_vlan;
    tbl_vlan_t *p_tbl_vlan = NULL;

    sal_memset(&tbl_vlan, 0, sizeof(tbl_vlan));
    vid = (vid_t)vlanid;
    tbl_vlan.key.vid = vid;
    p_tbl_vlan = tbl_vlan_get_vlan(&tbl_vlan.key);
    if (NULL != p_tbl_vlan)
    {
        return PM_E_EXIST;
    }

    return PM_E_NOT_EXIST;
}

/* --- dhcsnooping --- */
static int
_check_dhcsnooping_field(char *pszkey, char *pszValue, tbl_dhcsnooping_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = cdb_get_tbl(TBL_DHCSNOOPING);
    tbl_vlan_t      *p_db_vlan = NULL;
    cdb_field_t     *p_field = NULL;
    tbl_vlan_key_t  key;
    int32           field_id;
    int             nValue;
    int             rc;
    uint32 vid = 0;
    
    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_dhcsnooping_field_id_t)field_id) {
    case TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid bool value"); 
            goto err_out;
        }
        pitem->verify_mac_address = atoi(pszValue);
        break;
    case TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid bool value"); 
            goto err_out;
        }
        pitem->hostname_as_remote_id = atoi(pszValue);
        break;
    case TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING:
        if (cdb_check_str(pszValue, DHCSNOOPING_MAX_REMOTE_ID_STRING - 1,
                           para->p_rs->desc) < 0) {
            goto err_out;
        }
        snprintf(pitem->remote_id_string, DHCSNOOPING_MAX_REMOTE_ID_STRING, "%s", pszValue);
        break;
    case TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < DHCSNOOPING_SAVE_BINDING_INTERVAL_MIN
                || nValue > DHCSNOOPING_SAVE_BINDING_INTERVAL_MAX) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid interval value"); 
            goto err_out;
        }
        pitem->database_save_interval = nValue;
        break;

    case TBL_DHCSNOOPING_FLD_VLANS:
        sal_memset(&key, 0x00, sizeof(key));
        key.vid = sal_atoi(pszValue);
        /* check exist */
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (!p_db_vlan) {
            //CFG_INVALID_PARAM_RET("Vlan %d is not created", key.vid);
        }
        rc = cdb_bitmap_str2val(pszValue, GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, 
                pitem->vlans, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
        if (rc < 0) {
            goto err_out;
        }

        /*fix bug:43858, add check every vlan exist*/
        GLB_BMP_ITER_BEGIN(pitem->vlans, vid)
        {
            if (PM_E_NOT_EXIST == check_dhcbinding_vlan_exist(vid)) 
            {
                CFG_INVALID_PARAM_RET("Vlan %u is not created", vid);
            }
        }
        GLB_BMP_ITER_END(pitem->vlans, vid);
        /*fix bug:43858 end*/
        break;
    case TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS:
        if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid add agent options value"); 
            goto err_out;
        }
        pitem->add_agent_options = atoi(pszValue);
        break;
    case TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED:
        if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid value"); 
            goto err_out;
        }
        pitem->allow_untrusted = atoi(pszValue);
        break;
    case TBL_DHCSNOOPING_FLD_MAX:
    case TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_dhcsnooping_cmd_get(tbl_dhcsnooping_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dhcsnooping_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
dhcsnooping_set(tbl_dhcsnooping_field_id_t field_id, tbl_dhcsnooping_t *pitem)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    switch (field_id) {
    case TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL:
    case TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS:
    case TBL_DHCSNOOPING_FLD_VLANS:
    case TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED:
    case TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED:
    case TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS:
    case TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING:
    case TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID:
        break;
    case TBL_DHCSNOOPING_FLD_MAX:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_dhcsnooping_set_dhcsnooping_field(pitem, field_id));
    return rc;
}

int32
dhcsnooping_process_l3pdu()
{
    tbl_dhcsnooping_t *p_db_item = tbl_dhcsnooping_get_dhcsnooping();
    int bService = is_dhcsnooping_service_enabled();
    tbl_dhcsnooping_t item;
    uint32 vidx;
    int bEnable;
    int ops = 0;
    tbl_vlan_t      *p_db_vlan = NULL;
    tbl_vlan_key_t  vlan_key;

    sal_memset(&vlan_key, 0 ,sizeof(vlan_key));    
    sal_memcpy(&item.snooping_acl_applied, &p_db_item->snooping_acl_applied, sizeof(vlan_bmp_t));
    for(vidx = 0; vidx < GLB_BMP_BITS(p_db_item->vlans); vidx++) 
    {
        bEnable = 0;
        if (GLB_BMP_ISSET(p_db_item->vlans, vidx) && bService) 
        {
            bEnable = 1;
        }

        vlan_key.vid = vidx;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            continue;
        }
        
        if (GLB_BMP_ISSET(item.snooping_acl_applied, vidx)) 
        {
            if (0 == bEnable) 
            {
                /* disable acl */
                GLB_BMP_UNSET(item.snooping_acl_applied, vidx);
                p_db_vlan->dhcsnooping = FALSE;
                tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_DHCSNOOPING);
                ops++;
            }
        } 
        else if (1 == bEnable) 
        {
            /* enable acl */
            GLB_BMP_SET(item.snooping_acl_applied, vidx);
            p_db_vlan->dhcsnooping = TRUE;            
            tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_DHCSNOOPING);
            ops++;
        }
    }

    if (ops > 0) 
    {
        dhcsnooping_set(TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED, &item);
    }

    return 0;
}

static int32
_dhcsnooping_cmd_set(tbl_dhcsnooping_field_id_t field_id, tbl_dhcsnooping_t *pitem, 
    tbl_dhcsnooping_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL:
        if (pitem->database_save_interval == p_db_item->database_save_interval) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_VLANS:
        if (!memcmp(pitem->vlans, p_db_item->vlans, sizeof(vlan_bmp_t))) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS:
        if (pitem->add_agent_options == p_db_item->add_agent_options) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED:
        if (pitem->allow_untrusted == p_db_item->allow_untrusted) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID:
        if (pitem->hostname_as_remote_id == p_db_item->hostname_as_remote_id) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING:
        if (!sal_strcmp(pitem->remote_id_string, p_db_item->remote_id_string)) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS:
        if (pitem->verify_mac_address == p_db_item->verify_mac_address) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCSNOOPING_FLD_MAX:
    case TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED:
        break;
    }

    PM_E_RETURN(dhcsnooping_set(field_id, pitem));

    /* apply/disable snooping acls */
    if (TBL_DHCSNOOPING_FLD_VLANS == field_id) 
    {
        dhcsnooping_process_l3pdu();
    } 
    else if (TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL == field_id) 
    {
        dhcsnooping_modify_database_saver(pitem->database_save_interval);
    } 
    else if (TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID == field_id) 
    {
        if (pitem->hostname_as_remote_id) 
        {
            gethostname(pitem->remote_id_string, DHCSNOOPING_MAX_REMOTE_ID_STRING);
            dhcsnooping_set(TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING, pitem);
        }
    }

    return rc;
}

static int32
_dhcsnooping_interface_cfg(tbl_interface_t *p_intf, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs;

    if (NULL == p_intf) {
        return -1;
    }

    if (p_intf->dhcp_snooping_trust) {
        fprintf(fp, "/key=%s/dhcp_snooping_trust=%u\r\n", 
            p_intf->key.name, p_intf->dhcp_snooping_trust);
    }
    return 0;
}

static int32
_dhcsnooping_get_config(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    /* output interface config */
    tbl_interface_iterate((TBL_ITER_CB_FUNC)_dhcsnooping_interface_cfg, (tbl_iter_args_t *)fp);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_dhcsnooping_get_statistics(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    output_dhcsnooping_statistics(fp);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
dhcsnooping_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_dhcsnooping_field_id_t field_id = TBL_DHCSNOOPING_FLD_MAX;
    cdb_node_t          *p_node     = NULL;
    cdb_field_t         *p_field    = NULL;
    tbl_dhcsnooping_t   *p_db_item  = NULL;
    tbl_dhcsnooping_t   item;
    tbl_dhcsnooping_t   item_tmp;    
    field_parse_t       field;
    uint32      vidx;
    vid_t        vlanid;
    int32       i   = 0;
    int32       rc  = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCSNOOPING);    
    p_db_item = tbl_dhcsnooping_get_dhcsnooping();
    sal_memset(&item, 0x00, sizeof(tbl_dhcsnooping_t));
    switch (para->oper) {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        if (!sal_strcmp(argv[0], "statistics") && 1 == argc) {
            reset_dhcsnooping_statistics();
        } else if (!sal_strcmp(argv[0], "vlans") && 2 == argc) {
            sal_memset(&item_tmp, 0x00, sizeof(tbl_dhcsnooping_t));
                
            rc = cdb_bitmap_str2val(argv[1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, 
                item_tmp.vlans, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0) {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }

            /* modified by liwh for bug 50962, 2019-02-14 
                 when configure no dhcp snooping vlan vlan_id,  should clear learning binding entries for this vlan */
            GLB_BMP_ITER_BEGIN(item_tmp.vlans, vlanid)
            {
                dhcsnooping_binding_execute(0, DHCBINDING_FILTER_VLAN, DHCBINDING_TYPE_LEARNING,
                    (void *)&vlanid, 1, NULL);
            }
            GLB_BMP_ITER_END(item_tmp.vlans, vlanid);
            /* liwh end */

            sal_memcpy(&item.vlans, &p_db_item->vlans, sizeof(vlan_bmp_t));
            /* clear bits */
            vidx = 0;
            GLB_BMP_ITER_BEGIN(item_tmp.vlans, vidx) {
                if (GLB_BMP_ISSET(p_db_item->vlans, vidx)) {
                    GLB_BMP_UNSET(item.vlans, vidx);
                }
            }
            GLB_BMP_ITER_END(item_tmp.vlans, vidx);
            rc |= _dhcsnooping_cmd_set(TBL_DHCSNOOPING_FLD_VLANS, &item, p_db_item, para);
        } else {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        break;

    case CDB_OPER_SET:
        if (1 == argc) {
            if (!sal_strcasecmp(argv[0], "remote_id_string")) { 
                field_id = TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING;
                sal_memset(item.remote_id_string, 0x00, DHCSNOOPING_MAX_REMOTE_ID_STRING); 
                rc = _dhcsnooping_cmd_set(field_id, &item, p_db_item, para);

                field_id = TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID;
                item.hostname_as_remote_id = 0;
                rc = _dhcsnooping_cmd_set(field_id, &item, p_db_item, para);
                return rc;

            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
        }
        
        if ((argc % 2) != 0) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 0; i < argc; i += 2) {
            if (_check_dhcsnooping_field(argv[i], argv[i + 1], &item, para) < 0) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access) {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 0; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            if (TBL_DHCSNOOPING_FLD_VLANS == field_id) {
                /* setting old bits */
                vidx = 0;
                
                GLB_BMP_ITER_BEGIN(p_db_item->vlans, vidx) {
                    GLB_BMP_SET(item.vlans, vidx);
                }
                GLB_BMP_ITER_END(p_db_item->vlans, vidx);

                vlanid = sal_atoi(argv[1]);
                if (vlanid) {
                    if (PM_E_NOT_EXIST == check_dhcbinding_vlan_exist(vlanid)) {
                        CFG_INVALID_PARAM_RET("Vlan %u is not created", vlanid);
                    }
                    GLB_BMP_SET(item.vlans, vlanid);
                }
            }
            rc |= _dhcsnooping_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc && (!sal_strcasecmp(argv[0], "interfaces") 
                           || !sal_strcasecmp(argv[0], "trusted"))) {
            rc = _dhcsnooping_get_config(para);
            break;
        } else if (1 == argc && !sal_strcasecmp(argv[0], "statistics")) {
            rc = _dhcsnooping_get_statistics(para);
            break;
        }

        if (1 == argc && !sal_strcasecmp(argv[0], "config")) {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        } else {    
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0) {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }

        rc = _dhcsnooping_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int
dhcsnooping_check_ip(const char *pszAddr)
{
    gensock_t su;
    struct in6_addr addr6;
    u_int32_t network = 0;

    /* is it ip address */
    if (0 != str2gensock(&su, pszAddr)) {
        return -1;
    } 

    if (AF_INET == su.sa.sa_family) {
        u_char *p =  (u_char *)&su.sin.sin_addr;
        
        /* filtrate ip with 255.255.255.255 */
        if (255 == p[0] && 255 == p[1] && 255 == p[2] && 255 == p[3]) {
            return -1;
        }
        
        network = ntohl (*(int*)&su.sin.sin_addr);
        if (network == 0
            || IN_BADCLASS(network)
            || IN_MULTICAST (network)
            || IN_EXPERIMENTAL(network)
            || IPV4_NET127(network)
            || IPV4_NET0(network)) {
            return -1;
        }

        return 0;
    }

    inet_pton (AF_INET6, pszAddr, &addr6); 
    if (IN6_IS_ADDR_MULTICAST (&addr6)
            || IN6_IS_ADDR_LOOPBACK (&addr6)
            || IN6_IS_ADDR_UNSPECIFIED (&addr6)) {
        return -1;
    }
    return 0;
}

static int
dhcsnooping_check_mac(const char *str)
{
    mac_addr_t  mac;
    char        *mac_str    = NULL;
    char        buf[64];
    int32       rc = PM_E_NONE;

    u_char      szMac[ETH_ALEN] = {'\0'};
    u_int16_t   *psMac = (u_int16_t *)szMac;

    sal_memset(mac, 0, sizeof(mac_addr_t));
    sal_memset(buf, 0, sizeof(buf));
    sal_strncpy(buf, str, 64);
    mac_str = buf;

    rc = cdb_arp_mac_address_is_valid(mac_str);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    if (3 != sal_sscanf(mac_str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        return PM_E_FAIL;
    }
    
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);

    psMac[0] = *(uint16*)&mac[0];
    psMac[1] = *(uint16*)&mac[2];
    psMac[2] = *(uint16*)&mac[4];

    if (MAC_IS_BROADCAST(szMac)
        || MAC_IS_UNSPECIFIED(szMac)
        || MAC_IS_MULTICAST(szMac)) 
    {
        return PM_E_FAIL;     
    }

    return PM_E_NONE;
}

/* -- dhcp binding table */
static int
_check_dhcbinding_field(char *pszkey, char *pszValue, 
    tbl_dhcbinding_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t*     p_node = cdb_get_tbl(TBL_DHCBINDING);
    cdb_field_t     *p_field = NULL;
    int32           field_id = TBL_DHCBINDING_FLD_MAX;
    tbl_interface_t stIntf;
    tbl_interface_t *pIntf;
    u_int32_t       nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_dhcbinding_field_id_t)field_id) {
    case TBL_DHCBINDING_FLD_VID:
        nValue = atoi(pszValue);
        if (sal_strspn(pszValue, "0123456789") != sal_strlen(pszValue)
                || nValue < 1 || nValue > 4094) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid VLAN id %s", pszValue); 
            goto err_out;
        }
        pitem->vid = nValue;
        break;
    case TBL_DHCBINDING_FLD_IFNAME:
        sal_memset(&stIntf, 0, sizeof(tbl_interface_t));
        sal_snprintf(stIntf.key.name, IFNAME_SIZE, pszValue);

        if (NULL == (pIntf = tbl_interface_get_interface(&stIntf.key))) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "No such interface %s", pszValue); 
            goto err_out;
        }
        if (GLB_IF_MODE_L2 != pIntf->mode) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Please specify layer 2 interface"); 
            goto err_out;
        }
        sal_snprintf(pitem->ifname, IFNAME_SIZE, "%s", pszValue);
        break;
    case TBL_DHCBINDING_FLD_CLIENT_MAC:
        if (cdb_mac_str2val_dhcs(pitem->client_mac, pszValue, sal_strlen(pszValue)) != PM_E_NONE
                || dhcsnooping_check_mac(pszValue) != PM_E_NONE) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid client mac %s", pszValue); 
            goto err_out;
        }
        break;
    case TBL_DHCBINDING_FLD_CLIENT_IP:
        if (cdb_addr_str2val(&pitem->client_ip, pszValue, sal_strlen(pszValue)) != PM_E_NONE
                || dhcsnooping_check_ip(pszValue) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid client ip %s", pszValue); 
            goto err_out;
        }
        break;
    case TBL_DHCBINDING_FLD_LEASE:
        nValue = strtoul(pszValue, NULL, 10);
        if (sal_strspn(pszValue, "0123456789") != sal_strlen(pszValue)
            && nValue > DHCSNOOPING_DEFAULT_BOOTP_LEASE_TIME) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid lease time %s", pszValue); 
            goto err_out;
        }
        if (0 == nValue) { /* static entry */
            pitem->lease = 0;
        } else {
            struct sal_timeval tv;

            ctc_task_update_boottime(&tv);
            pitem->lease = nValue + ctc_time_boottime_sec(NULL);
        }
        break;
    case TBL_DHCBINDING_FLD_TRANSID:
        if (sal_strspn(pszValue, "0123456789") != sal_strlen(pszValue)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid transaction id %s", pszValue); 
            goto err_out;
        }
        pitem->transid = strtoul(pszValue, 0x00, 16);
        break;     
    case TBL_DHCBINDING_FLD_STATE:
        if ((nValue = cdb_enum_str2val(dhcbinding_state_strs, 
                DHCBINDING_STATE_MAX, pszValue)) == -1) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid state value"); 
            goto err_out;
        }
        pitem->state = nValue;
        break;   
    case TBL_DHCBINDING_FLD_TYPE:
        if ((nValue = cdb_enum_str2val(dhcbinding_type_strs, 
                DHCBINDING_TYPE_MAX, pszValue)) == -1) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid type value"); 
            goto err_out;
        }
        pitem->type = nValue;
        break;
    case TBL_DHCBINDING_FLD_CIPSOUR:
    case TBL_DHCBINDING_FLD_KEY:
    case TBL_DHCBINDING_FLD_MAX:
        break;
    default:
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

int32
dhcsnooping_mac_check_with_cdbstr(const char *str, cfg_cmd_para_t *para)
{
    char *mac_str = NULL;
    char *fid_str = NULL;
    char buf[64];

    sal_strncpy(buf, str, 64);
    fid_str = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == fid_str)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    mac_str = buf;
    *fid_str = CMD_ZERO_CHAR;
    fid_str += 1;
    
    if (PM_E_NONE != dhcsnooping_check_mac(mac_str))
    {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid MAC address %s", mac_str); 
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}

static int32
_dhcbinding_add(tbl_dhcbinding_t *pitem, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_dhcbinding_master_t *pMaster = tbl_dhcbinding_get_master();
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    char szIpBuf[128];
    int i;

    if (NULL != tbl_dhcbinding_get_dhcbinding(&pitem->key)) 
    {
        return PM_E_NONE;
    }

    /*mac check*/
    if (PM_E_NONE != dhcsnooping_mac_check_with_cdbstr(argv[0], para))
    {
        goto err_out;
    }

    for (i = 1; i < argc; i += 2) 
    {
        if (_check_dhcbinding_field(argv[i], argv[i + 1], pitem, para) == -1) 
        {
            goto err_out;
        }
    }
    
    if (dhcsnooping_binding_exist_check(pitem, para->p_rs->desc, MAX_CMD_RET_LEN) != 0) 
    {
        goto err_out;
    }

    if ((!pitem->key.ipv6 && pMaster->dhcbinding_count >= DHCSNOOPING_MAX_BINDING_ITEMS)
        || (pitem->key.ipv6 && pMaster->dhcbindingv6_count >= DHCSNOOPINGV6_MAX_BINDING_ITEMS)) 
    {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
            "The DHCP%s snooping binding entry num is more than maximum %u!", pitem->key.ipv6 ? "v6" : "",
            pitem->key.ipv6 ? DHCSNOOPINGV6_MAX_BINDING_ITEMS : DHCSNOOPING_MAX_BINDING_ITEMS);
        goto err_out;
    }
    
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "manual add binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
              "%s %s lease %d", pitem->vid,
              pitem->key.mac[0], pitem->key.mac[1], pitem->key.mac[2], 
              pitem->key.mac[3], pitem->key.mac[4], pitem->key.mac[5],
              cdb_addr_val2str((char*)szIpBuf, 128, &pitem->client_ip),
              IFNAME_ETH2FULL(pitem->ifname, ifname_ext), 
              (pitem->lease ? pitem->lease - ctc_time_boottime_sec(NULL) : 0));
    dhcsnooping_binding_hash_add(pMaster->dhcbinding_hash, 
        pMaster->dhcbinding_ip_hash, pitem);
    return PM_E_NONE;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_dhcbinding_del(tbl_dhcbinding_t *pitem, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    if (NULL == tbl_dhcbinding_get_dhcbinding(&pitem->key)) {
        return PM_E_NONE;
    }

    dhcsnooping_binding_hash_del(pitem);
    return PM_E_NONE;
}


static int32
tbl_dhcbinding_dump_filter(tbl_dhcbinding_t *p_dhcbinding, tbl_iter_args_t *pargs)
{
    tbl_dhcbinding_t *p_filter = (tbl_dhcbinding_t *)pargs->argv[2];

    if (NULL == p_filter || p_dhcbinding == p_filter) {
        return tbl_dhcbinding_dump_one(p_dhcbinding, pargs);
    }

    return PM_E_NONE;
}

static int32
_dhcbinding_cmd_get(tbl_dhcbinding_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_dhcbinding_iterate((TBL_ITER_CB_FUNC)tbl_dhcbinding_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_dhcbinding_cmd_set(tbl_dhcbinding_field_id_t field_id, tbl_dhcbinding_t *pitem, 
    tbl_dhcbinding_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_DHCBINDING_FLD_IFNAME:
        if (!strcasecmp(pitem->ifname, p_db_item->ifname)) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCBINDING_FLD_CLIENT_MAC:
        if (!memcmp(pitem->client_mac, p_db_item->client_mac, sizeof(mac_addr_t))) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCBINDING_FLD_CLIENT_IP:
        if (!memcmp(&pitem->client_ip, &p_db_item->client_ip, sizeof(addr_t))) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCBINDING_FLD_LEASE:
        if (pitem->lease == p_db_item->lease) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCBINDING_FLD_TRANSID:
        if (pitem->transid == p_db_item->transid) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCBINDING_FLD_VID:
        if (pitem->vid == p_db_item->vid) {
            return PM_E_NONE;
        }
        break;        
    case TBL_DHCBINDING_FLD_STATE:
        if (pitem->state == p_db_item->state) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCBINDING_FLD_TYPE:
        if (pitem->type == p_db_item->type) {
            return PM_E_NONE;
        }
        break; 

    case TBL_DHCBINDING_FLD_CIPSOUR:
    case TBL_DHCBINDING_FLD_KEY:
    case TBL_DHCBINDING_FLD_MAX:
        return PM_E_NONE;
        
    default:
        return PM_E_NONE;        
    }

    rc = tbl_dhcbinding_set_dhcbinding_field(pitem, field_id);
    return rc;
}

int32
_dhcbinding_cmd_delshow(char **argv, int32 argc, cfg_cmd_para_t *para, int bDel, void *pCbArg)
{
    int type;
    
    if (1 == argc && !strcasecmp(argv[0], "configed")) 
    {
        dhcsnooping_binding_execute(0, DHCBINDING_FILTER_ALL, 
            DHCBINDING_TYPE_CONFIGED, NULL, bDel, pCbArg);
    } 
    else if (1 == argc && !strcasecmp(argv[0], "learning")) 
    {
        dhcsnooping_binding_execute(0, DHCBINDING_FILTER_ALL, 
            DHCBINDING_TYPE_LEARNING, NULL, bDel, pCbArg);
    } 
    else if (1 == argc && !strcasecmp(argv[0], "all")) 
    {
        dhcsnooping_binding_execute(0, DHCBINDING_FILTER_ALL, 
            DHCBINDING_TYPE_ALL, NULL, bDel, pCbArg);
    } 
    else if (3 == argc 
        && (!strcasecmp(argv[0], "configed") || !strcasecmp(argv[0], "learning")
            || !strcasecmp(argv[0], "all"))) 
    {
        if (!strcasecmp(argv[0], "configed")) 
        {
            type = DHCBINDING_TYPE_CONFIGED;
        } 
        else if (!strcasecmp(argv[0], "learning")) 
        {
            type = DHCBINDING_TYPE_LEARNING;
        } 
        else 
        {
            type = DHCBINDING_TYPE_ALL;
        }
        if (!strcasecmp(argv[1], "ipv4")) 
        {
            addr_t client_ip;
            if (cdb_addr_str2val(&client_ip, argv[2], strlen(argv[2])) != PM_E_NONE) {
                CFG_INVALID_PARAM_RET("Invalid client ip %s", argv[2]);
            }
            dhcsnooping_binding_execute(0, DHCBINDING_FILTER_IP, type, 
                (void *)&client_ip, bDel, pCbArg);
        } 
        else if (!strcasecmp(argv[1], "mac")) 
        {
            mac_addr_t mac;
            if (cdb_mac_addr_str2val(mac, argv[2]) != PM_E_NONE) {
                CFG_INVALID_PARAM_RET("Invalid MAC %s", argv[2]);
            }
            dhcsnooping_binding_execute(0, DHCBINDING_FILTER_MAC, type, 
                (void *)mac, bDel, pCbArg);
        } 
        else if (!strcasecmp(argv[1], "vlan")) 
        {
            vid_t vid = atoi(argv[2]);
            if (strspn(argv[2], "0123456789") != strlen(argv[2])
                || vid < 1 || vid > 4094) {
                CFG_INVALID_PARAM_RET("Invalid VLAN %s", argv[2]);
            }
            dhcsnooping_binding_execute(0, DHCBINDING_FILTER_VLAN, type,
                (void *)&vid, bDel, pCbArg);
        } 
        else if (!strcasecmp(argv[1], "interface")) 
        {
            tbl_interface_t *pIntf = tbl_interface_get_interface_by_name(argv[2]);
            if (NULL == pIntf) {
                CFG_INVALID_PARAM_RET("No such interface %s", argv[2]);
            }
            dhcsnooping_binding_execute(0, DHCBINDING_FILTER_INTERFACE, type, 
                (void *)argv[2], bDel, pCbArg);
        } 
        else 
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } 
    else 
    {
        CFG_INVALID_PARAM_RET("Invalid parameters");
    }    
    return PM_E_NONE;
}

static int32
_dhcbinding_output_database(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_dhcbinding_output_summary(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    u_int32_t learning = 0;
    u_int32_t configed = 0;
    
    dhcsnooping_binding_queue_count(0, &learning, &configed);
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_fprintf(fp, "/dhcp_binding_learning_count=%u/dhcp_binding_configed_count=%u\n",
        learning, configed);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
dhcbinding_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_dhcbinding_field_id_t field_id = TBL_DHCBINDING_FLD_MAX;
    tbl_dhcbinding_t    *p_db_item  = NULL;
    cdb_node_t          *p_node     = NULL;
    cdb_field_t         *p_field    = NULL;
    field_parse_t       field;
    tbl_dhcbinding_t    item;
    char                buf[128];
    int32               rc  = PM_E_NONE;
    int32               i   = 0;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCBINDING);  
    sal_memset(buf, 0, sizeof(buf));
    
    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        sal_memset(&item, 0, sizeof(tbl_dhcbinding_t));
        rc = cdb_binding_key_str2val(&item.key, argv[0], para->p_rs->desc);
        if (rc < 0) {
            if (CDB_OPER_SET == para->oper) {
                CFG_INVALID_PARAM_RET("Invalid key %s", argv[0]);
            } 
        } else {
            p_db_item = tbl_dhcbinding_get_dhcbinding(&item.key);
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL != p_db_item) {
            /*modified by chenc for bug 52590, 2019-06-13*/
            CFG_INVALID_PARAM_RET("Binding item with the same MAC address and Vlan exist");
            /*chenc ended*/
        }
        rc = _dhcbinding_add(&item, argv, argc, para);
        break;
    case CDB_OPER_DEL:
        if (NULL != p_db_item) { /* delete target entry */
            rc = _dhcbinding_del(&item, argv, argc, para);
        } else {
            rc = _dhcbinding_cmd_delshow(argv, argc, para, 1, NULL);
        }
        break;
    case CDB_OPER_SET:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("DHCP Binding Item %s not found",  
                cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, item.key.mac));
        }
        if (2 == argc) {
            if (!strcasecmp(argv[1], "ifname")) { 
                field_id = TBL_DHCBINDING_FLD_IFNAME;
                memset(item.ifname, 0x00, IFNAME_SIZE); 
            } else if (!strcasecmp(argv[1], "client_ip")) { 
                field_id = TBL_DHCBINDING_FLD_CLIENT_IP;
                memset(&item.client_ip, 0x00, sizeof(addr_t)); 
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _dhcbinding_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_dhcbinding_field(argv[i], argv[i + 1], 
                    &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _dhcbinding_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc && !strcasecmp(argv[0], "summary")) {
            rc = _dhcbinding_output_summary(para);
            break;
        }

        if (1 == argc && !strcasecmp(argv[0], "database")) {
            rc = _dhcbinding_output_database(para);
            break;
        }
    
        if (argc > 0 && NULL == p_db_item) {
            FILE *fp = NULL;

            CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
            fp = sal_fopen(para->p_show->path, "w+");
            rc = _dhcbinding_cmd_delshow(argv, argc, para, 0, (void *)fp);
            sal_fclose(fp);
        } else {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _dhcbinding_cmd_get(p_db_item, para, p_node, &field);    
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
dhcsnooping_db_act_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32       rc      = PM_E_NONE;
    FILE        *fp     = NULL;
	char 		filePath[128] = "";

    CFG_DBG_FUNC();

    switch (para->oper) {
    case CDB_OPER_SET:
        if (1 == argc && !strcasecmp(argv[0], "save")) {
            dhcp_snooping_write_file(1, 1);
        } else if (2 == argc && !strcasecmp(argv[0], "load")) {
            /* check filename */
			sal_snprintf(filePath, sizeof(filePath), "/tmp/%s",argv[1]);
            fp = sal_fopen(filePath, "r");
            if (NULL == fp) {
                CFG_FUNC_ERR_RET("Can't open file: %s", sal_strerror(errno));
            }
            para->p_rs->desc[0] = '\0';
            rc = dhcsnooping_reload_database(fp, 0, 1, para->p_rs->desc, MAX_CMD_RET_LEN);
            sal_fclose(fp);
            if (0 != rc || '\0' != para->p_rs->desc[0]) {
                rc = PM_E_FAIL;
                para->p_rs->rc  = rc;
            } else {
                rc = PM_E_NONE;
            }
        } else if (2 == argc && !strcasecmp(argv[0], "loadv6")) {
            /* check filename */
            fp = sal_fopen(argv[1], "r");
            if (NULL == fp) {
                CFG_FUNC_ERR_RET("Can't open file: %s", sal_strerror(errno));
            }
            para->p_rs->desc[0] = '\0';
            rc = dhcsnooping_reload_database(fp, 1, 1, para->p_rs->desc, MAX_CMD_RET_LEN);
            sal_fclose(fp);
            if (0 != rc || '\0' != para->p_rs->desc[0]) {
                rc = PM_E_FAIL;
                para->p_rs->rc  = rc;
            } else {
                rc = PM_E_NONE;
            }
        } else {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

int
dhcpsnooping_binding_hash_del_main(u_int16_t vid)
{
    tbl_dhcbinding_t    *pItem = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *nextnode = NULL;
    u_int8_t bRun = 0;
    u_int8_t bReq = 0;

    /* del it one by loop */
    CTCLIB_SLIST_LOOP_DEL(DHCSNOOPING_QUEUE, pItem, listnode, nextnode) {
        /*not this interface, continue*/
        if (vid != pItem->vid) {
            continue;
        }

        if (DHCBINDING_STATE_REQUEST == pItem->state) {
            bReq = 1;
        } else {
            bRun = 1;
        } 

        if (bRun) {   /* even if ip equal 0 ?*/
            dhcsnooping_binding_hash_del(pItem);
        }

        if (bReq) {
            tbl_dhcbinding_del_dhcbinding(&pItem->key);
        }
    }    

    return PM_E_NONE;
}

int32
dhcpsnooping_vlan_del_callback(vid_t vid)
{
    tbl_dhcsnooping_t item_tmp;    
    tbl_dhcsnooping_t item;
    tbl_dhcsnooping_t *p_db_item = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&item, 0x00, sizeof(tbl_dhcsnooping_t));
    sal_memset(&item_tmp, 0x00, sizeof(tbl_dhcsnooping_t));
    
    /* for bug 4092 by weij, delete binding of this vlan */
    rc = dhcpsnooping_binding_hash_del_main(vid);
    p_db_item = tbl_dhcsnooping_get_dhcsnooping();
    sal_memcpy(&item.vlans, &p_db_item->vlans, sizeof(vlan_bmp_t));

    /* clear bits */

    if (GLB_BMP_ISSET(p_db_item->vlans, vid)) {
        GLB_BMP_UNSET(item.vlans, vid);
    }

    rc = dhcsnooping_set(TBL_DHCSNOOPING_FLD_VLANS, &item);
    rc = dhcsnooping_process_l3pdu();

    /* for bug 21598 modified status added by liuyang 2012-12-24*/
    if (!dhcsnooping_vlan_cnt_enabled_check())
    {
        dhcsnooping_service_onoff();
    }
    
    return rc;
}

int32
dhcsnooping_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_key_t  *p_vlan_key = NULL;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        p_vlan_key = (tbl_vlan_key_t*)p_tbl;
        dhcpsnooping_vlan_del_callback(p_vlan_key->vid);
        break;

    case CDB_OPER_SET:        
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
dhcsnooping_ipsour_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ipsg_fib_key_t      *p_ipsg_key     = NULL;
    tbl_ipsg_fib_t          *p_db_ipsg_fib  = NULL;
    tbl_ipsg_fib_t          *p_ipsg_fib     = NULL;
    tbl_dhcbinding_t    *p_db_dhcbinding   = NULL;    
    tbl_dhcbinding_key_t  dhcbinding_key;
    
    if (NULL == p_tbl)
    {
        return PM_E_FAIL;
    }

    sal_memset(&dhcbinding_key, 0, sizeof(dhcbinding_key));

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsg_fib = (tbl_ipsg_fib_t*)p_tbl;
        if (NULL != tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(p_ipsg_fib->mac_addr))
        {
            return PM_E_FAIL;
        }
        
        sal_memcpy(dhcbinding_key.mac, p_ipsg_fib->mac_addr, sizeof(mac_addr_t));
        p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&dhcbinding_key);
        if (NULL == p_db_dhcbinding)
        {
            return PM_E_FAIL;
        }

        p_db_dhcbinding->cIpsour = DHCPSNOOPING_ITEM_ADD_IPSOUR;
        tbl_dhcbinding_set_dhcbinding_field(p_db_dhcbinding, TBL_DHCBINDING_FLD_CIPSOUR);
        break;

    case CDB_OPER_DEL:
        p_ipsg_key = (tbl_ipsg_fib_key_t*)p_tbl;
        p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(p_ipsg_key);
        if (NULL == p_db_ipsg_fib)
        {
            return PM_E_FAIL;
        }

        if (NULL != tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(p_db_ipsg_fib->mac_addr))
        {
            return PM_E_FAIL;
        }

        sal_memcpy(dhcbinding_key.mac, p_db_ipsg_fib->mac_addr, sizeof(mac_addr_t));
        p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&dhcbinding_key);
        if (NULL == p_db_dhcbinding)
        {
            return PM_E_FAIL;
        }

        p_db_dhcbinding->cIpsour = DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR;
        tbl_dhcbinding_set_dhcbinding_field(p_db_dhcbinding, TBL_DHCBINDING_FLD_CIPSOUR);
        break;

    case CDB_OPER_SET:        
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
dhcsnooping_tbl_init()
{
    tbl_dhcsnooping_t dhcsnooping;
    
    /* -- dhcsnooping -- */
    sal_memset(&dhcsnooping, 0x00, sizeof(tbl_dhcsnooping_t));
    dhcsnooping.verify_mac_address = 1;
    dhcsnooping.database_save_interval = DHCSNOOPING_SAVE_BINDING_INTERVAL_DEFAULT;
    tbl_dhcsnooping_set_dhcsnooping_field(&dhcsnooping, TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS);
    tbl_dhcsnooping_set_dhcsnooping_field(&dhcsnooping, TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL);
    return PM_E_NONE;
}

int dhcsnooping_proc_init_done(void)
{
    dhcsnooping_tbl_init();
    return 0;
}
