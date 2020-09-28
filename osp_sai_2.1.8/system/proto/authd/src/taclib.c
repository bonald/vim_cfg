/*-
 * Copyright (c) 1998, 2001, 2002, Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <assert.h>

#include "taclib_private.h"

#include "proto.h"
#include "iv.h"
#include "ctc_task.h"

#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_auth_cfg_define.h"
#include "gen/tbl_auth_cfg.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "gen/tbl_authd_debug_define.h"
#include "gen/tbl_authd_debug.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "authd.h"
#include "authd_debug.h"
#include "sal_base64.h"

DEBUG_DECLAR(authd_debug);

static int		 add_str_8(struct tac_handle *, u_int8_t *,
			    struct clnt_str *);
static int		 add_str_16(struct tac_handle *, u_int16_t *,
			    struct clnt_str *);
static int		 protocol_version(int, int, int);
static void		 crypt_msg(struct tac_handle *, struct tac_msg *);
static void		*dup_str(struct tac_handle *, const struct srvr_str *,
			    size_t *);
static void		 free_str(struct clnt_str *);
static void		 generr(struct tac_handle *, const char *, ...);
//			    __printflike(2, 3);
static void		 gen_session_id(struct tac_msg *);
static int		 get_srvr_end(struct tac_handle *);
static int		 get_srvr_str(struct tac_handle *, const char *,
				      struct srvr_str *, size_t);
static void		 init_clnt_str(struct clnt_str *);
static void		 init_srvr_str(struct srvr_str *);
static int		 validate_msg(struct tac_handle *);
static int		 save_str(struct tac_handle *, struct clnt_str *,
			    const void *, size_t);
static int		 send_msg(struct tac_handle *);
static void		*xmalloc(struct tac_handle *, size_t);
void              clear_srvr_avs(struct tac_handle *);
static void              create_msg(struct tac_handle *, int, int, int);

/* MD5 context. */
typedef struct {
    u_int state[4];		/* state (ABCD) */
    u_int count[2];		/* number of bits, modulo 2^64 (lsb first) */
    u_char buffer[64];		/* input buffer */
} myMD5_CTX;

#define MD5_memcpy memcpy
#define MD5_memset memset

static u_char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (u_int)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (u_int)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (u_int)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (u_int)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }


/* Encodes input (u_int) into output (u_char). Assumes len is
   a multiple of 4.
 */
static void Encode(output, input, len)
u_char *output;
u_int *input;
u_int len;
{
    u_int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
	output[j] = (u_char) (input[i] & 0xff);
	output[j + 1] = (u_char) ((input[i] >> 8) & 0xff);
	output[j + 2] = (u_char) ((input[i] >> 16) & 0xff);
	output[j + 3] = (u_char) ((input[i] >> 24) & 0xff);
    }
}

/* Decodes input (u_char) into output (u_int). Assumes len is
   a multiple of 4.
 */
static void Decode(output, input, len)
u_int *output;
u_char *input;
u_int len;
{
    u_int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
	output[i] = ((u_int) input[j]) | (((u_int) input[j + 1]) << 8) |
	    (((u_int) input[j + 2]) << 16) | (((u_int) input[j + 3]) <<
					      24);
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform(state, block)
u_int state[4];
u_char block[64];
{
    u_int a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    /* Round 1 */
    FF(a, b, c, d, x[0], S11, 0xd76aa478);	/* 1 */
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);	/* 2 */
    FF(c, d, a, b, x[2], S13, 0x242070db);	/* 3 */
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);	/* 4 */
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);	/* 5 */
    FF(d, a, b, c, x[5], S12, 0x4787c62a);	/* 6 */
    FF(c, d, a, b, x[6], S13, 0xa8304613);	/* 7 */
    FF(b, c, d, a, x[7], S14, 0xfd469501);	/* 8 */
    FF(a, b, c, d, x[8], S11, 0x698098d8);	/* 9 */
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);	/* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);	/* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be);	/* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122);	/* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193);	/* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e);	/* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821);	/* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[1], S21, 0xf61e2562);	/* 17 */
    GG(d, a, b, c, x[6], S22, 0xc040b340);	/* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51);	/* 19 */
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);	/* 20 */
    GG(a, b, c, d, x[5], S21, 0xd62f105d);	/* 21 */
    GG(d, a, b, c, x[10], S22, 0x2441453);	/* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);	/* 23 */
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);	/* 24 */
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);	/* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6);	/* 26 */
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);	/* 27 */
    GG(b, c, d, a, x[8], S24, 0x455a14ed);	/* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);	/* 29 */
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);	/* 30 */
    GG(c, d, a, b, x[7], S23, 0x676f02d9);	/* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);	/* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[5], S31, 0xfffa3942);	/* 33 */
    HH(d, a, b, c, x[8], S32, 0x8771f681);	/* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);	/* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c);	/* 36 */
    HH(a, b, c, d, x[1], S31, 0xa4beea44);	/* 37 */
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);	/* 38 */
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);	/* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);	/* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);	/* 41 */
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);	/* 42 */
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);	/* 43 */
    HH(b, c, d, a, x[6], S34, 0x4881d05);	/* 44 */
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);	/* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);	/* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);	/* 47 */
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);	/* 48 */

    /* Round 4 */
    II(a, b, c, d, x[0], S41, 0xf4292244);	/* 49 */
    II(d, a, b, c, x[7], S42, 0x432aff97);	/* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7);	/* 51 */
    II(b, c, d, a, x[5], S44, 0xfc93a039);	/* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3);	/* 53 */
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);	/* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d);	/* 55 */
    II(b, c, d, a, x[1], S44, 0x85845dd1);	/* 56 */
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);	/* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);	/* 58 */
    II(c, d, a, b, x[6], S43, 0xa3014314);	/* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1);	/* 60 */
    II(a, b, c, d, x[4], S41, 0xf7537e82);	/* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235);	/* 62 */
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);	/* 63 */
    II(b, c, d, a, x[9], S44, 0xeb86d391);	/* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information.
     */
    MD5_memset((u_char *) x, 0, sizeof(x));
}


