#include "sal.h"

#define NS_INADDRSZ 4   /* IPv4 T_A */
#define NS_IN6ADDRSZ    16  /* IPv6 T_AAAA */
#define NS_INT16SZ  2   /* #/bytes of data in a u_int16_t */

#ifndef SPRINTF
#define SPRINTF(x) ((int32)sal_sprintf x)
#endif

#define X_INF       0x7ffffff0
#define X_STORE(c) \
    {    \
        if (bp < be){    \
            * bp = (c); }  \
        bp ++;       \
    }

STATIC const char*
_inet_ntop4(const uint8* src, int8* dst, uint32 size)
{
    static const char fmt[] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];

    if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) > size)
    {
        return (NULL);
    }

    return sal_strcpy((char*)dst, tmp);
}

STATIC const char*
_inet_ntop6(const uint8* src, int8* dst, uint32 size)
{
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], * tp;

    struct { int base, len;
    } best, cur;
    uint32 words[NS_IN6ADDRSZ / NS_INT16SZ];
    int32 i;

    sal_memset(&best, 0, sizeof(best));
    sal_memset(&cur, 0, sizeof(cur));

    /*
     * Preprocess:
     *	Copy the input (bytewise) array into a wordwise array.
     *	Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    sal_memset(words, '\0', sizeof words);

    for (i = 0; i < NS_IN6ADDRSZ; i += 2)
    {
        words[i / 2] = (src[i] << 8) | src[i + 1];
    }

    best.base = -1;
    cur.base = -1;

    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
            {
                cur.base = i, cur.len = 1;
            }
            else
            {
                cur.len++;
            }
        }
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                {
                    best = cur;
                }

                cur.base = -1;
            }
        }
    }

    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
        {
            best = cur;
        }
    }

    if (best.base != -1 && best.len < 2)
    {
        best.base = -1;
    }

    /*
     * Format the result.
     */
    tp = tmp;

    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len))
        {
            if (i == best.base)
            {
                *tp++ = ':';
            }

            continue;
        }

        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
        {
            *tp++ = ':';
        }

        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 &&
            (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
        {
            if (!_inet_ntop4(src + 12, (int8*)tp, sizeof tmp - (tp - tmp)))
            {
                return (NULL);
            }

            tp += sal_strlen(tp);
            break;
        }

        tp += SPRINTF((tp, "%x", words[i]));
    }

    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) ==
        (NS_IN6ADDRSZ / NS_INT16SZ))
    {
        *tp++ = ':';
    }

    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((uint32)(tp - tmp) > size)
    {
        return (NULL);
    }

    return sal_strcpy((char*)dst, tmp);
}

STATIC int
_inet_pton4(const int8* src, uint8* dst)
{
    int32 saw_digit, octets, ch;
    uint8 tmp[NS_INADDRSZ], * tp;
    uint32 new_value = 0;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;

    while ((ch = *src++) != '\0')
    {

        if (ch >= '0' && ch <= '9')
        {
            new_value = *tp * 10 + (ch - '0');

            if (saw_digit && *tp == 0)
            {
                return (0);
            }

            if (new_value > 255)
            {
                return (0);
            }

            *tp = new_value;
            if (!saw_digit)
            {
                if (++octets > 4)
                {
                    return (0);
                }

                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
            {
                return (0);
            }

            *++tp = 0;
            saw_digit = 0;
        }
        else
        {
            return (0);
        }
    }

    if (octets < 4)
    {
        return (0);
    }

    sal_memcpy(dst, tmp, NS_INADDRSZ);
    return (1);
}

STATIC int
_inet_pton6(const int8* src, uint8* dst)
{
    static const char xdigits[] = "0123456789abcdef";
    uint8 tmp[NS_IN6ADDRSZ], * tp, * endp, * colonp;
    const int8* curtok;
    int32 ch, saw_xdigit;
    uint32 val;

    tp = sal_memset(tmp, '\0', NS_IN6ADDRSZ);
    endp = tp + NS_IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
    {
        if (*++src != ':')
        {
            return (0);
        }
    }

    curtok = src;
    saw_xdigit = 0;
    val = 0;

    while ((ch = sal_tolower(*src++)) != '\0')
    {
        const char* pch;

        pch = sal_strchr(xdigits, ch);
        if (pch != NULL)
        {
            val <<= 4;
            val |= ((uint32)pch - (uint32)xdigits);
            if (val > 0xffff)
            {
                return (0);
            }

            saw_xdigit = 1;
            continue;
        }

        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                {
                    return (0);
                }

                colonp = tp;
                continue;
            }
            else if (*src == '\0')
            {
                return (0);
            }

            if (tp + NS_INT16SZ > endp)
            {
                return (0);
            }

            *tp++ = (uint8)(val >> 8) & 0xff;
            *tp++ = (uint8)val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }

        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
            _inet_pton4(curtok, tp) > 0)
        {
            tp += NS_INADDRSZ;
            saw_xdigit = 0;
            break;  /* '\0' was seen by inet_pton4(). */
        }

        return (0);
    }

    if (saw_xdigit)
    {
        if (tp + NS_INT16SZ > endp)
        {
            return (0);
        }

        *tp++ = (uint8)(val >> 8) & 0xff;
        *tp++ = (uint8)val & 0xff;
    }

    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        if (tp == endp)
        {
            return (0);
        }

        for (i = 1; i <= n; i++)
        {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }

        tp = endp;
    }

    if (tp != endp)
    {
        return (0);
    }

    sal_memcpy(dst, tmp, NS_IN6ADDRSZ);
    return (1);
}

