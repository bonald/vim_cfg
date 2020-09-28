#include "proto.h"

#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"
#include "gen/tbl_oam_flow_info_define.h"
#include "gen/tbl_oam_flow_info.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"

#include "lib_fs.h"
#include "tpoam_list.h"

#include "tpoam_sock.h"

#include "tpoam_pdu.h"
#include "tpoam_lm.h"
#include "tpoam_event.h"
#include "tpoam_dm.h"
#include "tpoam_error.h"
#include "tpoam_api.h"
#include "tpoam_debug.h"

#include "glb_openflow_define.h"
#include "fei_openflow_tpoam.h"

char*
tpoam_ifindex2namefull(char *name_str, uint32 ifindex)
{
    int32 rc = 0;
    char ifname[IFNAME_SIZE] = {0};
    
    rc = tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
    if (rc)
    {
         return NULL;
    }
    IFNAME_ETH2FULL(ifname, name_str);
    
    return name_str;
}

ctc_task_t *
tpoam_util_add_timer_ms(void (*func) (void *), void *arg, int32 ms)
{
    return ctc_task_add_timer_msec(TASK_PRI_NORMAL, func, arg, ms);
}

void
tpoam_util_cancel_timer(ctc_task_t **ppt)
{
    if (ppt && *ppt)
    {
        ctc_task_delete_timer(*ppt);
        *ppt = NULL;
    }
}

char*
tpoam_api_exec_type_desc(uint32 exec_type)
{
    switch (exec_type)
    {
    case TPOAM_EXEC_TYPE_NONE:
        return "None";
    case TPOAM_EXEC_TYPE_LB_DISCOVERY:
        return "Loopback discovery";
    case TPOAM_EXEC_TYPE_LB_MIP:
        return "Loopback MIP";
    case TPOAM_EXEC_TYPE_LB_MEP:
        return "Loopback MEP";
    case TPOAM_EXEC_TYPE_DM:
        return "DMM";
    case TPOAM_EXEC_TYPE_LMM:
        return "LMM";
    default:
        return "Invalid";
    }
}

char*
tpoam_api_pkt_type_desc(uint32 pkt_type)
{
    switch (pkt_type)
    {
    case TPOAM_Y1731_PDU_LBR:
        return "LBR";
    case TPOAM_Y1731_PDU_LBM:
        return "LBM";
    case TPOAM_Y1731_PDU_APS:
        return "APS";
    case TPOAM_Y1731_PDU_DMR:
        return "DMR";
    case TPOAM_Y1731_PDU_DMM:
        return "DMM";
    default:
        return "Invalid";
    }
}

int32
tpoam_api_exec_deinit(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    /*for show mpls-tp oam-y1731 loobback*/
    sal_memset(&p_tpoam_exec_global->exec_lb_discovery.para, 0, sizeof(p_tpoam_exec_global->exec_lb_discovery.para));
    sal_memset(&p_tpoam_exec_global->exec_lb_mip.para, 0, sizeof(p_tpoam_exec_global->exec_lb_mip.para));
    sal_memset(&p_tpoam_exec_global->exec_lb_rmep.para, 0, sizeof(p_tpoam_exec_global->exec_lb_rmep.para));

    return PM_E_NONE;
}

int32
tpoam_api_build_megid(char* pch_megstr, uint8* pch_megid)
{
    pch_megid[0] = TPOAM_MEGID_RESERVED;
    pch_megid[1] = TPOAM_MEGID_FORMAT;
    pch_megid[2] = GLB_TPOAM_MEGID_STR_LEN;
    sal_memcpy(&pch_megid[3], pch_megstr, GLB_TPOAM_MEGID_STR_LEN);

    return TPOAM_E_NONE;
}

int32
tpoam_api_stats_add_tx(tpoam_stats_tx_item_t* pst_tx_stats, uint8 opcode)
{
    switch (opcode)
    {
    case TPOAM_Y1731_PDU_LBM:
        pst_tx_stats->lbm++;
        break;

    case TPOAM_Y1731_PDU_LBR:
        pst_tx_stats->lbr++;
        break;

    case TPOAM_Y1731_PDU_AIS:
        pst_tx_stats->ais++;
        break;

    case TPOAM_Y1731_PDU_1DM:
        pst_tx_stats->onedm++;
        break;

    case TPOAM_Y1731_PDU_DMR:
        pst_tx_stats->dmr++;
        break;

    case TPOAM_Y1731_PDU_DMM:
        pst_tx_stats->dmm++;
        break;

    case TPOAM_Y1731_PDU_LCK:
        pst_tx_stats->lck++;
        break;

    case TPOAM_Y1731_PDU_CSF:
        pst_tx_stats->csf++;
        break;

    /*Modified by yejl for bug 42274, 2018-05-17*/
    case TPOAM_Y1731_PDU_APS:
        pst_tx_stats->aps++;
        break;
    /*end*/

    default:
        break;
    }
    return TPOAM_E_NONE;
}

int32
tpoam_api_stats_add_rx(tpoam_stats_rx_item_t* pst_rx_stats, uint8 opcode)
{
    switch (opcode)
    {
    case TPOAM_Y1731_PDU_LBM:
        pst_rx_stats->lbm++;
        break;

    case TPOAM_Y1731_PDU_LBR:
        pst_rx_stats->lbr++;
        break;

    case TPOAM_Y1731_PDU_AIS:
        pst_rx_stats->ais++;
        break;

    case TPOAM_Y1731_PDU_1DM:
        pst_rx_stats->onedm++;
        break;

    case TPOAM_Y1731_PDU_DMR:
        pst_rx_stats->dmr++;
        break;

    case TPOAM_Y1731_PDU_DMM:
        pst_rx_stats->dmm++;
        break;
        
    /*Modified by yejl for bug 42274, 2018-05-17*/
    case TPOAM_Y1731_PDU_APS:
        pst_rx_stats->aps++;
        break;
    /*end*/

    case TPOAM_Y1731_PDU_CCM:
        pst_rx_stats->unexp_ccm++;
        break;

    case TPOAM_Y1731_PDU_LCK:
        pst_rx_stats->lck++;
        break;

    case TPOAM_Y1731_PDU_CSF:
        pst_rx_stats->csf++;
        break;

    default:
        pst_rx_stats->unknown++;
    }
    return TPOAM_E_NONE;
}

int32
tpoam_api_stats_add_err(tpoam_stats_err_item_t* pst_err_stats, uint8 opcode)
{
    switch (opcode)
    {
    case TPOAM_Y1731_PDU_LBM:
        pst_err_stats->lbm++;
        break;

    case TPOAM_Y1731_PDU_LBR:
        pst_err_stats->lbr++;
        break;

    case TPOAM_Y1731_PDU_AIS:
        pst_err_stats->ais++;
        break;

    case TPOAM_Y1731_PDU_1DM:
        pst_err_stats->onedm++;
        break;

    case TPOAM_Y1731_PDU_DMR:
        pst_err_stats->dmr++;
        break;
        
    case TPOAM_Y1731_PDU_DMM:
        pst_err_stats->dmm++;
        break;

    case TPOAM_Y1731_PDU_LCK:
        pst_err_stats->lck++;
        break;

    case TPOAM_Y1731_PDU_CSF:
        pst_err_stats->csf++;
        break;

    /*Modified by yejl for bug 42274, 2018-05-17*/
    case TPOAM_Y1731_PDU_APS:
        pst_err_stats->aps++;
        break;
    /*end*/

    case TPOAM_Y1731_PDU_CCM:
        pst_err_stats->unexp_ccm++;

    default:
        pst_err_stats->unknown++;
    }
    return TPOAM_E_NONE;
}



int32
tpoam_api_dm_interval_to_ms(uint8 interval, int32* p_ms)
{
    switch (interval)
    {
    case TPOAM_DM_INTERVAL_100MS:
        *p_ms = 100;
        break;
    case TPOAM_DM_INTERVAL_1S:
        *p_ms = 1000;
        break;
    case TPOAM_DM_INTERVAL_10S:
        *p_ms = 10000;
        break;
    default:
        return TPOAM_E_DM_INVALID_INTERVAL;
    }
    
    return TPOAM_E_NONE;    
}

int32
tpoam_api_lm_interval_to_ms(uint8 interval, int32* p_ms)
{
    switch (interval)
    {
    case TPOAM_LM_INTERVAL_100MS:
        *p_ms = 100;
        break;
    case TPOAM_LM_INTERVAL_1S:
        *p_ms = 1000;
        break;
    case TPOAM_LM_INTERVAL_10S:
        *p_ms = 10000;
        break;
    default:
        return TPOAM_E_DM_INVALID_INTERVAL;
    }
    
    return TPOAM_E_NONE;    
}


int32
tpoam_api_lookup_oam_by_megid(char* pch_megid, tbl_oam_session_t** ppst_oam)
{
    tbl_oam_session_master_t *p_master = tbl_oam_session_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_oam_session_t *p_db_oam_session = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->oam_session_list, p_db_oam_session, listnode, next)
    {
        if (0 == sal_strcmp(p_db_oam_session->megid_str, pch_megid))
        {
            *ppst_oam = p_db_oam_session;
            return TPOAM_E_NONE;
        }
    }
    
    return TPOAM_E_FAIL;
}

#ifndef tbl_get
tbl_oam_session_t*
tbl_oam_session_get_oam_session_by_session_id(uint32 session_id)
{
    tbl_oam_session_key_t oam_session_key;

    if (0 != session_id)
    {
        oam_session_key.session_id = session_id;
        return tbl_oam_session_get_oam_session(&oam_session_key);
    }
    else
    {
        return NULL;
    }
}

tbl_oam_session_upload_t*
tbl_oam_session_upload_get_oam_session_upload_by_session_id(uint32 session_id)
{
    tbl_oam_session_upload_key_t oam_session_upload_key;

    if (0 != session_id)
    {
        oam_session_upload_key.session_id = session_id;
        return tbl_oam_session_upload_get_oam_session_upload(&oam_session_upload_key);
    }
    else
    {
        return NULL;
    }
}

tbl_section_oam_t*
tbl_section_oam_get_section_oam_by_ifindex(uint32 ifindex)
{
    tbl_section_oam_key_t section_oam_key;

    section_oam_key.ifindex = ifindex;
    return tbl_section_oam_get_section_oam(&section_oam_key);
}

tbl_lsp_pe_t*
tbl_lsp_pe_get_lsp_pe_by_name(char * name)
{
    tbl_lsp_pe_key_t lsp_pe_key;

    sal_memset(&lsp_pe_key, 0x00, sizeof(lsp_pe_key));
    sal_strcpy(lsp_pe_key.name, name);
    return tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
}

tbl_lsp_p_t*
tbl_lsp_p_get_lsp_p_by_name(char * name)
{
    tbl_lsp_p_key_t lsp_p_key;

    sal_memset(&lsp_p_key, 0x00, sizeof(lsp_p_key));
    sal_strcpy(lsp_p_key.name, name);
    return tbl_lsp_p_get_lsp_p(&lsp_p_key);
}

tbl_pw_tpe_t*
tbl_pw_tpe_get_pw_tpe_by_name(char * name)
{
    tbl_pw_tpe_key_t pw_tpe_key;

    sal_memset(&pw_tpe_key, 0x00, sizeof(pw_tpe_key));
    sal_strcpy(pw_tpe_key.name, name);
    return tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
}
#endif

#ifndef common_api
int32
tpoam_get_prefix_item (char** argv , uint8 argc, char* prefix, uint8 prefix_len)
{
    uint8 index = 0;
    while (index < argc)
    {
        if (!sal_strncmp(argv[index], prefix, prefix_len))
        {
            return (index + 1);
        }
        index++;
    }
    
    return 0;
}
int32
_tpoam_get_event_rule_value(uint32 argc, char** argv, uint32 para_index_base, uint32* p_evt_rule)
{
    uint32 arg_index = 0;

    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "loc", 3);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_LOC);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "unexp-rmepi", 11);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_UNEXP_MEP);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "unexp-inter", 11);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_UNEXP_PERIOD);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "unexp-megid", 11);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_MISMERGE);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "unexp-meg-l", 11);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_MEG_LVL);
    }
    
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "rdi-rx", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_RDI_RX);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "rdi-tx", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_RDI_TX);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "wait-1", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_WAIT_1ST_CC);
    }
    #ifdef support_many_event
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "ais-rx", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_AIS_RX);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "ais-tx", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_AIS_TX);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "ais-de", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_AIS_DEFECT);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "lck-rx", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_LCK_RX);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "csf-de", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_CSF_DEFECT);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "lck-tx", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_LCK_TX);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "lck-de", 6);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_LCK_DEFECT);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "event", 5);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_EVT_CACHE_FULL);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "1dm", 3);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_1DM_CACHE_FULL);
    }
    #endif
    /* modified by yaom for CRC trigger AIS 20140919 */
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "crc", 3);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_evt_rule, GLB_TPOAM_EVT_CRC_DEFECT);
    }
    return PM_E_NONE;
}


char *
tpoam_api_get_ifname_ext_by_ifindex(uint32 ifindex)
{
    int32 rc = 0;
    char if_name[IFNAME_SIZE] = {0};
    static char if_name_ext[IFNAME_EXT_SIZE] = {0};

    rc = tbl_interface_get_name_by_ifindex(ifindex, if_name, IFNAME_SIZE);
    if (0 == rc)
    {
        IFNAME_ETH2FULL(if_name, if_name_ext);
    }

    return if_name_ext;
}

int32
tpoam_api_get_service_name(tbl_oam_session_t* pst_oam, char* pch_name, uint32 size)
{
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;

    sal_memset(pch_name, 0x00, size);
    switch (pst_oam->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        sal_strncpy(pch_name, tpoam_api_get_ifname_ext_by_ifindex(pst_oam->service_ifindex), size);
        break;

    case GLB_OAM_TYPE_MPLS_LSP:
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_oam->service_name);
        if (NULL != pst_lsp_pe)
        {
            sal_strncpy(pch_name, pst_lsp_pe->key.name, size); 
        }
        else
        {
            pst_lsp_p = tbl_lsp_p_get_lsp_p_by_name(pst_oam->service_name);
            if (NULL != pst_lsp_p)
            {
                sal_strncpy(pch_name, pst_lsp_p->key.name, size); 
            }
        }
        break;
        
    case GLB_OAM_TYPE_MPLS_PW:
        switch (pst_oam->pw_type)
        {
        case TPOAM_PW_TYPE_TPE:
            pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(pst_oam->service_name);
            if (NULL != pst_tpe)
            {
                sal_strncpy(pch_name, pst_tpe->key.name, size); 
            }
            break;
        default:
            return TPOAM_E_INVALID_TYPE;
        }        
        break;

    default:
        return TPOAM_E_INVALID_TYPE;
    }

    return PM_E_NONE;
}


int32
tpoam_api_get_lsp_p(char* pch_lsp_name, tbl_lsp_p_t** ppst_lsp_p)
{
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_lsp_p_key_t lsp_p_key;

    *ppst_lsp_p = NULL;

    sal_memset(&lsp_p_key, 0x00, sizeof(lsp_p_key));
    sal_strcpy(lsp_p_key.name, pch_lsp_name);
    pst_lsp_p = tbl_lsp_p_get_lsp_p(&lsp_p_key);
    if (NULL == pst_lsp_p)
    {
        return TPOAM_E_LSP_NOT_EXIST;
    }

    *ppst_lsp_p = pst_lsp_p;
    return PM_E_NONE;
}

int32
tpoam_api_get_lsp_pe(char* pch_lsp_name, tbl_lsp_pe_t** ppst_lsp_pe)
{
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_pe_key_t lsp_pe_key;

    *ppst_lsp_pe = NULL;

    sal_memset(&lsp_pe_key, 0x00, sizeof(lsp_pe_key));
    sal_strcpy(lsp_pe_key.name, pch_lsp_name);
    pst_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
    if (NULL == pst_lsp_pe)
    {
        return TPOAM_E_LSP_NOT_EXIST;
    }

    *ppst_lsp_pe = pst_lsp_pe;
    return PM_E_NONE;
}

int32
tpoam_api_get_pw_tpe(char* pch_tpe_name, tbl_pw_tpe_t** ppst_tpe)
{
    tbl_pw_tpe_t* pst_tpe = NULL;
    tbl_pw_tpe_key_t pw_tpe_key;

    *ppst_tpe = NULL;

    sal_memset(&pw_tpe_key, 0x00, sizeof(pw_tpe_key));
    sal_strcpy(pw_tpe_key.name, pch_tpe_name);
    pst_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
    if (NULL == pst_tpe)
    {
        return TPOAM_E_PW_NOT_EXIST;
    }

    *ppst_tpe = pst_tpe;
    return PM_E_NONE;
}

int32
tpoam_api_get_section_new(uint32 ifindex, tbl_section_oam_t** ppst_section)
{
    tbl_section_oam_key_t section_key;
    tbl_section_oam_t* pst_section = NULL;

    *ppst_section = NULL;
    //TPOAM_SECTION_FORMAT_CHECK(pch_section_name);

    section_key.ifindex = ifindex;
    pst_section = tbl_section_oam_get_section_oam(&section_key);
    if (NULL == pst_section)
    {
        return TPOAM_E_SECTION_NOT_EXIST;
    }

    *ppst_section = pst_section;
    return PM_E_NONE;
}

// TODO: cc
int32
tpoam_api_get_section(char* pch_tpe_name, tbl_section_oam_t** ppst_section)
{
    return TPOAM_E_SECTION_NOT_EXIST;
    #if 0
    tbl_section_oam_key_t section_key;
    tbl_section_oam_t* pst_section = NULL;

    *ppst_section = NULL;
    //TPOAM_SECTION_FORMAT_CHECK(pch_section_name);

    section_key.ifindex = ifindex;
    pst_section = tbl_section_oam_get_section_oam(&section_key);
    if (NULL == pst_section)
    {
        return TPOAM_E_SECTION_NOT_EXIST;
    }

    *ppst_section = pst_section;
    return PM_E_NONE;
    #endif
}

#endif

#ifndef cmd_set_glb

int32
tpoam_api_set_fast_aps_enable(uint32 fast_aps_en)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->fast_aps_en = fast_aps_en;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN);

    return rc;
}

int32
tpoam_api_set_dm_global_enable(uint32 enable)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->dm_enable_global = enable;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL);

    return rc;
}

int32
tpoam_api_set_lm_global_enable(uint32 enable)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->lm_enable_global = enable;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL);

    return rc;
}

int32
tpoam_api_set_dm_calc_interval(uint32 interval)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->dm_calc_interval = interval;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL);

    return rc;
}


int32
tpoam_api_set_dm_delay_threshold(uint32 threshold)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();

    tbl_oam_session_upload_t * pst_oam_upload = NULL;
    tbl_oam_session_master_t *p_master = tbl_oam_session_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_oam_session_t* pst_oam_session = NULL;
    
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->dm_delay_threshold = threshold;
    /*sync to fea*/
    PM_E_RETURN(tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD));

    /*added by yejl for bug 52508, 2019-06-24*/
    if (threshold == 0)
    {
        CTCLIB_SLIST_LOOP(p_master->oam_session_list, pst_oam_session, listnode)
        {
            tpoam_dm_t* pst_oam_dm = &pst_oam_session->oam_dm;
            if (pst_oam_dm->is_event_occur)
            {
                pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam_session->key));
                if (pst_oam_upload)
                {
                    if (pst_oam_dm->delay_threshold == 0)
                    {
                        _tpoam_dm_event_clear(pst_oam_session, pst_oam_upload);    
                    }
                    else if (pst_oam_dm->delay_threshold > pst_oam_upload->upload_dm_event.delay)
                    {
                        _tpoam_dm_event_clear(pst_oam_session, pst_oam_upload);
                    }
                }
            }
        }
    }
    /*ended by yejl*/
    return rc;
}

int32
tpoam_api_set_lm_local_threshold(uint32 threshold)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->lm_local_threshold = threshold;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD);

    return rc;
}

int32
tpoam_api_set_lm_remote_threshold(uint32 threshold)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->lm_remote_threshold = threshold;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD);

    return rc;
}

int32
tpoam_api_set_lm_calc_interval(uint32 interval)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->lm_calc_interval = interval;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL);

    return rc;
}

int32
tpoam_api_set_node_id(uint32 node_id)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->node_id = node_id;
    
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_NODE_ID);

    return rc;
}


int32
tpoam_api_set_channel_type(uint32 channel_type)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->channel_type = channel_type;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE);

    return rc;
}

#define TPOAM_REASON_STR_LEN     256


static char*
_tpoam_api_rdi_reason_bmp2str(uint8 rdi_reason, char* pch_str, uint32 length)
{
    sal_memset(pch_str, 0, sizeof(length));

    if (TPOAM_RDI_RSN_NONE == rdi_reason)
    {
        sal_strcat(pch_str, "none ");
    }
    else if (TPOAM_RDI_RSN_ALL == rdi_reason)
    {
        sal_strcat(pch_str, "all ");
    }        
    else
    {
        if (GLB_FLAG_ISSET(rdi_reason, GLB_TPOAM_RDI_RSN_LOC))
        {
            sal_strcat(pch_str, "loc ");
        }
        if (GLB_FLAG_ISSET(rdi_reason, GLB_TPOAM_RDI_RSN_CC_ERR))
        {
            sal_strcat(pch_str, "cc-error ");
        }
        if (GLB_FLAG_ISSET(rdi_reason, GLB_TPOAM_RDI_RSN_CC_MISMERGE))
        {
            sal_strcat(pch_str, "cc-mismerge ");
        }
    }

    return pch_str;
}


