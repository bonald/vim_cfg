#include "proto.h"
#include "lib_snmp.h"
#include "gen/tbl_snmp_trap_msg_define.h"
#include "gen/tbl_snmp_trap_msg.h"

#define M_SNMP_TRAP_BUFSZ   1024

static char *g_psz_trap_buf = NULL;

extern ctc_sock_session_t* _ctc_sock_get_session(int32 sock, uint32 peer);
extern char* oem_info_get_snmp_enterprise_oid(char *buf);
extern int split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
             char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);
extern int free_tokens(char ***pppTokArr);

static int
_snprint_objid(oid_t *pOid, size_t len, char *pszOutput, size_t bufsz)
{
    char szOidBuf[32];
    int leftsz = bufsz;
    char *p = pszOutput;
    int bytes;
    int i;

    for (i = 0; i < len; i++) {
        if (0 == i) { 
            bytes = sal_snprintf(szOidBuf, 32, "%lu", pOid[i]);
        } else {
            bytes = sal_snprintf(szOidBuf, 32, ".%lu", pOid[i]);
        }
        if (leftsz <= bytes) { /* buffer overflow */
            return 0;
        }

        sal_snprintf(p, leftsz, "%s", szOidBuf);
        leftsz -= bytes;
        p += bytes;
    }
    
    return (bufsz - leftsz);
}

/* format:
 *   /oid/TRAPOID/uptime/UPTIME/OID/type/value\OID2\type2\value2\....\r\n')
 */
static int 
_build_snmp_trap_msg(oid_t *trap_oid, size_t trap_oid_len,
    oid_t spec_trap_val, oid_t *name, size_t namelen, trap_val_t *trapobj, 
    size_t trapobjlen, char *pszBuf, size_t bufsz, int *pRetBytes)
{
    struct sal_timeval tv;
    trap_val_t *trapv = trapobj;
    int left = bufsz;
    char *p = pszBuf;
    int nbytes;
    char type = 'i';
    int i;
    int ival;
    unsigned long uval;
    char szNumber[32];
    char *pval;

    /* output /oid/TRAPOID */
    nbytes = sal_snprintf(p, left, "/oid/");
    p += nbytes;
    left -= nbytes;

    nbytes = _snprint_objid(trap_oid, trap_oid_len, p, left);
    p += nbytes;
    left -= nbytes;    

    /* output /uptime/SYSUPTIME */
    ctc_task_update_boottime(&tv);
    nbytes = sal_snprintf(p, left, "/uptime/%lu", tv.tv_sec * 100);
    p += nbytes;
    left -= nbytes;

    /* output trap objs */
    for (i = 0; i < trapobjlen; i++) {
        trapv = &trapobj[i];

        sal_snprintf(p, left, "/");
        p += 1;
        left -= 1;
        
        nbytes = _snprint_objid(trapv->name, trapv->namelen, p, left);
        if (0 == nbytes) { /* buffer overflow */
            return -1;
        }        
        p += nbytes;
        left -= nbytes;

        switch (trapv->val_type) {
        case ASN_INTEGER:
            type = 'i';
            break;
        case ASN_UNSIGNED:
            type = 'u';
            break;
        case ASN_TIMETICKS:
            type = 't';
            break;
        case ASN_IPADDRESS:
            type = 'a';
            break;
        case ASN_OBJECT_ID:
            type = 'o';
            break;
        case ASN_OCTET_STR:
            type = 's';
            break;
        default:
            log_sys(M_MOD_APP, E_ERROR, 
                "_build_snmp_trap_msg unsupportted trap obj type %u\n",
                trapv->val_type);
            return -1;
        }
        
        pval = szNumber;
        switch (trapv->val_type) {
        case ASN_INTEGER:
            ival = *((int *)trapv->val);
            nbytes = sal_snprintf(szNumber, 32, "%d", ival); 
            break;
        case ASN_UNSIGNED:
        case ASN_TIMETICKS:
        case ASN_IPADDRESS:
            uval = *((unsigned long *)trapv->val);
            nbytes = sal_snprintf(szNumber, 32, "%lu", uval);
            break;
        case ASN_OBJECT_ID:
        case ASN_OCTET_STR:
            nbytes = trapv->val_len;
            pval = trapv->val;
        default:
            break;
        }

        if (left <= (nbytes + 4)) { /* buffer overflow */
            return -1;
        }

        nbytes = sal_snprintf(p, left, "/%c/%s", type, pval);
        p += nbytes;
        left -= nbytes;
    }

    if (left <= 2) { /* buffer overflow */
        return -1;
    }
    /* deleted by huwx for snmp multi trap process bug 41448; 
    sal_snprintf(p, left, "\r\n");
    left -= 2;
    */
    
    *pRetBytes = (bufsz - left);
    return 0;
}

static int
_ipc_sendn(int fd, char *pszBuf, size_t nbytes)
{
    char *p = pszBuf;
    int nleft;
    int nwrite;

    nleft = nbytes;

    while (nleft > 0) {
        nwrite = sal_write(fd, p, nleft);
        if (nwrite <= 0) {
            switch (errno) {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
                continue;
            }

            return nwrite;
        }
        nleft -= nwrite;
        p += nwrite;
    }

    return nbytes - nleft;
}

