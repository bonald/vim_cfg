
#ifndef __TBL_MLAG_PORT_DEFINE_H__
#define __TBL_MLAG_PORT_DEFINE_H__

/* TBL_MLAG_PORT field defines */
typedef enum
{
    TBL_MLAG_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_MLAG_PORT_FLD_PEER_CONF            = 1 ,  /* READ */
    TBL_MLAG_PORT_FLD_PEER_IF_UP           = 2 ,  /* READ */
    TBL_MLAG_PORT_FLD_STP_PORT_ENABLE      = 3 ,  /* READ */
    TBL_MLAG_PORT_FLD_IFINDEX              = 4 ,  /* READ */
    TBL_MLAG_PORT_FLD_PROTECT_EN           = 5 ,  /* READ */
    TBL_MLAG_PORT_FLD_MAX                  = 6 
} tbl_mlag_port_field_id_t;

/* TBL_MLAG_PORT defines */
typedef struct
{
    uint32               id;
} tbl_mlag_port_key_t;

typedef struct
{
    tbl_mlag_port_key_t  key;
    uint32               peer_conf;
    uint32               peer_if_up;
    uint32               stp_port_enable;
    uint32               ifindex;
    uint32               protect_en;          /* is MLAG local port protect enable, update FDB to peer-link port */
    void                 *p_if;               /* refer to tbl_interface_t */
} tbl_mlag_port_t;

typedef struct
{
    ctclib_slist_t       *port_list;
} tbl_mlag_port_master_t;

#endif /* !__TBL_MLAG_PORT_DEFINE_H__ */

