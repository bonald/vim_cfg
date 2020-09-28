
#include "proto.h"
#include "fei.h"
#include "opm.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#include "erps_api.h"
#include "gen/ds_lag_define.h"

#include "glb_g8131_define.h"
#include "g8131_api.h"
#include "g8032.h"
#include "tpoam.h"
#include "cfm_define.h"

/*--------------------------------------------------------- */
/* Print help message */
static void 
usage(int32 status, const char *argv0)
{
    const char *name = NULL;

    if (!argv0)
    {
        return;
    }

    /* Find the basename */
    name = sal_strrchr(argv0, '/');
    if (name)
        name++;
    else
        name = argv0;

    if (status != 0) {
        sal_printf("Try `%s -h' for more information.\n", name);
    } else {
        sal_printf("Usage: %s [options] [script_file] [script_file] ...\n", name);
        sal_printf("CLI utility. Command line shell."
            "The part of the klish project.\n");
        sal_printf("Options:\n");
        sal_printf("\t-v, --version\tPrint version.\n");
        sal_printf("\t-h, --help\tPrint this help.\n");
    }
}

static int32
_opm_sock_init()
{
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(10);
    
    /* client to FEA */
    fei_init(lib_get_master(), NULL, TRUE, 0);

    /* client to Packet */
    ipc_pkt_init(0);

    /* client to trap agent */
    ipc_connect_snmp_trap();

    return PM_E_NONE;
}

int32
opm_mstp_instance_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mstp_instance_t *      p_mstp_instance = NULL;   
    int32 rc = PM_E_NONE;
        
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        p_mstp_instance = (tbl_mstp_instance_t*)p_tbl;
        OPM_PTR_CHECK(p_mstp_instance);
        rc = erps_clear_erps_domain_instnace(p_mstp_instance->key.instance);
        break;

    case CDB_OPER_SET:        
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

int32
opm_interface_before_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_interface_t    *p_if = NULL;
    int32              rc = PM_E_NONE;
    ds_lag_t           *p_lagif = NULL;
    tbl_interface_key_t *p_if_key = NULL;

    if(p_ds_node)
    {
        if (DS_LAG == p_ds_node->id)
        {
            switch (oper)
            {
            case CDB_OPER_ADD:
                p_if_key = (tbl_interface_key_t*)p_tbl;
                OPM_PTR_CHECK(p_if_key);
                p_if = tbl_interface_get_interface(p_if_key);
                OPM_PTR_CHECK(p_if);

                if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
                {
                    return rc;
                }
        
                rc = erps_port_lacp_add_member(p_if, (ds_lag_t*)p_ds);
                break;
        
            case CDB_OPER_DEL:
                p_if_key = (tbl_interface_key_t*)p_tbl;
                OPM_PTR_CHECK(p_if_key);
                p_if = tbl_interface_get_interface(p_if_key);
                OPM_PTR_CHECK(p_if);

                if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
                {
                    return rc;
                }
        
                rc = erps_port_lacp_delete_member(p_if, (ds_lag_t*)p_ds);
                break;
        
            case CDB_OPER_SET: 
                p_if_key = (tbl_interface_key_t*)p_tbl;
                OPM_PTR_CHECK(p_if_key);
                p_if = tbl_interface_get_interface(p_if_key);
                OPM_PTR_CHECK(p_if);
        
                p_lagif = (ds_lag_t*)p_ds;
                OPM_PTR_CHECK(p_lagif);
        
                if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
                {
                    return rc;
                }
                switch(field_id)
                {
                case DS_LAG_FLD_OPER_STATE:
                    if (p_lagif->oper_state)
                    {
                        rc = erps_port_lacp_attach_member(p_if, p_lagif);
                    }
                    else
                    {
                        rc = erps_port_lacp_detach_member(p_if, p_lagif);
                    }
                    break;
                }
                break;
        
            default:
                return PM_E_INVALID_PARAM;
            }
        }
        
        return rc;
    }

    return rc;
}