int32
snmp_trapv2_to_socket(oid_t *trap_oid, size_t trap_oid_len,
    oid_t spec_trap_val, oid_t *name, size_t namelen,
    trap_val_t *trapobj, size_t trapobjlen)
{
    lib_master_t *lib_master = lib_get_master();
    int csock = lib_master ? lib_master->trap_sock : -1;
    ctc_sock_session_t *p_session = NULL;
    int nbytes = 0;
    int fd = -1;

    if (csock < 0) {
        return PM_E_NOT_INITED;
    }

    p_session = _ctc_sock_get_session(csock, IGNORE_SOCK_PEER);
    if (NULL == p_session) {
        return PM_E_SOCK;
    }
    fd = p_session->fd;

    if (NULL == g_psz_trap_buf) {
        /* allocate more memory for base 64 encoding */
        g_psz_trap_buf = (char *)XCALLOC(MEM_PM_TEMP, M_SNMP_TRAP_BUFSZ * 3);
        if (NULL == g_psz_trap_buf) {
            return PM_E_NO_MEMORY;
        }
    }

    if (_build_snmp_trap_msg(trap_oid, trap_oid_len, spec_trap_val, 
            name, namelen, trapobj, trapobjlen, 
            g_psz_trap_buf, M_SNMP_TRAP_BUFSZ, &nbytes) != 0) {
        return PM_E_RESOURCE_FULL;
    }

    nbytes = sal_b64_ntop((unsigned char *)g_psz_trap_buf, nbytes, 
        g_psz_trap_buf + M_SNMP_TRAP_BUFSZ, 2 * M_SNMP_TRAP_BUFSZ);

    /* added by huwx for snmp multi trap process bug 41448; */
    sal_strcat(g_psz_trap_buf + M_SNMP_TRAP_BUFSZ, "\r\n");
    nbytes += 2;
    /* end */
    
    if (_ipc_sendn(fd, g_psz_trap_buf + M_SNMP_TRAP_BUFSZ, nbytes) != nbytes) {
        return PM_E_FAIL;
    }
    
    fsync(fd);
    return PM_E_NONE;
}

int32
snmp_trapv2_to_cdb(oid_t *trap_oid, size_t trap_oid_len,
    oid_t spec_trap_val, oid_t *name, size_t namelen,
    trap_val_t *trapobj, size_t trapobjlen)
{
    tbl_snmp_trap_msg_t msg;
    char buf[M_SNMP_TRAP_BUFSZ];
    int nbytes = 0;

    sal_memset(&msg, 0, sizeof(msg));
    sal_memset(&buf, 0, sizeof(buf));

    if (_build_snmp_trap_msg(trap_oid, trap_oid_len, spec_trap_val, 
            name, namelen, trapobj, trapobjlen, 
            buf, M_SNMP_TRAP_BUFSZ, &nbytes) != 0) {
        return PM_E_RESOURCE_FULL;
    }

    sal_strcpy(msg.msg, buf);
    tbl_snmp_trap_msg_set_snmp_trap_msg_field(&msg, TBL_SNMP_TRAP_MSG_FLD_MSG);

    return PM_E_NONE;
}

int32
snmp_trapv2(oid_t *trap_oid, size_t trap_oid_len,
    oid_t spec_trap_val, oid_t *name, size_t namelen,
    trap_val_t *trapobj, size_t trapobjlen)
{
#ifdef CONFIG_SNMPAGT
    return snmp_trapv2_to_cdb(trap_oid, trap_oid_len, spec_trap_val, name, namelen, trapobj, trapobjlen);
#else
    return snmp_trapv2_to_socket(trap_oid, trap_oid_len, spec_trap_val, name, namelen, trapobj, trapobjlen);
#endif
}

unsigned long
get_snmp_enterprise_oid()
{
    unsigned long eoid = 27975;
    char szBuf[32];

    oem_info_get_snmp_enterprise_oid(szBuf);
    eoid = strtoul(szBuf, NULL, 10);
    return eoid;
}

int32
snmp_calc_enterprise_oid(oid_t *enterprise_oid, uint32 eoid_len, uint32 *oid_len)
{
    char enterprise_buf[MAX_OID_LEN];
    char **ppOidArr = NULL;
    unsigned long nCnt = 0;
    int i = 0;
    int num = 0;
    oid_t stcommoid[] = {ENTERPRISEBASEMIB};

    if (eoid_len < sizeof(stcommoid)/sizeof(oid_t))
    {
        return -1;
    }
    
    *oid_len = sizeof(stcommoid)/sizeof(oid_t);
    sal_memcpy(enterprise_oid, stcommoid, (*oid_len) * sizeof (oid_t));

#if CTC_IS_UML
    sal_snprintf(enterprise_buf, OEM_INFO_BUFSIZ, "27975");
#else
    oem_info_get_snmp_enterprise_oid(enterprise_buf);
#endif
    if (split_tokens(enterprise_buf, sal_strlen(enterprise_buf), MAX_OID_LEN,
              ",", &nCnt, &ppOidArr) != 0)
    {
        return -1;
    }

    if (eoid_len < sizeof(stcommoid)/sizeof(oid_t) + nCnt)
    {
        return -1;
    }

    for (i = 0; i < nCnt; i++)
    {
        if (NULL == ppOidArr[i])
        {
            continue;
        }
        
        num = sal_atoi(ppOidArr[i]);
        enterprise_oid[*oid_len+i] = num;
    }

    *oid_len += nCnt;
    free_tokens(&ppOidArr);

    return PM_E_NONE;
}

