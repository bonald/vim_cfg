#include "proto.h"
#include "gen/tbl_arpinsp_define.h"
#include "gen/tbl_arpinsp.h"
#include "gen/tbl_arpacl_config_define.h"
#include "gen/tbl_arpacl_config.h"
#include "gen/tbl_arpace_config_define.h"
#include "gen/tbl_arpace_config.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "glb_arpacl_define.h"
#include "cdb_const.h"
#include "cfg_cmd.h"
#include "ccs_cfg.h"

int32
_ccs_cfg_build_arpace_config(FILE* fp, tbl_arpace_config_t *p_arpace_config)
{
    char buf1[256];
    char buf2[256];
    sal_memset(buf1, 0, sizeof(buf1));
    sal_memset(buf2, 0, sizeof(buf2));
    
    /*1.sequence number*/
    sal_fprintf(fp, " %d", p_arpace_config->key.seq_no);

    /*2.deny or permit*/
    sal_fprintf(fp, " %s", (p_arpace_config->arp_deny? "deny" : "permit"));

    /*3. request or response or all*/
    if (p_arpace_config->arp_type == GLB_ARP_REQUEST)
    {
        sal_fprintf(fp, " %s", "request");
    }
    else if (p_arpace_config->arp_type == GLB_ARP_RESPONSE)
    {
        sal_fprintf(fp, " %s", "response");
    }

    /*4. ip*/
    sal_fprintf(fp, " %s", "ip");
    cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_arpace_config->src_ip);
    cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_arpace_config->src_ip_mask);
    if (0xffffffff == p_arpace_config->src_ip_mask.s_addr)
    {
        sal_fprintf (fp, " any");
    }
    else if (0 == p_arpace_config->src_ip_mask.s_addr)
    {
        sal_fprintf (fp, " host %s", buf1);
    }
    else
    {
        sal_fprintf (fp, " %s %s", buf1, buf2);
    }

    /*5. mac*/
    sal_fprintf(fp, " %s", "mac");
    cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_arpace_config->src_mac);
    cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_arpace_config->src_mac_mask);
    if (0 == sal_strcmp("ffff.ffff.ffff", buf2))
    {
        sal_fprintf (fp, " any");
    }
    else if (0 == sal_strcmp("0000.0000.0000", buf2))
    {
        sal_fprintf (fp, " host %s", buf1);
    }
    else
    {
        sal_fprintf (fp, " %s %s", buf1, buf2);
    }

    /*6. log*/
    if (p_arpace_config->arp_log)
    {
        sal_fprintf (fp, " %s", "log");
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_arpacl_config(FILE* fp, tbl_arpacl_config_t *p_arpacl_config)
{
    tbl_arpace_config_master_t *p_master_arpace = tbl_arpace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_arpace_config_t * p_arpace_config = NULL;

    sal_fprintf (fp, "arp access-list %s\n", p_arpacl_config->key.name);

    CTCLIB_SLIST_LOOP(p_master_arpace->arpace_config_list, p_arpace_config, listnode)
    {
        if ((!sal_strcmp(p_arpacl_config->key.name, p_arpace_config->key.arpacl_name)))
        {
            _ccs_cfg_build_arpace_config(fp, p_arpace_config);
            sal_fprintf (fp, "\n"); 
        }
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_arpacl(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_arpacl_config_master_t *p_master_arpacl = tbl_arpacl_config_get_master();
    tbl_arpacl_config_t *p_arpacl_config = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master_arpacl->arpacl_config_list, p_arpacl_config, listnode)
    {
        _ccs_cfg_build_arpacl_config(fp, p_arpacl_config);
        has_config = TRUE;
    }
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_get_vlan_enable_str(char* vlan_str, uint32 vlan_enable)
{
    vid_t vid = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    char vlan_tmp[16] = {'\0'};
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 has_left_vlan = FALSE;

    for (offset= GLB_DEF_VLAN_ID; offset< GLB_MAX_VLAN_ID; offset++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
        arpinsp_key.vlan_id = offset;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if (vlan_enable != p_db_arpinsp->enable_state)
        {
            continue;
        }
        vid = offset;
        end_vid = vid;
        do
        {
            end_vid ++;
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = end_vid;
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                break;
            }
        }while (vlan_enable == p_db_arpinsp->enable_state);
        
        end_vid--;
        offset = end_vid;
        if (vid == end_vid)
        {
            sal_sprintf(vlan_tmp, "%u,", vid);
            sal_strncat(vlan_str, vlan_tmp, 5);
            has_left_vlan = TRUE;
        }
        else
        {          
            sal_sprintf(vlan_tmp, "%u-%u,", vid, end_vid);
            sal_strncat(vlan_str, vlan_tmp, 10);
            has_left_vlan = TRUE;
        }   
    }

    if (has_left_vlan)
    {
        vlan_str[sal_strlen(vlan_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_get_acllog_config_str(char* vlan_str, uint32 acllog_filter)
{
    vid_t vid = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    char vlan_tmp[16] = {'\0'};
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 has_left_vlan = FALSE;

    for (offset= GLB_DEF_VLAN_ID; offset< GLB_MAX_VLAN_ID; offset++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
        arpinsp_key.vlan_id = offset;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if (acllog_filter != p_db_arpinsp->acllog_filter)
        {
            continue;
        }
        vid = offset;
        end_vid = vid;
        do
        {
            end_vid ++;
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = end_vid;
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                break;
            }
        }while (acllog_filter == p_db_arpinsp->acllog_filter);
        
        end_vid--;
        offset = end_vid;
        if (vid == end_vid)
        {
            sal_sprintf(vlan_tmp, "%u,", vid);
            sal_strncat(vlan_str, vlan_tmp, 5);
            has_left_vlan = TRUE;
        }
        else
        {          
            sal_sprintf(vlan_tmp, "%u-%u,", vid, end_vid);
            sal_strncat(vlan_str, vlan_tmp, 10);
            has_left_vlan = TRUE;
        }   
    }

    if (has_left_vlan)
    {
        vlan_str[sal_strlen(vlan_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_get_dhcplog_config_str(char* vlan_str, uint32 dhcplog_filter)
{
    vid_t vid = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    char vlan_tmp[16] = {'\0'};
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 has_left_vlan = FALSE;

    for (offset= GLB_DEF_VLAN_ID; offset< GLB_MAX_VLAN_ID; offset++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
        arpinsp_key.vlan_id = offset;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if (dhcplog_filter != p_db_arpinsp->dhcplog_filter)
        {
            continue;
        }
        vid = offset;
        end_vid = vid;
        do
        {
            end_vid ++;
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = end_vid;
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                break;
            }
        }while (dhcplog_filter == p_db_arpinsp->dhcplog_filter);
        
        end_vid--;
        offset = end_vid;
        if (vid == end_vid)
        {
            sal_sprintf(vlan_tmp, "%u,", vid);
            sal_strncat(vlan_str, vlan_tmp, 5);
            has_left_vlan = TRUE;
        }
        else
        {          
            sal_sprintf(vlan_tmp, "%u-%u,", vid, end_vid);
            sal_strncat(vlan_str, vlan_tmp, 10);
            has_left_vlan = TRUE;
        }  
    }
    
    if (has_left_vlan)
    {
        vlan_str[sal_strlen(vlan_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_arpinsp_vlan_enable(uint32 *vlan_enable, FILE* fp)
{
    char vlanstr[256] = {'\0'};
    _ccs_cfg_build_get_vlan_enable_str(vlanstr, TRUE);
    if (sal_strlen(vlanstr))
    {
        sal_fprintf(fp, "ip arp inspection vlan %s\n", vlanstr);
        *vlan_enable = TRUE;
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_arpinsp_dhcplog_config(uint32 *dhcplog_config, FILE* fp)
{
    char vlanstr1[256] = {'\0'};
    char vlanstr2[256] = {'\0'};
    char vlanstr3[256] = {'\0'};
    _ccs_cfg_build_get_dhcplog_config_str(vlanstr1, GLB_DHCP_BINDINGS_ALL);
    if (sal_strlen(vlanstr1))
    {
        sal_fprintf (fp, "ip arp inspection vlan %s logging dhcp-bindings all\n", vlanstr1);
        *dhcplog_config = TRUE;
    }

    _ccs_cfg_build_get_dhcplog_config_str(vlanstr2, GLB_DHCP_BINDINGS_NONE);
    if (sal_strlen(vlanstr2))
    {
        sal_fprintf (fp, "ip arp inspection vlan %s logging dhcp-bindings none\n", vlanstr2);
        *dhcplog_config = TRUE;
    }

    _ccs_cfg_build_get_dhcplog_config_str(vlanstr3, GLB_DHCP_BINDINGS_PERMIT);
    if (sal_strlen(vlanstr3))
    {
        sal_fprintf (fp, "ip arp inspection vlan %s logging dhcp-bindings permit\n", vlanstr3);
        *dhcplog_config = TRUE;
    }    
    return PM_E_NONE;
}   

int32
ccs_cfg_build_arpinsp_acllog_config(uint32 *acllog_config, FILE* fp)
{
    char vlanstr1[256] = {'\0'};
    char vlanstr2[256] = {'\0'};
    _ccs_cfg_build_get_acllog_config_str(vlanstr1, GLB_ACL_MATCH_MATCHLOG);
    if (sal_strlen(vlanstr1))
    {
        sal_fprintf (fp, "ip arp inspection vlan %s logging acl-match matchlog\n", vlanstr1);
        *acllog_config = TRUE;
    }

    _ccs_cfg_build_get_acllog_config_str(vlanstr2, GLB_ACL_MATCH_NONE);
    if (sal_strlen(vlanstr2))
    {
        sal_fprintf (fp, "ip arp inspection vlan %s logging acl-match none\n", vlanstr2);
        *acllog_config = TRUE;
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_arpinsp_logbuffer_config(uint32 *logbuf_config, FILE* fp)
{
    tbl_route_global_t* p_rt_glb = tbl_route_global_get_route_global();
    
    if (NULL == p_rt_glb)
    {
        return PM_E_NONE;
    }
    if (GLB_ARPINSP_LOGBUF_DEFAULT_NUM != p_rt_glb->arpinsp_logbuf_entrynum)
    {
        sal_fprintf(fp, "ip arp inspection log-buffer entries %d \n", p_rt_glb->arpinsp_logbuf_entrynum);
        *logbuf_config = TRUE;
    }
    if (GLB_ARPINSP_LOGLIST_DEFAULT_NUM != p_rt_glb->arpinsp_logbuf_lognum || 
        GLB_ARPINSP_LOGLIST_DEFAULT_INTERVAL != p_rt_glb->arpinsp_logbuf_logsec)
    {
        sal_fprintf(fp, "ip arp inspection log-buffer logs %d interval %d \n", 
            p_rt_glb->arpinsp_logbuf_lognum, p_rt_glb->arpinsp_logbuf_logsec);
        *logbuf_config = TRUE;
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_arpinsp_validate_config(uint32 *validate_config, FILE* fp)
{
    char validate_str[30] = {'\0'};
    tbl_route_global_t* p_rt_glb = tbl_route_global_get_route_global();
    if (NULL == p_rt_glb)
    {
        return PM_E_NONE;
    }
    if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_SRCMAC))
    {
        sal_strcat(validate_str, " src-mac");
    }
    if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_DESTMAC))
    {
        sal_strcat(validate_str, " dst-mac");
    }
    if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_IP))
    {
        sal_strcat(validate_str, " ip");
    }
    if (sal_strlen(validate_str))
    {
        sal_fprintf(fp, "ip arp inspection validate%s \n", validate_str);
        *validate_config = TRUE;
    }
    return PM_E_NONE;
}

static int32
ccs_cfg_build_arpinsp_acl_item(tbl_arpinsp_t * p_arpinsp, uint32 *vlan_array, FILE* fp)
{
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 i = 0;
    vid_t vid = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    uint32 state = 0;
    char tmp[16];
    char vlan_str[256]= {'\0'};
    char filter_str[ARPINSP_FILTER_MAX_NUM+1];
    uint32 has_left_vlan = FALSE;
    vlan_bmp_t bmp;
    
    sal_memset(bmp, 0, sizeof(bmp));
    sal_strncpy(filter_str, p_arpinsp->filter, ARPINSP_FILTER_MAX_NUM+1);
    state = p_arpinsp->filter_state;

    for (i = GLB_DEF_VLAN_ID; i< GLB_MAX_VLAN_ID; i++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(arpinsp_key));
        arpinsp_key.vlan_id = i;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if (!sal_strcmp(filter_str, p_db_arpinsp->filter) && (state == p_db_arpinsp->filter_state))
        {
            vlan_array[i] = 1;
            GLB_BMP_SET(bmp, i);
        }
    }
    
    for (offset = GLB_DEF_VLAN_ID; offset < GLB_BMP_BITS(bmp); offset++)
    {
        if (GLB_BMP_ISSET(bmp, offset))
        {
            vid = offset;
            end_vid = vid;
            do 
            {
                end_vid++;
            } while (GLB_BMP_ISSET(bmp, end_vid));
            end_vid--;
            offset = end_vid;
            if (vid == end_vid)
            {
                sal_sprintf(tmp, "%u,", vid);
                sal_strncat(vlan_str, tmp, 5);
                has_left_vlan = TRUE;
            }                
            else
            {          
                sal_sprintf(tmp, "%u-%u,", vid, end_vid);
                sal_strncat(vlan_str, tmp, 10);
                has_left_vlan = TRUE;
            }                             
        }
    }
    
    if (has_left_vlan)
    {
        vlan_str[sal_strlen(vlan_str)-1] = CMD_ZERO_CHAR;    
    }
    if (GLB_ARPINSP_ACL_STATIC == state)
    {
        sal_fprintf(fp, "ip arp inspection filter %s vlan %s static \n", filter_str, vlan_str);
    }
    else if (GLB_ARPINSP_ACL_DYNAMIC == state)
    {
        sal_fprintf(fp, "ip arp inspection filter %s vlan %s \n", filter_str, vlan_str);
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_arpinsp_filter_config (uint32 *filter_config, FILE* fp)
{
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 vlan_array[GLB_MAX_VLAN_ID] = {0};
    uint32 i = 0;

    for (i = GLB_DEF_VLAN_ID; i< GLB_MAX_VLAN_ID; i++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
        arpinsp_key.vlan_id = i;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if ((0 != sal_strlen(p_db_arpinsp->filter)) && (0 == vlan_array[i]))
        {
            ccs_cfg_build_arpinsp_acl_item(p_db_arpinsp, vlan_array, fp);
            *filter_config = TRUE;
        }
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_arpinsp(FILE* fp, cfg_cmd_para_t *para)
{   
    uint32 vlan_enable = FALSE;
    uint32 dhcplog_config = FALSE;
    uint32 acllog_config = FALSE;
    uint32 logbug_config = FALSE;
    uint32 validate_config = FALSE;
    uint32 filter_config = FALSE;

    ccs_cfg_build_arpinsp_vlan_enable(&vlan_enable,fp);
    ccs_cfg_build_arpinsp_dhcplog_config(&dhcplog_config, fp);
    ccs_cfg_build_arpinsp_acllog_config(&acllog_config, fp);
    ccs_cfg_build_arpinsp_logbuffer_config(&logbug_config, fp);
    ccs_cfg_build_arpinsp_validate_config(&validate_config, fp);
    ccs_cfg_build_arpinsp_filter_config(&filter_config, fp);
    if (vlan_enable || dhcplog_config || acllog_config || logbug_config || validate_config || filter_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

