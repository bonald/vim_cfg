#ifndef INCLUDE_BST_JSON_H
#define INCLUDE_BST_JSON_H

#ifdef __cplusplus
extern "C"
{
#endif



/* threshold types */
typedef enum _bst_threshold_realm_ {
  INSIGHT_BST_DEVICE_THRESHOLD = 1,
  INSIGHT_BST_EGRESS_PORT_SP_THRESHOLD,
  INSIGHT_BST_EGRESS_SP_THRESHOLD,
  INSIGHT_BST_EGRESS_UC_QUEUE_THRESHOLD,
  INSIGHT_BST_EGRESS_UC_QUEUEGROUPS_THRESHOLD,
  INSIGHT_BST_EGRESS_MC_QUEUE_THRESHOLD,
  INSIGHT_BST_EGRESS_CPU_QUEUE_THRESHOLD,
  INSIGHT_BST_EGRESS_RQE_QUEUE_THRESHOLD,
  INSIGHT_BST_INGRESS_PORT_PG_THRESHOLD,
  INSIGHT_BST_INGRESS_PORT_SP_THRESHOLD,
  INSIGHT_BST_INGRESS_SP_THRESHOLD
}INSIGHT_BST_THRESHOLD_TYPE_;


typedef enum _bst_config_param_mask_pos_
{
  BST_CONFIG_PARAMS_ENABLE = 0,
  BST_CONFIG_PARAMS_SND_ASYNC_REP,
  BST_CONFIG_PARAMS_COLL_INTRVL,
  BST_CONFIG_PARAMS_STATS_UNITS,
  BST_CONFIG_PARAMS_TGR_RATE_LIMIT,
  BST_CONFIG_PARAMS_SND_SNAP_TGR,
  BST_CONFIG_PARAMS_TGR_RL_INTVL,
  BST_CONFIG_PARAMS_ASYNC_FULL_REP,
  BST_CONFIG_PARAMS_STATS_IN_PERCENT
}BST_CONFIG_PARAM_MASK_t;


/* Structure to pass API parameters to the BST APP */
typedef struct _bstjson_configure_bst_feature_
{
    int bstEnable;
    int sendAsyncReports;
    int collectionInterval;
    int statsInPercentage;
    int statUnitsInCells;
    int bstMaxTriggers;
    int sendSnapshotOnTrigger;
    int triggerTransmitInterval;
    int sendIncrementalReport;
    int configMask;
} BSTJSON_CONFIGURE_BST_FEATURE_t;

/* Structure to pass API parameters to the BST APP */
typedef struct _bstjson_configure_bst_tracking_
{
    int trackPeakStats;
    int trackIngressPortPriorityGroup;
    int trackIngressPortServicePool;
    int trackIngressServicePool;
    int trackEgressPortServicePool;
    int trackEgressServicePool;
    int trackEgressUcQueue;
    int trackEgressUcQueueGroup;
    int trackEgressMcQueue;
    int trackEgressCpuQueue;
    int trackEgressRqeQueue;
    int trackDevice;
} BSTJSON_CONFIGURE_BST_TRACKING_t;


typedef enum _threshold_fields_
{
  DEVICE_COUNTER = 0,
  UMSHARE_COUNTER,
  UMHEADROOM_COUNTER,
  UCSHARE_COUNTER,
  MCSHARE_COUNTER,
  MCSHARE_QUEUE_COUNTER,
  UC_COUNTER,
  MCQUEUE_COUNTER,
  MC_COUNTER,
  CPU_COUNTER,
  RQE_COUNTER
}INSIGHT_BST_COUNTER_t;

#define INSIGHT_BST_REALM_MAX RQE_COUNTER

#define INSIGHT_BST_PORT_POS (INSIGHT_BST_REALM_MAX+1)
#define INSIGHT_BST_SP_POS (INSIGHT_BST_REALM_MAX+2)
#define INSIGHT_BST_PG_POS (INSIGHT_BST_REALM_MAX+3)
#define INSIGHT_BST_QUEUE_POS (INSIGHT_BST_REALM_MAX+4)
#define INSIGHT_BST_QUEUE_GRP_POS (INSIGHT_BST_REALM_MAX+5)


#define INSIGHT_BST_DEVICE_MASK  (1<<DEVICE_COUNTER)
#define INSIGHT_BST_UMSHARE_MASK  (1<<UMSHARE_COUNTER)
#define INSIGHT_BST_UMHEADROOM_MASK  (1<<UMHEADROOM_COUNTER)
#define INSIGHT_BST_UCSHARE_MASK  (1<<UCSHARE_COUNTER)
#define INSIGHT_BST_MCSHARE_MASK  (1<<MCSHARE_COUNTER)
#define INSIGHT_BST_MCSHARE_QUEUE_MASK  (1<<MCSHARE_QUEUE_COUNTER)
#define INSIGHT_BST_UC_MASK  (1<<UC_COUNTER)
#define INSIGHT_BST_MCQUEUE_MASK  (1<<MCQUEUE_COUNTER)
#define INSIGHT_BST_MC_MASK  (1<<MC_COUNTER)
#define INSIGHT_BST_CPU_MASK  (1<<CPU_COUNTER)
#define INSIGHT_BST_RQE_MASK  (1<<RQE_COUNTER)
#define INSIGHT_BST_PORT_MASK  (1<<INSIGHT_BST_PORT_POS)
#define INSIGHT_BST_SP_MASK  (1<<INSIGHT_BST_SP_POS)
#define INSIGHT_BST_PG_MASK  (1<<INSIGHT_BST_PG_POS)
#define INSIGHT_BST_QUEUE_MASK  (1<<INSIGHT_BST_QUEUE_POS)
#define INSIGHT_BST_QUEUE_GRP_MASK  (1<<INSIGHT_BST_QUEUE_GRP_POS)
typedef struct _bstjson_configure_bst_thresholds_
{
    char realm[INSIGHT_JSON_MAX_NODE_LENGTH];
    int port;
    int priorityGroup;
    int servicePool;
    int queue;
    int queueGroup;
    int threshold;
    int umShareThreshold;
    int umHeadroomThreshold;
    int ucShareThreshold;
    int mcShareThreshold;
    int mcShareQueueEntriesThreshold;
    int ucThreshold;
    int mcThreshold;
    int mcQueueEntriesThreshold;
    int cpuThreshold;
    int rqeThreshold;
    unsigned int mask;
} BSTJSON_CONFIGURE_BST_THRESHOLDS_t;


/* Structure to pass API parameters to the BST APP */
typedef struct _bstjson_get_bst_thresholds_
{
    int includeIngressPortPriorityGroup;
    int includeIngressPortServicePool;
    int includeIngressServicePool;
    int includeEgressPortServicePool;
    int includeEgressServicePool;
    int includeEgressUcQueue;
    int includeEgressUcQueueGroup;
    int includeEgressMcQueue;
    int includeEgressCpuQueue;
    int includeEgressRqeQueue;
    int includeDevice;
} BSTJSON_GET_BST_THRESHOLDS_t;



typedef struct _bstjson_get_bst_thresholds_   BSTJSON_GET_BST_REPORT_t;

#define INSIGHT_BST_CGSN_COLLECTION_INTERVAL (1 << 0)
#define INSIGHT_BST_CGSN_PORT_LIST (1 << 1)
#define INSIGHT_BST_CGSN_COUNT (1 << 2)
#define INSIGHT_BST_CGSN_QUEUE_TYPE (1 << 3)
#define INSIGHT_BST_CGSN_QUEUE_LIST (1 << 4)
#define INSIGHT_BST_CGSN_REQ_TYPE (1 << 5)

#define INSIGHT_BST_TOP_PRT_DRP_VALID (INSIGHT_BST_CGSN_COUNT | INSIGHT_BST_CGSN_REQ_TYPE)
#define INSIGHT_BST_TOP_PRT_Q_DRP_VALID (INSIGHT_BST_CGSN_COUNT | INSIGHT_BST_CGSN_REQ_TYPE | INSIGHT_BST_CGSN_QUEUE_TYPE)
#define INSIGHT_BST_PRT_DRP_VALID (INSIGHT_BST_CGSN_PORT_LIST | INSIGHT_BST_CGSN_REQ_TYPE)
#define INSIGHT_BST_PRT_Q_DRP_VALID (INSIGHT_BST_CGSN_PORT_LIST | INSIGHT_BST_CGSN_REQ_TYPE | INSIGHT_BST_CGSN_QUEUE_TYPE | INSIGHT_BST_CGSN_QUEUE_LIST)


typedef enum _bst_cgsn_req_type_
{
    INSIGHT_BST_CGSN_TOP_DROPS = 1,
    INSIGHT_BST_CGSN_TOP_PRT_Q_DROPS,
    INSIGHT_BST_CGSN_PRT_DROPS,
    INSIGHT_BST_CGSN_PRT_Q_DROPS
} INSIGHT_BST_CGSN_REQ_TYPE_t;


typedef enum _bst_cgsn_ctr_type_
{
    INSIGHT_BST_CGSN_UCAST = 1,
    INSIGHT_BST_CGSN_MCAST,
    INSIGHT_BST_CGSN_ALL,
    INSIGHT_BST_CGSN_TOTAL,
} INSIGHT_BST_CGSN_CTR_TYPE_t;

typedef struct _bstjson_get_bst_cgsn_drp_ctrs_
{
  unsigned int count;
  uint8 req_type;
  bool all_prts;
  INSIGHT_PORT_MASK_t port_list;
  uint8  queue_type;    /*0-ucast;1-mcast;2-all*/
  INSIGHT_QUEUE_MASK_t queue_list;
  unsigned int intrvl;
} BSTJSON_GET_BST_CGSN_DROP_CTRS_t;

/* Feature Configuration */
typedef struct _bst_config_
{
    /* enable the feature */
    bool enableStatsMonitoring;

    /* For enabling selectively. When all are disabled, feature is disabled. */
    bool enableDeviceStatsMonitoring;
    bool enableIngressStatsMonitoring;
    bool enableEgressStatsMonitoring;

    /*Periodic collection*/
    bool enablePeriodicCollection;
    int  collectionPeriod;
} BVIEW_BST_CONFIG_t;


/** Different scalability numbers for the ASIC */
typedef struct _capabilities_
{
        /** Number of ports of Asic */
        int numPorts;
        /** Number of unicast queues */
        int numUnicastQueues;
        /** Number of unicast queue groups */
        int numUnicastQueueGroups;
        /** Number of multicast queues */
        int numMulticastQueues;
        /** Number of service pools */
        int numServicePools;
        /** Number of common pools */
        int numCommonPools;
        /** Number of CPU queues */
        int numCpuQueues;
        /** Number of RQE queues */
        int numRqeQueues;
        /** Number of RQE queue pools */
        int numRqeQueuePools;
        /** Number of priority groups */
        int numPriorityGroups;
        /** can this ASIC provide accurate time ? */
        bool support1588;
        /** MMU Cell to BYTE conversion*/
        int cellToByteConv;

} INSIGHT_ASIC_CAPABILITIES_t;
/* Buffer Count for the device */
typedef struct _bst_device_
{
    uint64 bufferCount;
} INSIGHT_BST_DEVICE_DATA_t;

#define INSIGHT_ASIC_MAX_UC_QUEUES        512 //64*8 = 512 //Modify 2048
#define INSIGHT_ASIC_MAX_UC_QUEUE_GROUPS  128
#define INSIGHT_ASIC_MAX_MC_QUEUES        1040
#define INSIGHT_ASIC_MAX_SERVICE_POOLS    4
#define INSIGHT_ASIC_MAX_COMMON_POOLS     1
#define INSIGHT_ASIC_MAX_CPU_QUEUES       64
#define INSIGHT_ASIC_MAX_RQE_QUEUES       11
#define INSIGHT_ASIC_MAX_RQE_QUEUE_POOLS  4
#define INSIGHT_ASIC_MAX_PRIORITY_GROUPS  8
#define INSIGHT_ASIC_MAX_INGRESS_SERVICE_POOLS  \
    (INSIGHT_ASIC_MAX_SERVICE_POOLS + INSIGHT_ASIC_MAX_COMMON_POOLS)

/* Buffer Count for Ingress Port + Priority Groups */
typedef struct _bst_i_p_pg_
{

    struct _ippg_data_
    {
        uint64 umShareBufferCount;
        uint64 umHeadroomBufferCount;
    } data[INSIGHT_ASIC_MAX_PORTS][INSIGHT_ASIC_MAX_PRIORITY_GROUPS];

} INSIGHT_BST_INGRESS_PORT_PG_DATA_t;

/* Buffer Count for Ingress Port + Service Pools */
typedef struct _bst_i_p_sp_
{

    struct _ipsp_data_
    {
        uint64 umShareBufferCount;
    } data[INSIGHT_ASIC_MAX_PORTS][INSIGHT_ASIC_MAX_INGRESS_SERVICE_POOLS];

} INSIGHT_BST_INGRESS_PORT_SP_DATA_t;

/* Buffer Count for Ingress Service Pools */
typedef struct _bst_i_sp_
{

    struct _isp_data_
    {
        uint64 umShareBufferCount;
    } data[INSIGHT_ASIC_MAX_INGRESS_SERVICE_POOLS];

} INSIGHT_BST_INGRESS_SP_DATA_t;

/* Buffer Count for Egress Port + Service Pools */
typedef struct _bst_e_p_sp_
{
    struct _epsp_data_
    {
        uint64 ucShareBufferCount;
        uint64 umShareBufferCount;
        uint64 mcShareBufferCount;
        uint64 mcShareQueueEntries;
    } data[INSIGHT_ASIC_MAX_PORTS][INSIGHT_ASIC_MAX_SERVICE_POOLS];

} INSIGHT_BST_EGRESS_PORT_SP_DATA_t;

/* Buffer Count for Egress Service Pools */
typedef struct _bst_e_sp_
{

    struct _esp_data_
    {
        uint64 umShareBufferCount;
        uint64 mcShareBufferCount;
        uint64 mcShareQueueEntries;
    } data[INSIGHT_ASIC_MAX_SERVICE_POOLS];

} INSIGHT_BST_EGRESS_SP_DATA_t;

/* Buffer Count for Egress Unicast Queues */
typedef struct _bst_e_ucq_
{

    struct _eucq_data_
    {
        uint64 ucBufferCount;
        uint64 port; /* to indicate the port number using this queue */
    } data[INSIGHT_ASIC_MAX_UC_QUEUES];

} INSIGHT_BST_EGRESS_UC_QUEUE_DATA_t;

/* Buffer Count for Egress Unicast Queue Groups */
typedef struct _bst_e_ucqg_
{
    struct _eucqg_data_
    {
        uint64 ucBufferCount;
    } data[INSIGHT_ASIC_MAX_UC_QUEUE_GROUPS];

} INSIGHT_BST_EGRESS_UC_QUEUEGROUPS_DATA_t;

/* Buffer Count for Egress Multicast Queues */
typedef struct _bst_e_mcq_
{

    struct _emcq_data_
    {
        uint64 mcBufferCount;
        uint64 mcQueueEntries;
        uint64 port; /* to indicate the port number using this queue */
    } data[INSIGHT_ASIC_MAX_MC_QUEUES];

} INSIGHT_BST_EGRESS_MC_QUEUE_DATA_t;

/* Buffer Count for CPU Queues */
typedef struct _bst_cpu_q_
{

    struct _cpuq_data_
    {
        uint64 cpuBufferCount;
        uint64 cpuQueueEntries;
    } data[INSIGHT_ASIC_MAX_CPU_QUEUES];

} INSIGHT_BST_EGRESS_CPU_QUEUE_DATA_t;

/* Buffer Count for RQE Queues */
typedef struct _bst_rqe_q_
{

    struct _rqeq_data_
    {
        uint64 rqeBufferCount;
        uint64 rqeQueueEntries;
    } data[INSIGHT_ASIC_MAX_RQE_QUEUES];

} INSIGHT_BST_EGRESS_RQE_QUEUE_DATA_t;

/* A Complete Data set for a 'snapshot' */

typedef struct _bst_asic_data_snapshot_
{
    /* Device Section */
    INSIGHT_BST_DEVICE_DATA_t device; /*total*/

    /* Ingress Section */
    INSIGHT_BST_INGRESS_PORT_PG_DATA_t iPortPg; /*port + tc*/
    INSIGHT_BST_INGRESS_PORT_SP_DATA_t iPortSp; /*port + sc*/
    INSIGHT_BST_INGRESS_SP_DATA_t iSp; /*sc*/

    /* Egress Section */
    INSIGHT_BST_EGRESS_PORT_SP_DATA_t ePortSp; /*port + sc*/
    INSIGHT_BST_EGRESS_SP_DATA_t eSp;          /*sc*/
    INSIGHT_BST_EGRESS_UC_QUEUE_DATA_t eUcQ;    /*uc queue*/
    INSIGHT_BST_EGRESS_UC_QUEUEGROUPS_DATA_t eUcQg; /*group*/
    INSIGHT_BST_EGRESS_MC_QUEUE_DATA_t eMcQ;   /*mc queue*/
    INSIGHT_BST_EGRESS_CPU_QUEUE_DATA_t cpqQ;  /*cpu queue*/
    INSIGHT_BST_EGRESS_RQE_QUEUE_DATA_t rqeQ;  /*c2c queue*/

} INSIGHT_BST_ASIC_SNAPSHOT_DATA_t;


#define INSIGHT_MAX_STRING_NAME_LEN 256
/* Trigger info */
typedef struct  _bst_trigger_info_
{
  char realm[INSIGHT_MAX_STRING_NAME_LEN];
  char counter[INSIGHT_MAX_STRING_NAME_LEN];
  int port;
  int queue;
} INSIGHT_BST_TRIGGER_INFO_t;


/* Max buffers allocated for Egress Port + Service Pools */
typedef struct _system_ep_sp_max_buf_
{
    uint64 ucShareMaxBuf;
    uint64 umShareMaxBuf;
    uint64 mcShareMaxBuf;
} INSIGHT_SYSTEM_EGRESS_PORT_SP_MAX_BUF_t;

/* Max buffers allocated for the device level buffers*/
typedef struct _system_device_max_buf_
{
    uint64 maxBuf;
} INSIGHT_SYSTEM_DEVICE_MAX_BUF_t;

/* Max buffers allocated for Ingress Port + Priority Groups */
typedef struct _system_i_p_pg_max_buf_
{
    uint64 umShareMaxBuf;
    uint64 umHeadroomMaxBuf;
} INSIGHT_SYSTEM_INGRESS_PORT_PG_MAX_BUF_t;

/* Max buffers allocated for Ingress Port + Service Pools */
typedef struct _system_i_p_sp_max_buf_
{
    uint64 umShareMaxBuf;
} INSIGHT_SYSTEM_INGRESS_PORT_SP_MAX_BUF_t;

/* Max buffers allocated for Ingress Service Pools */
typedef struct _system_i_sp_max_buf_
{
    uint64 umShareMaxBuf;
} INSIGHT_SYSTEM_INGRESS_SP_MAX_BUF_t;

/* Max buffers allocated for Egress Service Pools */
typedef struct _system_e_sp_max_buf_
{
    uint64 umShareMaxBuf;
    uint64 mcShareMaxBuf;
} INSIGHT_SYSTEM_EGRESS_SP_MAX_BUF_t;

/* Max buffers allocated for Egress Unicast Queues */
typedef struct _system_e_ucq_max_buf_
{
    uint64 ucMaxBuf;
} INSIGHT_SYSTEM_EGRESS_UC_QUEUE_MAX_BUF_t;

/* Max buffers allocated for Egress Unicast Queue Groups */
typedef struct _system_e_ucqg_max_buf_
{
    uint64 ucMaxBuf;
} INSIGHT_SYSTEM_EGRESS_UC_QUEUEGROUPS_MAX_BUF_t;

/* Max buffers allocated for Egress Multicast Queues */
typedef struct _system_e_mcq_max_buf_
{
    uint64 mcMaxBuf;
} INSIGHT_SYSTEM_EGRESS_MC_QUEUE_MAX_BUF_t;

/* Max buffers allocated for CPU Queues */
typedef struct _system_cpu_q_max_buf_
{
    uint64 cpuMaxBuf;
} INSIGHT_SYSTEM_EGRESS_CPU_QUEUE_MAX_BUF_t;

/* Max buffers allocated for RQE Queues */
typedef struct _system_rqe_q_max_buf_
{
    uint64 rqeMaxBuf;
} INSIGHT_SYSTEM_EGRESS_RQE_QUEUE_MAX_BUF_t;


typedef struct _system_device_max_buf_data_
{

  INSIGHT_SYSTEM_DEVICE_MAX_BUF_t data;

} INSIGHT_SYSTEM_DEVICE_MAX_BUF_DATA_t;

/* Max Buffers for Ingress Port + Priority Groups */
typedef struct _system_i_p_pg_max_buf_data_
{

  INSIGHT_SYSTEM_INGRESS_PORT_PG_MAX_BUF_t data[INSIGHT_ASIC_MAX_PORTS][INSIGHT_ASIC_MAX_PRIORITY_GROUPS];

} INSIGHT_SYSTEM_INGRESS_PORT_PG_MAX_BUF_DATA_t;

/* Max Buffers for Ingress Port + Service Pools */
typedef struct _system_i_p_sp_max_buf_data_
{

  INSIGHT_SYSTEM_INGRESS_PORT_SP_MAX_BUF_t data[INSIGHT_ASIC_MAX_PORTS][INSIGHT_ASIC_MAX_INGRESS_SERVICE_POOLS];

} INSIGHT_SYSTEM_INGRESS_PORT_SP_MAX_BUF_DATA_t;

/* Max Buffers for Ingress Service Pools */
typedef struct _system_i_sp_max_buf_data_
{

  INSIGHT_SYSTEM_INGRESS_SP_MAX_BUF_t data[INSIGHT_ASIC_MAX_INGRESS_SERVICE_POOLS];

} INSIGHT_SYSTEM_INGRESS_SP_MAX_BUF_DATA_t;

/* Max Buffers for Egress Port + Service Pools */
typedef struct _system_e_p_sp_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_PORT_SP_MAX_BUF_t data[INSIGHT_ASIC_MAX_PORTS][INSIGHT_ASIC_MAX_SERVICE_POOLS];

} INSIGHT_SYSTEM_EGRESS_PORT_SP_MAX_BUF_DATA_t;

