
#ifndef __HSRV_OPENFLOW_BOND_H__
#define __HSRV_OPENFLOW_BOND_H__

#define HSRV_OPENFLOW_BOND_MASK                 0x3F         
#define HSRV_OPENFLOW_BOND_CHIPID               0x1F           /* bond port's global chip ID */
#define HSRV_OPENFLOW_BOND_ID(gport)           ((gport) & HSRV_OPENFLOW_BOND_MASK)
    
#define HSRV_OPENFLOW_BOND_PORT_LABEL(tid)   \
    (g_pst_openflow_bond_master->bond_label_base + tid)

#define HSRV_OPENFLOW_GET_BOND_TID(ifindex)   \
    (ifindex - GLB_BOND_IFINDEX_BASE)

#define HSRV_OPENFLOW_TRANS_TID_TO_GPORT(tid)   ((tid) | (GLB_BOND_CHIPID << 8))

struct hsrv_openflow_slave_data_s
{
    uint32 ifindex;    /* slave ifindex */
    uint32 port_config;

    bool enable;
};
typedef struct hsrv_openflow_slave_data_s hsrv_openflow_slave_data_t;

struct hsrv_openflow_bond_data_s
{
    uint32 ifindex;    /* bond ifindex */

    bool enable;
    uint32 port_config;
    uint32 label;
    uint8 tid;
    uint16 gport;
    uint16 ref_count;                   /* reference count by tunnel */
    
    hsrv_of_nh_info_t nh_info;
    ctclib_hash_t* bond_slave_hash;         /* key is slave ifindex */

    glb_openflow_tunnel_port_type_t tunnel_type;
};
typedef struct hsrv_openflow_bond_data_s hsrv_openflow_bond_data_t;

struct hsrv_openflow_slave_debug_data_s
{
    FILE* fp;
    hsrv_openflow_bond_data_t* p_bond;
};
typedef struct hsrv_openflow_slave_debug_data_s hsrv_openflow_slave_debug_data_t;

int32
hsrv_openflow_bond_lookup_info(uint32 ifindex, hsrv_openflow_bond_data_t **pp_info);

bool
hsrv_openflow_port_is_bond(uint32 bond_ifindex);

int32
hsrv_openflow_bond_get_tid(uint32 bond_ifindex, uint8 *p_tid);

bool
hsrv_openflow_bond_get_link_status(uint32 bond_ifindex);

int32
hsrv_openflow_bond_get_gport_by_ifindex(uint32 ifindex, uint16 *gport);

int32
hsrv_openflow_show_bond_brief(FILE *fp);

int32
hsrv_openflow_bond_start(void);

int32
hsrv_openflow_bond_init(hsrv_system_spec_info_t *p_system_info);

#endif /* !__HSRV_OPENFLOW_PORT_H__ */

