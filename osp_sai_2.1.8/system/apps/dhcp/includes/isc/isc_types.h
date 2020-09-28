/* create for replace isc/types.h by liuyang 2012-8-2 */

#ifndef ISC_P_TYPES_H
#define ISC_P_TYPES_H 1

/*
 * XXXDCL should isc_boolean_t be moved here, requiring an explicit include
 * of <isc/boolean.h> when ISC_TRUE/ISC_FALSE/ISC_TF() are desired?
 */
#include <isc/boolean.h>
/*
 * XXXDCL This is just for ISC_LIST and ISC_LINK, but gets all of the other
 * list macros too.
 */
#include <isc/list.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

struct isc_sockaddr {
	union {
		struct sockaddr		sa;
		struct sockaddr_in	sin;
		struct sockaddr_in6	sin6;
	}				type;
	unsigned int			length;		/* XXXRTH beginning? */
	ISC_LINK(struct isc_sockaddr)	link;
};
typedef struct isc_sockaddr		isc_sockaddr_t;		/*%< Socket Address */
typedef ISC_LIST(struct isc_sockaddr)	isc_sockaddrlist_t;
typedef unsigned int			isc_result_t;		/*%< Result */
typedef u_int32_t                    isc_uint32_t;
typedef isc_uint32_t			isc_fsaccess_t;		/*%< FS Access */
typedef u_int64_t                    isc_uint64_t;
typedef struct dst_key  dst_key_t;
typedef u_int16_t                    isc_uint16_t;
typedef isc_uint16_t				dns_rcode_t;

#endif /* ISC_TYPES_H */

