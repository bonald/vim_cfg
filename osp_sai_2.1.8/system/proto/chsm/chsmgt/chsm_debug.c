/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2016-02-23
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "chsm_inc.h"
#include "gen/tbl_chsm_debug_define.h"
#include "gen/tbl_chsm_debug.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

static int32
_chsm_debug_cmd_get_chsm_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_chsm_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_chsm_debug_get_chsm_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_chsm_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_chsm_debug_cmd_process_chsm_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_chsm_debug_t dbg;
    tbl_chsm_debug_t *p_db_dbg = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CHSM_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_chsm_debug_get_chsm_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("PM_DEBUG not found");
        }

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        switch (field_id)
        {
        case TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS:
            GLB_SET_FLAG_COND(value, dbg.chsm, CHSMDBG_FLAG_CHSM_CHASSIS);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        rc = tbl_chsm_debug_set_chsm_debug_field(&dbg, field_id);
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _chsm_debug_cmd_get_chsm_debug(para, p_node, &field);
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

int32
chsm_debug_start()
{
    cdb_register_cfg_tbl_cb(TBL_CHSM_DEBUG, _chsm_debug_cmd_process_chsm_debug);

    return PM_E_NONE;
}

