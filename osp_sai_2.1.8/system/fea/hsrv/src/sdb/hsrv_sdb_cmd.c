

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "sdb.h"
#include "gen/stbl_sai_glb_define.h"
#include "gen/stbl_sai_glb.h"
#include "gen/stbl_port_glb_define.h"
#include "gen/stbl_port_glb.h"
#include "gen/stbl_port_define.h"
#include "gen/stbl_port.h"
#include "gen/stbl_lag_memport_define.h"
#include "gen/stbl_lag_memport.h"
#include "gen/stbl_hostif_define.h"
#include "gen/stbl_hostif.h"
#include "gen/stbl_vlan_define.h"
#include "gen/stbl_vlan.h"
#include "gen/stbl_vlan_class_define.h"
#include "gen/stbl_vlan_class.h"
#include "gen/stbl_rif_define.h"
#include "gen/stbl_rif.h"
#include "gen/stbl_vr_define.h"
#include "gen/stbl_vr.h"
#include "gen/stbl_vrrp_vip_define.h"
#include "gen/stbl_vrrp_vip.h"
#include "gen/stbl_neighbor_define.h"
#include "gen/stbl_neighbor.h"
#include "gen/stbl_nexthop_define.h"
#include "gen/stbl_nexthop.h"
#include "gen/stbl_nexthop_group_define.h"
#include "gen/stbl_nexthop_group.h"
#include "gen/stbl_route_define.h"
#include "gen/stbl_route.h"
#include "gen/stbl_stp_define.h"
#include "gen/stbl_stp.h"
#include "gen/stbl_l2mc_define.h"
#include "gen/stbl_l2mc.h"
#include "gen/stbl_copp_group_define.h"
#include "gen/stbl_copp_group.h"
#include "gen/stbl_copp_trap_define.h"
#include "gen/stbl_copp_trap.h"
#include "gen/stbl_copp_reason_define.h"
#include "gen/stbl_copp_reason.h"
#include "gen/stbl_copp_entry_define.h"
#include "gen/stbl_copp_entry.h"
#include "gen/stbl_acl_glb_define.h"
#include "gen/stbl_acl_glb.h"
#include "gen/stbl_acl_port_define.h"
#include "gen/stbl_acl_port.h"
#include "gen/stbl_acl_tbl_define.h"
#include "gen/stbl_acl_tbl.h"
#include "gen/stbl_acl_entry_define.h"
#include "gen/stbl_acl_entry.h"
#include "gen/stbl_acl_tunnel_define.h"
#include "gen/stbl_acl_tunnel.h"
#include "gen/stbl_acl_udf_entry_define.h"
#include "gen/stbl_acl_udf_entry.h"
#include "gen/stbl_nat_forward_define.h"
#include "gen/stbl_nat_forward.h"
#include "gen/stbl_cfm_lmep_define.h"
#include "gen/stbl_cfm_lmep.h"
#include "gen/stbl_cfm_rmep_define.h"
#include "gen/stbl_cfm_rmep.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

