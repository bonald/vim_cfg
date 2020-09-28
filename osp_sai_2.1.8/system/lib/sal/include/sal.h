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

#ifndef __SAL_H__
#define __SAL_H__

#include "sal_types.h"

/**
 * @file sal.h
 */
 
#define _SAL_DEBUG

#if defined(_SAL_LINUX_KM)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <arpa/inet.h>
#elif defined(_SAL_LINUX_UM)
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <semaphore.h>
#include <errno.h>
#include <stropts.h>
#include <sched.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
/* IF_SPLITTING */
#include <assert.h>
#elif defined(_SAL_VXWORKS)
#endif

/* SDK sal sub headers copied to osp sal include subdir. */
#include "sal_error.h"
#include "sal_log.h"
#include "sal_mem.h"
#include "sal_task.h"
#include "sal_event.h"
#include "sal_mutex.h"
#include "sal_timer.h"
#include "sal_intr.h"
#include "sal_file.h"

/* OSP sal sub headers. */
#include "sal_async_queue.h"
#include "sal_socket.h"
#include "sal_util.h"
#include "sal_base64.h"
#include "sal_assert.h"

#define sal_waitpid waitpid
#define sal_getpid getpid

#define sal_fcntl fcntl
#define sal_feof feof
#define sal_ftell ftell
#define sal_rewind rewind
#define sal_symlink symlink
#define sal_fopen fopen
#define sal_fclose fclose
#define sal_fgets fgets
#define sal_fputs fputs
#define sal_fprintf fprintf
#define sal_fwrite fwrite
#define sal_fflush fflush
#define sal_unlink unlink
#define sal_rename rename
#define sal_mkdir mkdir
#define sal_rmdir rmdir
#define sal_fgetc fgetc
#define sal_fputc fputc
#define sal_stat stat
#define sal_fread   fread
#define sal_fwrite   fwrite
#define sal_S_ISDIR S_ISDIR
#define sal_fseek fseek
#define SAL_SEEK_SET SEEK_SET
#define SAL_SEEK_CUR SEEK_CUR
#define SAL_SEEK_END SEEK_END

#define sal_open        open
#define sal_close       close
#define sal_memcpy      memcpy
#define sal_memset      memset
#define sal_memcmp      memcmp
#define sal_memmove     memmove
#define sal_printf      printf
#define sal_vprintf     vprintf
#define sal_sprintf     sprintf
#define sal_sscanf      sscanf
#define sal_strcpy      strcpy
#define sal_strncpy     strncpy
#define sal_strcat      strcat
#define sal_strncat     strncat
#define sal_strcmp      strcmp
#define sal_strncmp     strncmp
#define sal_strlen      strlen
#define sal_fprintf     fprintf
#define sal_strtok      strtok
#define sal_strtok_r    strtok_r
#define sal_tolower     tolower
#define sal_toupper     toupper
#define sal_strchr      strchr
#define sal_strstr      strstr
#define sal_strrchr     strrchr
#define sal_strspn      strspn
#define sal_strerror    strerror
#define sal_strtok      strtok
#define sal_strtol      strtol
#define sal_strtok_r    strtok_r
#define sal_tolower     tolower
#define sal_toupper     toupper
#define sal_isspace     isspace
#define sal_isdigit     isdigit
#define sal_isxdigit    isxdigit
#define sal_isalpha     isalpha
#define sal_isalnum     isalnum
#define sal_isupper     isupper
#define sal_islower     islower
#define sal_isprint     isprint
#define sal_ntohl       ntohl
#define sal_htonl       htonl
#define sal_ntohs       ntohs
#define sal_htons       htons
#define sal_snprintf    snprintf
#define sal_vsnprintf   vsnprintf
#define sal_vsprintf    vsprintf
#define sal_strcasecmp  strcasecmp
#define sal_strncasecmp strncasecmp
#define sal_inet_pton   inet_pton
#define sal_inet_ntop   inet_ntop
#define sal_timeval     timeval
#define sal_timezone    timezone
#define sal_tm          tm
#define sal_gmtime      gmtime
#define sal_time        time
#define sal_ctime       ctime
#define sal_poll        poll
#define sal_pollfd      pollfd
#define sal_realloc     realloc
#define sal_strdup      strdup
#define sal_char_isprint isprint
#define sal_atoi atoi
#define sal_assert assert
#define sal_clock_gettime  clock_gettime
#define SAL_CLOCK_MONOTONIC CLOCK_MONOTONIC
#define sal_char_isalpha isalpha
#define sal_char_isdigit isdigit
#define sal_char_tolower tolower
#define sal_time_mk mktime
#define sal_strtos32(x, y, z) strtol((char*)x, (char**)y, z)
#define sal_strtou32(x, y, z) strtoul((char*)x, (char**)y, z)
#define sal_strtou64(x, y, z) strtoull((char*)x, (char**)y, z)

#define sal_system      system
#define sal_settimeofday settimeofday
#define sal_rand         rand

#define sal_time_strf strftime
#define sal_localtime localtime
#define sal_fd_set_t        fd_set
#define SAL_FD_SETSIZE      FD_SETSIZE
#define SAL_FD_ISSET(h,s)   FD_ISSET(h,s)
#define SAL_FD_SET(h,s)     FD_SET(h,s)
#define SAL_FD_CLR(h,s)     FD_CLR(h,s)

#ifndef sal_clock_t
typedef clock_t sal_clock_t; 
#endif

#ifndef sal_time_clock
#define sal_time_clock clock
#endif

/* IF_SPLITTING */
#define sal_assert assert
 
#define SAL_FILE            FILE
#define SAL_OPEN_RO         "r"
#define SAL_OPEN_RW         "w+"
#define SAL_OPEN_APPEND     "a+"

#define sal_char_isspace isspace

#endif /* !__SAL_H__ */
