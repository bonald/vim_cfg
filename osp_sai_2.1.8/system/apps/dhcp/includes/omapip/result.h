/* result.h
 */

/*
 * Copyright (C) 2009  Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   https://www.isc.org/
 */

#ifndef DHCP_RESULT_H
#define DHCP_RESULT_H 1

#include <isc/lang.h>
#include <isc/resultclass.h>
#ifndef _CENTEC_
#include <isc/types.h>

#include <isc/result.h>
#else
#define ISC_R_SUCCESS			0	/*%< success */
#define ISC_R_NOMEMORY			1	/*%< out of memory */
#define ISC_R_TIMEDOUT			2	/*%< timed out */
#define ISC_R_NOTHREADS			3	/*%< no available threads */
#define ISC_R_ADDRNOTAVAIL		4	/*%< address not available */
#define ISC_R_ADDRINUSE			5	/*%< address in use */
#define ISC_R_NOPERM			6	/*%< permission denied */
#define ISC_R_NOCONN			7	/*%< no pending connections */
#define ISC_R_NETUNREACH		8	/*%< network unreachable */
#define ISC_R_HOSTUNREACH		9	/*%< host unreachable */
#define ISC_R_NETDOWN			10	/*%< network down */
#define ISC_R_HOSTDOWN			11	/*%< host down */
#define ISC_R_CONNREFUSED		12	/*%< connection refused */
#define ISC_R_NORESOURCES		13	/*%< not enough free resources */
#define ISC_R_EOF			14	/*%< end of file */
#define ISC_R_BOUND			15	/*%< socket already bound */
#define ISC_R_RELOAD			16	/*%< reload */
#define ISC_R_SUSPEND	      ISC_R_RELOAD	/*%< alias of 'reload' */
#define ISC_R_LOCKBUSY			17	/*%< lock busy */
#define ISC_R_EXISTS			18	/*%< already exists */
#define ISC_R_NOSPACE			19	/*%< ran out of space */
#define ISC_R_CANCELED			20	/*%< operation canceled */
#define ISC_R_NOTBOUND			21	/*%< socket is not bound */
#define ISC_R_SHUTTINGDOWN		22	/*%< shutting down */
#define ISC_R_NOTFOUND			23	/*%< not found */
#define ISC_R_UNEXPECTEDEND		24	/*%< unexpected end of input */
#define ISC_R_FAILURE			25	/*%< generic failure */
#define ISC_R_IOERROR			26	/*%< I/O error */
#define ISC_R_NOTIMPLEMENTED		27	/*%< not implemented */
#define ISC_R_UNBALANCED		28	/*%< unbalanced parentheses */
#define ISC_R_NOMORE			29	/*%< no more */
#define ISC_R_INVALIDFILE		30	/*%< invalid file */
#define ISC_R_BADBASE64			31	/*%< bad base64 encoding */
#define ISC_R_UNEXPECTEDTOKEN		32	/*%< unexpected token */
#define ISC_R_QUOTA			33	/*%< quota reached */
#define ISC_R_UNEXPECTED		34	/*%< unexpected error */
#define ISC_R_ALREADYRUNNING		35	/*%< already running */
#define ISC_R_IGNORE			36	/*%< ignore */
#define ISC_R_MASKNONCONTIG             37	/*%< addr mask not contiguous */
#define ISC_R_FILENOTFOUND		38	/*%< file not found */
#define ISC_R_FILEEXISTS		39	/*%< file already exists */
#define ISC_R_NOTCONNECTED		40	/*%< socket is not connected */
#define ISC_R_RANGE			41	/*%< out of range */
#define ISC_R_NOENTROPY			42	/*%< out of entropy */
#define ISC_R_MULTICAST			43	/*%< invalid use of multicast */
#define ISC_R_NOTFILE			44	/*%< not a file */
#define ISC_R_NOTDIRECTORY		45	/*%< not a directory */
#define ISC_R_QUEUEFULL			46	/*%< queue is full */
#define ISC_R_FAMILYMISMATCH		47	/*%< address family mismatch */
#define ISC_R_FAMILYNOSUPPORT		48	/*%< AF not supported */
#define ISC_R_BADHEX			49	/*%< bad hex encoding */
#define ISC_R_TOOMANYOPENFILES		50	/*%< too many open files */
#define ISC_R_NOTBLOCKING		51	/*%< not blocking */
#define ISC_R_UNBALANCEDQUOTES		52	/*%< unbalanced quotes */
#define ISC_R_INPROGRESS		53	/*%< operation in progress */
#define ISC_R_CONNECTIONRESET		54	/*%< connection reset */
#define ISC_R_SOFTQUOTA			55	/*%< soft quota reached */
#define ISC_R_BADNUMBER			56	/*%< not a valid number */
#define ISC_R_DISABLED			57	/*%< disabled */
#define ISC_R_MAXSIZE			58	/*%< max size */
#define ISC_R_BADADDRESSFORM		59	/*%< invalid address format */
#define ISC_R_BADBASE32			60	/*%< bad base32 encoding */
#define ISC_R_UNSET			61	/*%< unset */

