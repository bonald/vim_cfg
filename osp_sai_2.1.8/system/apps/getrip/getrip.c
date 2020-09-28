
#include <ctype.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>
#include <stdarg.h>
#include <arpa/inet.h>

#define _M_CMD_BUFSZ    10240
#define IPV4_PREFIX_LEN 7

#define CONNTRACK "nf_conntrack"

int
getrip_cmd_exec(char *pszRetBuf, size_t nBufSize, const char *pszCmdFmt, ...)
{
    va_list ap;
    char *pszCmd = NULL;
    FILE *fp = NULL;
    int nLeft;
    int nOffset;
    char *p;

    pszCmd = malloc(_M_CMD_BUFSZ);
    if (NULL == pszCmd) {
        fprintf(stderr, "%% sal_cmd_exec: out of memory\r\n");
        return -1;
    }

    va_start(ap, pszCmdFmt);
    vsnprintf(pszCmd, _M_CMD_BUFSZ, pszCmdFmt, ap);
    va_end(ap);

    fp = popen(pszCmd, "r");  
    if (NULL == fp) {
        free(pszCmd);
        return -1;
    }

    if (pszRetBuf && nBufSize > 1) {
        pszRetBuf[0] = '\0';
        nLeft = nBufSize;
        nOffset = 0;
        for (;;) {
            p = fgets(pszRetBuf + nOffset, nBufSize - nOffset, fp);
            if (NULL == p) {
                break;
            }

            nLeft -= strlen(p);
            if (nLeft <= 1) {
                break;
            }
        }

        /* delete this tailer \r\n */
        if (nLeft < nBufSize) {
            p = pszRetBuf + (strlen(pszRetBuf) - 1);
            if ('\n' == *p) {
                *p = '\0';
                p--;
            }
            if (p >= pszRetBuf && '\r' == *p) {
                *p = '\0';
            }
        }
    }

    free(pszCmd);
    return pclose(fp);
}

int refineRemoteIP(char *pszRemoteIP, char *nSrcPort, char *pszRet, size_t bufsz)
{
    char szBuf[256];
    char szIp[128];
    struct in6_addr addrv6;

    if (!strncmp(pszRemoteIP, "::ffff:", IPV4_PREFIX_LEN))
    {
        pszRemoteIP += IPV4_PREFIX_LEN;
    }

    if (!strncmp(pszRemoteIP, "169.254.1.", strlen("169.254.1.")) 
            || !strncmp(pszRemoteIP, "169.254.2.", strlen("169.254.2."))
            || !strncmp(pszRemoteIP, "169.254.3.", strlen("169.254.3."))) {
        if (getrip_cmd_exec(szBuf, 256, 
                "ip netns exec mgmt cat /proc/net/" CONNTRACK
                " | "
                "grep \"sport=%s\" | awk '{print $7}'",
                nSrcPort) == 0) {
            if (!strncmp(szBuf, "src=", 4)) {
                snprintf(pszRet, bufsz, "%s", szBuf + 4);
                return 0;
            }
        }
    }

    if (!strncmp(pszRemoteIP, "fec0:fffe:fffe::1:", strlen("fec0:fffe:fffe::1:")) 
            || !strncmp(pszRemoteIP, "fec0:fffe:fffe::2:", strlen("fec0:fffe:fffe::2:")))
    {
        if (getrip_cmd_exec(szBuf, 256, 
                "ip netns exec mgmt cat /proc/net/nf_conntrack"
                " | grep ipv6 | grep \"sport=%s\" | awk '{print $7}'",
                nSrcPort) == 0) {
            if (!strncmp(szBuf, "src=", 4))
            {
                inet_pton(AF_INET6, szBuf + 4, (void*)&addrv6);
                inet_ntop(AF_INET6, (void*)&addrv6, pszRet, bufsz);
                return 0;
            }
        }
    }
    
    snprintf(szIp, 128, pszRemoteIP);
    snprintf(pszRet, bufsz, szIp);
    return 0;
}

int main(int argc, char **argv)
{
    char szBuf[256];

    if (3 != argc)
    {
        printf("Invalid parameter");
        exit(1);
    }

    memset(szBuf, 0, sizeof(szBuf));
    refineRemoteIP(argv[1], argv[2], szBuf, 256);
    printf(szBuf);    
    return 0;
}