int32
opm_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_interface_t        *p_if = NULL;
    int32                  rc = PM_E_NONE;
        
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_if = (tbl_interface_t*)p_tbl;
        OPM_PTR_CHECK(p_if);

        /* need to check by wangjj for erps bug 41433, 2016-11-24 */
        /* modified by wangjj for fix erps bug 41433, 2016-11-24 */
        //if (p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_RUNNING:
                rc = erps_port_report_link_state(p_if);
                break;

            default:
                break;
            }
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
opm_cmd_get_opm_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;  
    tbl_iter_args_t args;
    tbl_opm_debug_t* p_dbg = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    p_dbg = tbl_opm_debug_get_opm_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_opm_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

#define DBG_OPM_TYPE_FIELD     0
#define DBG_OPM_TYPE_ALL       1

static int32
opm_cmd_process_opm_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    field_parse_t field;

    tbl_opm_debug_t opm_dbg;
    tbl_opm_debug_t *p_db_opm_dbg = NULL;
    uint32 multi_type = DBG_OPM_TYPE_FIELD;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_OPM_DEBUG);
    sal_memset(&opm_dbg, 0, sizeof(opm_dbg));
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        /* check exist */
        p_db_opm_dbg = tbl_opm_debug_get_opm_debug();
        if (!p_db_opm_dbg)
        {
            CFG_CONFLICT_RET("OPM_DEBUG not found");
        }
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_OPM_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

        if (DBG_OPM_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_OPM_DEBUG_FLD_ERPS_ALL:
                GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_ALL);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_PACKET:
                GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_PACKET);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_TIMER:
                GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_TIMER);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL:
                GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_PROTO);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_EVENT:
                GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_EVENTS);
                break;
            case TBL_OPM_DEBUG_FLD_G8131_TX:
                GLB_SET_FLAG_COND(value, opm_dbg.g8131, OPMDBG_FLAG_G8131_TX);
                break;
            case TBL_OPM_DEBUG_FLD_G8131_RX:
                GLB_SET_FLAG_COND(value, opm_dbg.g8131, OPMDBG_FLAG_G8131_RX);
                break;
            case TBL_OPM_DEBUG_FLD_G8131_EVENT:
                GLB_SET_FLAG_COND(value, opm_dbg.g8131, OPMDBG_FLAG_G8131_EVENT);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LB_LBM_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LB_LBR_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LM_COMM_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LM_LMM_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LM_LMR_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_COMM_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_DMM_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_DMR_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_PACKET_RX_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_PACKET_TX_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG:
                GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_EVENT_DBG);
                break;
            case TBL_OPM_DEBUG_FLD_G8032_PACKET_TX:
                GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_TX);
                break;
            case TBL_OPM_DEBUG_FLD_G8032_PACKET_RX:
                GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_RX);
                break;
            case TBL_OPM_DEBUG_FLD_G8032_EVENT:
                GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_EVENT);
                break;
            case TBL_OPM_DEBUG_FLD_G8032_TIMER:
                GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_TIMER);
                break;
            case TBL_OPM_DEBUG_FLD_CFM_ALL:
#if (defined DUET2) || (defined TSINGMA)
                CFG_INVALID_PARAM_RET("Not supported on this product");
#endif                
                GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_ALL);
                break;
            case TBL_OPM_DEBUG_FLD_CFM_PACKET:
#if (defined DUET2) || (defined TSINGMA)
                CFG_INVALID_PARAM_RET("Not supported on this product");
#endif 
                GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_PACKET);
                break;
            case TBL_OPM_DEBUG_FLD_CFM_TIMER:
#if (defined DUET2) || (defined TSINGMA)
                CFG_INVALID_PARAM_RET("Not supported on this product");
#endif 
                GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_TIMER);
                break;
            case TBL_OPM_DEBUG_FLD_CFM_EVENT:
#if (defined DUET2) || (defined TSINGMA)
                CFG_INVALID_PARAM_RET("Not supported on this product");
