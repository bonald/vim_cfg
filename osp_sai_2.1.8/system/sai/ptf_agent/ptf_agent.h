
#ifndef __PTF_AGENT_H__
#define __PTF_AGENT_H__

#define PTF_ADDR_LEN        6
#define PTF_MAX_PORT_NUM    64

#define PTF_HDLR_OBJECT_TYPE_SET(objtype,index) (((sai_object_id_t)objtype << 32) | (sai_object_id_t)index)

#define PTF_ERROR_RETURN(op) \
do { \
    rc = (op); \
    if (SAI_STATUS_SUCCESS != rc) \
    {  \
        PTF_HDLR_LOG("HAL Server OP failed: rc = %d, %s:%d", rc, __FUNCTION__, __LINE__); \
        *_return = rc; \
        return TRUE; \
    } \
}while(0)

typedef int (*PTF_HDLR_LOG_OUT_FUNC)(const char* str, ...);

int32
sai_set_out_func(PTF_HDLR_LOG_OUT_FUNC func);

int32
ptf_agent_set_default();

void
ptf_agent_init(void* user_param);

void
ptf_agent_deinit();

#endif /* !__PTF_AGENT_H__ */

