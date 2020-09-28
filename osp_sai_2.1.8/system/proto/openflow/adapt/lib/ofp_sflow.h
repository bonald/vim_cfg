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
 * @brief This file defines data structure for adapter layer sflow
 */
#ifndef __OFP_SFLOW_H__
#define __OFP_SFLOW_H__

/******************************************************************************
* Header Files 
******************************************************************************/
#include "sset.h"

/****************************************************************************
 *  
 * Defines and Macros
 *
 ****************************************************************************/

#define OFP_SFLOW_DFT_HEADER_SIZE 128
#define OFP_SFLOW_DFT_COLLECTOR_PORT 6343
#define OFP_SFLOW_DFT_SAMPLING_RATE 400
#define OFP_SFLOW_DFT_POLLING_INTERVAL 30

#define OFP_SFLOW_DFT_SAMPLE_IGS_EN 1
#define OFP_SFLOW_DFT_SAMPLE_EGS_EN 1

#define OFP_SFLOW_DFT_SAMPLE_DISCARD_IGS_EN 0
#define OFP_SFLOW_DFT_SAMPLE_DISCARD_EGS_EN 0

#define OFP_SFLOW_MAX_TO_CPU_PPS  2000  /* pps */

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/

/****************************************************************************
 *  
 * Function
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Structures and macros, enums
 *
 ****************************************************************************/

struct ofp_sflow_options_s
{
    struct sset targets;
    uint32_ofp sampling_rate;
    uint32_ofp polling_interval;
    uint32_ofp header_len;
    uint32_ofp sub_id;
    uint32_ofp agent_ip;
    char*      agent_device;
	char*      real_target;
	sflow_band_type band_flag;
};
typedef struct ofp_sflow_options_s ofp_sflow_options_t;


#endif /* !__OFP_SFLOW_H__ */
