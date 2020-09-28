#if !defined (__CTC_SAI_COMMON_H_)
#define __CTC_SAI_COMMON_H_

#include <saitypes.h>
#include <saiexttypes.h>
#include <sal.h>
#include <ctc_linklist.h>
#include <ctc_hash.h>
#include <ctc_opf.h>

#define CTC_MAX_LPORT               0x200   /* Lport = 3bit slice_id + 8 port_id, GG has 0-1 slice */

#ifndef NEIGHBOR_ACTION_DROP_NHID
#define NEIGHBOR_ACTION_DROP_NHID   1
#endif

#ifndef NEIGHBOR_ACTION_TRAP_NHID
#define NEIGHBOR_ACTION_TRAP_NHID   2
#endif

/* Added by wangjj for supporting the max ecmp num to 64 */
#define CTC_SAI_DEFAULT_MAX_ECMP_NUM        64
/* Added by chenc for supporting the max ecmp num to 16 in gb */
#define CTC_SAI_DEFAULT_MAX_ECMP_NUM_GB        16
/* Added by wangjj for ecmp route nh num need to be checked for rpf supporting, copy from sdk define */
#define CTC_SAI_IP_MAX_RPF_IF               16            /**< Define max RPF interfaces that ipuc and ipmc supports*/
/*Added by hansf for qos queue num*/
#define CTC_SAI_PORT_QUEUE_NUM      8

/*added by huwx for ip source guard, 2012.01.21*/
#define GLB_IP_BINDING_SCL_TYPE_IP    1
#define GLB_IP_BINDING_SCL_TYPE_MAC   2

#define GLB_SFLOW_INGRESS_PACKET     1
#define GLB_SFLOW_EGRESS_PACKET      2

#define CTC_SAI_NHG_ATTR_LIST_MAX 64

#define CTC_SAI_VLAN_ID_MAX             4096

#define CTC_SAI_OBJECT_TYPE_SET(objtype,index)  					\
    (((sai_object_id_t)objtype << 32) | (sai_object_id_t)index)

#define CTC_SAI_OBJECT_TYPE_GET(objid)  							\
    ((objid >> 32) & 0xFF)

#define CTC_SAI_PORT_GET_LPORT(objid)  							\
    CTC_MAP_GPORT_TO_LPORT(objid & 0xFFFFFFFF)

#define CTC_SAI_OBJECT_INDEX_GET(objid)  							\
    (objid & 0xFFFFFFFF)

#define SAI_SET_FLAG(VAL,FLAG)          (VAL) = (VAL) | (FLAG)
#define SAI_UNSET_FLAG(VAL,FLAG)        (VAL) = (VAL) & ~(FLAG)
#define SAI_FLAG_ISSET(VAL,FLAG)        (((VAL) & (FLAG)) == (FLAG))

#define SAI_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1: 0)
#define SAI_SET_BIT(flag, bit)      (flag) = (flag) | (1 << (bit))
#define SAI_CLEAR_BIT(flag, bit)    (flag) = (flag) & (~(1 << (bit)))

#define SAI_VLAN_MAX                 4094              /* Max VLANs.   */
#define SAI_VLAN_DEFAULT_VID         1                 /* Default VID. */

/* List iteration macro. */
#define CTC_SAI_SLIST_LOOP(L,V,N) \
  if (L) \
    for ((N) = (L)->head; (N); (N) = (N)->next) \
      if (((V) = (N)->data) != NULL)

/* List iteration macro. 
 * It allows to delete N and V in the middle of the loop 
 */
#define CTC_SAI_SLIST_LOOP_DEL(L,V,N,NN) \
  if (L) \
    for ((N) = (L)->head, NN = ((N)!=NULL) ? (N)->next : NULL;  \
         (N);                                                   \
         (N) = (NN), NN = ((N)!=NULL) ? (N)->next : NULL)       \
      if (((V) = (N)->data) != NULL)
        

#define CTC_SAI_PTR_VALID_CHECK(ptr)    							\
	{                                       						\
		if(NULL == ptr)                     						\
			return SAI_STATUS_INVALID_PARAMETER;    				\
	}

#define CTC_SAI_ERROR_GOTO(op, ret, label) 							\
    { 																\
        ret = (op); 												\
        if (ret != SAI_STATUS_SUCCESS) 								\
        { 															\
            goto label; 											\
        } 															\
    }

#define CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(op, ret, label) 			\
    { 																\
        ret = (op); 												\
        if (ret != SAI_STATUS_SUCCESS) 								\
        { 															\
            ret = ctc_sai_get_error_from_sdk_error(ret);            \
            goto label; 											\
        } 															\
    }

#define CTC_SAI_GET_PORT_BY_QUEUE_ID(queue_id) \
        ((queue_id >> 16) &0xffff)

#define CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(queue_id) \
        (queue_id &0xffff)
 
static inline int __refcnt_add_unless(int32_t *v, int32_t a, int32_t u)
{
    int32_t ret;

	ret = *v;
	if (ret != u)
		*v += a;

	return ret;
}

static inline int refcnt_add_unless(int32_t *v, int32_t a, int32_t u)
{
	return __refcnt_add_unless(v, a, u) != u;
}


#define refcnt_inc(v)			v->ref++
#define refcnt_dec(v)			v->ref--

#define refcnt_inc_and_test(v)  (++v->ref == 0)

#define refcnt_sub_and_test(i, v) ((v->ref-=i),v->ref == 0)
#define refcnt_dec_and_test(v)  (--v->ref == 0)

#define refcnt_inc_not_zero(v) refcnt_add_unless(&(v->ref), 1, 0)


static inline bool is_zero_ether_addr(const uint8_t *addr)
{
    return (*(const uint16_t *)(addr + 0) |
        *(const uint16_t *)(addr + 2) |
        *(const uint16_t *)(addr + 4)) == 0;
}

static inline bool is_multicast_ether_addr(const uint8_t *addr)
{
    return 0x01 & addr[0];
}

static inline bool is_broadcast_ether_addr(const uint8_t *addr)
{
    return (*(const uint16_t *)(addr + 0) &
        *(const uint16_t *)(addr + 2) &
        *(const uint16_t *)(addr + 4)) == 0xffff;
}

static inline bool is_unicast_ether_addr(const uint8_t *addr)
{
    return !is_multicast_ether_addr(addr);
}

static inline bool is_valid_ether_addr(const uint8_t *addr)
{
    return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

uint16 ip_fast_csum(const void *iph, unsigned int ihl);

sai_status_t
ctc_sai_get_error_from_sdk_error(int32_t);

sai_status_t
ctc_sai_get_cut_through_en();

#if 1
#define SAI_CALL_CTC(_op, _ret) \
{ \
    _ret = (_op); \
}
#else
#define SAI_CALL_CTC(_op, _ret) \
{ \
    if (wb_restoring) \
    { \
        _ret = 0;\
    } \
    else \
    { \
        _ret = (_op); \
    } \
}
#endif

#define SAI_CALL_CTC_ERR_GOTO(op, ret, label) 					    \
    { 																\
        SAI_CALL_CTC(op, ret);									    \
        if (ret != SAI_STATUS_SUCCESS) 								\
        { 															\
            goto label; 											\
        } 															\
    }

#endif

