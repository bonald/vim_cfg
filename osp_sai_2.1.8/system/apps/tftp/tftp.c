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
 * TFTP User Program -- Command Interface
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */
#ifndef _GNU_SOURCE
/*modified by weij for bug 14248, 2011-03-16*/
#define _GNU_SOURCE
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/tftp.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tftplib.h"

#ifndef __dead
#define __dead
#endif

#define	LBUFLEN		200		/* size of input buffer */
#define	MAXARGV		20
#define HELPINDENT	(sizeof("connect"))

void			 get(int, char **);
void			 help(int, char **);
void			 modecmd(int, char **);
void			 put(int, char **);
void			 quit(int, char **);
void			 setascii(int, char **);
void			 setbinary(int, char **);
void		     setpeer(int, char **);
void			 setrexmt(int, char **);
void			 settimeout(int, char **);
void			 settrace(int, char **);
void			 setverbose(int, char **);
void			 settsize(int, char **);
void			 settout(int, char **);
void			 setblksize(int, char **);
void			 status(int, char **);
int			 readcmd(char *, int, FILE *);
static void		 getusage(char *);
static int		 makeargv(void);
static void		 putusage(char *);
static void		 settftpmode(char *);
static __dead void	 command(void);
struct cmd	 	*getcmd(char *);
char			*tail(char *);
union tftp_sockun {
	struct sockinet {
		u_short	si_family;
		u_short	si_port;
	} su_si;
	struct	sockaddr		su_sa;
	struct	sockaddr_in  		su_sin;
	struct	sockaddr_in6 		su_sin6;
};
#define	su_family	su_sa.sa_family
#define	su_port		su_si.si_port
static union tftp_sockun hisctladdr;
struct sockaddr_in	 peeraddr;
struct	sockaddr_in6 		peeraddr6;
int			 f;
int			 f4;
int			 f6;
short			 port;
int			 trace;
int			 verbose;
int			 connected;
char			 mode[32];
char			 line[LBUFLEN];
int			 margc;
char			*margv[MAXARGV+1];
char			*prompt = "tftp";
void			 intr(int);
struct servent		*sp;
int	 		 rexmtval = TIMEOUT;
int	 		 maxtimeout = 5 * TIMEOUT;
char	 		 hostname[MAXHOSTNAMELEN];
FILE			*file = NULL;
volatile sig_atomic_t	 intrflag = 0;
char			*ackbuf;
int			 has_options = 0;
int			 opt_tsize = 0;
int			 opt_tout = 0;
int			 opt_blksize = 0;

char	vhelp[] = "toggle verbose mode";
char	thelp[] = "toggle packet tracing";
char	chelp[] = "connect to remote tftp";
char	qhelp[] = "exit tftp";
char	hhelp[] = "print help information";
char	shelp[] = "send file";
char	rhelp[] = "receive file";
char	mhelp[] = "set file transfer mode";
char	sthelp[] = "show current status";
char	xhelp[] = "set per-packet retransmission timeout";
char	ihelp[] = "set total retransmission timeout";
char	ashelp[] = "set mode to netascii";
char	bnhelp[] = "set mode to octet";
char	oshelp[] = "toggle tsize option";
char	othelp[] = "toggle timeout option";
char	obhelp[] = "set alternative blksize option";

struct cmd {
	char	*name;
	char	*help;
	void	 (*handler)(int, char **);
};

struct cmd cmdtab[] = {
	{ "connect",	chelp,	setpeer},
	{ "mode",       mhelp,	modecmd },
	{ "put",	shelp,	put },
	{ "get",	rhelp,	get },
	{ "quit",	qhelp,	quit },
	{ "verbose",	vhelp,	setverbose },
	{ "trace",	thelp,	settrace },
	{ "status",	sthelp,	status },
	{ "binary",     bnhelp,	setbinary },
	{ "ascii",      ashelp,	setascii },
	{ "rexmt",	xhelp,	setrexmt },
	{ "timeout",	ihelp,	settimeout },
	{ "tsize",	oshelp, settsize },
	{ "tout",	othelp, settout },
	{ "blksize",	obhelp,	setblksize },
	{ "help",	hhelp,	help },
	{ "?",		hhelp,	help },
	{ NULL,		NULL,	NULL }
};

