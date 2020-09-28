#ifndef __HSRV_OPENFLOW_TPOAM_PRIV_H__
#define __HSRV_OPENFLOW_TPOAM_PRIV_H__

/* Master */
struct hsrv_openflow_tpoam_s
{
    ctclib_opf_t*        pst_mep_opf;                        /* OFP of MEP, allocate mep index */
    ctclib_hash_t*      pst_megid_hash;                    /* MEG hash table */
    ctclib_hash_t*      pst_section_lmep_hash;          /* SECTION LMEP&RMEP hash table */
    ctclib_hash_t*      pst_lsp_lmep_hash;                /* LSP LMEP&RMEP hash table */
    ctclib_hash_t*      pst_pw_lmep_hash;                /* PW LMEP&RMEP hash table */ 
    ctclib_hash_t*      pst_lmep_hash;                      /* SECTION & LSP & PW LMEP&RMEP hash table, mepIndex is key */
    ctclib_hash_t*      pst_mip_hash;                       /*MIP HASH*/
    ctclib_slist_t*     pst_mip_list;                          /*MIP LIST*/
    ctclib_opf_t*        pst_aps_offset_opf;                /* aps group id for chip */
    uint32                 fast_aps_en;
    uint32                 channel_type;
    uint8                   rdi_reason;
    uint32                 sf_reason;
    uint32                 pw_not_standard_mode;
};
typedef struct hsrv_openflow_tpoam_s hsrv_openflow_tpoam_t;


#define HSRV_TPOAM_MEGID_HASH_SIZE      1024
#define HSRV_TPOAM_LMEP_HASH_SIZE        1024

#define HSRV_APS_OFFSET_MIN 1
#define HSRV_APS_OFFSET_BLOCK_NUM 2
#define HSRV_APS_OFFSET_BLOCK_SIZE 1024
#define HSRV_APS_OFFSET_MAX    (HSRV_APS_OFFSET_BLOCK_NUM * HSRV_APS_OFFSET_BLOCK_SIZE -1)

#define HSRV_TPOAM_DBG_FUNC()\
do {\
    HSRV_TPOAM_COMMON_DEBUG("Func <%s>", __FUNCTION__);\
}while(0)

#endif /* !__HSRV_OPENFLOW_TPOAM_PRIV_H__ */
