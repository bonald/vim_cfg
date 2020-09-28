#ifndef __SAL_TYPE__H__
#define __SAL_TYPE__H__

/**
 * @file kal.h
 */

/*
#if !defined(_SAL_LINUX_KM) && !defined(_SAL_LINUX_UM)
#define _SAL_LINUX_UM
#endif
*/
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(SDK_IN_KERNEL)
#undef _SAL_LINUX_KM
#define _SAL_LINUX_KM
#elif defined(SDK_IN_USERMODE)
#undef _SAL_LINUX_UM
#define _SAL_LINUX_UM
#elif defined(SDK_IN_VXWORKS)
#undef _SAL_VXWORKS
#define _SAL_VXWORKS
#endif

#define _SAL_DEBUG
#if defined(_SAL_LINUX_KM)
#include <linux/kernel.h>

#elif defined(_SAL_LINUX_UM)
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#elif defined(_SAL_VXWORKS)
#include <vxWorks.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#if defined(_SAL_LINUX_UM)
typedef int8_t      int8;   /**< 8-bit  signed integer */
typedef int16_t     int16;  /**< 16-bit signed integer */
typedef int32_t     int32;  /**< 32-bit signed integer */
typedef int64_t     int64;  /**< 64-bit signed integer */
typedef uint8_t     uint8;  /**< 8-bit  unsigned integer */
typedef uint16_t    uint16; /**< 16-bit unsigned integer */
typedef uint32_t     uint32; /**< 32-bit unsigned integer */
typedef uint64_t    uint64; /**< 64-bit unsigned integer */
typedef float       float32; /**< 32-bit signed float */
typedef double      float64; /**< 64-bit signed float */
typedef time_t sal_time_t;
#ifndef __cplusplus
typedef int         bool;   /**< Boolean type */
#endif
#define sal_time_sys_current(z) time(z)
#if(__WORDSIZE == 64)
typedef uint64    uintptr; /**< uintptr define */
typedef int64    intptr; /**< intptr define */
#else
typedef uint32   uintptr; /**< uintptr define */
typedef int32    intptr; /**< intptr define */
#endif

#elif defined(_SAL_LINUX_KM)
typedef signed char           int8;   /**< 8-bit  signed integer */
typedef signed short          int16;  /**< 16-bit signed integer */
typedef signed int            int32;  /**< 32-bit signed integer */
typedef signed long long      int64;  /**< 64-bit signed integer */
typedef unsigned char  uint8;  /**< 8-bit  unsigned integer */
typedef unsigned short uint16; /**< 16-bit unsigned integer */
typedef unsigned int   uint32; /**< 32-bit unsigned integer */
typedef unsigned long long  uint64; /**< 64-bit unsigned integer */
typedef __kernel_time_t sal_time_t;
#ifndef bool
//typedef int            bool;   /**< Boolean type */
#endif
typedef float          float32; /**< 32-bit signed float */
typedef double         float64; /**< 64-bit signed float */
#if defined(CONFIG_RESOURCES_64BIT) || defined(CONFIG_PHYS_ADDR_T_64BIT)
typedef  long long  intptr;
typedef  unsigned long long uintptr;
#define  PRIu64  "llu"
#define  PRIx64  "llx"
#define  PRId64  "lld"
#else
typedef  int  intptr;
typedef  unsigned int uintptr;
#define  PRIu64  "lu"
#define  PRIx64  "lx"
#define  PRId64  "ld"
#endif

#elif defined(_SAL_VXWORKS)
typedef signed char    int8;   /**< 8-bit  signed integer */
typedef signed short   int16;  /**< 16-bit signed integer */
typedef signed int     int32;  /**< 32-bit signed integer */
typedef signed long long  int64;  /**< 64-bit signed integer */
typedef unsigned char  uint8;  /**< 8-bit  unsigned integer */
typedef unsigned short uint16; /**< 16-bit unsigned integer */
typedef unsigned int   uint32; /**< 32-bit unsigned integer */
typedef unsigned long long  uint64; /**< 64-bit unsigned integer */
#ifndef bool
typedef int            bool;   /**< Boolean type */
#endif
typedef float          float32; /**< 32-bit signed float */
typedef double         float64; /**< 64-bit signed float */
#define  PRIu64  "llu"
#define  PRIx64  "llx"
#define  PRId64  "lld"
#endif


typedef uint32  hw_mac_addr_t[2];
typedef uint8   mac_addr_t[6];
typedef uint32  ip_addr_t;
typedef uint32  ipv6_addr_t[4];


#undef TRUE
#ifndef TRUE
#define TRUE 1
#endif

#undef FALSE
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __inline
#elif __GNUC__
#define INLINE __inline__
#else
#define INLINE
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* !__SAL_TYPE__H__ */

