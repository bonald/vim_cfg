#ifndef __SAL_H__
#define __SAL_H__

/**
 * @file kal.h
 */

#include "sal_types.h"

#define _SAL_DEBUG
#if defined(_SAL_LINUX_KM)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#elif defined(_SAL_LINUX_UM)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <ctype.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <netpacket/packet.h>
#include <sys/un.h>
#include <time.h>
#include <signal.h>
#elif defined(_SAL_VXWORKS)
#include <vxWorks.h>
#include <taskLib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/times.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <timers.h>
#include <sockLib.h>
#include <inetLib.h>
#include <time.h>
#if VX_VERSION >= 66
#include <spinLockLib.h>
#endif
#define _SDK_NOT_READLINE_
#endif

#include "sal_error.h"
#include "sal_log.h"
#include "sal_mem.h"
#include "sal_task.h"
#include "sal_event.h"
#include "sal_mutex.h"
#include "sal_timer.h"
#include "sal_intr.h"
#include "sal_slab.h"
#include "sal_memmngr.h"
#include "sal_sock.h"
#include "sal_file.h"
#include "sal_lock.h"

#define CTC_SAL_DEBUG_OUT(text, ...) sal_printf(text, ##__VA_ARGS__)

#define BOOLEAN_BIT(b) ((b) ? 1 : 0)



#undef sal_memcmp
#define sal_memcmp   memcmp

#undef sal_memmove
#define sal_memmove  memmove
/*string*/
#undef sal_vprintf
#define sal_vprintf vprintf

#undef sal_sprintf
#define sal_sprintf sprintf

#undef sal_sscanf
#define sal_sscanf sscanf

#undef sal_strcpy
#define sal_strcpy strcpy

#undef sal_strncpy
#define sal_strncpy strncpy

#undef sal_strcat
#define sal_strcat strcat

#undef sal_strncat
#define sal_strncat strncat

#undef sal_strcmp
#define sal_strcmp strcmp

#undef sal_strncmp
#define sal_strncmp strncmp

#undef sal_strlen
#define sal_strlen strlen

#undef sal_snprintf
#define sal_snprintf snprintf

#undef sal_vsnprintf
#define sal_vsnprintf vsnprintf

#undef sal_vsprintf
#define sal_vsprintf vsprintf

#undef sal_strtos32
#undef sal_strtou32
#undef sal_strtou64
#undef sal_atoi
#undef sal_strtol
#undef sal_strtol
#if defined(_SAL_LINUX_KM)
#define sal_strtou32(x, y, z) simple_strtoul((char*)x, (char**)y, z)
#define sal_strtos32(x, y, z) simple_strtol((char*)x, (char**)y, z)
#define sal_strtou64(x, y, z) simple_strtoull((char*)x, (char**)y, z)
#define sal_atoi(x) simple_strtol((char*)x, NULL, 10)
#define sal_strtol(x, y, z) simple_strtol((char*)x, (char**)y, z)
#define sal_strtok(x, y) strsep((char**)&x, y)


#undef sal_fprintf
#define sal_fprintf ctc_sal_fprintf

int ctc_sal_get_rand();
#undef sal_rand
#define sal_rand ctc_sal_get_rand

void ctc_sal_srand(unsigned int seed);
#undef sal_srand
#define sal_srand ctc_sal_srand

extern void ctc_sal_free(void *p);
extern void *ctc_sal_malloc(size_t size);
extern void *ctc_sal_calloc(size_t size);
#define sal_free ctc_sal_free
#define sal_malloc ctc_sal_malloc
#define sal_calloc ctc_sal_calloc
#define sal_realloc ctc_sal_realloc

#define sal_time ctc_sal_time
#define sal_ctime ctc_sal_ctime
#else
#define sal_atoi atoi
#define sal_strtos32(x, y, z) strtol((char*)x, (char**)y, z)
#define sal_strtou32(x, y, z) strtoul((char*)x, (char**)y, z)
#define sal_strtol strtol

#undef sal_rand
#define sal_rand rand

#undef sal_srand
#define sal_srand srand

/*memory */
#undef sal_malloc
#define sal_malloc   malloc

#undef sal_realloc
#define sal_realloc realloc

#undef sal_free
#define sal_free   free

#undef sal_time
#define sal_time time

#undef sal_ctime
#define sal_ctime ctime

#undef sal_strtok
#define sal_strtok strtok

#endif