/* Max Buffers for Egress Service Pools */
typedef struct _system_e_sp_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_SP_MAX_BUF_t data[INSIGHT_ASIC_MAX_SERVICE_POOLS];

} INSIGHT_SYSTEM_EGRESS_SP_MAX_BUF_DATA_t;

/* Max Buffers for Egress Unicast Queues */
typedef struct _system_e_ucq_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_UC_QUEUE_MAX_BUF_t data[INSIGHT_ASIC_MAX_UC_QUEUES];

} INSIGHT_SYSTEM_EGRESS_UC_QUEUE_MAX_BUF_DATA_t;

/* Max Buffers for Egress Unicast Queue Groups */
typedef struct _system_e_ucqg_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_UC_QUEUEGROUPS_MAX_BUF_t data[INSIGHT_ASIC_MAX_UC_QUEUE_GROUPS];

} INSIGHT_SYSTEM_EGRESS_UC_QUEUEGROUPS_MAX_BUF_DATA_t;

/* Max Buffers for Egress Multicast Queues */
typedef struct _system_e_mcq_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_MC_QUEUE_MAX_BUF_t data[INSIGHT_ASIC_MAX_MC_QUEUES];

} INSIGHT_SYSTEM_EGRESS_MC_QUEUE_MAX_BUF_DATA_t;

