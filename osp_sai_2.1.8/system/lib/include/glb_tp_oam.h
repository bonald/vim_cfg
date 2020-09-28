#ifndef __GLB_TP_OAM_DEFINE_H__
#define __GLB_TP_OAM_DEFINE_H__

#define GLB_TPOAM_UPLOAD_EVENT_SET    1
#define GLB_TPOAM_UPLOAD_EVENT_CLEAR     2

#define GLB_TPOAM_EXEC_SEND_LB_MSG_TIMER     50

/* the frame ration will be displayed as 50.0000% */
#define TPOAM_LM_FRAME_LOSS_RATIO_BITS      1000000ULL
#define TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS  10000
#define TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10  10
#define TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000  1000

#define MTYPE_LINK_NODE MEM_PM_TEMP
#define MTYPE_LINK_LIST MEM_PM_TEMP

typedef enum
{
    TPOAM_HAL_MSG_TYPE_TPOAM_EVENT = 0, 
    TPOAM_HAL_MSG_TYPE_LM_DUAL_COUNTERS,
    TPOAM_HAL_MSG_TYPE_LM_SD,

    TPOAM_HAL_MSG_TYPE_MAX
} tpoam_fei_msg_type_t;

/* DM type */
#define TPOAM_DM_TYPE_1DM               0
#define TPOAM_DM_TYPE_DMM               1

/* DM interval */
#define TPOAM_DM_INTERVAL_100MS         0
#define TPOAM_DM_INTERVAL_1S            1
#define TPOAM_DM_INTERVAL_10S           2

#define FLAG_ISSET(V,F)        (((V) & (F)) == (F))

/* CC Interval */
#define TPOAM_MIN_CC_INTERVAL       1
#define TPOAM_MAX_CC_INTERVAL       7

enum glb_tpoam_tx_key_type
{
    GLB_TPOAM_TX_KEY_TYPE_SECTION_IP,
    GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP,
    GLB_TPOAM_TX_KEY_TYPE_LSP,
    GLB_TPOAM_TX_KEY_TYPE_PW,
};

/* ICC length */
#define TPOAM_ICC_LEN_MIN               1
#define TPOAM_ICC_LEN_MAX               6
#define TPOAM_ICC_LEN_DEF               6

/* MEG Level */
#define TPOAM_MIN_MEG_LEVEL         0
#define TPOAM_MAX_MEG_LEVEL         7
#define TPOAM_DEF_MEG_LEVEL         7

/* default TTL */
#define TPOAM_TTL_DEF                   255

/* wait for last DMR */
#define TPOAM_DM_LAST_DMR_WAIT_TIME     2

#define TPOAM_STATUS_DOWN       0
#define TPOAM_STATUS_UP         1

#define TPOAM_LB_DISCOVERY              0
#define TPOAM_LB_DISCOVERY_EGRESS       1   /* unsupport */
#define TPOAM_LB_MEP                    2
#define TPOAM_LB_MIP                    3
#define TPOAM_LB_ALL                    4

/* ICC length */
#define GLB_TPOAM_ICC_LEN               6

#define GLB_TP_OAM_DEFAULT_MEGID    1
#define GLB_TP_OAM_DEFAULT_MEPID     1
#define GLB_TP_OAM_DEFAULT_RMEPID     1
#define GLB_TP_OAM_DEFAULT_CC_ENABLE_STATE     TRUE
#define GLB_TP_OAM_DEFAULT_CC_INTERVAL     1
#ifdef DUET2
#ifdef _GLB_UML_SYSTEM_
#define GLB_TP_OAM_DEFAULT_CC_INTERVAL_PW     1
#else
#define GLB_TP_OAM_DEFAULT_CC_INTERVAL_PW     2
#endif
#else
#define GLB_TP_OAM_DEFAULT_CC_INTERVAL_PW     2
#endif
#define GLB_TP_OAM_DEFAULT_LM_ENABLE_STATE     TRUE
#define GLB_TP_OAM_DEFAULT_DM_ENABLE_STATE     TRUE

#define GLB_OAM_TYPE_MPLS_SECTION  1
#define GLB_OAM_TYPE_MPLS_LSP          2
#define GLB_OAM_TYPE_MPLS_PW           3
#define GLB_OAM_TYPE_MPLS_ALL          4

#define GLB_TPOAM_MEGID_LEN             16      /* 3 Bytes header + 13 Bytes MEG name string */
#define GLB_TPOAM_MEGID_STR_LEN     13

#define GLB_TPOAM_MEP_INDEX_MIN         2       /* MEP index: start from 2. 0, 1 is reserved */
#define GLB_TPOAM_MEP_INDEX_NUM         1024
#define GLB_TPOAM_MEP_INDEX_MAX         2049

#define GLB_TPOAM_DM_CACHE_SIZE         128

/* MEP */
#define TPOAM_INVALID_MEP_ID        0
#define TPOAM_MIN_MEP_ID            1
#define TPOAM_MAX_MEP_ID            8191

#define TPOAM_API_CLEAR_TYPE_EVENT  0
#define TPOAM_API_CLEAR_TYPE_1DM    1

