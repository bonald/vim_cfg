
#ifndef __TBL_DHCSRVGRP_DEFINE_H__
#define __TBL_DHCSRVGRP_DEFINE_H__

/* TBL_DHCSRVGRP field defines */
typedef enum
{
    TBL_DHCSRVGRP_FLD_KEY                  = 0 ,  /* READ */
    TBL_DHCSRVGRP_FLD_CNT                  = 1 ,  /* READ */
    TBL_DHCSRVGRP_FLD_ADDRS                = 2 ,  /* RW */
    TBL_DHCSRVGRP_FLD_MAX                  = 3 
} tbl_dhcsrvgrp_field_id_t;

/* TBL_DHCSRVGRP defines */
typedef struct
{
    uint8                id;
} tbl_dhcsrvgrp_key_t;

typedef struct
{
    tbl_dhcsrvgrp_key_t  key;
    uint8                cnt;                 /* dhcp server addresses count, max 8 */
    char                 addrs[DHCP_SRV_ADDRS_BUF_SIZE]; /* server addresses buffer, format: address;address2;...;address16 */
} tbl_dhcsrvgrp_t;

typedef struct
{
    tbl_dhcsrvgrp_t      *dhcsrvgrp_array[GLB_MAX_DHCSRVGRP_NUM];
} tbl_dhcsrvgrp_master_t;

#endif /* !__TBL_DHCSRVGRP_DEFINE_H__ */

