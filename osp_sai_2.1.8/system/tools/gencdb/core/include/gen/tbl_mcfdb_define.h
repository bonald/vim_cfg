
#ifndef __TBL_MCFDB_DEFINE_H__
#define __TBL_MCFDB_DEFINE_H__

/* TBL_MCFDB field defines */
typedef enum
{
    TBL_MCFDB_FLD_KEY                  = 0 ,  /* READ */
    TBL_MCFDB_FLD_L2MC_PORT            = 1 ,  /* READ */
    TBL_MCFDB_FLD_ACTION_IFINDEX       = 2 ,  /* READ */
    TBL_MCFDB_FLD_MANUAL               = 3 ,  /* READ */
    TBL_MCFDB_FLD_MAX                  = 4 
} tbl_mcfdb_field_id_t;

/* TBL_MCFDB defines */
typedef fdb_key_t tbl_mcfdb_key_t;

typedef struct
{
    tbl_mcfdb_key_t      key;
    port_bmp_t           l2mc_port;           /* bitmap of ifindex */
    uint32               action_ifindex;      /* newest changed action (high u16) and ifindex (low u16) */
    uint32               manual;
} tbl_mcfdb_t;

typedef struct
{
    ctclib_hash_t        *mcfdb_hash;
} tbl_mcfdb_master_t;

#endif /* !__TBL_MCFDB_DEFINE_H__ */

