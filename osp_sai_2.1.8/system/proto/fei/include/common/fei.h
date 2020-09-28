/**
 *  Copyright (C) 2011, 2012, 2013 CentecNetworks, Inc. All Rights Reserved.
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
 */

typedef enum fei_sock_prio_e
{
    FEI_PRIO_NORMAL  = 0,
    FEI_PRIO_HIGH    = 1,
    FEI_PRIO_MAX
} fei_sock_prio_t;

typedef int32 (*FEI_ASYNC_CB_FUNC)(void *);

int32
fei_init(lib_master_t *lib_master, FEI_ASYNC_CB_FUNC cb, uint32 high, int32 immediate);

int32
fei_talk(uint32 cmd, void *req, uint32 req_len);

int32
fei_talk_sync(uint32 cmd, void *req, uint32 req_len, void *resp, uint32 resp_len);

int32
fei_talk_sync_for_openflow_barrier(uint32 cmd, void *req, uint32 req_len, void *resp, uint32 resp_len);

int32
fei_talk_high(uint32_t cmd, void *req, uint32 req_len);

uint32
fei_get_error_count();

