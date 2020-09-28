/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * TFTP User Program -- Protocol Machines
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <arpa/tftp.h>

#include <err.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tftplib.h"

#define M_MAX_FILE_SIZE     (300 * 1024 * 1024)  /* 300M bytes */
#define MEMCHECK  (4 * 1024 *1024) /*4M*/

#define   SYSLIMIT_MEMCHECK_STATE_WARNING 1    /* > threshold1 & < threshold2 */
 
/* generate code for error process */
int g_code = 0;

int hashbytes = 0;
static void print_tail(const char *direction, unsigned long amount);

/* values for bf.counter */
#define BF_ALLOC	-3		/* alloc'd but not yet filled */
#define BF_FREE		-2		/* free */
/* [-1 .. SEGSIZE] = size of data in the data buffer */

static struct tftphdr	*rw_init(int);

struct bf {
	int	counter;		/* size of data in buffer, or flag */
	char	buf[SEGSIZE_MAX + 4];	/* room for data packet */
} bfs[2];

static int	nextone;	/* index of next buffer to use */
static int	current;	/* index of buffer in use */
				/* control flags for crlf conversions */
int		newline = 0;	/* fillbuf: in middle of newline expansion */
int		prevchar = -1;	/* putbuf: previous char (cr check) */

static int	makerequest(int, const char *, struct tftphdr *, const char *);
static void	nak(int);
static void 	tpacket(const char *, struct tftphdr *, int);
static void	startclock(void);
static void	stopclock(void);
static void	printstats(const char *, unsigned long);
static void print_progres(const char *direction, unsigned long amount);
static void	printtimeout(void);
static void	oack(struct tftphdr *, int, int);
static int	oack_set(const char *, const char *);

extern struct sockaddr_in	 peeraddr;	/* filled in by main */
extern struct	sockaddr_in6 		peeraddr6;
extern int			 f4;
extern int			 f6;
extern int			 f;		/* the opened socket */
extern int			 trace;
extern int			 verbose;
extern int			 rexmtval;
extern int			 maxtimeout;
extern FILE			*file;
extern volatile sig_atomic_t	 intrflag;
extern char			*ackbuf;
extern int			 has_options;
extern int			 opt_tsize;
extern int			 opt_tout;
extern int			 opt_blksize;

struct timeval	tstart;
struct timeval	tstop;
unsigned int	segment_size = SEGSIZE;
unsigned int	packet_size = SEGSIZE + 4;

struct errmsg {
	int	 e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ EOPTNEG,	"Option negotiation failed" },
	{ -1,		NULL }
};

/*add by dingyi for bug10213, 2009-09-23, reference to the OPER_ definitions in imi_file_snmp.h*/
#ifdef _CENTEC_
#define FILE_NOTFOUND 5
#define NETWORK_UNREACHABLE  7
#define UNKNOW_FAILURE 19
#endif

struct options {
	const char      *o_type;
} options[] = {
	{ "tsize" },
	{ "timeout" },
	{ "blksize" },
	{ NULL }
};

enum opt_enum {
	OPT_TSIZE = 0,
	OPT_TIMEOUT,
	OPT_BLKSIZE
};

#ifdef _CENTEC_
/*fix bug22224, liangf, 2013-2-1*/
int _memmgr_free_cached_mem()
{
  system("sync && echo 3 > /proc/sys/vm/drop_caches");
  return 0;
}