const char*
ctc_sal_inet_ntop(int32 af, void* src, char* dst, uint32 size)
{
    switch (af)
    {
    case AF_INET:
        return (_inet_ntop4(src, (int8*)dst, size));

    case AF_INET6:
        return (_inet_ntop6(src, (int8*)dst, size));

    default:
        return (NULL);
    }

    /* NOTREACHED */
}

int32
ctc_sal_inet_pton(int32 af, const char* src, void* dst)
{
    switch (af)
    {
    case AF_INET:
        return (_inet_pton4((int8*)src, dst));

    case AF_INET6:
        return (_inet_pton6((int8*)src, dst));

    default:
        return (-1);
    }

    /* NOTREACHED */
}

int32
ctc_sal_strcasecmp(const char* s1, const char* s2)
{
    const char* p1 = (const char*)s1;
    const char* p2 = (const char*)s2;
    int32 ret;

    if (p1 == p2)
    {
        return 0;
    }

    while ((ret = tolower(*p1) - tolower(*p2)) == 0)
    {
        if (*p1 == '\0')
        {
            break;
        }

        p1++;
        p2++;
    }

    return ret;
}

int32
ctc_sal_strncasecmp(const char* s1, const char* s2, int32 n)
{
    uint8 c1, c2;

    if (n == 0)
    {
        return 0;
    }

    do
    {
        c1 = tolower(*s1);
        c2 = tolower(*s2);
        s1++;
        s2++;
    }
    while ((--n) && (c1 == c2) && (c1 != 0));

    return (c1 - c2);
}

#if 0
int8*
sal_strdup(const int8* s)
{
    int8 len = sal_strlen(s);
    int8* rc = malloc(len + 1);

    if (rc != NULL)
    {
        sal_strcpy(rc, s);
    }

    return rc;
}

#endif

uint64 ctc_sal_strtoull(const char *ptr, char **end, int base)
{
    uint64 ret = 0;

    if (base == 16)
    {
        /* "0x" for hex format */
        ptr = ptr + 2;
    }
    else if (base != 10)
    {
    	  goto out;
    }

    while (*ptr)
    {
        int digit;

        if (*ptr >= '0' && *ptr <= '9' && *ptr < '0' + base)
        {
            digit = *ptr - '0';
	  }
        else if (*ptr >= 'A' && *ptr < 'A' + base - 10)
        {
            digit = *ptr - 'A' + 10;
    	  }
        else if (*ptr >= 'a' && *ptr < 'a' + base - 10)
        {
            digit = *ptr - 'a' + 10;
    	  }
        else
    	  {
            break;
         }

        ret *= base;
        ret += digit;
        ptr++;
    }

out:
    if (end)
    {
        *end = (char *)ptr;
    }

    return ret;
}