struct	modes {
	char	*m_name;
	char	*m_mode;
} modes[] = {
	{ "ascii",	"netascii" },
	{ "netascii",	"netascii" },
	{ "binary",	"octet" },
	{ "image",	"octet" },
	{ "octet",	"octet" },
/*	{ "mail",	"mail" }, */
	{ NULL,		NULL }
};

#define M_BUF_LEN   128
char cmdType[M_BUF_LEN];
int bCmdType = 0;
int bHostName = 0;
int nPort = 0;
char szLocalFile[M_BUF_LEN];
char szRemoteFile[M_BUF_LEN];

void printusage()
{
	printf("usage: tftp -v -Z host-or-ip -m mode -T timeout -w rextm-timeout \\\n\t-s blksize -c (get | put) -p port -r remote-file -l local-file\n");
    printf("       -v for verbose\n");
    return;
}

int
main(int argc, char *argv[])
{
    int pargc;
    char **pargv;
	struct sockaddr_in	s_in;
	struct sockaddr_in6	s_in6;
    int opt;

    memset(cmdType, 0x00, M_BUF_LEN);
    bCmdType = 0;
    bHostName = 0;
    nPort = 0;
    memset(szLocalFile, 0x00, M_BUF_LEN);
    memset(szRemoteFile, 0x00, M_BUF_LEN);

	/* socket, bind */
	sp = getservbyname("tftp", "udp");
	if (sp == 0)
		errx(1, "udp/tftp: unknown service");
		
	f4 = socket(AF_INET, SOCK_DGRAM, 0);
	if (f < 0)
		err(3, "socket");
	f6 = socket(AF_INET6, SOCK_DGRAM, 0);
	if (f < 0)
		err(3, "socket");
	memset(&hisctladdr, 0, sizeof(hisctladdr));
	if (inet_aton(argv[6], &hisctladdr.su_sin.sin_addr)) {
	
		f = f4;
		bzero((char *)&s_in, sizeof(s_in));
	    s_in.sin_family = AF_INET;
		if (bind(f, (struct sockaddr *)&s_in, sizeof(s_in)) < 0)
		    err(1, "bind");
		
	}
	else if (inet_pton(AF_INET6,argv[6], &hisctladdr.su_sin6.sin6_addr)){
            f = f6;
            bzero((char *)&s_in6, sizeof(s_in6));
            s_in6.sin6_family= AF_INET6;
            if (bind(f, (struct sockaddr *)&s_in6, sizeof(s_in6)) < 0)
		        err(1, "bind");
	
	}	

	/* allocate memory for packets */
	if ((ackbuf = malloc(SEGSIZE_MAX + 4)) == NULL)
		err(1, "malloc");

	/* set default transfer mode */
	strlcpy(mode, "netascii", sizeof(mode));

	/* catch SIGINT */
	signal(SIGINT, intr);

    /* Reset optind */
    optind = 1;

    while (1)
    {
        if (EOF == (opt = getopt(argc, argv, "vZ:m:s:w:c:p:l:r:htT:")))
        {
            break;
        }
        
        switch(opt)
        {
        case 't': /* trace */
            snprintf(line, LBUFLEN, "trace");
            makeargv();
            settrace(margc, margv);
            break;
        case 'v': /* verbose */
            snprintf(line, LBUFLEN, "verbose");
            makeargv();
            setverbose(margc, margv);
            break;
        case 'Z': /* host address */
            snprintf(hostname, MAXHOSTNAMELEN, "%s", optarg);
            bHostName = 1;
            break;
        case 'm': /* mode */
            snprintf(line, LBUFLEN, "mode %s", optarg);
            makeargv();
            modecmd(margc, margv);
            break;
        case 's': /* segment size */
            snprintf(line, LBUFLEN, "blksize %s", optarg);
            makeargv();
            setblksize(margc, margv);
            break;
        case 'T': /* timeout */
            snprintf(line, LBUFLEN, "timeout %s", optarg);
            makeargv();
            settimeout(margc, margv);
            break;
        case 'w': /* rextm timeout */
            snprintf(line, LBUFLEN, "rexmt %s", optarg);
            makeargv();
            setrexmt(margc, margv);
            break;
        case 'c': /* command type: get or put */
            bCmdType = 1;
            snprintf(cmdType, M_BUF_LEN, "%s", optarg);
            break;
        case 'p': /* port */
            nPort = atoi(optarg);
            break;
        case 'l': /* local filename */
            snprintf(szLocalFile, M_BUF_LEN, "%s", optarg);
            break;
        case 'r': /* remote filename */
            snprintf(szRemoteFile, M_BUF_LEN, "%s", optarg);
            break;
            break;
        case 'h':
        default: 
            printusage(); 
            exit(0);
            break;
        }
    } /* while (1) */

    if (bCmdType)
    {
        /* do command in batch mode */
        struct cmd *pcmd = NULL;
        
        pargc = argc - optind;
        pargv = argv + optind;

        if (pargc > 0)
        {
            fprintf(stderr, "Too many argument error.\r\n");
            exit(1);
        }

	    /* set peer if given */
        if (!bHostName)
        {
            fprintf(stderr, "Please setting peer host name or ip address.\r\n");
            exit(1);
        }
        if ('\0' == szLocalFile[0] && '\0' == szRemoteFile[0])
        {
		    fprintf(stderr, "Please setting file name.\r\n");
            exit(1);
            
        }
        if (nPort > 0)
        {
            snprintf(line, LBUFLEN, "tftp %s %d", hostname, nPort);
        }
        else
        {
            snprintf(line, LBUFLEN, "tftp %s", hostname);
        }
        makeargv();
        connected = 0;
        setpeer(margc, margv);
        if (0 == connected)
        {
            /* set peer fail */
            exit(1);
        }

        /* generate command and execute it */ 
        if (!strcmp(cmdType, "get"))
        {
            if ('\0' == szRemoteFile[0])
            {
                fprintf(stderr, "please setting remote file name.\r\n");
                exit(1);
            }

            if ('\0' != szLocalFile[0])
            {
                snprintf(line, LBUFLEN, "get %s %s", szRemoteFile, szLocalFile);
            }
            else
            {
                snprintf(line, LBUFLEN, "get %s", szRemoteFile);
            }
        }
        else if (!strcmp(cmdType, "put"))
        {
            if ('\0' == szLocalFile[0])
            {
                fprintf(stderr, "please setting local file name\r\n");
                exit(1);
            }

            if ('\0' != szRemoteFile[0])
            {
                snprintf(line, LBUFLEN, "put %s %s", szLocalFile, szRemoteFile);
            }
            else
            {
                snprintf(line, LBUFLEN, "put %s", szLocalFile);
            }
        }
        else
        {
            fprintf(stderr, "Command type error! Current support commands: get, put.\r\n");
            exit(1);
        }

        makeargv();
        pcmd = getcmd(margv[0]);
        if ((struct cmd *)-1 == pcmd || (struct cmd *)0 == pcmd)
        {
            fprintf(stderr, "Invalid command: %s.\r\n", margv[0]);
            exit(-1);
        }
        (*pcmd->handler)(margc, margv);
        if (0 != g_code || intrflag)
        {
        /*modified by dingyi for bug10213, 2009-09-23*/
        #ifndef _CENTEC_
            exit(1);
        #else
            exit (g_code);
        #endif
        }
        return 0;
    }
    else
    {
        pargc = argc - optind;
        pargv = argv + optind;
	    /* set peer if given */
    	if (pargc > 1)
        {
	    	setpeer(pargc, pargv);
        }
	    /* command prompt */
	    command();
    }

	return (0);
}