static int
_memory_check_reach_threshold()
{
    #define MAX_LINE_CHAR 128
    FILE *pFile = NULL;
    char strLine[MAX_LINE_CHAR] = {0};
    int mem_states = 0;

    pFile = fopen("/tmp/memchk_state", "r");
    if((NULL == pFile) || (feof(pFile)))
    {
        return -1;
    }
    _memmgr_free_cached_mem();
    
    memset(strLine, 0, sizeof(strLine));
    
    fgets(strLine, MAX_LINE_CHAR, pFile);
    sscanf(strLine, "%d", &mem_states);
    fclose(pFile);
    
    if (mem_states >= SYSLIMIT_MEMCHECK_STATE_WARNING)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

#endif
/*
 * Send the requested file.
 */
void
sendfile(int fd, char *name, char *mode)
{
	struct tftphdr		*dp, *ap; /* data and ack packets */
	struct sockaddr_in	 from;
	struct sockaddr_in6	 from6;
	struct pollfd		 pfd[1];
	unsigned long		 amount;
	int			 convert; /* true if converting crlf -> lf */
	int			 n, nfds, error, fromlen, timeouts, size;
    unsigned short block; /* xlliu */

	startclock();		/* start stat's clock */
	dp = r_init();		/* reset fillbuf/read-ahead code */
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "r");
	convert = !strcmp(mode, "netascii");
	block = 0;
	amount = 0;
    
/*modified by dingyi for bug10213, 2009-09-23*/
#ifndef _CENTEC_
    g_code = -1;
#else
    g_code = UNKNOW_FAILURE;
#endif
    hashbytes = 0;

	do {
		/* read data from file */
		if (!block)
			size = makerequest(WRQ, name, dp, mode) - 4;
		else {
			size = readit(file, &dp, convert, segment_size);
			if (size < 0) {
				nak(errno + 100);
				break;
			}
			dp->th_opcode = htons((u_short)DATA);
			dp->th_block = htons((u_short)block);
		}

		/* send data to server and wait for server ACK */
		for (timeouts = 0, error = 0; !intrflag;) {
			if (timeouts >= maxtimeout) {
				printtimeout();
				goto abort;
			}

			if (!error) {
				if (trace)
					tpacket("sent", dp, size + 4);
				if (f == f4){
    				if (sendto(f, dp, size + 4, 0,
    		    		    (struct sockaddr *)&peeraddr,
    				    sizeof(peeraddr)) != size + 4) {
    					warn("sendto");
                                /*add by dingyi for bug10213, 2009-09-23*/
                                #ifdef _CENTEC_
                                    g_code = NETWORK_UNREACHABLE;
                                #endif
    					goto abort;
    				}
				}
				if (f == f6){
    				if (sendto(f, dp, size + 4, 0,
    		    		    (struct sockaddr *)&peeraddr6,
    				    sizeof(peeraddr6)) != size + 4) {
    					warn("sendto");
                                /*add by dingyi for bug10213, 2009-09-23*/
                                #ifdef _CENTEC_
                                    g_code = NETWORK_UNREACHABLE;
                                #endif
    					goto abort;
    				}
				}
				if (block > 0)
					read_ahead(file, convert, segment_size);
			}
			error = 0;

			pfd[0].fd = f;
			pfd[0].events = POLLIN;
			nfds = poll(pfd, 1, rexmtval * 1000);
			if (nfds == 0) {
				timeouts += rexmtval;
				continue;
			}
			if (nfds == -1) {
				error = 1;
				if (errno == EINTR)
					continue;
				warn("poll");
				goto abort;
			}
			if (f == f4){
    			fromlen = sizeof(from);
    /*modified by weij for bug 14248, 2011-03-16*/            
    			n = recvfrom(f, ackbuf, packet_size, 0,
    			    (struct sockaddr *)&from, (socklen_t *)&fromlen);
			    }
			   if (f == f6){
    			fromlen = sizeof(from6);
    /*modified by weij for bug 14248, 2011-03-16*/            
    			n = recvfrom(f, ackbuf, packet_size, 0,
    			    (struct sockaddr *)&from6, (socklen_t *)&fromlen);
			    } 
			if (n == 0) {
				warn("recvfrom");
				goto abort;
			}
			if (n == -1) {
				error = 1;
				if (errno == EINTR)
					continue;
				warn("recvfrom");
				goto abort;
			}
			if (f == f4){
			    peeraddr.sin_port = from.sin_port;	/* added */
			}
			if (f == f6){
			    peeraddr6.sin6_port = from6.sin6_port;	/* added */
			}
			
			if (trace)
				tpacket("received", ap, n);

			ap->th_opcode = ntohs(ap->th_opcode);

			if (ap->th_opcode == OACK) {
				oack(ap, n, 0);
				break;
			}

			ap->th_block = ntohs(ap->th_block);

			if (ap->th_opcode == ERROR) {
				printf("Error code %d: %s\n",
				    ap->th_code, ap->th_msg);
                            /*add by dingyi for bug10213, 2009-09-23*/
                            #ifdef _CENTEC_
                                if(ENOTFOUND == dp->th_code)
                                    g_code = FILE_NOTFOUND;
                            #endif
				goto abort;
			}
			if (ap->th_opcode == ACK) {
				int j;
				if (ap->th_block == block)
					break;
				/* re-synchronize with other side */
				j = synchnet(f);
				if (j && trace)
					printf("discarded %d packets\n", j);
				if (ap->th_block == (block - 1))
					continue;
			}
			error = 1;	/* received packet does not match */
		}

		if (block > 0)
			amount += size;
		block++;
        print_progres("Sent", amount);
	} while ((size == segment_size || block == 1) && !intrflag);

    g_code = 0;
abort:
    print_tail("Sent", amount);
	fclose(file);
	stopclock();
	if (amount >= 0) {
		if (intrflag)
			putchar('\n');
		printstats("Sent", amount);
	}
}

