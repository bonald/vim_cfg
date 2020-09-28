
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_nexthop.h"


extern int32
tbl_nexthop_dump_one(tbl_nexthop_t *p_nh, tbl_iter_args_t *pargs);

static int32
_rt_cmd_get_nexthop(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nexthop_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rt_cmd_process_nexthop(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node = NULL;
    field_parse_t field;
    char sys_cmd[128];
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    sal_memset(sys_cmd, 0, sizeof(sys_cmd));
    p_node = cdb_get_tbl(TBL_NEXTHOP);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_nexthop(argv[0], para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

