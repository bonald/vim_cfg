
#ifndef __TBL_SNMP_TRAP_DEFINE_H__
#define __TBL_SNMP_TRAP_DEFINE_H__

/* TBL_SNMP_TRAP field defines */
typedef enum
{
    TBL_SNMP_TRAP_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_TRAP_FLD_IS_INBAND            = 1 ,  /* RW */
    TBL_SNMP_TRAP_FLD_MAX                  = 2 
} tbl_snmp_trap_field_id_t;

/* TBL_SNMP_TRAP defines */
typedef struct
{
    snmp_trap_key_t      trap;
} tbl_snmp_trap_key_t;

typedef struct
{
    tbl_snmp_trap_key_t  key;
    uint32               is_inband;
} tbl_snmp_trap_t;

typedef struct
{
    ctclib_slist_t       *snmp_trap_list;     /* no sort */
} tbl_snmp_trap_master_t;

#endif /* !__TBL_SNMP_TRAP_DEFINE_H__ */

