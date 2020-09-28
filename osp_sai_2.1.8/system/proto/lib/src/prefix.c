
#include "proto.h"
#include "glb_ip_define.h"
#include "prefix.h"

int32
apply_mask_ipv4_addr(addr_ipv4_t *p, uint32 prefixlen)
{
    addr_ipv4_t mask;
    addr_mask_len2ip_v4(prefixlen, &mask);
    p->s_addr &= mask.s_addr;

    return PM_E_NONE;
}

/*get network addr*/
int32
ip_apply_mask_ipv4(prefix_t *p)
{
    addr_ipv4_t mask;
    addr_mask_len2ip_v4(p->prefixlen, &mask);
    p->u.prefix4.s_addr &= mask.s_addr;

    return PM_E_NONE;
}

static const uint8 maskbit[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0,
                                  0xf8, 0xfc, 0xfe, 0xff};

int32 
apply_mask_ipv6_addr(addr_ipv6_t *p, uint32 prefixlen)
{
	uint8 *pnt;
	int32 index;
	int32 offset;

	index = prefixlen / 8;

	if (index < 16) {
		pnt = (uint8 *)p;
		offset = prefixlen % 8;

		pnt[index] &= maskbit[offset];
		index++;

		while (index < 16)
			pnt[index++] = 0;
	}

	return PM_E_NONE;
}

int32 
ip_apply_mask_ipv6(prefix_t *p)
{
	uint8 *pnt;
	int32 index;
	int32 offset;

	index = p->prefixlen / 8;

	if (index < 16) {
		pnt = (uint8 *)&p->u.prefix6;
		offset = p->prefixlen % 8;

		pnt[index] &= maskbit[offset];
		index++;

		while (index < 16)
			pnt[index++] = 0;
	}

	return PM_E_NONE;
}

int32
ip_apply_mask(prefix_t *p)
{
    switch (p->family)
    {
    case AF_INET:
        return ip_apply_mask_ipv4(p);
    case AF_INET6:
        return ip_apply_mask_ipv6(p);
    default:
        return PM_E_INVALID_PARAM;
    }
}

/*ipv4 mask length to mask addr*/
int32
addr_mask_len2ip_v4(int32 masklen, addr_ipv4_t *mask)
{
    SAL_ASSERT(masklen >= 0 && masklen <= GLB_IPV4_MAX_PREFIXLEN);

    if (sizeof(unsigned long long) > 4)
    {
        mask->s_addr = sal_htonl(0xffffffffULL << (32 - masklen));
    }
    else
    {
        mask->s_addr = sal_htonl(masklen ? 0xffffffffU << (32 - masklen) : 0);
    }

    return PM_E_NONE;
}

const uint8 mask32bit[][4] =
{
    {0x00, 0x00, 0x00, 0x00},

    {0x80, 0x00, 0x00, 0x00},
    {0xc0, 0x00, 0x00, 0x00},
    {0xe0, 0x00, 0x00, 0x00},
    {0xf0, 0x00, 0x00, 0x00},
    {0xf8, 0x00, 0x00, 0x00},
    {0xfc, 0x00, 0x00, 0x00},
    {0xfe, 0x00, 0x00, 0x00},
    {0xff, 0x00, 0x00, 0x00},

    {0xff, 0x80, 0x00, 0x00},
    {0xff, 0xc0, 0x00, 0x00},
    {0xff, 0xe0, 0x00, 0x00},
    {0xff, 0xf0, 0x00, 0x00},
    {0xff, 0xf8, 0x00, 0x00},
    {0xff, 0xfc, 0x00, 0x00},
    {0xff, 0xfe, 0x00, 0x00},
    {0xff, 0xff, 0x00, 0x00},

    {0xff, 0xff, 0x80, 0x00},
    {0xff, 0xff, 0xc0, 0x00},
    {0xff, 0xff, 0xe0, 0x00},
    {0xff, 0xff, 0xf0, 0x00},
    {0xff, 0xff, 0xf8, 0x00},
    {0xff, 0xff, 0xfc, 0x00},
    {0xff, 0xff, 0xfe, 0x00},
    {0xff, 0xff, 0xff, 0x00},

    {0xff, 0xff, 0xff, 0x80},
    {0xff, 0xff, 0xff, 0xc0},
    {0xff, 0xff, 0xff, 0xe0},
    {0xff, 0xff, 0xff, 0xf0},
    {0xff, 0xff, 0xff, 0xf8},
    {0xff, 0xff, 0xff, 0xfc},
    {0xff, 0xff, 0xff, 0xfe}
};

