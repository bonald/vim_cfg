
#ifndef __TBL_ISOLATION_DEFINE_H__
#define __TBL_ISOLATION_DEFINE_H__

/* TBL_ISOLATION field defines */
typedef enum
{
    TBL_ISOLATION_FLD_KEY                  = 0 ,  /* READ */
    TBL_ISOLATION_FLD_ISOLATION_OID        = 1 ,  /* READ */
    TBL_ISOLATION_FLD_PORTS                = 2 ,  /* READ */
    TBL_ISOLATION_FLD_MAX                  = 3 
} tbl_isolation_field_id_t;

/* TBL_ISOLATION defines */
typedef struct
{
    uint32               isolation_id;
} tbl_isolation_key_t;

typedef struct
{
    tbl_isolation_key_t  key;
    uint64               isolation_oid;
    cdb_reference_list_t ports;               /* list of isolation ports */
} tbl_isolation_t;

typedef struct
{
    tbl_isolation_t      *iso_array[GLB_PORT_ISOLATE_GROUP_MAX];
} tbl_isolation_master_t;

#endif /* !__TBL_ISOLATION_DEFINE_H__ */

