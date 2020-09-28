#include "proto.h"

#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#ifdef tpoam_debug
int32
_tpoam_cmd_get_tpoam_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_tpoam_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_tpoam_debug_get_tpoam_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_tpoam_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
tpoam_cmd_process_tpoam_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    #define DBG_TPOAM_TYPE_FIELD       0
    #define DBG_TPOAM_TYPE_ALL         1

    cdb_node_t* p_node = NULL;
    tbl_tpoam_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_tpoam_debug_t *p_db_dbg = NULL;
    field_parse_t field;
    uint32 multi_type = DBG_TPOAM_TYPE_FIELD;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_TPOAM_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_tpoam_debug_get_tpoam_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("TPOAM_DEBUG not found");
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_TPOAM_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }

        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_TPOAM_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_TPOAM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LB_LBM_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LB_LBR_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LM_LMM_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LM_LMR_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_DMM_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_DMR_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_1DMRX_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_1DMTX_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_PACKET_RX_DBG);
                break;
            case TBL_TPOAM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_PACKET_TX_DBG);
                break;
            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_tpoam_debug_set_tpoam_debug_field(&dbg, field_id);
        }
        else if (DBG_TPOAM_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LB_LBM_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LB_LBR_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LM_LMM_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_LM_LMR_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_DMM_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_DMR_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_1DMRX_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_DM_1DMTX_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_PACKET_RX_DBG);
            GLB_SET_FLAG_COND(value, dbg.tpoam, TPOAMDBG_FLAG_PACKET_TX_DBG);
            
            

            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_LB_LBM_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_LB_LBR_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_LM_LMM_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_LM_LMR_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_DM_DMM_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_DM_DMR_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_DM_1DMRX_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_DM_1DMTX_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_PACKET_RX_DBG);
            rc += tbl_tpoam_debug_set_tpoam_debug_field(&dbg, TPOAMDBG_FLAG_PACKET_TX_DBG);
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _tpoam_cmd_get_tpoam_debug(para, p_node, &field);
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
#endif


