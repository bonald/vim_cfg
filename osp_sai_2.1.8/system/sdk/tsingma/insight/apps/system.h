
#ifndef INCLUDE_SYSTEM_APP_H
#define INCLUDE_SYSTEM_APP_H

#ifdef __cplusplus
extern "C"
{
#endif




  typedef enum _insight_system_cmd_
  {
      /* Set group */
      INSIGHT_SYSTEM_CMD_API_SET_FEATURE = 1,
    /* get group */
     INSIGHT_SYSTEM_CMD_API_GET_FEATURE,
    INSIGHT_SYSTEM_CMD_API_GET_SWITCH_PROPERTIES,
    INSIGHT_SYSTEM_CMD_API_CANCEL_REQUEST
  }
  insight_system_cmd_t;

#define INSIGHT_IPADDR_LEN_MAX 20
#define INSIGHT_UID_LEN_MAX 128
#define INSIGHT_VERSION_LEN_MAX 32


  typedef struct _asic_info_
  {
      char          asic_notation[32];
      uint8         asicType;
      int             numPorts;
  }
  insight_asic_info_t;

  /* Structure for switch properties parameters */
  typedef struct _switch_properties_
  {
      sal_time_t timeStamp;
      uint8    networkOs[INSIGHT_NETWORK_OS_LEN_MAX];
      uint8     numAsics;
      uint8     featureMask;
      insight_asic_info_t   asicInfo[INSIGHT_MAX_ASICS];
      uint8      uid[INSIGHT_UID_LEN_MAX];
      uint8      agent_ipaddr[INSIGHT_IPADDR_LEN_MAX];
      uint16     agent_port;
      uint8    agent_sw_version[INSIGHT_VERSION_LEN_MAX];
  }
  insight_system_properties_t;

  /* Structure to pass API parameters to the REG_HB APP */
  typedef struct _switch_hb_params_
  {
      int enable;
      int interval;
      int configMask;
  }
  insight_system_config_feature_params_t;

  /* Structure to pass API parameters to the BST APP */
  typedef struct _switch_cancel_request_
  {
      unsigned int id;
  }
  insight_system_cancel_request_t;


  typedef struct _system_utils_request_msg_
  {
      long msg_type; /* message type */
      int unit; /* variable to hold the asic type */
      void *cookie;
      int id; /* id passed from the request */
      int version; /* json version */
      uint8  report_type;  /*1:non period;2: period*/
      union
      {
          /* feature params */
          insight_system_config_feature_params_t   config;
          insight_system_cancel_request_t cancel;
      }
      request;
  }
  insight_system_request_msg_t;


  typedef struct _system_utils_response_msg_
  {
      long msg_type;
      int unit;
      void *cookie;
      int id;
      uint8 report_type;

      INSIGHT_STATUS ret; /* return value for set request */

     insight_system_properties_t  switch_properties;
      insight_system_config_feature_params_t config;

  }
  insight_system_response_msg_t;


typedef INSIGHT_STATUS(*INSIGHT_CANCEL_API_HANDLER_t) (unsigned int unit, unsigned int id);
 typedef INSIGHT_STATUS(*INSIGHT_SYSTEM_API_HANDLER_t) (insight_system_request_msg_t * msg_data);


 /** Definition of an SYSTEM_UTILS API */
 typedef struct _insight_system_cancel_api_
 {
     /* this element is free or not */
     bool present;
     /** system_utils command */
     int module;
     /** Handler associated with the corresponding bst command */
     INSIGHT_CANCEL_API_HANDLER_t    handler;
 }
 insight_system_cancel_api_t;


 /** Definition of an SYSTEM_UTILS API */
 typedef struct _insight_system_api_
 {
     /** system_utils command */
     int command;
     /** Handler associated with the corresponding bst command */
     INSIGHT_SYSTEM_API_HANDLER_t    handler;
 }
 insight_system_api_t;


 typedef struct insight_system_info_s
 {
     pthread_mutex_t  mutex;
     /* Switch Properties*/
     uint8 hb_enable;
     uint8 hb_inuse;
     uint32_t hb_interval;
     timer_t hb_timer;

     /*  Key to Queue Message*/
     key_t key1;
     /* message queue id  */
     int recvMsgQid;

     insight_system_properties_t switch_properties;
     insight_system_cancel_api_t cancel_api_list[INSIGHT_MAX_FEATURES];

     /* pthread ID*/
     pthread_t rx_thread;

          pthread_t rest_thread;
 }
 insight_system_info_t;

  extern INSIGHT_STATUS
  insight_system_register_cancel_request(unsigned int featId,
                                                INSIGHT_CANCEL_API_HANDLER_t handler);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_SYSTEM_UTILS_APP_H */

/*!  @}
 * @}
 */
