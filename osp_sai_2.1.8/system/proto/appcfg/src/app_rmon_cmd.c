
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "lib_fs.h"
#include <errno.h>
#include "glb_const.h"
#include "cdb_ds.h"
#include "glb_ip_define.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "gen/tbl_rmon_stats_history_define.h"
#include "gen/tbl_rmon_stats_history.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "appcfg.h"
#include "app_rmon.h"


extern int32
cdb_check_snmp_name(const char *name, int32 max_len, char *err_desc);

static int32
_rmon_cmd_create_event(uint32 index, char **argv, cfg_cmd_para_t *para)
{
    int32                 rc         = PM_E_NONE;
    tbl_rmon_event_t      event;
    tbl_rmon_event_t     *p_db_event = NULL;
    uint32   event_type   = 0;

    if (index < RMON_INDEX_MIN || index > RMON_INDEX_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon event index %d", index);
    }

    sal_memset(&event, 0, sizeof(event));
    event.key.index = index;
    p_db_event = tbl_rmon_event_get_rmon_event(&event.key);
    if (p_db_event)
    {
        CFG_CONFLICT_RET("Event %d is already created", index);
    }

    if (rmon_event_get_count() >= RMON_EVENT_NUM_MAX)
    {
        CFG_CONFLICT_RET("Can't set rmon Event Entry: only %d event supported in system", RMON_EVENT_NUM_MAX);
    }

    if (!sal_strcmp(argv[2], "log"))
    {
        event_type += RMON_EVENT_LOG;
    }
    if (sal_strlen(argv[4]))
    {
        event_type += RMON_EVENT_TRAP;
        rc = cdb_check_snmp_name(argv[4], RMON_DESCR_LENGTH, para->p_rs->desc);
        if (rc < 0) 
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strcpy(event.community, argv[4]);
    }
    if (0 == event_type)
    {
        event_type = RMON_EVENT_NONE;
    }
    else if (5 == event_type)
    {
        event_type--;
    }
    event.type = event_type;

    if (sal_strlen(argv[6]))
    {
        rc = cdb_check_snmp_name(argv[6], RMON_DESCR_LENGTH, para->p_rs->desc);
        if (rc < 0) 
        {
            CFG_INVALID_PARAM_RET("Description must be started with alphanumeric and can only include [0-9a-zA-Z.-_], max length is 127");
        }
        sal_strcpy(event.desc, argv[6]);
    }
    else
    {
        sal_strcpy(event.desc, RMON_DEFAULT_NAME);
    }

    if (sal_strlen(argv[8]))
    {
        rc = cdb_check_snmp_name(argv[8], RMON_OWNER_NAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
            {
                sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
            }
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strcpy(event.owner, argv[8]);
    }
    else
    {
        sal_strcpy(event.owner, RMON_DEFAULT_NAME);
    }

    PM_E_RETURN(tbl_rmon_event_add_rmon_event(&event));

    p_db_event = tbl_rmon_event_get_rmon_event(&event.key);
    if (!p_db_event)
    {
        CFG_CONFLICT_RET("Event %d is create fail", index);
    }
    
    if (rmon_event_set_active(p_db_event) < 0)
    {
        PM_E_RETURN(tbl_rmon_event_del_rmon_event(&event.key));
        CFG_CONFLICT_RET("Event %d set active fail", index);
    }

    return rc;
}

static int32
_rmon_cmd_delete_event(uint32 index, cfg_cmd_para_t *para)
{
    int32                 rc = PM_E_NONE;
    tbl_rmon_event_key_t  event_key;
    tbl_rmon_event_t     *p_db_event = NULL;
    tbl_rmon_alarm_master_t  *p_master = tbl_rmon_alarm_get_master();
    ctclib_slistnode_t       *listnode   = NULL;
    tbl_rmon_alarm_t         *p_db_alarm = NULL;
    int32 event_used = FALSE;

    if (index < RMON_INDEX_MIN || index > RMON_INDEX_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon event index %d", index);
    }

    sal_memset(&event_key, 0, sizeof(event_key));
    event_key.index = index;
    p_db_event = tbl_rmon_event_get_rmon_event(&event_key);
    if (!p_db_event)
    {
        CFG_CONFLICT_RET("Event %d is not exist", index);
    }

    CTC_TASK_STOP_TIMER(p_db_event->t_under_creation_timer);

    CTCLIB_SLIST_LOOP(p_master->rmon_alarm_list, p_db_alarm, listnode)
    {
        if (!p_db_alarm)
        {
            continue;
        }

        if (p_db_alarm->rising_event_index == index || p_db_alarm->falling_event_index == index)
        {
            event_used = TRUE;
            break;
        }
    }

    PM_E_RETURN(tbl_rmon_event_del_rmon_event(&p_db_event->key));

    if (event_used)
    {
        CFG_PROMPT_RET("Notice: The rmon event %d was used by rmon alarm and delete it will result in the alarm's status switch to underCreation", index);
    }

    return rc;
}

static int32
_rmon_cmd_set_event_type(tbl_rmon_event_t *p_db_event, uint32 value, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (RMON_EVENT_NONE > value || RMON_EVENT_LOG_TRAP < value)
    {
        CFG_CONFLICT_RET("Invalid rmon event type");
    }

    if (p_db_event->type == value)
    {
        return rc;
    }

    p_db_event->type = value;
    PM_E_RETURN(tbl_rmon_event_set_rmon_event_field(p_db_event, TBL_RMON_EVENT_FLD_TYPE));

    return rc;
}

static int32
_rmon_cmd_set_event_status(tbl_rmon_event_t *p_db_event, uint32 value, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (VALID_STATUS > value || INVALID_STATUS < value)
    {
        CFG_CONFLICT_RET("Invalid rmon event status");
    }

    if (p_db_event->status == value)
    {
        return rc;
    }

    PM_E_RETURN(rmon_snmp_set_event_status(p_db_event, value));

    return rc;
}

static int32
_rmon_cmd_set_event_desc(tbl_rmon_event_t *p_db_event, char *desc, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    rc = cdb_check_snmp_name(desc, RMON_DESCR_LENGTH, para->p_rs->desc);
    if (rc < 0) 
    {
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }

    if (!sal_strcmp(p_db_event->desc, desc))
    {
        return rc;
    }

    sal_strcpy(p_db_event->desc, desc);
    PM_E_RETURN(tbl_rmon_event_set_rmon_event_field(p_db_event, TBL_RMON_EVENT_FLD_DESC));

    return rc;
}

static int32
_rmon_cmd_set_event_comm(tbl_rmon_event_t *p_db_event, char *comm, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    rc = cdb_check_snmp_name(comm, SNMP_COMMUNITY_SIZE - 1, para->p_rs->desc);
    if (rc < 0) 
    {
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }

    if (!sal_strcmp(p_db_event->community, comm))
    {
        return rc;
    }

    sal_strcpy(p_db_event->community, comm);
    PM_E_RETURN(tbl_rmon_event_set_rmon_event_field(p_db_event, TBL_RMON_EVENT_FLD_COMMUNITY));

    return rc;
}

static int32
 _rmon_cmd_set_event_owner(tbl_rmon_event_t *p_db_event, char *owner, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    rc = cdb_check_snmp_name(owner, RMON_OWNER_NAME_SIZE, para->p_rs->desc);
    if (rc < 0) 
    {
        if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
        {
            sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
        }
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }

    if (!sal_strcmp(p_db_event->owner, owner))
    {
        return rc;
    }

    sal_strcpy(p_db_event->owner, owner);
    PM_E_RETURN(tbl_rmon_event_set_rmon_event_field(p_db_event, TBL_RMON_EVENT_FLD_OWNER));

    return rc;
}

static int32
_rmon_cmd_get_event(uint32 index, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                 *fp = NULL;
    tbl_rmon_event_key_t  event;
    tbl_rmon_event_t     *p_db_event = NULL;
    tbl_iter_args_t       args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!index)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_rmon_event_dump_one, &args);
    }
    else
    {
        sal_memset(&event, 0, sizeof(event));
        event.index = index;
        p_db_event = tbl_rmon_event_get_rmon_event(&event);
        if (!p_db_event)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("rmon event %d does not exist", index);
        }

        tbl_rmon_event_dump_one(p_db_event, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rmon_cmd_process_event(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    int32                  value    = 0;
    int32                  i        = 0;
    uint32                 index    = 0;
    field_parse_t          field;
    tbl_rmon_event_key_t   event_key;
    tbl_rmon_event_t      *p_db_event = NULL;

    CFG_DBG_FUNC();

    sal_memset(&event_key, 0, sizeof(event_key));

    p_node = cdb_get_tbl(TBL_RMON_EVENT);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 10, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_create_event(index, argv, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_delete_event(index, para);
        break;

    case CDB_OPER_SET:
        /* support set filed for snmp set */
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        event_key.index = index;
        p_db_event = tbl_rmon_event_get_rmon_event(&event_key);
        if (!p_db_event)
        {
            CFG_CONFLICT_RET("Event %d is not exist", index);
        }

        i++;
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
        case TBL_RMON_EVENT_FLD_TYPE:
            /* cdbctl update/cdb/app/rmon_event/${index}/type/${type} called by snmp agent */
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            
            rc = _rmon_cmd_set_event_type(p_db_event, value, para);
            break;
        case TBL_RMON_EVENT_FLD_STATUS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            
            rc = _rmon_cmd_set_event_status(p_db_event, value, para);
            break;
        case TBL_RMON_EVENT_FLD_DESC:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            
            rc = _rmon_cmd_set_event_desc(p_db_event, argv[2], para);
            break;
        case TBL_RMON_EVENT_FLD_COMMUNITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            
            rc = _rmon_cmd_set_event_comm(p_db_event, argv[2], para);
            break;
        case TBL_RMON_EVENT_FLD_OWNER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            
            rc = _rmon_cmd_set_event_owner(p_db_event, argv[2], para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(index, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _rmon_cmd_get_event(index, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_rmon_cmd_create_alarm(uint32 index, char **argv, cfg_cmd_para_t *para)
{
    int32                 rc         = PM_E_NONE;
    tbl_rmon_alarm_t      alarm;
    tbl_rmon_alarm_t     *p_db_alarm = NULL;

    uint32  interval = 0;
    uint32  alarmSampleType   = 0;
    int64   risingThresValue  = 0;
    int64   fallingThresValue = 0;
    uint32  risingEvent = 0;
    uint32  fallingEvent = 0;

    if (index < RMON_INDEX_MIN || index > RMON_INDEX_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon alarm index %d", index);
    }
    sal_memset(&alarm, 0, sizeof(alarm));
    alarm.key.index = index;
    p_db_alarm = tbl_rmon_alarm_get_rmon_alarm(&alarm.key);
    if (p_db_alarm)
    {
        CFG_CONFLICT_RET("Alarm %d is already created, can't set rmon alarm", index);
    }

    if (rmon_alarm_get_count() >= RMON_ALARM_NUM_MAX)
    {
        CFG_CONFLICT_RET("Can't set rmon Alarm Entry: only %d alarm supported in system", RMON_ALARM_NUM_MAX);
    }

    PM_CFG_GET_UINT(interval, argv[4], para->p_rs);
    if (interval < RMON_INTERVAL_MIN || interval > RMON_INTERVAL_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon alarm interval %d", interval);
    }
    alarm.interval = interval;

    if (sal_strcmp (argv[6], "absolute") == 0)
        alarmSampleType = RMON_ALARM_ABS;
    else
        alarmSampleType = RMON_ALARM_DELTA;
    alarm.sample_type = alarmSampleType;

    PM_CFG_GET_INT64(risingThresValue, argv[8], para->p_rs);
    if (risingThresValue < RMON_MIN_THRESHOLD || risingThresValue > RMON_MAX_THRESHOLD)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon alarm rising threshold value %s", argv[8]);
    }
    if (sal_strlen(argv[9]))
    {
        PM_CFG_GET_UINT(risingEvent, argv[9], para->p_rs);
        if (risingEvent < RMON_INDEX_MIN || risingEvent > RMON_INDEX_MAX)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon rising event index %d", risingEvent);
        }
#if 0
        /* don't check event existence when add rmon alarm */
        sal_memset(&event_key, 0, sizeof(event_key));
        event_key.index = risingEvent;
        p_db_event = tbl_rmon_event_get_rmon_event(&event_key);
        if (!p_db_event)
        {
            CFG_CONFLICT_RET("Rmon rising event %d no exist yet", risingEvent);
        }
#endif
    }
    alarm.rising_threshold = risingThresValue;
    alarm.rising_event_index = risingEvent;

    PM_CFG_GET_INT64(fallingThresValue, argv[11], para->p_rs);
    if (fallingThresValue < RMON_MIN_THRESHOLD || fallingThresValue > RMON_MAX_THRESHOLD)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon alarm falling threshold value %s", argv[11]);
    }
    if (sal_strlen(argv[12]))
    {
        PM_CFG_GET_UINT(fallingEvent, argv[12], para->p_rs);
        if (fallingEvent < RMON_INDEX_MIN || fallingEvent > RMON_INDEX_MAX)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon falling event index %d", fallingEvent);
        }
#if 0
        /* don't check event existence when add rmon alarm */
        sal_memset(&event_key, 0, sizeof(event_key));
        event_key.index = fallingEvent;
        p_db_event = tbl_rmon_event_get_rmon_event(&event_key);
        if (!p_db_event)
        {
            CFG_CONFLICT_RET("Rmon falling event %d no exist yet", fallingEvent);
        }
#endif
    }
    alarm.falling_threshold = fallingThresValue;
    alarm.falling_event_index = fallingEvent;

    if (alarmSampleType == RMON_ALARM_DELTA && risingThresValue <= fallingThresValue)
    {
        CFG_INVALID_PARAM_RET("The falling-threshold is greater than or equal to the rising-threshold");
    }

    if (sal_strlen(argv[14]))
    {
        rc = cdb_check_snmp_name(argv[14], RMON_OWNER_NAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
            {
                sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
            }
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strcpy(alarm.owner, argv[14]);
    }
    else
    {
        sal_strcpy(alarm.owner, RMON_DEFAULT_NAME);
    }

    PM_E_RETURN(rmon_api_alarm_add(&alarm, argv[2], para, RISING_OR_FALLING_ALARM));

    p_db_alarm = tbl_rmon_alarm_get_rmon_alarm(&alarm.key);
    if (!p_db_alarm)
    {
        CFG_CONFLICT_RET("Alarm %d is create fail", index);
    }

    if (rmon_alarm_set_active(p_db_alarm) < 0)
    {
        PM_E_RETURN(tbl_rmon_alarm_del_rmon_alarm(&alarm.key));
        CFG_CONFLICT_RET("Alarm %d set active fail", index);
    }

    return rc;
}

static int32
_rmon_cmd_delete_alarm(uint32 index, cfg_cmd_para_t *para)
{
    int32                 rc = PM_E_NONE;
    tbl_rmon_alarm_key_t  alarm_key;
    tbl_rmon_alarm_t     *p_db_alarm = NULL;

    if (index < RMON_INDEX_MIN || index > RMON_INDEX_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon alarm index %d", index);
    }

    sal_memset(&alarm_key, 0, sizeof(alarm_key));
    alarm_key.index = index;
    p_db_alarm = tbl_rmon_alarm_get_rmon_alarm(&alarm_key);
    if (!p_db_alarm)
    {
        CFG_CONFLICT_RET("Alarm %d is not exist", index);
    }

    CTC_TASK_STOP_TIMER(p_db_alarm->t_under_creation_timer);
    CTC_TASK_STOP_TIMER(p_db_alarm->t_alarm_timer);

    PM_E_RETURN(tbl_rmon_alarm_del_rmon_alarm(&p_db_alarm->key));

    return rc;
}

static int32
_rmon_cmd_get_alarm(uint32 index, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                 *fp = NULL;
    tbl_rmon_alarm_key_t  alarm;
    tbl_rmon_alarm_t     *p_db_alarm = NULL;
    tbl_iter_args_t       args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!index)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_rmon_alarm_dump_one, &args);
    }
    else
    {
        sal_memset(&alarm, 0, sizeof(alarm));
        alarm.index = index;
        p_db_alarm = tbl_rmon_alarm_get_rmon_alarm(&alarm);
        if (!p_db_alarm)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("rmon alarm %d does not exist", index);
        }

        tbl_rmon_alarm_dump_one(p_db_alarm, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rmon_cmd_process_alarm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    tbl_rmon_alarm_key_t   alarm_key;
    tbl_rmon_alarm_t      *p_db_alarm = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    //int32                  value    = 0;
    int32                  i        = 0;
    uint32                 index    = 0;
    field_parse_t          field;

    CFG_DBG_FUNC();

    sal_memset(&alarm_key, 0, sizeof(alarm_key));

    p_node = cdb_get_tbl(TBL_RMON_ALARM);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 16, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_create_alarm(index, argv, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_delete_alarm(index, para);
        break;

    case CDB_OPER_SET:
        /* support set filed for snmp set */
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        alarm_key.index = index;
        p_db_alarm = tbl_rmon_alarm_get_rmon_alarm(&alarm_key);
        if (!p_db_alarm)
        {
            CFG_CONFLICT_RET("Event %d is not exist", index);
        }

        i++;
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
        /* case TBL_RMON_EVENT_FLD_STATUS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            
            rc = _rmon_cmd_set_event_status(p_db_event, value, para);
            break; */

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(index, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _rmon_cmd_get_alarm(index, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


static int32
_rmon_cmd_create_stats(uint32 index, char **argv, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_rmon_ether_stats_t  stats;
    tbl_rmon_ether_stats_t *p_db_stats = NULL;
    tbl_interface_key_t     intf;
    tbl_interface_t        *p_db_if = NULL;

    if (index < RMON_INDEX_MIN || index > RMON_INDEX_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon ether statistics index %d", index);
    }

    if (sal_strlen(argv[2]))
    {
        if (sal_strlen(argv[2]) > IFNAME_SIZE)
        {
            CFG_CONFLICT_RET("Invalid interface %s", argv[2]);
        }
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.name, argv[2], IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&intf);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Invalid interface %s", argv[2]);
        }

        if (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF)
        {
            CFG_CONFLICT_RET("Statistics entry can only be added on physical port");
        }
    }
    else
    {
        CFG_CONFLICT_RET("Invalid interface");
    }

    sal_memset(&stats, 0, sizeof(stats));
    stats.key.index = index;
    p_db_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&stats.key);
    if (p_db_stats)
    {
        CFG_CONFLICT_RET("Statistics %d is already created", index);
    }
        
    if (rmon_api_stats_check_if(p_db_if->ifindex) < 0)
    {
        CFG_CONFLICT_RET("Statistics have already enabled on this interface");
    }

    if (sal_strlen(argv[4]))
    {
        rc = cdb_check_snmp_name(argv[4], RMON_OWNER_NAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
            {
                sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
            }
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strcpy(stats.owner, argv[4]);
    }
    else
    {
        sal_strcpy(stats.owner, RMON_DEFAULT_NAME);
    }

    PM_E_RETURN(rmon_api_ether_stats_add(p_db_if, &stats));

    return rc;
}

static int32
_rmon_cmd_delete_stats(uint32 index, char *ifname, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_rmon_ether_stats_t    *p_db_stats = NULL;
    tbl_interface_key_t        intf;
    tbl_interface_t           *p_db_if = NULL;
    tbl_rmon_ether_stats_master_t *p_master  = NULL;
    ctclib_slistnode_t            *listnode   = NULL;
    ctclib_slistnode_t            *next   = NULL;

    if (sal_strlen(ifname))
    {
        if (sal_strlen(ifname) > IFNAME_SIZE)
        {
            CFG_CONFLICT_RET("Invalid interface %s", ifname);
        }
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.name, ifname, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&intf);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Invalid interface %s", ifname);
        }
    }
    else
    {
        CFG_CONFLICT_RET("Invalid interface");
    }

    p_master = tbl_rmon_ether_stats_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Rmon statistics not exist");
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_ether_stats_list, p_db_stats, listnode, next)
    {
        if (!p_db_stats)
        {
            continue;
        }

        if (p_db_stats->data_source_ifindex == p_db_if->ifindex)
        {
            CTC_TASK_STOP_TIMER(p_db_stats->t_under_creation_timer);
            PM_E_RETURN(tbl_rmon_ether_stats_del_rmon_ether_stats(&p_db_stats->key));
            return rc;
        }
    }

    CFG_CONFLICT_RET("Rmon statistics is not configured on this interface");
}