/*
 * Receive a file.
 */
void
recvfile(int fd, char *name, char *mode)
{
	struct tftphdr		*dp, *ap; /* data and ack packets */
	struct sockaddr_in	 from;
	struct sockaddr_in6	 from6;
	struct pollfd		 pfd[1];
	unsigned long		 amount;
	int			 convert; /* true if converting crlf -> lf */
	int			 n, nfds, error, fromlen, timeouts, size;
	int			 firsttrip;
    unsigned short block; /* xlliu */
    int memcheck = 0; /*fix bug22224, liangf, 2013-2-1*/
    
	startclock();		/* start stat's clock */
	dp = w_init();		/* reset fillbuf/read-ahead code */
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "w");
	convert = !strcmp(mode, "netascii");
	n = 0;
	block = 1;
	amount = 0;
	firsttrip = 1;

/*modified by dingyi for bug10213, 2009-09-23*/
#ifndef _CENTEC_
    g_code = -1;
#else
    g_code = UNKNOW_FAILURE;
#endif
    hashbytes = 0;

options:
	do {
		/* create new ACK packet */
		if (firsttrip) {
			size = makerequest(RRQ, name, ap, mode);
			firsttrip = 0;
		} else {
			ap->th_opcode = htons((u_short)ACK);
			ap->th_block = htons((u_short)(block));
			size = 4;
			block++;
		}

		/* send ACK to server and wait for server data */
		for (timeouts = 0, error = 0; !intrflag;) {
			if (timeouts >= maxtimeout) {
				printtimeout();
				goto abort;
			}

			if (!error) {
				if (trace)
					tpacket("sent", ap, size);
				if (f == f4){
				    if (sendto(f, ackbuf, size, 0,
			    	    (struct sockaddr *)&peeraddr,
				        sizeof(peeraddr)) != size) {
					    warn("sendto");
                            /*add by dingyi for bug10213, 2009-09-23*/
                            #ifdef _CENTEC_
                                    g_code = NETWORK_UNREACHABLE;
                            #endif
					    goto abort;
				    }
				}
				if (f == f6) {
				    if (sendto(f, ackbuf, size, 0,
			    	    (struct sockaddr *)&peeraddr6,
				    sizeof(peeraddr6)) != size) {
					warn("sendto"); 
                            #ifdef _CENTEC_
                                    g_code = NETWORK_UNREACHABLE;
                            #endif
					goto abort;
				    }
				}
				
				write_behind(file, convert);
			}
			error = 0;

			pfd[0].fd = f;
			pfd[0].events = POLLIN;
			nfds = poll(pfd, 1, rexmtval * 1000);
			if (nfds == 0) {
				timeouts += rexmtval;
				continue;
			}
			if (nfds == -1) {
				error = 1;
				if (errno == EINTR)
					continue;
				warn("poll");
				goto abort;
			}
			if (f == f4){
    			fromlen = sizeof(from);
                /*modified by weij for bug 14248, 2011-03-16*/    
    			n = recvfrom(f, dp, packet_size, 0,
    			    (struct sockaddr *)&from, (socklen_t *)&fromlen);
    			if (n == 0) {
    				warn("recvfrom");
    				goto abort;
    			}
    			if (n == -1) {
    				error = 1;
    				if (errno == EINTR)
    					continue;
    				warn("recvfrom");
    				goto abort;
    			}
    			peeraddr.sin_port = from.sin_port;	/* added */
		     }

		     if (f == f6){
    			fromlen = sizeof(from6);
                /*modified by weij for bug 14248, 2011-03-16*/    
    			n = recvfrom(f, dp, packet_size, 0,
    			    (struct sockaddr *)&from6, (socklen_t *)&fromlen);
    			if (n == 0) {
    				warn("recvfrom");
    				goto abort;
    			}
    			if (n == -1) {
    				error = 1;
    				if (errno == EINTR)
    					continue;
    				warn("recvfrom");
    				goto abort;
    			}
    			peeraddr6.sin6_port = from6.sin6_port;	/* added */
		     }
			if (trace)
				tpacket("received", dp, n);

			dp->th_opcode = ntohs(dp->th_opcode);

			if (dp->th_opcode == OACK) {
				oack(dp, n, 0);
				block = 0;
				goto options;
			}

			dp->th_block = ntohs(dp->th_block);

			if (dp->th_opcode == ERROR) {
				printf("Error code %d: %s\n",
				    dp->th_code, dp->th_msg);
                       /*add by dingyi for bug10213, 2009-09-23*/
                       #ifdef _CENTEC_
                           if(dp->th_code == ENOTFOUND)
                               g_code = FILE_NOTFOUND;
                       #endif
				goto abort;
			}
			if (dp->th_opcode == DATA) {
				int j;
				if (dp->th_block == block)
					break;
				/* re-synchronize with other side */
				j = synchnet(f);
				if (j && trace)
					printf("discarded %d packets\n", j);
				if (dp->th_block == (block - 1))
					continue;
			}
			error = 1;	/* received packet does not match */
		}

		/* write data to file */
		size = writeit(file, &dp, n - 4, convert);
		if (size < 0) {
			nak(errno + 100);
			break;
		}
		amount += size;
        print_progres("Received", amount);
        /*bug21865 remove file size limit. */

#ifdef _CENTEC_ /* Modified by Alexander Liu 2007-08-21 15:52 */
        /*fix bug22224, liangf, 2013-2-1*/
        if (amount > M_MAX_FILE_SIZE)
        {
            printf("\n%% File too large error. The maximal size is %d bytes",
                    M_MAX_FILE_SIZE);
            nak(ENOSPACE);
            g_code = -ENOSPACE;
            goto abort;
        }
        /* fix bug22224, liangf, 2013-2-1*/
        if(amount >= memcheck) 
        {
            if((_memory_check_reach_threshold() < 0))
            {
                printf("\n%% Exceed system memory threshold, no free memory\n");            
                goto abort;
            }
            memcheck += MEMCHECK;
        }

#endif /* !_CENTEC_ */
	} while (size == segment_size && !intrflag);

    if (!intrflag)
    {
        g_code = 0;
    }