void
setpeer(int argc, char *argv[])
{
	struct hostent	*host;
	const char	*errstr;

	if (argc < 2) {
		strlcpy(line, "Connect ", sizeof(line));
		printf("(to) ");
		readcmd(&line[strlen(line)], LBUFLEN - strlen(line), stdin);
		if (makeargv())
			return;
		argc = margc;
		argv = margv;
	}
	if ((argc < 2) || (argc > 3)) {
		printf("usage: %s [host [port]]\n", argv[0]);
		return;
	}
	if (inet_aton(argv[1], &peeraddr.sin_addr) != 0) {
		peeraddr.sin_family = AF_INET;
		(void)strncpy(hostname, argv[1], sizeof(hostname));
		hostname[sizeof(hostname) - 1] = '\0';
	} 
    else if (inet_pton(AF_INET6,argv[1], &peeraddr6.sin6_addr) != 0){
        peeraddr6.sin6_family = AF_INET6;
        (void)strncpy(hostname, argv[1], sizeof(hostname));
        hostname[sizeof(hostname) - 1] = '\0';
    }
	else {
		host = gethostbyname(argv[1]);
		if (host == 0) {
			connected = 0;
			printf("%s: unknown host\n", argv[1]);
			return;
		}
		if (host->h_addrtype == AF_INET){
		peeraddr.sin_family = host->h_addrtype;
		bcopy(host->h_addr, &peeraddr.sin_addr, host->h_length);
		(void)strlcpy(hostname, host->h_name, sizeof(hostname));
		}
		else{
        peeraddr6.sin6_family = host->h_addrtype;
        bcopy(host->h_addr, &peeraddr6.sin6_addr, host->h_length);
		(void)strlcpy(hostname, host->h_name, sizeof(hostname));
		}
		
	}
	port = sp->s_port;
	if (argc == 3) {
		port = strtonum(argv[2], 1, 65535, &errstr);
		if (errstr) {
			printf("%s: port number is %s\n", argv[2], errstr);
			connected = 0;
			return;
		}
		port = htons(port);
	}
	connected = 1;
    return;
}