#define GLB_TPOAM_UPDATE_CC_EN            (1<<0)
#define GLB_TPOAM_UPDATE_LM_EN            (1<<1)
#define GLB_TPOAM_UPDATE_LM_TYPE            (1<<2)
#define GLB_TPOAM_UPDATE_LM_COS_TYPE            (1<<3)
#define GLB_TPOAM_UPDATE_LM_COS            (1<<4)
#define GLB_TPOAM_UPDATE_TX_COS_EXP            (1<<5)
#define GLB_TPOAM_UPDATE_NHOP            (1<<6)
#define GLB_TPOAM_UPDATE_RDI            (1<<7)
#define GLB_TPOAM_UPDATE_MEP_EN            (1<<8)
#define GLB_TPOAM_UPDATE_EXP            (1<<9)
#define GLB_TPOAM_UPDATE_DM_EN            (1<<10)
#define GLB_TPOAM_UPDATE_MEL            (1<<11)
#define GLB_TPOAM_UPDATE_CC_INTERVAL            (1<<12)

//#define TPOAM_SERVICE_STR_SIZE      32
#define TPOAM_SERVICE_STR_SIZE      128

#define TPOAM_DESCRIPTION_SIZE      33

#define TPOAM_LM_DUAL_STATS_INTERVAL_MAX     10
#define TPOAM_LM_DUAL_STATS_INTERVAL_MIN      1
#define TPOAM_LM_DUAL_STATS_INTERVAL_DEFAULT  1

#define TPOAM_LM_DUAL_CACHE_SIZE_MAX          512
#define TPOAM_LM_DUAL_CACHE_SIZE_MIN          1
#define TPOAM_LM_DUAL_CACHE_SIZE_DEFAULT      128

#define TPOAM_LM_INTERVAL_100MS         0
#define TPOAM_LM_INTERVAL_1S            1
#define TPOAM_LM_INTERVAL_10S           2

#define GLB_MPLS_OAM_ACH_CHAN_TYPE_MCC    0x1
#define GLB_MPLS_OAM_ACH_CHAN_TYPE_SCC    0x2
#define GLB_MPLS_OAM_ACH_CHAN_TYPE_Y1731    0x8902

#define TPOAM_PRIORITY_MAX            7
#define TPOAM_PRIORITY_MIN            0

#define MPLS_LSP_TYPE_PE   0
#define MPLS_LSP_TYPE_P    1

/* packet buffer for DM, AIS, etc. */
#define TPOAM_PKT_BUF_SIZE              256

/* default EXP */
#define TPOAM_EXP_MIN                   0
#define TPOAM_EXP_MAX                   7
//#define TPOAM_EXP_DEF                   7
#define TPOAM_EXP_DEF                   6



#define SNMP_ERR_NOERROR                (0)     /* XXX  Used only for PDUs? */
#define SNMP_ERR_TOOBIG	                (1)
#define SNMP_ERR_NOSUCHNAME             (2)
#define SNMP_ERR_BADVALUE               (3)
#define SNMP_ERR_READONLY               (4)
#define SNMP_ERR_GENERR	                (5)

    /*
     * in SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs 
     */
#define SNMP_ERR_NOACCESS		(6)
#define SNMP_ERR_WRONGTYPE		(7)
#define SNMP_ERR_WRONGLENGTH		(8)
#define SNMP_ERR_WRONGENCODING		(9)
#define SNMP_ERR_WRONGVALUE		(10)
#define SNMP_ERR_NOCREATION		(11)
#define SNMP_ERR_INCONSISTENTVALUE	(12)
#define SNMP_ERR_RESOURCEUNAVAILABLE	(13)
#define SNMP_ERR_COMMITFAILED		(14)
#define SNMP_ERR_UNDOFAILED		(15)
#define SNMP_ERR_AUTHORIZATIONERROR	(16)
#define SNMP_ERR_NOTWRITABLE		(17)

#define MPLS_LSP_P_MODE               140/*add by dingyi for co-routed lsp, 2011-04-27*/
#define MPLS_LSP_PE_MODE              139/*add by dingyi for co-routed lsp, 2011-04-27*/
#define MPLS_TP_OAM_SECTION_MODE      146/* Added by kcao for MPLS-TP OAM develop, bug 15633, 2011-07-28 */
#define MPLS_PW_TPE_MODE        158

enum tpoam_lm_cos_type
{
  TPOAM_LM_COS_TYPE_ALL = 0,
  TPOAM_LM_COS_TYPE_SPECIFIED_COS = 1,
  TPOAM_LM_COS_TYPE_PER_COS = 2,
};
#define TPOAM_LM_COS_TYPE_DEFAULT        TPOAM_LM_COS_TYPE_ALL

/* cache size for event */
#define GLB_TPOAM_EVENT_CACHE_SIZE      32