/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init(myMD5_CTX *context)
{
    context->count[0] = context->count[1] = 0;
    /* Load magic initialization constants.
     */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
   operation, processing another message block, and updating the
   context.
 */
void MD5Update(myMD5_CTX *context, void *input, size_t inputLen)
{
    u_int index;
    size_t i, partLen;

    /* Compute number of bytes mod 64 */
    index = (u_int) ((context->count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((context->count[0] += ((u_int) inputLen << 3))
	< ((u_int) inputLen << 3))
	context->count[1]++;
    context->count[1] += ((u_int) inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible.
     */
    if (inputLen >= partLen) {
	MD5_memcpy
	    ((u_char *) & context->buffer[index], (u_char *) input,
	     partLen);
	MD5Transform(context->state, context->buffer);

	for (i = partLen; i + 63 < inputLen; i += 64)
	    MD5Transform(context->state, &((u_char *) input)[i]);

	index = 0;
    } else
	i = 0;

    /* Buffer remaining input */
    MD5_memcpy
	((u_char *) & context->buffer[index],
	 (u_char *) & ((u_char *) input)[i], inputLen - i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
   the message digest and zeroizing the context.
 */
void MD5Final(u_char *digest, myMD5_CTX *context)
{
    u_char bits[8];
    u_int index;
    size_t padLen;

    /* Save number of bits */
    Encode(bits, context->count, 8);

    /* Pad out to 56 mod 64.
     */
    index = (u_int) ((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(context, PADDING, padLen);

    /* Append length (before padding) */
    MD5Update(context, bits, (size_t) 8);

    /* Store state in digest */
    Encode(digest, context->state, 16);

    /* Zeroize sensitive information.
     */
    MD5_memset((u_char *) context, 0, sizeof(*context));
}

/*
 * Append some optional data to the current request, and store its
 * length into the 8-bit field referenced by "fld".  Returns 0 on
 * success, or -1 on failure.
 *
 * This function also frees the "cs" string data and initializes it
 * for the next time.
 */
static int
add_str_8(struct tac_handle *h, u_int8_t *fld, struct clnt_str *cs)
{
	u_int16_t len;

	if (add_str_16(h, &len, cs) == -1)
		return -1;
	len = ntohs(len);
	if (len > 0xff) {
		generr(h, "Field too long");
		return -1;
	}
	*fld = len;
	return 0;
}

/*
 * Append some optional data to the current request, and store its
 * length into the 16-bit field (network byte order) referenced by
 * "fld".  Returns 0 on success, or -1 on failure.
 *
 * This function also frees the "cs" string data and initializes it
 * for the next time.
 */
static int
add_str_16(struct tac_handle *h, u_int16_t *fld, struct clnt_str *cs)
{
	size_t len;

	len = cs->len;
	if (cs->data == NULL)
		len = 0;
	if (len != 0) {
		int offset;

		if (len > 0xffff) {
			generr(h, "Field too long");
			return -1;
		}
		offset = ntohl(h->request.length);
		if (offset + len > BODYSIZE) {
			generr(h, "Message too long");
			return -1;
		}
		memcpy(h->request.u.body + offset, cs->data, len);
		h->request.length = htonl(offset + len);
	}
	*fld = htons(len);
	free_str(cs);
	return 0;
}

static int
protocol_version(int msg_type, int var, int type)
{
    int minor;

    switch (msg_type) {
        case TAC_AUTHEN:
	    /* 'var' represents the 'action' */
	    switch (var) {
	        case TAC_AUTHEN_LOGIN:
		    switch (type) {

		        case TAC_AUTHEN_TYPE_PAP:
			case TAC_AUTHEN_TYPE_CHAP:
			case TAC_AUTHEN_TYPE_MSCHAP:
			case TAC_AUTHEN_TYPE_ARAP:
			    minor = 1;
			break;

			default:
			    minor = 0;
			break;
		     }
		break;

		case TAC_AUTHEN_SENDAUTH:
		    minor = 1;
		break;

		default:
		    minor = 0;
		break;
	    };
	break;

	case TAC_AUTHOR:
	    /* 'var' represents the 'method' */
	    switch (var) {
	        /*
		 * When new authentication methods are added, include 'method'
		 * in determining the value of 'minor'.  At this point, all
                 * methods defined in this implementation (see "Authorization 
                 * authentication methods" in taclib.h) are minor version 0
		 * Not all types, however, indicate minor version 0.
		 */
                case TAC_AUTHEN_METH_NOT_SET:
                case TAC_AUTHEN_METH_NONE:
                case TAC_AUTHEN_METH_KRB5:
                case TAC_AUTHEN_METH_LINE:
                case TAC_AUTHEN_METH_ENABLE:
                case TAC_AUTHEN_METH_LOCAL:
                case TAC_AUTHEN_METH_TACACSPLUS:
                case TAC_AUTHEN_METH_RCMD:
		    switch (type) {
		        case TAC_AUTHEN_TYPE_PAP:
			case TAC_AUTHEN_TYPE_CHAP:
			case TAC_AUTHEN_TYPE_MSCHAP:
			case TAC_AUTHEN_TYPE_ARAP:
			    minor = 1;
			break;

			default:
			    minor = 0;
			break;
		     }
	        break;
	        default:
		    minor = 0;
		break;
	    }
        break;

	case TAC_ACCT:
        minor = 3;

	default:
	    minor = 0;
        break;
    }

    return TAC_VER_MAJOR << 4 | minor;
}

/*
 * Encrypt or decrypt a message.  The operations are symmetrical.
 */
static void
crypt_msg(struct tac_handle *h, struct tac_msg *msg)
{
	const char *secret;
	myMD5_CTX base_ctx;
	myMD5_CTX ctx;
	unsigned char md5[16];
	int chunk;
	int msg_len;

	secret = h->secret;
	if (!secret || secret[0] == '\0')
		msg->flags |= TAC_UNENCRYPTED;
	if (msg->flags & TAC_UNENCRYPTED)
		return;

	msg_len = ntohl(msg->length);

	MD5Init(&base_ctx);
	MD5Update(&base_ctx, msg->session_id, sizeof msg->session_id);
	MD5Update(&base_ctx, (void *)secret, strlen(secret));
	MD5Update(&base_ctx, &msg->version, sizeof msg->version);
	MD5Update(&base_ctx, &msg->seq_no, sizeof msg->seq_no);

	ctx = base_ctx;
	for (chunk = 0;  chunk < msg_len;  chunk += sizeof md5) {
		int chunk_len;
		int i;

		MD5Final(md5, &ctx);

		if ((chunk_len = msg_len - chunk) > sizeof md5)
			chunk_len = sizeof md5;
		for (i = 0;  i < chunk_len;  i++)
			msg->u.body[chunk + i] ^= md5[i];

		ctx = base_ctx;
		MD5Update(&ctx, md5, sizeof md5);
	}
}

/*
 * Return a dynamically allocated copy of the given server string.
 * The copy is null-terminated.  If "len" is non-NULL, the length of
 * the string (excluding the terminating null byte) is stored via it.
 * Returns NULL on failure.  Empty strings are still allocated even
 * though they have no content.
 */
static void *
dup_str(struct tac_handle *h, const struct srvr_str *ss, size_t *len)
{
	unsigned char *p;

	if ((p = (unsigned char *)xmalloc(h, ss->len + 1)) == NULL)
		return NULL;
	if (ss->data != NULL && ss->len != 0)
		memcpy(p, ss->data, ss->len);
	p[ss->len] = '\0';
	if (len != NULL)
		*len = ss->len;
	return p;
}


/*
 * Free a client string, obliterating its contents first for security.
 */
static void
free_str(struct clnt_str *cs)
{
	if (cs->data != NULL) {
		memset(cs->data, 0, cs->len);
		free(cs->data);
		cs->data = NULL;
		cs->len = 0;
	}
}

static void
generr(struct tac_handle *h, const char *format, ...)
{
	va_list		 ap;

	va_start(ap, format);
	vsnprintf(h->errmsg, ERRSIZE, format, ap);
	va_end(ap);
}

static void
gen_session_id(struct tac_msg *msg)
{
	int r;

	r = random();
	msg->session_id[0] = r >> 8;
	msg->session_id[1] = r;
	r = random();
	msg->session_id[2] = r >> 8;
	msg->session_id[3] = r;
}

/*
 * Verify that we are exactly at the end of the response message.
 * Returns 0 on success, -1 on failure.
 */
static int
get_srvr_end(struct tac_handle *h)
{
	int len;

	len = ntohl(h->response->length);

	if (h->srvr_pos != len) {
		generr(h, "Invalid length field in response "
		       "from server: end expected at %u, response length %u",
		       h->srvr_pos, len);
		return -1;
	}
	return 0;
}

static int
get_srvr_str(struct tac_handle *h, const char *field,
	     struct srvr_str *ss, size_t len)
{
	if (h->srvr_pos + len > ntohl(h->response->length)) {
		generr(h, "Invalid length field in %s response from server "
		       "(%lu > %lu)", field, (u_long)(h->srvr_pos + len),
		       (u_long)ntohl(h->response->length));
		return -1;
	}
	ss->data = len != 0 ? h->response->u.body + h->srvr_pos : NULL;
	ss->len = len;
	h->srvr_pos += len;
	return 0;
}

static void
init_clnt_str(struct clnt_str *cs)
{
	cs->data = NULL;
	cs->len = 0;
}

static void
init_srvr_str(struct srvr_str *ss)
{
	ss->data = NULL;
	ss->len = 0;
}

/*
 * Receive a response from the server and decrypt it.  Returns 0 on
 * success, or -1 on failure.
 */
static int
validate_msg(struct tac_handle *h)
{
	struct tac_msg *msg;
	u_int32_t len;

	msg = h->response;
	
	if (memcmp(msg->session_id, h->request.session_id,
	    sizeof msg->session_id) != 0) {
		generr(h, "Invalid session ID in received message");
		return -1;
	}
	if (msg->type != h->request.type) {
		generr(h, "Invalid type in received message"
			  " (got %u, expected %u)",
			  msg->type, h->request.type);
		return -1;
	}
	len = ntohl(msg->length);
	if (len > BODYSIZE) {
		generr(h, "Received message too large (%u > %u)",
			  len, BODYSIZE);
		return -1;
	}
	if (msg->seq_no != ++h->last_seq_no) {
		generr(h, "Invalid sequence number in received message"
			  " (got %u, expected %u)",
			  msg->seq_no, h->last_seq_no);
		return -1;
	}

	/* Decrypt it. */
	crypt_msg(h, msg);

	/*
	 * Turn off single-connection mode if the server isn't amenable
	 * to it.
	 */
	if (!(msg->flags & TAC_SINGLE_CONNECT)) {
	//    assert(0); /* log it */
	//	return -1;
    }

	return 0;
}

static int
save_str(struct tac_handle *h, struct clnt_str *cs, const void *data,
    size_t len)
{
	free_str(cs);
	if (data != NULL && len != 0) {
		if ((cs->data = xmalloc(h, len)) == NULL)
			return -1;
		cs->len = len;
		memcpy(cs->data, data, len);
	}
	return 0;
}

/*
 * Send the current request, after encrypting it.  Returns 0 on success,
 * or -1 on failure.
 */
static int
send_msg(struct tac_handle *h)
{
	struct timeval deadline;
	struct tac_msg *msg;
	char *ptr;
	int len;

	if (h->last_seq_no & 1) {
		generr(h, "Attempt to send message out of sequence");
		return -1;
	}

	if (h->fd < 0)
		return -1;

	msg = &h->request;
	msg->seq_no = ++h->last_seq_no;
    if (msg->seq_no == 1)
		gen_session_id(msg);
	crypt_msg(h, msg);

	//msg->flags |= TAC_SINGLE_CONNECT;
	gettimeofday(&deadline, NULL);
	deadline.tv_sec += h->timeout;
	len = HDRSIZE + ntohl(msg->length);
	ptr = (char *)msg;
	while (len > 0) {
		int n;

		n = write(h->fd, ptr, len);
		if (n == -1) {
			struct timeval tv;
			int nfds;

			if (errno != EAGAIN) {
				generr(h, "Network write error: %s",
				    strerror(errno));
				return -1;
			}

			/* Wait until we can write more data. */
			gettimeofday(&tv, NULL);
			timersub(&deadline, &tv, &tv);
			if (tv.tv_sec >= 0) {
				fd_set wfds;

				FD_ZERO(&wfds);
				FD_SET(h->fd, &wfds);
				nfds =
				    select(h->fd + 1, NULL, &wfds, NULL, &tv);
				if (nfds == -1) {
					generr(h, "select: %s",
					    strerror(errno));
					return -1;
				}
			} else
				nfds = 0;
			if (nfds == 0) {
				generr(h, "Network write timed out");
				return -1;
			}
		} else {
			ptr += n;
			len -= n;
		}
	}

	return 0;
}

void
tac_close(struct tac_handle *h)
{
	int i;

#if 0
	if (h->fd != -1)
		close(h->fd);
#endif

      AUTHD_EVENT_DEBUG(
            "close tacplus backend handle %p", h);

	free_str(&h->user);
	free_str(&h->port);
	free_str(&h->rem_addr);
	free_str(&h->data);
	free_str(&h->user_msg);
	for (i=0; i<MAXAVPAIRS; i++)
		free_str(&(h->avs[i]));

	/* Clear everything else before freeing memory */
	memset(h, 0, sizeof(struct tac_handle));
	free(h);
}

int
tac_create_authen(struct tac_handle *h, int action, int priv_lvl, int type, int service)
{
	struct tac_authen_start *as;

	create_msg(h, TAC_AUTHEN, action, type);

	as = &h->request.u.authen_start;
	as->action = action;
	as->priv_lvl = priv_lvl;
	as->authen_type = type;
	as->service = service;

	return 0;
}

int
tac_create_author(struct tac_handle *h, int method, int priv_lvl, int type, int service)
{
	struct tac_author_request *areq;

	create_msg(h, TAC_AUTHOR, 0, 0);

	areq = &h->request.u.author_request;
	areq->authen_meth = method;
	areq->priv_lvl = priv_lvl;
	areq->authen_type = type;
	areq->service = service;

	return 0;
}

int
tac_create_acct(struct tac_handle *h, int flags, int method, int priv_lvl, int type, int service)
{
	struct tac_acct_start *as;

	create_msg(h, TAC_ACCT, 0, 0);

	as = &h->request.u.acct_start;
	as->flags = flags;
	as->auth_method = method;
	as->priv_lvl = priv_lvl;
	as->authen_type = type;
	as->authen_service = service;

	return 0;
}

static void
create_msg(struct tac_handle *h, int msg_type, int var, int type)
{
	struct tac_msg *msg;
	int i;

	h->last_seq_no = 0;

	msg = &h->request;
	msg->type = msg_type;
	msg->version = protocol_version(msg_type, var, type);
	msg->flags = 0; /* encrypted packet body */

	free_str(&h->user);
	free_str(&h->port);
	free_str(&h->rem_addr);
	free_str(&h->data);
	free_str(&h->user_msg);

	for (i=0; i<MAXAVPAIRS; i++)
		free_str(&(h->avs[i]));
}

void *
tac_get_data(struct tac_handle *h, size_t *len)
{
	return dup_str(h, &h->srvr_data, len);
}

char *
tac_get_msg(struct tac_handle *h)
{
	return dup_str(h, &h->srvr_msg, NULL);
}

/*
 * Create and initialize a tac_handle structure, and return it to the
 * caller.  Can fail only if the necessary memory cannot be allocated.
 * In that case, it returns NULL.
 */
struct tac_handle *
tac_open(void)
{
	int i;
	struct tac_handle *h;

	h = (struct tac_handle *)malloc(sizeof(struct tac_handle));
	if (h != NULL) {
		h->fd = -1;
		h->errmsg[0] = '\0';
		init_clnt_str(&h->user);
		init_clnt_str(&h->port);
		init_clnt_str(&h->rem_addr);
		init_clnt_str(&h->data);
		init_clnt_str(&h->user_msg);
		for (i=0; i<MAXAVPAIRS; i++) {
			init_clnt_str(&(h->avs[i]));
			init_srvr_str(&(h->srvr_avs[i]));
		}
		init_srvr_str(&h->srvr_msg);
		init_srvr_str(&h->srvr_data);
#if 0
		srandomdev();
#endif
	}
	return h;
}

int
tac_recv_authen(struct tac_handle *h)
{
    struct tac_authen_reply *ar;

    /* Send the message and retrieve the reply. */
    if (validate_msg(h) == -1)
        return -1;

    /* Scan the optional fields in the reply. */
    ar = &h->response->u.authen_reply;
    h->srvr_pos = offsetof(struct tac_authen_reply, rest[0]);
    if (get_srvr_str(h, "msg", &h->srvr_msg, ntohs(ar->msg_len)) == -1 ||
        get_srvr_str(h, "data", &h->srvr_data, ntohs(ar->data_len)) == -1 ||
        get_srvr_end(h) == -1)
        return -1;

    return ar->flags << 8 | ar->status;
}

int
tac_recv_author(struct tac_handle *h)
{
	int i = 0;
	char dbgstr[64] = {0};
	struct tac_author_response *ares = &h->response->u.author_response;

    /* Send the message and retrieve the reply. */
    if (validate_msg(h) == -1)
    {
        return -1;
    }

	/* Update the offset in the response packet based on av pairs count */
	h->srvr_pos = offsetof(struct tac_author_response, rest[0]) + ares->av_cnt;

	/* Scan the optional fields in the response. */
	if (get_srvr_str(h, "msg", &h->srvr_msg, ntohs(ares->msg_len)) == -1 ||
	    get_srvr_str(h, "data", &h->srvr_data, ntohs(ares->data_len)) ==-1)
    {
		return -1;
    }

	/* Get each AV pair (just setting pointers, not malloc'ing) */
	clear_srvr_avs(h);
	for (i = 0; i < ares->av_cnt; i++) 
    {
		snprintf(dbgstr, sizeof dbgstr, "av-pair-%d", i);
		if (get_srvr_str(h, dbgstr, &(h->srvr_avs[i]), ares->rest[i]) == -1)
        {
            return -1;
        }
	}

	/* Should have ended up at the end */
	if (get_srvr_end(h) == -1)
    {
        return -1;
    }

	return ares->av_cnt << 8 | ares->status;
}

int
tac_recv_acct(struct tac_handle *h)
{
    struct tac_acct_reply *ares = &h->response->u.acct_reply;    

    /* retrieve the reply. */
    if (validate_msg(h) == -1)
    {
        return -1;
    }

    /* Scan the optional fields in the reply. */
    h->srvr_pos = offsetof(struct tac_acct_reply, rest[0]);
    if (get_srvr_str(h, "msg", &h->srvr_msg, ntohs(ares->msg_len)) == -1 ||
        get_srvr_str(h, "data", &h->srvr_data, ntohs(ares->data_len)) == -1 ||
        get_srvr_end(h) == -1)
    {
        return -1;
    }

    return ares->status;
}

int
tac_send_authen(struct tac_handle *h)
{
	if (h->last_seq_no == 0) { /* Authentication START packet */
		struct tac_authen_start *as;

		as = &h->request.u.authen_start;
		h->request.length =
		    htonl(offsetof(struct tac_authen_start, rest[0]));
		/* modified by liwh for bug 41224, 2017-01-16 */
		//if (add_str_8(h, &as->user_len, &h->user) == -1 ||
		//   (add_str_8(h, &as->port_len, &h->port) == -1 ||
		as->user_len = 0;
		if (add_str_8(h, &as->port_len, &h->port) == -1 ||
		/* liwh end */
		    add_str_8(h, &as->rem_addr_len, &h->rem_addr) == -1 ||
		    add_str_8(h, &as->data_len, &h->data) == -1)
			return -1;
	} else { /* Authentication CONTINUE packet */
		struct tac_authen_cont *ac;

		ac = &h->request.u.authen_cont;
		ac->flags = 0;
		h->request.length =
		    htonl(offsetof(struct tac_authen_cont, rest[0]));
		if (add_str_16(h, &ac->user_msg_len, &h->user_msg) == -1 ||
		    add_str_16(h, &ac->data_len, &h->data) == -1)
			return -1;
	}

	/* Send the message and retrieve the reply. */
	if (send_msg(h) == -1)
		return -1;

	return 0;
}

int
tac_set_rem_addr(struct tac_handle *h, const char *addr)
{
	return save_str(h, &h->rem_addr, addr, addr != NULL ? strlen(addr) : 0);
}

int
tac_set_data(struct tac_handle *h, const void *data, size_t data_len)
{
	return save_str(h, &h->data, data, data_len);
}

int
tac_set_msg(struct tac_handle *h, const char *msg)
{
	return save_str(h, &h->user_msg, msg, msg != NULL ? strlen(msg) : 0);
}

int
tac_set_port(struct tac_handle *h, const char *port)
{
	return save_str(h, &h->port, port, port != NULL ? strlen(port) : 0);
}

int
tac_set_priv(struct tac_handle *h, int priv)
{
	if (!(TAC_PRIV_LVL_MIN <= priv && priv <= TAC_PRIV_LVL_MAX)) {
		generr(h, "Attempt to set invalid privilege level");
		return -1;
	}
	h->request.u.authen_start.priv_lvl = priv;
	return 0;
}

int
tac_set_user(struct tac_handle *h, const char *user)
{
	return save_str(h, &h->user, user, user != NULL ? strlen(user) : 0);
}

int
tac_set_av(struct tac_handle *h, u_int index, const char *av)
{
	if (index >= MAXAVPAIRS)
		return -1;
	return save_str(h, &(h->avs[index]), av, av != NULL ? strlen(av) : 0);
}

char *
tac_get_av(struct tac_handle *h, u_int index)
{
	if (index >= MAXAVPAIRS)
		return NULL;
	return dup_str(h, &(h->srvr_avs[index]), NULL);
}

char *
tac_get_av_value(struct tac_handle *h, const char *attribute)
{
	int i, len;
	const char *ch, *end;
	const char *candidate;
	int   candidate_len;
	int   found_seperator;
	struct srvr_str srvr;

	if (attribute == NULL || ((len = strlen(attribute)) == 0))
		return NULL;

	for (i=0; i<MAXAVPAIRS; i++) {
		candidate = h->srvr_avs[i].data;
		candidate_len = h->srvr_avs[i].len;

		/*
		 * Valid 'srvr_avs' guaranteed to be contiguous starting at 
		 * index 0 (not necessarily the case with 'avs').  Break out
		 * when the "end" of the list has been reached.
		 */
		if (!candidate)
			break;

		if (len < candidate_len && 
		    !strncmp(candidate, attribute, len)) {

			ch = candidate + len;
			end = candidate + candidate_len;

			/*
			 * Sift out the white space between A and V (should not
			 * be any, but don't trust implementation of server...)
			 */
			found_seperator = 0;
			while ((*ch == '=' || *ch == '*' || *ch == ' ' ||
				*ch == '\t') && ch != end) {
				if (*ch == '=' || *ch == '*')
					found_seperator++;
				ch++;
			}

			/*
			 * Note:
			 *     The case of 'attribute' == "foo" and
			 *     h->srvr_avs[0] = "foobie=var1"
			 *     h->srvr_avs[1] = "foo=var2"
			 * is handled.
			 *
			 * Note that for empty string attribute values a
			 * 0-length string is returned in order to distinguish
			 * against unset values.
			 * dup_str() will handle srvr.len == 0 correctly.
			 */
			if (found_seperator == 1) {
				srvr.len = end - ch;
				srvr.data = ch;
				return dup_str(h, &srvr, NULL);
			}
		}
	}
	return NULL;
}

void
tac_clear_avs(struct tac_handle *h)
{
	int i;
	for (i=0; i<MAXAVPAIRS; i++)
		save_str(h, &(h->avs[i]), NULL, 0);
}

void
clear_srvr_avs(struct tac_handle *h)
{
	int i;
	for (i=0; i<MAXAVPAIRS; i++)
		init_srvr_str(&(h->srvr_avs[i]));
}


const char *
tac_strerror(struct tac_handle *h)
{
	return h->errmsg;
}

static void *
xmalloc(struct tac_handle *h, size_t size)
{
	void *r;

	if ((r = malloc(size)) == NULL)
		generr(h, "Out of memory");
	return r;
}

extern int auth_get_shared_secret(tbl_auth_server_t *pSrv, char *pszOut, size_t nBufLen);
extern int auth_get_timeout(tbl_auth_server_t * pSrv);

int tac_op_init(auth_handle_t *h)
{
    struct tac_handle *pObj = NULL;
    
    pObj = tac_open(); 

    AUTHD_EVENT_DEBUG(
            "initialize tacplus backend handle %p", pObj);
    if (NULL == pObj) {
        return -1;
    }

    h->pObj = (void *)pObj;
    return 0;
}

void tac_op_release(auth_handle_t *h)
{
    struct tac_handle *pObj = (struct tac_handle *)h->pObj;

    AUTHD_EVENT_DEBUG(
            "release tacplus backend handle %p", pObj);
    if (NULL != pObj) {
        tac_close(pObj);
    }
    return;
}

int tac_op_connect(auth_handle_t *h, tbl_auth_server_t *pSrv)
{
    struct sockaddr_in dest;
    struct sockaddr_in6 dest6;
    struct sockaddr *psa;
    socklen_t salen;
    int flags;
    int fd = -1;

    AUTHD_EVENT_DEBUG(
            "tacplus op connect %p", h->pObj);

    if (pSrv->connected) {
        return M_AUTH_CONNECT_SUCCESS;
    }

    if (pSrv->fd > 0) {
        close(pSrv->fd);
        pSrv->fd = -1;
    }

    if (AF_INET == pSrv->key.addr.family) {
        psa = (struct sockaddr *)&dest;
        salen = sizeof(struct sockaddr_in);
        memset(psa, 0x00, salen);
        dest.sin_family = AF_INET;
        dest.sin_port = htons(pSrv->port);
        memcpy(&dest.sin_addr, &pSrv->key.addr.u.prefix4, sizeof(struct in_addr));
    } else {
        psa = (struct sockaddr *)&dest6;
        salen = sizeof(struct sockaddr_in6);
        memset(psa, 0x00, salen);
        dest6.sin6_family = AF_INET6;
        dest6.sin6_port = htons(pSrv->port);
        memcpy(&dest6.sin6_addr, &pSrv->key.addr.u.prefix6, sizeof(struct in6_addr));
    }

    if ((fd = socket(pSrv->key.addr.family, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        goto err_out;
    }

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1 ||
            fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        goto err_out;
    }
    
    if (connect(fd, psa, salen) == 0) {
        goto err_out;
    }

    pSrv->fd = fd;
    if (errno == EINPROGRESS) {
        fd_set wfds;
        struct timeval tv;
        int nfds;
        struct sockaddr peer;
        socklen_t peerlen;
        
        /* Wait for the connection to complete. */
        FD_ZERO(&wfds);
        FD_SET(pSrv->fd, &wfds);
        tv.tv_sec = pSrv->timeout;
        tv.tv_usec = 0;
        nfds = select(pSrv->fd + 1, NULL, &wfds, NULL, &tv);
        if (nfds == -1) {
            goto err_out;
        }
        if (nfds == 0) {
            goto err_out;
        }
        
        /* See whether we are connected now. */
        peerlen = sizeof peer;
        if (getpeername(pSrv->fd, &peer, &peerlen) == 0)
        {
            pSrv->connected = 1;
            return M_AUTH_CONNECT_SUCCESS;
        }
        if (errno != ENOTCONN) {
            goto err_out;
        }
    }

    pSrv->connected = 1;
    return M_AUTH_CONNECT_SUCCESS;

err_out:
    if (-1 != fd) {
        close(fd);
        pSrv->fd = -1;
    }
    return M_AUTH_CONNECT_FAILED;
}

int
tac_send_author(struct tac_handle *h)
{
    int i, current;
    struct tac_author_request *areq = &h->request.u.author_request;

    h->request.length =
        htonl(offsetof(struct tac_author_request, rest[0]));

    /* Count each specified AV pair */
    for (areq->av_cnt=0, i=0; i<MAXAVPAIRS; i++)
        if (h->avs[i].len && h->avs[i].data)
            areq->av_cnt++;

    /*
     * Each AV size is a byte starting right after 'av_cnt'.  Update the
     * offset to include these AV sizes.
     */
    h->request.length = ntohl(htonl(h->request.length) + areq->av_cnt);

    /* Now add the string arguments from 'h' */
    if (add_str_8(h, &areq->user_len, &h->user) == -1 ||
        add_str_8(h, &areq->port_len, &h->port) == -1 ||
        add_str_8(h, &areq->rem_addr_len, &h->rem_addr) == -1)
        return -1;

    /* Add each AV pair, the size of each placed in areq->rest[current] */
    for (current=0, i=0; i<MAXAVPAIRS; i++) {
        if (h->avs[i].len && h->avs[i].data) {
            if (add_str_8(h, &areq->rest[current++],
                &(h->avs[i])) == -1)
                return -1;
        }
    }

    /* send */
    if (send_msg(h) == -1)
    {
        return M_AUTH_SEND_FAILED;
    }
    return M_AUTH_SEND_SUCCESS;
}

int
tac_send_acct(struct tac_handle *h)
{
	register int i, current;
	struct tac_acct_start *as = &h->request.u.acct_start;

	/* start */
	as = &h->request.u.acct_start;
	h->request.length = htonl(offsetof(struct tac_acct_start, rest[0]));
	for (as->av_cnt = 0, i = 0; i < MAXAVPAIRS; i++)
		if (h->avs[i].len && h->avs[i].data)
			as->av_cnt++;
	h->request.length = ntohl(htonl(h->request.length) + as->av_cnt);

	if (add_str_8(h, &as->user_len, &h->user) == -1 ||
	    add_str_8(h, &as->port_len, &h->port) == -1 ||
	    add_str_8(h, &as->rem_addr_len, &h->rem_addr) == -1)
		return -1;

	for (i = current = 0; i < MAXAVPAIRS; i++)
		if (h->avs[i].len && h->avs[i].data)
			if (add_str_8(h, &as->rest[current++], &(h->avs[i])) == -1)
				return -1;

    /* send */
    if (send_msg(h) == -1)
    {
        return M_AUTH_SEND_FAILED;
    }
    return M_AUTH_SEND_SUCCESS;
}

tbl_login_t *auth_get_login_by_vty(tbl_vty_t *p_vty)
{
    tbl_login_master_t *p_master = tbl_login_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_login_t *p_login = NULL;
    char         tmp[VTY_BUF_SIZE] = {0};
    
    if (NULL == p_master)
    {
        return NULL;
    }

    sal_sprintf(tmp, "vty%d", p_vty->key.id);

    CTCLIB_SLIST_LOOP(p_master->login_list, p_login, listnode)
    {
        if (0 == sal_strcmp(p_login->vty, tmp))
        {
            return p_login;
        }
    }

    return NULL;
}

int tac_op_send_author_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    struct tac_handle  *pObj = (struct tac_handle *)h->pObj;
    int                ret = -1;
    tbl_vty_t          *p_vty = NULL;
    tbl_vty_t          vty;
    char               short_buf[AAA_MSG_STR_SHORT] = {0};
    char               long_buf[AAA_MSG_STR_LONG] = {0};
    uint8              arg_cnt = 0;
    tbl_login_t        *p_db_login = NULL;
    uint8              privilege = 0;
                
    sal_memset(&vty, 0, sizeof(tbl_vty_t));
    vty.key.id = pSess->key.index - 1;

    AUTHD_EVENT_DEBUG(
            "tac_op_send_author_request is called");
    
    p_vty = tbl_vty_get_vty(&vty.key);
    if (!p_vty)
    {
        AUTHD_EVENT_DEBUG(
            "tac_op_send_author_request p_vty is null");
    }

    if (NULL == pObj || !pSrv->connected) {
        return M_AUTH_SEND_FAILED;
    }

    AUTHD_EVENT_DEBUG(
            "tacplus op send author request %p", pObj);

    pObj->fd = pSrv->fd;
    pObj->timeout = auth_get_timeout(pSrv);    
    pObj->secret[0] = '\0';
    auth_get_shared_secret(pSrv, pObj->secret, TAC_SECRET_BUFSZ);

    /* set privilege */
    privilege = aaa_remap_privilege(p_vty->privilege);
    tac_create_author(pObj, TAC_AUTHEN_METH_TACACSPLUS, privilege, TAC_ACCT_TYPE_ASCII, TAC_ACCT_SVC_LOGIN);

    /*  set user */
    tac_set_user(pObj, pSess->user.name);

    /* set port */
    sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "%s%d", "vty", p_vty->key.id);
    tac_set_port(pObj, short_buf);

    /* set ip addr */
    p_db_login = auth_get_login_by_vty(p_vty);
    if (p_db_login && (p_db_login->ipaddr[0] != '\0'))
    {
        tac_set_rem_addr(pObj, p_db_login->ipaddr);    
    }

    /* allways shell */
    sal_memcpy(short_buf, "service=shell", AAA_MSG_STR_SHORT);
    tac_set_av(pObj, arg_cnt, short_buf);
    arg_cnt++;

    /* CLI string */
    sal_snprintf(long_buf, AAA_MSG_STR_LONG, "%s", "cmd*");
    tac_set_av(pObj, arg_cnt, long_buf);
    arg_cnt++;
        
    ret = tac_send_author(pObj);
    if (ret < 0) {
        /* auth failed */
        return M_AUTH_SEND_FAILED;
    }

    pSess->uCurSessId = *((u_int32_t *)&(pObj->request.session_id));
    return M_AUTH_SEND_SUCCESS;
}


int tac_op_send_account_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    struct tac_handle  *pObj = (struct tac_handle *)h->pObj;
    int                ret = -1;
    tbl_vty_t          *p_vty = NULL;
    tbl_vty_t          vty;
    char               short_buf[AAA_MSG_STR_SHORT] = {0};
    uint8              arg_cnt = 0;
    tbl_login_t        *p_db_login = NULL;
    uint8              privilege = 0;
    char               timezone[AAA_MSG_STR_SHORT] = {0};           
    time_t             date; 
#ifndef _GLB_UML_SYSTEM_
    uint8              disc_cause = 1;
    uint16             disc_cause_ext = 1022;
#endif

    sal_memset(&vty, 0, sizeof(tbl_vty_t));
    vty.key.id = pSess->key.index - 1;
    
    p_vty = tbl_vty_get_vty(&vty.key);
    if (!p_vty)
    {
        return M_AUTH_SEND_FAILED;     
    }

    if (NULL == pObj || !pSrv->connected) {
        return M_AUTH_SEND_FAILED;
    }

    AUTHD_EVENT_DEBUG(
            "tacplus op send account request %p", pObj);

    pObj->fd = pSrv->fd;
    pObj->timeout = auth_get_timeout(pSrv);    
    pObj->secret[0] = '\0';
    auth_get_shared_secret(pSrv, pObj->secret, TAC_SECRET_BUFSZ);

    if (pSess->acct_start)
    {
        /* set privilege */
        privilege = aaa_remap_privilege(p_vty->privilege);
        tac_create_acct(pObj, TAC_ACCT_START, TAC_AUTHEN_METH_TACACSPLUS, privilege, 
            TAC_ACCT_TYPE_ASCII, TAC_ACCT_SVC_LOGIN);
    
        /*  set user */
        tac_set_user(pObj, pSess->user.name);
    
        /* set vty name */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "%s%d", "vty", p_vty->key.id);
        tac_set_port(pObj, short_buf);
    
        /* set ip addr */
        p_db_login = auth_get_login_by_vty(p_vty);
        if (p_db_login)
        {
            if (p_db_login->ipaddr[0] != '\0')
            {
                tac_set_rem_addr(pObj, p_db_login->ipaddr);  
            }

            /* the pid of login */
            sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "task_id=%d", p_db_login->pid);
            tac_set_av(pObj, arg_cnt, short_buf);
            arg_cnt++;
        }

        /* time zone */
        date = sal_time(NULL);
        sal_time_strf(timezone, sizeof(timezone), "%Z", sal_localtime(&date));
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "timezone=%s", timezone);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;

        p_vty->aaa_start_time = date;
        tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_AAA_START_TIME);

        /* allways shell */
        sal_memcpy(short_buf, "service=shell", AAA_MSG_STR_SHORT);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;
    }
    else
    {
        /* set privilege */
        privilege = aaa_remap_privilege(p_vty->acct_stop_privilege);
        tac_create_acct(pObj, TAC_ACCT_STOP, TAC_AUTHEN_METH_TACACSPLUS, privilege, 
            TAC_ACCT_TYPE_ASCII, TAC_ACCT_SVC_LOGIN);
    
        /*  set user */
        /* modified by liwh for bug 50704, 2019-01-08 */
        if (sal_strcmp(pSess->user.name, "null"))
        /* liwh end */
        {
            tac_set_user(pObj, pSess->user.name);
        }
    
        /* set vty name */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "%s%d", "vty", p_vty->key.id);
        tac_set_port(pObj, short_buf);
    
        /* set ip addr */
        tac_set_rem_addr(pObj, p_vty->acct_stop_ipaddr);  

        /* the pid of login */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "task_id=%d", p_vty->acct_stop_pid);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;

        /* time zone */
        date = sal_time(NULL);
        sal_time_strf(timezone, sizeof(timezone), "%Z", sal_localtime(&date));
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "timezone=%s", timezone);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;

        /* allways shell */
        sal_memcpy(short_buf, "service=shell", AAA_MSG_STR_SHORT);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;

#ifndef _GLB_UML_SYSTEM_
        /* disc cause  */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "disc-cause=%d", disc_cause);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;  

        /* disc cause ext */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "disc-cause-ext=%d", disc_cause_ext);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;
