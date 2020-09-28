#include "proto.h"
#include "g8032_define.h"
#include "g8032.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"

int32
g8032_ring_cmd_get_ring(uint32 ring_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == ring_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_g8032_ring_dump_one, &args);
    }
    else
    {
        sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
        g8032_ring_key.ring_id= ring_id;
        p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);
        if (!p_db_g8032_ring)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET(" Ring %u does not exist", ring_id);
        }        
        tbl_g8032_ring_dump_one(p_db_g8032_ring, &args);
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}