
#ifndef __TBL_FEA_PORT_IF_DEFINE_H__
#define __TBL_FEA_PORT_IF_DEFINE_H__

/* TBL_FEA_PORT_IF field defines */
typedef enum
{
    TBL_FEA_PORT_IF_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_PORT_IF_FLD_IFINDEX              = 1 ,  /* RW */
    TBL_FEA_PORT_IF_FLD_NAME                 = 2 ,  /* RW */
    TBL_FEA_PORT_IF_FLD_LAG_MEMBER           = 3 ,  /* RW */
    TBL_FEA_PORT_IF_FLD_MAX                  = 4 
} tbl_fea_port_if_field_id_t;

/* TBL_FEA_PORT_IF defines */
typedef struct
{
    uint64               portid;
} tbl_fea_port_if_key_t;

typedef struct
{
    tbl_fea_port_if_key_t key;
    uint32               ifindex;             /* ifindex assign in user-space(PMs, Fea) to indicate an interface */
    char                 name[IFNAME_SIZE];
    uint64               lag_member;
} tbl_fea_port_if_t;

typedef struct
{
    ctclib_hash_t        *portif_hash;
    ctclib_slist_t       *portif_list;
} tbl_fea_port_if_master_t;

#endif /* !__TBL_FEA_PORT_IF_DEFINE_H__ */

