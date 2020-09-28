/*****************************************************************************
 * ctcutil_log.h    logging facility
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:    R0.01
 * Author:      Jack Yang
 * Date:        2005-08-16
 * Reason:      Initial version
 *****************************************************************************/
#ifndef _CTCUTIL_LOG_H_
#define _CTCUTIL_LOG_H_

/*
 * output device
 */
#define CTCUTIL_LOG_CONSOLE                 1
#define CTCUTIL_LOG_FILE                    2
#define CTCUTIL_LOG_ALL                     0xffffffff


/*
 * module names
 */
enum log_module
{
    CTCUTIL_LOG_SAL,
    CTCUTIL_LOG_CMODEL,
    CTCUTIL_LOG_SDK,
    CTCUTIL_LOG_CLI,
    CTCUTIL_LOG_OTHER,
    CTCUTIL_LOG_MAX_MODULE
};


/*
 * log level
 */
enum log_level
{
    CTCUTIL_LOG_EMERGENCY,
    CTCUTIL_LOG_ALERT,
    CTCUTIL_LOG_CRITICAL,
    CTCUTIL_LOG_ERROR,
    CTCUTIL_LOG_WARNING,
    CTCUTIL_LOG_NOTICE,
    CTCUTIL_LOG_INFO,
    CTCUTIL_LOG_DEBUG,
    CTCUTIL_LOG_MAX_LEVEL
};


/*
 * log flags, can be or-ed
 */
#define CTCUTIL_LOG_ADD_TIME                1
#define CTCUTIL_LOG_PREFIX_SEPARATE         2


/*
 * rotate mode
 */
enum rotate_mode
{
    CTCUTIL_LOG_NO_ROTATE,
    CTCUTIL_LOG_ROTATE_BY_TIME,
    CTCUTIL_LOG_ROTATE_BY_SIZE
};


/*
 * convenient macros
 */
#define CTCUTIL_LOG_STANDARD(module, level, fmt...) \
    ctcutil_log_standard(__FILE__, __FUNCTION__, __LINE__, module, level, fmt);


/*
 * exported log function
 */
int32 ctcutil_log(uint32 module, int32 level, char *fmt, ...);
int32 ctcutil_log_standard(char *file, char *function, int32 line,
                           uint32 module, int32 level, char *fmt, ...);
int32 ctcutil_log_prefix(uint32 module, int32 level, char *prefix,
                         char *fmt, ...);

int32 ctcutil_log_set_device(uint32 device);
int32 ctcutil_log_get_device(uint32 *p_device);

int32 ctcutil_log_set_filename(char *filename);
int32 ctcutil_log_get_filename(char *buf, size_t size);

int32 ctcutil_log_set_flag(uint32 flag);
int32 ctcutil_log_get_flag(uint32 *p_flag);

int32 ctcutil_log_set_level(uint32 module, int32 level);
int32 ctcutil_log_get_level(uint32 module, int32 *p_level);

int32 ctcutil_log_set_rotate_mode(uint32 mode, int32 max_rotate_num,
                                  uint32 sec, uint32 size);
int32 ctcutil_log_get_rotate_mode(uint32 *p_mode, int32 *p_max_rotate_num,
                                  uint32 *p_sec, uint32 *p_size);


int32 ctcutil_log_mem(uint32 module, int32 level, void *addr, uint32 size);
#if 0
int32 ctcutil_log_mac(uint32 module, int32 level, mac_addr_t mac);
uint8 *ctcutil_mac_to_str(mac_addr_t mac);
#endif


#endif  /* _CTCUTIL_LOG_H_ */