static int32
_rmon_cmd_set_stats_status(tbl_rmon_ether_stats_t *p_db_stats, uint32 value, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (VALID_STATUS > value || INVALID_STATUS < value)
    {
        CFG_CONFLICT_RET("Invalid rmon statistics status");
    }

    if (p_db_stats->status == value)
    {
        return rc;
    }

    PM_E_RETURN(rmon_snmp_set_ether_stats_status(p_db_stats, value));

    return rc;
}

static int32
 _rmon_cmd_set_stats_owner(tbl_rmon_ether_stats_t *p_db_stats, char *owner, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    rc = cdb_check_snmp_name(owner, RMON_OWNER_NAME_SIZE, para->p_rs->desc);
    if (rc < 0) 
    {
        if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
        {
            sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
        }
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }

    if (!sal_strcmp(p_db_stats->owner, owner))
    {
        return rc;
    }

    sal_strcpy(p_db_stats->owner, owner);
    PM_E_RETURN(tbl_rmon_ether_stats_set_rmon_ether_stats_field(p_db_stats, TBL_RMON_ETHER_STATS_FLD_OWNER));

    return rc;
}

static int32
_rmon_cmd_get_stats(uint32 index, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                 *fp = NULL;
    tbl_iter_args_t       args;
    tbl_rmon_ether_stats_key_t  stats;
    tbl_rmon_ether_stats_t     *p_db_stats = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!index)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_rmon_ether_stats_dump_one, &args);
    }
    else
    {
        sal_memset(&stats, 0, sizeof(stats));
        stats.index = index;
        p_db_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&stats);
        if (!p_db_stats)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("rmon statistics %d does not exist", index);
        }

        tbl_rmon_ether_stats_dump_one(p_db_stats, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rmon_cmd_process_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    int32                  value    = 0;
    int32                  i        = 0;
    uint32                 index    = 0;
    field_parse_t          field;
    tbl_rmon_ether_stats_key_t stats_key;
    tbl_rmon_ether_stats_t    *p_db_stats = NULL;

    CFG_DBG_FUNC();

    sal_memset(&stats_key, 0, sizeof(stats_key));

    p_node = cdb_get_tbl(TBL_RMON_ETHER_STATS);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_create_stats(index, argv, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_delete_stats(index, argv[2], para);
        break;

    case CDB_OPER_SET:
        /* support set filed for snmp set */
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        stats_key.index = index;
        p_db_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&stats_key);
        if (!p_db_stats)
        {
            CFG_CONFLICT_RET("Statistics %d is not exist", index);
        }

        i++;
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
        case TBL_RMON_ETHER_STATS_FLD_STATUS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            
            rc = _rmon_cmd_set_stats_status(p_db_stats, value, para);
            break;
        case TBL_RMON_ETHER_STATS_FLD_OWNER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            
            rc = _rmon_cmd_set_stats_owner(p_db_stats, argv[2], para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(index, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _rmon_cmd_get_stats(index, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_rmon_cmd_create_history(uint32 index, char **argv, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_rmon_history_t   history;
    tbl_rmon_history_t  *p_db_history = NULL;
    tbl_interface_key_t  intf;
    tbl_interface_t     *p_db_if = NULL;
    uint32 bucket_size = 0;
    uint32 interval    = 0;

    if (index < RMON_INDEX_MIN || index > RMON_INDEX_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon history index %d", index);
    }

    if (sal_strlen(argv[2]))
    {
        if (sal_strlen(argv[2]) > IFNAME_SIZE)
        {
            CFG_CONFLICT_RET("Invalid interface %s", argv[2]);
        }
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.name, argv[2], IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&intf);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Invalid interface %s", argv[2]);
        }

        if (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF)
        {
            CFG_CONFLICT_RET("History entry can only be added on physical port");
        }
    }
    else
    {
        CFG_CONFLICT_RET("Invalid interface");
    }

    sal_memset(&history, 0, sizeof(history));
    history.key.index = index;
    p_db_history = tbl_rmon_history_get_rmon_history(&history.key);
    if (p_db_history)
    {
        CFG_CONFLICT_RET("History %d is already created", index);
    }
        
    if (rmon_api_history_check_if(p_db_if->ifindex) < 0)
    {
        CFG_CONFLICT_RET("History have already enabled on this interface");
    }

    if (sal_strlen(argv[4]))
    {
        PM_CFG_GET_UINT_RANGE(bucket_size, 1, RMON_BUCKET_NUM_MAX, argv[4], para->p_rs);
    }
    else
    {
        bucket_size = RMON_HISTORY_CTRL_BR_DEFAULT;
    }
    /* as of now requested and granted will be same */
    history.buckets_requested = bucket_size;
    history.buckets_granted = bucket_size;

    if (sal_strlen(argv[6]))
    {
        PM_CFG_GET_UINT_RANGE(interval, 1, RMON_INTERVAL_MAX, argv[6], para->p_rs);
    }
    else
    {
        interval = RMON_HISTORY_CTRL_SAMPLE_INTERVAL_DEFAULT;
    }
    history.interval = interval;
 
    if (sal_strlen(argv[8]))
    {
        rc = cdb_check_snmp_name(argv[8], RMON_OWNER_NAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
            {
                sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
            }
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strcpy(history.owner, argv[8]);
    }
    else
    {
        sal_strcpy(history.owner, RMON_DEFAULT_NAME);
    }

    PM_E_RETURN(rmon_api_history_add(p_db_if, &history));

    return rc;
}

static int32
_rmon_cmd_delete_history(uint32 index, char *ifname, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    tbl_rmon_history_t    *p_db_history = NULL;
    tbl_interface_key_t    intf;
    tbl_interface_t       *p_db_if = NULL;
    tbl_rmon_history_master_t *p_master  = NULL;
    ctclib_slistnode_t        *listnode   = NULL;
    ctclib_slistnode_t        *next   = NULL;

    p_master = tbl_rmon_history_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Rmon statistics not exist");
    }


    if (sal_strlen(ifname))
    {
        if (sal_strlen(ifname) > IFNAME_SIZE)
        {
            CFG_CONFLICT_RET("Invalid interface %s", ifname);
        }
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.name, ifname, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&intf);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Invalid interface %s", ifname);
        }
    }
    else
    {
        CFG_CONFLICT_RET("Invalid interface");
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_history_list, p_db_history, listnode, next)
    {
        if (!p_db_history)
        {
            continue;
        }

        if (p_db_history->data_source == p_db_if->ifindex)
        {
            rmon_history_set_inactive(p_db_history);
            PM_E_RETURN(tbl_rmon_history_del_rmon_history(&p_db_history->key));
            return rc;
        }
    }

    CFG_CONFLICT_RET("Rmon history is not configured on this interface");
}