abort:
	/* ok to ack, since user has seen err msg */
	ap->th_opcode = htons((u_short)ACK);
	ap->th_block = htons((u_short)block);
	if (f == f4){
	    (void)sendto(f, ackbuf, 4, 0, (struct sockaddr *)&peeraddr,sizeof(peeraddr));
	}
	if (f == f6){
	    (void)sendto(f, ackbuf, 4, 0, (struct sockaddr *)&peeraddr6,sizeof(peeraddr6));
	}
	write_behind(file, convert);	/* flush last buffer */

    /*modified by liul for bug 21937, 2013-01-14*/
    system("sync");

    print_tail("Received", amount);
	fclose(file);
	stopclock();
	if (amount >= 0) {
		if (intrflag)
			putchar('\n');
		printstats("Received", amount);
	}
}

static int
makerequest(int request, const char *name, struct tftphdr *tp,
    const char *mode)
{
	char		*cp;
	int		 len, pktlen;
	off_t		 fsize = 0;
	struct stat	 st;

	tp->th_opcode = htons((u_short)request);
	cp = tp->th_stuff;
	pktlen = packet_size - offsetof(struct tftphdr, th_stuff);
	len = strlen(name) + 1;
	strlcpy(cp, name, pktlen);
	strlcpy(cp + len, mode, pktlen - len);
	len += strlen(mode) + 1;

	if (opt_tsize) {
		if (request == WRQ) {
			stat(name, &st);
			fsize = st.st_size;
		}
		len += snprintf(cp + len, pktlen - len, "%s%c%lld%c",
		    options[OPT_TSIZE].o_type, 0, (long long)fsize, 0);
	}
	if (opt_tout)
		len += snprintf(cp + len, pktlen - len, "%s%c%d%c",
		    options[OPT_TIMEOUT].o_type, 0, rexmtval, 0);
	if (opt_blksize)
		len += snprintf(cp + len, pktlen - len, "%s%c%d%c",
		    options[OPT_BLKSIZE].o_type, 0, opt_blksize, 0);

	return (cp + len - (char *)tp);
}

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
static void
nak(int error)
{
	struct errmsg	*pe;
	struct tftphdr	*tp;
	int		 length;

	tp = (struct tftphdr *)ackbuf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = strerror(error - 100);
		tp->th_code = EUNDEF;
	}
	length = strlcpy(tp->th_msg, pe->e_msg, packet_size) + 5;
	if (length > packet_size)
		length = packet_size;
	if (trace)
		tpacket("sent", tp, length);
	if (sendto(f, ackbuf, length, 0, (struct sockaddr *)&peeraddr,
	    sizeof(peeraddr)) != length)
		warn("nak");
}

