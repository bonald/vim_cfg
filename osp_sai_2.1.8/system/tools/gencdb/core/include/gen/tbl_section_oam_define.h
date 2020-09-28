
#ifndef __TBL_SECTION_OAM_DEFINE_H__
#define __TBL_SECTION_OAM_DEFINE_H__

/* TBL_SECTION_OAM field defines */
typedef enum
{
    TBL_SECTION_OAM_FLD_KEY                  = 0 ,  /* READ */
    TBL_SECTION_OAM_FLD_OAM_SESSION_ID       = 1 ,  /* RW */
    TBL_SECTION_OAM_FLD_NEXTHOP_MAC          = 2 ,  /* RW */
    TBL_SECTION_OAM_FLD_MAX                  = 3 
} tbl_section_oam_field_id_t;

/* TBL_SECTION_OAM defines */
typedef struct
{
    uint32               ifindex;
} tbl_section_oam_key_t;

typedef struct
{
    tbl_section_oam_key_t key;
    uint32               oam_session_id;
    mac_addr_t           nexthop_mac;
} tbl_section_oam_t;

typedef struct
{
    ctclib_hash_t        *section_oam_hash;
    ctclib_slist_t       *section_oam_list;
} tbl_section_oam_master_t;

#endif /* !__TBL_SECTION_OAM_DEFINE_H__ */