const uint32 *maskbit32 = (uint32*)mask32bit;

int32
ip_prefix_copy(prefix_t *dest, const prefix_t *src)
{
    dest->family = src->family;
    dest->prefixlen = src->prefixlen;

    if (src->family == AF_INET)
    {
        dest->u.prefix4 = src->u.prefix4;
    }
    else if (src->family == AF_INET6)
    {
        dest->u.prefix6 = src->u.prefix6;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;    
}

uint32
prefix_bit(const uint8 *prefix, const uint8 prefixlen)
{
    uint32 offset = prefixlen / 8;
    uint32 shift  = 7 - (prefixlen % 8);
  
    return (prefix[offset] >> shift) & 1;
}

/* If n includes p prefix then return 1 else return 0. */
int32
sal_prefix_match(prefix_t *n, prefix_t *p)
{
    int32 offset = 0;
    int32 shift = 0;

    /* Set both prefix's head pointer. */
    uint32 *np = (uint32*)&n->u.prefix4;
    uint32 *pp = (uint32*)&p->u.prefix4;

    /* If n's prefix is longer than p's one return 0. */
    if (n->prefixlen > p->prefixlen)
    {
        return FALSE;
    }

    offset = n->prefixlen / 32;
    shift =  n->prefixlen % 32;

    if (shift)
    {
        if (maskbit32[shift] & (np[offset] ^ pp[offset]))
        {
            return FALSE;
        }
    }

    while (offset--)
    {
        if (np[offset] != pp[offset])
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* If n includes p prefix then return 1 else return 0. */
int32
prefix_compare_net(prefix_t *n, prefix_t *p)
{
    int32 offset = 0;
    int32 shift = 0;

    /* Set both prefix's head pointer. */
    //n->u.prefix4.s_addr = sal_htonl(n->u.prefix4.s_addr);
    //p->u.prefix4.s_addr = sal_htonl(p->u.prefix4.s_addr);
    uint32 *np = (uint32*)&n->u.prefix4;
    uint32 *pp = (uint32*)&p->u.prefix4;

    /* If n's prefix is longer than p's one return 0. */
    if (n->prefixlen > p->prefixlen)
    {
        return FALSE;
    }

    offset = n->prefixlen / 32;
    shift =  n->prefixlen % 32;

    if (shift)
    {
        if ((maskbit32[shift] & (np[offset])) ^ (maskbit32[shift] & pp[offset]))
        {
            return FALSE;
        }
    }

    while (offset--)
    {
        if (np[offset] != pp[offset])
        {
            return FALSE;
        }
    }

    return TRUE;
}

int32
prefix6_compare_net(prefix_t *n, prefix_t *p)
{
    int32 offset = 0;
    int32 shift = 0;
    uint32 i = 0;

    uint8 *np = (uint8*)&n->u.prefix6;
    uint8 *pp = (uint8*)&p->u.prefix6;

    /* If n's prefix is longer than p's one return 0. */
    if (n->prefixlen > p->prefixlen)
    {
        return FALSE;
    }

    offset = n->prefixlen / 8;
    shift  = n->prefixlen % 8;

    for (i = 0; i < offset; ++i)
    {
        if (*(np+i) != *(pp+i))
        {
            return FALSE;
        }
    }

    if (shift)
    {
        if ((*(np+i) >> (8 - shift)) != (*(pp+i) >> (8 - shift)))
        {
            return FALSE;
        }
    }
    return TRUE;
}


/*from frr prefix.c*/
/*same: 1*/
/*not same: 0*/
int 
sal_prefix_same(const prefix_t *p1, const prefix_t *p2)
{
    if ((p1 && !p2) || (!p1 && p2))
    {
        return 0;
     }

    if (!p1 && !p2)
    {
        return 1;
    }

    if (p1->family == p2->family && p1->prefixlen == p2->prefixlen) 
    {
        if (p1->family == AF_INET)
        {
            if (IPV4_ADDR_SAME(&p1->u.prefix4.s_addr, &p2->u.prefix4.s_addr))
            {
                return 1;
            }
        }
        if (p1->family == AF_INET6)
        {
            if (IPV6_ADDR_SAME(&p1->u.prefix6.s6_addr, &p2->u.prefix6.s6_addr))
            {
                return 1;
            }
        }
        #if 0
        if (p1->family == AF_ETHERNET)
        {
            if (!memcmp(&p1->u.prefix_evpn, &p2->u.prefix_evpn, sizeof(struct evpn_addr)))
            {
                return 1;
            }
        }
        #endif
    }
    return 0;
}

void
sal_masklen2ip6 (int32 masklen, struct sal_in6_addr *netmask)
{
  char *pnt;
  int bit;
  int offset;

  sal_memset (netmask, 0, sizeof (struct sal_in6_addr));
  pnt = (char *) netmask;

  offset = masklen / 8;
  bit = masklen % 8;

  while (offset--)
    *pnt++ = 0xff;

  if (bit)
    *pnt = maskbit[bit];
}

void
sal_masklen2ip (int32 masklen, struct sal_in4_addr *netmask)
{
  u_int8_t *pnt;
  int32 bit;
  int32 offset;

  sal_memset (netmask, 0, sizeof (struct sal_in4_addr));
  pnt = (u_int8_t *) netmask;

  offset = masklen / 8;
  bit = masklen % 8;

  while (offset--)
    *pnt++ = 0xff;

  if (bit)
    *pnt = maskbit[bit];
}

int32
masklen2ipstr(uint8 family, int32 len, char *str)
{
    struct sal_in4_addr netmask4;
    struct sal_in6_addr netmask6;
    
    if (AF_INET == family)
    {
        sal_masklen2ip(len, &netmask4);
        sal_inet_ntop(AF_INET, &netmask4, str, INET_ADDRSTRLEN);
    }
    else if (AF_INET6 == family)
    {
        sal_masklen2ip6(len, &netmask6);
        sal_inet_ntop(AF_INET6, &netmask6, str, INET6_ADDRSTRLEN);
    }
    else
    {
        return -1;
    }
    return 0;
}

/*cdb_prefix_val2str is xxx#xx, prefix_val2str is xxx/x*/
char*
prefix_val2str(char *str, uint32 str_len, const prefix_t *prefix, uint32 with_str_mask)
{
    uint32 len = 0;

    if (AF_INET == prefix->family)
    {
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET, (void*)&(prefix->u.prefix4), str, str_len);
        len = sal_strlen(str);
    }
    else if (AF_INET6 == prefix->family)
    {
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET6, (void*)&(prefix->u.prefix6), str, str_len);
        len = sal_strlen(str);
    }

    if (with_str_mask)
    {
        sal_sprintf(str + len, "/%u", prefix->prefixlen);
    }
    
    return str;
}

/* Convert IP address's netmask into integer. We assume netmask is
   sequential one. Argument netmask should be network byte order. */
uint8
ip_convert_masklen(struct sal_in4_addr netmask)
{
    uint8 len;
    uint8 *pnt;
    uint8 *end;
    uint8 val;

    len = 0;
    pnt = (uint8 *) &netmask;
    end = pnt + 4;

    while (pnt < end && (*pnt == 0xff))
    {
        len+= 8;
        pnt++;
    }

    if (pnt < end)
    {
        val = *pnt;
        while (val)
        {
            len++;
            val <<= 1;
        }
    }
    return len;
}


