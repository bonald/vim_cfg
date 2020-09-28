#include "proto.h"
#include "glb_ns_port_forwarding.h"
#include "gen/tbl_ns_port_forwarding_define.h"
#include "gen/tbl_ns_port_forwarding.h"
#include "switch.h"
#include "ns_port_forwarding.h"

#define NS_PORT_FORWARDING_ARG_NUM 4

static int32
_ns_port_forwarding_cmd_entry_op(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 field_id;
    cdb_node_t *node;
    cdb_field_t *field;
    tbl_ns_port_forwarding_t entry;
    
    node = cdb_get_tbl(TBL_NS_PORT_FORWARDING);

    sal_memset(&entry, 0, sizeof(tbl_ns_port_forwarding_t));
    rc = cdb_ns_port_forwarding_key_str2val(&entry.key, argv[0], NULL);
    if (PM_E_NONE != rc)
    {
        CFG_INVALID_PARAM_RET("Parmameter %s is not in type#proto#port#mapped_address form", argv[0]);        
    }

    field_id = cdb_get_field_id(argv[1], node, &field);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
    }

    if (field_id != TBL_NS_PORT_FORWARDING_FLD_IP)
    {
        CFG_INVALID_PARAM_RET("Invalid field %s", argv[1]);
    }

    sal_inet_pton(AF_INET, argv[2], &entry.ip);

    rc = ns_port_forwarding_op_entry(para->oper, &entry);

    if (PM_E_EXIST == rc)
    {
        CFG_INVALID_PARAM_RET("Mapping exists port %u", entry.key.port);
    }
    
    if (PM_E_NOT_EXIST == rc)
    {
        CFG_INVALID_PARAM_RET("Mapping not exists");
    }

    return rc;
}

static int32
_ns_port_forwarding_dump_all(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
 
    tbl_ns_port_forwarding_iterate((TBL_ITER_CB_FUNC)tbl_ns_port_forwarding_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
ns_port_forwarding_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    
    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
        case CDB_OPER_ADD:
        case CDB_OPER_DEL:
            PM_E_RETURN(_ns_port_forwarding_cmd_entry_op(argv, argc, para));
            break;        
            
        case CDB_OPER_GET:
            /* Only dump all is supported now. */
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));

            PM_E_RETURN(_ns_port_forwarding_dump_all(para, &field));
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }
    
    return rc;
}