#endif

        /* elapsed time */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "elapsed_time=%d", (int)(date - p_vty->aaa_start_time));
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;
    
        /* stop time */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "stop_time=%ld", date);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;
    }
        
    ret = tac_send_acct(pObj);
    if (ret < 0) {
        /* auth failed */
        return M_AUTH_SEND_FAILED;
    }

    pSess->uCurSessId = *((u_int32_t *)&(pObj->request.session_id));

    return M_AUTH_SEND_SUCCESS;
}

int tac_op_send_account_commands_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    struct tac_handle  *pObj = (struct tac_handle *)h->pObj;
    int                ret = -1;
    tbl_vty_t          *p_vty = NULL;
    tbl_vty_t          vty;
    char               short_buf[AAA_MSG_STR_SHORT] = {0};
    char               long_buf[AAA_MSG_STR_LONG] = {0};
    uint8              arg_cnt = 0;
    tbl_login_t        *p_db_login = NULL;
    uint8              privilege = 0;
    char               timezone[AAA_MSG_STR_SHORT] = {0};           
    time_t             date; 

    sal_memset(&vty, 0, sizeof(tbl_vty_t));
    vty.key.id = pSess->key.index - 1;
    
    p_vty = tbl_vty_get_vty(&vty.key);
    if (!p_vty)
    {
        return M_AUTH_SEND_FAILED;     
    }

    if (NULL == pObj || !pSrv->connected) {
        return M_AUTH_SEND_FAILED;
    }

    AUTHD_EVENT_DEBUG(
            "tacplus op send account command request %p", pObj);

    pObj->fd = pSrv->fd;
    pObj->timeout = auth_get_timeout(pSrv);    
    pObj->secret[0] = '\0';
    auth_get_shared_secret(pSrv, pObj->secret, TAC_SECRET_BUFSZ);

    /* set privilege */
    privilege = aaa_remap_privilege(p_vty->privilege);
    tac_create_acct(pObj, TAC_ACCT_STOP, TAC_AUTHEN_METH_TACACSPLUS, privilege, 
        TAC_ACCT_TYPE_ASCII, TAC_ACCT_SVC_LOGIN);

    /*  set user */
    if (sal_strcmp(pSess->user.name, "null"))
    {
        tac_set_user(pObj, pSess->user.name);
    }

    /* set vty name */
    sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "%s%d", "vty", p_vty->key.id);
    tac_set_port(pObj, short_buf);

    /* set ip addr */
    p_db_login = auth_get_login_by_vty(p_vty);
    if (p_db_login)
    {
        if (p_db_login->ipaddr[0] != '\0')
        {
            tac_set_rem_addr(pObj, p_db_login->ipaddr);  
        }

        /* the pid of login */
        sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "task_id=%d", p_db_login->pid);
        tac_set_av(pObj, arg_cnt, short_buf);
        arg_cnt++;
    }

    /* time zone */
    date = sal_time(NULL);
    sal_time_strf(timezone, sizeof(timezone), "%Z", sal_localtime(&date));
    sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "timezone=%s", timezone);
    tac_set_av(pObj, arg_cnt, short_buf);
    arg_cnt++;

    /* allways shell */
    sal_memcpy(short_buf, "service=shell", AAA_MSG_STR_SHORT);
    tac_set_av(pObj, arg_cnt, short_buf);
    arg_cnt++;

    /* CLI level  */
    sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "priv-lvl=%d", privilege);
    tac_set_av(pObj, arg_cnt, short_buf);
    arg_cnt++;

    /* CLI string */
    sal_snprintf(long_buf, AAA_MSG_STR_LONG, "cmd=%s", p_vty->cmd_buf);
    tac_set_av(pObj, arg_cnt, long_buf);
    arg_cnt++;

    ret = tac_send_acct(pObj);
    if (ret < 0) {
        /* auth failed */
        return M_AUTH_SEND_FAILED;
    }

    pSess->uCurSessId = *((u_int32_t *)&(pObj->request.session_id));

    return M_AUTH_SEND_SUCCESS;
}