#ifndef support_many_event
/* Event bits defines */
enum glb_tpoam_evt_e
{
    GLB_TPOAM_EVT_NONE             = 0x00000000,
    GLB_TPOAM_EVT_LOC              = 0x00000001,    /* HW state, DLOC : 3 - RMEP */
    GLB_TPOAM_EVT_UNEXP_MEP        = 0x00000002,    /* HW state, CCM Error : 4 - LMEP */
    GLB_TPOAM_EVT_UNEXP_PERIOD     = 0x00000004,    /* HW state, CCM Error : 4 - RMEP */
    GLB_TPOAM_EVT_MISMERGE         = 0x00000008,    /* HW state, CCM XCON : 5 - LMEP */
    GLB_TPOAM_EVT_MEG_LVL          = 0x00000010,    /* HW state, CCM XCON : 5 - LMEP */
    GLB_TPOAM_EVT_WAIT_1ST_CC      = 0x00000020,    /* HW state, Not rx first valid CCM PDU - RMEP */
    GLB_TPOAM_EVT_RDI_RX           = 0x00000040,    /* HW state, RDI : 1 - RMEP */
    GLB_TPOAM_EVT_RDI_TX           = 0x00000080,    /* HW state, RDI TX : set by HW, clear by SW */
    //GLB_TPOAM_EVT_AIS_RX           = 0x00000100,    /* SW state, AIS RX */
    //GLB_TPOAM_EVT_AIS_TX           = 0x00000200,    /* SW state, AIS TX */
    //GLB_TPOAM_EVT_AIS_DEFECT       = 0x00000400,    /* SW state, AIS defect */
    //GLB_TPOAM_EVT_LCK_RX           = 0x00000800,    /* SW state, LCK RX defect */
    //GLB_TPOAM_EVT_LCK_TX           = 0x00001000,    /* SW state, LCK TX defect */
    //GLB_TPOAM_EVT_LCK_DEFECT       = 0x00002000,    /* SW state, LCK defect */
    //GLB_TPOAM_EVT_EVT_CACHE_FULL   = 0x00004000,    /* SW state, Event cache is full */
    //GLB_TPOAM_EVT_1DM_CACHE_FULL   = 0x00008000,    /* SW state, 1DM cache is full */
    //GLB_TPOAM_EVT_CSF_DEFECT       = 0x00010000,    /* Client Signal Fail */
    /* modified by yaom for CRC trigger AIS 20140919 */    
    GLB_TPOAM_EVT_CRC_DEFECT       = 0x00020000     /* SW state, CRC defect */
};
typedef enum glb_tpoam_evt_e glb_tpoam_evt_t;
#else
enum glb_tpoam_evt_e
{
    GLB_TPOAM_EVT_NONE             = 0x00000000,
    GLB_TPOAM_EVT_LOC              = 0x00000001,    /* HW state, DLOC : 3 - RMEP */
    GLB_TPOAM_EVT_UNEXP_MEP        = 0x00000002,    /* HW state, CCM Error : 4 - LMEP */
    GLB_TPOAM_EVT_UNEXP_PERIOD     = 0x00000004,    /* HW state, CCM Error : 4 - RMEP */
    GLB_TPOAM_EVT_MISMERGE         = 0x00000008,    /* HW state, CCM XCON : 5 - LMEP */
    GLB_TPOAM_EVT_MEG_LVL          = 0x00000010,    /* HW state, CCM XCON : 5 - LMEP */
    GLB_TPOAM_EVT_WAIT_1ST_CC      = 0x00000020,    /* HW state, Not rx first valid CCM PDU - RMEP */
    GLB_TPOAM_EVT_RDI_RX           = 0x00000040,    /* HW state, RDI : 1 - RMEP */
    GLB_TPOAM_EVT_RDI_TX           = 0x00000080,    /* HW state, RDI TX : set by HW, clear by SW */
    GLB_TPOAM_EVT_AIS_RX           = 0x00000100,    /* SW state, AIS RX */
    GLB_TPOAM_EVT_AIS_TX           = 0x00000200,    /* SW state, AIS TX */
    GLB_TPOAM_EVT_AIS_DEFECT       = 0x00000400,    /* SW state, AIS defect */
    GLB_TPOAM_EVT_LCK_RX           = 0x00000800,    /* SW state, LCK RX defect */
    GLB_TPOAM_EVT_LCK_TX           = 0x00001000,    /* SW state, LCK TX defect */
    GLB_TPOAM_EVT_LCK_DEFECT       = 0x00002000,    /* SW state, LCK defect */
    GLB_TPOAM_EVT_EVT_CACHE_FULL   = 0x00004000,    /* SW state, Event cache is full */
    GLB_TPOAM_EVT_1DM_CACHE_FULL   = 0x00008000,    /* SW state, 1DM cache is full */
    GLB_TPOAM_EVT_CSF_DEFECT       = 0x00010000,    /* Client Signal Fail */
    /* modified by yaom for CRC trigger AIS 20140919 */    
    GLB_TPOAM_EVT_CRC_DEFECT       = 0x00020000     /* SW state, CRC defect */
};
typedef enum glb_tpoam_evt_e glb_tpoam_evt_t;
#endif

/* DM time interval defines : unit is ns */
typedef int64 tpoam_interval_t;