int32
tpoam_api_set_rdi_reason(uint32 rdi_reason, uint32 to_hal)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;
    char cfg_str1[TPOAM_REASON_STR_LEN];
    char cfg_str2[TPOAM_REASON_STR_LEN];
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();

    /* Added by kcao 2012-04-09 for bug 18795 : log only after system startup done */
    if (TRUE == p_sys_glb->startup_done)
    {
        TPOAM_LOG_USER(E_WARNING, TPOAM_4_CHANGE_RDI_REASONS, 
            _tpoam_api_rdi_reason_bmp2str(p_db_tpoam_glb->rdi_reason, cfg_str1, TPOAM_REASON_STR_LEN),
            _tpoam_api_rdi_reason_bmp2str(rdi_reason, cfg_str2, TPOAM_REASON_STR_LEN));
    }
    
    p_db_tpoam_glb->rdi_reason = rdi_reason;

    /* KCAO TODO maybe need to update all service's GLB_TPOAM_DEFECT_FLAG_TX_RDI */
    p_db_tpoam_glb->rdi_reason_mask = 0;
    if (GLB_FLAG_ISSET(rdi_reason, GLB_TPOAM_RDI_RSN_LOC))
    {
        GLB_SET_FLAG(p_db_tpoam_glb->rdi_reason_mask, GLB_TPOAM_EVT_LOC);
    }
    if (GLB_FLAG_ISSET(rdi_reason, GLB_TPOAM_RDI_RSN_CC_ERR))
    {
        GLB_SET_FLAG(p_db_tpoam_glb->rdi_reason_mask, GLB_TPOAM_EVT_UNEXP_MEP);
        GLB_SET_FLAG(p_db_tpoam_glb->rdi_reason_mask, GLB_TPOAM_EVT_UNEXP_PERIOD);
    }
    if (GLB_FLAG_ISSET(rdi_reason, GLB_TPOAM_RDI_RSN_CC_MISMERGE))
    {
        GLB_SET_FLAG(p_db_tpoam_glb->rdi_reason_mask, GLB_TPOAM_EVT_MISMERGE);
        GLB_SET_FLAG(p_db_tpoam_glb->rdi_reason_mask, GLB_TPOAM_EVT_MEG_LVL);
    }

    if (to_hal)
    {
        /*sync to fea*/
        rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_RDI_REASON);
    }
    return rc;
}

int32
tpoam_api_set_sf_reason(uint32 sf_reason_mask)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;
    char cfg_str1[TPOAM_REASON_STR_LEN];
    char cfg_str2[TPOAM_REASON_STR_LEN];
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();

    /* Added by kcao 2012-04-09 for bug 18795 : log only after system startup done */
    if (TRUE == p_sys_glb->startup_done)
    {
        TPOAM_LOG_USER(E_WARNING, TPOAM_4_CHANGE_SF_REASONS, 
            _tpoam_api_rdi_reason_bmp2str(p_db_tpoam_glb->rdi_reason, cfg_str1, TPOAM_REASON_STR_LEN),
            _tpoam_api_rdi_reason_bmp2str(sf_reason_mask, cfg_str2, TPOAM_REASON_STR_LEN));
    }
    
    p_db_tpoam_glb->sf_reason_mask = sf_reason_mask;

    /*sync to fea*/
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK);

    return rc;
}

int32
tpoam_api_set_sd_reason(uint32 sd_reason_mask)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;

    p_db_tpoam_glb->sd_reason_mask = sd_reason_mask;
    
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK);

    return rc;
}

int32
tpoam_api_set_event_rule_map(uint32 event_rule_map, tpoam_evt_dest_t dest)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    int32 rc = PM_E_NONE;
    
    p_db_tpoam_glb->event_rule_map[dest] = event_rule_map;
    
    rc = tbl_tpoam_global_set_tpoam_global_field(p_db_tpoam_glb, TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP);

    return rc;
}
#endif

#ifndef str_desc
char*
tpoam_api_service_type_for_display_desc(uint32 oam_type, tpoam_pw_type_t pw_type, uint32 lsp_type)
{
    switch (oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        return "Section";
    case GLB_OAM_TYPE_MPLS_LSP:
        switch (lsp_type)
        {
        case 0:
            return "LSP-PE";
        case 1:
            return "LSP-P";
        default:
            return "LSP";
        }
    case GLB_OAM_TYPE_MPLS_PW:
        switch (pw_type)
        {
        case TPOAM_PW_TYPE_SPE:
            return "PW-SPE";
        case TPOAM_PW_TYPE_TPE:
            return "PW-TPE";            
        default:
            return "PW";
        }
    default:
        return "UNBIND";
    }
}

char*
tpoam_api_lb_testtype_desc(uint32 type)
{
    switch (type)
    {
    case TPOAM_EXEC_TYPE_LB_DISCOVERY:
        return "Discovery";
    case TPOAM_EXEC_TYPE_LB_MEP:
        return "MEP";
    case TPOAM_EXEC_TYPE_LB_MIP:
        return "MIP";
    default:
        return "None";
    }
}

char*
tpoam_api_lb_subtype_desc(uint32 type)
{
    switch (type)
    {
    case TPOAM_LB_DISCOVERY:
        return "Discovery";
    case TPOAM_LB_MEP:
        return "MEP";
    case TPOAM_LB_MIP:
        return "MIP";
    default:
        return "Invalid";
    }
}



char*
tpoam_api_test_tlv_mode_desc(uint8 test_tlv_mode)
{
    switch (test_tlv_mode)
    {
    case TPOAM_TEST_MODE_0_NULL_WO_CRC:
        return "NULL without CRC32";
    case TPOAM_TEST_MODE_1_NULL_WI_CRC:
        return "NULL with CRC32";
    case TPOAM_TEST_MODE_2_PRBS_WO_CRC:
        return "PRBS without CRC32";
    case TPOAM_TEST_MODE_3_PRBS_WI_CRC:
        return "PRBS with CRC32";

    default:
        return "Invalid";
    }
}


char*
tpoam_api_event_brief_desc(glb_tpoam_evt_t event)
{
    switch (event)
    {
    case GLB_TPOAM_EVT_LOC:
        return "LOC";
    case GLB_TPOAM_EVT_UNEXP_MEP:
        return "UMP";
    case GLB_TPOAM_EVT_UNEXP_PERIOD:
        return "UIN";
    case GLB_TPOAM_EVT_MISMERGE:
        return "UMG";
    case GLB_TPOAM_EVT_MEG_LVL:
        return "ULV";
    case GLB_TPOAM_EVT_RDI_RX:
        return "RDI R";
    case GLB_TPOAM_EVT_RDI_TX:
        return "RDI T";
    case GLB_TPOAM_EVT_WAIT_1ST_CC:
        return "W1ST";
    #ifdef support_many_event
    case GLB_TPOAM_EVT_AIS_RX:
        return "AIS R";
    case GLB_TPOAM_EVT_AIS_TX:
        return "AIS T";
    case GLB_TPOAM_EVT_AIS_DEFECT:
        return "AIS D";
    case GLB_TPOAM_EVT_LCK_RX:
        return "LCK R";
    case GLB_TPOAM_EVT_LCK_TX:
        return "LCK T";
    case GLB_TPOAM_EVT_LCK_DEFECT:
        return "LCK D";
    case GLB_TPOAM_EVT_CSF_DEFECT:
        return "CSF D";
    #endif
    /* modified by yaom for CRC trigger AIS 20140919 */        
    case GLB_TPOAM_EVT_CRC_DEFECT:
        return "CRC";
    default:
        return "Invalid";
    }
}

char*
tpoam_api_status_brief_desc(uint8 status)
{
    switch (status)
    {
    case FALSE:
        return "N";
    default:
        return "Y";
    }
}

char*
tpoam_api_dm_type_desc(uint8 type)
{
    switch (type)
    {
    case TPOAM_DM_TYPE_1DM:
        return "1DM";
    case TPOAM_DM_TYPE_DMM:
        return "DMM";
    default:
        return "Invalid";
    }
}

char*
tpoam_api_dm_interval_desc(uint8 interval)
{
    switch (interval)
    {
    case TPOAM_DM_INTERVAL_100MS:
        return "100ms";
    case TPOAM_DM_INTERVAL_1S:
        return "1s";
    case TPOAM_DM_INTERVAL_10S:
        return "10s";
    default:
        return "Invalid";
    }
}

char*
tpoam_api_mp_type_desc(tpoam_mp_type_t type)
{
    switch (type)
    {
    case TPOAM_MP_TYPE_MEP:
        return "MEP";
    case TPOAM_MP_TYPE_MIP:
        return "MIP";
    default:
        return "Error";
    }
}

char*
tpoam_api_en_desc(uint8 is_short, uint8 enable)
{
    switch (enable)
    {
    case FALSE:
        if (is_short)
        {
            return "Dis";
        }
        else
        {
            return "Disable";
        }
    default:
        if (is_short)
        {
            return "En";
        }
        else
        {
            return "Enable";
        }
    }
}

char*
tpoam_api_cc_interval_desc(uint8 interval)
{
    switch (interval)
    {
    case 1:
        return "3.3ms";
    case 2:
        return "10ms";
    case 3:
        return "100ms";
    case 4:
        return "1s";
    case  5 :
        return "10s";
    case  6 :
        return "60s";
    case  7 :
        return "600s";
    default:
        return "Invalid";
    }
}

char*
tpoam_api_event_action_desc(uint32 event_action)
{
    switch (event_action)
    {
    case 1:
        return "SET";
    case 2:
        return "CLEAR";    
    default:
        return "Invalid";
    }
}

char*
tpoam_api_status_desc(uint8 status)
{
    switch (status)
    {
    case FALSE:
        return "No";
    default:
        return "Yes";
    }
}


char*
tpoam_api_event_desc(glb_tpoam_evt_t event)
{
    switch (event)
    {
    case GLB_TPOAM_EVT_NONE:
        return "None";
    case GLB_TPOAM_EVT_LOC:
        return "LOC";
    case GLB_TPOAM_EVT_UNEXP_MEP:
        return "Unexp RMEPID";
    case GLB_TPOAM_EVT_UNEXP_PERIOD:
        return "Unexp Interval";
    case GLB_TPOAM_EVT_MISMERGE:
        return "Unexp MEGID";
    case GLB_TPOAM_EVT_MEG_LVL:
        return "Unexp Level";
    case GLB_TPOAM_EVT_RDI_RX:
        return "RDI RX";
    case GLB_TPOAM_EVT_RDI_TX:
        return "RDI TX";
    #ifdef support_many_event
    case GLB_TPOAM_EVT_AIS_RX:
        return "AIS RX";
    case GLB_TPOAM_EVT_AIS_TX:
        return "AIS TX";
    case GLB_TPOAM_EVT_AIS_DEFECT:
        return "AIS Defect";
    case GLB_TPOAM_EVT_CSF_DEFECT:
        return "CSF Defect";
    case GLB_TPOAM_EVT_LCK_RX:
        return "LCK RX";
    case GLB_TPOAM_EVT_LCK_TX:
        return "LCK TX";
    case GLB_TPOAM_EVT_LCK_DEFECT:
        return "LCK Defect";
    case GLB_TPOAM_EVT_EVT_CACHE_FULL:
        return "Event Full";
    case GLB_TPOAM_EVT_1DM_CACHE_FULL:
        return "1DM Full";
    #endif
    case GLB_TPOAM_EVT_WAIT_1ST_CC:
        return "Wait 1st CCM";
    /* modified by yaom for CRC trigger AIS 20140919 */       
    case GLB_TPOAM_EVT_CRC_DEFECT:
        return "CRC RX";
    default:
        return "Invalid";
    }
}

char*
tpoam_api_direction_desc(tpoam_direction_t direction)
{
    switch (direction)
    {
    case TPOAM_DIR_EAST:
        return "east";
    case TPOAM_DIR_WEST:
        return "west";
    case TPOAM_DIR_NONE:
        return "ignore";        
    default:
        return "error";
    }
}

char*
_tpoam_lb_sub_type_desc(uint8 lb_sub_type)
{
    switch (lb_sub_type)
    {
    case TPOAM_LB_DISCOVERY:
        return "Discovery";
    case TPOAM_LB_DISCOVERY_EGRESS:
        return "Discovery egress";
    case TPOAM_LB_MEP:
        return "MEP";
    case TPOAM_LB_MIP:
        return "MIP";
    default:
        return "Invalid";
    }
}


#endif

#ifndef cmd_show
#ifndef show_mp
static void
_tpoam_api_show_oam(FILE *fp, tbl_oam_session_t* pst_oam)
{
    sal_fprintf(fp, "%-13s %-3d ", pst_oam->megid_str, pst_oam->level);
    if ((0 != pst_oam->mepid) && (TPOAM_MP_TYPE_MIP != pst_oam->mp_type))
    {
        sal_fprintf(fp, "%-3s %-4d %-3s %-5s ", tpoam_api_mp_type_desc(pst_oam->mp_type), 
            pst_oam->mepid, tpoam_api_en_desc(TRUE, pst_oam->cc_enable), 
            tpoam_api_cc_interval_desc(pst_oam->cc_interval));
        if (TPOAM_INVALID_MEP_ID != pst_oam->rmepid)
        {
            sal_fprintf(fp, "%-4d %-5s", pst_oam->rmepid, 
                tpoam_api_signal_status_desc(pst_oam->signal_status)); /* modified by yaom for CRC trigger AIS 20140919 */
        }
        else
        {
            sal_fprintf(fp, "%-4s %-5s", "", "");
        }
    }
    else if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
    {
        sal_fprintf(fp, "%-3s", tpoam_api_mp_type_desc(pst_oam->mp_type));
    }
}


static int
_tpoam_api_show_mp_pw_tpe_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_pw_tpe_t* pst_tpe = obj;
    FILE *fp = pargs->argv[0];
    uint32* p_is_first = (uint32*)(pargs->argv[1]);

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (0 != pst_tpe->oam_session_id)
    {
        if (*p_is_first)
        {
            sal_fprintf(fp, "%-120s %-6s %-13s %-3s %-3s %-4s %-3s %-5s %-4s %-5s\n", "PW", "Type", "MEG", "LVL", "MP", "LMEP", "CCM", "INTVL", "RMEP", "State");
            sal_fprintf(fp, "---------------------------------------------------------------------------\n");
            *p_is_first = FALSE;
        }
        sal_fprintf(fp, "%-120s %-6s ", pst_tpe->key.name, "T-PE");
        
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
        if (NULL != p_db_oam_session)
        {
            _tpoam_api_show_oam(fp, p_db_oam_session);
        }
        
        sal_fprintf(fp, "\n");
    }
    return PM_E_NONE;
}


static int
_tpoam_api_show_mp_section_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_section_oam_t* pst_section = obj;
    FILE *fp = pargs->argv[0];
    uint32* p_is_first = (uint32*)(pargs->argv[1]);

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (0 != pst_section->oam_session_id)
    {
        if (*p_is_first)
        {
            sal_fprintf(fp, "%-127s %-13s %-3s %-3s %-4s %-3s %-5s %-4s %-5s\n", "SECTION", "MEG", "LVL", "MP", "LMEP", "CCM", "INTVL", "RMEP", "State");
            sal_fprintf(fp, "---------------------------------------------------------------------------\n");
            *p_is_first = FALSE;
        }
        sal_fprintf(fp, "%-127s ", tpoam_api_get_ifname_ext_by_ifindex(pst_section->key.ifindex));
        
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
        if (NULL != p_db_oam_session)
        {
            _tpoam_api_show_oam(fp, p_db_oam_session);
        }
        
        sal_fprintf(fp, "\n");
    }
    return PM_E_NONE;
}


static int
_tpoam_api_show_mp_lsp_pe_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_lsp_pe_t* pst_lsp_pe = obj;
    FILE *fp = pargs->argv[0];
    uint32* p_is_first = (uint32*)(pargs->argv[1]);

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (0 != pst_lsp_pe->oam_session_id)
    {
        if (*p_is_first)
        {
            sal_fprintf(fp, "%-120s %-6s %-13s %-3s %-3s %-4s %-3s %-5s %-4s %-5s\n", "LSP", "Type", "MEG", "LVL", "MP", "LMEP", "CCM", "INTVL", "RMEP", "State");
            sal_fprintf(fp, "---------------------------------------------------------------------------\n");
            *p_is_first = FALSE;
        }
        
        sal_fprintf(fp, "%-120s %-6s ", pst_lsp_pe->key.name, "LSP-PE");
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
        if (NULL != p_db_oam_session)
        {
            _tpoam_api_show_oam(fp, p_db_oam_session);
        }
        
        sal_fprintf(fp, "\n");
    }
    return PM_E_NONE;
}


static int
_tpoam_api_show_mp_lsp_p_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_lsp_p_t* pst_lsp_p = obj;
    FILE *fp = pargs->argv[0];
    uint32* p_is_first = (uint32*)(pargs->argv[1]);

    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;
    
    if (0 != pst_lsp_p->oam_session_id)
    {
        if (*p_is_first)
        {
            sal_fprintf(fp, "%-120s %-6s %-13s %-3s %-3s %-4s %-3s %-5s %-4s %-5s\n", "LSP", "Type", "MEG", "LVL", "MP", "LMEP", "CCM", "INTVL", "RMEP", "State");
            sal_fprintf(fp, "---------------------------------------------------------------------------\n");
            *p_is_first = FALSE;
        }
        
        sal_fprintf(fp,"%-120s %-6s ", pst_lsp_p->key.name, "LSP-P");
        oam_session_key.session_id = pst_lsp_p->oam_session_id;
        p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
        if (NULL != p_db_oam_session)
        {
            _tpoam_api_show_oam(fp, p_db_oam_session);
        }
        
        sal_fprintf(fp, "\n");
    }
    return PM_E_NONE;
}


static int
_tpoam_api_show_mp_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32* p_is_first = (uint32*)(pargs->argv[1]);
    uint32* p_oam_type = (uint32*)(pargs->argv[2]);
    
    if (*p_is_first)
    {
        sal_fprintf(fp, "%-6s %-12s %-13s %-3s %-3s %-4s %-3s %-5s %-4s %-5s\n", 
            "SessID", "Type", "MEG", "LVL", "MP", "LMEP", "CCM", "INTVL", "RMEP", "State");
        sal_fprintf(fp, "------+------------+-------------+---+---+----+---+-----+----+----------\n");
        *p_is_first = FALSE;
    }

    if (*p_oam_type == 0 || *p_oam_type == pst_oam->oam_type)
    {
        sal_fprintf(fp, "%-6d %-12s ", pst_oam->key.session_id, 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type));
    
        _tpoam_api_show_oam(fp, pst_oam);
        
        sal_fprintf(fp, "\n");
    }
    
    return PM_E_NONE;
}

int32
tpoam_api_lb_show_para(char *str_return, uint32* len, tpoam_lbm_para_t* pst_para)
{
    if (TPOAM_LB_MIP == pst_para->lb_sub_type)
    {
        sal_sprintf(str_return + *len, "TTL           :   %d\n", pst_para->ttl);
        *len = sal_strlen(str_return);
    }
    else if (TPOAM_LB_DISCOVERY == pst_para->lb_sub_type)
    {
        sal_sprintf(str_return + *len, "TTL           :   [%d-%d]\n", pst_para->ttl, pst_para->ttl_end);
        *len = sal_strlen(str_return);
    }
    else if (TPOAM_LB_MEP == pst_para->lb_sub_type)
    {
        sal_sprintf(str_return + *len, "Remote MEP    :   %d\n", pst_para->rmep_id);
        *len = sal_strlen(str_return);
    }
    sal_sprintf(str_return + *len, "Timeout       :   %d\n", pst_para->time_out);
    *len = sal_strlen(str_return);
    sal_sprintf(str_return + *len, "Repeat Count  :   %d\n", pst_para->repeat_cnt);
    *len = sal_strlen(str_return);
    sal_sprintf(str_return + *len, "EXP           :   %d\n", pst_para->exp);
    *len = sal_strlen(str_return);

    #if 0
    uint32 data_tlv = FALSE;
    uint32 test_tlv = FALSE;
    
    sal_sprintf(str_return + *len, "Req-MEP TLV   :   %s\n", tpoam_api_status_desc(pst_para->is_req_mep_id));
    *len = sal_strlen(str_return);
    data_tlv = (TPOAM_Y1731_PDU_DATA_TLV == pst_para->opt_tlv_type);
    sal_sprintf(str_return + *len, "Data TLV      :   %s\n", tpoam_api_status_desc(data_tlv));
    *len = sal_strlen(str_return);
    if (data_tlv)
    {
        sal_sprintf(str_return + *len, "Data TLV Len  :   %d\n", pst_para->opt_tlv_len);
        *len = sal_strlen(str_return);
    }
    test_tlv = (TPOAM_Y1731_PDU_TEST_TLV == pst_para->opt_tlv_type);
    sal_sprintf(str_return + *len, "Test TLV      :   %s\n", tpoam_api_status_desc(test_tlv));
    *len = sal_strlen(str_return);
    if (test_tlv)
    {
        sal_sprintf(str_return + *len, "Test TLV Len  :   %d\n", pst_para->opt_tlv_len);
        *len = sal_strlen(str_return);
        sal_sprintf(str_return + *len, "Test TLV Mode :   %s\n", tpoam_api_test_tlv_mode_desc(pst_para->test_tlv_mode));
        *len = sal_strlen(str_return);
    }    
    #endif
    return PM_E_NONE;
}