/*added by yejl to fix bug41223, 2017-01-07*/
int tac_op_send_authen_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    struct tac_handle  *pObj = (struct tac_handle *)h->pObj;
    int                ret = -1;
    tbl_vty_t          *p_vty = NULL;
    tbl_vty_t          vty;
    uint8              privilege = 0;
    /* modified by liwh for bug 41224, 2017-01-16 */
    char               short_buf[AAA_MSG_STR_SHORT] = {0};
    tbl_login_t        *p_db_login = NULL;
    /* liwh end */

    sal_memset(&vty, 0, sizeof(tbl_vty_t));
    vty.key.id = pSess->key.index - 1;
        
    p_vty = tbl_vty_get_vty(&vty.key);
    if (!p_vty)
    {
        return M_AUTH_SEND_FAILED;     
    }

    if (NULL == pObj || !pSrv->connected) {
        return M_AUTH_SEND_FAILED;
    }

    AUTHD_EVENT_DEBUG("tacplus op send authen request %p", pObj);
    
    pObj->fd = pSrv->fd;
    pObj->timeout = auth_get_timeout(pSrv);    
    pObj->secret[0] = '\0';
    auth_get_shared_secret(pSrv, pObj->secret, TAC_SECRET_BUFSZ);

    /* set privilege */
    privilege = aaa_remap_privilege(p_vty->privilege);
    tac_create_authen(pObj, TAC_AUTHEN_LOGIN, privilege, TAC_AUTHEN_TYPE_ASCII, TAC_AUTHEN_SVC_LOGIN);

    /* modified by liwh for bug 41224, 2017-01-16 */
    //tac_set_user(pObj, pSess->user.name);

    /* set port */
    sal_snprintf(short_buf, AAA_MSG_STR_SHORT, "%s%d", "vty", p_vty->key.id);
    tac_set_port(pObj, short_buf);

    /* set ip addr */
    p_db_login = auth_get_login_by_vty(p_vty);
    if (p_db_login && (p_db_login->ipaddr[0] != '\0'))
    {
        tac_set_rem_addr(pObj, p_db_login->ipaddr);    
    }
    /* liwh end */

    ret = tac_send_authen(pObj);
    if (ret < 0) {
        /* auth failed */
        return M_AUTH_SEND_FAILED;
    }

    pSess->uCurSessId = *((u_int32_t *)&(pObj->request.session_id));
    return M_AUTH_SEND_SUCCESS;
    
}