#define GLB_TPOAM_EVT_NUM           18  /* Need change with glb_tpoam_evt_t */

/* Event reason bitmap */
#ifdef support_many_event
#define TPOAM_EVT_ALL           ( GLB_TPOAM_EVT_LOC \
                                    | GLB_TPOAM_EVT_UNEXP_MEP \
                                    | GLB_TPOAM_EVT_UNEXP_PERIOD \
                                    | GLB_TPOAM_EVT_MISMERGE \
                                    | GLB_TPOAM_EVT_MEG_LVL \
                                    | GLB_TPOAM_EVT_RDI_RX \
                                    | GLB_TPOAM_EVT_RDI_TX \
                                    | GLB_TPOAM_EVT_WAIT_1ST_CC \
                                    | GLB_TPOAM_EVT_AIS_RX \
                                    | GLB_TPOAM_EVT_AIS_TX \
                                    | GLB_TPOAM_EVT_AIS_DEFECT \
                                    | GLB_TPOAM_EVT_LCK_RX \
                                    | GLB_TPOAM_EVT_LCK_TX \
                                    | GLB_TPOAM_EVT_LCK_DEFECT \
                                    | GLB_TPOAM_EVT_EVT_CACHE_FULL \
                                    | GLB_TPOAM_EVT_1DM_CACHE_FULL \
                                    | GLB_TPOAM_EVT_CRC_DEFECT)
#else
#define TPOAM_EVT_ALL           ( GLB_TPOAM_EVT_LOC \
                                    | GLB_TPOAM_EVT_UNEXP_MEP \
                                    | GLB_TPOAM_EVT_UNEXP_PERIOD \
                                    | GLB_TPOAM_EVT_MISMERGE \
                                    | GLB_TPOAM_EVT_MEG_LVL \
                                    | GLB_TPOAM_EVT_RDI_RX \
                                    | GLB_TPOAM_EVT_RDI_TX \
                                    | GLB_TPOAM_EVT_WAIT_1ST_CC \
                                    | GLB_TPOAM_EVT_CRC_DEFECT)
#endif
#define TPOAM_EVT_NONE          ( GLB_TPOAM_EVT_NONE )

#define TPOAM_EVT_LOG_DEF       ( GLB_TPOAM_EVT_NONE ) 
#define TPOAM_EVT_TRAP_DEF      ( GLB_TPOAM_EVT_NONE )
#define TPOAM_EVT_CACHE_DEF     ( TPOAM_EVT_ALL ) 

/* Event cache size */
#define TPOAM_EVT_CACHE_SIZE_DEF    8
#define TPOAM_EVT_CACHE_SIZE_MIN    8
#define TPOAM_EVT_CACHE_SIZE_MAX    256

/* 1DM cache size */
#define TPOAM_1DM_CACHE_SIZE_DEF    8
#define TPOAM_1DM_CACHE_SIZE_MIN    8
#define TPOAM_1DM_CACHE_SIZE_MAX    256

/* MEG */
#define TPOAM_MEGID_RESERVED        1
#define TPOAM_MEGID_FORMAT          32
#define GLB_TPOAM_MEGID_STR_DEFAULT "megdefault"

enum tpoam_mp_type_e
{
    TPOAM_MP_TYPE_MEP,
    TPOAM_MP_TYPE_MIP,
    TPOAM_MP_TYPE_MAX
};
typedef enum tpoam_mp_type_e tpoam_mp_type_t;

enum tpoam_pw_type_e
{
    TPOAM_PW_TYPE_VPWS,
    TPOAM_PW_TYPE_VPLS,
    TPOAM_PW_TYPE_SPE,
    TPOAM_PW_TYPE_TPE,
    TPOAM_PW_TYPE_MAX
};
typedef enum tpoam_pw_type_e tpoam_pw_type_t;


/* used by HSRV & HAGT */
struct glb_tpoam_event_entry_s
{
    uint16  lmep_index;
    uint16  rmep_index;
    uint32  event_bmp;
};
typedef struct glb_tpoam_event_entry_s glb_tpoam_event_entry_t;


/* LB discovery reply defines */
struct tpoam_lb_reply_s
{
    uint8   ttl;
    uint8   lb_sub_type;                /* TPOAM_LB_DISCOVERY, TPOAM_LB_MEP, TPOAM_LB_MIP */
    uint16  mep_id;
    uint8   icc[GLB_TPOAM_ICC_LEN+1];
    uint32  node_id;
    uint32  if_num;
};
typedef struct tpoam_lb_reply_s tpoam_lb_reply_t;

/* LB discovery defines */
struct tpoam_lb_discovery_s
{
    uint8                       curr_ttl;
    uint8                       timeout_ttl;
    tpoam_lb_reply_t        reply;
};
typedef struct tpoam_lb_discovery_s tpoam_lb_discovery_t;


struct tpoam_ratelimit_stats_s
{
    uint32  count;
    uint32  limited;
};
typedef struct tpoam_ratelimit_stats_s tpoam_ratelimit_stats_t;