#if defined (_SAL_LINUX_UM)
#define sal_strtou64(x, y, z) strtoull((char*)x, (char**)y, z)
#elif defined (_SAL_VXWORKS)
extern uint64 ctc_sal_strtoull(const char *ptr, char **end, int base);
#define sal_strtou64(x, y, z) ctc_sal_strtoull((char*)x, (char**)y, z)
#endif

#undef sal_strchr
#define sal_strchr strchr

#undef sal_strstr
#define sal_strstr strstr

#undef sal_strrchr
#define sal_strrchr strrchr

#undef sal_strspn
#define sal_strspn strspn

#undef sal_strerror
#define sal_strerror strerror

#undef sal_strsep
#define sal_strsep strsep

#undef sal_strtok_r
#define sal_strtok_r strtok_r

#undef sal_tolower
#undef sal_toupper
#define sal_tolower tolower
#define sal_toupper toupper

#undef sal_isspace
#undef sal_isdigit
#undef sal_isxdigit
#undef sal_isalpha
#undef sal_isalnum
#undef sal_isupper
#undef sal_islower
#define sal_isspace isspace
#define sal_isdigit isdigit
#define sal_isxdigit isxdigit
#define sal_isalpha isalpha
#define sal_isalnum isalnum
#define sal_isupper isupper
#define sal_islower islower
#define sal_isprint isprint

#undef sal_ntohl
#undef sal_htonl
#undef sal_ntohs
#undef sal_htons

#define sal_ntohl ntohl
#define sal_htonl htonl
#define sal_ntohs ntohs
#define sal_htons htons

#define sal_printf  ctc_sal_print_func