#endif 
                GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_EVENT);
                break;
            }
            PM_E_RETURN(tbl_opm_debug_set_opm_debug_field(&opm_dbg, field_id));
        }
        else if (DBG_OPM_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_ALL);
            GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_PACKET);
            GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_TIMER);
            GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_PROTO);
            GLB_SET_FLAG_COND(value, opm_dbg.erps, OPMDBG_FLAG_ERPS_EVENTS);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_ERPS_ALL);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_ERPS_PACKET);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_ERPS_TIMER);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_ERPS_EVENT);
            
            GLB_SET_FLAG_COND(value, opm_dbg.g8131, OPMDBG_FLAG_G8131_TX);
            GLB_SET_FLAG_COND(value, opm_dbg.g8131, OPMDBG_FLAG_G8131_RX);
            GLB_SET_FLAG_COND(value, opm_dbg.g8131, OPMDBG_FLAG_G8131_EVENT);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8131_TX);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8131_RX);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8131_EVENT);

            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LB_LBM_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LB_LBR_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LM_COMM_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LM_LMM_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_LM_LMR_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_COMM_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_DMM_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_DM_DMR_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_PACKET_RX_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_PACKET_TX_DBG);
            GLB_SET_FLAG_COND(value, opm_dbg.tpoam, OPMDBG_FLAG_TPOAM_EVENT_DBG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG);

            GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_ALL);
            GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_TX);
            GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_RX);
            GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_EVENT);
            GLB_SET_FLAG_COND(value, opm_dbg.g8032, OPMDBG_FLAG_G8032_TIMER);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8032_ALL);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8032_PACKET_TX);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8032_PACKET_RX);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8032_EVENT);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_G8032_TIMER);
            GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_ALL);
            GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_PACKET);
            GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_TIMER);
            GLB_SET_FLAG_COND(value, opm_dbg.cfm, OPMDBG_FLAG_CFM_EVENT);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_CFM_ALL);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_CFM_PACKET);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_CFM_TIMER);
            rc = tbl_opm_debug_set_opm_debug_field(&opm_dbg, TBL_OPM_DEBUG_FLD_CFM_EVENT);

        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = opm_cmd_get_opm_debug(para, p_node, &field);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Invalid path");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_opm_sync_init()
{
    cdb_pm_subscribe_tbl(TBL_MSTP_INSTANCE, opm_mstp_instance_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_INTERFACE, opm_interface_before_sync, opm_interface_sync);

    cdb_register_cfg_tbl_cb(TBL_OPM_DEBUG, opm_cmd_process_opm_debug);
    return PM_E_NONE;
}


int32
opm_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_OPM);
    lib_master->task_mst = ctc_task_init();

    lib_master->proto_mst = NULL;
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    _opm_sock_init();
    _opm_sync_init();

    erps_start();
    g8131_start();
    g8032_start();
    tpoam_start();

    cfm_start();  /* modified by liwh for bug 53010, 2019-07-22 */

    ctc_task_main();
   
    return PM_E_NONE;
}

int32
main(int32 argc, char **argv)
{
    char *p;
    char *progname;
    uint16 daemon_mode = 0;
    static const char *shortopts = "d:h:v";
 
/* 
Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
 { "daemon",      no_argument,       NULL, 'd'},
 { "help",        no_argument,       NULL, 'h'},
 { "version",     no_argument,       NULL, 'v'},
 { 0 }
};
#endif /* HAVE_GETOPT_H */

  
 
    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1)
    {   
         int32 opt;
   
#ifdef HAVE_GETOPT_H
         opt = getopt_long(argc, argv, shortopts, longopts, 0);
#else
         opt = getopt(argc, argv, shortopts);
#endif /* HAVE_GETOPT_H */
   
         if (EOF == opt)
         {   
             break;
         }
     
         switch (opt)
         {
         case 'd':
             daemon_mode = TRUE;
             break;
         case 'v':
             lib_print_version(progname);
             exit(0);
             break;
         case 'h':
             usage(FALSE, progname);
             break;
         default:
             usage(TRUE, progname);
             break;
         }
    }
 
    opm_init(daemon_mode);
    exit(0);
}

