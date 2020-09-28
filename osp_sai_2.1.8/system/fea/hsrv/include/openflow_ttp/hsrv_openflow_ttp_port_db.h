#ifndef __HSRV_OPENFLOW_TTP_PORT_DB_H__
#define __HSRV_OPENFLOW_TTP_PORT_DB_H__

struct hsrv_openflow_ttp_port_master_s
{
    ctclib_hash_t* p_port_hash;         /**< hsrv_openflow_ttp_port_info_t */
};
typedef struct hsrv_openflow_ttp_port_master_s hsrv_openflow_ttp_port_master_t;

extern hsrv_openflow_ttp_port_master_t *g_pst_openflow_ttp_port_master;

#define HSRV_OPENFLOW_TTP_PORT_HASH           g_pst_openflow_ttp_port_master->p_port_hash


int32
hsrv_openflow_portdb_add_port(hsrv_openflow_ttp_port_info_t* p_port);

int32
hsrv_openflow_portdb_del_port(uint32 ifindex);

int32
hsrv_openflow_portdb_lookup_port(uint32 ifindex, hsrv_openflow_ttp_port_info_t** pp_port);

int32
hsrv_openflow_portdb_start(void);

#endif
