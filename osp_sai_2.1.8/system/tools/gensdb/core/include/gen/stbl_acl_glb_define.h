
#ifndef __STBL_ACL_GLB_DEFINE_H__
#define __STBL_ACL_GLB_DEFINE_H__

/* STBL_ACL_GLB field defines */
typedef enum
{
    STBL_ACL_GLB_FLD_ACL_TBL_MAX_COUNT    = 0 ,  /* READ */
    STBL_ACL_GLB_FLD_ACL_ENTRY_MAX_COUNT  = 1 ,  /* READ */
    STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_MAX_COUNT = 2 ,  /* READ */
    STBL_ACL_GLB_FLD_TUNNEL_MAX_COUNT     = 3 ,  /* READ */
    STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB_MAX_COUNT = 4 ,  /* READ */
    STBL_ACL_GLB_FLD_ACL_TBL_OPB          = 5 ,  /* READ */
    STBL_ACL_GLB_FLD_ACL_ENTRY_OPB        = 6 ,  /* READ */
    STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_OPB    = 7 ,  /* READ */
    STBL_ACL_GLB_FLD_TUNNEL_OPB           = 8 ,  /* READ */
    STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB   = 9 ,  /* READ */
    STBL_ACL_GLB_FLD_MAX                  = 10
} stbl_acl_glb_field_id_t;

/* STBL_ACL_GLB defines */
typedef struct
{
    uint32               acl_tbl_max_count;
    uint32               acl_entry_max_count;
    uint32               sdk_acl_entry_max_count;
    uint32               tunnel_max_count;
    uint32               erspan_service_opb_max_count;
    ctclib_opb_t         acl_tbl_opb;
    ctclib_opb_t         acl_entry_opb;
    ctclib_opb_t         sdk_acl_entry_opb;
    ctclib_opb_t         tunnel_opb;
    ctclib_opb_t         erspan_service_opb;
} stbl_acl_glb_t;

#endif /* !__STBL_ACL_GLB_DEFINE_H__ */

