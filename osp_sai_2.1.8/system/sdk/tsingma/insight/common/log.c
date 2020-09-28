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

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "log_api.h"


static pthread_mutex_t logMutex;
static void  map_bview_to_syslog_severity(INSIGHT_SEVERITY severity,
                                           int * syslog_severity);
extern int check_file_availability(char * filename);
extern int filebased_logging_init(void);

void log_to_file(char *buf);

static char logFile[MAX_STRING_LENGTH] = "/tmp/agent_log_latest";
static char logBkpFile[MAX_STRING_LENGTH] = "/tmp/agent_log_latest.bkp";

#define FILE_OPEN do { \
    fp = fopen(logFile, "a");\
    if (fp == NULL) { \
        printf("Log : Unable to open file for logging [%d:%s] \n", \
                    errno, strerror(errno)); \
        pthread_mutex_unlock(&logMutex); \
        return; \
    } \
} while(0)

/*********************************************************************
* @brief        Initialize logging module.
*               This function uses "openlog()" in-order
*               to open a connection to system logger
*               Logs are stored in a file incase syslog 
*               is not available 
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

void logging_init()
{
  /*Filebased logging is used when syslog is not available*/
  if ((check_file_availability(RSYSLOG_CFG_FILENAME) == -1) &&
      (check_file_availability(SYSLOG_CFG_FILENAME) == -1))
  {
    if (filebased_logging_init() != 0)
    {
      printf("Error! Filebased Logging not initialized\n");
    }
  }
  else
  {
  /*"openapps" is prepended to every logged message*/
  /*SYSLOG is configured to log generic user-level messages
    and includes PID in every message*/
    openlog("openapps", LOG_PID, LOG_USER);
  }
}

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

void logging_deinit()
{
  if ((check_file_availability(RSYSLOG_CFG_FILENAME) == -1) &&
     (check_file_availability(SYSLOG_CFG_FILENAME) == -1))
  {
    pthread_mutex_destroy(&logMutex);
  }
  else
  {
    closelog();
  }
}


/*********************************************************************
* @brief  Initializes file based logging
*
*
* @param[in]    NA
*
* @retval 0  when logging is initialized successfully
*
* @note         NA
*
* @end
*********************************************************************/

int filebased_logging_init(void)
{
    FILE *fp;
    struct stat st = {0};

    if (stat(LOG_PATH,&st) == -1)
    {
      if (mkdir(LOG_PATH,0777) != -1)
      {
        strcpy(logFile,LOG_FILE);
        strcpy(logBkpFile,BACKUP_LOG_FILE);
      }
      else
      {
        printf("Log files will be stored in /tmp \n");
      }
    }
    else
    {
      if (S_ISDIR(st.st_mode))
      {
        strcpy(logFile,LOG_FILE);
        strcpy(logBkpFile,BACKUP_LOG_FILE);
      }
      else
      {
        printf("Log files will be stored in /tmp \n");
      }
    } 

    /* initialize the mutex*/
    if(pthread_mutex_init(&logMutex, NULL) != 0)
    {
      printf("Logging Mutex Error \n");
      return -1;
    }
    /* truncate the logging file if already available */
    fp = fopen(logFile, "w");
    if (fp != NULL)
    {
        fclose(fp);
    }
    else
    {
      printf("Log : Unable to open file for logging [%d:%s] \n", 
                  errno, strerror(errno)); 
      return -1;
    }
    return 0;
}

/******************************************************************
 * @brief  Check if file exists
 *
 * @param[in] filename Name of the file/directory 
 *
 * @retval    0  when logging is initialized successfully
 *
 * @note
 *
 * @end
 *********************************************************************/

int check_file_availability(char * filename)
{
 
  return -1;
}



/*********************************************************************
* @brief         This function is used to map INSIGHT_SEVERITY
*                to standard SYSLOG severity
*
*
* @param[in]     severity BVIEW severity set by a component
* @param[out]    SYSLOG severity corrosponding to the BVIEW
*                severity
*
* @retval       INSIGHT_STATUS
*
* @note         NA
*
* @end
*********************************************************************/

