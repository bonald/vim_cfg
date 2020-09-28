
#ifndef __PREFIX_H__
#define __PREFIX_H__

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif /* INET_ADDRSTRLEN */

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif /* INET6_ADDRSTRLEN */

#define IPV6_MIN_PREFIXLEN  0
#define IPV6_MAX_PREFIXLEN  128

/* Max bit/byte length of IPv4 address. */
#define IPV4_MAX_BYTELEN    4
#define IPV4_MAX_BITLEN     32
#define IPV4_MIN_PREFIXLEN  0
#define IPV4_MAX_PREFIXLEN  32
#define IPV4_ADDR_CMP(D,S)   memcmp ((D), (S), IPV4_MAX_BYTELEN)
#define IPV4_ADDR_SAME(D,S)  (memcmp ((D), (S), IPV4_MAX_BYTELEN) == 0)
#define IPV4_ADDR_COPY(D,S)  memcpy ((D), (S), IPV4_MAX_BYTELEN)

#define IPV4_BROADCAST(a)         ((((u_int32_t) (a)) & 0xffffffff) == 0xffffffff)
#define IPV4_NET0(a)    ((((u_int32_t) (a)) & 0xff000000) == 0x00000000)
#define IPV4_NET127(a)  ((((u_int32_t) (a)) & 0xff000000) == 0x7f000000)
#define IPV4_LINKLOCAL(a) ((((u_int32_t) (a)) & 0xffff0000) == 0xa9fe0000)
#define IPV4_CLASS_DE(a)  ((((u_int32_t) (a)) & 0xe0000000) == 0xe0000000)
#define IPV4_CHECK1(a)    ((((u_int32_t) (a)) & 0xff000000) == 0xff000000)
#define IPV4_CHECK2(a)    ((((u_int32_t) (a)) & 0xff000000) <= 0xff000000 && (((u_int32_t) (a)) & 0xff000000) >= 0xe0000000)
#define IPV4_CHECK3(a)    ((((u_int32_t) (a)) & 0x000000ff) == 0x00000000)
#define IPV4_CHECK4(a)    ((((u_int32_t) (a)) & 0x000000ff) == 0x000000ff)
#define IPV4_CHECK5(a)  ((((u_int32_t) (a)) & 0x000000ff) == 0x0000007f)
#define IPV4_CHECK6(a)    ((((u_int32_t) (a)) & 0x000000ff) <= 0x000000ff && (((u_int32_t) (a)) & 0x000000ff) >= 0x000000e0)

#ifndef IN_LOOPBACK
#define IN_LOOPBACK(a)  ((((long int) (a)) & 0xff000000) == 0x7f000000)
#endif /* !IN_LOOPBACK */

/* Max bit/byte length of IPv6 address. */
#define IPV6_MAX_BYTELEN    16
#define IPV6_MAX_BITLEN    128
#define IPV6_MAX_PREFIXLEN 128
#define IPV6_ADDR_CMP(D,S)   memcmp ((D), (S), IPV6_MAX_BYTELEN)
#define IPV6_ADDR_SAME(D,S)  (memcmp ((D), (S), IPV6_MAX_BYTELEN) == 0)
#define IPV6_ADDR_COPY(D,S)  memcpy ((D), (S), IPV6_MAX_BYTELEN)

#ifdef HAVE_IPV6
#define IS_PREFIX_MAX_BITLEN(P) \
    ((P)->family == AF_INET ? (P)->prefixlen == IPV4_MAX_BITLEN : \
     (P)->family == AF_INET6 ? (P)->prefixlen == IPV6_MAX_BITLEN : 0)
#else
#define IS_PREFIX_MAX_BITLEN(P) \
    ((P)->family == AF_INET ? (P)->prefixlen == IPV4_MAX_BITLEN : 0)
#endif /* HAVE_IPV6 */

int32
ip_apply_mask_ipv4(prefix_t *p);

int32
apply_mask_ipv4_addr(addr_ipv4_t *p, uint32 prefixlen);

int32
ip_apply_mask(prefix_t *p);

int32 
ip_apply_mask_ipv6(prefix_t *p);

int32 
apply_mask_ipv6_addr(addr_ipv6_t *p, uint32 prefixlen);

int32
addr_mask_len2ip_v4(int32 masklen, addr_ipv4_t *mask);

int32
ip_prefix_copy(prefix_t *dest, const prefix_t *src);

uint32
prefix_bit(const uint8 *prefix, const uint8 prefixlen);

int32
sal_prefix_match(prefix_t *n, prefix_t *p);

int32
prefix_compare_net(prefix_t *n, prefix_t *p);

int32
prefix6_compare_net(prefix_t *n, prefix_t *p);

int32
masklen2ipstr(uint8 family, int32 len, char *str);

void
sal_masklen2ip (int32 masklen, struct sal_in4_addr *netmask);

char*
prefix_val2str(char *str, uint32 str_len, const prefix_t *prefix, uint32 with_str_mask);

uint8
ip_convert_masklen(struct sal_in4_addr netmask);

#endif /* !__PREFIX_H__ */

