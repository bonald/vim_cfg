
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "insight.h"
#include "log_api.h"

#define CONVERSION_FACTOR 1000000L



/*********************************************************************
* @brief         Function used to create and arm a timer
*
*
* @param[in]     handler        function pointer to handle the callback
*                               The callback function address is passed
*                               using a void * because the function declared
*                               in linux "sigev_notify_function" has a 
*                               return type "void"  
* @param[in,out] timerId        timerId of the timer which is created
* @param[in]     timeInMilliSec Time after which callback is required
* @param[in]     mode           mode specifing if the timer must be
*                               periodic or oneshot
* @param[in]     param          Arguments passed from the calling function
*
* @retval        INSIGHT_STATUS_SUCCESS
* @retval        INSIGHT_STATUS_FAILURE
*
* @note          NA
*
* @end
*********************************************************************/


INSIGHT_STATUS system_timer_add(void * handler,timer_t * timerId, int timeInMilliSec, uint8 mode, void * param)
{
  struct sigevent sigTime;
  struct timespec monotonic_time;
  clockid_t clock;
  int ret = INSIGHT_STATUS_SUCCESS;
  /*Validate callback function pointers*/
  if (handler == NULL)
  {
    LOG_POST (INSIGHT_LOG_ERROR, "Invalid handler during timer registration \r\n");
    return INSIGHT_STATUS_FAILURE;
  }
  
  /*Validate pointer which stores timer id*/
  if (timerId == NULL)
  {
    LOG_POST (INSIGHT_LOG_ERROR, "Invalid timer id during timer registration\r\n");
    return INSIGHT_STATUS_FAILURE;
  }

  /*Populate the structures in-order to create the timer*/
  memset(&sigTime, 0,sizeof(struct sigevent)); 
  sigTime.sigev_notify = SIGEV_THREAD;
  sigTime.sigev_notify_attributes = NULL;
  sigTime.sigev_notify_function = handler;
  sigTime.sigev_value.sival_ptr = param;
  /*Monotonic clock is used if available else realtime clock is used*/ 
  if (!clock_gettime(CLOCK_MONOTONIC, &monotonic_time)) 
   {
     clock = CLOCK_MONOTONIC;
   } 
   else 
   {
     clock = CLOCK_REALTIME;
   }

  /*Timer is created*/
  if (timer_create(clock, &sigTime, timerId) == 0)
   {
     ret = system_timer_set(*timerId,timeInMilliSec,mode); 
   }
   else 
   {
     LOG_POST (INSIGHT_LOG_ERROR, "Timer Creation Failed Error no : %d\r\n",errno);
     ret = INSIGHT_STATUS_FAILURE;
   }
  return ret;
}

/*********************************************************************
* @brief      Function used to destroy a timer
*
*
* @param[in]  timerId Timer id of the timer which needs to be destroyed
*
* @retval     INSIGHT_STATUS_SUCCESS
* @retval     INSIGHT_STATUS_FAILURE
*
* @note          NA
*
* @end
*********************************************************************/


INSIGHT_STATUS system_timer_delete(timer_t timerId)
{
  int ret = INSIGHT_STATUS_SUCCESS;
  if ((timer_delete(timerId)) !=0)
  {
    ret = INSIGHT_STATUS_FAILURE;
  }
  return ret;
}

/*********************************************************************
* @brief      Function used to set/reset a timer
*
*
* @param[in]  timerId        Timer id of the timer which needs to be 
*                            set/reset
* @param[in]  timeInMilliSec Time after which callback is required
* @param[in]  mode           mode specifing if the timer must be
*                            periodic or oneshot
*
* @retval     INSIGHT_STATUS_SUCCESS
* @retval     INSIGHT_STATUS_FAILURE
*
* @note       NA
*
* @end
*********************************************************************/


INSIGHT_STATUS system_timer_set(timer_t timerId,int timeInMilliSec,uint8 mode)
{
  int ret = INSIGHT_STATUS_SUCCESS;
  struct itimerspec timerVal;
  timerVal.it_value.tv_sec = timeInMilliSec / 1000;
  timerVal.it_value.tv_nsec = (long)(timeInMilliSec % 1000) * CONVERSION_FACTOR;
  if (mode == 1)/*Timer is triggered periodically*/
  {
    timerVal.it_interval.tv_sec = timerVal.it_value.tv_sec;
    timerVal.it_interval.tv_nsec = timerVal.it_value.tv_nsec;
  }
  else/*Timer is triggered only once, but timer is not destroyed*/
  {
    timerVal.it_interval.tv_sec = 0;
    timerVal.it_interval.tv_nsec = 0;
  }
  /*Timer is set*/
  if (timer_settime(timerId, 0, &timerVal, NULL) != 0)
  {
    LOG_POST (INSIGHT_LOG_ERROR, "Timer Initialization Failed, Error no : %d\r\n",errno);
    ret = INSIGHT_STATUS_FAILURE;
  }
  return ret;
}

/*********************************************************************
* @brief      Function to get current time in %Y-%m-%dT%H:%M:%S:%ms
*
* @param[out]  buf        Pointer to buf which has current time 
*                            
*
* @retval     INSIGHT_STATUS_SUCCESS
*
* @note       NA
*
* @end
*********************************************************************/
INSIGHT_STATUS   system_dispaly_local_time_get (char *buf)
{
  struct timeval tv;
  time_t l_now;
  char   local_buf[INSIGHT_TIME_BUFFER_SIZE];
  struct tm date, *loc_time;

  memset (&tv, 0, sizeof (tv));
  gettimeofday(&tv,NULL);
  l_now = tv.tv_sec;
  loc_time = localtime_r (&l_now, &date);
  strftime (local_buf, INSIGHT_TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", loc_time);
  sprintf (buf,"%s.%03u",local_buf,(unsigned int)tv.tv_usec/1000);
  return INSIGHT_STATUS_SUCCESS;
}