#define sal_eadp_printf(enable,FMT, ...)\
        if(enable)\
        {\
            ctc_sal_print_func(FMT, ##__VA_ARGS__);\
        }\


#undef sal_memcpy
#undef sal_memset

#if MEM_RUNTIME_CHECK
#define sal_memcpy(a,b,c)    memcpy(a,b,c); if(mem_mgr_ptr_check(a)) sal_printf("sal_memcpy Error Happened!! Fun:%s()  Line:%d\n",__FUNCTION__,__LINE__);
#define sal_memset(a,b,c)    memset(a,b,c); if(mem_mgr_ptr_check(a)) sal_printf("sal_memset Error Happened!! Fun:%s()  Line:%d\n",__FUNCTION__,__LINE__);
#else
#define sal_memcpy    memcpy
#define sal_memset    memset
#endif


#ifdef _SAL_LINUX_UM

#undef sal_strcasecmp
#define sal_strcasecmp strcasecmp

#undef sal_strncasecmp
#define sal_strncasecmp strncasecmp

#undef sal_inet_pton
#define sal_inet_pton inet_pton

#undef sal_inet_ntop
#define sal_inet_ntop inet_ntop

#undef sal_ioctl
#define sal_ioctl ioctl

#elif defined (_SAL_LINUX_KM) || defined (_SAL_VXWORKS)
extern const char* ctc_sal_inet_ntop(int32 af, void* src, char* dst, uint32 size);
extern int ctc_sal_inet_pton(int32 af, const char* src, void* dst);
extern int ctc_sal_strcasecmp(const char* s1, const char* s2);
extern int ctc_sal_strncasecmp(const char* s1, const char* s2, int32 n);

#define sal_inet_ntop ctc_sal_inet_ntop
#define sal_inet_pton ctc_sal_inet_pton
#define sal_strcasecmp ctc_sal_strcasecmp
#define sal_strncasecmp ctc_sal_strncasecmp
#endif


#ifdef _SAL_LINUX_KM
extern void ctc_sal_qsort(void *const pbase, size_t total_elems, size_t size, int (*cmp)(const void*, const void*));
#define sal_qsort ctc_sal_qsort

#elif defined (_SAL_LINUX_UM) || defined (_SAL_VXWORKS)

#undef sal_qsort
#define sal_qsort qsort
#endif

#define SET_BIT(flag, bit)      (flag) = (flag) | (1 << (bit))
#define CLEAR_BIT(flag, bit)    (flag) = (flag) & (~(1 << (bit)))
#define IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)

#define SET_BIT_RANGE(dst, src, s_bit, len) \
    { \
        uint8 i = 0; \
        for (i = 0; i < len; i++) \
        { \
            if (IS_BIT_SET(src, i)) \
            { \
                SET_BIT(dst, (s_bit + i)); \
            } \
            else \
            { \
                CLEAR_BIT(dst, (s_bit + i)); \
            } \
        } \
    }

#ifdef _SAL_VXWORKS
#define PTR_TO_INT(x)       ((uint32)(((uint32)(x)) & 0xFFFFFFFF))
#define INT_TO_PTR(x)       ((void*)(uint32)(x))

#if 0
struct in6_addr
{
    union
    {
        uint8       u6_addr8[16];
        uint16      u6_addr16[8];
        uint32      u6_addr32[4];
    }
    in6_u;
#define s6_addr         in6_u.u6_addr8
#define s6_addr16       in6_u.u6_addr16
#define s6_addr32       in6_u.u6_addr32
};
#endif

#ifndef AF_INET6
#define AF_INET6    10  /* IP version 6 */
#endif

#endif

#ifdef _SAL_LINUX_KM
#ifndef AF_INET6
#define AF_INET6    10  /* IP version 6 */
#endif

#ifndef AF_INET
#define AF_INET    9  /* IP version 4 */
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern sal_err_t ctc_sal_init(void);
extern void ctc_sal_deinit(void);
#define sal_init ctc_sal_init
#define sal_deinit ctc_sal_deinit

/* sal_file define */
#if defined(_SAL_LINUX_KM)
#define sal_open ctc_sal_open
#define sal_fopen ctc_sal_fopen
#define sal_fclose ctc_sal_fclose
#define sal_read ctc_sal_read
#define sal_write ctc_sal_write
#define sal_fgets ctc_sal_fgets
#define sal_eof ctc_sal_eof
#define sal_fseek ctc_sal_fseek
#define sal_ftell ctc_sal_ftell
#define sal_fwrite(buff, len, count, ft)    ctc_sal_write(ft, buff, (count)*(len))
#define sal_fread(buff, len, count, ft)    ctc_sal_read(ft, buff, (count)*(len))
#define sal_feof ctc_sal_eof
#else
/* linux user and vxworks file operation */
#undef sal_open
#define sal_open open

#undef sal_close
#define sal_close close

#undef sal_fopen
#define sal_fopen fopen

#undef sal_fclose
#define sal_fclose fclose

#undef sal_read
#define sal_read read

#undef sal_write
#define sal_write write

#undef sal_fread
#define sal_fread fread

#undef sal_fwrite
#define sal_fwrite fwrite

#undef sal_fprintf
#define sal_fprintf fprintf

#undef sal_fgets
#define sal_fgets fgets

#undef sal_fseek
#define sal_fseek fseek

#undef sal_ftell
#define sal_ftell ftell

#undef sal_feof
#define sal_feof feof

#endif

/* sal_mutex define */
#define sal_sem_create ctc_sal_sem_create
#define sal_sem_destroy ctc_sal_sem_destroy
#define sal_sem_take ctc_sal_sem_take
#define sal_sem_give ctc_sal_sem_give
#define sal_mutex_create ctc_sal_mutex_create
#define sal_mutex_destroy ctc_sal_mutex_destroy
#define sal_mutex_lock ctc_sal_mutex_lock
#define sal_mutex_unlock ctc_sal_mutex_unlock
#define sal_mutex_try_lock ctc_sal_mutex_try_lock

/* sal_sock define */
#define sal_sock_server_init ctc_sal_sock_server_init
#define sal_sock_client_init ctc_sal_sock_client_init
#define sal_sock_server_send ctc_sal_sock_server_send
#define sal_sock_client_send ctc_sal_sock_client_send
#define sal_sock_register_recv_cb ctc_sal_sock_register_recv_cb

/* sal_task define */
#define sal_task_create ctc_sal_task_create
#define sal_task_destroy ctc_sal_task_destroy
#define sal_task_exit ctc_sal_task_exit
#define sal_task_sleep ctc_sal_task_sleep
#define sal_task_yield ctc_sal_task_yield
#define sal_gettime ctc_sal_gettime
#define sal_getuptime ctc_sal_getuptime
#define sal_udelay ctc_sal_udelay
#define sal_delay ctc_sal_delay
#define sal_task_set_priority ctc_sal_task_set_priority

/* sal_timer define */
#define sal_timer_init ctc_sal_timer_init
#define sal_timer_fini ctc_sal_timer_fini
#define sal_timer_create ctc_sal_timer_create
#define sal_timer_destroy ctc_sal_timer_destroy
#define sal_timer_start ctc_sal_timer_start
#define sal_timer_stop ctc_sal_timer_stop

/* sal_spinlock define */
#define sal_spinlock_create ctc_sal_spinlock_create
#define sal_spinlock_destroy ctc_sal_spinlock_destroy
#define sal_spinlock_lock ctc_sal_spinlock_lock
#define sal_spinlock_unlock ctc_sal_spinlock_unlock

#ifdef __cplusplus
}
#endif

#endif /* !__SAL_H__ */

