#ifndef __HAGT_FLOW_PRIV_H__
#define __HAGT_FLOW_PRIV_H__

#if 0
#define HAGT_DEBUG_FLOW(fmt, args...)                                               \
    HAGT_LOG_DEBUG(flow, flow, DEBUG_FLOW, fmt, ##args)
#else
#define HAGT_DEBUG_FLOW(fmt, args...)
#endif


#define HAGT_FLOW_MAP_TCP_FLAG_OPERATOR(operator)                                   \
    ((operator) == GLB_FLOW_TCP_FLAG_OP_ANY ? 1 :             \
    0)

#endif
