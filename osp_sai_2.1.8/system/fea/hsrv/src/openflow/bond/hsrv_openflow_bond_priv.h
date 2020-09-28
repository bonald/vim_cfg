#ifndef __HSRV_OPENFLOW_BOND_PRIV_H__
#define __HSRV_OPENFLOW_BOND_PRIV_H__

#define HSRV_OPENFLOW_BOND_PORT_MAX_SIZE                       16

struct hsrv_openflow_bond_master_s
{
    ctclib_hash_t* bond_hash;         /* key is bond's ifindex */
    uint32 bond_label_base;
};
typedef struct hsrv_openflow_bond_master_s hsrv_openflow_bond_master_t;

#define HSRV_OPENFLOW_BOND_INFO_HASH               g_pst_openflow_bond_master->bond_hash

#endif

