/**
 @file glb_macro.h
 
 @date 2010-07-01
 
 @version v1.0
 
The file define  all CNOS module's  common Macros .

*/

#ifndef __GLB_MACRO_H__
#define __GLB_MACRO_H__

/* alignment with 4 bytes */
#define GLB_ALIGN(n) (((n) + 3) & ~3)

#define GLB_IS_NULL_PTR(_PTR)        ((_PTR) ? 1 : 0)

#define GLB_DEC(_COUNT) \
do { \
    if ((_COUNT) > 0) \
    { \
        (_COUNT)--; \
    } \
} while (0)

/**
 @brief define bit operations  
*/
#define GLB_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1: 0)
#define GLB_SET_BIT(flag, bit)      (flag) = (flag) | (1 << (bit))
#define GLB_CLEAR_BIT(flag, bit)    (flag) = (flag) & (~(1 << (bit)))

#define GLB_SET_FLAG(VAL,FLAG)          (VAL) = (VAL) | (FLAG)
#define GLB_UNSET_FLAG(VAL,FLAG)        (VAL) = (VAL) & ~(FLAG)
#define GLB_FLAG_ISSET(VAL,FLAG)        (((VAL) & (FLAG)) == (FLAG))

#define GLB_SET_FLAG_COND(COND, VAL, FLAG) \
if (COND) \
{ \
    GLB_SET_FLAG((VAL), (FLAG)); \
} \
else \
{ \
    GLB_UNSET_FLAG((VAL), (FLAG)); \
}

#define GLB_SET_BIT_COND(COND, flag, bit) \
if (COND) \
{ \
    GLB_SET_BIT(flag, bit); \
} \
else \
{ \
    GLB_CLEAR_BIT(flag, bit); \
}
            
/* added by chenxw for bitmap operation, especially for 32+bit offset*/
/* define bitmap array operation */
 
/* internal used */
#define GLB_UINT32_BITS     32

#define _GLB_BMP_OP(_bmp, _offset, _op)     \
    (_bmp[(_offset) / GLB_UINT32_BITS] _op (1U << ((_offset) % GLB_UINT32_BITS)))
 
/* bmp should be a array of uint32, e.g. uint32 bmp[2] */
 
/* init bitmap */
#define GLB_BMP_INIT(bmp)                   \
    sal_memset((bmp), 0, sizeof((bmp)));

/* check bitmap */
#define GLB_BMP_OFFSET_CHECK(bmp, offset)   \
    ((0 <= offset) && ((offset) < GLB_BMP_BITS(bmp)))

/* set bit */
#define GLB_BMP_SET(bmp, offset)            \
    _GLB_BMP_OP((bmp), (offset), |=)
 
/* unset bit */
#define GLB_BMP_UNSET(bmp, offset)          \
    _GLB_BMP_OP((bmp), (offset), &= ~)
 
/* get bit set */
#define GLB_BMP_ISSET(bmp, offset)          \
    _GLB_BMP_OP((bmp), (offset), &)
 
/* get bit count */
#define GLB_BMP_BITS(bmp)                   \
    ((GLB_UINT32_BITS * sizeof((bmp)) / sizeof(uint32)))
 
/* bit iterate begin */
#define GLB_BMP_ITER_BEGIN(bmp, offset)                             \
do {                                                                \
    for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)  \
        if (GLB_BMP_ISSET((bmp), (offset)))

/* bit iterate begin */
#define GLB_BMP_TRAV_BEGIN(bmp, offset)                             \
do {                                                                \
    for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)  \
 

/* bit iterate end */
#define GLB_BMP_ITER_END(bmp, offset)       \
} while (0)

#define GLB_BMP_CHECK_ALL_ZERO(bmp, all_zero) \
do { \
    int32 _offset = 0; \
    all_zero = TRUE; \
    for (_offset = 0; _offset < sizeof((bmp)) / sizeof(uint32); _offset++) \
        if (0 != bmp[_offset]) \
        { \
            all_zero = FALSE; \
            break; \
        } \
} while (0)

#define GLB_BMP_CHECK_COMMUNITY_GROUP_IS_FULL(bmp, full) \
do { \
    int32 _offset = 0; \
    full = TRUE; \
    for (_offset = 0; _offset < 16; _offset++) \
        if (!GLB_BMP_ISSET(bmp, _offset)) \
        { \
            full = FALSE; \
            break; \
        } \
} while (0)

