
#ifndef __TBL_CFM_MD_DEFINE_H__
#define __TBL_CFM_MD_DEFINE_H__

#include "gen/ds_cfm_error_define.h"

/* TBL_CFM_MD field defines */
typedef enum
{
    TBL_CFM_MD_FLD_KEY                  = 0 ,  /* READ */
    TBL_CFM_MD_FLD_LEVEL                = 1 ,  /* RW */
    TBL_CFM_MD_FLD_NAME_TYPE            = 2 ,  /* RW */
    TBL_CFM_MD_FLD_PKT_NAME             = 3 ,  /* RW */
    TBL_CFM_MD_FLD_PKT_NAME_LENGTH      = 4 ,  /* RW */
    TBL_CFM_MD_FLD_PKT_NAME_INPUT       = 5 ,  /* RW */
    TBL_CFM_MD_FLD_SERVICE_NUMBER       = 6 ,  /* RW */
    TBL_CFM_MD_FLD_CFM_ERROR            = 7 ,  /* SUB */
    TBL_CFM_MD_FLD_MAX                  = 8 
} tbl_cfm_md_field_id_t;

/* TBL_CFM_MD defines */
typedef struct
{
    char                 name[CFM_MD_NAME_MAX_LENGTH_8021AG+1];
} tbl_cfm_md_key_t;

typedef struct
{
    tbl_cfm_md_key_t     key;
    uint32               level;               /* MD Level */
    uint8                name_type;           /* MD name type */
    char                 pkt_name[CFM_MD_NAME_MAX_LENGTH_8021AG+1]; /* valid when md name type is dns/string/mac address */
    uint8                pkt_name_length;     /* Packet md name length */
    char                 pkt_name_input[CFM_MD_NAME_MAX_LENGTH_8021AG+1]; /* valid when md name type is dns/string/mac address */
    uint32               service_number;      /* Service number */
    cdb_reference_list_t cfm_error;
} tbl_cfm_md_t;

typedef struct
{
    ctclib_hash_t        *cfm_md_hash;
    ctclib_slist_t       *cfm_md_list;
} tbl_cfm_md_master_t;

#endif /* !__TBL_CFM_MD_DEFINE_H__ */