struct tpoam_ratelimit_param_s
{
    uint32  token_interval;
    uint32  token_rate;
    uint32  token_max;
};
typedef struct tpoam_ratelimit_param_s tpoam_ratelimit_param_t;

struct tpoam_stats_tx_item_s
{
    uint32  lbm;
    uint32  lbr;
    uint32  ais;
    uint32  onedm;
    uint32  dmm;
    uint32  dmr;
    uint32  lck;
    uint32  csf;
    uint32  aps;
};
typedef struct tpoam_stats_tx_item_s tpoam_stats_tx_item_t;

struct tpoam_stats_rx_item_s
{
    uint32  lbm;
    uint32  lbr;
    uint32  ais;
    uint32  onedm;
    uint32  dmm;
    uint32  dmr;
    uint32  lck;
    uint32  csf;    
    uint32  aps;
    uint32  unexp_ccm;
    uint32  unknown;
};
typedef struct tpoam_stats_rx_item_s tpoam_stats_rx_item_t;

struct tpoam_stats_err_item_s
{
    uint32  lbm;
    uint32  lbr;
    uint32  ais;
    uint32  onedm;
    uint32  dmm;
    uint32  dmr;
    uint32  lck;
    uint32  csf;    
    uint32  aps;
    uint32  unexp_ccm;
    uint32  mac_withdraw;
    uint32  unknown;
};
typedef struct tpoam_stats_err_item_s tpoam_stats_err_item_t;

struct tpoam_sk_buff_s
{
    uint8 *head;
    uint8 *data;
    uint8 *tail;
    uint8 *end;
    uint32 len;
};
typedef struct tpoam_sk_buff_s tpoam_sk_buff_t;

/* MPLS-TP OAM entities defines */
/* MEGID */
struct tpoam_megid_s
{
    char    str[GLB_TPOAM_MEGID_STR_LEN + 1];
};
typedef struct tpoam_megid_s tpoam_megid_t;

/*for required*/
/* MPLS-TP OAM resource count */
struct tpoam_rc_count_s
{
    uint32  section_cnt;
    uint32  megid_cnt;
    uint32  lmep_cnt;
    uint32  rmep_cnt;
    uint32  mip_cnt;
    uint32  csf_cnt;
};
typedef struct tpoam_rc_count_s tpoam_rc_count_t;

struct tpoam_stats_s
{
    tpoam_stats_tx_item_t  tx;
    tpoam_stats_rx_item_t  rx;
    tpoam_stats_err_item_t err;
};
typedef struct tpoam_stats_s tpoam_stats_t;

struct tpoam_lmm_para_s
{
    uint32   repeat_cnt;
    uint32   interval;
};
typedef struct tpoam_lmm_para_s tpoam_lmm_para_t;

/*exec test */
struct tpoam_lmm_s
{
    uint32   exist;
    uint32 service_type;
    char  service_name[TPOAM_SERVICE_STR_SIZE];
    tpoam_lmm_para_t para;
};
typedef struct tpoam_lmm_s tpoam_lmm_t;
typedef struct tpoam_lmm_s tpoam_lmm_exec_t;


enum tpoam_exec_type_e
{
    TPOAM_EXEC_TYPE_NONE,
    TPOAM_EXEC_TYPE_LB_DISCOVERY,
    TPOAM_EXEC_TYPE_LB_MIP,
    TPOAM_EXEC_TYPE_LB_MEP,
    TPOAM_EXEC_TYPE_DM,
    TPOAM_EXEC_TYPE_LMM,
};
typedef enum tpoam_exec_type_e tpoam_exec_type_t;


struct tpoam_dm_para_s
{
    uint8   type;           /* TPOAM_DM_TYPE_1DM, TPOAM_DM_TYPE_DMM */
    uint8   repeat_cnt;
    uint8   interval;
    uint8   exp;
};
typedef struct tpoam_dm_para_s tpoam_dm_para_t;

struct tpoam_1dm_dmm_s
{
    bool   exist;
    uint32 service_type;
    char  service_name[TPOAM_SERVICE_STR_SIZE];
    tpoam_dm_para_t para;
};
typedef struct tpoam_1dm_dmm_s tpoam_1dm_dmm_t;
typedef struct tpoam_1dm_dmm_s tpoam_1dm_dmm_exec_t;


/* LB parameter */
struct tpoam_lbm_para_s
{
    uint8   lb_sub_type;    /* TPOAM_LB_DISCOVERY, TPOAM_LB_MEP, TPOAM_LB_MIP */
    uint8   repeat_cnt;
    uint8   is_req_mep_id;
    uint8   time_out;
    uint8   exp;
    uint8   opt_tlv_type;
    uint16  opt_tlv_len;
    uint16  test_tlv_mode;   /* TPOAM_TEST_MODE_XXX */
    
    /* option parameters */
    uint16  rmep_id;    /* only for TPOAM_LB_MEP */
    uint32  node_id;    /* only for TPOAM_LB_MIP */
    uint8   ttl;        /* for TPOAM_LB_MIP & TPOAM_LB_DISCOVERY */
    uint8   ttl_end;    /* only for TPOAM_LB_DISCOVERY */
};
typedef struct tpoam_lbm_para_s tpoam_lbm_para_t;