int32
_tpoam_api_show_stats(FILE *fp, tpoam_stats_t* pst_stats, uint32 show_statistics, uint32 session_id, uint32 show_hardware_process)
{
    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (show_statistics)
    {
        sal_fprintf(fp, "\nStatistics\n");
    }

    if (show_hardware_process)
    {
        sal_fprintf(fp, "\"-\" means hardware processed\n");
    }

    if (session_id)
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        sal_fprintf(fp, "%16s %u\n", "Session ID     :", session_id);
    }
    
    sal_fprintf(fp, "%-17s %-10s %-10s %-10s\n", "Packet", "TX", "RX", "RX Discard");
    sal_fprintf(fp, "-----------------+----------+----------+----------\n");
    if (p_db_oam_session && TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
    {
        sal_fprintf(fp, "%-17s %-10s %-10d %-10d\n", "LBM", 
            "-", pst_stats->rx.lbm, pst_stats->err.lbm);
        sal_fprintf(fp, "%-17s %-10d %-10s %-10d\n", "LBR", 
            pst_stats->tx.lbr, "-", pst_stats->err.lbr);
    }
    else
    {
        /*show global statistics and mep*/
        sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "LBM", 
            pst_stats->tx.lbm, pst_stats->rx.lbm, pst_stats->err.lbm);
        sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "LBR", 
            pst_stats->tx.lbr, pst_stats->rx.lbr, pst_stats->err.lbr);
    }
        
    #ifdef support_ais
    sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "AIS", 
        pst_stats->tx.ais, pst_stats->rx.ais, pst_stats->err.ais);
    #endif
    #ifdef support_1dm
    sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "1DM", 
        pst_stats->tx.onedm, pst_stats->rx.onedm, pst_stats->err.onedm);
    #endif

    if (p_db_oam_session && TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
    {
    }
    else
    {
        sal_fprintf(fp, "%-17s %-10d %-10s %-10d\n", "DMM", 
            pst_stats->tx.dmm, "-", pst_stats->err.dmm);
        sal_fprintf(fp, "%-17s %-10s %-10d %-10d\n", "DMR", 
            "-", pst_stats->rx.dmr, pst_stats->err.dmr);
    }
    
    #ifdef support_lck
    sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "LCK", 
        pst_stats->tx.lck, pst_stats->rx.lck, pst_stats->err.lck);
    #endif
    #ifdef support_csf
    sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "CSF", 
        pst_stats->tx.csf, pst_stats->rx.csf, pst_stats->err.csf);
    #endif

    if (p_db_oam_session && TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
    {
    }
    else
    {
        sal_fprintf(fp, "%-17s %-10d %-10d %-10d\n", "APS", 
            pst_stats->tx.aps, pst_stats->rx.aps, pst_stats->err.aps);
    }

    return PM_E_NONE;
}

static int32
_tpoam_api_show_oam_detail(FILE *fp, tbl_oam_session_t* pst_oam)
{
    //tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    
    sal_fprintf(fp, "Service Type  :   %s\n", tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type));
    sal_fprintf(fp, "MP Type       :   %s\n", tpoam_api_mp_type_desc(pst_oam->mp_type));
    sal_fprintf(fp, "MEGID         :   %s\n", pst_oam->megid_str);
    sal_fprintf(fp, "Level         :   %d\n", pst_oam->level);
    sal_fprintf(fp, "ICC length    :   %d\n", pst_oam->icclen);

    if (TPOAM_MP_TYPE_MEP == pst_oam->mp_type)
    {
        /* has MEP */
        if (0 != pst_oam->mepid)
        {
            sal_fprintf(fp, "MEPID         :   %d\n", pst_oam->mepid);
            sal_fprintf(fp, "CC            :   %s\n", tpoam_api_en_desc(FALSE, pst_oam->cc_enable));
            sal_fprintf(fp, "CC Interval   :   %s\n", tpoam_api_cc_interval_desc(pst_oam->cc_interval));
            sal_fprintf(fp, "EXP           :   %d\n", pst_oam->exp);
            if (pst_oam->rmepid)
            {
                sal_fprintf(fp, "RMEPID        :   %d\n", pst_oam->rmepid);
                sal_fprintf(fp, "STATE         :   %-5s\n", tpoam_api_signal_status_desc(pst_oam->signal_status)); /* modified by yaom for CRC trigger AIS 20140919 */
            }
            else
            {
                sal_fprintf(fp, "RMEPID        :   None\n");
                sal_fprintf(fp, "STATE         :   %-5s\n", "");
            }
        }
        else
        {
            sal_fprintf(fp, "MEPID         :   None\n");
        }

        #ifdef support_event
        uint32 i = 0;
        /* Event */
        sal_fprintf(fp, "\nEvent Cache\n");
        sal_fprintf(fp, "----------------------------\n");
        sal_fprintf(fp, "Max Size      :   %d\n", pst_oam->oam_event.max_count);
        sal_fprintf(fp, "Current Size  :   %d\n", pst_oam->oam_event.pst_event_cache_list->count);
        
        sal_fprintf(fp, "\n");

        /* Status */
        sal_fprintf(fp, "%-17s %-10s\n", "Event", "Defect");
        sal_fprintf(fp, "----------------------------\n");
        for (i = 0; i < GLB_TPOAM_EVT_NUM; i++)
        {
            sal_fprintf(fp, "%-17s %-10s\n", tpoam_api_event_desc((1 << i)), 
                tpoam_api_status_desc(GLB_IS_BIT_SET(pst_oam->oam_status, i)));
        }
        #endif

        #ifdef support_lb
        tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
        /* LB */
        sal_fprintf(fp, "\nLoopback\n");
        sal_fprintf(fp, "----------------------------\n");
        sal_fprintf(fp, "Active        :   %s\n", tpoam_api_status_desc(pst_lb->active));
        if (pst_lb->active)
        {
            sal_fprintf(fp, "Type          :   %s\n", tpoam_api_lb_subtype_desc(pst_lb->lbm_para.lb_sub_type));            
                tpoam_api_lb_show_para(fp, &pst_lb->lbm_para);
            sal_fprintf(fp, "Transaction ID:   %d\n", pst_lb->lbm_next_trans_id);
            sal_fprintf(fp, "TX            :   %d\n", pst_lb->lbm_tx_count);
            sal_fprintf(fp, "RX            :   %d\n", pst_lb->lbr_rx_count);
        }
        #endif

        #if 0
        /* DM */
        sal_fprintf(fp, "\nDM\n");
        sal_fprintf(fp, "----------------------------\n");
        sal_fprintf(fp, "Active        :   %s\n", tpoam_api_status_desc(pst_dm->active));
        if (pst_dm->active)
        {
            sal_fprintf(fp, "Type          :   %s\n", tpoam_api_dm_type_desc(pst_dm->para.type));
            sal_fprintf(fp, "Interval      :   %s\n", tpoam_api_dm_interval_desc(pst_dm->para.interval));
            sal_fprintf(fp, "EXP           :   %d\n", pst_dm->para.exp);
            sal_fprintf(fp, "Repeat        :   %d\n", pst_dm->para.repeat_cnt);
            sal_fprintf(fp, "TX            :   %d\n", pst_dm->tx_count);
            sal_fprintf(fp, "RX            :   %d\n", pst_dm->rx_count);
        }
        #endif
    }
    else
    {
        //sal_fprintf(fp, "MIP           :   %s\n", tpoam_api_en_desc(FALSE, pst_oam->mip_en));
    }

    _tpoam_api_show_stats(fp, &pst_oam->oam_stats, TRUE, 0, TRUE);
    //sal_fprintf(fp, "\n");
    return PM_E_NONE;
}



int32
tpoam_api_show_mp_section(FILE *fp, char* pch_section_name)
{
    tbl_section_oam_t* pst_section = NULL;
    int32 rc = PM_E_NONE;
    uint32 is_first = TRUE;

    /*show all section*/
    if (!pch_section_name)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &is_first;
        tbl_section_oam_iterate(_tpoam_api_show_mp_section_interator, &args);
        if (!is_first)
        {
            sal_fprintf(fp, "\n");
        }
    }
    else
    {
        rc = tpoam_api_get_section(pch_section_name, &pst_section);
        if (rc < 0)
        {
            return rc;
        }

        sal_fprintf(fp, "Section Name  :   %-12s\n", tpoam_api_get_ifname_ext_by_ifindex(pst_section->key.ifindex));
        {
            if (pst_section->key.ifindex)
            {
                char ifname[IFNAME_SIZE];
                char ifname_ext[IFNAME_EXT_SIZE] = {0};
                
                sal_memset(ifname, 0x00, sizeof(ifname));
                tbl_interface_get_name_by_ifindex(pst_section->key.ifindex, ifname, sizeof(ifname));
                IFNAME_ETH2FULL(ifname, ifname_ext);
                sal_fprintf(fp, "Ifname        :   %s\n", ifname_ext);
                // TODO: support mem_if
                #if 0
                TPOAM_SEC_NONIP_IS_VLAN_IF(pst_section)
                {
                    sal_fprintf(fp, "Member Ifname :   %s\n", if_index2name(&cli->vr->ifm, pst_section->mem_oifindex));
                }
                #endif
            }
        }

        if (!pst_section->oam_session_id)
        {
            sal_fprintf(fp, "No OAM\n");
        }
        else
        {
            tbl_oam_session_t *p_db_oam_session = NULL;
            
            p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
            if (NULL != p_db_oam_session)
            {
                _tpoam_api_show_oam_detail(fp, p_db_oam_session);
            }
            else
            {
                sal_fprintf(fp, "No OAM\n");
            }
        }
    }
    return PM_E_NONE;
}

int32
tpoam_api_show_mp_service_session(FILE *fp, uint32 oam_type)
{
    uint32 is_first = TRUE;
    //uint32 oam_type = GLB_OAM_TYPE_MPLS_LSP;
    tbl_iter_args_t args;
    args.argv[0] = fp;
    args.argv[1] = &is_first;
    args.argv[2] = &oam_type;
    tbl_oam_session_iterate(_tpoam_api_show_mp_session_interator, &args);
    if (!is_first)
    {
        sal_fprintf(fp, "\n");
    }

    return PM_E_NONE;
}

int32
tpoam_api_show_mp_lsp(FILE *fp, char* pch_lsp_name)
{
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_lsp_p_t *pst_lsp_p = NULL;
    int32 rc = PM_E_NONE;
    uint32 is_first = TRUE;

    /*show all lsp*/
    if (!pch_lsp_name)
    {    
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &is_first;
        tbl_lsp_pe_iterate(_tpoam_api_show_mp_lsp_pe_interator, &args);
        if (!is_first)
        {
            sal_fprintf(fp, "\n");
        }
        tbl_lsp_p_iterate(_tpoam_api_show_mp_lsp_p_interator, &args);
        if (!is_first)
        {
            sal_fprintf(fp, "\n");
        }
    }
    else
    {
        rc = tpoam_api_get_lsp_pe(pch_lsp_name, &pst_lsp_pe);
        if (rc < 0)
        {
            rc = tpoam_api_get_lsp_p(pch_lsp_name, &pst_lsp_p);
            if (rc < 0)
            {
                return rc;
            }
            else
            {
                /*show lsp_p*/
                sal_fprintf(fp, "LSP Name      :   %s\n", pst_lsp_p->key.name);
                if (0 == pst_lsp_p->oam_session_id)
                {
                    sal_fprintf(fp, "No OAM\n");
                }
                else
                {
                    tbl_oam_session_t *p_db_oam_session = NULL;
            
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        _tpoam_api_show_oam_detail(fp, p_db_oam_session);
                    }
                    else
                    {
                        sal_fprintf(fp, "No OAM\n");
                    }
                }
            }
        }
        else
        {
            /*show lsp_pe*/
            sal_fprintf(fp, "LSP Name      :   %s\n", pst_lsp_pe->key.name);
            if (0 == pst_lsp_pe->oam_session_id)
            {
                sal_fprintf(fp, "No OAM\n");
            }
            else
            {
                tbl_oam_session_t *p_db_oam_session = NULL;
            
                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
                if (NULL != p_db_oam_session)
                {
                    _tpoam_api_show_oam_detail(fp, p_db_oam_session);
                }
                else
                {
                    sal_fprintf(fp, "No OAM\n");
                }
            }
        }
    } 
    
    return PM_E_NONE;
}

int32
tpoam_api_show_mp_pw_tpe(FILE *fp, char* pch_tpe_name)
{
    tbl_pw_tpe_t* pst_tpe = NULL;
    int32 rc = PM_E_NONE;
    uint32 is_first = TRUE;

    /*show all pw_tpe*/
    if (!pch_tpe_name)
    {    
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &is_first;
        tbl_pw_tpe_iterate(_tpoam_api_show_mp_pw_tpe_interator, &args);
        if (!is_first)
        {
            sal_fprintf(fp, "\n");
        }
    }
    else
    {
        rc = tpoam_api_get_pw_tpe(pch_tpe_name, &pst_tpe);
        if (rc < 0)
        {
            return rc;
        }

        sal_fprintf(fp, "PW TPE Name   :   %s\n", pst_tpe->key.name);        
        if (0 == pst_tpe->oam_session_id)
        {
            sal_fprintf(fp, "No OAM\n");
        }
        else
        {
            tbl_oam_session_t *p_db_oam_session = NULL;
            
            p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
            if (NULL != p_db_oam_session)
            {
                _tpoam_api_show_oam_detail(fp, p_db_oam_session);
            }
            else
            {
                sal_fprintf(fp, "No OAM\n");
            }
        }
    } 
    return PM_E_NONE;
}


int32
tpoam_api_show_mp_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para)
{
    tbl_oam_session_t* pst_oam = NULL;
//    int32 rc = PM_E_NONE;
    uint32 is_first = TRUE;
    uint32 oam_type = 0;

    /*show all*/
    if (!session_id)
    {    
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &is_first;
        args.argv[2] = &oam_type;
        tbl_oam_session_iterate(_tpoam_api_show_mp_session_interator, &args);
        if (!is_first)
        {
            sal_fprintf(fp, "\n");
        }
    }
    else
    {
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (pst_oam)
        {
            sal_fprintf(fp, "Session ID    :   %d\n", pst_oam ->key.session_id);

            if (pst_oam->oam_type == GLB_OAM_TYPE_MPLS_LSP)
            {
                sal_fprintf(fp, "LSP Name      :   %s\n", pst_oam ->service_name);
            }
            else if (pst_oam->oam_type == GLB_OAM_TYPE_MPLS_PW)
            {
                sal_fprintf(fp, "PW TPE Name   :   %s\n", pst_oam->service_name);     
            }
            else if (pst_oam->oam_type == GLB_OAM_TYPE_MPLS_SECTION)
            {
                sal_fprintf(fp, "Section Name  :   %-12s\n", tpoam_api_get_ifname_ext_by_ifindex(pst_oam->service_ifindex));
            }
            else
            {
                sal_fprintf(fp, "No bind service\n");
                return PM_E_NONE;
            }

            _tpoam_api_show_oam_detail(fp, pst_oam);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    } 
    
    return PM_E_NONE;
}


#endif
int
_tpoam_api_show_section_param2_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_section_oam_t* pst_section = obj;
    FILE *fp = pargs->argv[0];
    tbl_oam_session_t *p_db_oam_session = NULL;
    
    tpoam_api_service_show_cb p_cb = (tpoam_api_service_show_cb)pargs->argv[1];

    if (0 == pst_section->oam_session_id)
    {
        return PM_E_NONE;
    }

    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
    if (NULL != p_db_oam_session)
    {
        /*clear cmd*/
        if (NULL == pargs->argv[0])
        {
            p_cb(NULL, p_db_oam_session, NULL);
        }
        else 
        {
            if (NULL != pargs->argv[2])
            {
                p_cb(fp, p_db_oam_session, (tpoam_lm_dual_enable_input_t *)(pargs->argv[2]));
            }
            else
            {
                p_cb(fp, p_db_oam_session, NULL);
            }
        }
    }

    return PM_E_NONE;
} 



int
_tpoam_api_show_lsp_pe_param2_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_lsp_pe_t* pst_lsp_pe = obj;
    FILE *fp = pargs->argv[0];
    tbl_oam_session_t *p_db_oam_session = NULL;
    
    tpoam_api_service_show_cb p_cb = (tpoam_api_service_show_cb)pargs->argv[1];
    
    if (0 == pst_lsp_pe->oam_session_id)
    {
        return PM_E_NONE;
    }

    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
    if (NULL != p_db_oam_session)
    {
        /*clear cmd*/
        if (NULL == pargs->argv[0])
        {
            p_cb(NULL, p_db_oam_session, NULL);
        }
        else 
        {
            if (NULL != pargs->argv[2])
            {
                p_cb(fp, p_db_oam_session, (tpoam_lm_dual_enable_input_t *)(pargs->argv[2]));
            }
            else
            {
                p_cb(fp, p_db_oam_session, NULL);
            }
        }
    }

    return PM_E_NONE;
} 

int
_tpoam_api_show_lsp_p_param2_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_lsp_p_t* pst_lsp_p = obj;
    FILE *fp = pargs->argv[0];
    tbl_oam_session_t *p_db_oam_session = NULL;
    
    tpoam_api_service_show_cb p_cb = (tpoam_api_service_show_cb)pargs->argv[1];
    
    if (0 == pst_lsp_p->oam_session_id)
    {
        return PM_E_NONE;
    }

    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
    if (NULL != p_db_oam_session)
    {
        /*clear cmd*/
        if (NULL == pargs->argv[0])
        {
            p_cb(NULL, p_db_oam_session, NULL);
        }
        else 
        {
            if (NULL != pargs->argv[2])
            {
                p_cb(fp, p_db_oam_session, (tpoam_lm_dual_enable_input_t *)(pargs->argv[2]));
            }
            else
            {
                p_cb(fp, p_db_oam_session, NULL);
            }
        }
    }

    return PM_E_NONE;
} 


int
_tpoam_api_show_pw_tpe_param2_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_pw_tpe_t* pst_pw_tpe = obj;
    FILE *fp = pargs->argv[0];
    tbl_oam_session_t *p_db_oam_session = NULL;

    tpoam_api_service_show_cb p_cb = (tpoam_api_service_show_cb)pargs->argv[1];
    
    if (0 == pst_pw_tpe->oam_session_id)
    {
        return PM_E_NONE;
    }
    
    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_pw_tpe->oam_session_id);
    if (NULL != p_db_oam_session)
    {
        /*clear cmd*/
        if (NULL == pargs->argv[0])
        {
            p_cb(NULL, p_db_oam_session, NULL);
        }
        else 
        {
            if (NULL != pargs->argv[2])
            {
                p_cb(fp, p_db_oam_session, (tpoam_lm_dual_enable_input_t *)(pargs->argv[2]));
            }
            else
            {
                p_cb(fp, p_db_oam_session, NULL);
            }
        }
    }

    return PM_E_NONE;
} 

#ifndef show_mp_status
void
tpoam_api_show_mp_status_session_title(FILE *fp)
{
#define NSM_EVENT_DESC(_event_) \
tpoam_api_event_brief_desc((_event_)), tpoam_api_event_desc((_event_))

    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_LOC), NSM_EVENT_DESC(GLB_TPOAM_EVT_UNEXP_MEP));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_UNEXP_PERIOD), NSM_EVENT_DESC(GLB_TPOAM_EVT_MISMERGE));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_MEG_LVL), NSM_EVENT_DESC(GLB_TPOAM_EVT_WAIT_1ST_CC));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_RDI_RX), NSM_EVENT_DESC(GLB_TPOAM_EVT_RDI_TX));
    #ifdef support_ais
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_AIS_RX), NSM_EVENT_DESC(GLB_TPOAM_EVT_AIS_TX));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_AIS_DEFECT), NSM_EVENT_DESC(GLB_TPOAM_EVT_LCK_RX));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_LCK_TX), NSM_EVENT_DESC(GLB_TPOAM_EVT_LCK_DEFECT));
    #endif
    /* modified by yaom for CRC trigger AIS 20140919 */
    sal_fprintf(fp, "%-6s - %-17s\n", NSM_EVENT_DESC(GLB_TPOAM_EVT_CRC_DEFECT));

    #ifdef support_ais
    sal_fprintf(fp, "\n%-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-5s  %-5s %-3s\n", "MEGID", "State", 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_LOC), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_MEP), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_PERIOD), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MISMERGE), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MEG_LVL), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_WAIT_1ST_CC), 
        "RDI",
        "AIS",
        "LCK",
        "CRC");
    sal_fprintf(fp, "%-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-5s  %-5s %-3s\n", "", "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "R-T",
        "R-T-D",
        "R-T-D",
        "");
    sal_fprintf(fp, "-----------------------------------------------------------------------------\n");
    #else
    sal_fprintf(fp, "\n%-6s %-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-3s\n", "SessID", "MEGID", "State", 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_LOC), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_MEP), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_PERIOD), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MISMERGE), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MEG_LVL), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_WAIT_1ST_CC), 
        "RDI",
        "CRC");
    sal_fprintf(fp, "%-6s %-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-3s\n", "", "", "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "R-T",
        "");
    sal_fprintf(fp, "------+-------------+-----+---+---+---+---+---+----+----+---\n");
    #endif
    
}