/* Max Buffers for CPU Queues */
typedef struct _system_cpu_q_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_CPU_QUEUE_MAX_BUF_t data[INSIGHT_ASIC_MAX_CPU_QUEUES];

} INSIGHT_SYSTEM_EGRESS_CPU_QUEUE_MAX_BUF_DATA_t;

/* Max Buffers for RQE Queues */
typedef struct _system_rqe_q_max_buf_data_
{

  INSIGHT_SYSTEM_EGRESS_RQE_QUEUE_MAX_BUF_t data[INSIGHT_ASIC_MAX_RQE_QUEUES];

} INSIGHT_SYSTEM_EGRESS_RQE_QUEUE_MAX_BUF_DATA_t;

/* A Complete Data set for a 'snapshot' */

typedef struct _system_asic_max_buf_snapshot_data_
{
    /* Device Section */
    INSIGHT_SYSTEM_DEVICE_MAX_BUF_DATA_t device;

    /* Ingress Section */
    INSIGHT_SYSTEM_INGRESS_PORT_PG_MAX_BUF_DATA_t iPortPg;
    INSIGHT_SYSTEM_INGRESS_PORT_SP_MAX_BUF_DATA_t iPortSp;
    INSIGHT_SYSTEM_INGRESS_SP_MAX_BUF_DATA_t iSp;

    /* Egress Section */
    INSIGHT_SYSTEM_EGRESS_PORT_SP_MAX_BUF_DATA_t ePortSp;
    INSIGHT_SYSTEM_EGRESS_SP_MAX_BUF_DATA_t eSp;
    INSIGHT_SYSTEM_EGRESS_UC_QUEUE_MAX_BUF_DATA_t eUcQ;
    INSIGHT_SYSTEM_EGRESS_UC_QUEUEGROUPS_MAX_BUF_DATA_t eUcQg;
    INSIGHT_SYSTEM_EGRESS_MC_QUEUE_MAX_BUF_DATA_t eMcQ;
    INSIGHT_SYSTEM_EGRESS_CPU_QUEUE_MAX_BUF_DATA_t cpqQ;
    INSIGHT_SYSTEM_EGRESS_RQE_QUEUE_MAX_BUF_DATA_t rqeQ;

} INSIGHT_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t;

