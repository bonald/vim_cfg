/*****************************************************************************
  *
  * Copyright � 2016 Broadcom.  The term "Broadcom" refers
  * to Broadcom Limited and/or its subsidiaries.
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  *
  * You may obtain a copy of the License at
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ***************************************************************************/

#ifndef INCLUDE_LOG_H
#define INCLUDE_LOG_H


#define RSYSLOG_CFG_FILENAME      "/etc/rsyslog.conf"
#define SYSLOG_CFG_FILENAME       "/etc/syslog.conf"
#define MAX_STRING_LENGTH         128
#define MAX_FILE_SIZE             (1024*1024)
#define TIME_FORMAT               "%Y-%m-%d %H:%M:%S "
#define LOG_FILE                  "./agent_log_latest"
#define BACKUP_LOG_FILE           "./agent_log_latest.bkp"
#define LOG_PATH                  "/var/log"

/**Severity levels used to post logs*/
typedef enum
{
  INSIGHT_LOG_EMERGENCY = 0,/* system is unusable */
  INSIGHT_LOG_ALERT,        /* action must be taken immediately */
  INSIGHT_LOG_CRITICAL,     /* critical conditions */
  INSIGHT_LOG_ERROR,        /* error conditions */
  INSIGHT_LOG_WARNING,      /* warning conditions */
  INSIGHT_LOG_NOTICE,       /* normal but significant condition */
  INSIGHT_LOG_INFO,         /* informational */
  INSIGHT_LOG_DEBUG         /* debug-level messages */
} INSIGHT_SEVERITY;

/*********************************************************************
* @brief        Function used to post log
*
*
* @param[in]    severity severety of the log (SYSLOG severity)
* @param[in]    format string with arguments to be stored in log
*
* @retval       NA
*
* @note         NA
*
* @end
*********************************************************************/

extern void log_post(INSIGHT_SEVERITY severity, char * format, ...);
/** Default Msg Log Macro for all logs  */
/** Log with a format string and variable arguments */

#define LOG_POST(__sev__,__fmt__,__args__...)                         \
          log_post(__sev__, (char *)__fmt__, ## __args__);\
                      log_post(__sev__, "------- Fun:%s()  Line:%d\n\n\n",__FUNCTION__,__LINE__); \

/*********************************************************************
* @brief        Initialize logging module
*
*
* @param[in]    NA
*
* @retval       NA
*
* @note         NA
*
* @end
*********************************************************************/

void logging_init();

/*********************************************************************
* @brief        Deinitialize logging module
*
*
* @param[in]    NA
*
* @retval       NA
*
* @note         NA
*
* @end
*********************************************************************/

void logging_deinit();

#endif /* INCLUDE_LOG_H */