static void
_tpoam_api_show_oam_mp_status(FILE *fp, tbl_oam_session_t* pst_oam, uint32 *title_not_showed)
{
    if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
    {
        return;
    }
    
    if (*title_not_showed)
    {
        tpoam_api_show_mp_status_session_title(fp);
        *title_not_showed = FALSE;
    }

    #ifdef support_ais
    /* modified by yaom for CRC trigger AIS 20140919 */
    sal_fprintf(fp, "%-6u %-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %1s %1s  %1s %1s %1s  %1s %1s %1s %1s\n", pst_oam->key.session_id, pst_oam->megid_str,
        tpoam_api_signal_status_desc(pst_oam->signal_status),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_LOC)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_UNEXP_MEP)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_UNEXP_PERIOD)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_MISMERGE)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_MEG_LVL)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_WAIT_1ST_CC)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_RDI_RX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_RDI_TX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_AIS_RX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_AIS_TX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_AIS_DEFECT)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_LCK_RX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_LCK_TX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_LCK_DEFECT)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_CRC_DEFECT))
        );    
    #else
    sal_fprintf(fp, "%-6u %-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %1s %1s  %1s\n", pst_oam->key.session_id, pst_oam->megid_str,
        tpoam_api_signal_status_desc(pst_oam->signal_status),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_LOC)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_UNEXP_MEP)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_UNEXP_PERIOD)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_MISMERGE)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_MEG_LVL)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_WAIT_1ST_CC)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_RDI_RX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_RDI_TX)),
        tpoam_api_status_brief_desc(GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_CRC_DEFECT))
        );    
    #endif
}


static int
_tpoam_api_show_mp_status_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32 *title_not_showed = pargs->argv[1];

    _tpoam_api_show_oam_mp_status(fp, pst_oam, title_not_showed);
        
    return PM_E_NONE;
}

int32
tpoam_api_show_mp_status_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para)
{
    tbl_oam_session_t* pst_oam = NULL;
//    int32 rc = 0;
    uint32 title_not_showed = TRUE;

    if (0 == session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &title_not_showed;
        tbl_oam_session_iterate(_tpoam_api_show_mp_status_session_interator, &args);
        //if (!is_first)
        {
            //sal_fprintf(fp, "\n");
        }
    }
    else
    {
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (pst_oam)
        {
            if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
            {
                CFG_INVALID_PARAM_RET("Tpoam session %u is MIP", session_id);
            }
            _tpoam_api_show_oam_mp_status(fp, pst_oam, &title_not_showed);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    } 
    
    return PM_E_NONE;
}


int32
tpoam_api_show_mp_status(FILE *fp, uint32 type, tpoam_pw_type_t pw_type)
{

#define NSM_EVENT_DESC(_event_) \
tpoam_api_event_brief_desc((_event_)), tpoam_api_event_desc((_event_))

    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_LOC), NSM_EVENT_DESC(GLB_TPOAM_EVT_UNEXP_MEP));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_UNEXP_PERIOD), NSM_EVENT_DESC(GLB_TPOAM_EVT_MISMERGE));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_MEG_LVL), NSM_EVENT_DESC(GLB_TPOAM_EVT_WAIT_1ST_CC));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_RDI_RX), NSM_EVENT_DESC(GLB_TPOAM_EVT_RDI_TX));
    #ifdef support_ais
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_AIS_RX), NSM_EVENT_DESC(GLB_TPOAM_EVT_AIS_TX));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_AIS_DEFECT), NSM_EVENT_DESC(GLB_TPOAM_EVT_LCK_RX));
    sal_fprintf(fp, "%-6s - %-17s \t\t%-6s - %-17s \n", 
        NSM_EVENT_DESC(GLB_TPOAM_EVT_LCK_TX), NSM_EVENT_DESC(GLB_TPOAM_EVT_LCK_DEFECT));
    #endif
    /* modified by yaom for CRC trigger AIS 20140919 */
    sal_fprintf(fp, "%-6s - %-17s\n", NSM_EVENT_DESC(GLB_TPOAM_EVT_CRC_DEFECT));

    #ifdef support_ais
    sal_fprintf(fp, "\n%-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-5s  %-5s %-3s\n", "MEGID", "State", 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_LOC), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_MEP), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_PERIOD), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MISMERGE), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MEG_LVL), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_WAIT_1ST_CC), 
        "RDI",
        "AIS",
        "LCK",
        "CRC");
    sal_fprintf(fp, "%-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-5s  %-5s %-3s\n", "", "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "R-T",
        "R-T-D",
        "R-T-D",
        "");
    sal_fprintf(fp, "-----------------------------------------------------------------------------\n");
    #else
    sal_fprintf(fp, "\n%-6s %-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-3s\n", "SessID", "MEGID", "State", 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_LOC), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_MEP), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_UNEXP_PERIOD), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MISMERGE), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_MEG_LVL), 
        tpoam_api_event_brief_desc(GLB_TPOAM_EVT_WAIT_1ST_CC), 
        "RDI",
        "CRC");
    sal_fprintf(fp, "%-6s %-13s %-5s %-3s %-3s %-3s %-3s %-3s %-4s %-3s  %-3s\n", "", "", "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "", 
        "R-T",
        "");
    sal_fprintf(fp, "------+-------------+-----+---+---+---+---+---+----+----+-----------------\n");
    #endif

    if (GLB_OAM_TYPE_MPLS_SECTION == type || 0 == type)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = _tpoam_api_show_oam_mp_status;
        tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
    }
    
    if (GLB_OAM_TYPE_MPLS_LSP == type || 0 == type)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = _tpoam_api_show_oam_mp_status;
        tbl_lsp_pe_iterate(_tpoam_api_show_lsp_pe_param2_interator, &args);
        tbl_lsp_p_iterate(_tpoam_api_show_lsp_p_param2_interator, &args);
    }
    
    if (GLB_OAM_TYPE_MPLS_PW == type || 0 == type)
    {
        if (TPOAM_PW_TYPE_TPE == pw_type || TPOAM_PW_TYPE_MAX == pw_type)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_mp_status;
            tbl_lsp_pe_iterate(_tpoam_api_show_pw_tpe_param2_interator, &args);
        }
    }
    
    return PM_E_NONE;
}

#endif

#ifndef show_statistics

static int
_tpoam_api_show_statistics_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32 *hardware_process_not_showed = (uint32*)pargs->argv[1];
    uint32 *not_first = (uint32*)pargs->argv[2];

    if (*not_first)
    {
        sal_fprintf(fp, "\n");
    }
    else
    {
        *not_first = TRUE;
    }
    
    _tpoam_api_show_stats(fp, &pst_oam->oam_stats, FALSE, pst_oam->key.session_id, *hardware_process_not_showed);
    *hardware_process_not_showed = FALSE;
        
    return PM_E_NONE;
}

int32
tpoam_api_show_statistics_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para)
{
    tbl_oam_session_t* pst_oam = NULL;
    uint32 hardware_process_not_showed = TRUE;
    uint32 not_first = FALSE;

    if (0 == session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &hardware_process_not_showed;
        args.argv[2] = &not_first;
        tbl_oam_session_iterate(_tpoam_api_show_statistics_session_interator, &args);
        //if (!is_first)
        {
            //sal_fprintf(fp, "\n");
        }
    }
    else
    {
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (pst_oam)
        {
            _tpoam_api_show_stats(fp, &pst_oam->oam_stats, FALSE, pst_oam->key.session_id, TRUE);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    } 
    
    return PM_E_NONE;
}
#endif

#ifndef show_event
static void
_tpoam_api_show_oam_event(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t *bb)
{
    struct listnode* pst_node = NULL;
    tpoam_event_entry_t* pst_entry = NULL;
    tpoam_event_t* pst_event = &pst_oam->oam_event;
    struct sal_tm* pst_time = NULL;
    uint32 index = 0;

    LIST_LOOP(pst_event->pst_event_cache_list, pst_entry, pst_node)
    {
        index++;
        pst_time = sal_localtime(&pst_entry->time);
        sal_fprintf(fp, "%-3d %-6u %-7s %-13s %-5d %-14s %-6s %04d/%02d/%02d %02d:%02d:%02d\n", 
                index, pst_oam->key.session_id, 
                tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
                pst_oam->megid_str, 
                (pst_oam->mepid) ? pst_oam->mepid : 0, 
                tpoam_api_event_desc(pst_entry->event),
                pst_entry->is_set ? "occur" : "clear", 
                pst_time->tm_year + 1900, 
                pst_time->tm_mon + 1, 
                pst_time->tm_mday, 
                pst_time->tm_hour, 
                pst_time->tm_min, 
                pst_time->tm_sec);
    }
}


int32
tpoam_api_show_event(FILE *fp, uint32 type, tpoam_pw_type_t pw_type, char* pch_service_name)
{
    tbl_section_oam_t* pst_section = NULL;
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;
    int32 rc = PM_E_NONE;

    tbl_oam_session_t *p_db_oam_session = NULL;

    sal_fprintf(fp, "%-3s %-6s %-7s %-13s %-5s %-14s %-6s %-19s\n", "Idx", "SessID", "Type", "MEGID", "MEPID", "Event", "Action", "Time");
    sal_fprintf(fp, "---+------+-------+-------------+-----+--------------+------+---------\n");
    
    if (GLB_OAM_TYPE_MPLS_SECTION == type || 0 == type)
    {
        if (!pch_service_name)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_event;
            tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
        }
        else
        {
            rc = tpoam_api_get_section(pch_service_name, &pst_section);
            if (rc < 0)
            {
                return rc;
            }
            if (0 == pst_section->oam_session_id)
            {
                return TPOAM_E_SERVICE_NON_OAM;
            }
            else
            {
                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                if (NULL != p_db_oam_session)
                {
                    _tpoam_api_show_oam_event(fp, p_db_oam_session, NULL);
                }
            }
        }        
    }
    
    if (GLB_OAM_TYPE_MPLS_LSP == type || 0 == type)
    {
        if (!pch_service_name)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_event;
            tbl_lsp_pe_iterate(_tpoam_api_show_lsp_pe_param2_interator, &args);
            tbl_lsp_p_iterate(_tpoam_api_show_lsp_p_param2_interator, &args);
        }
        else
        {
            rc = tpoam_api_get_lsp_pe(pch_service_name, &pst_lsp_pe);
            if (rc < 0)
            {
                rc = tpoam_api_get_lsp_p(pch_service_name, &pst_lsp_p);
                if (rc < 0)
                {
                    return rc;
                }
                else
                {
                    if (0 == pst_lsp_p->oam_session_id)
                    {
                        return TPOAM_E_SERVICE_NON_OAM;
                    }
                    else
                    {
                        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                        if (NULL != p_db_oam_session)
                        {
                            _tpoam_api_show_oam_event(fp, p_db_oam_session, NULL);
                        }
                    }
                }
            }
            else
            {
                if (0 == pst_lsp_pe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                else
                {
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        _tpoam_api_show_oam_event(fp, p_db_oam_session, NULL);
                    }
                }
            }
        }
    }
    
    if (GLB_OAM_TYPE_MPLS_PW == type || 0 == type)
    {
        if (!pch_service_name)
        {
            if (TPOAM_PW_TYPE_TPE == pw_type || TPOAM_PW_TYPE_MAX == pw_type)
            {
                tbl_iter_args_t args;
                args.argv[0] = fp;
                args.argv[1] = _tpoam_api_show_oam_event;
                tbl_section_oam_iterate(_tpoam_api_show_pw_tpe_param2_interator, &args);
            }
        }
        else
        {
            if (TPOAM_PW_TYPE_TPE == pw_type)
            {
                rc = tpoam_api_get_pw_tpe(pch_service_name, &pst_tpe);
                if (rc < 0)
                {
                    return rc;
                }
                if (0 == pst_tpe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                else
                {
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        _tpoam_api_show_oam_event(fp, p_db_oam_session, NULL);
                    }
                }
            }
        }
    }
    
    return PM_E_NONE;
}

static int
_tpoam_api_show_event_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];

    _tpoam_api_show_oam_event(fp, pst_oam, NULL);
        
    return PM_E_NONE;
}

int32
tpoam_api_show_event_session(FILE *fp, uint32 session_id)
{
    tbl_oam_session_t *p_db_oam_session = NULL;

    sal_fprintf(fp, "%-3s %-6s %-7s %-13s %-5s %-14s %-6s %-19s\n", "Idx", "SessID", "Type", "MEGID", "MEPID", "Event", "Action", "Time");
    sal_fprintf(fp, "---+------+-------+-------------+-----+--------------+------+---------\n");
    
    if (!session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        tbl_oam_session_iterate(_tpoam_api_show_event_session_interator, &args);
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            _tpoam_api_show_oam_event(fp, p_db_oam_session, NULL);
        }
    }        
    
    
    return PM_E_NONE;
}
#endif

static void
_tpoam_api_show_oam_binding_flow(FILE *fp, uint32 bind_oam_flow_id, uint32 *had_title_not_showed)
{
    tbl_oam_flow_info_t *p_db_oam_flow_info = NULL;
    tbl_oam_flow_info_key_t oam_flow_info_key = {0};

    oam_flow_info_key.flow_id = bind_oam_flow_id;
    p_db_oam_flow_info = tbl_oam_flow_info_get_oam_flow_info(&oam_flow_info_key);

    if (p_db_oam_flow_info)
    {
        if (*had_title_not_showed)
        {
            sal_fprintf(fp, "%-6s %-13s %-14s %-15s\n", "SessID", "InlabelFlowID", "OutlabelFlowID", "OutlabelGroupID");
            sal_fprintf(fp, "------+-------------+--------------+---------------\n");
            *had_title_not_showed = FALSE;
        }

        if (0 == p_db_oam_flow_info->inlabel_flow_id)
        {
            sal_fprintf(fp, "%-6u %-13s ", 
                p_db_oam_flow_info->session_id, 
                "N/A");
        }
        else
        {
            sal_fprintf(fp, "%-6u %-13u ", 
                p_db_oam_flow_info->session_id, 
                p_db_oam_flow_info->inlabel_flow_id);
        }

        if (0 == p_db_oam_flow_info->outlabel_flow_id)
        {
            sal_fprintf(fp, "%-14s ", 
                "N/A");
        }
        else
        {
            sal_fprintf(fp, "%-14u ", 
                p_db_oam_flow_info->outlabel_flow_id);
        }

        if (0 == p_db_oam_flow_info->outlabel_group_id)
        {
            sal_fprintf(fp, "%-15s\n", 
                "N/A");
        }
        else
        {
            sal_fprintf(fp, "%-15u\n", 
                p_db_oam_flow_info->outlabel_group_id);
        }
    }
}

static int
_tpoam_api_show_binding_flow_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];

    if (pst_oam->bind_oam_flow_id)
    {
        _tpoam_api_show_oam_binding_flow(fp, pst_oam->bind_oam_flow_id, pargs->argv[1]);
    }
    
    return PM_E_NONE;
}

int32
tpoam_api_show_binding_flow_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para)
{
    tbl_oam_session_t *p_db_oam_session = NULL;
    uint32 had_title_not_showed = TRUE;
    
    if (!session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &had_title_not_showed;
        tbl_oam_session_iterate(_tpoam_api_show_binding_flow_session_interator, &args);
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            if (p_db_oam_session->bind_oam_flow_id)
            {
                _tpoam_api_show_oam_binding_flow(fp, p_db_oam_session->bind_oam_flow_id, &had_title_not_showed);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Tpoam session %u does not bind oam flow", session_id);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    }
    
    return PM_E_NONE;
}

#ifndef show_session
static void
_tpoam_api_show_oam_session(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t *bb)
{
    tbl_section_oam_t *pst_section = NULL;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    //tbl_lsp_p_t *pst_lsp_p = NULL;
    tbl_pw_tpe_t * pst_tpe = NULL;

    uint32 ifindex = 0;
    uint32 outlabel = 0;

    if (GLB_OAM_TYPE_MPLS_SECTION == pst_oam->oam_type)
    {
        pst_section = tbl_section_oam_get_section_oam_by_ifindex(pst_oam->service_ifindex);
        if (NULL == pst_section)
        {
            //continue;
            return;
        }
        else
        {
            ifindex = pst_section->key.ifindex;
            outlabel = 0;
        }
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == pst_oam->oam_type)
    {
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_oam->service_name);
        if (NULL == pst_lsp_pe)
        {
            //continue;
            return;
        }
        else
        {
            ifindex = pst_lsp_pe->ifindex;
            outlabel = pst_lsp_pe->outlabel;
        }
    }
    else if (GLB_OAM_TYPE_MPLS_PW == pst_oam->oam_type)
    {
        pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(pst_oam->service_name);
        if (NULL == pst_tpe)
        {
            //continue;
            return;
        }
        else
        {
            ifindex = 0;
            outlabel = pst_tpe->outlabel;
        }
    }
    else
    {
        ifindex = 0;
        outlabel = 0;
    }
    
    sal_fprintf(fp, "%-6u %-7s %-13s %-5d %-14u %-6u\n", 
            pst_oam->key.session_id, 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
            pst_oam->megid_str, 
            (pst_oam->mepid) ? pst_oam->mepid : 0, 
            outlabel,
            ifindex);
}


static int
_tpoam_api_show_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];

    _tpoam_api_show_oam_session(fp, pst_oam, NULL);
        
    return PM_E_NONE;
}

int32
tpoam_api_show_session_2(FILE *fp, uint32 session_id)
{
    tbl_oam_session_t *p_db_oam_session = NULL;

    sal_fprintf(fp, "%-6s %-7s %-13s %-5s %-14s %-6s\n", 
        "SessID", "Type", "MEGID", "MEPID", "OutLabel", "OfPort");
    sal_fprintf(fp, "------+-------+-------------+-----+--------------+----------------\n");
    
    if (!session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        tbl_oam_session_iterate(_tpoam_api_show_session_interator, &args);
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            _tpoam_api_show_oam_session(fp, p_db_oam_session, NULL);
        }
    }
    
    return PM_E_NONE;
}

void
tpoam_api_show_session_unbind_title(FILE *fp)
{
    sal_fprintf(fp, "%-6s %-7s %-13s %-5s %-6s\n", 
        "SessID", "Type", "MEGID", "MEPID", "RMEPID");
    sal_fprintf(fp, "------+-------+-------------+------+------\n");
}

void
tpoam_api_show_session_section_title(FILE *fp)
{
    sal_fprintf(fp, "%-6s %-7s %-13s %-5s %-6s %-11s\n", 
        "SessID", "Type", "MEGID", "MEPID", "RMEPID", "Port");
    sal_fprintf(fp, "------+-------+-------------+-----+------+-----------\n");
}

void
tpoam_api_show_session_lsp_title(FILE *fp)
{
    sal_fprintf(fp, "%-6s %-7s %-13s %-5s %-6s %-8s %-8s %-11s\n", 
        "SessID", "Type", "MEGID", "MEPID", "RMEPID", "InLabel", "OutLabel", "Port");
    sal_fprintf(fp, "------+-------+-------------+-----+------+--------+--------+-----------\n");
}

void
tpoam_api_show_session_lsp_p_title(FILE *fp)
{
    sal_fprintf(fp, "%-4s %-7s %-13s %-7s %-7s %-10s %-7s %-7s %-10s\n", 
        "SeID", "Type", "MEGID", "WInLab", "WOutLab", "WOutPort", "EInLab", "EOutLab", "EOutPort");
    sal_fprintf(fp, "----+-------+-------------+-------+-------+----------+-------+-------+----------\n");
}

void
tpoam_api_show_session_pw_title(FILE *fp)
{
    sal_fprintf(fp, "%-6s %-7s %-13s %-5s %-6s %-8s %-8s %-7s %-11s\n", 
        "SessID", "Type", "MEGID", "MEPID", "RMEPID", "InLabel", "OutLabel", "GroupID", "Port");
    sal_fprintf(fp, "------+-------+-------------+-----+------+--------+--------+-------+-----------\n");
}

static void
_tpoam_api_show_oam_session_unbind(FILE *fp, tbl_oam_session_t* pst_oam, uint32 *had_title_showed)
{
    if (0 != pst_oam->oam_type)
    {
        return;
    }

    if (!*had_title_showed)
    {
        tpoam_api_show_session_unbind_title(fp);
        *had_title_showed = 1;
    }
    
    sal_fprintf(fp, "%-6u %-7s %-13s %-5d %-6d\n", 
            pst_oam->key.session_id, 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
            pst_oam->megid_str, 
            (pst_oam->mepid) ? pst_oam->mepid : 0,
            (pst_oam->rmepid) ? pst_oam->rmepid : 0);
}

static void
_tpoam_api_show_oam_session_section(FILE *fp, tbl_oam_session_t* pst_oam, uint32 *had_title_showed, uint32 print_enter)
{
    tbl_section_oam_t *pst_section = NULL;
    uint32 ifindex = 0;

    if (GLB_OAM_TYPE_MPLS_SECTION == pst_oam->oam_type)
    {
        pst_section = tbl_section_oam_get_section_oam_by_ifindex(pst_oam->service_ifindex);
        if (NULL == pst_section)
        {
            //continue;
            return;
        }
        else
        {
            ifindex = pst_section->key.ifindex;
        }
    }
    else
    {
        return;
    }

    if (print_enter && !*had_title_showed)
    {
        sal_fprintf(fp, "\n");
    }

    if (!*had_title_showed)
    {
        tpoam_api_show_session_section_title(fp);
        *had_title_showed = 1;
    }

    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    sal_fprintf(fp, "%-6u %-7s %-13s %-5d %-6d %-11s\n", 
            pst_oam->key.session_id, 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
            pst_oam->megid_str, 
            (pst_oam->mepid) ? pst_oam->mepid : 0, 
            (pst_oam->rmepid) ? pst_oam->rmepid : 0, 
            tpoam_ifindex2namefull(ifname_ext, ifindex) ? ifname_ext : "");
}