/* modified by liwh for bug 16355 to support mpls tp mib, 2011-09-09 */
struct tpoam_loopback_s
{
    bool   exist;
    uint32 service_type;
    char  service_name[TPOAM_SERVICE_STR_SIZE];
    tpoam_lbm_para_t para;
};
typedef struct tpoam_loopback_s tpoam_loopback_t;
typedef struct tpoam_loopback_s tpoam_lb_exec_t;

/* LMEP : Local MEP *//*cli oam-y1731 mepid <1-8191> interval*/
struct tpoam_lmep_s
{
    uint16              mep_id;
    uint8               dm_enable;
    uint8               active;
    uint8               cc_interval;
    uint8               cc_enable;
    uint8               ttl;
    uint8               exp;
    uint32              rmep_id;
    /* modified by yaom for y1731 tp oam adaption for GreatBelt 20130204*/
    uint32              internal_tunnel_id; /* for section oam nexthop */
};
typedef struct tpoam_lmep_s tpoam_lmep_t;

struct tpoam_list_s
{
    struct list*                pst_dual_lm_cache_list;
    struct list*                pst_dmm_cache_list;
};
typedef struct tpoam_list_s tpoam_list_t;

/* LB defines */
struct tpoam_lb_s
{
    uint8                       active;             /* 1: lb session is running, 0: lb session is not running */    
    uint8                       per_lb_status;
    tpoam_lbm_para_t        lbm_para;
    tpoam_lb_discovery_t    discovery;      /* only for TPOAM_LB_DISCOVERY */
    uint8*                      p_lbm_data;         /* packet buffer only for LBM */
    tpoam_sk_buff_t *tx_skb;
    uint16                      payload_len;        /* payload length from MEL (after ACH) */
    uint32                      lbm_next_trans_id;  /* init param : TPOAM_Y1731_PDU_LBM_START_TRANS_ID */
    uint32                       lbm_tx_count;
    uint32                      lbr_rx_count;
    uint32                      lbr_out_order_count;
    struct timeval              lbm_start;
    struct timeval              lbm_stop;
    ctc_task_t*              p_t_lb_timer;
    ctc_task_t*              p_t_lb_exec_timer;
    struct list*                pst_lb_cache_list;
};
typedef struct tpoam_lb_s tpoam_lb_t;

struct tpoam_event_s
{
    uint32          max_count;
    struct list*    pst_event_cache_list;
};
typedef struct tpoam_event_s tpoam_event_t;


enum tpoam_status_e
{
    TPOAM_STATUS_INIT = 0,
    TPOAM_STATUS_FAIL,
    TPOAM_STATUS_OK,
    TPOAM_STATUS_DEGRADE, /* modified by yaom for CRC trigger AIS 20140919 */    
    TPOAM_STATUS_MAX
};
typedef enum tpoam_status_e tpoam_status_t;

struct tpoam_trap_cache_s
{
    uint32          tx_count;
    uint32          peak_count;
    uint32          max_count;
    uint32          timer_count;
    struct list*    pst_cache_list;
    ctc_task_t*  pst_t_pending_timer;
};
typedef struct tpoam_trap_cache_s tpoam_trap_cache_t;

struct tpoam_ratelimit_s
{
    uint32  slice;
    uint32  token;
    tpoam_ratelimit_param_t param;
    tpoam_ratelimit_stats_t stats;
};
typedef struct tpoam_ratelimit_s tpoam_ratelimit_t;

enum tpoam_lm_type
{
  TPOAM_LM_TYPE_NONE = 0,
  TPOAM_LM_TYPE_DUAL = 1,
  TPOAM_LM_TYPE_SINGLE = 2,
};

/* RDI reason bit defines, related to ctc_foam_defect_flag_t */
enum glb_tpoam_rdi_rsn_s
{
    GLB_TPOAM_RDI_RSN_NONE          = 0x00,
    GLB_TPOAM_RDI_RSN_LOC           = 0x01,
    GLB_TPOAM_RDI_RSN_CC_ERR        = 0x02,
    GLB_TPOAM_RDI_RSN_CC_MISMERGE   = 0x04
};
typedef enum glb_tpoam_rdi_rsn_s glb_tpoam_rdi_rsn_t;

/* RDI reason bitmap */
#define TPOAM_RDI_RSN_ALL       ( GLB_TPOAM_RDI_RSN_LOC \
                                    | GLB_TPOAM_RDI_RSN_CC_ERR \
                                    | GLB_TPOAM_RDI_RSN_CC_MISMERGE )
#define TPOAM_RDI_RSN_NONE      ( GLB_TPOAM_RDI_RSN_NONE )
#define TPOAM_RDI_RSN_DEF       ( TPOAM_RDI_RSN_ALL )

/* SF reason bitmap */
#ifdef support_many_event
#define TPOAM_SF_RSN_ALL        ( GLB_TPOAM_EVT_LOC \
                                    | GLB_TPOAM_EVT_RDI_RX \
                                    | GLB_TPOAM_EVT_AIS_RX \
                                    |GLB_TPOAM_EVT_CSF_DEFECT\
                                    | GLB_TPOAM_EVT_AIS_DEFECT )
