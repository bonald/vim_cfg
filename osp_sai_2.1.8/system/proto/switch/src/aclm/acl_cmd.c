#include "proto.h"
#include "glb_acl_define.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_acl_config_define.h" 
#include "gen/tbl_acl_config.h" 
#include "gen/tbl_ace_config_define.h" 
#include "gen/tbl_ace_config.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_acl_worm_filter_define.h"
#include "gen/tbl_acl_worm_filter.h"
#include "gen/tbl_acl_entry_define.h" 
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_udf_group_define.h"
#include "gen/tbl_acl_udf_group.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_sys_spec_define.h"
#include "switch.h"
#include "acl.h"
#include "lib_acl.h"


extern int32 acl_time_range_add_time_range(tbl_time_range_t *p_time_range);

extern int32 acl_time_range_del_time_range(tbl_time_range_t *p_time_range);

int32
_acl_entry_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)tbl_acl_entry_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
_ace_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)tbl_ace_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
_acl_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_acl_config_iterate((TBL_ITER_CB_FUNC)tbl_acl_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
_acl_worm_filter_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_acl_worm_filter_iterate((TBL_ITER_CB_FUNC)tbl_acl_worm_filter_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
acl_entry_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ACL_ENTRY);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_GET:
        _acl_entry_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
_acl_udf_group_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_acl_udf_group_iterate((TBL_ITER_CB_FUNC)tbl_acl_udf_group_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
acl_udf_group_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ACL_UDF_GROUP);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_GET:
        _acl_udf_group_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;    
}

static int32
_acl_cmd_get_udf_entry(uint32  entry_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_acl_udf_entry_key_t udf_key;
    tbl_acl_udf_entry_t    *p_db_entry = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == entry_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_acl_udf_entry_dump_one, &args);
    }
    else
    {
        udf_key.id = entry_id;
        p_db_entry = tbl_acl_udf_entry_get_acl_udf_entry(&udf_key);
        if (!p_db_entry)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("ipsla entry %u does not exist", entry_id);
        }

        tbl_acl_udf_entry_dump_one(p_db_entry, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
acl_udf_entry_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    tbl_acl_udf_entry_t    udf_entry;
    tbl_acl_udf_entry_t   *p_db_entry  = NULL;
    uint32                 udf_id   = 0;
    uint32                 offset_type = 0;
    uint32                 offset_num  = 0;
    field_parse_t          field;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    int32                  iter = 0;

    CFG_DBG_FUNC();

    sal_memset(&udf_entry, 0, sizeof(udf_entry));

    p_node = cdb_get_tbl(TBL_ACL_UDF_ENTRY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);
        if (udf_id >=  GLB_ACL_UDF_ENTRY_NUM_MAX)
        {
            CFG_INVALID_PARAM_RET("UDF id maximum is %d on this product", GLB_ACL_UDF_ENTRY_NUM_MAX-1);
        }
        udf_entry.key.id = udf_id;

        p_db_entry = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
        if (1 == argc)
        {
            if (NULL == p_db_entry)
            {
                CFG_CONFLICT_RET("UDF entry %d not exist, specify the UDF type to create", udf_id);
            }
            break;
        }
        else
        {
            PM_CFG_GET_UINT(offset_type, argv[2], para->p_rs);
            udf_entry.offset_type = offset_type;

            if (NULL != p_db_entry)
            {
                if (offset_type != p_db_entry->offset_type)
                {
                    CFG_CONFLICT_RET("UDF entry %d have configured", udf_id);
                }
                break;
            }
        }

        rc = acl_api_add_udf_entry(&udf_entry, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);
        udf_entry.key.id = udf_id;
        p_db_entry = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
        if (NULL == p_db_entry)
        {
            CFG_CONFLICT_RET("UDF entry %d not exist", udf_id);
        }
        else
        {
            if (0 != p_db_entry->acl_ref_cnt)
            {
                CFG_CONFLICT_RET("Can not delete UDF entry %d while used in flow", udf_id);
            }
            PM_E_RETURN(tbl_acl_udf_entry_del_acl_udf_entry(&udf_entry.key));
        }
        break;

    case CDB_OPER_SET:
        PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);

        udf_entry.key.id = udf_id;
        p_db_entry = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
        if (NULL == p_db_entry)
        {
            CFG_CONFLICT_RET("UDF entry %d not exist", udf_id);
        }

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_ACL_UDF_ENTRY_FLD_OFFSET:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);

            PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);

            udf_entry.key.id = udf_id;
            p_db_entry = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
            if (NULL == p_db_entry)
            {
                CFG_CONFLICT_RET("UDF entry %d not exist", udf_id);
            }

            for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
            {
                if (0 == sal_strlen(argv[iter+2]))
                {
                    continue;
                }
                PM_CFG_GET_UINT_RANGE(offset_num, 0, GLB_ACL_UDF_OFFSET_MAX, argv[iter+2], para->p_rs);
                udf_entry.offset[iter] = offset_num;
                udf_entry.valid[iter]  = TRUE;
            }

            rc = acl_api_set_udf_offset(&udf_entry, p_db_entry, para);
            break;

        case TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS:
            PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);

            udf_entry.key.id = udf_id;
            p_db_entry = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
            if (NULL == p_db_entry)
            {
                CFG_CONFLICT_RET("UDF entry %d not exist", udf_id);
            }

            rc = acl_api_set_udf_match(p_db_entry, argv, argc, para);
            break;

        default:
            break;
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _acl_cmd_get_udf_entry(udf_id, para, p_node, &field);
        break;

    default:
        break;
    }

    return rc;    
}

