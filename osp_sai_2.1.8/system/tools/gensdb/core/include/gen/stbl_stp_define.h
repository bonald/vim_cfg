
#ifndef __STBL_STP_DEFINE_H__
#define __STBL_STP_DEFINE_H__

/* STBL_STP field defines */
typedef enum
{
    STBL_STP_FLD_KEY                  = 0 ,  /* READ */
    STBL_STP_FLD_STP_OID              = 1 ,  /* READ */
    STBL_STP_FLD_VLAN_BMP             = 2 ,  /* READ */
    STBL_STP_FLD_MAX                  = 3 
} stbl_stp_field_id_t;

/* STBL_STP defines */
typedef struct
{
    uint32               stp_id;
} stbl_stp_key_t;

typedef struct
{
    stbl_stp_key_t       key;
    sai_object_id_t      stp_oid;             /* STP instance OID */
    vlan_bmp_t           vlan_bmp;            /* VLAN bitmap of this instance */
} stbl_stp_t;

typedef struct
{
    stbl_stp_t           *array[GLB_STP_INSTANCE_MAX];
} stbl_stp_master_t;

#endif /* !__STBL_STP_DEFINE_H__ */

