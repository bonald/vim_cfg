
#ifndef __TBL_MRT_DEFINE_H__
#define __TBL_MRT_DEFINE_H__

/* TBL_MRT field defines */
typedef enum
{
    TBL_MRT_FLD_KEY                  = 0 ,  /* READ */
    TBL_MRT_FLD_RPF_IFINDEX          = 1 ,  /* READ */
    TBL_MRT_FLD_OIF_INDEX_BMP        = 2 ,  /* READ */
    TBL_MRT_FLD_NHG_OID              = 3 ,  /* RW */
    TBL_MRT_FLD_PACKET_MATCH         = 4 ,  /* RW */
    TBL_MRT_FLD_BYTE_MATCH           = 5 ,  /* RW */
    TBL_MRT_FLD_STATIC_MROUTE        = 6 ,  /* RW */
    TBL_MRT_FLD_MAX                  = 7 
} tbl_mrt_field_id_t;

/* TBL_MRT defines */
typedef mrt_key_t tbl_mrt_key_t;

typedef struct
{
    tbl_mrt_key_t        key;
    uint32               rpf_ifindex;         /* data packet in ifindex */
    port_vlan_bmp_t      oif_index_bmp;       /* output ifindex */
    uint64               nhg_oid;
    uint64               packet_match;        /*stats from hardware*/
    uint64               byte_match;
    uint32               static_mroute;       /*static mroute or dynamic mroute*/
} tbl_mrt_t;

typedef struct
{
    ctclib_hash_t        *mrt_hash;
} tbl_mrt_master_t;

#endif /* !__TBL_MRT_DEFINE_H__ */

