
#ifndef __TBL_RMON_EVENT_DEFINE_H__
#define __TBL_RMON_EVENT_DEFINE_H__

/* TBL_RMON_EVENT field defines */
typedef enum
{
    TBL_RMON_EVENT_FLD_KEY                  = 0 ,  /* READ */
    TBL_RMON_EVENT_FLD_CONF                 = 1 ,  /* RW */
    TBL_RMON_EVENT_FLD_TYPE                 = 2 ,  /* RW */
    TBL_RMON_EVENT_FLD_STATUS               = 3 ,  /* RW */
    TBL_RMON_EVENT_FLD_LAST_SENT            = 4 ,  /* RW */
    TBL_RMON_EVENT_FLD_DESC                 = 5 ,  /* RW */
    TBL_RMON_EVENT_FLD_COMMUNITY            = 6 ,  /* RW */
    TBL_RMON_EVENT_FLD_OWNER                = 7 ,  /* RW */
    TBL_RMON_EVENT_FLD_T_UNDER_CREATION_TIMER = 8 ,  /* READ */
    TBL_RMON_EVENT_FLD_MAX                  = 9 
} tbl_rmon_event_field_id_t;

/* TBL_RMON_EVENT defines */
typedef struct
{
    uint32               index;
} tbl_rmon_event_key_t;

typedef struct
{
    tbl_rmon_event_key_t key;
    uint32               conf;
    uint32               type;                /* 1- none, 2- log, 3-snmpTrap, 4-logandTrap */
    uint32               status;              /* 1-Valid, 2-createRequest, *3-UnderCreation, 4- invalid, *5-nonExistent */
    uint64               last_sent;
    char                 desc[RMON_DESCR_LENGTH+1];
    char                 community[SNMP_COMMUNITY_SIZE+1];
    char                 owner[RMON_OWNER_NAME_SIZE+1];
    ctc_task_t           *t_under_creation_timer;
} tbl_rmon_event_t;

typedef struct
{
    ctclib_hash_t        *rmon_event_hash;
    ctclib_slist_t       *rmon_event_list;
} tbl_rmon_event_master_t;

#endif /* !__TBL_RMON_EVENT_DEFINE_H__ */