static void
_tpoam_api_show_oam_session_lsp(FILE *fp, tbl_oam_session_t* pst_oam, uint32 *had_title_showed, uint32 print_enter)
{
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    //tbl_lsp_p_t *pst_lsp_p = NULL;

    uint32 ifindex = 0;
    uint32 inlabel = 0;
    uint32 outlabel = 0;

    if (GLB_OAM_TYPE_MPLS_LSP == pst_oam->oam_type)
    {
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_oam->service_name);
        if (NULL == pst_lsp_pe)
        {
            //continue;
            return;
        }
        else
        {
            ifindex = pst_lsp_pe->ifindex;
            inlabel = pst_lsp_pe->inlabel;
            outlabel = pst_lsp_pe->outlabel;
        }
    }
    else
    {
        return;
    }

    if (print_enter && !*had_title_showed)
    {
        sal_fprintf(fp, "\n");
    }

    if (!*had_title_showed)
    {
        tpoam_api_show_session_lsp_title(fp);
        *had_title_showed = 1;
    }
    
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    sal_fprintf(fp, "%-6u %-7s %-13s %-5d %-6d %-8u %-8u %-11s\n", 
            pst_oam->key.session_id, 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
            pst_oam->megid_str, 
            (pst_oam->mepid) ? pst_oam->mepid : 0, 
            (pst_oam->rmepid) ? pst_oam->rmepid : 0, 
            inlabel,
            outlabel,
            tpoam_ifindex2namefull(ifname_ext, ifindex) ? ifname_ext : "");
}

static void
_tpoam_api_show_oam_session_lsp_p(FILE *fp, tbl_oam_session_t* pst_oam, uint32 *had_title_showed, uint32 print_enter)
{
    tbl_lsp_p_t *pst_lsp_p = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    char ifname_ext_2[IFNAME_EXT_SIZE] = {0};

    if ((GLB_OAM_TYPE_MPLS_LSP == pst_oam->oam_type) && (TPOAM_MP_TYPE_MIP == pst_oam->mp_type))
    {
        pst_lsp_p = tbl_lsp_p_get_lsp_p_by_name(pst_oam->service_name);
        if (NULL == pst_lsp_p)
        {
            //continue;
            return;
        }
    }
    else
    {
        return;
    }

    if (print_enter && !*had_title_showed)
    {
        sal_fprintf(fp, "\n");
    }

    if (!*had_title_showed)
    {
        tpoam_api_show_session_lsp_p_title(fp);
        *had_title_showed = 1;
    }
    
    sal_fprintf(fp, "%-4u %-7s %-13s %-7d %-7d %-10s %-7d %-7d %-10s\n", 
            pst_oam->key.session_id, 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
            pst_oam->megid_str, 
            pst_lsp_p->west_inlabel, 
            pst_lsp_p->west_outlabel, 
            tpoam_ifindex2namefull(ifname_ext, pst_lsp_p->west_ifindex) ? ifname_ext : "",
            pst_lsp_p->east_inlabel,
            pst_lsp_p->east_outlabel,
            tpoam_ifindex2namefull(ifname_ext_2, pst_lsp_p->east_ifindex) ? ifname_ext_2 : "");
}

static void
_tpoam_api_show_oam_session_pw(FILE *fp, tbl_oam_session_t* pst_oam, uint32 *had_title_showed, uint32 print_enter)
{
    tbl_pw_tpe_t * pst_tpe = NULL;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;

    uint32 ifindex = 0;
    uint32 inlabel = 0;
    uint32 outlabel = 0;
    uint32 aps_group_id = 0;

    if (GLB_OAM_TYPE_MPLS_PW == pst_oam->oam_type)
    {
        pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(pst_oam->service_name);
        if (NULL == pst_tpe)
        {
            //continue;
            return;
        }
        else
        {
            /*ifindex*/
            if (0 != sal_strlen(pst_tpe->lsp_name))
            {
                pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_tpe->lsp_name);
                if (NULL == pst_lsp_pe)
                {
                    return;
                }
                else
                {
                    ifindex = pst_lsp_pe->ifindex;
                    inlabel = pst_tpe->inlabel;
                    outlabel = pst_tpe->outlabel;
                }
            }
            /*aps group id*/
            else
            {
                inlabel = pst_tpe->inlabel;
                outlabel = pst_tpe->outlabel;
                aps_group_id = pst_tpe->lsp_aps_group_id;
            }
        }
    }
    else
    {
        return;
    }

    if (print_enter && !*had_title_showed)
    {
        sal_fprintf(fp, "\n");
    }

    if (!*had_title_showed)
    {
        tpoam_api_show_session_pw_title(fp);
        *had_title_showed = 1;
    }
    
    if (0 != ifindex)
    {
        char ifname_ext[IFNAME_EXT_SIZE] = {0};
        
        sal_fprintf(fp, "%-6u %-7s %-13s %-5d %-6d %-8u %-8u %-7s %-11s\n", 
                pst_oam->key.session_id, 
                tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
                pst_oam->megid_str, 
                (pst_oam->mepid) ? pst_oam->mepid : 0, 
                (pst_oam->rmepid) ? pst_oam->rmepid : 0, 
                inlabel,
                outlabel,
                "-",
                tpoam_ifindex2namefull(ifname_ext, ifindex) ? ifname_ext : "");
    }
    else
    {
        sal_fprintf(fp, "%-6u %-7s %-13s %-5d %-6d %-8u %-8u %-7u %-11s\n", 
                pst_oam->key.session_id, 
                tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type),
                pst_oam->megid_str, 
                (pst_oam->mepid) ? pst_oam->mepid : 0, 
                (pst_oam->rmepid) ? pst_oam->rmepid : 0, 
                inlabel,
                outlabel,
                aps_group_id,
                "-");
    }
}

static int
_tpoam_api_show_session_interator_2(void *obj, tbl_iter_args_t *pargs)
{
    uint32 *had_title_showed = pargs->argv[2];
    uint32 *print_enter = pargs->argv[3];

    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32 *oam_type = pargs->argv[1];
    uint32 *mp_type = pargs->argv[4];

    if (*oam_type == 0)
    {
        _tpoam_api_show_oam_session_unbind(fp, pst_oam, had_title_showed);
    }
    else if (*oam_type == GLB_OAM_TYPE_MPLS_SECTION)
    {
        _tpoam_api_show_oam_session_section(fp, pst_oam, had_title_showed, *print_enter);
    }
    else if ((*oam_type == GLB_OAM_TYPE_MPLS_LSP) && (TPOAM_MP_TYPE_MEP == *mp_type))
    {
        _tpoam_api_show_oam_session_lsp(fp, pst_oam, had_title_showed, *print_enter);
    }
    else if ((*oam_type == GLB_OAM_TYPE_MPLS_LSP) && (TPOAM_MP_TYPE_MIP == *mp_type))
    {
        _tpoam_api_show_oam_session_lsp_p(fp, pst_oam, had_title_showed, *print_enter);
    }
    else if (*oam_type == GLB_OAM_TYPE_MPLS_PW)
    {
        _tpoam_api_show_oam_session_pw(fp, pst_oam, had_title_showed, *print_enter);
    }
        
    return PM_E_NONE;
}

int32
tpoam_api_show_session(FILE *fp, uint32 session_id, uint32 oam_type, uint32 mp_type, cfg_cmd_para_t *para)
{
    tbl_oam_session_t *p_db_oam_session = NULL;
    uint32 had_unbind_title_showed = 0;
    uint32 had_section_title_showed = 0;
    uint32 had_lsp_title_showed = 0;
    uint32 had_lsp_p_title_showed = 0;
    uint32 had_pw_title_showed = 0;
    uint32 print_enter = 0;
    tbl_iter_args_t args;

    uint32 oam_type_tmp = 0;
    uint32 mp_type_tmp = 0;

    if (0 == session_id)
    {
        /*unbind*/
        if (GLB_OAM_TYPE_MPLS_ALL == oam_type || 0 == oam_type)
        {
            oam_type_tmp = 0;
            args.argv[0] = fp;
            args.argv[1] = &oam_type_tmp;
            args.argv[2] = &had_unbind_title_showed;
            args.argv[3] = &print_enter;
            args.argv[4] = &mp_type;
            tbl_oam_session_iterate(_tpoam_api_show_session_interator_2, &args);
        }

        if (had_unbind_title_showed)
        {
            print_enter = TRUE;
        }

        /*section*/
        if (GLB_OAM_TYPE_MPLS_ALL == oam_type || GLB_OAM_TYPE_MPLS_SECTION == oam_type)
        {
            oam_type_tmp = GLB_OAM_TYPE_MPLS_SECTION;
            args.argv[0] = fp;
            args.argv[1] = &oam_type_tmp;
            args.argv[2] = &had_section_title_showed;
            args.argv[3] = &print_enter;
            args.argv[4] = &mp_type;
            tbl_oam_session_iterate(_tpoam_api_show_session_interator_2, &args);
        }

        if ((!had_section_title_showed && print_enter) || had_section_title_showed)
        {
            print_enter = TRUE;
        }
        else
        {
            print_enter = FALSE;
        }

        /*lsp_pe*/
        if (GLB_OAM_TYPE_MPLS_ALL == oam_type || 
            (GLB_OAM_TYPE_MPLS_LSP == oam_type && TPOAM_MP_TYPE_MEP == mp_type))
        {
            oam_type_tmp = GLB_OAM_TYPE_MPLS_LSP;
            mp_type_tmp= TPOAM_MP_TYPE_MEP;
            args.argv[0] = fp;
            args.argv[1] = &oam_type_tmp;
            args.argv[2] = &had_lsp_title_showed;
            args.argv[3] = &print_enter;
            args.argv[4] = &mp_type_tmp;
            tbl_oam_session_iterate(_tpoam_api_show_session_interator_2, &args);
        }

        if ((!had_lsp_title_showed && print_enter) || had_lsp_title_showed)
        {
            print_enter = TRUE;
        }
        else
        {
            print_enter = FALSE;
        }

        /*pw*/
        if (GLB_OAM_TYPE_MPLS_ALL == oam_type || GLB_OAM_TYPE_MPLS_PW == oam_type)
        {
            oam_type_tmp = GLB_OAM_TYPE_MPLS_PW;
            args.argv[0] = fp;
            args.argv[1] = &oam_type_tmp;
            args.argv[2] = &had_pw_title_showed;
            args.argv[3] = &print_enter;
            args.argv[4] = &mp_type;
            tbl_oam_session_iterate(_tpoam_api_show_session_interator_2, &args);
        }

        if ((!had_pw_title_showed && print_enter) || had_pw_title_showed)
        {
            print_enter = TRUE;
        }
        else
        {
            print_enter = FALSE;
        }

        /*lsp_p*/
        if (GLB_OAM_TYPE_MPLS_ALL == oam_type || 
            (GLB_OAM_TYPE_MPLS_LSP == oam_type && TPOAM_MP_TYPE_MIP == mp_type))
        {
            oam_type_tmp = GLB_OAM_TYPE_MPLS_LSP;
            mp_type_tmp= TPOAM_MP_TYPE_MIP;
            args.argv[0] = fp;
            args.argv[1] = &oam_type_tmp;
            args.argv[2] = &had_lsp_p_title_showed;
            args.argv[3] = &print_enter;
            args.argv[4] = &mp_type_tmp;
            tbl_oam_session_iterate(_tpoam_api_show_session_interator_2, &args);
        }
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            if (0 == p_db_oam_session->oam_type)
            {
                _tpoam_api_show_oam_session_unbind(fp, p_db_oam_session, &had_unbind_title_showed);
            }
            else if (GLB_OAM_TYPE_MPLS_SECTION== p_db_oam_session->oam_type)
            {
                _tpoam_api_show_oam_session_section(fp, p_db_oam_session, &had_section_title_showed, FALSE);
            }
            else if (GLB_OAM_TYPE_MPLS_LSP == p_db_oam_session->oam_type && TPOAM_MP_TYPE_MEP == p_db_oam_session->mp_type)
            {
                _tpoam_api_show_oam_session_lsp(fp, p_db_oam_session, &had_lsp_title_showed, FALSE);
            }
            else if (GLB_OAM_TYPE_MPLS_LSP == p_db_oam_session->oam_type && TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
            {
                _tpoam_api_show_oam_session_lsp_p(fp, p_db_oam_session, &had_lsp_p_title_showed, FALSE);
            }
            else if (GLB_OAM_TYPE_MPLS_PW == p_db_oam_session->oam_type)
            {
                _tpoam_api_show_oam_session_pw(fp, p_db_oam_session, &had_pw_title_showed, FALSE);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    }

    return PM_E_NONE;
}

int32
tpoam_api_show_session_detail(FILE *fp, uint32 session_id, cfg_cmd_para_t *para)
{
    tbl_oam_session_t *p_db_oam_session = NULL;
    uint32 local_threshold = 0;
    uint32 remote_threshold = 0;
    uint32 delay_threshold = 0;

    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    char ifname_ext_2[IFNAME_EXT_SIZE] = {0};
    uint32 ifindex = 0;

    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
    if (NULL == p_db_oam_session)
    {
        CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
    }

    sal_fprintf(fp, "%-21s %s %u\n", "Session ID", ":", p_db_oam_session->key.session_id);
    sal_fprintf(fp, "%-21s %s %s\n", "MEGID",":", p_db_oam_session->megid_str);
    sal_fprintf(fp, "%-21s %s %d\n", "MEPID",":", p_db_oam_session->mepid);
    sal_fprintf(fp, "%-21s %s %d\n", "RMEPID",":", p_db_oam_session->rmepid);
    sal_fprintf(fp, "%-21s %s %s\n", "Session State", ":", 
        tpoam_api_signal_status_desc(p_db_oam_session->signal_status));
    sal_fprintf(fp, "%-21s %s %s\n", "Type", ":", 
        tpoam_api_service_type_for_display_desc(p_db_oam_session->oam_type, p_db_oam_session->pw_type, p_db_oam_session->mp_type));
    
    if (0 == p_db_oam_session->oam_type)
    {
    }
    else if (GLB_OAM_TYPE_MPLS_SECTION== p_db_oam_session->oam_type)
    {
        tbl_section_oam_t *pst_section = NULL;
        
        pst_section = tbl_section_oam_get_section_oam_by_ifindex(p_db_oam_session->service_ifindex);
        if (NULL != pst_section)
        {
            ifindex = pst_section->key.ifindex;
            sal_fprintf(fp, "%-21s %s %s\n", "PORT",":", tpoam_ifindex2namefull(ifname_ext, ifindex) ? ifname_ext : "");
        }
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == p_db_oam_session->oam_type && TPOAM_MP_TYPE_MEP == p_db_oam_session->mp_type)
    {
        tbl_lsp_pe_t *pst_lsp_pe = NULL;
        uint32 inlabel = 0;
        uint32 outlabel = 0;

        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(p_db_oam_session->service_name);
        if (NULL != pst_lsp_pe)
        {
            ifindex = pst_lsp_pe->ifindex;
            inlabel = pst_lsp_pe->inlabel;
            outlabel = pst_lsp_pe->outlabel;

            sal_fprintf(fp, "%-21s %s %d\n", "InLabel",":", inlabel);
            sal_fprintf(fp, "%-21s %s %d\n", "OutLabel",":", outlabel);
            sal_fprintf(fp, "%-21s %s %s\n", "PORT",":", tpoam_ifindex2namefull(ifname_ext, ifindex) ? ifname_ext : "");
        }
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == p_db_oam_session->oam_type && TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
    {
        tbl_lsp_p_t *pst_lsp_p = NULL;

        pst_lsp_p = tbl_lsp_p_get_lsp_p_by_name(p_db_oam_session->service_name);
        if (NULL != pst_lsp_p)
        {
            sal_fprintf(fp, "%-21s %s %d\n", "West InLabel",":", pst_lsp_p->west_inlabel);
            sal_fprintf(fp, "%-21s %s %d\n", "West OutLabel",":", pst_lsp_p->west_outlabel);
            sal_fprintf(fp, "%-21s %s %s\n", "West OutPort",":", tpoam_ifindex2namefull(ifname_ext, pst_lsp_p->west_ifindex) ? ifname_ext : "");

            sal_fprintf(fp, "%-21s %s %d\n", "East InLabel",":", pst_lsp_p->east_inlabel);
            sal_fprintf(fp, "%-21s %s %d\n", "East OutLabel",":", pst_lsp_p->east_outlabel);
            sal_fprintf(fp, "%-21s %s %s\n", "East OutPort",":", tpoam_ifindex2namefull(ifname_ext_2, pst_lsp_p->east_ifindex) ? ifname_ext_2: "");
        }
    }
    else if (GLB_OAM_TYPE_MPLS_PW == p_db_oam_session->oam_type)
    {
        tbl_pw_tpe_t * pst_tpe = NULL;
        tbl_lsp_pe_t *pst_lsp_pe = NULL;
        uint32 inlabel = 0;
        uint32 outlabel = 0;
        uint32 aps_group_id = 0;

        pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(p_db_oam_session->service_name);
        if (NULL != pst_tpe)
        {
            /*ifindex*/
            if (0 != sal_strlen(pst_tpe->lsp_name))
            {
                pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_tpe->lsp_name);
                if (NULL != pst_lsp_pe)
                {
                    ifindex = pst_lsp_pe->ifindex;
                    inlabel = pst_tpe->inlabel;
                    outlabel = pst_tpe->outlabel;

                    sal_fprintf(fp, "%-21s %s %d\n", "InLabel",":", inlabel);
                    sal_fprintf(fp, "%-21s %s %d\n", "OutLabel",":", outlabel);
                    sal_fprintf(fp, "%-21s %s %s\n", "PORT",":", tpoam_ifindex2namefull(ifname_ext, ifindex) ? ifname_ext : "");
                }
            }
            /*aps group id*/
            else
            {
                inlabel = pst_tpe->inlabel;
                outlabel = pst_tpe->outlabel;
                aps_group_id = pst_tpe->lsp_aps_group_id;

                sal_fprintf(fp, "%-21s %s %d\n", "InLabel",":", inlabel);
                sal_fprintf(fp, "%-21s %s %d\n", "OutLabel",":", outlabel);
                sal_fprintf(fp, "%-21s %s %d\n", "GroupID",":", aps_group_id);
            }
        }
    }

    if (p_db_oam_session->mp_type != TPOAM_MP_TYPE_MIP)
    {
        /*LM*/
        sal_fprintf(fp, "%s\n", "Dual LM:");
        sal_fprintf(fp, "    %-17s %s %s\n", "LM Enable", ":", 
            tpoam_api_status_desc(p_db_oam_session->lm_enable));
        local_threshold = tpoam_lm_get_local_threshold(p_db_oam_session);
        remote_threshold = tpoam_lm_get_remote_threshold(p_db_oam_session);
        sal_fprintf(fp, "    %-17s %s %.3f%%\n", "Local Threshold", ":", (float)local_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
        sal_fprintf(fp, "    %-17s %s %.3f%%\n", "Remote Threshold", ":", (float)remote_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
        sal_fprintf(fp, "    %-17s %s %ds\n",   "Stats Interval", ":", p_db_oam_session->oam_lm.lm_dual.stats_interval);

        /*DM*/
        sal_fprintf(fp, "%s\n", "DM:");
        sal_fprintf(fp, "    %-17s %s %s\n", "DM Enable", ":", 
            tpoam_api_status_desc(p_db_oam_session->dm_enable));
        delay_threshold = tpoam_get_delay_threshold(p_db_oam_session);
        sal_fprintf(fp, "    %-17s %s %uns\n", "Threshold", ":", delay_threshold);
        sal_fprintf(fp, "    %-17s %s %s\n", "DMM Tx interval", ":", 
            tpoam_desc_dm_interval_val2str(p_db_oam_session->oam_dm.para.interval));
    }
    
    /*Flow Info*/
    tbl_oam_flow_info_t *p_db_oam_flow_info = NULL;
    tbl_oam_flow_info_key_t oam_flow_info_key = {0};

    oam_flow_info_key.flow_id = p_db_oam_session->bind_oam_flow_id;
    p_db_oam_flow_info = tbl_oam_flow_info_get_oam_flow_info(&oam_flow_info_key);

    if (p_db_oam_flow_info)
    {
        sal_fprintf(fp, "%s\n", "Flow Info:");

        if (0 == p_db_oam_flow_info->inlabel_flow_id)
        {
            sal_fprintf(fp, "    %-17s %s %s\n", "InlabelFlowID", ":", "N/A");
        }
        else
        {
            sal_fprintf(fp, "    %-17s %s %u\n", "InlabelFlowID", ":", p_db_oam_flow_info->inlabel_flow_id);
        }

        if (0 == p_db_oam_flow_info->outlabel_flow_id)
        {
            sal_fprintf(fp, "    %-17s %s %s\n", "OutlabelFlowID", ":", "N/A");
        }
        else
        {
            sal_fprintf(fp, "    %-17s %s %u\n", "OutlabelFlowID", ":", p_db_oam_flow_info->outlabel_flow_id);
        }

        if (0 == p_db_oam_flow_info->outlabel_group_id)
        {
            sal_fprintf(fp, "    %-17s %s %s\n", "OutlabelGroupID", ":", "N/A");
        }
        else
        {
            sal_fprintf(fp, "    %-17s %s %u\n", "OutlabelGroupID", ":", p_db_oam_flow_info->outlabel_group_id);
        }
            
    }
    return PM_E_NONE;
}
#endif

void
tpoam_api_show_oam_dm(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_dm_t* pst_dm)
{
    struct list* pst_cache_list = NULL;
    struct listnode* pst_node = NULL;
    tpoam_rx1dm_t* pst_1dm = &pst_oam->oam_dm.rx1dm;
    tpoam_dm_entry_t* pst_entry = NULL;
    struct sal_tm* pst_time = NULL;
    uint32 index = 0;
    char delay_str[DELAY_STR_LEN];
    char variation_str[DELAY_STR_LEN];
    char avg_delay_str[DELAY_STR_LEN];
    char avg_variation_str[DELAY_STR_LEN];
    char max_delay_str[DELAY_STR_LEN];
    char min_delay_str[DELAY_STR_LEN];

    tpoam_interval_t avg_variation = 0;
    tpoam_interval_t avg_delay = 0;
    tpoam_interval_t max_delay = 0;
    tpoam_interval_t min_delay = 0;
    tpoam_interval_t variation = 0;
    tpoam_interval_t local_delay = 0;

    char service_str[TPOAM_SERVICE_STR_SIZE];
    tpoam_api_get_service_name(pst_oam, service_str, TPOAM_SERVICE_STR_SIZE);

    if (pst_dm)
    {
        pst_cache_list = pst_oam->oam_list.pst_dmm_cache_list;
        sal_fprintf(fp, " %s %s MEG %s MEP %d DM result: units(ns)\n", 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type), 
            service_str, pst_oam->megid_str, pst_oam->mepid);
    }
    else
    {
        pst_cache_list = pst_1dm->pst_rx1dm_cache_list;
        sal_fprintf(fp, " %s %s MEG %s MEP %d 1DM cache: size %d, used %d, units(ns)\n", 
            tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type), 
            service_str, pst_oam->megid_str, pst_oam->mepid, 
            pst_oam->oam_dm.rx1dm.max_count, LISTCOUNT(pst_oam->oam_dm.rx1dm.pst_rx1dm_cache_list));
    }
    
    if (0 == LISTCOUNT(pst_cache_list))
    {
        return;
    }

    sal_fprintf(fp, "%-3s %14s %14s %-3s %-19s\n", "Idx", "Delay", "Variation", "EXP", "Time");
    sal_fprintf(fp, "---+--------------+--------------+---+---------------------\n");

    LIST_LOOP(pst_cache_list, pst_entry, pst_node)
    {
        index++;
        if (1 == index)
        {
            variation = 0;
            min_delay = pst_entry->delay;
            max_delay = pst_entry->delay;
        }
        else
        {
            variation = MAX(pst_entry->delay, local_delay) - MIN(pst_entry->delay, local_delay);
        }
        local_delay = pst_entry->delay;
        avg_delay += pst_entry->delay;
        avg_variation += variation;
        if (pst_entry->delay > max_delay)
        {
            max_delay = pst_entry->delay;
        }
        if (pst_entry->delay < min_delay)
        {
            min_delay = pst_entry->delay;
        }
        pst_time = sal_localtime(&pst_entry->time);
        TPOAM_TIMEINTERVAL_TO_STR(delay_str, local_delay);
        TPOAM_TIMEINTERVAL_TO_STR(variation_str, variation);
        sal_fprintf(fp, "%-3d %14s %14s %-3d %04d/%02d/%02d %02d:%02d:%02d\n", 
                index,
                delay_str,
                variation_str,
                pst_entry->exp,
                pst_time->tm_year + 1900, 
                pst_time->tm_mon + 1, 
                pst_time->tm_mday, 
                pst_time->tm_hour, 
                pst_time->tm_min, 
                pst_time->tm_sec);
    }

    avg_delay = avg_delay/index;
    avg_variation = (index > 1) ? avg_variation/(index - 1) : 0;
    TPOAM_TIMEINTERVAL_TO_STR(avg_delay_str, avg_delay);
    TPOAM_TIMEINTERVAL_TO_STR(avg_variation_str, avg_variation);
    TPOAM_TIMEINTERVAL_TO_STR(max_delay_str, max_delay);
    TPOAM_TIMEINTERVAL_TO_STR(min_delay_str, min_delay);

    sal_fprintf(fp, "%14s %14s %14s %14s\n", "Average Delay", "Average Var", "Max Delay", "Min Delay");
    sal_fprintf(fp, "--------------+--------------+--------------+--------------\n");
    sal_fprintf(fp, "%14s %14s %14s %14s\n", 
                avg_delay_str, avg_variation_str, max_delay_str, min_delay_str);
    
    sal_fprintf(fp, "\n");

}

#ifndef show_dmm
static void
_tpoam_api_show_oam_dmm_session_title(FILE *fp, tbl_oam_session_t* pst_oam, uint32 delay_threshold)
{
    sal_fprintf(fp, "%16s %u\n", "Session ID     :", pst_oam->key.session_id);
    sal_fprintf(fp, "%16s %s\n", "MEGID          :", pst_oam->megid_str);
    sal_fprintf(fp, "%16s %d\n",   "MEPID          :", pst_oam->mepid);
    sal_fprintf(fp, "%16s %s\n",   "Type           :", 
        tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type));
    sal_fprintf(fp, "%16s %s\n",   "DM Enable      :", 
        tpoam_api_status_desc(pst_oam->dm_enable));
    sal_fprintf(fp, "%16s %s\n",   "Session State  :", 
        tpoam_api_signal_status_desc(pst_oam->signal_status));
    sal_fprintf(fp, "%16s %uns\n",   "Threshold      :", delay_threshold);
    sal_fprintf(fp, "%16s %s\n",   "DMM Tx interval:", 
        tpoam_desc_dm_interval_val2str(pst_oam->oam_dm.para.interval));

    return;
}
static void
_tpoam_api_show_oam_dmm_session(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t *bb, uint32 *mip_session_count)
{
    struct listnode* pst_node = NULL;
    tpoam_list_t         *oam_list = &pst_oam->oam_list;
    tpoam_dm_entry_t* pst_entry = NULL;
    struct sal_tm* pst_time = NULL;
    uint32 index = 0;

    tpoam_interval_t min_delay_tmp = 0;
    tpoam_interval_t max_delay_tmp = 0;
    tpoam_interval_t delay_tmp = 0;
    tpoam_interval_t variation_tmp = 0;
    tpoam_interval_t avg_delay_tmp = 0;
    tpoam_interval_t avg_variation_tmp = 0;
    
    char avg_delay_str[DELAY_STR_LEN];
    char avg_variation_str[DELAY_STR_LEN];
    char max_delay_str[DELAY_STR_LEN];
    char min_delay_str[DELAY_STR_LEN];

    tpoam_interval_t avg_variation = 0;
    tpoam_interval_t avg_delay = 0;
    tpoam_interval_t max_delay = 0;
    tpoam_interval_t min_delay = 0;
    //tpoam_interval_t variation = 0;
    //tpoam_interval_t local_delay = 0;
    uint32 delay_threshold = 0;
    char buf[5] = {0};

    uint32 has_title_showed = FALSE;

    delay_threshold = tpoam_get_delay_threshold(pst_oam);

    if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
    {
        (*mip_session_count)++;
        return;
    }
    
    _tpoam_api_show_oam_dmm_session_title(fp, pst_oam, delay_threshold);
    
    LIST_LOOP(oam_list->pst_dmm_cache_list, pst_entry, pst_node)
    {
        if (!has_title_showed)
        {
            has_title_showed = TRUE;

            sal_fprintf(fp, "Codes: * - delay exceed the configured threshold\n");
            #ifdef dm_ns_to_us
                sal_fprintf(fp, " %-3s %13s %-13s %-19s\n", 
                    "Idx", "Delay(us)", "Variation(us)", "Time");
            #else
                sal_fprintf(fp, " %-3s %13s %-13s %-19s\n", 
                    "Idx", "Delay(ns)", "Variation(ns)", "Time");
            #endif
            sal_fprintf(fp, "----+-------------+-------------+-------------------------------\n");
        }
        
        /*ns to us*/
        #ifdef dm_ns_to_us
            delay_tmp = pst_entry->delay/1000;
            variation_tmp = pst_entry->variation/1000;
        #else
            delay_tmp = pst_entry->delay;
            variation_tmp = pst_entry->variation;
        #endif
        
        index++;
        pst_time = sal_localtime(&pst_entry->time);
        
        if (delay_threshold != 0 && delay_threshold <= delay_tmp)
        {
            sal_sprintf(buf, "%d*", index);
            sal_fprintf(fp, "%-4s %13"PRId64" %13"PRId64" %04d/%02d/%02d %02d:%02d:%02d\n", 
                buf,
                delay_tmp,
                variation_tmp,
                pst_time->tm_year + 1900, 
                pst_time->tm_mon + 1, 
                pst_time->tm_mday, 
                pst_time->tm_hour, 
                pst_time->tm_min, 
                pst_time->tm_sec);
        }
        else
        {
            sal_fprintf(fp, "%-4d %13"PRId64" %13"PRId64" %04d/%02d/%02d %02d:%02d:%02d\n", 
                index,
                delay_tmp,
                variation_tmp,
                pst_time->tm_year + 1900, 
                pst_time->tm_mon + 1, 
                pst_time->tm_mday, 
                pst_time->tm_hour, 
                pst_time->tm_min, 
                pst_time->tm_sec);
        }

        /*calc max min average*/
        if (1 == index)
        {
            //variation = 0;
            min_delay = pst_entry->delay;
            max_delay = pst_entry->delay;
        }
        else
        {
            //variation = MAX(pst_entry->delay, local_delay) - MIN(pst_entry->delay, local_delay);
        }

        //local_delay = pst_entry->delay;
        avg_delay += pst_entry->delay;
        //avg_variation += variation;
        avg_variation += pst_entry->variation;
        if (pst_entry->delay > max_delay)
        {
            max_delay = pst_entry->delay;
        }
        if (pst_entry->delay < min_delay)
        {
            min_delay = pst_entry->delay;
        }
    }

    if (has_title_showed)
    {
        avg_delay = (index > 0) ? avg_delay/index : 0;
        avg_variation = (index > 1) ? avg_variation/(index - 1) : 0;

        /*ns to us*/
        #ifdef dm_ns_to_us
            avg_delay_tmp = avg_delay/1000;
            avg_variation_tmp = avg_variation/1000;
            max_delay_tmp = max_delay/1000;
            min_delay_tmp = min_delay/1000;
        #else
            avg_delay_tmp = avg_delay;
            avg_variation_tmp = avg_variation;
            max_delay_tmp = max_delay;
            min_delay_tmp = min_delay;
        #endif
            
        TPOAM_TIMEINTERVAL_TO_STR(avg_delay_str, avg_delay_tmp);
        TPOAM_TIMEINTERVAL_TO_STR(avg_variation_str, avg_variation_tmp);
        TPOAM_TIMEINTERVAL_TO_STR(max_delay_str, max_delay_tmp);
        TPOAM_TIMEINTERVAL_TO_STR(min_delay_str, min_delay_tmp);

        #ifdef dm_ns_to_us
            sal_fprintf(fp, "%17s %15s %14s %14s\n", "Average Delay(us)", "Average Var(us)", "Max Delay(us)", "Min Delay(us)");
        #else
            sal_fprintf(fp, "%17s %15s %14s %14s\n", "Average Delay(ns)", "Average Var(ns)", "Max Delay(ns)", "Min Delay(ns)");
        #endif
        sal_fprintf(fp, "-----------------+---------------+--------------+--------------\n");
        sal_fprintf(fp, "%17s %15s %14s %14s\n",
                    avg_delay_str, avg_variation_str, max_delay_str, min_delay_str);
        
        //sal_fprintf(fp, "\n");
    }

    return;
}

static int
_tpoam_api_show_dmm_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32 *mip_session_count = (uint32*)pargs->argv[1];
    uint32 tmp = (*mip_session_count);
    
    _tpoam_api_show_oam_dmm_session(fp, pst_oam, NULL, mip_session_count);
    if (tmp == (*mip_session_count))
    {
        sal_fprintf(fp, "\n");
    }
        
    return PM_E_NONE;
}

