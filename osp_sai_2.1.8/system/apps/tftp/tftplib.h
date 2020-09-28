/*
 * Copyright (c) 1993
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
 *
 *	@(#)extern.h	8.1 (Berkeley) 6/6/93
 */
#ifndef TFTPSUBS_H__
#define TFTPSUBS_H__

extern int g_code;

#define INVALID 	1
#define TOOSMALL 	2
#define TOOLARGE 	3

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif

#define TIMEOUT		5		/* packet rexmt timeout */
#define TIMEOUT_MIN	1		/* minimal packet rexmt timeout */
#define TIMEOUT_MAX	255		/* maximal packet rexmt timeout */

#ifndef OACK
#define OACK    06          /* option acknowledgement */
#endif

#ifndef SEGSIZE_MAX
#define SEGSIZE_MIN 8       /* minimal data segment size */
#define SEGSIZE_MAX 65464       /* maximal data segment size */
#endif

#ifndef INFTIM
#define INFTIM          (-1)     /* infinite poll timeout */
#endif

#ifndef EOPTNEG
#define EOPTNEG     8       /* option negotiation failed */
#endif

/*
 * Prototypes for read-ahead/write-behind subroutines for tftp user and
 * server.
 */
struct tftphdr	*r_init(void);
void		 read_ahead(FILE *, int, int);
int		 readit(FILE *, struct tftphdr **, int, int);

int		synchnet(int);

struct tftphdr	*w_init(void);
int		 write_behind(FILE *, int);
int		 writeit(FILE *, struct tftphdr **, int, int);

void	recvfile(int, char *, char *);
void	sendfile(int, char *, char *);

long long strtonum(const char *numstr, long long minval, 
                   long long maxval, const char **errstrp);
size_t strlcpy(char *dst, const char *src, size_t siz);

#endif /* ! TFTPSUBS_H__ */