static char *
acl_get_udf_type(int ret)
{
    char *str = NULL;

    switch (ret)
    {
    case GLB_ACL_UDF_OFFSET_L2:
        str = "l2 header";
        break;
    case GLB_ACL_UDF_OFFSET_L3:
        str = "l3 header";
        break;
    case GLB_ACL_UDF_OFFSET_L4:
        str = "l4 header";
        break;
    default:
        str = "N/A";
        break;
    }

    return str;
}


static void
acl_show_udf_entry(cfg_cmd_para_t *para, FILE *fp, tbl_acl_udf_entry_t *p_db_udf)
{
    sal_fprintf(fp, "Udf Index %d\n", p_db_udf->key.id);
    sal_fprintf(fp, "  Udf Type : %s\n", acl_get_udf_type(p_db_udf->offset_type));
    sal_fprintf(fp, "  Udf Match-Field:\n");
    if (p_db_udf->key_flags)
    {
        sal_fprintf(fp, "    ");
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_ETHTYPE))
        {
            sal_fprintf(fp, "ether-type 0x%x 0x%x ", p_db_udf->ethtype, (~p_db_udf->ethtype_mask) & 0xffff);
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_VLANNUM))
        {
            sal_fprintf(fp, "vlan-num %d ", p_db_udf->vlannum);
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_IPPROTO))
        {
            sal_fprintf(fp, "ip-protocol ");
            if (0 == p_db_udf->ipproto_mask)
            {
                sal_fprintf (fp, "any ");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "icmp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_IGMP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "igmp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_SCTP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "sctp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_TCP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "tcp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_UDP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "udp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_GRE == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "gre ");
            }
            else
            {
                sal_fprintf(fp, "%d ", p_db_udf->ipproto);
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_L4_TYPE))
        {
            sal_fprintf(fp, "ip-protocol ");
            if (GLB_ACL_UDF_L4_TYPE_TCP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "tcp ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_UDP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "udp ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_GRE == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "gre ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_SCTP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "sctp ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_RDP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "27 ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_DCCP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "33 ");
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_SRCPORT))
        {
            if (0 == p_db_udf->srcport_mask)
            {
                sal_fprintf(fp, "src-port any ");
            }
            else
            {
                sal_fprintf(fp, "src-port %d ", p_db_udf->srcport);
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_DSTPORT))
        {
             if (0 == p_db_udf->dstport_mask)
            {
                sal_fprintf(fp, "dst-port any ");
            }
            else
            {
                sal_fprintf(fp, "dst-port %d ", p_db_udf->dstport);
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_MPLSNUM))
        {
            sal_fprintf(fp, "mpls-label-num %d ", p_db_udf->mplsnum);
        }
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "    any\n");
    }

    sal_fprintf(fp, "  Offset : ");
    int32 iter = 0;
    for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
    {
#ifdef OFPRODUCT
        sal_fprintf(fp, "%d", p_db_udf->offset[iter]);
#else
        if (p_db_udf->valid[iter])
        {
            sal_fprintf(fp, "%d", p_db_udf->offset[iter]);
        }
        else
        {
            sal_fprintf(fp, "%s", "n/a");
        }
#endif
        if (iter != GLB_ACL_UDF_FIELD_NUM_MAX-1)
        {
            sal_fprintf(fp,"%s", "|");
        }
    }
    sal_fprintf(fp, "\n\n");

    return;
}