void
modecmd(int argc, char *argv[])
{
	struct modes	*p;
	char		*sep;

	if (argc < 2) {
		printf("Using %s mode to transfer files.\n", mode);
		return;
	}
	if (argc == 2) {
		for (p = modes; p->m_name != NULL; p++)
			if (strcmp(argv[1], p->m_name) == 0)
				break;
		if (p->m_name) {
			settftpmode(p->m_mode);
			return;
		}
		printf("%s: unknown mode\n", argv[1]);
		/* drop through and print usage message */
	}

	printf("usage: %s [", argv[0]);
	sep = " ";
	for (p = modes; p->m_name != NULL; p++) {
		printf("%s%s", sep, p->m_name);
		if (*sep == ' ')
			sep = " | ";
	}
	printf(" ]\n");

	return;
}

void
setbinary(int argc, char *argv[])
{
	settftpmode("octet");
}

void
setascii(int argc, char *argv[])
{
	settftpmode("netascii");
}

static void
settftpmode(char *newmode)
{
	strlcpy(mode, newmode, sizeof(mode));
	if (verbose)
		printf("mode set to %s\n", mode);
}

/*
 * Send file(s).
 */
void
put(int argc, char *argv[])
{
	int	 fd;
	int	 n;
	char	*cp, *targ;

    g_code = -1;
	if (argc < 2) {
		strlcpy(line, "send ", sizeof(line));
		printf("(file) ");
		readcmd(&line[strlen(line)], LBUFLEN - strlen(line), stdin);
		if (makeargv())
			return;
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		putusage(argv[0]);
		return;
	}
	targ = argv[argc - 1];
	if (strchr(argv[argc - 1], ':')) {
		char		*cp;
		struct hostent	*hp;

		for (n = 1; n < argc - 1; n++)
			if (strchr(argv[n], ':')) {
				putusage(argv[0]);
				return;
			}
		cp = argv[argc - 1];
		targ = strchr(cp, ':');
		*targ++ = 0;
		hp = gethostbyname(cp);
		if (hp == NULL) {
			warnx("%s: %s", cp, hstrerror(h_errno));
			return;
		}
		if (hp->h_addrtype == AF_INET){
    		bcopy(hp->h_addr, (caddr_t)&peeraddr.sin_addr, hp->h_length);
    		peeraddr.sin_family = hp->h_addrtype;
    		connected = 1;
    		port = sp->s_port;
    		strlcpy(hostname, hp->h_name, sizeof(hostname));
		}
		else{
		    bcopy(hp->h_addr, (caddr_t)&peeraddr6.sin6_addr, hp->h_length);
    		peeraddr6.sin6_family = hp->h_addrtype;
    		connected = 1;
    		port = sp->s_port;
    		strlcpy(hostname, hp->h_name, sizeof(hostname));
		}
	}
	if (!connected) {
		printf("No target machine specified.\n");
		return;
	}
	if (argc < 4) {
		cp = argc == 2 ? tail(targ) : argv[1];
		fd = open(cp, O_RDONLY);
		if (fd < 0) {
			warn("open: %s", cp);
			return;
		}
		if (verbose)
			printf("putting %s to %s:%s [%s]\n",
			    cp, hostname, targ, mode);
		if (f == f4){ 
		peeraddr.sin_port = port;
		}
		if (f == f6){ 
		peeraddr6.sin6_port = port;
		}
		sendfile(fd, targ, mode);
		return;
	}

	/*
	 * this assumes the target is a directory on
	 * on a remote unix system.  hmmmm.
	 */
	for (n = 1; n < argc - 1; n++) {
		if (asprintf(&cp, "%s/%s", targ, tail(argv[n])) == -1)
			err(1, "asprintf");
		fd = open(argv[n], O_RDONLY);
		if (fd < 0) {
			warn("open: %s", argv[n]);
			free(cp);
			continue;
		}
		if (verbose)
			printf("putting %s to %s:%s [%s]\n",
			    argv[n], hostname, cp, mode);
		if (f == f4){ 
		peeraddr.sin_port = port;
		}
		if (f == f6){ 
		peeraddr6.sin6_port = port;
		}
		sendfile(fd, cp, mode);
		free(cp);
	}
}

