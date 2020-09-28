
#ifndef __TBL_DHCRELAY_DEFINE_H__
#define __TBL_DHCRELAY_DEFINE_H__

/* TBL_DHCRELAY field defines */
typedef enum
{
    TBL_DHCRELAY_FLD_ENABLE               = 0 ,  /* RW */
    TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH  = 1 ,  /* RW */
    TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS    = 2 ,  /* RW */
    TBL_DHCRELAY_FLD_AGENT_PROC_OPTION    = 3 ,  /* RW */
    TBL_DHCRELAY_FLD_TRUST_ALL            = 4 ,  /* RW */
    TBL_DHCRELAY_FLD_MAX                  = 5 
} tbl_dhcrelay_field_id_t;

/* TBL_DHCRELAY defines */
typedef struct
{
    uint8                enable;              /* enable / disable */
    uint8                drop_agent_mismatch; /* drop agent mismatch packets */
    uint8                add_agent_options;   /* add agent options or not */
    uint8                agent_proc_option;   /* append / replace / untouched / discard */
    uint8                trust_all;           /* trust all interface or not */
} tbl_dhcrelay_t;

#endif /* !__TBL_DHCRELAY_DEFINE_H__ */