int tac_op_send_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    int ret = -1;
    if (pSess->author_start)
    {
        ret = tac_op_send_author_request(h, pSess, pSrv);
        return ret;
    }

    if (pSess->acct_start || pSess->acct_stop) 
    {
        ret = tac_op_send_account_request(h, pSess, pSrv);
        return ret;    
    }

    if (pSess->acctcmd_start)
    {
        ret = tac_op_send_account_commands_request(h, pSess, pSrv);
        return ret;
    }

    ret = tac_op_send_authen_request(h, pSess, pSrv);
    return ret;
}

/* modified by liwh for bug 41224, 2017-01-16 */
static int tac_send_user(struct tac_handle *pObj, tbl_auth_session_t *pSess)
{
    int ret;
    
    tac_set_user(pObj, pSess->user.name);
    tac_set_msg(pObj, pSess->user.name);
    
    ret = tac_send_authen(pObj);
    if (-1 == ret) {
        return M_AUTH_ERROR;
    }
    return M_AUTH_INPROGRESS;
}
/* liwh end */

static int tac_send_pass(struct tac_handle *pObj, tbl_auth_session_t *pSess)
{
    char szPasswd[PASSWD_SIZE];
    int ret;
    
    sal_b64_pton(pSess->passwd, (unsigned char *)szPasswd, PASSWD_SIZE);
    tac_set_user(pObj, pSess->user.name);
    tac_set_msg(pObj, szPasswd);
    
    ret = tac_send_authen(pObj);
    if (-1 == ret) {
        return M_AUTH_ERROR;
    }
    return M_AUTH_INPROGRESS;
}