static void
putusage(char *s)
{
	printf("usage: %s file [[host:]remotename]\n", s);
	printf("       %s file1 file2 ... fileN [[host:]remote-directory]\n",
	    s);
}

/*
 * Receive file(s).
 */
void
get(int argc, char *argv[])
{
	int	 fd;
	int	 n;
	char	*cp;
	char	*src;

    g_code = -1;
	if (argc < 2) {
		strlcpy(line, "get ", sizeof(line));
		printf("(files) ");
		readcmd(&line[strlen(line)], LBUFLEN - strlen(line), stdin);
		if (makeargv())
			return;
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		getusage(argv[0]);
		return;
	}
	if (!connected) {
		for (n = 1; n < argc; n++)
			if (strchr(argv[n], ':') == 0) {
				getusage(argv[0]);
				return;
			}
	}
	for (n = 1; n < argc; n++) {
		src = strchr(argv[n], ':');
		if (src == NULL)
			src = argv[n];
		else {
			struct hostent	*hp;

			*src++ = 0;
			hp = gethostbyname(argv[n]);
			if (hp == NULL) {
				warnx("%s: %s", argv[n], hstrerror(h_errno));
				continue;
			}
			if (hp->h_addrtype == AF_INET){
    			bcopy(hp->h_addr, (caddr_t)&peeraddr.sin_addr,
    			    hp->h_length);
    			peeraddr.sin_family = hp->h_addrtype;
    			connected = 1;
    			strlcpy(hostname, hp->h_name, sizeof(hostname));
			}
			else{
    			bcopy(hp->h_addr, (caddr_t)&peeraddr6.sin6_addr,
    			    hp->h_length);
    			peeraddr6.sin6_family = hp->h_addrtype;
    			connected = 1;
    			strlcpy(hostname, hp->h_name, sizeof(hostname));
			}
		
		}
		if (argc < 4) {
			cp = argc == 3 ? argv[2] : tail(src);
			fd = creat(cp, 0644);
			if (fd < 0) {
				warn("create: %s", cp);
				return;
			}
			if (verbose)
				printf("getting from %s:%s to %s [%s]\n",
				    hostname, src, cp, mode);
		    if (f == f4){
			peeraddr.sin_port = port;
			}
			if (f == f6){
			peeraddr6.sin6_port = port;
			}
			recvfile(fd, src, mode);
			break;
		}
		cp = tail(src);	/* new .. jdg */
		fd = creat(cp, 0644);
		if (fd < 0) {
			warn("create: %s", cp);
			continue;
		}
		if (verbose)
			printf("getting from %s:%s to %s [%s]\n",
			    hostname, src, cp, mode);
		if (f == f4){
			peeraddr.sin_port = port;
			}
		if (f == f6){
			peeraddr6.sin6_port = port;
			}
		recvfile(fd, src, mode);
	}
}

