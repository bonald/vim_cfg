/*****************************************************************************
  *
  * Copyright © 2016 Broadcom.  The term "Broadcom" refers
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

#ifndef INCLUDE_JSON_H
#define INCLUDE_JSON_H

#ifdef __cplusplus
extern "C"
{
#endif



#define JSON_MAX_NODE_LENGTH   64

#define _jsonlog(format,args...)              printf(format, ##args)

#define JSON_VALIDATE_POINTER(x,y,z)  do { \
    if ((x) == NULL) { \
    _jsonlog("Invalid (NULL) value for parameter %s ", (y) ); \
    return (z); \
    } \
} while(0)

#define JSON_VALIDATE_JSON_POINTER(x,y,z) do { \
    if ((x) == NULL) { \
    _jsonlog("Error parsing JSON %s ", (y) ); \
    return (z); \
    } \
}while(0)

#define JSON_VALIDATE_POINTER_AND_CLEANUP(x,y,z)  do { \
    if ((x) == NULL) { \
    _jsonlog("Invalid (NULL) value for parameter %s ", (y) ); \
    if (root != NULL) { \
    cJSON_Delete(root); \
    } \
    return (z); \
    } \
} while(0)

#define JSON_VALIDATE_JSON_POINTER_AND_CLEANUP(x,y,z)  do { \
    if ((x) == NULL) { \
    _jsonlog("Error parsing JSON %s ", (y) ); \
    if (root != NULL) { \
    cJSON_Delete(root); \
    } \
    return (z); \
    } \
}while(0)

#define JSON_VALIDATE_JSON_AS_STRING(x,y,z)  do { \
    if ((x)->type != cJSON_String) { \
    _jsonlog("Error parsing JSON, %s not a string ", (y) ); \
    if (root != NULL)  { \
    cJSON_Delete(root); \
    } \
    return (z); \
    } \
    if((x)->valuestring == NULL) { \
    _jsonlog("Error parsing JSON, %s not a valid string ", (y) ); \
    if (root != NULL) { \
    cJSON_Delete(root); \
    } \
    return (z); \
    } \
}while(0)

#define JSON_VALIDATE_JSON_AS_NUMBER(x,y)   do { \
    if ((x)->type != cJSON_Number) { \
    _jsonlog("Error parsing JSON, %s not a integer ", (y) ); \
    if (root != NULL)  { \
    cJSON_Delete(root); \
    } \
    return (INSIGHT_STATUS_INVALID_JSON); \
    } \
}while(0)

#define JSON_COMPARE_STRINGS_AND_CLEANUP(x,y,z)  do { \
    if (strcmp((y), (z)) != 0) { \
    _jsonlog("The JSON contains invalid value for %s (actual %s, required %s) ", (x), (y), (z) ); \
    if (root != NULL) { \
    cJSON_Delete(root); \
    } \
    return (INSIGHT_STATUS_INVALID_JSON); \
    } \
}while(0)

#define JSON_COMPARE_VALUE_AND_CLEANUP(x,y,z)  do { \
    if ((y) != (z)) { \
    _jsonlog("The JSON contains invalid value for %s (actual %d, required %d) ", (x), (y), (z) ); \
    if (root != NULL) { \
    cJSON_Delete(root); \
    } \
    return (INSIGHT_STATUS_INVALID_JSON); \
    } \
}while(0)

#define JSON_CHECK_VALUE_AND_CLEANUP(x,y,z)  do { \
    if ( ((x) < (y)) || ( (x) > (z)) ) { \
    _jsonlog("The JSON number out of range %d (min %d, max %d) ", (x), (y), (z) ); \
    if (root != NULL) { \
    cJSON_Delete(root); \
    } \
    return (INSIGHT_STATUS_INVALID_JSON); \
    } \
}while(0)

#define JSON_ASIC_ID_MAP_FROM_NOTATION(_aId, _asicStr)

    //    _aId = atoi(_asicStr);

#define JSON_PORT_MAP_FROM_NOTATION(_port, _portStr)

   //    _port = atoi(_portStr);

#define JSON_ASIC_ID_MAP_TO_NOTATION(_aId, _asicStr)\
     sprintf(_asicStr, "%d", _aId);

#define JSON_PORT_MAP_TO_NOTATION(_port, _asic, _portStr)\
     sprintf(_portStr, "%d", (int)_port);

 #define JSON_LAG_MAP_TO_NOTATION(_port, _asic, _portStr)\
     sprintf(_portStr, "%d", (int)_port);

#define _BVIEW_JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actLen, dst, len, lenptr, format, args...) \
    do { \
            int xtemp = *(lenptr); \
            (actLen) = snprintf((dst), (len), format, ##args); \
            *(lenptr) = (xtemp) + (actLen); \
            if ( (len) == (actLen)) { \
                        /* Out of buffer here */ \
                        _jsonlog("(%s:%d) Out of Json memory while encoding \n", __func__, __LINE__); \
                        return INSIGHT_STATUS_OUTOFMEMORY; \
                    } \
            (dst) += (actLen); \
            (len) -= (actLen); \
        } while(0)


#define BVIEW_COMPARE_WILDCARD(_str,y)  do { \
    if (0 == strncmp("any", _str, strlen(_str))) { \
    y = true; \
    } \
}while(0)



#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_JSON_H */

