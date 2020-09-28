#ifndef __PARAM_CHECK_H__
#define __PARAM_CHECK_H__

#include <sys/socket.h>
#include <netdb.h>

#define MAXHOSTNAMELEN              64

/* username */
#define M_MAX_USERNAME_LEN      32
#define M_USERNAME_LIMIT_STR    "can only include [0-9a-zA-Z.-_], and must starting with alphabetic. Its length range is [1, 32)."
#define M_MAX_USER_COUNTER 256

/* password length limit */
#define M_MAX_PASSWORD_LEN      64
#define M_PASSWORD_BUFSIZE      256
#define M_PASSWORD_LIMIT_STR    "can only include printable characters and its length range is [1, 64)."
#define M_USER_PASSWORD_LIMIT_STR    "can only include [0-9a-zA-Z.-_] characters and its length range is [1, 64)."
#define M_PASSWD_LIMIT_STR    "can only include 0-9A-Za-z~!@#^&*()_+-={}[]|:;<>,. characters and its length range is [1, 64)."

/* password process flags */
#define E_PASSWORD_ENCRYPT      0x1
#define E_PASSWORD_DECRYPT      0x0

/* file size unit enum */
typedef enum 
{
    NONE = 0, 
    KILO = 1, 
    MEGA = 2, 
    GIGA = 3, 
    TERA = 4, 
    PETA = 5, 
    EXA = 6
} unit_type;

typedef int addr_iter_fn_t(void *pArg, void *pArg2, 
    int vrf, const char *pszAddr, int nIdx);
int logging_server_addr_iter(const char *pszAddrs, 
    addr_iter_fn_t *pCB, void *pArg, void *pArg2);

typedef union gensock_u {
  struct sockaddr sa;
  struct sockaddr_in sin;
  struct sockaddr_in6 sin6;
} gensock_t;

typedef int dhcsrvgrp_iter_fn_t(int idx, int vrf, gensock_t *psu, void *pArg);
int dhcsrvgrp_addr_iter(char *pszAddrs, dhcsrvgrp_iter_fn_t *pCB, void *pArg);

#define GENSOCK_ADDRSTRLEN  64
char *gensock2str (gensock_t *pgs, char *buf, size_t len);
int str2gensock(gensock_t *pgs, const char *pszAddr);
int gensock_get_family(gensock_t *pgs);
void *gensock_get_addr(gensock_t *pgs);
int gensock_set_port(gensock_t *pgs, u_int16_t port);
u_int16_t gensock_get_port(gensock_t *pgs);

int syscmd_file_exist(char *filename);
int64_t check_disk_free_space(char* name);
int check_filename(const char *pszFilename);

int check_space_available(char * src_filename, char * dst_filename, int64_t size);

int check_ipaddress(char *p);

/*
 * "Human-readable" output uses 4 digits max, and puts a unit suffix at
 * the end.  Makes output compact and easy-to-read esp. on huge disks.
 */
#define FMT_SCALED_STRSIZE  7   /* minus sign, 4 digits, suffix, null byte */

/* Convert the given input string "scaled" into numeric in "result".
 * Return 0 on success, -1 and errno set on error.
 */
int scan_scaled(char *scaled, long long *result);

/* Format the given "number" into human-readable form in "result".
 * Result must point to an allocated buffer of length FMT_SCALED_STRSIZE.
 * Return 0 on success, -1 and errno set if error.
 */
int fmt_scaled(long long number, char *result);
int signal_app(const char *pszPidFile, int signal);

int check_macaddr(char *pszMac);
int check_hostname(const char *pszHostname);
int check_ipv6(const char *hostip);
int check_prefix_string(char *prifix);
int check_prefix_full_string(char *prifix);
char *check_prefix_dup(char *prifix);
#endif /* !__PARAM_CHECK_H__ */

