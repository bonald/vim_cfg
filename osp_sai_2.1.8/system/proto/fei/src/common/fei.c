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

#include "proto.h"
#include "fei.h"

#define FEI_ERROR_ARRAY_SIZE    2048
#define FEI_SYNC_TIME   10  /* seconds */

static uint32 g_fei_sync_time = FEI_SYNC_TIME;
static int32 g_fei_sock[FEI_PRIO_MAX];
static uint32 g_fei_error_count = 0;
static uint32 g_fei_error_array[FEI_ERROR_ARRAY_SIZE] = {0};

int32
_fei_sock_init(fei_sock_prio_t prio, int32 immediate)
{
    const char *sock_path = NULL;
    const char *desc = NULL;
    int32 sock = INVALID_SOCK_ID;
    int32 rc = PM_E_NONE;
    
    switch (prio)
    {
    case FEI_PRIO_NORMAL:
        sock_path = SOCK_ADDR_FEI;
        desc = "FeiNormal";
        break;
    case FEI_PRIO_HIGH:
        sock_path = SOCK_ADDR_FEI_HIGH;
        desc = "FeiHigh";
        break;
    default:
        return PM_E_INVALID_PARAM;
    }

    sock = ctc_socket(CTC_SOCK_IPC, 0, desc);
    rc = ctc_sock_connect(sock, sock_path, immediate);
    if (rc < 0)
    {
        return rc;
    }

    g_fei_sock[prio] = sock;

    return 0;
}

int32
fei_init(lib_master_t *lib_master, FEI_ASYNC_CB_FUNC cb, uint32 high, int32 immediate)
{
    int32 rc = PM_E_NONE;

    g_fei_sock[FEI_PRIO_NORMAL] = INVALID_SOCK_ID;
    g_fei_sock[FEI_PRIO_HIGH] = INVALID_SOCK_ID;
    
    PM_E_RETURN(_fei_sock_init(FEI_PRIO_NORMAL, immediate));
    if (high)
    {
        PM_E_RETURN(_fei_sock_init(FEI_PRIO_HIGH, immediate));
    }
    
    return PM_E_NONE;
}

static inline int32
_fei_talk_async(uint32 cmd, void *req, uint32 req_len, uint32 high)
{
    ctc_msg_t hal_req;
    ctc_msg_t *p_hal_req = &hal_req;
    uint32 sock_index = 0;
    int32 rc = PM_E_NONE;

    sock_index = (high) ? FEI_PRIO_HIGH : FEI_PRIO_NORMAL;
    sal_memset(&hal_req, 0, sizeof(hal_req));
    MSG_BUILD_HDR(p_hal_req, MSG_OPER_FEI, req_len);
    p_hal_req->u_hdr.fei.type = cmd;
    p_hal_req->u_hdr.fei.flags = 0;
    p_hal_req->data_len = req_len;
    p_hal_req->data = req;

    rc = ctc_sock_send(g_fei_sock[sock_index], IGNORE_SOCK_PEER, p_hal_req);
    if (rc < 0)
    {
        if (g_fei_error_count < FEI_ERROR_ARRAY_SIZE)
        {
            g_fei_error_array[g_fei_error_count] = cmd;
        }
        g_fei_error_count++;
    }
    return rc;
}

int32
fei_talk_sync(uint32 cmd, void *req, uint32 req_len, void *resp, uint32 resp_len)
{
    ctc_msg_t req_msg;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_req_msg = &req_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    uint32 sock_index = FEI_PRIO_NORMAL;
    int32 rc = PM_E_NONE;

    sal_memset(&req_msg, 0, sizeof(req_msg));
    sal_memset(&resp_msg, 0, sizeof(resp_msg));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_FEI, req_len);
    p_req_msg->u_hdr.fei.type = cmd;
    p_req_msg->u_hdr.fei.flags = MSG_HAL_FLAGS_SYNC;
    p_req_msg->data_len = req_len;
    p_req_msg->data = req;
    
    rc = ctc_sock_send_sync_fei(g_fei_sock[sock_index], IGNORE_SOCK_PEER, p_req_msg, p_resp_msg, g_fei_sync_time * SOCK_TICS_PER_SEC, resp_len);
    if (rc < 0)
    {
        if (g_fei_error_count < FEI_ERROR_ARRAY_SIZE)
        {
            g_fei_error_array[g_fei_error_count] = cmd;
        }
        g_fei_error_count++;
        return rc;
    }
    SAL_ASSERT(p_resp_msg->u_hdr.fei.type == cmd);
    SAL_ASSERT(p_resp_msg->data_len == resp_len);
    sal_memcpy(resp, p_resp_msg->data, p_resp_msg->data_len);
    ctc_msg_free(p_resp_msg);
    return rc;
}

int32
fei_talk_sync_for_openflow_barrier(uint32 cmd, void *req, uint32 req_len, void *resp, uint32 resp_len)
{
    ctc_msg_t req_msg;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_req_msg = &req_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    uint32 sock_index = FEI_PRIO_NORMAL;
    int32 rc = PM_E_NONE;

    sal_memset(&req_msg, 0, sizeof(req_msg));
    sal_memset(&resp_msg, 0, sizeof(resp_msg));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_FEI, req_len);
    p_req_msg->u_hdr.fei.type = cmd;
    p_req_msg->u_hdr.fei.flags = MSG_HAL_FLAGS_SYNC;
    p_req_msg->data_len = req_len;
    p_req_msg->data = req;
    
    rc = ctc_sock_send_sync(g_fei_sock[sock_index], IGNORE_SOCK_PEER, p_req_msg, p_resp_msg, 100 * SOCK_TICS_PER_SEC); /*100s for openflow*/
    if (rc < 0)
    {
        if (g_fei_error_count < FEI_ERROR_ARRAY_SIZE)
        {
            g_fei_error_array[g_fei_error_count] = cmd;
        }
        g_fei_error_count++;
        return rc;
    }
    SAL_ASSERT(p_resp_msg->u_hdr.fei.type == cmd);
    SAL_ASSERT(p_resp_msg->data_len == resp_len);
    sal_memcpy(resp, p_resp_msg->data, p_resp_msg->data_len);
    ctc_msg_free(p_resp_msg);
    return rc;
}

int32
fei_talk(uint32 cmd, void *req, uint32 req_len)
{
    return _fei_talk_async(cmd, req, req_len, FALSE);
}

int32
fei_talk_high(uint32_t cmd, void *req, uint32 req_len)
{
    /* high is used only for APS etc. , need not use sync mode */
    return _fei_talk_async(cmd, req, req_len, TRUE);
}

/*  */
int32
fei_talk_with_rc(uint32 cmd, void *req, uint32 req_len)
{
    int32 resp = 0;
    int32 rc = PM_E_NONE;

    rc = fei_talk_sync(cmd, req, req_len, &resp, sizeof(resp));
    if (rc < 0)
    {
        return rc;
    }

    /* resp is the rc from FEA */
    return resp;
}

uint32
fei_get_error_count()
{
    return g_fei_error_count;
}
