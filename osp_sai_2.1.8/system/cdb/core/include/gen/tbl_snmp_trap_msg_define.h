
#ifndef __TBL_SNMP_TRAP_MSG_DEFINE_H__
#define __TBL_SNMP_TRAP_MSG_DEFINE_H__

/* TBL_SNMP_TRAP_MSG field defines */
typedef enum
{
    TBL_SNMP_TRAP_MSG_FLD_MSG                  = 0 ,  /* RW */
    TBL_SNMP_TRAP_MSG_FLD_MAX                  = 1 
} tbl_snmp_trap_msg_field_id_t;

/* TBL_SNMP_TRAP_MSG defines */
typedef struct
{
    char                 msg[CMD_BUF_1024];
} tbl_snmp_trap_msg_t;

#endif /* !__TBL_SNMP_TRAP_MSG_DEFINE_H__ */

