
#ifndef __TBL_SNMP_COMMUNITY_DEFINE_H__
#define __TBL_SNMP_COMMUNITY_DEFINE_H__

/* TBL_SNMP_COMMUNITY field defines */
typedef enum
{
    TBL_SNMP_COMMUNITY_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_COMMUNITY_FLD_TYPE                 = 1 ,  /* RW */
    TBL_SNMP_COMMUNITY_FLD_VIEW                 = 2 ,  /* RW */
    TBL_SNMP_COMMUNITY_FLD_SECIDX               = 3 ,  /* READ */
    TBL_SNMP_COMMUNITY_FLD_MAX                  = 4 
} tbl_snmp_community_field_id_t;

/* TBL_SNMP_COMMUNITY defines */
typedef struct
{
    char                 name[SNMP_COMMUNITY_SIZE];
} tbl_snmp_community_key_t;

typedef struct
{
    tbl_snmp_community_key_t key;
    uint8                type;                /* read-only or read-write */
    char                 view[SNMP_NAME_SIZE]; /* view name */
    uint8                secidx;              /* sec name index: commXX */
} tbl_snmp_community_t;

typedef struct
{
    ctclib_slist_t       *snmp_community_list;
} tbl_snmp_community_master_t;

#endif /* !__TBL_SNMP_COMMUNITY_DEFINE_H__ */

