/**
 *  Copyright (C) 2011, 2012 CentecNetworks, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file
 * @brief This file defines macro used in adapter layer
 */
#ifndef __OFP_MACRO_H__
#define __OFP_MACRO_H__

/******************************************************************************
* Header Files 
******************************************************************************/

/*******************************************************************
*
*Structures and macros, enums
*
********************************************************************/

extern int ctc_cli_out_ofp(const char *fmt, ...);
extern uint16_ofp ofp_get_debug_mode(void);

#define IPV4_MASK_2_LEN(mask, len) \
    { \
        int32 i; \
        for (i = 31; i >= 0; i--) \
        { \
            if (GLB_FLAG_ISSET((mask), 1 << (uint32)i)) \
            {(len)++; } \
            else \
            {break; } \
        } \
    }

#define IPV4_MASK_BITS_NUM(mask, num) \
    { \
        int32 i; \
        for (i = 31; i >= 0; i--) \
        { \
            if (GLB_FLAG_ISSET((mask), 1 << (uint32)i)) \
            {(num)++; } \
        } \
    }

#define OFP_PTR_CHECK(ptr)                                              \
do {                                                                    \
    if (NULL == (ptr))                                                  \
    {                                                                   \
        VLOG_ERR("Invalid pointer, %s()@%d", __FUNCTION__, __LINE__);    \
        return (OFP_ERR_INVALID_PTR);                                   \
    }                                                                   \
}while(0)

#define OFP_MEM_PTR_CHECK(ptr)                                          \
do {                                                                    \
    if (NULL == (ptr))                                                  \
    {                                                                   \
        VLOG_ERR("No memory, %s()@%d", __FUNCTION__, __LINE__);         \
        return (OFP_ERR_NO_MEMORY);                                     \
    }                                                                   \
}while(0)

#define OFP_ERROR_RETURN(op) \
do { \
    int rv; \
    if ((rv = (op)) < 0) \
    {\
        OFP_LOG_DEBUG("%s:%d, rv = %d\n", __FUNCTION__, __LINE__, rv); \
        return(rv); \
    }\
}while(0)

#define OFP_DEBUG_PRINT_TO_FILE(fmt, args...)                      \
{ \
    FILE * fp = fopen("/mnt/flash/log1.txt", "a"); \
    if (fp != NULL) \
    { \
        fprintf(fp, fmt, ##args); \
        fclose(fp); \
    } \
}

#define OFP_DEBUG_PRINT(fmt, args...)                       \
    OFP_LOG_DEBUG(fmt, ##args);

#define OFP_CONSOLE_PRINT(fmt, args...)                     \
{                                                           \
    FILE * fp_console = NULL;                               \
    fp_console = fopen("/dev/console", "a+");               \
    fprintf(fp_console, fmt, ##args);                       \
    fclose(fp_console);                                     \
}

#if 1
#define OFP_LOG_DEBUG_FUNC()                                \
{                                                           \
    VLOG_DBG("Enter into %s\n", __FUNCTION__);              \
}
#else
#define OFP_LOG_DEBUG_FUNC()
#endif

#define OFP_LOG_INFO(fmt, args...)                          \
{                                                           \
    VLOG_INFO(fmt"\n", ##args);                             \
}

#if 1
#define OFP_LOG_DEBUG(fmt, args...)                         \
{                                                           \
    VLOG_DBG(fmt"\n", ##args);                              \
}
#else
#define OFP_LOG_DEBUG(fmt, args...)
#endif

#define OFP_LOG_WARN(fmt, args...)                          \
{                                                           \
    VLOG_WARN(fmt"\n", ##args);                             \
}

#define OFP_LOG_ERROR(fmt, args...)                         \
{                                                           \
    VLOG_ERR(fmt"\n", ##args);                              \
}

#define OFP_PORT_XMALLOC(size) XMALLOC(MEM_PM_OPENFLOW_PORT, size)
#define OFP_PORT_XCALLOC(size) XCALLOC(MEM_PM_OPENFLOW_PORT, size)
#define OFP_PORT_XFREE(ptr)    XFREE(MEM_PM_OPENFLOW_PORT, ptr)

#define OFP_FLOW_XMALLOC(size) XMALLOC(MEM_PM_OPENFLOW_FLOW, size)
#define OFP_FLOW_XCALLOC(size) XCALLOC(MEM_PM_OPENFLOW_FLOW, size)
#define OFP_FLOW_XFREE(ptr)    XFREE(MEM_PM_OPENFLOW_FLOW, ptr)

#define OFP_GROUP_XMALLOC(size) XMALLOC(MEM_PM_OPENFLOW_GROUP, size)
#define OFP_GROUP_XCALLOC(size) XCALLOC(MEM_PM_OPENFLOW_GROUP, size)
#define OFP_GROUP_XFREE(ptr)    XFREE(MEM_PM_OPENFLOW_GROUP, ptr)

/* Added by weizj for meter */
#define OFP_METER_XMALLOC(size) XMALLOC(MEM_PM_OPENFLOW_METER, size)
#define OFP_METER_XCALLOC(size) XCALLOC(MEM_PM_OPENFLOW_METER, size)
#define OFP_METER_XFREE(ptr)    XFREE(MEM_PM_OPENFLOW_METER, ptr)
/* End by weizj for meter */

/* Added by weizj for tunnel */
#define OFP_TUNNEL_XMALLOC(size) XMALLOC(MEM_PM_OPENFLOW_TUNNEL, size)
#define OFP_TUNNEL_XCALLOC(size) XCALLOC(MEM_PM_OPENFLOW_TUNNEL, size)
#define OFP_TUNNEL_XFREE(ptr)    XFREE(MEM_PM_OPENFLOW_TUNNEL, ptr)
/* End by weizj for tunnel */

#define OFP_COMMON_XMALLOC(size) XMALLOC(MEM_PM_OPENFLOW_COMMON, size)
#define OFP_COMMON_XCALLOC(size) XCALLOC(MEM_PM_OPENFLOW_COMMON, size)
#define OFP_COMMON_XFREE(ptr)    XFREE(MEM_PM_OPENFLOW_COMMON, ptr)

#endif /* !__OFP_MACRO_H__ */
