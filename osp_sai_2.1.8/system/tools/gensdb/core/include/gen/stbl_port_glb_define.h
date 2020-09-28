
#ifndef __STBL_PORT_GLB_DEFINE_H__
#define __STBL_PORT_GLB_DEFINE_H__

/* STBL_PORT_GLB field defines */
typedef enum
{
    STBL_PORT_GLB_FLD_GROUP_ID_IGS_OPB     = 0 ,  /* READ */
    STBL_PORT_GLB_FLD_GROUP_ID_EGS_OPB     = 1 ,  /* READ */
    STBL_PORT_GLB_FLD_TTP_GROUP_ID_IGS_OPB = 2 ,  /* READ */
    STBL_PORT_GLB_FLD_PORT_CLASS_ID_OPB    = 3 ,  /* READ */
    STBL_PORT_GLB_FLD_HOSTIF_ID_OPB        = 4 ,  /* READ */
    STBL_PORT_GLB_FLD_HOSTIF_GROUP_ID_OPB  = 5 ,  /* READ */
    STBL_PORT_GLB_FLD_ROUTE_IF_OPB         = 6 ,  /* READ */
    STBL_PORT_GLB_FLD_MAX                  = 7 
} stbl_port_glb_field_id_t;

/* STBL_PORT_GLB defines */
typedef struct
{
    ctclib_opb_t         group_id_igs_opb;
    ctclib_opb_t         group_id_egs_opb;
    ctclib_opb_t         ttp_group_id_igs_opb;
    ctclib_opb_t         port_class_id_opb;
    ctclib_opb_t         hostif_id_opb;
    ctclib_opb_t         hostif_group_id_opb;
    ctclib_opb_t         route_if_opb;
} stbl_port_glb_t;

#endif /* !__STBL_PORT_GLB_DEFINE_H__ */

