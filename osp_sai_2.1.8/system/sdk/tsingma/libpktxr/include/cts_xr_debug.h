/******************************************************************************/
/* Filename: cts_xr_debug.h                                                   */
/* File description:                                                              */
/*                                                                            */
/* belong to module:                                                              */
/*                                                */
/* author: flin                                                               */
/* Date: 2006-08-09 14:11                                                     */
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
#ifndef _CTS_XR_DEBUG_H
#define _CTS_XR_DEBUG_H
/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/* #include*/

/* #define*/


#define CTS_ERROR       1
#define CTS_WARNING     2
#define CTS_INFO        3

#define CTS_MSG(type, fmt...) do {\
        char *pre_msg;      \
        if (CTS_ERROR == type) {\
            pre_msg = "Error";  \
        } if (CTS_WARNING == type) {\
            pre_msg = "Warning";  \
        } if (CTS_INFO == type) {\
            pre_msg = "Info";  \
        }\
        printf("%s::\n\t", pre_msg);    \
        printf(fmt);        \
        printf("\n");        \
    }while (0)

#define CTS_DBG_FLAG_FUNCTION   0x01
#define CTS_DBG_FLAG_ARGS       0x02
#define CTS_DBG_FLAG_PKT        0x04
#define CTS_DBG_FLAG_CAP        0x08
#define CTS_DBG_FLAG_MSG        0x10
#define CTS_DBG_FLAG_FILTER         0x20
#define CTS_DBG_FLAG_LOOPBACK       0x40


#define CTS_DBG_FLAG_ALL        (0xFFFFFFFF)

#define CTS_DBG_INIT_FLAG   (0)
//#define CTS_DBG_INIT_FLAG   (CTS_DBG_FLAG_LOOPBACK | CTS_DBG_FLAG_FILTER | CTS_DBG_FLAG_ARGS | CTS_DBG_FLAG_CAP| CTS_DBG_FLAG_MSG)
//#define CTS_DBG_INIT_FLAG (CTS_DBG_FLAG_ARGS)
//#define CTS_DBG_INIT_FLAG (CTS_DBG_FLAG_PKT)
extern uint32 cts_dbg_flag;
#define CTS_DBG_FLAG cts_dbg_flag


#define CTS_DBG_ENABLE_PKT  (CTS_DBG_FLAG & CTS_DBG_FLAG_PKT)

#ifdef CTS_DEBUG_ENABLE
#define CTS_DBG_BE(fmt...) do {\
        if (CTS_DBG_FLAG & CTS_DBG_FLAG_FUNCTION) {\
            printf("%s[%d]::", __FUNCTION__, __LINE__ );    \
            printf(fmt);        \
            printf("\n");        \
        }\
    }while (0)

#define CTS_DBG(fmt...) do {\
        if (CTS_DBG_FLAG & CTS_DBG_FLAG_MSG) {\
            printf("%s[%d]::", __FUNCTION__, __LINE__ );    \
            printf(fmt);        \
            printf("\n");        \
        }\
    }while (0)

#define CTS_PKT(fmt...) do {\
        if (CTS_DBG_FLAG & CTS_DBG_FLAG_PKT) {\
            printf("%s[%d]::", __FUNCTION__, __LINE__ );    \
            printf(fmt);        \
            printf("\n");        \
        }\
    }while (0)


#define CTS_DBG_ARGS \
    do { \
        int argid;\
        if (CTS_DBG_FLAG & CTS_DBG_FLAG_ARGS) {\
            for (argid = 0; argid < objc; argid++) {\
                printf("%s ", Tcl_GetString(objv[argid]));\
            }\
            printf("\n");\
        }\
    }while(0)

#define CTS_CAP(fmt...) do {\
        if (CTS_DBG_FLAG & CTS_DBG_FLAG_CAP) {\
            printf("%s[%d]::", __FUNCTION__, __LINE__ );    \
            printf(fmt);        \
            printf("\n");        \
        }\
    }while (0)

#define CTS_DBG_FILTER(fmt...) do {\
        if (CTS_DBG_FLAG & CTS_DBG_FLAG_FILTER) {\
            printf("%s[%d]::", __FUNCTION__, __LINE__ );    \
            printf(fmt);        \
            printf("\n");        \
        }\
    }while (0)
#else /* CTS_DEBUG_ENABLE*/
#define CTS_DBG(fmt...)
#define CTS_PKT(fmt...)
#define CTS_CAP(fmt...)
#define CTS_DBG_FILTER(fmt...)
#define CTS_DBG_ARGS
#endif /* CTS_DEBUG_ENABLE*/
/* const*/

/* structure declare */

/* global veriable declare */

/* function declare */

/* static function declare */

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif /*_CTS_XR_DEBUG_H*/