/*% Not a result code: the number of results. */
#define ISC_R_NRESULTS 			62
#endif
/*
 * DHCP result codes
 */

/*
 * In the previous code the results started at 36
 * rather than ISC_RESULTCLASS_DHCP + 0
 * ISC_R_NOTCONNECTED was + 4 (40), it has been superseded by the isc version
 */

#define DHCP_R_HOSTUNKNOWN	(ISC_RESULTCLASS_DHCP + 0)
#define DHCP_R_VERSIONMISMATCH	(ISC_RESULTCLASS_DHCP + 1)
#define DHCP_R_PROTOCOLERROR	(ISC_RESULTCLASS_DHCP + 2)
#define DHCP_R_INVALIDARG	(ISC_RESULTCLASS_DHCP + 3)
#define DHCP_R_NOTYET		(ISC_RESULTCLASS_DHCP + 4)
#define DHCP_R_UNCHANGED	(ISC_RESULTCLASS_DHCP + 5)
#define DHCP_R_MULTIPLE		(ISC_RESULTCLASS_DHCP + 6)
#define DHCP_R_KEYCONFLICT	(ISC_RESULTCLASS_DHCP + 7)
#define DHCP_R_BADPARSE		(ISC_RESULTCLASS_DHCP + 8)
#define DHCP_R_NOKEYS		(ISC_RESULTCLASS_DHCP + 9)
#define DHCP_R_KEY_UNKNOWN	(ISC_RESULTCLASS_DHCP + 10)
#define DHCP_R_INVALIDKEY	(ISC_RESULTCLASS_DHCP + 11)
#define DHCP_R_INCOMPLETE	(ISC_RESULTCLASS_DHCP + 12)
#define DHCP_R_FORMERR		(ISC_RESULTCLASS_DHCP + 13)
#define DHCP_R_SERVFAIL		(ISC_RESULTCLASS_DHCP + 14)
#define DHCP_R_NXDOMAIN		(ISC_RESULTCLASS_DHCP + 15)
#define DHCP_R_NOTIMPL		(ISC_RESULTCLASS_DHCP + 16)
#define DHCP_R_REFUSED		(ISC_RESULTCLASS_DHCP + 17)
#define DHCP_R_YXDOMAIN		(ISC_RESULTCLASS_DHCP + 18)
#define DHCP_R_YXRRSET		(ISC_RESULTCLASS_DHCP + 19)
#define DHCP_R_NXRRSET		(ISC_RESULTCLASS_DHCP + 20)
#define DHCP_R_NOTAUTH		(ISC_RESULTCLASS_DHCP + 21)
#define DHCP_R_NOTZONE		(ISC_RESULTCLASS_DHCP + 22)
#define DHCP_R_BADSIG		(ISC_RESULTCLASS_DHCP + 23)
#define DHCP_R_BADKEY		(ISC_RESULTCLASS_DHCP + 24)
#define DHCP_R_BADTIME		(ISC_RESULTCLASS_DHCP + 25)
#define DHCP_R_NOROOTZONE	(ISC_RESULTCLASS_DHCP + 26)
#define DHCP_R_DESTADDRREQ	(ISC_RESULTCLASS_DHCP + 27)
#define DHCP_R_CROSSZONE	(ISC_RESULTCLASS_DHCP + 28)
#define DHCP_R_NO_TSIG		(ISC_RESULTCLASS_DHCP + 29)
#define DHCP_R_NOT_EQUAL	(ISC_RESULTCLASS_DHCP + 30)
#define DHCP_R_CONNRESET	(ISC_RESULTCLASS_DHCP + 31)
#define DHCP_R_UNKNOWNATTRIBUTE	(ISC_RESULTCLASS_DHCP + 32)