static void
getusage(char *s)
{
	printf("usage: %s [host:]file [localname]\n", s);
	printf("       %s [host1:]file1 [host2:]file2 ... [hostN:]fileN\n", s);
}

void
setrexmt(int argc, char *argv[])
{
	int		 t;
	const char	*errstr;

	if (argc < 2) {
		strlcpy(line, "Rexmt-timeout ", sizeof(line));
		printf("(value) ");
		readcmd(&line[strlen(line)], LBUFLEN - strlen(line), stdin);
		if (makeargv())
			return;
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
		printf("usage: %s value\n", argv[0]);
		return;
	}
	t = strtonum(argv[1], TIMEOUT_MIN, TIMEOUT_MAX, &errstr);
	if (errstr)
		printf("%s: value is %s\n", argv[1], errstr);
	else
		rexmtval = t;
}

void
settimeout(int argc, char *argv[])
{
	int		 t;
	const char	*errstr;

	if (argc < 2) {
		strlcpy(line, "Maximum-timeout ", sizeof(line));
		printf("(value) ");
		readcmd(&line[strlen(line)], LBUFLEN - strlen(line), stdin);
		if (makeargv())
			return;
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
		printf("usage: %s value\n", argv[0]);
		return;
	}
	t = strtonum(argv[1], TIMEOUT_MIN, TIMEOUT_MAX, &errstr);
	if (errstr)
		printf("%s: value is %s\n", argv[1], errstr);
	else
		maxtimeout = t;
}

void
status(int argc, char *argv[])
{
	if (connected)
		printf("Connected to %s.\n", hostname);
	else
		printf("Not connected.\n");
	printf("Mode: %s Verbose: %s Tracing: %s\n",
	    mode, verbose ? "on" : "off", trace ? "on" : "off");
	printf("Rexmt-interval: %d seconds, Max-timeout: %d seconds\n",
	    rexmtval, maxtimeout);
}

void
intr(int signo)
{
	intrflag = 1;
    printf("\nInterrupted by user...\n");
    g_code = -1;
    exit (-1);
}

