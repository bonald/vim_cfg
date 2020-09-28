
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "track.h"
#include "track_api.h"
#include "ipsla.h"
#include "vrrp.h"
#include "vrrp_api.h"


static int32
_track_cmd_get_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;
    tbl_track_global_t     *p_db_track_glb = NULL;
    int32                   field_id = 0;
    field_parse_t           field;

    sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        CFG_CONFLICT_RET("Tbl_track_global not found");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;

    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    tbl_track_global_dump_one(p_db_track_glb, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
track_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    cdb_node_t            *p_node = NULL;
    tbl_track_global_t     track_glb;
    tbl_track_global_t    *p_db_track_glb = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field = NULL;
    int32                  value = 0;
    int32                  i = 0;

    CFG_DBG_FUNC();

    sal_memset(&track_glb, 0, sizeof(tbl_track_global_t));

    p_node = cdb_get_tbl(TBL_TRACK_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_db_track_glb = tbl_track_global_get_track_global();
        if (!p_db_track_glb)
        {
            CFG_CONFLICT_RET("Tbl_track_global not found");
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

        switch (field_id)
        {
        case TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            p_db_track_glb->track_timer_interval = value;
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _track_cmd_get_global(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_track_cmd_create_object(char **argv, int32 argc, uint32 object_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_track_object_key_t    object_key;
    tbl_track_object_t       *p_db_object    = NULL;
    tbl_track_global_t       *p_db_track_glb = NULL;
    char    track_type[20];
    char    track_intf[IFNAME_SIZE];
    uint32  rtr_entry_id = 0;
    char    rtr_type[20];

    sal_memset(&object_key, 0, sizeof(object_key));

    if (TRACK_OBJECTID_MIN > object_id || TRACK_OBJECTID_MAX < object_id)
    {
        CFG_CONFLICT_RET("Track parameter illegal");
    }

    object_key.trackobj_id = object_id;
    p_db_object = tbl_track_object_get_track_object(&object_key);

    if (argc == 1)
    {
        if (!p_db_object)
        {
            CFG_CONFLICT_RET("Track object is not exist");
        }
        return PM_E_NONE;
    }

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        CFG_CONFLICT_RET("Tbl_track_global not found");
    }

    /* check track object number */
    if (!p_db_object && p_db_track_glb->track_object_num >= GLB_TRACK_OBJECT_MAX)
    {
        CFG_CONFLICT_RET("Track create object exceeding maximum object limit");
    }

    sal_strncpy(track_type, argv[1], sizeof(track_type));
    if (!sal_strcmp(track_type, "interface"))
    {
        sal_strncpy(track_intf, argv[2], IFNAME_SIZE);
        rc = track_create_object_intf(p_db_track_glb, p_db_object, track_intf, object_id, para);
    }
    else if (!sal_strcmp(track_type, "rtr"))
    {
        PM_CFG_GET_UINT(rtr_entry_id, argv[2], para->p_rs);
        sal_strncpy(rtr_type, argv[3], sizeof(rtr_type));
        rc = track_create_object_rtr(p_db_track_glb, p_db_object, rtr_type, rtr_entry_id, object_id, para);
    }
    else
    {
        CFG_CONFLICT_RET("Unsupported track type %s", track_type);
    }

    return rc;
}

static int32
_track_cmd_delete_object(uint32 object_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_track_object_key_t    object_key;
    tbl_track_object_t       *p_db_object = NULL;
    tbl_track_global_t       *p_db_track_glb = NULL;

    sal_memset(&object_key, 0, sizeof(object_key));

    object_key.trackobj_id = object_id;
    p_db_object = tbl_track_object_get_track_object(&object_key);
    if (!p_db_object)
    {
        CFG_CONFLICT_RET("Track object not exist");
    }

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        CFG_CONFLICT_RET("Tbl_track_global not found");
    }

    rc = track_delete_object(p_db_track_glb, p_db_object);

    return rc;

}

static int32
_track_cmd_set_delay_up(tbl_track_object_t *p_db_object, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (p_db_object->delay_up== value)
    {
        return rc;
    }

    if (TRACK_DELAY_MIN > value || TRACK_DELAY_MAX < value)
    {
        CFG_CONFLICT_RET("Track parameter illegal");
    }

    p_db_object->delay_up = value;
    p_db_object->running_delay_up = 0;
    PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_DELAY_UP));

    return rc;
}

static int32
_track_cmd_set_delay_down(tbl_track_object_t *p_db_object, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (p_db_object->delay_down == value)
    {
        return rc;
    }

    if (TRACK_DELAY_MIN > value || TRACK_DELAY_MAX < value)
    {
        CFG_CONFLICT_RET("Track parameter illegal");
    }

    p_db_object->delay_down = value;
    p_db_object->running_delay_down = 0;
    PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_DELAY_DOWN));

    return rc;
}