/**
 @brief define mcast addr check  
*/
#define GLB_IS_MCAST_MAC(mac) (mac[0] & 0x01)
#define GLB_IS_BCAST_MAC(mac) (*(uint32*)mac == 0xFFFFFFFF && *(uint16*)&mac[4] == 0xFFFF)
#define GLB_IS_ZERO_MAC(mac)  (0 == (mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]))



#define GLB_IS_ZERO_IPV6_MASK(value) (0 == (value[0]+value[1]+value[2]+value[3]+value[4]+        \
                                            value[5]+value[6]+value[7]+value[8]+value[9]+        \
                                            value[10]+value[11]+value[12]+value[13]+value[14]+   \
                                            value[15]))


#define GLB_IS_FULL_IPV6_MASK(value, flag)         \
do {                                         \
    int32 i;                                 \
    for(i = 0; i < sizeof(value); i++)       \
    {                                        \
        if (value[i] == 0xFF)                \
            flag++;                       \
    }                                        \
}while(0)


#define GLB_PTR_VALID_CHECK(ptr, err_code)    \
        if ( NULL == (ptr) )                      \
        {                                           \
            return err_code;                      \
        }

#define GLB_PTR_VALID_CHECK_RETURN_NULL(ptr)    \
        if ( NULL == (ptr) )                      \
        {                                           \
            return;                      \
        }

#define GLB_MEM_PTR_CHECK(ptr, err_code)\
        if ( NULL == (ptr) )\
        {\
            return err_code;\
        }

#define GLB_MAX_VALUE_CHECK(var,max_value, err_code) \
        if((var) > (max_value))\
        {\
            return err_code;\
        }

#define GLB_EQUAL_CHECK(var, value, err_code) \
do { \
        if((var) != (value)) \
        { \
            return err_code; \
        } \
} while(0)

#define GLB_CTC_SWAP16(val) \
        ((uint16)( \
                (((uint16)(val) & (uint16)0x00ffU) << 8) | \
                (((uint16)(val) & (uint16)0xff00U) >> 8) ))

#define GLB_CTC_SWAP32(val) \
        ((uint32)( \
                (((uint32)(val) & (uint32)0x000000ffUL) << 24) | \
                (((uint32)(val) & (uint32)0x0000ff00UL) <<  8) | \
                (((uint32)(val) & (uint32)0x00ff0000UL) >>  8) | \
                (((uint32)(val) & (uint32)0xff000000UL) >> 24) ))

#define UINT64_TO_OCTET_STR(data, str)                              \
do{                                                                 \
    int32 i;                                                        \
    str.buf = sal_malloc (sizeof(uint64));\
    str.size = sizeof(uint64);                                      \
    for(i = 0; i < sizeof(uint64); i++)                             \
    {                                                               \
        str.buf[i] = (uint8)((data >> (i*8))&0xFF);                 \
    }                                                               \
}while(0)

#define OCTET_STR_TO_UINT64(data, str)                              \
do{                                                                 \
    int32 i;                                                        \
    data = 0;                                                       \
    for(i = 0; i < str.size; i++)                                   \
    {                                                               \
        data += ((uint64)str.buf[i]) << (i*8);                      \
    }                                                               \
}while(0)

#define FREE_OCTET_STR(str) sal_free(str.buf)

/*Added by yanxa, for printing int64/uint64 variables in syslog/console */
/* 2^64 - 1 = 18,446,744,073,709,551,615 */
#define GLB_INT64_DEC_LENGTH    24
#define INT64_TO_STR(data, str)                                     \
do{                                                                 \
    sal_memset(str, 0x00, GLB_INT64_DEC_LENGTH);                    \
    sal_snprintf(str, GLB_INT64_DEC_LENGTH, "%"PRId64, data);          \
}while(0)

#define UINT64_TO_STR(data, str)                                    \
do{                                                                 \
    sal_memset(str, 0x00, GLB_INT64_DEC_LENGTH);                    \
    sal_snprintf(str, GLB_INT64_DEC_LENGTH, "%"PRIu64, data);          \
}while(0)

#define CTC_BUILD_ASSERT(EXPR)                                          \
    extern int (*build_assert(void))[ sizeof(struct {               \
           unsigned int build_assert_failed : (EXPR) ? 1 : -1; })]

#define CTC_ETC_BASE_DIR            "/switch"

#define SDB_NODE_BASE       6       /* CDB_NODE_MAX+1 */

#endif /* __GLB_MACRO_H__*/