static void map_bview_to_syslog_severity(INSIGHT_SEVERITY severity,
                                                   int * syslog_severity)
{
  switch(severity)
  {
    case INSIGHT_LOG_EMERGENCY:
      *syslog_severity = LOG_EMERG;
      break;
    case INSIGHT_LOG_ALERT:
      *syslog_severity = LOG_ALERT;
      break;
    case INSIGHT_LOG_CRITICAL:
      *syslog_severity = LOG_CRIT;
      break;
    case INSIGHT_LOG_ERROR:
      *syslog_severity = LOG_ERR;
      break;
    case INSIGHT_LOG_WARNING:
      *syslog_severity = LOG_WARNING;
      break;
    case INSIGHT_LOG_NOTICE:
      *syslog_severity = LOG_NOTICE;
      break;
    case INSIGHT_LOG_INFO:
      *syslog_severity = LOG_INFO;
      break;
    case INSIGHT_LOG_DEBUG:
      *syslog_severity = LOG_DEBUG;
      break;
    default:
      *syslog_severity = LOG_EMERG;
      break;
  }
}


/*********************************************************************
* @brief        Function used to post log to a file
*
*
* @param[in]    Text which needs to be printed in log file 
*
* @retval       NA
*
* @note         Logs are stored at a location based on their severity 
*               and the location specified in rsyslog.conf
*
* @end
*********************************************************************/

void log_to_file(char *buf)
{
  FILE *fp = NULL;
  int length = 0,status = 0;
  char timeString[MAX_STRING_LENGTH] = { 0 };
  time_t logtime;
  struct stat fileStat;
  struct tm *timeinfo;
  
  if (buf == NULL)
  {
    printf("Log: NULL Pointer, Cannot log to file\n");
    return;
  }
  
  time(&logtime);
  timeinfo = localtime(&logtime);
  strftime(timeString, MAX_STRING_LENGTH, TIME_FORMAT, timeinfo);
  
  pthread_mutex_lock(&logMutex);
  FILE_OPEN;
  length = (strlen(buf) + strlen(timeString));
  if (stat(logFile,&fileStat) < 0)
  {
    printf("Log : Unable to read file stats [%d:%s] \n",
                errno, strerror(errno));
    pthread_mutex_unlock(&logMutex);
    fclose(fp);
    return;
  }
  if ((fileStat.st_size + length) > MAX_FILE_SIZE)
  {
    if (check_file_availability(logBkpFile) == -1)
    {
      fclose(fp);
      status = rename(logFile,logBkpFile);
      if (status == -1)
      {
        printf("Error Unable to rename file [%d:%s] \n",
                  errno, strerror(errno));
      }
    }
    else
    {
      status = remove(logBkpFile);
      if (status == -1)
      {
        printf("Error Unable to remove file [%d:%s] \n",
                  errno, strerror(errno));
      }
      fclose(fp);
      status = rename(logFile,logBkpFile);
      if (status == -1)
      {
        printf("Error Unable to rename file [%d:%s] \n",
                  errno, strerror(errno));
      }
    }
    FILE_OPEN;
  }
  fprintf(fp,"%s %s\n",timeString,buf);  
  fclose(fp);
  pthread_mutex_unlock(&logMutex);
}


/*********************************************************************
* @brief        Function used to post log
*
*
* @param[in]    severity of the log (SYSLOG severity) 
* @param[in]    format string with arguments to be stored in log
*
* @retval       NA
*
* @note         Logs are stored at a location based on their severity 
*               and the location specified in rsyslog.conf
*
* @end
*********************************************************************/

void log_post(INSIGHT_SEVERITY severity, char * format, ...)
{
  char buf[MAX_STRING_LENGTH];
  int syslog_severity,rc=0;

  map_bview_to_syslog_severity(severity, &syslog_severity); 
  if ((check_file_availability(RSYSLOG_CFG_FILENAME) == -1) &&
     (check_file_availability(SYSLOG_CFG_FILENAME) == -1))
  {
    va_list ap;
    va_start(ap, format);
    rc = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    if (rc < 0)
    {
      printf("Log : Unable to copy data to buffer  \n");
      return;
    }
    log_to_file(buf);
  }
  else
  {
    va_list ap;
    va_start(ap, format);
    vsyslog(syslog_severity, format,ap);
    va_end(ap);
  }
}