#else
#define TPOAM_SF_RSN_ALL        ( GLB_TPOAM_EVT_LOC \
                                    | GLB_TPOAM_EVT_RDI_RX)
#endif
#define TPOAM_SF_RSN_NONE       ( GLB_TPOAM_EVT_NONE )
#define TPOAM_SF_RSN_DEF        ( GLB_TPOAM_EVT_LOC )

#ifdef support_many_event
#define TPOAM_SD_RSN_DEF        ( GLB_TPOAM_EVT_AIS_RX \
                                    | GLB_TPOAM_EVT_AIS_DEFECT \
                                    | GLB_TPOAM_EVT_CRC_DEFECT )
#else
#define TPOAM_SD_RSN_DEF        GLB_TPOAM_EVT_CRC_DEFECT 
#endif

/* SD reason bitmap */
#ifdef support_many_event
#define TPOAM_SD_RSN_ALL        ( GLB_TPOAM_EVT_AIS_RX \
                                    | GLB_TPOAM_EVT_AIS_DEFECT \
                                    | GLB_TPOAM_EVT_CRC_DEFECT )
#else
#define TPOAM_SD_RSN_ALL        GLB_TPOAM_EVT_CRC_DEFECT
#endif
#define TPOAM_SD_RSN_NONE       ( GLB_TPOAM_EVT_NONE )

#define GLB_TPOAM_MAX_LM_DUAL_COUNTER_CACHE_NUM      10

struct glb_tpoam_lm_dual_counter_s
{
    uint8   oam_type;
    uint32  key_id;
    uint16  mep_index;
    
    uint32  TxFCf;
    uint32  RxFCb;
    uint32  TxFCb;
    uint32  RxFCl;

    uint8  ccm_cos_value;
    uint16 reserved;
};
typedef struct glb_tpoam_lm_dual_counter_s glb_tpoam_lm_dual_counter_t;

struct hal_tpoam_lm_dual_entry_s
{
    uint32  TxFCf;
    uint32  RxFCb;
    uint32  TxFCb;
    uint32  RxFCl;
    uint8  ccm_cos_value;
    uint32     oam_session_id;
    char    megid[GLB_TPOAM_MEGID_STR_LEN + 1];
};
typedef struct hal_tpoam_lm_dual_entry_s hal_tpoam_lm_dual_entry_t;

struct hal_tpoam_lm_dual_s
{
    uint32  count;
    hal_tpoam_lm_dual_entry_t lm_data[GLB_TPOAM_MAX_LM_DUAL_COUNTER_CACHE_NUM];
};
typedef struct hal_tpoam_lm_dual_s hal_tpoam_lm_dual_t;

#define GLB_TPOAM_MAX_LM_SD_CACHE_NUM      10

struct glb_tpoam_lm_sd_s
{
    uint8   oam_type;
    uint32  key_id;
    uint16  mep_Index;
    
    uint8 reserved[3];
};
typedef struct glb_tpoam_lm_sd_s glb_tpoam_lm_sd_t;

struct glb_tpoam_lm_sd_cache_s
{
    uint8 num;
    glb_tpoam_lm_sd_t entry[GLB_TPOAM_MAX_LM_SD_CACHE_NUM];
};
typedef struct glb_tpoam_lm_sd_cache_s glb_tpoam_lm_sd_cache_t;


/* Support for dual-ended ETH-LM */
struct tpoam_dual_lm
{
    /* Back pointer to MEP */
    uint32 mepid;

    /* interval for read counters and calculate results */
    uint16 stats_interval;

    /* cos type */
    enum tpoam_lm_cos_type  cos_type;
    uint8  cos_value; /* valid when cos_type is specified cos */

    uint16 cache_size;
    //struct list *lm_cache_list; /* dual-lm cache entry */
};
typedef struct tpoam_dual_lm tpoam_dual_lm_t;


/* Support for single-ended ETH-LM */
struct tpoam_single_lm
{
    /* Back pointer to MEP */
    void* pst_oam;    
    uint32   mepid;
    enum tpoam_lm_cos_type  cos_type;
    uint8                    cos_value; /* valid when cos_type is specified cos */
    uint16                   cache_size; /* it should be lmm count */
    struct list *               lm_cache_list; /* single-lm cache entry */
    uint8                       active;     /* 1: LM session is running, 0: LM session is not running */
    uint8                       per_lm_status;
    uint8                       tx_count;
    uint8                       rx_count;
    tpoam_lmm_para_t        para;
    ctc_task_t*              p_t_send_timer;
    /* modified by yaom for proactive single lm 20140416 */
    uint32                      local_loss_threshold;
    uint32                      remote_loss_threshold;
    uint32                      local_loss_alarm;
    uint32                      remote_loss_alarm;
    uint32                      is_proactive;
    uint8                    interval;
};
typedef struct tpoam_single_lm tpoam_single_lm_t;