tbl_auth_session_t *auth_session_get_by_id(u_int32_t uSessId);
int tac_op_process_reply(tbl_auth_server_t *pSrv, AUTH_RESULT_CB pCB)
{
    struct tac_msg       *msg = (struct tac_msg *)pSrv->buf;
    tbl_auth_session_t   *pSess = NULL;
    auth_handle_t        *h = NULL;
    struct tac_handle    *pObj = NULL;
    int                  ret = 0;
    int                  pktlen = 0;
    int                  leftlen;   
    int                  i = 0;
    int                  priv_lvl = PRIVILEGE_INVALID;
    char                 tmp_str[256] = {0};
    tbl_vty_t            *p_vty = NULL;
    tbl_vty_t            vty;

    ret = read(pSrv->fd, pSrv->buf + pSrv->blen, GLB_AUTH_MSG_BUF_SZ - pSrv->blen);
    if (ret <= 0) {
     
        AUTHD_EVENT_DEBUG(
             "tac_op_process_reply return errno %s fd %d ret %d when read socket", strerror(errno), pSrv->fd, ret);
        /* socket closed */
        return M_AUTH_ERROR;
    }

    pSrv->blen += ret;
    leftlen = pSrv->blen;
    
    if (leftlen < HDRSIZE) {
        AUTHD_EVENT_DEBUG(
                "tac_op_process_reply return leftlen %d when check leftlen", leftlen);
        return M_AUTH_INPROGRESS;
    }

    while (leftlen > HDRSIZE) {
        pktlen = ntohl(msg->length);
        if (pktlen > BODYSIZE) {            
            AUTHD_EVENT_DEBUG(
                    "tac_op_process_reply return pktlen %d when check pktlen", pktlen);
            return M_AUTH_ERROR;
        }

        if ((leftlen - HDRSIZE) < pktlen) {
            break;
        }

        if ((pSess = auth_session_get_by_id(*((u_int32_t *)&msg->session_id[0]))) == NULL) {
            /* session deleted, drop this message */
            leftlen -= (HDRSIZE + pktlen);
            msg = (struct tac_msg *)((char *)msg + (HDRSIZE + pktlen));
            continue;
        }
        
        h = (auth_handle_t *)pSess->pHandle;
        pObj = (struct tac_handle *)h->pObj;
        if (NULL == pObj) {
            if (pCB) {
                pCB(pSess, M_AUTH_ERROR,
                    "Internal error, tacplus handle NULL");
            }
            return M_AUTH_ERROR;
        }
        pObj->response = msg;

        AUTHD_EVENT_DEBUG(
                "tacplus op process reply %p", pObj);

        if (TAC_AUTHEN == pObj->request.type)
        {
            if ((ret = tac_recv_authen(pObj)) == -1) {
                if (pCB) {
                    pCB(pSess, M_AUTH_ERROR, pObj->errmsg);
                }
                return M_AUTH_ERROR;
            }
    
            ret &= 0xff;
            switch (ret) {
            /* modified by liwh for bug 41224, 2017-01-16 */
            case TAC_AUTHEN_STATUS_GETUSER:
                ret = tac_send_user(pObj, pSess);
                if (M_AUTH_ERROR == ret) {
                    if (pCB) {
                        pCB(pSess, M_AUTH_ERROR, pObj->errmsg);
                    }            
                    return M_AUTH_ERROR;
                }
                break;
            /* liwh end */
                
            case TAC_AUTHEN_STATUS_GETPASS:
                ret = tac_send_pass(pObj, pSess);
                if (M_AUTH_ERROR == ret) {
                    if (pCB) {
                        pCB(pSess, M_AUTH_ERROR, pObj->errmsg);
                    }            
                    return M_AUTH_ERROR;
                }
                break;
                
            case TAC_AUTHEN_STATUS_FAIL:
                if (pCB) {
                    pCB(pSess, M_AUTH_FAILED, "access denied");
                }     
                break;
                
            case TAC_AUTHEN_STATUS_PASS:   
                if (pCB) {
                    pCB(pSess, M_AUTH_SUCCESS, "access permited");
                }     
                break;
                
            default:
                if (pCB) {
                    pCB(pSess, M_AUTH_ERROR, 
                        "unknown authentication return code: %d", ret);
                }            
                return M_AUTH_ERROR;
            }
        }
        else if (TAC_AUTHOR == pObj->request.type)
        {
            if ((ret = tac_recv_author(pObj)) == -1) {
                if (pCB) {
                    pCB(pSess, M_AUTH_ERROR, pObj->errmsg);
                }
                return M_AUTH_ERROR;
            }
    
            ret &= 0xff;
            switch (ret) {
            case TAC_AUTHOR_STATUS_PASS_ADD:
                //get priv
                for (i = 0; i < MAXAVPAIRS; i++)
                {
                    if (!pObj->srvr_avs[i].len)
                    {
                        break;
                    }
                    sal_memset(tmp_str, 0, 256);
                    sal_memcpy(tmp_str, pObj->srvr_avs[i].data, pObj->srvr_avs[i].len);
    
                    if (sal_sscanf(tmp_str, "priv-lvl=%d", &priv_lvl))
                    {               
                        AUTHD_EVENT_DEBUG(
                            "tac_op_process_reply receive TAC_AUTHOR_STATUS_PASS_ADD priv_lvl %d", priv_lvl);

                        sal_memset(&vty, 0, sizeof(tbl_vty_t));
                        vty.key.id = pSess->key.index - 1;
        
                        p_vty = tbl_vty_get_vty(&vty.key);
                        if (p_vty)
                        {
                            AUTHD_EVENT_DEBUG(
                               "tac_op_process_reply p_vty->inuse %d ", p_vty->inuse);

                            if (priv_lvl < PRIVILEGE_INVALID)
                            {
                                /* reserve aaa privilege and set vty privilege */
                                p_vty->aaa_privilege = priv_lvl;
                                tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_AAA_PRIVILEGE);
                                 
                                p_vty->config_privilege = p_vty->privilege;  
                                tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_CONFIG_PRIVILEGE);
                                 
                                p_vty->privilege = aaa_map_privilege(priv_lvl);
                                tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_PRIVILEGE);
                            }
                        }
                     }
                     break;
                }

                if (pCB) {
                    pCB(pSess, M_AUTH_SUCCESS, "access permited");
                } 
                break;
                
            case TAC_AUTHOR_STATUS_FAIL:
                if (pCB) {
                    pCB(pSess, M_AUTH_FAILED, "access denied");
                }     
                break;
                
            default:
                if (pCB) {
                    pCB(pSess, M_AUTH_ERROR, 
                        "unknown authorization return code: %d", ret);
                }            
                return M_AUTH_ERROR;
            }    
        }
        else if (TAC_ACCT == pObj->request.type)
        {
            if ((ret = tac_recv_acct(pObj)) == -1) {
                if (pCB) {
                    pCB(pSess, M_AUTH_ERROR, pObj->errmsg);
                }
                return M_AUTH_ERROR;
            }
    
            ret &= 0xff;
            switch (ret) {
            case TAC_AUTHEN_STATUS_PASS:
                if (pCB) {
                    pCB(pSess, M_AUTH_SUCCESS, "access permited");
                } 
                return M_AUTH_SUCCESS;
                
            case TAC_AUTHEN_STATUS_FAIL:
                if (pCB) {
                    pCB(pSess, M_AUTH_FAILED, "access denied");
                }     
                return M_AUTH_SUCCESS;
                
            default:
                if (pCB) {
                    pCB(pSess, M_AUTH_ERROR, 
                        "unknown accounting return code: %d", ret);
                }            
                return M_AUTH_ERROR;
            }    
        }
         
        leftlen -= (HDRSIZE + pktlen);
        msg = (struct tac_msg *)((char *)msg + (HDRSIZE + pktlen));
    }

    /* uncomplete message */
    if (leftlen > 0 && (pSrv->buf != (char *)msg)) {
        memmove(pSrv->buf, msg, leftlen);
        pSrv->blen = leftlen;
    } else {
        pSrv->blen = 0;
    }
    return M_AUTH_INPROGRESS;
}

void tac_op_get_error_msg(auth_handle_t *h, char *pszErrBuf, size_t nBufSz)
{
    struct tac_handle *pObj = (struct tac_handle *)h->pObj;

    if (NULL != pObj && '\0' != pObj->errmsg[0]) {
        snprintf(pszErrBuf, nBufSz, "%s", pObj->errmsg);
    } else {
        snprintf(pszErrBuf, nBufSz, "unknown error");
    }

    AUTHD_EVENT_DEBUG(
            "tacplus op error msg %s tac %p", pszErrBuf, pObj);
    return;
}

auth_be_op_t tacplus_be_ops = {
    .init = tac_op_init,
    .release = tac_op_release,
    .do_connect = tac_op_connect,
    .send_request = tac_op_send_request,
    .process_reply = tac_op_process_reply,
    .get_error_msg = tac_op_get_error_msg,
};  

