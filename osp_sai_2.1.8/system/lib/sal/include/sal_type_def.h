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

#ifndef __SAL_TYPE_DEF_H__
#define __SAL_TYPE_DEF_H__
/**
 * @defgroup types Basic Types
 * @{
 */
#ifndef _BASIC_DATA_TYPE
#define _BASIC_DATA_TYPE
typedef int8_t      int8;   /**< 8-bit  signed integer */
typedef int16_t     int16;  /**< 16-bit signed integer */
typedef int32_t     int32;  /**< 32-bit signed integer */
typedef int64_t     int64;  /**< 64-bit signed integer */
typedef uint8_t     uint8;  /**< 8-bit  unsigned integer */
typedef uint16_t    uint16; /**< 16-bit unsigned integer */
typedef uint32_t    uint32; /**< 32-bit unsigned integer */
typedef uint64_t    uint64; /**< 64-bit unsigned integer */
typedef long    ptr_int;
typedef ulong   ptr_uint;
typedef float float32; /**< 32-bit signed float */
typedef double float64; /**< 64-bit signed float */
#ifndef bool
typedef int32_t     bool;   /**< Boolean type */
#endif
typedef uint8 mac_addr_t[6];/**< mac address define */
typedef uint32 ip_addr_t; /**< ip address define */
typedef uint32 ipv6_addr_t[4]; /**< ipv6 address define */
#endif /* !_BASIC_DATA_TYPE */

/**@}*/ /* End of @defgroup types */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define RESULT_OK                   0
#define RESULT_ERROR                -1

#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __inline
#elif __GNUC__
#define INLINE __inline__
#else
#define INLINE
#endif
#endif
#endif /*!__SAL_TYPE_DEF_H__*/