typedef struct _bst_reporting_options_
{
    bool includeIngressPortPriorityGroup;
    bool includeIngressPortServicePool;
    bool includeIngressServicePool;
    bool includeEgressPortServicePool;
    bool includeEgressServicePool;
    bool includeEgressUcQueue;
    bool includeEgressUcQueueGroup;
    bool includeEgressMcQueue;
    bool includeEgressCpuQueue;
    bool includeEgressRqeQueue;
    bool includeDevice;
    bool statUnitsInCells;
    bool reportTrigger;
    bool reportThreshold;
    bool sendSnapShotOnTrigger;
    INSIGHT_BST_TRIGGER_INFO_t triggerInfo;
    bool sendIncrementalReport;
    bool statsInPercentage;
    INSIGHT_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t *bst_max_buffers_ptr;
} BSTJSON_REPORT_OPTIONS_t;


 /* Decode Json */
extern INSIGHT_STATUS
insight_bst_json_configure_bst_tracking (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_configure_bst_feature (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_configure_bst_thresholds (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_get_bst_report (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_get_bst_feature (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_get_bst_tracking (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_get_bst_thresholds (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_clear_bst_thresholds (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_clear_bst_statistics (void *cookie, char *jsonBuffer, int bufLength);
extern INSIGHT_STATUS
insight_bst_json_get_bst_congestion_drop_counters (void *cookie, char *jsonBuffer, int bufLength);


 /* Encode Json */
extern INSIGHT_STATUS
bstjson_encode_get_bst_feature( int asicId,
                                            int method,
                                            const BSTJSON_CONFIGURE_BST_FEATURE_t *pData,
                                            uint8 **pJsonBuffer
                                            );

extern INSIGHT_STATUS
bstjson_encode_get_bst_tracking( int asicId,
                                             int method,
                                             const BSTJSON_CONFIGURE_BST_TRACKING_t *pData,
                                             uint8 **pJsonBuffer
                                             );
extern INSIGHT_STATUS
bstjson_encode_get_bst_report(int asicId,
                                           int method,
                                           const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                           const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                           const BSTJSON_REPORT_OPTIONS_t *options,
                                           const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                           const sal_time_t *reportTime,
                                           uint8 **pJsonBuffer
                                           );
extern INSIGHT_STATUS
bstjson_encode_get_cgsn_drop_ctrs ( int asicId,
                                            const void *in_ptr,
                                            const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                            uint8 **pJsonBuffer
                                            );

#endif

