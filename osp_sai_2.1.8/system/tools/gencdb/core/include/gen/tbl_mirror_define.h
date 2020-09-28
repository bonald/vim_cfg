
#ifndef __TBL_MIRROR_DEFINE_H__
#define __TBL_MIRROR_DEFINE_H__

/* TBL_MIRROR field defines */
typedef enum
{
    TBL_MIRROR_FLD_KEY                  = 0 ,  /* READ */
    TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP  = 1 ,  /* RW */
    TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP  = 2 ,  /* RW */
    TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP  = 3 ,  /* RW */
    TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP  = 4 ,  /* RW */
    TBL_MIRROR_FLD_CPU_SOURCE_DIR       = 5 ,  /* RW */
    TBL_MIRROR_FLD_DEST_TYPE            = 6 ,  /* RW */
    TBL_MIRROR_FLD_DEST_PORT_BMP        = 7 ,  /* RW */
    TBL_MIRROR_FLD_DEST_PORT            = 8 ,  /* RW */
    TBL_MIRROR_FLD_DEST_GROUP           = 9 ,  /* RW */
    TBL_MIRROR_FLD_DEST_VLAN            = 10,  /* RW */
    TBL_MIRROR_FLD_MAX                  = 11
} tbl_mirror_field_id_t;

/* TBL_MIRROR defines */
typedef struct
{
    uint32               id;
} tbl_mirror_key_t;

typedef struct
{
    tbl_mirror_key_t     key;
    port_bmp_t           source_igs_port_bmp;
    port_bmp_t           source_egs_port_bmp;
    vlan_bmp_t           source_igs_vlan_bmp;
    vlan_bmp_t           source_egs_vlan_bmp;
    uint32               cpu_source_dir;
    uint32               dest_type;
    port_bmp_t           dest_port_bmp;
    uint32               dest_port;
    uint32               dest_group;
    vid_t                dest_vlan;
} tbl_mirror_t;

typedef struct
{
    tbl_mirror_t         *mirror_array[GLB_MIRROR_MAX_SESSION_ID+1];
} tbl_mirror_master_t;

#endif /* !__TBL_MIRROR_DEFINE_H__ */