#define DHCP_R_NRESULTS 	33	/*%< Number of results */

// Included for historical reasons, these should be removed as
// soon as reasonable
#define ISC_R_HOSTUNKNOWN	DHCP_R_HOSTUNKNOWN
#define ISC_R_VERSIONMISMATCH	DHCP_R_VERSIONMISMATCH
#define ISC_R_PROTOCOLERROR	DHCP_R_PROTOCOLERROR	
#define ISC_R_INVALIDARG	DHCP_R_INVALIDARG	
#define ISC_R_NOTYET		DHCP_R_NOTYET		
#define ISC_R_UNCHANGED		DHCP_R_UNCHANGED		
#define ISC_R_MULTIPLE		DHCP_R_MULTIPLE		
#define ISC_R_KEYCONFLICT	DHCP_R_KEYCONFLICT	
#define ISC_R_BADPARSE		DHCP_R_BADPARSE		
#define ISC_R_NOKEYS		DHCP_R_NOKEYS		
#define ISC_R_KEY_UNKNOWN	DHCP_R_KEY_UNKNOWN	
#define ISC_R_INVALIDKEY	DHCP_R_INVALIDKEY	
#define ISC_R_INCOMPLETE	DHCP_R_INCOMPLETE	
#define ISC_R_FORMERR		DHCP_R_FORMERR		
#define ISC_R_SERVFAIL		DHCP_R_SERVFAIL		
#define ISC_R_NXDOMAIN		DHCP_R_NXDOMAIN		
#define ISC_R_NOTIMPL		DHCP_R_NOTIMPL		
#define ISC_R_REFUSED		DHCP_R_REFUSED		
#define ISC_R_YXDOMAIN		DHCP_R_YXDOMAIN		
#define ISC_R_YXRRSET		DHCP_R_YXRRSET		
#define ISC_R_NXRRSET		DHCP_R_NXRRSET		
#define ISC_R_NOTAUTH		DHCP_R_NOTAUTH		
#define ISC_R_NOTZONE		DHCP_R_NOTZONE		
#define ISC_R_BADSIG		DHCP_R_BADSIG		
#define ISC_R_BADKEY		DHCP_R_BADKEY		
#define ISC_R_BADTIME		DHCP_R_BADTIME		
#define ISC_R_NOROOTZONE	DHCP_R_NOROOTZONE	
#define ISC_R_DESTADDRREQ	DHCP_R_DESTADDRREQ	
#define ISC_R_CROSSZONE		DHCP_R_CROSSZONE		
#define ISC_R_NO_TSIG		DHCP_R_NO_TSIG		
#define ISC_R_NOT_EQUAL		DHCP_R_NOT_EQUAL		
#define ISC_R_CONNRESET		DHCP_R_CONNRESET		
#define ISC_R_UNKNOWNATTRIBUTE	DHCP_R_UNKNOWNATTRIBUTE	

isc_result_t
dhcp_result_register(void);

#endif /* DHCP_RESULT_H */