#if 0
static void
acl_show_udf_status(cfg_cmd_para_t *para, FILE *fp, tbl_acl_udf_entry_master_t *p_master)
{
    uint32  udf_id = 0;
    tbl_acl_udf_entry_t *p_db_udf = NULL;

    sal_fprintf(fp, "%-8s%-8s%-10s%s\n", "UDF-ID", "Valid", "Type", "Offset-Num");
    sal_fprintf(fp, "-----------------------------------------\n");

    for (udf_id = 0; udf_id < GLB_ACL_UDF_ENTRY_NUM_MAX; udf_id++)
    {
        p_db_udf = p_master->acl_udf_entry_array[udf_id];
        if (!p_db_udf
#ifdef GOLDENGATE
            || 0 == p_db_udf->key_flags
#endif
            )
        {
            sal_fprintf(fp, "%-8d%-8s%-10s%s\n", udf_id, "N", "N/A", "(0 |0 |0 |0 )");
        }
        else
        {
            sal_fprintf(fp, "%-8d%-8s%-10s(%-2d|%-2d|%-2d|%-2d)\n", udf_id, "Y", acl_get_udf_type(p_db_udf->offset_type), 
                        p_db_udf->offset[0], p_db_udf->offset[1], p_db_udf->offset[2], p_db_udf->offset[3]);
        }
    }

    return;
}
#endif

int32
acl_cmd_show_acl_udf(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32   rc     = PM_E_NONE;
    FILE   *fp     = NULL;
    uint32  udf_id = 0;
    tbl_acl_udf_entry_master_t *p_master = NULL;
    tbl_acl_udf_entry_t        *p_db_udf = NULL;

    p_master = tbl_acl_udf_entry_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Udf entry not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        sal_fprintf(fp, "Udf Global Information:\n");
        sal_fprintf(fp, "  Offset Unit : %d Bytes\n\n", GLB_ACL_UDF_GROUP_LENGTH);
 
        for (udf_id = 0; udf_id < GLB_ACL_UDF_ENTRY_NUM_MAX; udf_id++)
        {
            p_db_udf = p_master->acl_udf_entry_array[udf_id];
            if (!p_db_udf)
            {
                continue;
            }

            acl_show_udf_entry(para, fp, p_db_udf);
        }
    }
    else
    {
    /* optimize show udf remove udf status */
#if 0
        if (!sal_strcmp(argv[0], "status"))
        {
            acl_show_udf_status(para, fp, p_master);
        }
        else
#endif
        {
            PM_CFG_GET_UINT(udf_id, argv[0], para->p_rs);
            p_db_udf = p_master->acl_udf_entry_array[udf_id];

            if (!p_db_udf)
            {
                sal_fclose(fp);
                fp = NULL;
                CFG_CONFLICT_RET("Udf entry %d not exist", udf_id);
            }

            acl_show_udf_entry(para, fp, p_db_udf);
        }
    }

    sal_fclose(fp);
    fp = NULL;

    return rc;
}