int32
tpoam_api_show_dmm_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para)
{
    tbl_oam_session_master_t *p_master = tbl_oam_session_get_master();
    tbl_oam_session_t *p_db_oam_session = NULL;
    uint32 mip_session_count = 0;
    
    if (!session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = &mip_session_count;
        tbl_oam_session_iterate(_tpoam_api_show_dmm_session_interator, &args);
        if (mip_session_count == p_master->oam_session_list->count)
        {
            CFG_INVALID_PARAM_RET("The command is denied because all tpoam sessions are MIP");
        }
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            if (TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
            {
                CFG_INVALID_PARAM_RET("Tpoam session %u is MIP", session_id);
            }
            _tpoam_api_show_oam_dmm_session(fp, p_db_oam_session, NULL, &mip_session_count);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    }        
    
    
    return PM_E_NONE;
}
#endif

#ifndef show_1dm
static void
_tpoam_api_show_oam_1dm(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t* bb)
{
    /* 1DM only support on MEP */
    if (0 == pst_oam->mepid)
    {
        return;
    }
    
    tpoam_api_show_oam_dm(fp, pst_oam, NULL);
    return;
}

int32
tpoam_api_show_1dm(FILE *fp, uint32 type, tpoam_pw_type_t pw_type, char* pch_service_name)
{
    tbl_section_oam_t* pst_section = NULL;
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;
    int32 rc = PM_E_NONE;

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (GLB_OAM_TYPE_MPLS_SECTION == type || 0 == type)
    {
        if (!pch_service_name)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_1dm;
            tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
        }
        else
        {
            rc = tpoam_api_get_section(pch_service_name, &pst_section);
            if (rc < 0)
            {
                return rc;
            }
            if (!pst_section->oam_session_id)
            {
                return TPOAM_E_SERVICE_NON_OAM;
            }
            else
            {
                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
                if (NULL != p_db_oam_session)
                {
                    _tpoam_api_show_oam_1dm(fp, p_db_oam_session, NULL);
                }
            }
        }
    }
    
    if (GLB_OAM_TYPE_MPLS_LSP == type || 0 == type)
    {
        if (!pch_service_name)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_1dm;
            tbl_lsp_pe_iterate(_tpoam_api_show_lsp_pe_param2_interator, &args);
            tbl_lsp_p_iterate(_tpoam_api_show_lsp_p_param2_interator, &args);
        }
        else
        {
            rc = tpoam_api_get_lsp_pe(pch_service_name, &pst_lsp_pe);
            if (rc < 0)
            {
                rc = tpoam_api_get_lsp_p(pch_service_name, &pst_lsp_p);
                if (rc < 0)
                {
                    return rc;
                }
                else
                {
                    if (!pst_lsp_p->oam_session_id)
                    {
                        return TPOAM_E_SERVICE_NON_OAM;
                    }
                    else
                    {
                        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                        if (NULL != p_db_oam_session)
                        {
                            _tpoam_api_show_oam_1dm(fp, p_db_oam_session, NULL);
                        }
                    }
                }
            }
            else
            {
                if (!pst_lsp_pe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                else
                {
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        _tpoam_api_show_oam_1dm(fp, p_db_oam_session, NULL);
                    }
                }
            }
        }
    }
    
    if (GLB_OAM_TYPE_MPLS_PW == type || 0 == type)
    {
        if (!pch_service_name)
        {
            if (TPOAM_PW_TYPE_TPE == pw_type || TPOAM_PW_TYPE_MAX == pw_type)
            {
                tbl_iter_args_t args;
                args.argv[0] = fp;
                args.argv[1] = _tpoam_api_show_oam_1dm;
                tbl_lsp_pe_iterate(_tpoam_api_show_pw_tpe_param2_interator, &args);
            }
        }
        else
        {
            if (TPOAM_PW_TYPE_TPE == pw_type)
            {
                rc = tpoam_api_get_pw_tpe(pch_service_name, &pst_tpe);
                if (rc < 0)
                {
                    return rc;
                }
                if (!pst_tpe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                else
                {
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        _tpoam_api_show_oam_1dm(fp, p_db_oam_session, NULL);
                    }
                }
            }
        }
    }
    
    return PM_E_NONE;
}


