
#ifndef __TBL_MLAG_DEFINE_H__
#define __TBL_MLAG_DEFINE_H__

/* TBL_MLAG field defines */
typedef enum
{
    TBL_MLAG_FLD_ENABLE               = 0 ,  /* RW */
    TBL_MLAG_FLD_LOCAL_ADDR           = 1 ,  /* RW */
    TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL = 2 ,  /* RW */
    TBL_MLAG_FLD_RELOAD_DELAY_AUTO    = 3 ,  /* RW */
    TBL_MLAG_FLD_ROLE                 = 4 ,  /* READ */
    TBL_MLAG_FLD_LOCAL_SYSID          = 5 ,  /* READ */
    TBL_MLAG_FLD_REMOTE_SYSID         = 6 ,  /* READ */
    TBL_MLAG_FLD_MLAG_SYSID           = 7 ,  /* READ */
    TBL_MLAG_FLD_PEER_ROUTEMAC        = 8 ,  /* READ */
    TBL_MLAG_FLD_LOCAL_SYSPRI         = 9 ,  /* READ */
    TBL_MLAG_FLD_REMOTE_SYSPRI        = 10,  /* READ */
    TBL_MLAG_FLD_MLAG_SYSPRI          = 11,  /* READ */
    TBL_MLAG_FLD_PEER_VLANIF_BMP      = 12,  /* READ */
    TBL_MLAG_FLD_PEER_FDB_BMP         = 13,  /* READ */
    TBL_MLAG_FLD_PEER_LINK_IFINDEX    = 14,  /* RW */
    TBL_MLAG_FLD_FLUSH_FDB_MI         = 15,  /* READ */
    TBL_MLAG_FLD_RELOAD_DELAY_TIMER   = 16,  /* READ */
    TBL_MLAG_FLD_MAX                  = 17
} tbl_mlag_field_id_t;

/* TBL_MLAG defines */
typedef struct
{
    uint32               enable;
    addr_t               local_addr;
    uint32               reload_delay_interval;
    uint32               reload_delay_auto;
    uint32               role;
    mac_addr_t           local_sysid;
    mac_addr_t           remote_sysid;
    mac_addr_t           mlag_sysid;
    mac_addr_t           peer_routemac;
    uint16               local_syspri;
    uint16               remote_syspri;
    uint16               mlag_syspri;
    vlan_bmp_t           peer_vlanif_bmp;
    vlan_bmp_t           peer_fdb_bmp;
    uint32               peer_link_ifindex;
    uint32               flush_fdb_mi;        /* flush FDB MI when MLAG connection is closed */
    ctc_task_t           *reload_delay_timer;
    void                 *peer_link_if;       /* refer to tbl_interface_t */
} tbl_mlag_t;

#endif /* !__TBL_MLAG_DEFINE_H__ */