char *
tail(char *filename)
{
	char	*s;

	while (*filename) {
		s = strrchr(filename, '/');
		if (s == NULL)
			break;
		if (s[1])
			return (s + 1);
		*s = '\0';
	}

	return (filename);
}

/*
 * Command parser.
 */
static __dead void
command(void)
{
	struct cmd	*c;

	for (;;) {
		printf("%s> ", prompt);
		if (readcmd(line, LBUFLEN, stdin) < 1)
			continue;
		if ((line[0] == 0) || (line[0] == '\n'))
			continue;
		if (makeargv())
			continue;
		if (margc == 0)
			continue;
		c = getcmd(margv[0]);
		if (c == (struct cmd *) - 1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == 0) {
			printf("?Invalid command\n");
			continue;
		}
		(*c->handler)(margc, margv);
	}
}

struct cmd *
getcmd(char *name)
{
	char		*p, *q;
	struct cmd	*c, *found;
	int		 nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	intrflag = 0;
	for (c = cmdtab; (p = c->name) != NULL; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *) - 1);

	return (found);
}

/*
 * Slice a string up into argc/argv.
 */
static int
makeargv(void)
{
	char	 *cp;
	char	**argp = margv;
	int	  ret = 0;

	margc = 0;
	for (cp = line; *cp;) {
		if (margc >= MAXARGV) {
			printf("too many arguments\n");
			ret = 1;
			break;
		}
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;

	return (ret);
}

void
quit(int argc, char *argv[])
{
	exit(0);
}

/*
 * Help command.
 */
void
help(int argc, char *argv[])
{
	struct cmd	*c;

	if (argc == 1) {
		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c->name != NULL; c++)
			printf("%-*s\t%s\n", (int)HELPINDENT, c->name, c->help);
		return;
	}
	while (--argc > 0) {
		char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *) - 1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == (struct cmd *)0)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%s\n", c->help);
	}
}

void
settrace(int argc, char *argv[])
{
	trace = !trace;
	printf("Packet tracing %s.\n", trace ? "on" : "off");
}

void
setverbose(int argc, char *argv[])
{
	verbose = !verbose;
	printf("Verbose mode %s.\n", verbose ? "on" : "off");
}

void
settsize(int argc, char *argv[])
{
	opt_tsize = !opt_tsize;
	printf("Tsize option %s.\n", opt_tsize ? "on" : "off");
	if (opt_tsize)
		has_options++;
	else
		has_options--;
}

void
settout(int argc, char *argv[])
{
	opt_tout = !opt_tout;
	printf("Timeout option %s.\n", opt_tout ? "on" : "off");
	if (opt_tout)
		has_options++;
	else
		has_options--;
}

void
setblksize(int argc, char *argv[])
{
	int		 t;
	const char	*errstr;

	if (argc < 2) {
		strlcpy(line, "Blocksize ", sizeof(line));
		printf("(value) ");
		readcmd(&line[strlen(line)], LBUFLEN - strlen(line), stdin);
		if (makeargv())
			return;
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
		printf("usage: %s value\n", argv[0]);
		return;
	}
	t = strtonum(argv[1], SEGSIZE_MIN, SEGSIZE_MAX, &errstr);
	if (errstr)
		printf("%s: value is %s\n", argv[1], errstr);
	else {
		if (opt_blksize == 0)
			has_options++;
		opt_blksize = t;
	}
}

int
readcmd(char *input, int len, FILE *stream)
{
	int		nfds;
	struct pollfd	pfd[1];

	fflush(stdout);

	pfd[0].fd = 0;
	pfd[0].events = POLLIN;
	nfds = poll(pfd, 1, INFTIM);
	if (nfds == -1) {
		if (intrflag) {
			intrflag = 0;
			putchar('\n');
			return (0);
		}
		exit(1);
	}

	if (fgets(input, len, stream) == NULL) {
		if (feof(stdin))
			exit(0);
		else
			return (-1);
	}

	return (1);
}