static int32
_track_cmd_get_object(uint32  object_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                    *fp = NULL;
    tbl_track_object_key_t   key;
    tbl_track_object_t      *p_db_object = NULL;
    tbl_iter_args_t          args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == object_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_track_object_dump_one, &args);
    }
    else
    {
        key.trackobj_id = object_id;
        p_db_object = tbl_track_object_get_track_object(&key);
        if (!p_db_object)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("track object %u does not exist", object_id);
        }

        tbl_track_object_dump_one(p_db_object, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
track_cmd_process_object(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    tbl_track_object_t     object;
    tbl_track_object_t    *p_db_object  = NULL;
    int32                  value    = 0;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    uint32                 object_id = 0;
    field_parse_t          field;

    CFG_DBG_FUNC();

    sal_memset(&object, 0, sizeof(tbl_track_object_t));

    p_node = cdb_get_tbl(TBL_TRACK_OBJECT);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(object_id, argv[0], para->p_rs);

        rc = _track_cmd_create_object(argv, argc, object_id, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(object_id, argv[0], para->p_rs);

        rc = _track_cmd_delete_object(object_id, para);
        break;

    case CDB_OPER_SET:
        PM_CFG_GET_UINT(object_id, argv[0], para->p_rs);

        object.key.trackobj_id = object_id;
        p_db_object = tbl_track_object_get_track_object(&object.key);
        if (!p_db_object)
        {
            CFG_CONFLICT_RET("Track object not exist");
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
        case TBL_TRACK_OBJECT_FLD_TYPE:
            /* do not set type here */
            break;

        case TBL_TRACK_OBJECT_FLD_DELAY_UP:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            rc = _track_cmd_set_delay_up(p_db_object, value, field_id, para);
            break;

        case TBL_TRACK_OBJECT_FLD_DELAY_DOWN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            rc = _track_cmd_set_delay_down(p_db_object, value, field_id, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(object_id, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _track_cmd_get_object(object_id, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static void
track_show_object(cfg_cmd_para_t *para, FILE *fp, tbl_track_object_t *p_db_object)
{
    char *str = NULL;
    uint32 vrid = 0;
    uint32 tracked_vrrp_bmp_zero = TRUE;
    char   ifname_ext[IFNAME_EXT_SIZE];

    sal_fprintf(fp, "Track %d\n", p_db_object->key.trackobj_id);

    switch (p_db_object->type)
    {
        case TRACK_INTERFACE_LINKSTATE:
            str = "Interface Linkstate";
            break;
        case TRACK_RTR_REACHABILITY:
            str = "IP SLA Reachability";
            break;
        case TRACK_RTR_STATE:
            str = "IP SLA State";
            break;
        default:
            break;
    }
    if (str)
        sal_fprintf(fp, "    %-20s: %s\n", "Type", str);

    if (sal_strlen(p_db_object->track_if))
        sal_fprintf(fp, "    %-20s: %s\n", "Interface", IFNAME_ETH2FULL(p_db_object->track_if, ifname_ext));

    if (p_db_object->rtr_entryid != 0)
        sal_fprintf(fp, "    %-20s: %d\n", "IP-Sla entry number", p_db_object->rtr_entryid);

    if (p_db_object->state == TRACK_STATE_UP)
        sal_fprintf(fp, "    %-20s: %s\n", "State", "up");
    else
        sal_fprintf(fp, "    %-20s: %s\n", "State", "down");

    if (p_db_object->delay_up != 0)
        sal_fprintf(fp, "    %-20s: %d seconds\n", "Delay up",p_db_object->delay_up);

    if (p_db_object->delay_down != 0)
        sal_fprintf(fp, "    %-20s: %d seconds\n", "Delay down",p_db_object->delay_down);

    if (p_db_object->running_delay_up != 0)
        sal_fprintf(fp, "    %-20s: %d seconds\n", "Running Delay up",p_db_object->running_delay_up);

    if (p_db_object->running_delay_down != 0)
        sal_fprintf(fp, "    %-20s: %d seconds\n", "Running Delay down",p_db_object->running_delay_down);

    GLB_BMP_CHECK_ALL_ZERO(p_db_object->tracked_vrid_bmp, tracked_vrrp_bmp_zero);
    if(!tracked_vrrp_bmp_zero)
    {
        sal_fprintf(fp, "    %-20s\n","Tracked by");
    }

    GLB_BMP_ITER_BEGIN(p_db_object->tracked_vrid_bmp, vrid)
    {
        if (GLB_BMP_ISSET(p_db_object->tracked_vrid_bmp, vrid))
        {
            sal_fprintf(fp, "        %-16s: %d\n","Router vrrp", vrid);
        }

    }
    GLB_BMP_ITER_END(p_db_object->tracked_vrid_bmp, vrid);

    return;
}

int32
track_cmd_show_track(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                      rc        = PM_E_NONE;
    FILE                      *fp        = NULL;
    tbl_track_object_master_t *p_master  = NULL;
    uint32                     object_id  = 0;
    tbl_track_object_t        *p_db_object     = NULL;
    tbl_track_global_t        *p_db_track_glb = NULL;

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        CFG_CONFLICT_RET("Tbl_track_global not found");
    }
    uint16 track_object_num = p_db_track_glb->track_object_num;

    p_master = tbl_track_object_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_track_object not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        sal_fprintf(fp, "track object count : %d\n", p_db_track_glb->track_object_num);

        for (object_id = 1; object_id <= GLB_TRACK_OBJECT_MAX && track_object_num > 0; object_id++)
        {
            p_db_object = p_master->trackobj_array[object_id];
            if (!p_db_object)
            {
                continue;
            }

            track_show_object(para, fp, p_db_object);
            track_object_num--;
        }
    }
    else
    {
        PM_CFG_GET_UINT(object_id, argv[0], para->p_rs);
        p_db_object = p_master->trackobj_array[object_id];

        if (!p_db_object)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("Track object not found");
        }

        track_show_object(para, fp, p_db_object);
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}


