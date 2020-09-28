/**
 @file knet_kernel.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-4-9

 @version v2.0

*/
#ifndef _KNET_KERNEL_H_
#define _KNET_KERNEL_H_
#ifdef __cplusplus
extern "C" {
#endif

#define KNET_MAX_RX_CHAN_NUM     4
#define KNET_MAX_TX_CHAN_NUM     2

#define KNET_DEV_MAJOR     201

#define KNET_NAME          "linux_knet"  /* "linux_knet" */

#define KNET_DEV_NAME      "/dev/" KNET_NAME

#define KNET_PKT_MTU    9600

struct DsDesc_le_s
{
    unsigned int u1_pkt_sop:1;
    unsigned int u1_pkt_eop:1;
    unsigned int u1_pkt_pktError:1;
    unsigned int u1_pkt_truncPkt:1;
    unsigned int u1_pkt_truncEn:1;
    unsigned int atomic:2;
    unsigned int rsv0:1;
    
    unsigned int dataStruct:6;
    unsigned int rsv1:2;
    
    unsigned int intrMask:1;
    unsigned int pause:1;
    unsigned int rsv2:2;
    unsigned int invalid0:4;

    unsigned int u2_info_timeout:1;
    unsigned int descError:1;
    unsigned int dataError:1;
    unsigned int rsv3:1;
    unsigned int error:1;
    unsigned int rsv4:2;
    unsigned int done:1;
    
    unsigned int cfgSize:16;
    unsigned int realSize:16;
    
    unsigned int rsv5:4;
    unsigned int memAddr:28;
    
    unsigned int chipAddr;
    unsigned int timestamp_l;
    
    unsigned int timestamp_h:30;
    unsigned int rsv6:2;
};
typedef struct DsDesc_le_s DsDesc_le_t;

struct DsDesc_be_s
{
    unsigned int u2_info_timeout:1;
    unsigned int descError:1;
    unsigned int dataError:1;
    unsigned int rsv3:1;
    unsigned int error:1;
    unsigned int rsv4:2;
    unsigned int done:1;

    unsigned int intrMask:1;
    unsigned int pause:1;
    unsigned int rsv2:2;
    unsigned int invalid0:4;

    unsigned int dataStruct:6;
    unsigned int rsv1:2;
    
    unsigned int u1_pkt_sop:1;
    unsigned int u1_pkt_eop:1;
    unsigned int u1_pkt_pktError:1;
    unsigned int u1_pkt_truncPkt:1;
    unsigned int u1_pkt_truncEn:1;
    unsigned int atomic:2;
    unsigned int rsv0:1;

    unsigned short realSize;
    unsigned short cfgSize;
    
    unsigned int memAddr:28;
    unsigned int rsv5:4;
    
    unsigned int chipAddr;
    unsigned int timestamp_l;
    
    unsigned int rsv6:2;
    unsigned int timestamp_h:30;
};
#ifdef DUET2
typedef struct DsDesc_be_s DsDesc_t;
#else
typedef struct DsDesc_le_s DsDesc_t;
#endif
struct knet_dma_desc_s
{
    DsDesc_t desc_info;
    unsigned int desc_padding0;  /* Notice: padding fields cannot used for any purpose*/
    unsigned int desc_padding1;  /* Notice: padding fields cannot used for any purpose*/
};
typedef struct knet_dma_desc_s knet_dma_desc_t;

#ifdef __cplusplus
}
#endif

#endif

