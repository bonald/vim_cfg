/******************************************************************************/
/* Filename: cts_xr_config.h                                                  */
/* File description:                                                              */
/*                                                                            */
/* belong to module:                                                              */
/*                                                */
/* author: flin                                                               */
/* Date: 2006-08-09 21:08                                                     */
/*                                                                            */
/* Modify log                            */
/* Modifyer:                                                                    */
/* Date:                                                                      */
/* Description:                                                                      */
/*                                                                            */
/* Modifyer:                                                                    */
/* Date:                                                                      */
/* Description:                                                                      */
/*                                                                            */
/* Version:                                                                      */
/******************************************************************************/
#ifndef _CTS_XR_CONFIG_H
#define _CTS_XR_CONFIG_H
/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/* #include*/

/* #define*/

#define CTS_DEBUG_ENABLE

#if 0 /* flin, 2006-08-11 8:28 */
#if (HOST_IS_LE == 1)
#define CTC_HOST_IS_BIG_ENDIAN          0
#define CTC_HOST_IS_LITTLE_ENDIAN       1
#else
#define CTC_HOST_IS_BIG_ENDIAN          1
#define CTC_HOST_IS_LITTLE_ENDIAN       0
#endif
#endif /* #if 0 */
/* const*/
#define CTS_XR_PACKAGE_NAME     "pktxr"
#define CTS_XR_PACKAGE_VERSION  "1.1"

#define CTS_XR_PORT_MAX_NUM    512
#define CTS_XR_PKT_MTU     (9 * 1024 + 64) /* 64 bytes for forwarding headers */
#define CTS_XR_LENGTH_OF_STRUCT_STREAM_PKT (CTS_XR_PKT_MTU)
#define CTS_XR_MAX_PAYLOAD_LEN (9 * 1024)
#define CTS_XR_SOCKET_MAX_ERR_CNT     5
#define CTS_XR_MAX_ERR_STR_LEN      100
#define CTS_XR_MAX_RS_STR_LEN       100
#define CTS_XR_MAX_IF_NAME_LEN      16  /* same as IF_NAMESIZE */
#define CTS_XR_MAX_FILTER_STRING    256
#define CTS_XR_RCV_SOCKET_TIME_OUT  5   /* second */
#define CTS_XR_MAX_FILE_NAME_LEN    50
#define CTS_XR_MAX_PATH_LEN         256
#define CTS_XR_MAX_FILE_PATH_LEN    (CTS_XR_MAX_PATH_LEN + CTS_XR_MAX_FILE_NAME_LEN)
#define CTS_XR_CAPTURE_BASE_FILE_NAME    "./pkt_capture"
#define CTS_XR_CAPTURE_FILE_NAME_SUFFIX    ".cap"
#define CTS_XR_MAX_FILE_SIZE        (512 * 1024 *1024)
#define CTS_XR_PCAP_DISPATCH_SLEEP  10 * 1000 /* us */
#define CTS_XR_STREAM_SEND_SLEEP    100 * 1000 /* us */
/* return val */
#define CTS_RS_SETED        100

/* packet capture macro */
#define CTS_XR_SNAPLEN              65535
#define CTS_XR_PROMISCUOUS_MODE     1
#define CTS_XR_BFP_OPTIMIZE         0
#define CTS_XR_FILTER_NET           0
/* structure declare */

/* global veriable declare */

/* function declare */

/* static function declare */

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif /*_CTS_XR_CONFIG_H*/

