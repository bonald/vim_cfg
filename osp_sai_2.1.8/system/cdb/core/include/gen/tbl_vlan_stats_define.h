
#ifndef __TBL_VLAN_STATS_DEFINE_H__
#define __TBL_VLAN_STATS_DEFINE_H__

/* TBL_VLAN_STATS field defines */
typedef enum
{
    TBL_VLAN_STATS_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLAN_STATS_FLD_PKTCNTL2IN           = 1 ,  /* READ */
    TBL_VLAN_STATS_FLD_PKTCNTL2OUT          = 2 ,  /* READ */
    TBL_VLAN_STATS_FLD_PKTCNTL3IN           = 3 ,  /* READ */
    TBL_VLAN_STATS_FLD_PKTCNTL3OUT          = 4 ,  /* READ */
    TBL_VLAN_STATS_FLD_PKTCNTACL            = 5 ,  /* READ */
    TBL_VLAN_STATS_FLD_BYTECNTL2IN          = 6 ,  /* READ */
    TBL_VLAN_STATS_FLD_BYTECNTL2OUT         = 7 ,  /* READ */
    TBL_VLAN_STATS_FLD_BYTECNTL3IN          = 8 ,  /* READ */
    TBL_VLAN_STATS_FLD_BYTECNTL3OUT         = 9 ,  /* READ */
    TBL_VLAN_STATS_FLD_BYTECNTACL           = 10,  /* READ */
    TBL_VLAN_STATS_FLD_MAX                  = 11
} tbl_vlan_stats_field_id_t;

/* TBL_VLAN_STATS defines */
typedef struct
{
    vid_t                vid;
} tbl_vlan_stats_key_t;

typedef struct
{
    tbl_vlan_stats_key_t key;
    uint64               pktcntl2in;
    uint64               pktcntl2out;
    uint64               pktcntl3in;
    uint64               pktcntl3out;
    uint64               pktcntacl;
    uint64               bytecntl2in;
    uint64               bytecntl2out;
    uint64               bytecntl3in;
    uint64               bytecntl3out;
    uint64               bytecntacl;
} tbl_vlan_stats_t;

typedef struct
{
    tbl_vlan_stats_t     *vlan_stats_array[GLB_MAX_VLAN_ID];
} tbl_vlan_stats_master_t;

#endif /* !__TBL_VLAN_STATS_DEFINE_H__ */

