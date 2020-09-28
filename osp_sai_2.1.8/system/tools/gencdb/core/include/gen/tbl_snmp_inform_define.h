
#ifndef __TBL_SNMP_INFORM_DEFINE_H__
#define __TBL_SNMP_INFORM_DEFINE_H__

/* TBL_SNMP_INFORM field defines */
typedef enum
{
    TBL_SNMP_INFORM_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_INFORM_FLD_IS_INBAND            = 1 ,  /* RW */
    TBL_SNMP_INFORM_FLD_TIMEOUT              = 2 ,  /* RW */
    TBL_SNMP_INFORM_FLD_RETRIES              = 3 ,  /* RW */
    TBL_SNMP_INFORM_FLD_MAX                  = 4 
} tbl_snmp_inform_field_id_t;

/* TBL_SNMP_INFORM defines */
typedef struct
{
    snmp_inform_key_t    inform;
} tbl_snmp_inform_key_t;

typedef struct
{
    tbl_snmp_inform_key_t key;
    uint32               is_inband;
    uint32               timeout;             /* timeout, default is 1500, 0-65535 */
    uint8                retries;             /* retries, default is 3, 0-255 */
} tbl_snmp_inform_t;

typedef struct
{
    ctclib_slist_t       *snmp_inform_list;   /* no sort */
} tbl_snmp_inform_master_t;

#endif /* !__TBL_SNMP_INFORM_DEFINE_H__ */