static void
tpacket(const char *s, struct tftphdr *tp, int n)
{
	char		*cp, *file;
	static char	*opcodes[] =
	    { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR", "OACK" };

	u_short op = ntohs(tp->th_opcode);

	if (op < RRQ || op > OACK)
		printf("%s opcode=%x ", s, op);
	else
		printf("%s %s ", s, opcodes[op]);

	switch (op) {
	case RRQ:
	case WRQ:
		n -= 2;
		file = cp = tp->th_stuff;
		cp = strchr(cp, '\0');
		printf("<file=%s, mode=%s", file, cp + 1);
		if (has_options)
			oack(tp, n, 1);
		printf(">\n");
		break;
	case DATA:
		printf("<block=%d, %d bytes>\n", ntohs(tp->th_block), n - 4);
		break;
	case ACK:
		printf("<block=%d>\n", ntohs(tp->th_block));
		break;
	case ERROR:
		printf("<code=%d, msg=%s>\n", ntohs(tp->th_code), tp->th_msg);
		break;
	case OACK:
		printf("<");
		oack(tp, n, 1);
		printf(">\n");
		break;
	}
}

static void
startclock(void)
{
	(void)gettimeofday(&tstart, NULL);
}

static void
stopclock(void)
{
	(void)gettimeofday(&tstop, NULL);
}

#define HASHBYTES   (1024 * 50)

static void
print_tail(const char *direction, unsigned long amount)
{
    if (!intrflag)
    {
        if (amount > hashbytes)
        {
            (void) putchar('.');
            (void) fflush(stdout);
        }
    }
    return;
}

static void
print_progres(const char *direction, unsigned long amount)
{
    if (!intrflag)
    {
        while (amount >= hashbytes) 
        {
            (void) putchar('.');
            (void) fflush(stdout);
            hashbytes += HASHBYTES;
        }
    }
    return;
}

static void
printstats(const char *direction, unsigned long amount)
{
	double	delta;

	/* compute delta in 1/10's second units */
	delta = ((tstop.tv_sec * 10.) + (tstop.tv_usec / 100000)) -
	    ((tstart.tv_sec * 10.) + (tstart.tv_usec / 100000));
	delta = delta / 10.;	/* back to seconds */
    if (!intrflag)
    {
	    printf("\n%s %lu bytes in %.1f seconds", direction, amount, delta);
	    if (verbose)
		    printf(" [%.0f bits/sec]", (amount * 8.) / delta);
	    putchar('\n');
    }
}

static void
printtimeout(void)
{
	printf("Transfer timed out.\n");
}

static void
oack(struct tftphdr *tp, int size, int trace)
{
	int	 i, len, off;
	char	*opt, *val;

	u_short op = ntohs(tp->th_opcode);

#if defined (_CTC_X86_64_) || defined (_CTC_ARM_LS1023A_) || defined (_CTC_ARM_CTC5236_)
#ifndef _GLB_UML_SYSTEM_
    /* 25.26 server /usr/include/arpa/tftp.h */
	opt = tp->th_u1.tu_stuff;
	val = tp->th_u1.tu_stuff;
#else
	opt = tp->th_u.tu_stuff;
	val = tp->th_u.tu_stuff;
#endif
#else
	opt = tp->th_u.tu_stuff;
	val = tp->th_u.tu_stuff;
#endif

	if (op == RRQ || op == WRQ) {
		len = strlen(opt) + 1;
		opt = strchr(opt, '\0');
		opt++;
		len += strlen(opt) + 1;
		opt = strchr(opt, '\0');
		opt++;
		val = opt;
		off = len;
		if (trace)
			printf(", ");
	} else
		off = 2;

	for (i = off, len = 0; i < size - 1; i++) {
		if (*val != '\0') {
			val++;
			continue;
		}
		/* got option and value */
		val++;
		if (trace)
			printf("%s=%s", opt, val);
		else
			if (oack_set(opt, val) == -1)
				break;
		len = strlen(val) + 1;
		val += len;
		opt = val;
		i += len;
		if (trace && i < size - 1)
			printf(", ");
	}
}

int
oack_set(const char *option, const char *value)
{
	int		 i, n;
	const char	*errstr;

	for (i = 0; options[i].o_type != NULL; i++) {
		if (!strcasecmp(options[i].o_type, option)) {
			if (i == OPT_TSIZE) {
				/* XXX verify OACK response */
			}
			if (i == OPT_TIMEOUT) {
				/* verify OACK response */
				n = strtonum(value, TIMEOUT_MIN, TIMEOUT_MAX,
				    &errstr);
				if (errstr || rexmtval != n ||
				    opt_tout == 0) {
					nak(EOPTNEG);
					intrflag = 1;
					return (-1);
				}
				/* OK */
			}
			if (i == OPT_BLKSIZE) {
				/* verify OACK response */
				n = strtonum(value, SEGSIZE_MIN, SEGSIZE_MAX,
				    &errstr);
				if (errstr || opt_blksize != n ||
				    opt_blksize == 0) {
					nak(EOPTNEG);	
					intrflag = 1;
					return (-1);
				}
				/* OK, set option */
				segment_size = n;
				packet_size = segment_size + 4;
			}
		}
	}

	return (1);
}

struct tftphdr *
w_init(void)
{
	return (rw_init(0));	/* write-behind */
}

struct tftphdr *
r_init(void)
{
	return (rw_init(1));	/* read-ahead */
}

/*
 * Init for either read-ahead or write-behind.
 * Zero for write-behind, one for read-head.
 */
static struct tftphdr *
rw_init(int x)
{
	newline = 0;			/* init crlf flag */
	prevchar = -1;
	bfs[0].counter = BF_ALLOC;	/* pass out the first buffer */
	current = 0;
	bfs[1].counter = BF_FREE;
	nextone = x;			/* ahead or behind? */

	return ((struct tftphdr *)bfs[0].buf);
}

/*
 * Have emptied current buffer by sending to net and getting ack.
 * Free it and return next buffer filled with data.
 */
int
readit(FILE *file, struct tftphdr **dpp, int convert, int segment_size)
{
	struct bf	*b;

	bfs[current].counter = BF_FREE;		/* free old one */
	current = !current;			/* "incr" current */

	b = &bfs[current];			/* look at new buffer */
	if (b->counter == BF_FREE)		/* if it's empty */
		read_ahead(file, convert, segment_size);	/* fill it */
	/* assert(b->counter != BF_FREE); */	/* check */
	*dpp = (struct tftphdr *)b->buf;	/* set caller's ptr */

	return (b->counter);
}

/*
 * Fill the input buffer, doing ascii conversions if requested.
 * Conversions are lf -> cr, lf and cr -> cr, nul.
 */
void
read_ahead(FILE *file, int convert, int segment_size)
{
	int		 i;
	char		*p;
	int		 c;
	struct bf	*b;
	struct tftphdr	*dp;

	b = &bfs[nextone];			/* look at "next" buffer */
	if (b->counter != BF_FREE)		/* nop if not free */
		return;
	nextone = !nextone;			/* "incr" next buffer ptr */

	dp = (struct tftphdr *)b->buf;

	if (convert == 0) {
		b->counter = read(fileno(file), dp->th_data, segment_size);
		return;
	}

	p = dp->th_data;
	for (i = 0; i < segment_size; i++) {
		if (newline) {
			if (prevchar == '\n')
				c = '\n';	/* lf to cr, lf */
			else
				c = '\0';	/* cr to cr, nul */
			newline = 0;
		} else {
			c = getc(file);
			if (c == EOF)
				break;
			if (c == '\n' || c == '\r') {
				prevchar = c;
				c = '\r';
				newline = 1;
			}
		}
	       *p++ = c;
	}
	b->counter = (int)(p - dp->th_data);
}

/*
 * Update count associated with the buffer, get new buffer
 * from the queue.  Calls write_behind only if next buffer not
 * available.
 */
int
writeit(FILE *file, struct tftphdr **dpp, int ct, int convert)
{
	bfs[current].counter = ct;		/* set size of data to write */
	current = !current;			/* switch to other buffer */
	if (bfs[current].counter != BF_FREE)	/* if not free */
		/* flush it */
		(void)write_behind(file, convert);
	bfs[current].counter = BF_ALLOC;	/* mark as alloc'd */
	*dpp =  (struct tftphdr *)bfs[current].buf;

	return (ct);				/* this is a lie of course */
}

/*
 * Output a buffer to a file, converting from netascii if requested.
 * CR, NUL -> CR and CR, LF -> LF.
 * Note spec is undefined if we get CR as last byte of file or a
 * CR followed by anything else.  In this case we leave it alone.
 */
int
write_behind(FILE *file, int convert)
{
	char		*buf;
	int		 count;
	int		 ct;
	char		*p;
	int		 c; /* current character */
	struct bf	*b;
	struct tftphdr	*dp;

	b = &bfs[nextone];
	if (b->counter < -1)		/* anything to flush? */
		return (0);		/* just nop if nothing to do */

	count = b->counter;		/* remember byte count */
	b->counter = BF_FREE;		/* reset flag */
	dp = (struct tftphdr *)b->buf;
	nextone = !nextone;		/* incr for next time */
	buf = dp->th_data;

	if (count <= 0)			/* nak logic? */
		return (-1);

	if (convert == 0)
		return (write(fileno(file), buf, count));

	p = buf;
	ct = count;
	while (ct--) {				/* loop over the buffer */
		c = *p++;			/* pick up a character */
		if (prevchar == '\r') {		/* if prev char was cr */
			if (c == '\n')		/* if have cr,lf then just */
				/* smash lf on top of the cr */
				fseek(file, -1, 1);
			else if (c == '\0')	/* if have cr,nul then */
				goto skipit;	/* just skip over the putc */
			/* FALLTHROUGH */
		}
		putc(c, file);
skipit:
		prevchar = c;
	}

	return (count);
}

/*
 * When an error has occurred, it is possible that the two sides
 * are out of synch.  Ie: that what I think is the other side's
 * response to packet N is really their response to packet N-1.
 *
 * So, to try to prevent that, we flush all the input queued up
 * for us on the network connection on our host.
 *
 * We return the number of packets we flushed (mostly for reporting
 * when trace is active).
 */
int
synchnet(int f)
{
	int			i, j = 0;
	char			rbuf[SEGSIZE_MIN];
	struct sockaddr_in	from;
	socklen_t		fromlen;

	for (;;) {
		(void)ioctl(f, FIONREAD, &i);
		if (i) {
			j++;
			fromlen = sizeof(from);
			(void)recvfrom(f, rbuf, sizeof(rbuf), 0,
			    (struct sockaddr *)&from, &fromlen);
		} else
			return (j);
	}
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

long long strtonum(const char *numstr, long long minval, 
                   long long maxval, const char **errstrp)
{
	long long ll = 0;
	char *ep;
	int error = 0;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval)
		error = INVALID;
	else {
		ll = strtoll(numstr, &ep, 10);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
			error = TOOSMALL;
		else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}