static int32
_rmon_cmd_set_history_status(tbl_rmon_history_t *p_db_history, uint32 value, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (VALID_STATUS > value || INVALID_STATUS < value)
    {
        CFG_CONFLICT_RET("Invalid rmon history status");
    }

    if (p_db_history->status == value)
    {
        return rc;
    }

    //PM_E_RETURN(rmon_snmp_set_history_status(p_db_history, value));

    return rc;
}

static int32
 _rmon_cmd_set_history_owner(tbl_rmon_history_t *p_db_history, char *owner, cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    rc = cdb_check_snmp_name(owner, RMON_OWNER_NAME_SIZE, para->p_rs->desc);
    if (rc < 0) 
    {
        if (!sal_strncmp(para->p_rs->desc, "The name max len", 16))
        {
            sal_sprintf(para->p_rs->desc, "The owner configuration max length is %d.", RMON_OWNER_NAME_SIZE);
        }
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }

    if (!sal_strcmp(p_db_history->owner, owner))
    {
        return rc;
    }

    sal_strcpy(p_db_history->owner, owner);
    PM_E_RETURN(tbl_rmon_history_set_rmon_history_field(p_db_history, TBL_RMON_HISTORY_FLD_OWNER));

    return rc;
}

static int32
_rmon_cmd_get_history(uint32 index, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                 *fp = NULL;
    tbl_iter_args_t       args;
    tbl_rmon_history_key_t  history;
    tbl_rmon_history_t     *p_db_history = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!index)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_rmon_history_dump_one, &args);
    }
    else
    {
        sal_memset(&history, 0, sizeof(history));
        history.index = index;
        p_db_history = tbl_rmon_history_get_rmon_history(&history);
        if (!p_db_history)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("rmon history %d does not exist", index);
        }

        tbl_rmon_history_dump_one(p_db_history, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rmon_cmd_process_history(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    int32                  value    = 0;
    int32                  i        = 0;
    uint32                 index    = 0;
    field_parse_t          field;
    tbl_rmon_history_key_t history_key;
    tbl_rmon_history_t    *p_db_history = NULL;

    CFG_DBG_FUNC();

    sal_memset(&history_key, 0, sizeof(history_key));

    p_node = cdb_get_tbl(TBL_RMON_HISTORY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 10, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_create_history(index, argv, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        rc = _rmon_cmd_delete_history(index, argv[2], para);
        break;

    case CDB_OPER_SET:
        /* support set filed for snmp set */
        PM_CFG_GET_UINT(index, argv[0], para->p_rs);

        history_key.index = index;
        p_db_history = tbl_rmon_history_get_rmon_history(&history_key);
        if (!p_db_history)
        {
            CFG_CONFLICT_RET("History %d is not exist", index);
        }

        i++;
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
        case TBL_RMON_HISTORY_FLD_STATUS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            
            rc = _rmon_cmd_set_history_status(p_db_history, value, para);
            break;
        case TBL_RMON_HISTORY_FLD_OWNER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            
            rc = _rmon_cmd_set_history_owner(p_db_history, argv[2], para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(index, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _rmon_cmd_get_history(index, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
rmon_cmd_show_event(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc        = PM_E_NONE;
    FILE                     *fp        = NULL;
    tbl_rmon_event_master_t  *p_master  = NULL;
    uint32                    event_id  = 0;
    ctclib_slistnode_t       *listnode   = NULL;
    tbl_rmon_event_key_t      event_key;
    tbl_rmon_event_t         *p_db_event = NULL;

    p_master = tbl_rmon_event_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Rmon event not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_master->rmon_event_list, p_db_event, listnode)
        {
            if (!p_db_event)
            {
                continue;
            }

            rmon_show_event(para, fp, p_db_event);
        }
    }
    else
    {
        PM_CFG_GET_UINT(event_id, argv[0], para->p_rs);
        sal_memset(&event_key, 0, sizeof(event_key));
        event_key.index = event_id;
        p_db_event = tbl_rmon_event_get_rmon_event(&event_key);

        if (!p_db_event)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("Rmon event %d is invalid", event_id);
        }

        rmon_show_event(para, fp, p_db_event);
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
rmon_cmd_show_alarm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc        = PM_E_NONE;
    FILE                     *fp        = NULL;
    tbl_rmon_alarm_master_t  *p_master  = NULL;
    uint32                    alarm_id  = 0;
    ctclib_slistnode_t       *listnode   = NULL;
    tbl_rmon_alarm_key_t      alarm_key;
    tbl_rmon_alarm_t         *p_db_alarm = NULL;

    p_master = tbl_rmon_alarm_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Rmon alarm not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_master->rmon_alarm_list, p_db_alarm, listnode)
        {
            if (!p_db_alarm)
            {
                continue;
            }

            rmon_show_alarm(para, fp, p_db_alarm);
        }
    }
    else
    {
        PM_CFG_GET_UINT(alarm_id, argv[0], para->p_rs);
        sal_memset(&alarm_key, 0, sizeof(alarm_key));
        alarm_key.index = alarm_id;
        p_db_alarm = tbl_rmon_alarm_get_rmon_alarm(&alarm_key);

        if (!p_db_alarm)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("Rmon alarm %d is invalid", alarm_id);
        }

        rmon_show_alarm(para, fp, p_db_alarm);
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
rmon_cmd_show_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32  rc  = PM_E_NONE;
    FILE  *fp  = NULL;
    tbl_rmon_ether_stats_master_t *p_master  = NULL;
    uint32                         stats_id  = 0;
    ctclib_slistnode_t            *listnode   = NULL;
    tbl_rmon_ether_stats_key_t     stats_key;
    tbl_rmon_ether_stats_t        *p_db_stats = NULL;

    p_master = tbl_rmon_ether_stats_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Rmon statistics not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_master->rmon_ether_stats_list, p_db_stats, listnode)
        {
            if (!p_db_stats)
            {
                continue;
            }

            rmon_show_stats(para, fp, p_db_stats);
        }
    }
    else
    {
        PM_CFG_GET_UINT(stats_id, argv[0], para->p_rs);
        sal_memset(&stats_key, 0, sizeof(stats_key));
        stats_key.index = stats_id;
        p_db_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&stats_key);

        if (!p_db_stats)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("Rmon stats %d is invalid", stats_id);
        }

        rmon_show_stats(para, fp, p_db_stats);
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
rmon_cmd_show_history(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32  rc  = PM_E_NONE;
    FILE  *fp  = NULL;
    tbl_rmon_history_master_t *p_master  = NULL;
    uint32                     his_id  = 0;
    ctclib_slistnode_t        *listnode   = NULL;
    tbl_rmon_history_key_t     his_key;
    tbl_rmon_history_t        *p_db_his = NULL;

    p_master = tbl_rmon_history_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Rmon statistics not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_master->rmon_history_list, p_db_his, listnode)
        {
            if (!p_db_his)
            {
                continue;
            }

            rmon_show_history(para, fp, p_db_his);
        }
    }
    else
    {
        PM_CFG_GET_UINT(his_id, argv[0], para->p_rs);
        sal_memset(&his_key, 0, sizeof(his_key));
        his_key.index = his_id;
        p_db_his = tbl_rmon_history_get_rmon_history(&his_key);

        if (!p_db_his)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("Rmon history %d is invalid", his_id);
        }

        rmon_show_history(para, fp, p_db_his);
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}


