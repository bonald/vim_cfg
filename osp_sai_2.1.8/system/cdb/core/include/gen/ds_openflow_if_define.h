
#ifndef __DS_OPENFLOW_IF_DEFINE_H__
#define __DS_OPENFLOW_IF_DEFINE_H__

/* DS_OPENFLOW_IF field defines */
typedef enum
{
    DS_OPENFLOW_IF_FLD_ENABLE               = 0 ,  /* RW */
    DS_OPENFLOW_IF_FLD_OBEY_VLAN_FILTER     = 1 ,  /* RW */
    DS_OPENFLOW_IF_FLD_MAX                  = 2 
} ds_openflow_if_field_id_t;

/* DS_OPENFLOW_IF defines */
typedef struct
{
    uint8                enable;              /* openflow enable, default:0 */
    uint8                obey_vlan_filter;    /* obey vlan filter, default: 0 */
} ds_openflow_if_t;

#endif /* !__DS_OPENFLOW_IF_DEFINE_H__ */