int32
ace_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_global_t* p_glb = tbl_sys_global_get_sys_global();
    cdb_node_t* p_node   = NULL;
    char *acl_name       = NULL;
    uint32 seq_no        = 0;
    bool deny_packet     = FALSE;
    int rc               = 0;
    uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
  #ifdef TAPPRODUCT 
    uint32 ether_type = 0;
    uint32 ether_type_mask = 0;
    char output_str[100];
    uint8 i = 0;
  #endif
    uint32 argc_idx  = 0;
    int32 type = GLB_ACL_CONFIG_TYPE_NORMAL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ACE_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if (argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        acl_name = argv[0];
        PM_CFG_GET_UINT(seq_no, argv[1], para->p_rs);
        deny_packet = (0 == sal_strcmp(argv[2], "permit")) ? FALSE : TRUE;

        #ifdef TAPPRODUCT
        rc = acl_config_add_ace_tap(acl_name, &seq_no, deny_packet, argv+3, argv, argc, para);    
        #else
        rc = acl_config_add_ace(acl_name, &seq_no, deny_packet, argv+3, argv, argc, para);
        #endif
        if (PM_E_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Seq %d should be removed first", seq_no);
        }
        else if (PM_E_INIT_FAIL == rc)
        {
            CFG_INVALID_PARAM_RET("The entry with same key has already existed");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The gt port num max is 65533, the lt port num min is 2");
        }
        else if (PM_E_CFG_ERR_UDF_TYPE == rc)
        {
            CFG_INVALID_PARAM_RET("Can't configure this udf head type, confilct with other");
        }
        else if (PM_E_CFG_ERR_UDF_VALUE == rc)
        {
            CFG_INVALID_PARAM_RET("The udf head match must ether_type or l3-protocl with mask 0");
        }
        else if (PM_E_CFG_DB_FAIL == rc)
        {
            CFG_INVALID_PARAM_RET("The second port num should be greater than the first");
        }
        else if (PM_E_CFG_NO_RESOURCE == rc)
        {
            if(check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
            {
                CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
                                    stm_max_ingress, stm_max_egress);
                
            }
            else
            {
                CFG_INVALID_PARAM_RET("The max hardware ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
                                    stm_max_ingress, 
                                    stm_max_egress,
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
            }
        }
        else if (PM_E_NOT_PERMIT == rc)
        {
            CFG_INVALID_PARAM_RET("Exceeded maximum sequence number");
        }
        else if (PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
        }
        else if (PM_E_CFG_ERR_UDF_NO_RESPURCE == rc)
        {
            CFG_INVALID_PARAM_RET("System only support 1 ipv4-head udf or 3 l4-head udf match");
        }
        else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
        {
            CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
        }
        else if (PM_E_CFG_ACL_TCP_FLAGS_NONE == rc)
        {
#ifdef TAPPRODUCT        
            CFG_INVALID_PARAM_RET("Tcp-code must be configured any of ack or fin or psh or rst or syn or urg");
#else
            CFG_INVALID_PARAM_RET("Tcp-flag must be configured any of ack or fin or psh or rst or syn or urg");
#endif
        }
        else if (PM_E_CFG_PROMPT == rc)
        {
            CFG_INVALID_PARAM_RET("DSCP conflict with Ip-precedence field");
        }
        else if (PM_E_CFG_PM_ERROR == rc)
        {
#ifdef TAPPRODUCT        
            CFG_INVALID_PARAM_RET("Not support ether_type configuration in egress flow entry on this product");
#else
            CFG_INVALID_PARAM_RET("Not support ether_type configuration in egress ace on this product");
#endif
        }
        else if (PM_E_EXCEED_TAP_GRP_TRCA_MAX_NUM == rc)
        {
            CFG_INVALID_PARAM_RET("Truncation resources is full");
        }
        else if (PM_E_CFG_ACL_VNI_CONFIG_CONFLICT == rc)
        {
            CFG_INVALID_PARAM_RET("Vxlan-VNI only match vxlan dst-port type is eq");
        }
        else if (PM_E_CFG_ACL_STRIP_HEADER_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("Unknow packet type, only gre/nvgre/vxlan support strip-header");
        }
        else if (PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT == rc)
        {
            CFG_INVALID_PARAM_RET("Trunction conflict");
        }
        else if (PM_E_CFG_ACL_MAKR_SOURCE_UNTAG_CONFIG_CONFLICT == rc)
        {
            CFG_INVALID_PARAM_RET("Make-source untag conflict");
        }
        else if (PM_E_NOT_SUPPORT == rc)
        {
            CFG_INVALID_PARAM_RET("This function not support");
        }
        else if (PM_E_CFG_ACL_SERVICE_ATTACHED_CONFLICT == rc)
        {
            CFG_CONTINUE_RET("Failed to update ACL: ACL is attached to telnet or ssh, should specify protocol TCP and source IP with sequential mask, destination IP is optional");
        }

        /*ip access-list: arp any in GB*/
        if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
        {
            if (argc > 44)
            {
                if ((PM_E_NONE == rc) && (0 == sal_strcmp(argv[43], "any")))
                {
                    if (p_glb->startup_done)
                    {
                        CFG_PROMPT_RET("Notice: it can not support the rarp-request field and rarp-response field");
                    }
                }  
            } 
        }
        /*ended*/

        #ifdef TAPPRODUCT 
        if ((PM_E_NONE == rc) && (0 == sal_strcmp(argv[30], "any")))
        {
            if (p_glb->startup_done)
            {       
                CFG_PROMPT_RET("0x0800 0x8847 0x8848 0x0806 0x8906 0x22F3 0x8902 0x88E7 0x8809 0x88F7 should be added independently");
            }
        }
        else if ((PM_E_NONE == rc) && (0 == sal_strcmp(argv[30], "ether_type")))
        {
            PM_CFG_GET_UINT(ether_type, argv[31], para->p_rs);
            PM_CFG_GET_UINT(ether_type_mask, argv[32], para->p_rs);
            sal_memset(output_str, 0, sizeof(output_str));
            ether_type_mask = 0xffff - ether_type_mask;
            if (0xffff != ether_type_mask)
            {
                if ((0x0800 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x0800 ", 7);
                    i += 7;
                }
                if ((0x86DD & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x86DD ", 7);
                    i += 7;
                }
                if ((0x8847 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x8847 ", 7);
                    i += 7;
                }
                if ((0x8848 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x8848 ", 7);
                    i += 7;
                }
                if ((0x0806 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x0806 ", 7);
                    i += 7;
                }
                if ((0x8906 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x8906 ", 7);
                    i += 7;
                }
                if ((0x22F3 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x22F3 ", 7);
                    i += 7;
                }
                if ((0x8902 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x8902 ", 7);
                    i += 7;
                }
                if ((0x88E7 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x88E7 ", 7);
                    i += 7;
                }
                if ((0x8809 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x8809 ", 7);
                    i += 7;
                }
                if ((0x88F7 & ether_type_mask) == (ether_type & ether_type_mask))
                {
                    sal_strncpy(&output_str[i], "0x88F7 ", 7);
                    i += 7;
                }
                if (0 != i)
                {
                    if (p_glb->startup_done)
                    {
                        CFG_PROMPT_RET("%sshould be added independently", output_str);
                    }
                }
            }
        }
        #endif
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 4, 6, para->p_rs);
        acl_name = argv[0];
        PM_CFG_GET_UINT(seq_no, argv[1], para->p_rs);
        if (0 != sal_strlen(argv[3]))
        {
            rc = acl_config_del_ace(acl_name, seq_no, argv, argc, TRUE);
        }
        else
        {
            rc = acl_config_del_ace(acl_name, seq_no, argv, argc, FALSE);
        }
        
        if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The %s does not exist", glb_acl_type_str[type]);
        }
        if (PM_E_NOT_EXIST == rc)
        {
#ifdef TAPPRODUCT        
            CFG_INVALID_PARAM_RET("The flow entry filter does not exist");
#else
            CFG_INVALID_PARAM_RET("The ACE filter does not exist");
#endif
        }
        break;

    case CDB_OPER_GET:
        _ace_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
acl_worm_filter_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    char *acl_worm_fliter_name = NULL;
    uint8 istcp;
    uint32 dstport;
    uint32 isrange = 0;
    int rc = PM_E_NONE;

    #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
    #endif

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ACL_WORM_FILTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        /* modified by liwh for bug 52282, 2018-06-06 */
        //PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        /* liwh end */
        acl_worm_fliter_name = argv[0];
        PM_CFG_GET_UINT(istcp, argv[1], para->p_rs);
        PM_CFG_GET_UINT(dstport, argv[2], para->p_rs);

        /* modified by liwh for bug 52282, 2018-06-06 */
        if (4 == argc)
        {
            PM_CFG_GET_UINT(isrange, argv[3], para->p_rs);
        }
        /* liwh end */

        rc = acl_worm_filter_add(acl_worm_fliter_name, istcp, dstport, isrange);

        if (PM_E_EXCEED_MAX_NUM == rc)
        {
            CFG_CONFLICT_RET("The Worm Filter resource is full");
        }
        else if (PM_E_EXIST == rc)
        {
            CFG_CONFLICT_RET("The Worm Filter has already existed");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of worm filter should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_CFG_NO_RESOURCE == rc)
        {
            CFG_CONFLICT_RET("The Worm Filter range l4-destport resource is full");
        }
        else if (PM_E_NONE != rc)
        {
            CFG_CONFLICT_RET("Worm Filter create failed");
        }

        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        acl_worm_fliter_name = argv[0];

        rc = acl_worm_filter_del(acl_worm_fliter_name);
        if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of worm filter should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_NONE != rc)
        {
            CFG_CONFLICT_RET("Worm Filter remove failed");
        }

        break;

    case CDB_OPER_GET:
        _acl_worm_filter_read_session_all(para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
acl_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *acl_name       = NULL;
    uint32 argc_idx  = 0;
    int32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ACL_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    /* get acl_config type, normal acl or flow or ipv6 acl or mac acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        acl_name = argv[0];

        rc = acl_config_add_acl(acl_name, argv, argc, para);
        break;

    case CDB_OPER_SET:
        acl_name = argv[0];
        if (0 == sal_strcmp(argv[1], "remark"))
        {
            if (0 == sal_strcmp(argv[2], "add"))
            {
                if (argc >3)
                {
                    rc = acl_config_set_remark(acl_name, argv, argc, argv[3]);
                }
            }
            else if (0 == sal_strcmp(argv[2], "del"))
            {
                rc = acl_config_set_remark(acl_name, argv, argc, NULL);
            }
            else
            {        
                CFG_INVALID_PARAM_RET("Unknown %s remark operation : %s", glb_acl_type_str[type], argv[2]);            
            }
            if (PM_E_INVALID_PARAM == rc)
            {
                CFG_CONFLICT_RET("The initial character of %s remark should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 100.",
                            glb_acl_type_str[type]);
            }
            /*modified by yej for bug 54155, 2019-10-29*/
            else if (PM_E_ACL_NOT_EXIST == rc)
            {
                CFG_CONFLICT_RET("%s should be configured first", glb_acl_type_str[type]);
            }
        }
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 3, para->p_rs);
        acl_name = argv[0];

        rc = acl_config_del_acl(acl_name, argv, argc, para);
        break;

    case CDB_OPER_GET:
        _acl_cmd_read_session_all(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
acl_cmd_show_acl(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_acl_config_key_t acl_config_key;
    tbl_acl_config_t *p_db_acl_config = NULL;
    
    int32 rc = PM_E_NONE;
    uint32 argc_idx  = 0;
    uint32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    
    /* get acl_config type, flow or ip/mac/ipv6 acl */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }

    /*added by yejl for ipv6 acl*/
    if ((FALSE == acl_stm_is_ipv6_profile())&& 
        (type == GLB_ACL_CONFIG_TYPE_IPV6))
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    /*ended by yejl*/
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        if (sal_strlen(argv[0]) != 0)
        {
            if (check_name_character_and_len(argv[0], ACL_NAME_SIZE) != 0)
            {
                CFG_CONFLICT_RET("The initial character of %s name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20",
                    glb_acl_type_str[type]);
            }
            sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
            acl_generate_acl_config_key_name_by_type(argv[0], type, acl_config_key.name);
            p_db_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
            if (NULL == p_db_acl_config || p_db_acl_config->type_identifying != type)
            {
                CFG_CONFLICT_RET("The %s %s does not exist", glb_acl_type_str[type], argv[0]);
            }
            acl_show_acl(p_db_acl_config, type, para);
        }
        else
        {
            acl_show_acl(NULL, type, para);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}


int32
acl_cmd_show_timerange_info(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_time_range_t *p_db_time_range = NULL;
    tbl_time_range_key_t time_range_key;  
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (argc == 1)
        {
            if (check_name_character_and_len(argv[0], TIME_RANGE_SIZE) != 0)
            {
                CFG_CONFLICT_RET("The initial character of time-range name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
            }
            sal_memset(&time_range_key, 0, sizeof(time_range_key));
            sal_strcpy(time_range_key.name, argv[0]);
            p_db_time_range = tbl_time_range_get_time_range(&time_range_key);
            if (NULL == p_db_time_range)
            {
                CFG_CONFLICT_RET("Time range %s does not exist", argv[0]);
            }
            time_range_show_info(p_db_time_range, para);
        }
        else
        {
            time_range_show_info(NULL, para);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

static int32
_acl_cmd_time_range_add_time_range(char *range_name, cfg_cmd_para_t *para)
{
    tbl_time_range_t time_range;
    tbl_time_range_t *p_time_range = &time_range;
    tbl_time_range_t *p_db_time_range = NULL;
    int32 rc = PM_E_NONE;
    
    if (NULL == range_name || (check_name_character_and_len(range_name, TIME_RANGE_NAME_SIZE) != 0))
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_memset(p_time_range, 0, sizeof(tbl_time_range_t));
    sal_strncpy(p_time_range->key.name, range_name, TIME_RANGE_NAME_SIZE);

    /* 2. check exist */
    p_db_time_range = tbl_time_range_get_time_range(&p_time_range->key);
    if (p_db_time_range)
    {
        return PM_E_NONE;
    }

    if (tbl_time_range_get_time_range_count() >= GLB_TIME_RANGE_MAX_NUM)
    {
        return PM_E_EXCEED_MAX_NUM;
    }

    rc = acl_time_range_add_time_range(p_time_range);

    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add time range, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32
_acl_cmd_time_range_del_time_range(char *range_name, cfg_cmd_para_t *para)
{
    tbl_time_range_t time_range;
    tbl_time_range_t *p_time_range = &time_range;
    tbl_time_range_t *p_db_time_range = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == range_name || (check_name_character_and_len(range_name, TIME_RANGE_NAME_SIZE) != 0))
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    sal_memset(p_time_range, 0, sizeof(tbl_time_range_t));
    sal_strncpy(p_time_range->key.name, range_name, TIME_RANGE_NAME_SIZE);

    /* 2. check exist */
    p_db_time_range = tbl_time_range_get_time_range(&p_time_range->key);
    if (!p_db_time_range)
    {
        CFG_FUNC_ERR_RET("Can't find time-range (%s)", range_name);
    }

    if (p_db_time_range->ref_cnt)
    {
        CFG_FUNC_ERR_RET("Time range is attached");        
    }
    rc = acl_time_range_del_time_range(p_time_range);

    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Del time range, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32
_acl_cmd_time_range_get_time_range_all(char *range_name, cfg_cmd_para_t *para, 
                                                    cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_time_range_key_t key;
    tbl_time_range_t *p_db_time_range = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == range_name)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_time_range_dump_one, &args);
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.name, range_name, TIME_RANGE_NAME_SIZE);
        p_db_time_range = tbl_time_range_get_time_range(&key);
        if (!p_db_time_range)
        {
            para->p_show->rc = PM_E_NOT_EXIST;
            sal_fclose(fp);
            return PM_E_NOT_EXIST;
        }
        tbl_time_range_dump_one(p_db_time_range, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_acl_cmd_time_range_get_time_range(char *range_name, cfg_cmd_para_t *para, 
                                                    cdb_node_t *p_node, field_parse_t *p_field)
{
    int32 rc = PM_E_NONE;
    
    rc = _acl_cmd_time_range_get_time_range_all(range_name, para, p_node, p_field);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Can't find time range (%s)", range_name);
    }

    return PM_E_NONE;
}


int32
acl_cmd_process_time_range(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    char* range_name = NULL;
    int32 rc = PM_E_NONE;
    tbl_time_range_t time_range;
    tbl_time_range_t *p_db_time_range = NULL;
    time_range_value_t time;
    int i = 0;
    char *time_str;
    char *day_str;
    char *month_str;
    char *year_str;
    
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    field_parse_t field;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_TIME_RANGE);

    /*modified by yejl for bug 54199, 2019-10-31*/
    if (argc > 0)
    {
        range_name = argv[0];
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = _acl_cmd_time_range_add_time_range(range_name, para);
        if (PM_E_EXCEED_MAX_NUM == rc)
        {
            CFG_CONFLICT_RET("The max time range number is %d", GLB_TIME_RANGE_MAX_NUM);
        }
        if (PM_E_CFG_INVALID_PARAM == rc)
        {
            CFG_CONFLICT_RET("The initial character of timerange name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 40");
        }
        break;

    case CDB_OPER_DEL:
        rc = _acl_cmd_time_range_del_time_range(range_name, para);
        if (PM_E_CFG_INVALID_PARAM == rc)
        {
            CFG_CONFLICT_RET("The initial character of timerange name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 40");
        }
        break;
    case CDB_OPER_SET:
        sal_memset(&time_range, 0, sizeof(time_range));
        sal_strncpy(time_range.key.name, range_name, TIME_RANGE_NAME_SIZE);

        /* check exist */
        p_db_time_range = tbl_time_range_get_time_range(&time_range.key);
        if (!p_db_time_range)
        {
            CFG_CONFLICT_RET("Time range %s not found", time_range.key.name);
        }
        if (p_db_time_range->ref_cnt)
        {
            CFG_CONFLICT_RET("Time range is attached");
        }
        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[2]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        switch (field_id)
        {
        case TBL_TIME_RANGE_FLD_TIME:
            sal_memset(&time, 0 , sizeof(time));
            
            if (0 == sal_strcmp(argv[2], "absolute"))
            {
                time.type = GLB_TIME_RANGE_TYPE_ABSOLUTE;
                i  = 3;
                /* start time */
                time_str = argv[i++];
                month_str = argv[i++];                
                day_str = argv[i++];                
                year_str = argv[i++];
                if (sal_strlen(time_str))
                {
                    time.ab_time.start = cdb_time_range_str2time(time_str, day_str, month_str, year_str);

                    if (-2 == time.ab_time.start )
                    {
                        CFG_INVALID_PARAM_RET("Invalid month letters(%s)", month_str);
                    }
                    else if (-3 == time.ab_time.start )
                    {
                        CFG_INVALID_PARAM_RET("Invalid date(%s): doesn't exist", day_str);
                    }
                    else if (time.ab_time.start < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid command");
                    }
                }
                
                /* end time */
                time_str = argv[i++];
                month_str = argv[i++];                
                day_str = argv[i++];                
                year_str = argv[i++];
                if (sal_strlen(time_str))
                {
                    time.ab_time.end = cdb_time_range_str2time(time_str, day_str, month_str, year_str);

                    if (-2 == time.ab_time.end )
                    {
                        CFG_INVALID_PARAM_RET("Invalid month letters(%s)", month_str);
                    }
                    else if (-3 == time.ab_time.end )
                    {
                        CFG_INVALID_PARAM_RET("Invalid date(%s): doesn't exist", day_str);
                    }
                    else if (time.ab_time.end < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid command");
                    }

                    if (time.ab_time.start >= time.ab_time.end)
                    {
                        CFG_INVALID_PARAM_RET("The start time is later than end time");                        
                    }
                }
            }
            else if (0 == sal_strcmp(argv[2], "periodic"))
            {
                time.type = GLB_TIME_RANGE_TYPE_PERIODIC;
                i = 3;    
                if (0 == sal_strcmp(argv[i], "weekdays"))
                {
                    time.per_time.start.day = WEEKDAY;
                    time.per_time.end.day = WEEKDAY;
                }
                else if (0 == sal_strcmp(argv[i], "weekend"))
                {
                    time.per_time.start.day = WEEKEND;
                    time.per_time.end.day = WEEKEND;
                }
                else if (0 == sal_strcmp(argv[i], "daily"))
                {
                    time.per_time.start.day = DAILY;
                    time.per_time.end.day = DAILY;
                }
                else
                {
                    /* get start day */
                    if (cdb_get_weekday(argv[i+2], &time.per_time.start.day) < 0)
                    {
                        CFG_INVALID_PARAM_RET("The weekday(%s) is wrong", argv[i+2]);                        
                    }
                    /* get end day, if exist */
                    if (sal_strlen(argv[i+2+2]) > 0 )
                    {
                        if (cdb_get_weekday(argv[i+2+2], &time.per_time.end.day) < 0)
                        {
                            CFG_INVALID_PARAM_RET("The weekday(%s) is wrong", argv[i+2+2]);                        
                        }
                    }
                    else
                    {
                        time.per_time.end.day = time.per_time.start.day;
                    }
                }
                /* start time */
                i = 4;
                if (cdb_time_range_get_time(argv[i], &time.per_time.start.hour, 
                                                &time.per_time.start.min) < 0)
                {
                    CFG_INVALID_PARAM_RET("Start time(%s) is wrong", argv[i]);
                }
                /* end time */
                i = 6;
                if (cdb_time_range_get_time(argv[i], &time.per_time.end.hour, 
                                                &time.per_time.end.min) < 0)
                {
                    CFG_INVALID_PARAM_RET("End time(%s) is wrong", argv[i]);
                }     

                if ((time.per_time.end.hour == time.per_time.start.hour)
                        && (time.per_time.end.min == time.per_time.start.min)
                        && (time.per_time.end.day == time.per_time.start.day))
                {
                    CFG_INVALID_PARAM_RET("The end time should be different from start time"); 
                }
                if ((time.per_time.end.day == time.per_time.start.day &&
                     time.per_time.start.day != DAILY)
                    &&((time.per_time.end.hour < time.per_time.start.hour)
                      ||(time.per_time.end.hour == time.per_time.start.hour &&
                         time.per_time.end.min <= time.per_time.start.min)))
                {
                    CFG_INVALID_PARAM_RET("Ending time must be greater than starting time");
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unkown time range type(%s) ", argv[2]);            
            }            
           
            sal_memcpy(&time_range.time, &time, sizeof(time));
            rc = tbl_time_range_set_time_range_field(&time_range, TBL_TIME_RANGE_FLD_TIME);
            if (PM_E_NONE != rc)
            {
                CFG_INVALID_PARAM_RET("Set time range failed, %s", pm_err_desc(rc));          
            }
            
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[2]);
            break;
            
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _acl_cmd_time_range_get_time_range(range_name, para, p_node, &field);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}