#endif
#ifndef show_lm
static void
_tpoam_api_show_oam_lm_dual_title(FILE *fp, tbl_oam_session_t* pst_oam, uint32 local_threshold, uint32 remote_threshold)
{
    sal_fprintf(fp, "%17s %u\n", "Session ID      :", pst_oam->key.session_id);
    sal_fprintf(fp, "%17s %s\n", "MEGID           :", pst_oam->megid_str);
    sal_fprintf(fp, "%17s %d\n", "MEPID           :", pst_oam->mepid);
    sal_fprintf(fp, "%17s %s\n", "Type            :", 
        tpoam_api_service_type_for_display_desc(pst_oam->oam_type, pst_oam->pw_type, pst_oam->mp_type));
    sal_fprintf(fp, "%17s %s\n", "LM Enable       :", 
        tpoam_api_status_desc(pst_oam->lm_enable));
    sal_fprintf(fp, "%17s %s\n", "Session State   :", 
        tpoam_api_signal_status_desc(pst_oam->signal_status));
    sal_fprintf(fp, "%17s %.3f%%\n", "Local Threshold :", (float)local_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
    sal_fprintf(fp, "%17s %.3f%%\n", "Remote Threshold:", (float)remote_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
    sal_fprintf(fp, "%17s %ds\n",   "Stats Interval  :", pst_oam->oam_lm.lm_dual.stats_interval);

    return;
}

static void
_tpoam_api_show_oam_lm_dual(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t* lm_dual_inputs, uint32 *mip_session_count)
{
    struct sal_tm  *time = NULL;
    tpoam_dual_lm_t *lm_dual = NULL;
    struct listnode *node = NULL;
    struct listnode *node_head = NULL;
    struct listnode *node_tail = NULL;
    tpoam_dual_lm_entry_t *lm_dual_entry = NULL;
    tpoam_dual_lm_entry_t *lm_dual_entry_head = NULL;
    tpoam_dual_lm_entry_t *lm_dual_entry_tail = NULL;
    u_int16_t index = 0;
    bool first_node = TRUE;
    char buf[16] = {0};
    char index_buf[5] = {0};

    uint32 local_threshold = 0;
    uint32 remote_threshold = 0;
    uint32 has_title_showed = FALSE;

    u_int32_t max_lm_far_end_frame_loss = 0; 
    u_int32_t max_lm_far_end_frame_loss_ratio = 0; 
    u_int32_t max_lm_near_end_frame_loss = 0;
    u_int32_t max_lm_near_end_frame_loss_ratio = 0; 

    u_int32_t min_lm_far_end_frame_loss = 0; 
    u_int32_t min_lm_far_end_frame_loss_ratio = 0; 
    u_int32_t min_lm_near_end_frame_loss = 0;
    u_int32_t min_lm_near_end_frame_loss_ratio = 0; 

    if (!pst_oam || !pst_oam->mepid)
    {
        return;
    }

    if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
    {
        (*mip_session_count)++;
        return;
    }

    local_threshold = tpoam_lm_get_local_threshold(pst_oam);
    remote_threshold = tpoam_lm_get_remote_threshold(pst_oam);
    _tpoam_api_show_oam_lm_dual_title(fp, pst_oam, local_threshold, remote_threshold);

    /* check mep enable */
    if ((TPOAM_LM_TYPE_DUAL != pst_oam->oam_lm.lm_type) /*|| (pst_oam->oam_lm.lm_dual == NULL)*/)
    {
        return;
    }
    
    lm_dual = &(pst_oam->oam_lm.lm_dual);

    if (NULL == pst_oam->oam_list.pst_dual_lm_cache_list)
    {
        return;
    }
    
    node_head = pst_oam->oam_list.pst_dual_lm_cache_list->head;
    if (node_head == NULL)
    {
        return;
    }

    LIST_LOOP (pst_oam->oam_list.pst_dual_lm_cache_list, lm_dual_entry, node)
    {
        if ((TPOAM_LM_COS_TYPE_SPECIFIED_COS == lm_dual_inputs->cos_type)
            && ((lm_dual_inputs->cos_value != lm_dual_entry->ccm_cos_value)
            || (TPOAM_LM_COS_TYPE_ALL == pst_oam->oam_lm.lm_dual.cos_type)))
        {
            continue;
        }

        if (first_node)
        {
            first_node = FALSE;
            continue;
        }

        index++;
        if (lm_dual_entry->signal_status == TPOAM_STATUS_FAIL)
        {
            continue;
        }

        if (!has_title_showed)
        {
            has_title_showed = TRUE;
            
            /* print start time */
            lm_dual_entry_head = node_head->data;

            time = sal_localtime(&lm_dual_entry_head->recv_time); 
            sal_fprintf(fp, "%17s %d/%02d/%02d %d:%02d:%02d\n", "Start Time      :", time->tm_year + 1900, time->tm_mon + 1, 
                time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);

            /* print end time */
            node_tail = pst_oam->oam_list.pst_dual_lm_cache_list->tail;
            if (node_tail == NULL)
            {
                return;
            }
            else
            {
                lm_dual_entry_tail = node_tail->data;
            
                time = sal_localtime(&lm_dual_entry_tail->recv_time); 
                sal_fprintf(fp, "%17s %d/%02d/%02d %d:%02d:%02d\n", "End Time        :", time->tm_year + 1900, time->tm_mon + 1, 
                    time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
            }

            sal_fprintf(fp, "Codes: * - loss ratio exceed the configured value\n");
            sal_fprintf(fp, "Latest dual-ended loss statistics:\n");

            sal_fprintf(fp, "------------------------------------------------------------------------\n");

            sal_fprintf(fp, "%-4s %3s %10s %19s %11s %20s\n", 
                "Idx", "Exp", "Local-loss", "Local-loss-ratio(%)", "Remote-loss", "Remote-loss-ratio(%)");

            sal_fprintf(fp, "----+---+----------+-------------------+-----------+--------------------\n");
        }

        /*modified by chenc for bug 51665, 2019-04-15*/
        if (lm_dual_entry->lm_far_end_frame_loss_ratio > remote_threshold * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10 ||
            lm_dual_entry->lm_near_end_frame_loss_ratio > local_threshold * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10)
        /*
        if ((remote_threshold != 0 && 
            lm_dual_entry->lm_far_end_frame_loss_ratio >= remote_threshold * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10) ||
            (local_threshold != 0 &&
            lm_dual_entry->lm_near_end_frame_loss_ratio >= local_threshold * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10))*/
        {
            sal_sprintf(index_buf, "%d*", index);
            sal_fprintf(fp, "%-4s ", index_buf);
        }
        else
        {
            sal_fprintf(fp, "%-4d ", index);
        }

        if (TPOAM_LM_COS_TYPE_ALL == lm_dual->cos_type)
        {
            sal_fprintf(fp, "%3s ", "all");
        }
        else
        {
            sal_fprintf(fp, "%3d ", lm_dual_entry->ccm_cos_value);
        }

        sal_fprintf(fp, "%10u ", lm_dual_entry->lm_near_end_frame_loss);
        
        sal_fprintf(fp, "            %02d.%04d  ", 
            lm_dual_entry->lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            lm_dual_entry->lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

        sal_sprintf(buf, "%8u ", lm_dual_entry->lm_far_end_frame_loss);
        sal_fprintf(fp, "%11s ", buf);
        
        sal_fprintf(fp, "            %02d.%04d\n", 
            lm_dual_entry->lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            lm_dual_entry->lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

        if (1 == index)
        {
            max_lm_far_end_frame_loss = lm_dual_entry->lm_far_end_frame_loss; 
            min_lm_far_end_frame_loss = lm_dual_entry->lm_far_end_frame_loss; 
            
            max_lm_far_end_frame_loss_ratio = lm_dual_entry->lm_far_end_frame_loss_ratio; 
            min_lm_far_end_frame_loss_ratio = lm_dual_entry->lm_far_end_frame_loss_ratio; 

            max_lm_near_end_frame_loss = lm_dual_entry->lm_near_end_frame_loss;
            min_lm_near_end_frame_loss = lm_dual_entry->lm_near_end_frame_loss;

            max_lm_near_end_frame_loss_ratio = lm_dual_entry->lm_near_end_frame_loss_ratio; 
            min_lm_near_end_frame_loss_ratio = lm_dual_entry->lm_near_end_frame_loss_ratio; 
        }
        else
        {
            if (max_lm_far_end_frame_loss < lm_dual_entry->lm_far_end_frame_loss)
            {
                max_lm_far_end_frame_loss = lm_dual_entry->lm_far_end_frame_loss; 
            }

            if (min_lm_far_end_frame_loss > lm_dual_entry->lm_far_end_frame_loss)
            {
                min_lm_far_end_frame_loss = lm_dual_entry->lm_far_end_frame_loss; 
            }

            if (max_lm_far_end_frame_loss_ratio < lm_dual_entry->lm_far_end_frame_loss_ratio)
            {
                max_lm_far_end_frame_loss_ratio = lm_dual_entry->lm_far_end_frame_loss_ratio; 
            }

            if (min_lm_far_end_frame_loss_ratio > lm_dual_entry->lm_far_end_frame_loss_ratio)
            {
                min_lm_far_end_frame_loss_ratio = lm_dual_entry->lm_far_end_frame_loss_ratio; 
            }

            if (max_lm_near_end_frame_loss < lm_dual_entry->lm_near_end_frame_loss)
            {
                max_lm_near_end_frame_loss = lm_dual_entry->lm_near_end_frame_loss; 
            }

            if (min_lm_near_end_frame_loss > lm_dual_entry->lm_near_end_frame_loss)
            {
                min_lm_near_end_frame_loss = lm_dual_entry->lm_near_end_frame_loss; 
            }

            if (max_lm_near_end_frame_loss_ratio < lm_dual_entry->lm_near_end_frame_loss_ratio)
            {
                max_lm_near_end_frame_loss_ratio = lm_dual_entry->lm_near_end_frame_loss_ratio; 
            }

            if (min_lm_near_end_frame_loss_ratio > lm_dual_entry->lm_near_end_frame_loss_ratio)
            {
                min_lm_near_end_frame_loss_ratio = lm_dual_entry->lm_near_end_frame_loss_ratio; 
            }
        }
    }

    if (has_title_showed)
    {
        sal_fprintf(fp, "------------------------------------------------------------------------\n");

        sal_fprintf(fp, "Maximum Local-loss  : %-10u ", max_lm_near_end_frame_loss);
        sal_fprintf(fp, "Maximum Local-loss-ratio  : %02d.%04d%%\n", 
                max_lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                max_lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);       

        sal_fprintf(fp, "Minimum Local-loss  : %-10u ", min_lm_near_end_frame_loss);
        sal_fprintf(fp, "Minimum Local-loss-ratio  : %02d.%04d%%\n", 
                min_lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                min_lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);        

        sal_fprintf(fp, "Maximum Remote-loss : %-10u ", max_lm_far_end_frame_loss);
        sal_fprintf(fp, "Maximum Remote-loss-ratio : %02d.%04d%%\n", 
                max_lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                max_lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

        sal_fprintf(fp, "Minimum Remote-loss : %-10u ", min_lm_far_end_frame_loss);
        sal_fprintf(fp, "Minimum Remote-loss-ratio : %02d.%04d%%\n", 
                min_lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                min_lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

        //sal_fprintf(fp, "\n");
    }
}


void
_tpoam_api_show_oam_lm_dual_upload(FILE *fp, tbl_oam_session_upload_t* pst_oam_upload, tpoam_lm_dual_enable_input_t* lm_dual_inputs, uint32 *has_title_showed, uint32 *mip_session_count)
{
    char buf[16] = {0};
    char index_buf[5] = {0};
    uint32 local_threshold = 0;
    uint32 remote_threshold = 0;
    
    tbl_oam_session_t* pst_oam = NULL;

    pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_oam_upload->key.session_id);
    if (!pst_oam || !pst_oam->mepid)
    {
        return;
    }
    
    if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
    {
        (*mip_session_count)++;
        return;
    }

    local_threshold = tpoam_lm_get_local_threshold(pst_oam);
    remote_threshold = tpoam_lm_get_remote_threshold(pst_oam);
    //_tpoam_api_show_oam_lm_dual_title(fp, pst_oam, local_threshold, remote_threshold);

    /* check mep enable */
    if ((TPOAM_LM_TYPE_DUAL != pst_oam->oam_lm.lm_type) /*|| (pst_oam->oam_lm.lm_dual == NULL)*/)
    {
        return;
    }

    if (NULL == pst_oam->oam_list.pst_dual_lm_cache_list)
    {
        return;
    }

    //LIST_LOOP (pst_oam->oam_list.pst_dual_lm_cache_list, lm_dual_entry, node)
    {
        //if ((TPOAM_LM_COS_TYPE_SPECIFIED_COS == lm_dual_inputs->cos_type)
        //    && ((lm_dual_inputs->cos_value != lm_dual_entry->ccm_cos_value)
        //    || (TPOAM_LM_COS_TYPE_ALL == pst_oam->oam_lm.lm_dual.cos_type)))
        {
            //continue;
        }

        //index++;
        //if (lm_dual_entry->signal_status == TPOAM_STATUS_FAIL)
        {
            //continue;
        }

        if (!*has_title_showed)
        {
            *has_title_showed = TRUE;
            
            sal_fprintf(fp, "Codes: * - loss ratio exceed the configured value\n");
            sal_fprintf(fp, "Latest dual-ended loss statistics:\n");

            sal_fprintf(fp, "------------------------------------------------------------------------\n");

            //sal_fprintf(fp, "%-7s %3s %10s %19s %11s %20s\n", 
            //    "SessID", "Exp", "Local-loss", "Local-loss-ratio(%)", "Remote-loss", "Remote-loss-ratio(%)");

            //sal_fprintf(fp, "-------+---+----------+-------------------+-----------+--------------------\n");

            sal_fprintf(fp, "%-7s %10s %19s %11s %20s\n", 
                "SessID", "Local-loss", "Local-loss-ratio(%)", "Remote-loss", "Remote-loss-ratio(%)");

            sal_fprintf(fp, "-------+----------+-------------------+-----------+---------------------\n");
        }

        if ((remote_threshold != 0 && 
            pst_oam_upload->upload_lm.lm_far_end_frame_loss_ratio >= remote_threshold * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10) ||
            (local_threshold != 0 &&
            pst_oam_upload->upload_lm.lm_near_end_frame_loss_ratio >= local_threshold * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10))
        {
            sal_sprintf(index_buf, "%d*",  pst_oam_upload->key.session_id);
            sal_fprintf(fp, "%-7s ", index_buf);
        }
        else
        {
            sal_fprintf(fp, "%-7d ", pst_oam_upload->key.session_id);
        }

        //if (TPOAM_LM_COS_TYPE_ALL == lm_dual->cos_type)
        {
            //sal_fprintf(fp, "%3s ", "all");
        }
        //else
        {
            //sal_fprintf(fp, "%3d ", lm_dual_entry->ccm_cos_value);
        }

        sal_fprintf(fp, "%10u ", pst_oam_upload->upload_lm.lm_near_end_frame_loss);
        
        sal_fprintf(fp, "            %02d.%04d  ", 
            pst_oam_upload->upload_lm.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            pst_oam_upload->upload_lm.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

        sal_sprintf(buf, "%8u ", pst_oam_upload->upload_lm.lm_far_end_frame_loss);
        sal_fprintf(fp, "%11s ", buf);
        
        sal_fprintf(fp, "            %02d.%04d\n", 
            pst_oam_upload->upload_lm.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            pst_oam_upload->upload_lm.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
    }

    return;
}


int32
tpoam_api_show_lm_dual(FILE *fp, uint32 type, tpoam_pw_type_t pw_type, 
        char* pch_service_name, tpoam_lm_dual_enable_input_t* lm_dual_inputs)
{
    tbl_section_oam_t* pst_section = NULL;
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;
    int32 rc = PM_E_NONE;
    uint32 mip_session_count = 0;

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (GLB_OAM_TYPE_MPLS_SECTION == type || 0 == type)
    {
        if (!pch_service_name)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_lm_dual;
            args.argv[2] = lm_dual_inputs;
            tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
        }
        else
        {
            rc = tpoam_api_get_section(pch_service_name, &pst_section);
            if (rc < 0)
            {
                return rc;
            }
            if (!pst_section->oam_session_id)
            {
                return TPOAM_E_SERVICE_NON_OAM;
            }
            else
            {
                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
                if (NULL != p_db_oam_session)
                {
                    _tpoam_api_show_oam_lm_dual(fp, p_db_oam_session, lm_dual_inputs, &mip_session_count);
                }
            }
        }        
    }
    
    if (GLB_OAM_TYPE_MPLS_LSP == type || 0 == type)
    {
        if (!pch_service_name)
        {
            tbl_iter_args_t args;
            args.argv[0] = fp;
            args.argv[1] = _tpoam_api_show_oam_lm_dual;
            args.argv[2] = lm_dual_inputs;
            tbl_lsp_pe_iterate(_tpoam_api_show_lsp_pe_param2_interator, &args);
            tbl_lsp_p_iterate(_tpoam_api_show_lsp_p_param2_interator, &args);
        }
        else
        {
            rc = tpoam_api_get_lsp_pe(pch_service_name, &pst_lsp_pe);
            if (rc < 0)
            {
                rc = tpoam_api_get_lsp_p(pch_service_name, &pst_lsp_p);
                if (rc < 0)
                {
                    return rc;
                }
                else
                {
                    if (!pst_lsp_p->oam_session_id)
                    {
                        return TPOAM_E_SERVICE_NON_OAM;
                    }
                    else
                    {
                        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                        if (NULL != p_db_oam_session)
                        {
                            _tpoam_api_show_oam_lm_dual(fp, p_db_oam_session, lm_dual_inputs, &mip_session_count);
                        }
                    }
                }
            }
            if (!pst_lsp_pe->oam_session_id)
            {
                return TPOAM_E_SERVICE_NON_OAM;
            }
            else
            {
                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
                if (NULL != p_db_oam_session)
                {
                    _tpoam_api_show_oam_lm_dual(fp, p_db_oam_session, lm_dual_inputs, &mip_session_count);
                }
            }
        }
    }
    
    if (GLB_OAM_TYPE_MPLS_PW == type || 0 == type)
    {
        if (!pch_service_name)
        {
            if (TPOAM_PW_TYPE_TPE == pw_type || TPOAM_PW_TYPE_MAX == pw_type)
            {
                tbl_iter_args_t args;
                args.argv[0] = fp;
                args.argv[1] = _tpoam_api_show_oam_lm_dual;
                args.argv[2] = lm_dual_inputs;
                tbl_pw_tpe_iterate(_tpoam_api_show_pw_tpe_param2_interator, &args);
            }
        }
        else
        {
            if (TPOAM_PW_TYPE_TPE == pw_type)
            {
                rc = tpoam_api_get_pw_tpe(pch_service_name, &pst_tpe);
                if (rc < 0)
                {
                    return rc;
                }
                if (!pst_tpe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                else
                {
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        _tpoam_api_show_oam_lm_dual(fp, p_db_oam_session, lm_dual_inputs, &mip_session_count);
                    }
                }
            }
        }
    }
    return PM_E_NONE;
}

static int
_tpoam_api_show_lm_dual_session_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32 *mip_session_count = (uint32*)pargs->argv[2];
    uint32 tmp = (*mip_session_count);
    
    _tpoam_api_show_oam_lm_dual(fp, pst_oam, (tpoam_lm_dual_enable_input_t *)(pargs->argv[1]), mip_session_count);
    if (tmp == (*mip_session_count))
    {
        sal_fprintf(fp, "\n");
    }
    return PM_E_NONE;
}

static int
_tpoam_api_show_lm_dual_session_upload_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_upload_t* pst_oam = obj;
    FILE *fp = pargs->argv[0];
    uint32 *mip_session_count = (uint32*)pargs->argv[2];
    uint32 *has_title_showed = (uint32*)pargs->argv[3];

    _tpoam_api_show_oam_lm_dual_upload(fp, pst_oam, (tpoam_lm_dual_enable_input_t *)(pargs->argv[1]), has_title_showed, mip_session_count);
    return PM_E_NONE;
}

int32
tpoam_api_show_lm_dual_session(FILE *fp, uint32 session_id, tpoam_lm_dual_enable_input_t* lm_dual_inputs, cfg_cmd_para_t *para)
{
    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_master_t *p_master = tbl_oam_session_get_master();
    uint32 mip_session_count = 0;
    
    if (!session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = lm_dual_inputs;
        args.argv[2] = &mip_session_count;
        tbl_oam_session_iterate(_tpoam_api_show_lm_dual_session_interator, &args);
        if (mip_session_count == p_master->oam_session_list->count)
        {
            CFG_INVALID_PARAM_RET("The command is denied because all tpoam sessions are MIP");
        }
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            if (TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
            {
                CFG_INVALID_PARAM_RET("Tpoam session %u is MIP", session_id);
            }
            _tpoam_api_show_oam_lm_dual(fp, p_db_oam_session, lm_dual_inputs, &mip_session_count);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    }        
    
    return PM_E_NONE;
}

int32
tpoam_api_show_lm_dual_upload_session(FILE *fp, uint32 session_id, tpoam_lm_dual_enable_input_t* lm_dual_inputs, cfg_cmd_para_t *para)
{
    tbl_oam_session_upload_master_t *p_master = tbl_oam_session_upload_get_master();
    tbl_oam_session_upload_t *p_db_oam_session_upload = NULL;
    uint32 mip_session_count = 0;
    uint32 has_title_showed = FALSE;
    
    if (!session_id)
    {
        tbl_iter_args_t args;
        args.argv[0] = fp;
        args.argv[1] = lm_dual_inputs;
        args.argv[2] = &mip_session_count;
        args.argv[3] = &has_title_showed;
        tbl_oam_session_upload_iterate(_tpoam_api_show_lm_dual_session_upload_interator, &args);
        if (mip_session_count == p_master->oam_session_list->count)
        {
            CFG_INVALID_PARAM_RET("The command is denied because all tpoam sessions are MIP");    
        }
    }
    else
    {
        p_db_oam_session_upload = tbl_oam_session_upload_get_oam_session_upload_by_session_id(session_id);
        if (NULL != p_db_oam_session_upload)
        {
            tbl_oam_session_t* pst_oam = NULL;

            pst_oam = tbl_oam_session_get_oam_session_by_session_id(session_id);
            if (!pst_oam || !pst_oam->mepid)
            {
                CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
            }
            
            if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
            {
                CFG_INVALID_PARAM_RET("Tpoam session %u is MIP", session_id);
            }
            
            _tpoam_api_show_oam_lm_dual_upload(fp, p_db_oam_session_upload, lm_dual_inputs, &has_title_showed, &mip_session_count);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Tpoam session %u does not exist", session_id);
        }
    }        
    
    return PM_E_NONE;
}
#endif
#ifndef show_single_lm
void
tpoam_api_show_oam_lm_single(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_single_lm_t* pst_lm_single)
{
    struct sal_tm  *time = NULL;
    struct listnode *node = NULL;
    struct listnode *next_node = NULL;
    tpoam_single_lm_entry_t *lm_single_entry = NULL;
    u_int16_t index = 0;
    bool first_node = TRUE;

    u_int32_t max_lm_far_end_frame_loss = 0; 
    u_int32_t max_lm_far_end_frame_loss_ratio = 0; 
    u_int32_t max_lm_near_end_frame_loss = 0;
    u_int32_t max_lm_near_end_frame_loss_ratio = 0; 

    u_int32_t min_lm_far_end_frame_loss = 0; 
    u_int32_t min_lm_far_end_frame_loss_ratio = 0; 
    u_int32_t min_lm_near_end_frame_loss = 0;
    u_int32_t min_lm_near_end_frame_loss_ratio = 0; 

    node = pst_lm_single->lm_cache_list->head;
    if (node == NULL)
    {
        sal_fprintf(fp, "No single-ended loss statistics\n");
        return;
    }

    sal_fprintf(fp, "%12s %s\n", "MEGID      :", pst_oam->megid_str);
    sal_fprintf(fp, "%12s %d\n", "MEPID      :", pst_lm_single->mepid);

    /* print start time */
    lm_single_entry = node->data;

    time = sal_localtime(&lm_single_entry->recv_time); 
    sal_fprintf(fp, "%12s %d/%02d/%02d %d:%02d:%02d\n", "Start Time :", time->tm_year + 1900, time->tm_mon + 1, 
        time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);

    /* print end time */
    node = pst_lm_single->lm_cache_list->tail;
    if (node == NULL)
    {
        sal_fprintf(fp, "%12s null\n", "End Time   :");
        return;
    }
    else
    {
        lm_single_entry = (tpoam_single_lm_entry_t *)node->data;
    
        time = sal_localtime(&lm_single_entry->recv_time); 
        sal_fprintf(fp, "%12s %d/%02d/%02d %d:%02d:%02d\n", "End Time   :", time->tm_year + 1900, time->tm_mon + 1, 
            time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
    }

    sal_fprintf(fp, "Latest single-ended loss statistics:\n");

    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");

    sal_fprintf(fp, "%-5s %3s %10s %16s %11s %17s\n", "Index", "Exp", "Local-loss", "Local-loss-ratio", 
        "Remote-loss", "Remote-loss-ratio");

    sal_fprintf(fp, "-----+---+----------+----------------+-----------+------------------------------\n");

    LIST_LOOP (pst_lm_single->lm_cache_list, lm_single_entry, node)
    {
        if (first_node)
        {
            first_node = FALSE;
            continue;
        }
        
        index++;
        
        sal_fprintf(fp, "%-5d ", index);

        if (TPOAM_LM_COS_TYPE_ALL == pst_lm_single->cos_type)
        {
            sal_fprintf(fp, "%3s ", "all");
        }
        else
        {
            sal_fprintf(fp, "%3d ", lm_single_entry->lmm_cos_value);
        }

        sal_fprintf(fp, "%10u ", lm_single_entry->lm_near_end_frame_loss);
            
        sal_fprintf(fp, "        %02u.%04u%% ", 
            lm_single_entry->lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            lm_single_entry->lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
        
        sal_fprintf(fp, "%11u ", lm_single_entry->lm_far_end_frame_loss);
            
        sal_fprintf(fp, "         %02u.%04u%%\n", 
            lm_single_entry->lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            lm_single_entry->lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);


        if (1 == index)
        {
            max_lm_far_end_frame_loss = lm_single_entry->lm_far_end_frame_loss; 
            min_lm_far_end_frame_loss = lm_single_entry->lm_far_end_frame_loss; 
            
            max_lm_far_end_frame_loss_ratio = lm_single_entry->lm_far_end_frame_loss_ratio; 
            min_lm_far_end_frame_loss_ratio = lm_single_entry->lm_far_end_frame_loss_ratio; 

            max_lm_near_end_frame_loss = lm_single_entry->lm_near_end_frame_loss;
            min_lm_near_end_frame_loss = lm_single_entry->lm_near_end_frame_loss;

            max_lm_near_end_frame_loss_ratio = lm_single_entry->lm_near_end_frame_loss_ratio; 
            min_lm_near_end_frame_loss_ratio = lm_single_entry->lm_near_end_frame_loss_ratio; 
        }
        else
        {
            if (max_lm_far_end_frame_loss < lm_single_entry->lm_far_end_frame_loss)
            {
                max_lm_far_end_frame_loss = lm_single_entry->lm_far_end_frame_loss; 
            }

            if (min_lm_far_end_frame_loss > lm_single_entry->lm_far_end_frame_loss)
            {
                min_lm_far_end_frame_loss = lm_single_entry->lm_far_end_frame_loss; 
            }

            if (max_lm_far_end_frame_loss_ratio < lm_single_entry->lm_far_end_frame_loss_ratio)
            {
                max_lm_far_end_frame_loss_ratio = lm_single_entry->lm_far_end_frame_loss_ratio; 
            }

            if (min_lm_far_end_frame_loss_ratio > lm_single_entry->lm_far_end_frame_loss_ratio)
            {
                min_lm_far_end_frame_loss_ratio = lm_single_entry->lm_far_end_frame_loss_ratio; 
            }

            if (max_lm_near_end_frame_loss < lm_single_entry->lm_near_end_frame_loss)
            {
                max_lm_near_end_frame_loss = lm_single_entry->lm_near_end_frame_loss; 
            }

            if (min_lm_near_end_frame_loss > lm_single_entry->lm_near_end_frame_loss)
            {
                min_lm_near_end_frame_loss = lm_single_entry->lm_near_end_frame_loss; 
            }

            if (max_lm_near_end_frame_loss_ratio < lm_single_entry->lm_near_end_frame_loss_ratio)
            {
                max_lm_near_end_frame_loss_ratio = lm_single_entry->lm_near_end_frame_loss_ratio; 
            }

            if (min_lm_near_end_frame_loss_ratio > lm_single_entry->lm_near_end_frame_loss_ratio)
            {
                min_lm_near_end_frame_loss_ratio = lm_single_entry->lm_near_end_frame_loss_ratio; 
            }
        }
    }
    
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");

    sal_fprintf(fp, "Maximum Local-loss  : %-10u ", max_lm_near_end_frame_loss);
    sal_fprintf(fp, "Maximum Local-loss-ratio  : %02u.%04u%%\n", 
            max_lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            max_lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);       

    sal_fprintf(fp, "Minimum Local-loss  : %-10u ", min_lm_near_end_frame_loss);
    sal_fprintf(fp, "Minimum Local-loss-ratio  : %02u.%04u%%\n", 
            min_lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            min_lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);        

    sal_fprintf(fp, "Maximum Remote-loss : %-10u ", max_lm_far_end_frame_loss);
    sal_fprintf(fp, "Maximum Remote-loss-ratio : %02u.%04u%%\n", 
            max_lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            max_lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

    sal_fprintf(fp, "Minimum Remote-loss : %-10u ", min_lm_far_end_frame_loss);
    sal_fprintf(fp, "Minimum Remote-loss-ratio : %02u.%04u%%\n", 
        min_lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        min_lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

    /* modified by yaom for proactive single lm 20140416 */
    if (!pst_lm_single->is_proactive)
    {
        LIST_LOOP_DEL (pst_lm_single->lm_cache_list, lm_single_entry, node, next_node)
        {
            listnode_delete (pst_lm_single->lm_cache_list, lm_single_entry); 
            XFREE (MEM_PM_TPOAM_DM_SINGLE_ENTRY, lm_single_entry);
        }
    }

    return;
}
#endif


int32
tpoam_api_clear_evt_or_1dm(uint32 type, tpoam_pw_type_t pw_type, 
    char* pch_service_name, uint32 clear_type)
{
    tbl_section_oam_t* pst_section = NULL;
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;
    int32 rc = TPOAM_E_NONE;
    tbl_iter_args_t args;

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (GLB_OAM_TYPE_MPLS_SECTION == type || 0 == type)
    {
        if (!pch_service_name)
        {
            if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
            {
                args.argv[0] = NULL;
                args.argv[1] = tpoam_event_clear_oam_event;
                tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
            }
            else
            {
                //1dm
                //args.argv[0] = NULL;
                //args.argv[1] = tpoam_dm_clear_1dm;
                //tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
            }
        }
        else
        {
            rc = tpoam_api_get_section(pch_service_name, &pst_section);
            if (rc < 0)
            {
                return rc;
            }
            
            if (!pst_section->oam_session_id)
            {
                return TPOAM_E_SERVICE_NON_OAM;
            }

            p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
            if (NULL != p_db_oam_session)
            {
                if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
                {
                    tpoam_event_clear_oam_event(NULL, p_db_oam_session, NULL);
                }
                else
                {
                    //tpoam_dm_clear_1dm(NULL, p_db_oam_session, NULL);
                }
            }
        }
    }
    
    if (GLB_OAM_TYPE_MPLS_LSP == type || 0 == type)
    {
        if (!pch_service_name)
        {
            if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
            {
                args.argv[0] = NULL;
                args.argv[1] = tpoam_event_clear_oam_event;
                tbl_lsp_pe_iterate(_tpoam_api_show_lsp_pe_param2_interator, &args);
                tbl_lsp_p_iterate(_tpoam_api_show_lsp_p_param2_interator, &args);
            }
            else
            {
                args.argv[0] = NULL;
                //args.argv[1] = tpoam_dm_clear_1dm;
                tbl_lsp_pe_iterate(_tpoam_api_show_lsp_pe_param2_interator, &args);
                tbl_lsp_p_iterate(_tpoam_api_show_lsp_p_param2_interator, &args);
            }
        }
        else
        {
            rc = tpoam_api_get_lsp_pe(pch_service_name, &pst_lsp_pe);
            if (rc < 0)
            {
                rc = tpoam_api_get_lsp_p(pch_service_name, &pst_lsp_p);
                if (rc < 0)
                {
                    return rc;
                }
                else
                {
                    if (!pst_lsp_p->oam_session_id)
                    {
                        return TPOAM_E_SERVICE_NON_OAM;
                    }

                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
                        {
                            tpoam_event_clear_oam_event(NULL, p_db_oam_session, NULL);
                        }
                        else
                        {
                            //tpoam_dm_clear_1dm(NULL, p_db_oam_session, NULL);
                        }
                    }
                }
            }
            else
            {
                if (!pst_lsp_pe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }

                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
                if (NULL != p_db_oam_session)
                {
                    if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
                    {
                        tpoam_event_clear_oam_event(NULL, p_db_oam_session, NULL);
                    }
                    else
                    {
                        //tpoam_dm_clear_1dm(NULL, p_db_oam_session, NULL);
                    }
                }
            }
        }
    }
    
    if (GLB_OAM_TYPE_MPLS_PW == type || 0 == type)
    {
        if (!pch_service_name)
        {
            if (TPOAM_PW_TYPE_TPE == pw_type || TPOAM_PW_TYPE_MAX == pw_type)
            {
                if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
                {
                    args.argv[0] = NULL;
                    args.argv[1] = tpoam_event_clear_oam_event;
                    tbl_pw_tpe_iterate(_tpoam_api_show_pw_tpe_param2_interator, &args);              
                }
                else
                {
                    //args.argv[0] = NULL;
                    //args.argv[1] = tpoam_dm_clear_1dm;
                    //tbl_pw_tpe_iterate(_tpoam_api_show_pw_tpe_param2_interator, &args);
                }
            }
        }
        else
        {
            if (TPOAM_PW_TYPE_TPE == pw_type)
            {
                rc = tpoam_api_get_pw_tpe(pch_service_name, &pst_tpe);
                if (rc < 0)
                {
                    return rc;
                }
                if (!pst_tpe->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                else
                {
                    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
                    if (NULL != p_db_oam_session)
                    {
                        if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
                        {
                            tpoam_event_clear_oam_event(NULL, p_db_oam_session, NULL);
                        }
                        else
                        {
                            //tpoam_dm_clear_1dm(NULL, p_db_oam_session, NULL);
                        }
                    }
                }
            }
        }
    }
    
    return TPOAM_E_NONE;
}


int32
_tpoam_cmd_clear_event_one(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t           *p_db_oam_session = obj;

    tpoam_event_clear_oam_event(NULL, p_db_oam_session, NULL);

    return PM_E_NONE;
}

int32
tpoam_api_clear_evt_or_1dm_session(uint32 session_id, uint32 clear_type)
{
    tbl_oam_session_t *p_db_oam_session = NULL;
    
    if (!session_id)
    {
        if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
        {
            tbl_oam_session_iterate(_tpoam_cmd_clear_event_one, NULL);
        }
        else
        {
            //1dm
            //args.argv[0] = NULL;
            //args.argv[1] = tpoam_dm_clear_1dm;
            //tbl_section_oam_iterate(_tpoam_api_show_section_param2_interator, &args);
        }
    }
    else
    {
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            if (TPOAM_API_CLEAR_TYPE_EVENT == clear_type)
            {
                tpoam_event_clear_oam_event(NULL, p_db_oam_session, NULL);
            }
            else
            {
                //tpoam_dm_clear_1dm(NULL, p_db_oam_session, NULL);
            }
        }
    }
    
    return TPOAM_E_NONE;
}


#endif
int32
tpoam_api_set_lm_dual_en_to_fea(tbl_oam_session_t* pst_oam)
{
    glb_openflow_tpoam_t  oam_flow;
    int32 rc = PM_E_NONE;
    
    if (pst_oam->bind_oam_flow_id)
    {
        sal_memset(&oam_flow, 0x0, sizeof(oam_flow));
        rc = tpoam_api_get_fei_key(pst_oam, &oam_flow);
        if (PM_E_NONE == rc)
        {
            oam_flow.flag |= GLB_TPOAM_UPDATE_LM_EN;
            oam_flow.mepid = pst_oam->mepid;
            oam_flow.lm_enable = pst_oam->lm_enable;
            oam_flow.lm_type = TPOAM_LM_TYPE_DUAL;
            oam_flow.lm_cos_type = pst_oam->oam_lm.lm_dual.cos_type;
            oam_flow.lm_cos_value = pst_oam->oam_lm.lm_dual.cos_value;
            oam_flow.lm_stats_interval = pst_oam->oam_lm.lm_dual.stats_interval;
            fei_flow_update_tpoam_flow(&oam_flow);
        }
    }
    else
    {
        /*not bind flow, not need to set to fea*/
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

int32
tpoam_api_set_lm_dual_en(tbl_oam_session_t* pst_oam)
{  
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }

    TPOAM_DBG_LM_COMM("set session id %u MEG %s LMEP %d dual lm enable %d", 
        pst_oam->key.session_id, pst_oam->megid_str, pst_oam->mepid, pst_oam->lm_enable);

    if (pst_oam->lm_enable)
    {
        if (TPOAM_LM_TYPE_NONE != pst_oam->oam_lm.lm_type)
        {
            //return TPOAM_E_LM_ALREADY_CONFIG;
        }

        /* 1. Set to DB */
        //sal_memset(&(pst_oam->oam_lm.lm_dual), 0x00, sizeof(tpoam_dual_lm_t));
        
        //pst_oam->oam_lm.lm_type = TPOAM_LM_TYPE_DUAL;
        pst_oam->oam_lm.lm_dual.mepid = pst_oam->mepid;
        //pst_oam->oam_lm.lm_dual.stats_interval = lm_dual_inputs->stats_interval;
        //pst_oam->oam_lm.lm_dual.cos_type = lm_dual_inputs->cos_type;
        //pst_oam->oam_lm.lm_dual.cos_value = lm_dual_inputs->cos_value;
        //pst_oam->oam_lm.lm_dual.cache_size = lm_dual_inputs->cache_size;
        if (NULL == pst_oam->oam_list.pst_dual_lm_cache_list)
        {
            pst_oam->oam_list.pst_dual_lm_cache_list = list_new();
        }
    }
    else
    {
        if (TPOAM_LM_TYPE_DUAL != pst_oam->oam_lm.lm_type)
        {
            //return TPOAM_E_NONE;
        }

        /* delete lm*/
       tpoam_lm_destroy_list_dual_lm(pst_oam);
       
        //sal_memset(&(pst_oam->oam_lm.lm_dual), 0x00, sizeof(tpoam_dual_lm_t));
        //pst_oam->oam_lm.lm_type = TPOAM_LM_TYPE_NONE;
    }

    /* 2. Set to HAL */
    tpoam_api_set_lm_dual_en_to_fea(pst_oam);
    
    return TPOAM_E_NONE;    
}


static int32
_tpoam_api_service_get_section_status(tbl_section_oam_t* pst_section, uint32* pst_status)
{
    tbl_interface_t     *p_db_if    = NULL;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(pst_section->key.ifindex);
    if (NULL == p_db_if)
    {
        return TPOAM_E_NONE;
    }
    
    *pst_status = tbl_interface_is_running(p_db_if) ? TPOAM_STATUS_UP : TPOAM_STATUS_DOWN;
    
    if (TPOAM_STATUS_DOWN == *pst_status)
    {
        return TPOAM_E_NONE;
    }

    // TODO: vlan if 
    #if 0
    /* if ifp is UP, member ifp also need to be checked if has */
    NSM_TPOAM_SEC_NONIP_IS_VLAN_IF(pst_section)
    {
        *pst_status = NSM_TPOAM_STATUS_DOWN;
        mem_ifp = nsm_get_ifp_by_ifindex(pst_section->mem_oifindex);
        if (!mem_ifp)
        {
            return TPOAM_E_NONE;
        }
        *pst_status = if_is_running(mem_ifp) ? NSM_TPOAM_STATUS_UP : NSM_TPOAM_STATUS_DOWN;
    }
    #endif
     
    return TPOAM_E_NONE;
}


static int32
_tpoam_api_service_get_lsp_p_status(tbl_lsp_p_t* pst_lsp_p, tpoam_direction_t direction, uint32* pst_status)
{
    tbl_interface_t     *p_db_if    = NULL;

    if (TPOAM_DIR_EAST == direction)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(pst_lsp_p->east_ifindex);
        if (NULL == p_db_if)
        {
            return TPOAM_E_NONE;
        }
        
        *pst_status = tbl_interface_is_running(p_db_if) ? TPOAM_STATUS_UP : TPOAM_STATUS_DOWN;
        if (TPOAM_STATUS_DOWN == *pst_status)
        {
            return TPOAM_E_NONE;
        }

        // TODO: support mem_ifindex
        /* if ifp is UP, member ifp also need to be checked if has */
        #if 0
        if (pst_lsp->u.lsp_p.mem_ifindex_east)
        {
            *pst_status = TPOAM_STATUS_DOWN;
            mem_ifp = nsm_get_ifp_by_ifindex(pst_lsp->u.lsp_p.mem_ifindex_east);
            if (!mem_ifp)
            {
                return TPOAM_E_NONE;
            }
            *pst_status = if_is_running(mem_ifp) ? TPOAM_STATUS_UP : TPOAM_STATUS_DOWN;
        }
        #endif
    }
    else if (TPOAM_DIR_WEST == direction)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(pst_lsp_p->west_ifindex);
        if (NULL == p_db_if)
        {
            return TPOAM_E_NONE;
        }
        
        *pst_status = tbl_interface_is_running(p_db_if) ? TPOAM_STATUS_UP : TPOAM_STATUS_DOWN;
        if (TPOAM_STATUS_DOWN == *pst_status)
        {
            return TPOAM_E_NONE;
        }

        // TODO: support mem_ifindex
        /* if ifp is UP, member ifp also need to be checked if has */
        #if 0
        if (pst_lsp->u.lsp_p.mem_ifindex_west)
        {
            *pst_status = TPOAM_STATUS_DOWN;
            mem_ifp = nsm_get_ifp_by_ifindex(pst_lsp->u.lsp_p.mem_ifindex_west);
            if (!mem_ifp)
            {
                return TPOAM_E_NONE;
            }
            *pst_status = if_is_running(mem_ifp) ? TPOAM_STATUS_UP : TPOAM_STATUS_DOWN;
        }
        #endif
    }

    return TPOAM_E_NONE;
}


static int32
_tpoam_api_service_get_lsp_pe_status(tbl_lsp_pe_t *pst_lsp_pe , uint32* pst_status)
{
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
    tbl_interface_t     *p_db_if    = NULL;

    sal_memset(&ag_info_key, 0x00, sizeof(ag_info_key));
    ag_info_key.aps_group_id = pst_lsp_pe->lsp_aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
    if (NULL != p_db_ag_info)
    {
        if (0 == sal_strcmp(p_db_ag_info->select_name, pst_lsp_pe->key.name))  
        {
            *pst_status = TPOAM_STATUS_UP;
        }
        else
        {
            *pst_status = TPOAM_STATUS_DOWN;
            return TPOAM_E_NONE;
        }
           
    }
    else
    {
        //return TPOAM_E_NONE;
    }
    
    p_db_if = tbl_interface_get_interface_by_ifindex(pst_lsp_pe->ifindex);
    if (NULL == p_db_if)
    {
        return TPOAM_E_NONE;
    }
    *pst_status = tbl_interface_is_running(p_db_if) ? TPOAM_STATUS_UP : TPOAM_STATUS_DOWN;  

    return TPOAM_E_NONE;
}


static int32
_tpoam_api_service_get_pw_tpe_status(tbl_pw_tpe_t *pst_tpe , uint32* pst_status)
{
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
    tbl_lsp_pe_t* pst_lsp_pe = NULL;

    sal_memset(&ag_info_key, 0x00, sizeof(ag_info_key));
    ag_info_key.aps_group_id = pst_tpe->pw_aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
    if (NULL != p_db_ag_info)
    {
        if (0 == sal_strcmp(p_db_ag_info->select_name, pst_tpe->key.name))  
        {
            *pst_status = TPOAM_STATUS_UP;
        }
        else
        {
            *pst_status = TPOAM_STATUS_DOWN;
            return TPOAM_E_NONE;
        }
    }
    else
    {
        //return TPOAM_E_NONE;
    }
    
    pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_tpe->lsp_name);
    if (NULL == pst_lsp_pe)
    {
        sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
        ag_info_key.aps_group_id = pst_tpe->lsp_aps_group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
        if (p_db_ag_info)
        {
            pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(p_db_ag_info->select_name);
            if (NULL == pst_lsp_pe)
            {
                return TPOAM_E_NONE;
            }
        }
        else
        {
            //return TPOAM_E_NONE;
        }
    } 
    _tpoam_api_service_get_lsp_pe_status(pst_lsp_pe, pst_status);

    return TPOAM_E_NONE;
}

int32
tpoam_api_service_get_status(tbl_oam_session_t* pst_oam, uint32 direction, uint32* pst_status)
{
    tbl_section_oam_t *pst_section = NULL;
    tbl_section_oam_key_t section_key;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_lsp_pe_key_t lsp_pe_key;
    tbl_lsp_p_t *pst_lsp_p = NULL;
    tbl_lsp_p_key_t lsp_p_key;
    tbl_pw_tpe_t * pst_tpe = NULL;
    tbl_pw_tpe_key_t pw_tpe_key;

    *pst_status = TPOAM_STATUS_DOWN;
    switch (pst_oam->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        section_key.ifindex = pst_oam->service_ifindex;
        pst_section = tbl_section_oam_get_section_oam(&section_key);
        if (NULL != pst_section)
        {
            return _tpoam_api_service_get_section_status(pst_section, pst_status);
        }
        return TPOAM_E_NONE;

    case GLB_OAM_TYPE_MPLS_LSP:
        sal_memset(&lsp_pe_key, 0x00, sizeof(lsp_pe_key));
        sal_strcpy(lsp_pe_key.name, pst_oam->service_name);
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
        if (NULL != pst_lsp_pe)
        {
            return _tpoam_api_service_get_lsp_pe_status(pst_lsp_pe, pst_status);
        }

        sal_memset(&lsp_p_key, 0x00, sizeof(lsp_p_key));
        sal_strcpy(lsp_p_key.name, pst_oam->service_name);
        pst_lsp_p = tbl_lsp_p_get_lsp_p(&lsp_p_key);
        if (NULL != pst_lsp_p)
        {
            return _tpoam_api_service_get_lsp_p_status(pst_lsp_p, direction, pst_status);
        }
        return TPOAM_E_NONE;
        
    case GLB_OAM_TYPE_MPLS_PW:
        switch (pst_oam->pw_type)
        {
        case TPOAM_PW_TYPE_TPE:
            sal_memset(&pw_tpe_key, 0x00, sizeof(pw_tpe_key));
            sal_strcpy(pw_tpe_key.name, pst_oam->service_name);
            pst_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
            if (NULL != pst_tpe)
            {
                return _tpoam_api_service_get_pw_tpe_status(pst_tpe, pst_status);
            }
            return TPOAM_E_NONE;
        default:
            return TPOAM_E_INVALID_TYPE;
        }        
        break;

    default:
        return TPOAM_E_INVALID_TYPE;
    }
    return TPOAM_E_NONE;
}

