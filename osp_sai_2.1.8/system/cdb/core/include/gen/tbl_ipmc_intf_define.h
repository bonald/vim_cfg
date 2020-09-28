
#ifndef __TBL_IPMC_INTF_DEFINE_H__
#define __TBL_IPMC_INTF_DEFINE_H__

/* TBL_IPMC_INTF field defines */
typedef enum
{
    TBL_IPMC_INTF_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPMC_INTF_FLD_PRIMARY_ADDRESS      = 1 ,  /* RW */
    TBL_IPMC_INTF_FLD_IGMP_ENABLE          = 2 ,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_ENABLE           = 3 ,  /* RW */
    TBL_IPMC_INTF_FLD_IGMP_VERSION         = 4 ,  /* RW */
    TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL  = 5 ,  /* RW */
    TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME = 6 ,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD     = 7 ,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME   = 8 ,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_GENERATION_ID    = 9 ,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_PROPAGATION_DELAY_MSEC = 10,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_OVERRIDE_INTERVAL_MSEC = 11,  /* RW */
    TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY      = 12,  /* RW */
    TBL_IPMC_INTF_FLD_MAX                  = 13
} tbl_ipmc_intf_field_id_t;

/* TBL_IPMC_INTF defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_ipmc_intf_key_t;

typedef struct
{
    tbl_ipmc_intf_key_t  key;
    addr_ipv4_t          primary_address;
    uint8                igmp_enable;
    uint8                pim_enable;
    int32                igmp_version;        /* igmp configuration */
    int32                igmp_query_interval;
    int32                igmp_query_max_response_time;
    int32                pim_hello_period;    /* pim hello configuration */
    int32                pim_hello_holdtime;
    uint32               pim_generation_id;
    uint16               pim_propagation_delay_msec;
    uint16               pim_override_interval_msec;
    uint32               pim_dr_priority;
} tbl_ipmc_intf_t;

typedef struct
{
    ctclib_hash_t        *pim_intf_hash;
    ctclib_slist_t       *pim_intf_list;
} tbl_ipmc_intf_master_t;

#endif /* !__TBL_IPMC_INTF_DEFINE_H__ */