static int32
_hsrv_sdb_cmd_process_sai_glb(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_sai_glb_t *p_db_glb = stbl_sai_glb_get_sai_glb();
    stbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    stbl_sai_glb_dump_one(p_db_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_sai_glb(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_process_sai_glb(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_port_glb(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_port_glb_t *p_db_glb = stbl_port_glb_get_port_glb();
    stbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    stbl_port_glb_dump_one(p_db_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_port_glb(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_port_glb(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_vr(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    stbl_vr_iterate((STBL_ITER_CB_FUNC)stbl_vr_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_vr(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_vr(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}
    
static int32
_hsrv_sdb_cmd_get_port(sai_object_id_t port_oid, cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    stbl_port_key_t key;
    stbl_port_t *p_sdb_port = NULL;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    if (0 == port_oid)
    {
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[1] = fp;
        stbl_port_iterate((STBL_ITER_CB_FUNC)stbl_port_dump_one, &args);
        sal_fclose(fp);
    }
    else
    {
        key.port_oid = port_oid;
        p_sdb_port = stbl_port_get_port(&key);
        if (NULL == p_sdb_port)
        {
            CFG_CONFLICT_RET("Cannot find the port");
        }
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[1] = fp;
        stbl_port_dump_one(p_sdb_port, &args);
        sal_fclose(fp);
    }

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_port(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;
    sai_object_id_t port_oid = 0;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc > 0)
        {
            PM_CFG_GET_UINT64(port_oid, argv[0], para->p_rs);
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_port(port_oid, para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_hostif(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_hostif_iterate((STBL_ITER_CB_FUNC)stbl_hostif_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_hostif(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_hostif(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_lag_memport(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_lag_memport_iterate((STBL_ITER_CB_FUNC)stbl_lag_memport_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_lag_memport(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_lag_memport(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_vlan(vid_t vid, cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    stbl_vlan_key_t key;
    stbl_vlan_t *p_sdb_vlan = NULL;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    if (0 == vid)
    {
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[1] = fp;
        stbl_vlan_iterate((STBL_ITER_CB_FUNC)stbl_vlan_dump_one, &args);
        sal_fclose(fp);
    }
    else
    {
        key.vid = vid;
        p_sdb_vlan = stbl_vlan_get_vlan(&key);
        if (NULL == p_sdb_vlan)
        {
            CFG_CONFLICT_RET("Cannot find the vlan");
        }
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[1] = fp;
        stbl_vlan_dump_one(p_sdb_vlan, &args);
        sal_fclose(fp);
    }

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_vlan(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;
    vid_t vid = 0;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc > 0)
        {
            PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_vlan(vid, para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_vlan_class(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_vlan_class_iterate((STBL_ITER_CB_FUNC)stbl_vlan_class_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_vlan_class(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_vlan_class(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_rif(uint32 rif_id, cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_rif_iterate((STBL_ITER_CB_FUNC)stbl_rif_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_rif(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;
    uint32 rif_id = 0;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc > 0)
        {
            PM_CFG_GET_UINT(rif_id, argv[0], para->p_rs);
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_rif(rif_id, para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_nb(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_neighbor_iterate((STBL_ITER_CB_FUNC)stbl_neighbor_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_nb(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_nb(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_nh(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_nexthop_iterate((STBL_ITER_CB_FUNC)stbl_nexthop_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_nh(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_nh(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_nh_group(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_nexthop_group_iterate((STBL_ITER_CB_FUNC)stbl_nexthop_group_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_nh_group(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_nh_group(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_route(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_route_iterate((STBL_ITER_CB_FUNC)stbl_route_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_route(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_route(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_vrrp_vip(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_vrrp_vip_iterate((STBL_ITER_CB_FUNC)stbl_vrrp_vip_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_vrrp_vip(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_vrrp_vip(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_stp(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_stp_iterate((STBL_ITER_CB_FUNC)stbl_stp_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_stp(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_stp(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_l2mc(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_l2mc_iterate((STBL_ITER_CB_FUNC)stbl_l2mc_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_l2mc(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_l2mc(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_trap(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_copp_trap_iterate((STBL_ITER_CB_FUNC)stbl_copp_trap_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_trap(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_trap(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_group(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_copp_group_iterate((STBL_ITER_CB_FUNC)stbl_copp_group_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_group(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_group(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_reason(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_copp_reason_iterate((STBL_ITER_CB_FUNC)stbl_copp_reason_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_reason(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_reason(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_acl_glb(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_acl_glb_t *p_db_glb = stbl_acl_glb_get_acl_glb();
    stbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    stbl_acl_glb_dump_one(p_db_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_acl_glb(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_acl_glb(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_acl_port(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_acl_port_iterate((STBL_ITER_CB_FUNC)stbl_acl_port_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_acl_port(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_acl_port(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_acl_tbl(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_acl_tbl_iterate((STBL_ITER_CB_FUNC)stbl_acl_tbl_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_acl_tbl(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_acl_tbl(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_acl_entry(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_acl_entry_iterate((STBL_ITER_CB_FUNC)stbl_acl_entry_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_acl_entry(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_acl_entry(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_acl_udf_entry(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_acl_entry_iterate((STBL_ITER_CB_FUNC)stbl_acl_udf_entry_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_acl_udf_entry(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_acl_udf_entry(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_acl_tunnel(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_acl_tunnel_iterate((STBL_ITER_CB_FUNC)stbl_acl_tunnel_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_acl_tunnel(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_acl_tunnel(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_nat_forward(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_nat_forward_iterate((STBL_ITER_CB_FUNC)stbl_nat_forward_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_nat_forward(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_nat_forward(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_cfm_lmep(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_cfm_lmep_iterate((STBL_ITER_CB_FUNC)stbl_cfm_lmep_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_cfm_lmep(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_cfm_lmep(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_hsrv_sdb_cmd_get_cfm_rmep(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    stbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    args.argv[1] = fp;
    stbl_cfm_rmep_iterate((STBL_ITER_CB_FUNC)stbl_cfm_rmep_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_sdb_cmd_process_cfm_rmep(char **argv, int32 argc, void *void_para)
{
    cfg_cmd_para_t *para = void_para;
    field_parse_t field;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _hsrv_sdb_cmd_get_cfm_rmep(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_sdb_cmd_init()
{
    sdb_register_cfg_tbl_cb(STBL_SAI_GLB,       hsrv_sdb_cmd_process_sai_glb);
    sdb_register_cfg_tbl_cb(STBL_PORT_GLB,      hsrv_sdb_cmd_process_port_glb);
    sdb_register_cfg_tbl_cb(STBL_PORT,          hsrv_sdb_cmd_process_port);
    sdb_register_cfg_tbl_cb(STBL_HOSTIF,        hsrv_sdb_cmd_process_hostif);
    sdb_register_cfg_tbl_cb(STBL_LAG_MEMPORT,   hsrv_sdb_cmd_process_lag_memport);
    sdb_register_cfg_tbl_cb(STBL_VLAN,          hsrv_sdb_cmd_process_vlan);
    sdb_register_cfg_tbl_cb(STBL_VLAN_CLASS,    hsrv_sdb_cmd_process_vlan_class);
    sdb_register_cfg_tbl_cb(STBL_VR,            hsrv_sdb_cmd_process_vr);
    sdb_register_cfg_tbl_cb(STBL_RIF,           hsrv_sdb_cmd_process_rif);
    sdb_register_cfg_tbl_cb(STBL_NEIGHBOR,      hsrv_sdb_cmd_process_nb);
    sdb_register_cfg_tbl_cb(STBL_NEXTHOP,       hsrv_sdb_cmd_process_nh);
    sdb_register_cfg_tbl_cb(STBL_NEXTHOP_GROUP, hsrv_sdb_cmd_process_nh_group);
    sdb_register_cfg_tbl_cb(STBL_ROUTE,         hsrv_sdb_cmd_process_route);
    sdb_register_cfg_tbl_cb(STBL_VRRP_VIP,      hsrv_sdb_cmd_process_vrrp_vip);
    sdb_register_cfg_tbl_cb(STBL_STP,           hsrv_sdb_cmd_process_stp);
    sdb_register_cfg_tbl_cb(STBL_L2MC,          hsrv_sdb_cmd_process_l2mc);
    sdb_register_cfg_tbl_cb(STBL_COPP_TRAP,     hsrv_sdb_cmd_process_trap);
    sdb_register_cfg_tbl_cb(STBL_COPP_GROUP,    hsrv_sdb_cmd_process_group);
    sdb_register_cfg_tbl_cb(STBL_COPP_REASON,   hsrv_sdb_cmd_process_reason);
    sdb_register_cfg_tbl_cb(STBL_ACL_GLB,       hsrv_sdb_cmd_process_acl_glb);
    sdb_register_cfg_tbl_cb(STBL_ACL_PORT,      hsrv_sdb_cmd_process_acl_port);
    sdb_register_cfg_tbl_cb(STBL_ACL_TBL,       hsrv_sdb_cmd_process_acl_tbl);
    sdb_register_cfg_tbl_cb(STBL_ACL_ENTRY,     hsrv_sdb_cmd_process_acl_entry);
    sdb_register_cfg_tbl_cb(STBL_ACL_TUNNEL,    hsrv_sdb_cmd_process_acl_tunnel);
    sdb_register_cfg_tbl_cb(STBL_ACL_UDF_ENTRY, hsrv_sdb_cmd_process_acl_udf_entry);
    sdb_register_cfg_tbl_cb(STBL_NAT_FORWARD,   hsrv_sdb_cmd_process_nat_forward);/* added by taocy for NAT 2019-06-13*/
    sdb_register_cfg_tbl_cb(STBL_CFM_LMEP, hsrv_sdb_cmd_process_cfm_lmep);
    sdb_register_cfg_tbl_cb(STBL_CFM_RMEP, hsrv_sdb_cmd_process_cfm_rmep);

    return HSRV_E_NONE;
}