/* LMM defines : per service */
struct tpoam_lm_s
{
    enum tpoam_lm_type      lm_type;
    tpoam_dual_lm_t   lm_dual;
    tpoam_single_lm_t lm_single;
    /* modified by yaom for proactive single lm 20140416 */
    uint32                      local_loss_threshold;
    uint32                      remote_loss_threshold;
    uint32                    sd_enable;
    uint32               is_event_occur;//for event upload
    uint32               normal_packet_cnt_continue;//for event upload
};
typedef struct tpoam_lm_s tpoam_lm_t;

/* RX1DM defines : per service */
struct tpoam_rx1dm_s
{
    uint32          max_count;
    struct list*    pst_rx1dm_cache_list;
};
typedef struct tpoam_rx1dm_s tpoam_rx1dm_t;

/* DM defines : per service */
struct tpoam_dm_s
{
    //uint8               active;     /* 1: DM session is running, 0: DM session is not running */
    uint8               per_dm_status;
    uint8               tx_count;
    uint8               rx_count;
    tpoam_dm_para_t para;
    //struct list*        pst_dmm_cache_list;
    tpoam_rx1dm_t   rx1dm;
    int64                 delay_threshold;
    uint32               is_event_occur;//for event upload
    uint32               normal_packet_cnt_continue;//for event upload
    uint32               packet_cnt_for_upload;//for timer upload
};
typedef struct tpoam_dm_s tpoam_dm_t;

#define GLB_TPOAM_UPLOAD_DM    0x1
#define GLB_TPOAM_UPLOAD_DM_EVENT    0x2
#define GLB_TPOAM_UPLOAD_LM_EVENT    0x4
#define GLB_TPOAM_UPLOAD_CCM    0x8

struct upload_tpoam_lm_s
{
    uint8 index;
    uint8 bit_flag;//0x1 means event occur, 0x2means event clear
    uint32 lm_far_end_frame_loss; 
    uint32 lm_far_end_frame_loss_ratio; //been expand 1000000
    uint32 lm_near_end_frame_loss;
    uint32 lm_near_end_frame_loss_ratio; //been expand 1000000
};
typedef struct upload_tpoam_lm_s upload_tpoam_lm_t;

struct upload_tpoam_dm_s
{
    uint8 index;//valid packet
    uint32 bit_flag;//0x1 means event occur, 0x2means event clear
    uint32 delay;//average delay
    uint32 variation;//?改方差暂时不关注,名字改一下
};
typedef struct upload_tpoam_dm_s upload_tpoam_dm_t;

struct upload_tpoam_ccm_s
{
    tpoam_status_t ccm_status;
};
typedef struct upload_tpoam_ccm_s upload_tpoam_ccm_t;

#define TPOAM_SESSION_DM_DELAY_THRESHOLD_MAX 4000000000UL
#define TPOAM_SESSION_DM_DELAY_THRESHOLD_DEFAULT 0
#define TPOAM_SESSION_DM_INTERVAL_DEFAULT TPOAM_DM_INTERVAL_10S
#define TPOAM_SESSION_LM_LOCAL_LOSS_THRESHOLD_DEFAULT 0
#define TPOAM_SESSION_LM_REMOTE_LOSS_THRESHOLD_DEFAULT 0

#define GLB_TPOAM_DM_DELAY_THRESHOLD_DEFAULT 0
#define GLB_TPOAM_DM_CALCULATE_INTERVAL_DEFAULT 10

#define GLB_TPOAM_LM_LOCAL_THRESHOLD_DEFAULT 0
#define GLB_TPOAM_LM_REMOTE_THRESHOLD_DEFAULT 0
#define GLB_TPOAM_LM_CALCULATE_INTERVAL_DEFAULT 20

enum tpoam_evt_dest_e
{
    TPOAM_EVT_DEST_LOG,
    TPOAM_EVT_DEST_TRAP,
    TPOAM_EVT_DEST_CACHE,    
    TPOAM_EVT_DEST_MAX
};
typedef enum tpoam_evt_dest_e tpoam_evt_dest_t;

/*current on doing test*/
struct tpoam_exec_s
{
    tpoam_exec_type_t   exec_type;
    //tbl_oam_session_t*            pst_oam;
    uint32 session_id;
};
typedef struct tpoam_exec_s tpoam_exec_t;


struct hal_tpoam_event_entry_s
{
    uint32  event_bmp;
    uint32  oam_session_id;
    char    megid[GLB_TPOAM_MEGID_STR_LEN + 1];
};
typedef struct hal_tpoam_event_entry_s hal_tpoam_event_entry_t;

struct hal_tpoam_event_s
{
    uint32  count;
    hal_tpoam_event_entry_t events[GLB_TPOAM_EVENT_CACHE_SIZE];
};
typedef struct hal_tpoam_event_s hal_tpoam_event_t;


#define GLB_MPLS_ONE_LEVEL_APS             1

struct glb_tpoam_megid_s
{
    char    id[GLB_TPOAM_MEGID_LEN];
    uint32  count;
};
typedef struct glb_tpoam_megid_s glb_tpoam_megid_t;

#endif
