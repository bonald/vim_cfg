/****************************************************************************
* $Id$
*  Generate / Convert RSA der hex
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-04-12 17:16
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rsa_derhex.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

/* password process flags */
#define E_PASSWORD_ENCRYPT      0x1
#define E_PASSWORD_DECRYPT      0x0

#ifndef PASSWD_SIZE
#define PASSWD_SIZE         256
#endif

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
static const char rnd_seed[] =
	"string to make the random number generator think it has entropy";
static int g_crypto_initialized = 0;

/****************************************************************************
 *
* Function
* 
****************************************************************************/
int rsa_init_crypto()
{
	if (g_crypto_initialized) {
		return 0;
	}

	g_crypto_initialized = 1;
	ERR_load_crypto_strings();
	OpenSSL_add_ssl_algorithms();
	RAND_seed(rnd_seed, sizeof rnd_seed);
	OpenSSL_add_all_ciphers();
	return 0;
}

/*******************************************************************************
 * Name:    rsa_asn1_length2str
 * Purpose: get asn1 length string, please make sure pOut buffer size
 * Input: 
 *   length: length
 *   nOutLen: out buffer length
 * Output: 
 *   pOut: out buffer
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int rsa_asn1_length2str(int length, unsigned char *pOut, int nOutLen)
{
	unsigned char c;
	int nLeftLen = nOutLen;
	char *p = (char *) pOut;
	int nRet = 0;
	int i = 0;
	int l = 0;
	unsigned char szLenArr[M_RSA_ASN1_LEN_BUFSZ];
	int bytes = 0;

	if (NULL == pOut) {
		return -1;
	}

	if (length <= 127) {
		snprintf((char *) pOut, nOutLen, "%02X", length);
	} else {
		l = length;
		for (i = 0; l > 0; i++) {
			l >>= 8;
		}

		c = i | 0x80;
		nRet = snprintf(p, nLeftLen, "%02X", c);
		p += nRet;
		nLeftLen -= nRet;

		if (i >= M_RSA_ASN1_LEN_BUFSZ) {
			return -1;
		}

		l = i;
		bytes = i;
		szLenArr[i] = '\0';
		while (i-- > 0) {
			szLenArr[i] = length & 0xff;
			length >>= 8;
		}
		i = 0;
		while (bytes-- > 0) {
			nRet = snprintf(p, nLeftLen, "%02X", szLenArr[i]);
			p += nRet;
			nLeftLen -= nRet;
			i++;
		}
	}

	return 0;
}

/*******************************************************************************
 * Name:    rsa_print_bignumber
 * Purpose: print big number to bio
 * Input: 
 *   pnum: big number (NULL should consider success)
 *   buf: 
 *   off: 
 * Output: 
 *   bio: output bio
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int rsa_print_bignumber(BIO * bio, BIGNUM * pnum, unsigned char *buf, int off)
{
	unsigned char szLenBuf[M_RSA_ASN1_LEN_BUFSZ];
	int retlen = 0;
	int n = 0;
	int i = 0;
	const char *neg = NULL;
	int cnt = 0;

	if (NULL == pnum || NULL == bio || NULL == buf || off < 0) {
		return 0;
	}

	if (BIO_write(bio, "\n", 1) <= 0) {
		return -1;
	}

	/* output length */
	retlen = BN_num_bytes(pnum);
	if ((rsa_asn1_length2str(retlen, szLenBuf, M_RSA_ASN1_LEN_BUFSZ)) != 0) {
		return -1;
	}
	if (!BIO_indent(bio, off, off)) {
		return -1;
	}
	if (BIO_printf(bio, "02%s\n", szLenBuf) < 0) {
		return -1;
	}

	/* output big number */
	if (!BIO_indent(bio, off + 2, off + 2)) {
		return -1;
	}

	neg = (pnum->neg != 0) ? "-" : "";
	if (BN_is_zero(pnum)) {
		if (BIO_printf(bio, "0") <= 0) {
			return -1;
		}
		return 0;
	}

	buf[0] = 0;
	if (pnum->neg != 0 && BIO_printf(bio, "%s", neg) <= 0) {
		return -1;
	}
	n = BN_bn2bin(pnum, &buf[1]);
	if (buf[1] & 0x80) {
		n++;
	} else {
		buf++;
	}

	for (i = (retlen == n) ? 0 : 1; i < n; i++) {
		if (BIO_printf(bio, "%02X", buf[i]) <= 0) {
			return -1;
		}

		cnt++;
		if ((cnt % 32) == 0 && ((i + 1) != n)) {
			if (BIO_puts(bio, "\n") <= 0
				|| !BIO_indent(bio, off + 2, off + 2)) {
				return -1;
			}
		} else if ((cnt % 4) == 0) {
			if (BIO_printf(bio, " ") <= 0) {
				return -1;
			}
		}
	}

	return 0;
}

/*******************************************************************************
 * Name:    rsa_convert_key_der2str
 * Purpose: convert rsk key to der format hex string
 * Input: 
 *   prsa: rsa key
 *   nBufLen: buf length
 *   bPubOnly: show only public key for private key
 * Output: 
 *   pBuf: buffer
 *   pnRetLen: data length
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
#define M_RSA_KEY_INC_TOTAL(total, i) \
do  \
{ \
    if ((i) <= 127) \
    { \
        (total) += ((i) + 2); \
    } \
    else \
    { \
        int _lll_ = (i); \
        int _iii_ = 0; \
        for (_iii_ = 0; _lll_ > 0; _iii_++) \
        { \
            _lll_ >>= 8; \
        } \
        (total) += ((i) + 2 + _iii_); \
    } \
} \
while (0)
int
rsa_convert_key_der2str(RSA * prsa, u_int8_t bPubOnly,
						char *pBuf, u_int16_t nBufLen, int *pnRetLen)
{
	unsigned char szLenBuf[M_RSA_ASN1_LEN_BUFSZ];
	unsigned char *m = NULL;
	int ret = -1;
	size_t buf_len = 0;
	u_int32_t total = 0;
	BIO *bio = NULL;
	int i = 0;
	u_int8_t bOutPriKey = 0;

	if (NULL == prsa || NULL == pBuf || NULL == pnRetLen) {
		return -1;
	}

	if (!bPubOnly) {
		bOutPriKey = 1;
	}

	*pnRetLen = 0;
	if ((bio = BIO_new(BIO_s_mem())) == NULL) {
		/* out of memory */
		goto err_out;
	}

	/* get max buffer length */
	if (bOutPriKey && prsa->d) {
		/* add version number */
		total = 3;
	}
	if (prsa->n) {
		buf_len = (size_t) BN_num_bytes(prsa->n);
		M_RSA_KEY_INC_TOTAL(total, buf_len);
	}
	if (prsa->e) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->e))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}
	if (bOutPriKey && prsa->d) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->d))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}
	if (bOutPriKey && prsa->p) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->p))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}
	if (bOutPriKey && prsa->q) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->q))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}
	if (bOutPriKey && prsa->dmp1) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->dmp1))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}
	if (bOutPriKey && prsa->dmq1) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->dmq1))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}
	if (bOutPriKey && prsa->iqmp) {
		if (buf_len < (i = (size_t) BN_num_bytes(prsa->iqmp))) {
			buf_len = i;
		}
		M_RSA_KEY_INC_TOTAL(total, i);
	}

	m = malloc(buf_len + 32);
	if (NULL == m) {
		/* out of memory */
		goto err_out;
	}

	/* output header */
	if ((rsa_asn1_length2str(total, szLenBuf, M_RSA_ASN1_LEN_BUFSZ)) != 0) {
		goto err_out;
	}
	if (BIO_printf(bio, "  30%s", szLenBuf) < 0) {
		goto err_out;
	}

	/* print version */
	if (bOutPriKey && prsa->d) {
		if (BIO_printf(bio, "\n    0201\n      00") < 0) {
			goto err_out;
		}
	}

	/* print big numbers one bye one */
	if ((rsa_print_bignumber(bio, prsa->n, m, 4) != 0)
		|| (rsa_print_bignumber(bio, prsa->e, m, 4) != 0)) {
		goto err_out;
	}
	if (bOutPriKey) {
		if ((rsa_print_bignumber(bio, prsa->d, m, 4) != 0)
			|| (rsa_print_bignumber(bio, prsa->p, m, 4) != 0)
			|| (rsa_print_bignumber(bio, prsa->q, m, 4) != 0)
			|| (rsa_print_bignumber(bio, prsa->dmp1, m, 4) != 0)
			|| (rsa_print_bignumber(bio, prsa->dmq1, m, 4) != 0)
			|| (rsa_print_bignumber(bio, prsa->iqmp, m, 4) != 0)) {
			goto err_out;
		}
	}

	if ((i = BIO_read(bio, pBuf, nBufLen)) > 0) {
		ret = 0;
		*pnRetLen = i;
	}

  err_out:
	if (NULL != bio) {
		BIO_free(bio);
	}
	if (NULL != m) {
		free(m);
	}
	return ret;
}

/*******************************************************************************
 * Name:    rsa_convert_str2der
 * Purpose: convert string 2 der format
 * Input: 
 *   pszKey: key string
 *   nKeyLen: key string length
 *   nBufLen: output buffer length
 * Output: 
 *   pBuf: output buffer 
 *   pnLen: outputted length
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
rsa_convert_str2der(char *pszKey, u_int16_t nKeyLen, char *pBuf,
					u_int16_t nBufLen, int *pnLen)
{
	char *p = NULL;
	unsigned int t[2];
	int i = 0;
	int n = 0;
	int nOutLen = 0;

	if (NULL == pszKey || 0 >= nKeyLen
		|| NULL == pBuf || 0 >= nBufLen || NULL == pnLen) {
		return -1;
	}

	p = pszKey;
	nOutLen = 0;
	for (i = 0; i < nKeyLen; i++, p++) {
		if (!(isxdigit(*p)
			  || isspace(*p)
			  || '\r' == *p || '\t' == *p || '\n' == *p)) {
			/* invalid peer public key code */
			return -1;
		}

		if (isdigit(*p)) {
			t[n] = *p - '0';
		} else if (isxdigit(*p)) {
			t[n] = 10 + tolower(*p) - 'a';
		} else {
			continue;
		}

		n++;
		if (2 == n) {
			if ((nOutLen + 1) >= nBufLen) {
				/* buffer overflow */
				return -1;
			}
			pBuf[nOutLen] = t[0] * 16 + t[1];
			nOutLen++;
			n = 0;
		}
	}

	if (n > 0) {
		/* odd error */
		return -1;
	}

	*pnLen = nOutLen;
	return 0;
}

static int _i64c(int i)
{
	i &= 0x3f;
	if (i == 0)
		return '.';
	if (i == 1)
		return '/';
	if (i < 12)
		return ('0' - 2 + i);
	if (i < 38)
		return ('A' - 12 + i);
	return ('a' - 38 + i);
}

static void _crypt_make_salt(char *p, int cnt)
{
	unsigned x = x;		/* it's pointless to initialize it anyway :) */
	struct timeval tv;

	gettimeofday(&tv, NULL);
	x += getpid() + tv.tv_sec + tv.tv_usec + clock();
	do {
		/* x = (x*1664525 + 1013904223) % 2^32 generator is lame
		 * (low-order bit is not "random", etc...),
		 * but for our purposes it is good enough */
		x = x * 1664525 + 1013904223;
		/* BTW, Park and Miller's "minimal standard generator" is
		 * x = x*16807 % ((2^31)-1)
		 * It has no problem with visibly alternating lowest bit
		 * but is also weak in cryptographic sense + needs div,
		 * which needs more code (and slower) on many CPUs */
		*p++ = _i64c(x >> 16);
		*p++ = _i64c(x >> 22);
	} while (--cnt);
	*p = '\0';
}

#ifndef CTC_USE_CRYPT

/*******************************************************************************
 * Name:    do_encrypt
 * Purpose: do encrypt, have '\0' at the buffer end
 * Input: 
 *   pw: password
 *   ibuf: input buffer
 *   ilen: input length
 *   olen: output length
 * Output: 
 *   obuf: output buffer
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int do_encrypt(char *pw, char *ibuf, size_t ilen, char *obuf, size_t olen)
{
	unsigned char ebuf[PASSWD_SIZE];
	unsigned char iv[EVP_MAX_IV_LENGTH];
	unsigned char key[EVP_MAX_KEY_LENGTH];
	EVP_CIPHER_CTX ectx;
	char *pout = NULL;
	int ebuflen = 0;
	int nretlen = 0;
	int nleftlen = olen;
	int i = 0;

	if (NULL == ibuf || NULL == ebuf || !strcmp(ibuf, "")) {
		return -1;
	}

	obuf[0] = '\0';
	memcpy(iv, INIT_VECTOR, sizeof(iv));
	EVP_BytesToKey(M_ALG, EVP_md5(), (unsigned char *) "salu",
				   (unsigned char *) pw, strlen(pw), 1, key, iv);
	EVP_CIPHER_CTX_init(&ectx);
	EVP_CipherInit_ex(&ectx, M_ALG, NULL, key, iv, E_PASSWORD_ENCRYPT);
	EVP_CipherUpdate(&ectx, ebuf, &ebuflen, (unsigned char *) ibuf, ilen);
	if (ebuflen < 0) {
		EVP_CIPHER_CTX_cleanup(&ectx);
		obuf[0] = '\0';
		return -1;
	}

	pout = (char *) obuf;
	for (i = 0; i < ebuflen; i++) {
		if (nleftlen < 3) {
			/* output buffer overflow */
			obuf[0] = '\0';
			return -1;
		}

		nretlen = snprintf(pout, nleftlen, "%02x", (ebuf[i] & 0xff));
		nleftlen -= nretlen;
		pout += nretlen;
	}

	EVP_CipherFinal_ex(&ectx, ebuf, &ebuflen);
	EVP_CIPHER_CTX_cleanup(&ectx);
	if (ebuflen < 0) {
		obuf[0] = '\0';
		return -1;
	}
	for (i = 0; i < ebuflen; i++) {
		if (nleftlen < 3) {
			/* output buffer overflow */
			obuf[0] = '\0';
			return -1;
		}

		nretlen = snprintf(pout, nleftlen, "%02x", (ebuf[i] & 0xff));
		nleftlen -= nretlen;
		pout += nretlen;
	}

	if ('\0' == obuf[0]) {
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * Name:    do_decrypt
 * Purpose: do decrypt, return string will have '\0'
 * Input: 
 *   pw: password
 *   ibuf: input buffer
 *   ilen: input length
 *   olen: output buffer length
 * Output: 
 *   obuf: output buffer
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int do_decrypt(char *pw, char *ibuf, size_t ilen, char *obuf, size_t olen)
{
	char buf[PASSWD_SIZE];
	unsigned char iv[EVP_MAX_IV_LENGTH];
	unsigned char key[EVP_MAX_KEY_LENGTH];
	EVP_CIPHER_CTX ectx;
	char *pout = NULL;
	int nretlen = 0;
	int i = 0;

	if (ibuf == NULL || NULL == obuf) {
		return -1;
	}

	obuf[0] = '\0';
	pout = buf;
	for (i = 0; i < (ilen - 1); i += 2) {
		unsigned short value = 0;

		sscanf((ibuf + i), "%2hx", &value);
		*pout = (value & 0xff);
		pout++;
	}

	memcpy(iv, INIT_VECTOR, sizeof(iv));
	EVP_BytesToKey(M_ALG, EVP_md5(), (unsigned char *) "salu",
				   (unsigned char *) pw, strlen(pw), 1, key, iv);
	EVP_CIPHER_CTX_init(&ectx);
	EVP_CipherInit_ex(&ectx, M_ALG, NULL, key, iv, E_PASSWORD_DECRYPT);
	EVP_CipherUpdate(&ectx, (unsigned char *) obuf, &nretlen,
					 (unsigned char *) buf, pout - buf);
	if (nretlen < 0) {
		EVP_CIPHER_CTX_cleanup(&ectx);
		obuf[0] = '\0';
		return -1;
	}

	pout = (char *) obuf + nretlen;
	EVP_CipherFinal_ex(&ectx, (unsigned char *) pout, &nretlen);
	EVP_CIPHER_CTX_cleanup(&ectx);
	if (nretlen < 0) {
		obuf[0] = '\0';
		return -1;
	}

	pout[nretlen] = '\0';
	if ('\0' == obuf[0]) {
		return -1;
	}
	return 0;
}

int gen_passwd_crypt(char *pszPin, char *pszDestBuf, size_t bufsz)
{
	return do_encrypt(M_DEFAULT_PW_STR, pszPin, strlen(pszPin), pszDestBuf,
					  bufsz);
}

int convert_passwd_for_nss(char *ibuf, size_t ilen, char *obuf, size_t olen)
{
	char szBuf[PASSWD_SIZE];
	char salt[sizeof("$N$XXXXXXXX")];	/* "$N$XXXXXXXX" or "XX" */
	char *tmp;

	if (do_decrypt(M_DEFAULT_PW_STR, ibuf, ilen, szBuf, PASSWD_SIZE) != 0) {
		return -1;
	}

	strcpy(salt, "$1$");
	_crypt_make_salt(salt + 3, 4);

	tmp = crypt((const char *) szBuf, (const char *) salt);
	snprintf(obuf, olen, "%s", tmp);
	return 0;
}

int gen_passwd_validate(char *pszOldPasswd, char *pszInput)
{
	char szBuf[PASSWD_SIZE];

	if (do_encrypt
		(M_DEFAULT_PW_STR, pszInput, strlen(pszInput), szBuf,
		 PASSWD_SIZE) != 0) {
		return -1;
	}

	if (!strcmp(pszOldPasswd, szBuf)) {
		return 1;
	}

	return 0;
}

#else							/* ! CTC_USE_CRYPT */

int gen_passwd_crypt(char *pszPin, char *pszDestBuf, size_t bufsz)
{
	char salt[sizeof("$N$XXXXXXXX")];	/* "$N$XXXXXXXX" or "XX" */
	char *tmp;
	int noutput = 0;
	int nret;

	if (bufsz <= 96) {
		return -1;
	}

	_crypt_make_salt(salt, 1);	/* des */
	//strcpy(salt, "$1$");
	//_crypt_make_salt(salt + 3, 4);

	tmp = crypt((const char *) pszPin, (const char *) salt);

	/* convert to hex */
	do {
		nret = snprintf(pszDestBuf + noutput, bufsz - noutput,
						"%02x", (*tmp & 0xff));
		noutput += nret;
		if (noutput >= (bufsz - 1)) {
			assert(0);
			return -1;
		}

		tmp++;
	} while ('\0' != *tmp);

	pszDestBuf[noutput] = '\0';
	return 0;
}

int convert_passwd_for_nss(char *ibuf, size_t ilen, char *obuf, size_t olen)
{
	unsigned short value;
	char *pout = NULL;
	int i = 0;

	if (ibuf == NULL || NULL == obuf) {
		return -1;
	}

	pout = obuf;
	for (i = 0; i < (ilen - 1); i += 2) {
		sscanf((ibuf + i), "%2hx", &value);
		*pout = (value & 0xff);
		pout++;

		if ((pout - obuf) >= (olen - 1)) {
			assert(0);
			return -1;
		}
	}

	obuf[(pout - obuf)] = '\0';
	return 0;
}

int gen_passwd_validate(char *pszOldPasswd, char *pszInput)
{
	char szBuf[256];
	char *pszEnc;

	if (convert_passwd_for_nss(pszOldPasswd, strlen(pszOldPasswd), szBuf, 256)
		!= 0) {
		assert(0);
		return -1;
	}

	pszEnc = crypt(pszInput, szBuf);
	if (NULL == pszEnc || strcmp(szBuf, pszEnc)) {
		return 0;
	}

	return 1;
}

#endif							/* CTC_USE_CRYPT */

int rsa_key_bits_check(RSA * prsa, char *pszErr, size_t err_bufsz)
{
	int nMod = 0;

	if (prsa == NULL) {
		return -1;
	}

	nMod = BN_num_bits(prsa->n);
	if (nMod < M_RSA_IMPORT_KEY_MIN_MODULUS) {
		snprintf(pszErr, err_bufsz,
				 "Modulus too short (%d bit). Please don't less than %d bit.",
				 nMod, M_RSA_IMPORT_KEY_MIN_MODULUS);
		return -1;
	} else if (nMod > M_RSA_IMPORT_KEY_MAX_MODULUS) {
		snprintf(pszErr, err_bufsz,
				 "Modulus too long (%d bit). Please don't larger than %d bit",
				 nMod, M_RSA_IMPORT_KEY_MAX_MODULUS);
		return -1;
	}

	return 0;
}


/*******************************************************************************
 * Name:    rsa_bio_trim_lines_add
 * Purpose: 
 * Input: 
 *   pBuf: lines buffer
 *   size: buffer size
 * Output: 
 *   bio: output bio
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int rsa_bio_trim_lines_add(BIO * bio, char *pBuf, int size)
{
	BIO *btmp = NULL;

#define M_RSA_TRIM_BUF_SZ     1024
	char szBuf[M_RSA_TRIM_BUF_SZ];
	char *p = NULL;
	char *q = NULL;
	int nLen = 0;

	if (NULL == bio || NULL == pBuf) {
		return -1;
	}

	btmp = BIO_new(BIO_s_mem());
	if (NULL == btmp) {
		return -1;
	}

	if (size != BIO_write(btmp, pBuf, size)) {
		BIO_free(btmp);
		return -1;
	}

	while (BIO_gets(btmp, szBuf, 1024) > 0) {
		p = szBuf;

		/* skip at header */
		while ('\0' != *p) {
			if ('\t' == *p || ' ' == *p || '\r' == *p || '\n' == *p) {
				p++;
				continue;
			}
			break;
		}

		if ('\0' == *p) {
			/* skip blank line */
			continue;
		}

		/* trim \t & space at tail */
		q = p + strlen(p) - 1;
		while (q > p) {
			if ('\t' == *q || ' ' == *q || '\r' == *q || '\n' == *q) {
				q--;
				continue;
			}
			break;
		}

		/* try to add this new line */
		nLen = q - p + 1;
		if (nLen != BIO_write(bio, p, nLen)) {
			BIO_free(btmp);
			return -1;
		}

		/* try add \n */
		if (1 != BIO_write(bio, "\n", 1)) {
			BIO_free(btmp);
			return -1;
		}
		if (!strncasecmp(p, "DEK-Info:", 9)) {
			if (1 != BIO_write(bio, "\n", 1)) {
				BIO_free(btmp);
				return -1;
			}
		}
	}

	if (1 != BIO_write(bio, "\n", 1)) {
		BIO_free(btmp);
		return -1;
	}
	BIO_free(btmp);
	return 0;
}

/*******************************************************************************
 * Name:    rsa_password_callback
 * Purpose: callback to input password for RSA
 * Input: 
 *   buf: output buf
 *   size: output buf size
 *   rwflag: r/w flag
 *   userdata: callback arg
 * Output: 
 * Return: 
 *   success: password length
 *   failed : 
 * Note: 
 ******************************************************************************/
int rsa_password_callback(char *buf, int size, int rwflag, void *userdata)
{
	unsigned char *pszPassword = (unsigned char *) userdata;

	if (NULL == buf || NULL == pszPassword) {
		return 0;
	}

	snprintf(buf, size, "%s", pszPassword);
	return (strlen(buf));
}

/*******************************************************************************
 * Name:    validate_rsa_key_pem
 * Purpose: validate rsa key (PEM format)
 * Input: 
 *   pszPasswd: rsa key password
 *   pszKey: key 
 *   nKeyLen: key length
 *   bPrivateKey: is it private key
 * Output: 
 *   pprsa: return rsa pointer (may be NULL)
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
validate_rsa_key_pem(const char *pszPasswd, char *pszKey, u_int16_t nKeyLen,
					 int bPrivateKey, RSA ** pprsa, char *pszErr,
					 size_t err_bufsz)
{
	EVP_PKEY *pkey = NULL;
	BIO *bio = NULL;
	int nRet = -1;
	RSA *prsa = NULL;

	bio = BIO_new(BIO_s_mem());
	if (NULL == bio) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}

	if (rsa_bio_trim_lines_add(bio, pszKey, nKeyLen) != 0) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}
	if (bPrivateKey) {
		pkey = PEM_read_bio_PrivateKey(bio, NULL,
									   rsa_password_callback,
									   (void *) pszPasswd);
	} else {
		pkey = PEM_read_bio_PUBKEY(bio, NULL,
								   rsa_password_callback, (void *) pszPasswd);
	}
	if (NULL == pkey) {
		if (PEM_R_BAD_PASSWORD_READ == ERR_GET_REASON(ERR_peek_last_error())) {
			snprintf(pszErr, err_bufsz, "Please specify password");
		} else if (PEM_R_BAD_DECRYPT == ERR_GET_REASON(ERR_peek_last_error())) {
			snprintf(pszErr, err_bufsz, "Invalid password");
		} else {
			snprintf(pszErr, err_bufsz, "Invalid key string");
		}
		goto err_out;
	}

	if ((prsa = EVP_PKEY_get1_RSA(pkey)) == NULL) {
		snprintf(pszErr, err_bufsz, "Extract RSA key failed");
		goto err_out;
	}

	if (rsa_key_bits_check(prsa, pszErr, err_bufsz) != 0) {
		goto err_out;
	}

	nRet = 0;
	if (NULL != pprsa) {
		*pprsa = prsa;
		prsa = NULL;	/* don't free it twice */
	}

  err_out:
	if (NULL != prsa) {
		RSA_free(prsa);
	}

	if (NULL != pkey) {
		EVP_PKEY_free(pkey);
	}

	if (NULL != bio) {
		BIO_free(bio);
	}
	return nRet;
}

/*******************************************************************************
 * Name:    validate_rsa_key_der
 * Purpose: validate rsa key (for der format)
 * Input: 
 *   pszKey: key string
 *   nKeyLen: key string length
 *   bPrivateKey: is it private key ?
 * Output: 
 *   pprsa: get rsa pointer (may be NULL)
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
validate_rsa_key_der(char *pszKey, u_int16_t nKeyLen,
					 int bPrivateKey, RSA ** pprsa, char *pszErr,
					 size_t err_bufsz)
{
	RSA *prsa = NULL;
	char *pBuf = NULL;
	char *pRet = NULL;
	int nLen = 0;
	int nRet = 0;
	BIO *bio = NULL;

	if (NULL == (pBuf = malloc(M_RSA_KEYSTR_BUFSZ * 2))) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		return -1;
	}

	if (-1 == rsa_convert_str2der(pszKey, nKeyLen, pBuf,
								  M_RSA_KEYSTR_BUFSZ * 2, &nLen)) {
		snprintf(pszErr, err_bufsz, "Invalid key string");
		nRet = -1;
		goto err_out;
	}

	pRet = pBuf;
	/* try short format */
	if (bPrivateKey) {
		prsa = d2i_RSAPrivateKey(NULL, (const unsigned char **) &pRet, nLen);
	} else {
		prsa = d2i_RSAPublicKey(NULL, (const unsigned char **) &pRet, nLen);
	}
	if (NULL == prsa) {
		/* try long format */
		EVP_PKEY *pkey = NULL;

		bio = BIO_new(BIO_s_mem());
		if (NULL == bio || (nLen != BIO_write(bio, pBuf, nLen))) {
			snprintf(pszErr, err_bufsz, "WARNING: out of memory");
			nRet = -1;
			goto err_out;
		}

		if (bPrivateKey) {
			pkey = d2i_PrivateKey_bio(bio, NULL);
		} else {
			pkey = d2i_PUBKEY_bio(bio, NULL);
		}
		if (NULL != pkey) {
			prsa = EVP_PKEY_get1_RSA(pkey);
			EVP_PKEY_free(pkey);
		}
	}

	if (NULL == prsa) {
		snprintf(pszErr, err_bufsz, "Invalid key string");
		nRet = -1;
		goto err_out;
	}

	if (rsa_key_bits_check(prsa, pszErr, err_bufsz) != 0) {
		nRet = -1;
		goto err_out;
	}

	/* need return prsa pointer */
	if (NULL != pprsa) {
		*pprsa = prsa;
		prsa = NULL;	/* don't free twice */
	}

  err_out:
	if (NULL != pBuf) {
		free(pBuf);
	}
	if (NULL != prsa) {
		RSA_free(prsa);
	}
	if (NULL != bio) {
		BIO_free(bio);
	}

	return nRet;
}

/* pem, der-hex, der */
int rsa_pub_convert_format(RSA * pRsa, char *pszType,
						   char *pszOut, size_t out_bufsz, char *pszErr,
						   size_t err_bufsz, size_t * pOutBytes)
{
	BIO *bio = NULL;
	int nRet = -1;
	int nSize = 0;
	char *pBuf = NULL;
	char *pMem = NULL;
	long destSize;

	if (NULL == pRsa || NULL == pszOut || NULL == pszType) {
		return -1;
	}

	if (strcasecmp(pszType, "pem")
		&& strcasecmp(pszType, "der-hex")
		&& strcasecmp(pszType, "der")) {
		snprintf(pszErr, err_bufsz, "Invalid format type: %s", pszType);
		return -1;
	}

	/* prepare bio */
	if ((bio = BIO_new(BIO_s_mem())) == NULL) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}

	if (!strcasecmp(pszType, "pem")) {
		if (!PEM_write_bio_RSA_PUBKEY(bio, pRsa)) {
			snprintf(pszErr, err_bufsz,
					 "Convert RSA key to PEM format failed");
			goto err_out;
		}
	} else if (!strcasecmp(pszType, "der")) {
		if (!i2d_RSA_PUBKEY_bio(bio, pRsa)) {
			snprintf(pszErr, err_bufsz,
					 "Convert RSA key to DER format failed");
			goto err_out;
		}
	} else if (!strcasecmp(pszType, "der-hex")) {
		if ((pBuf = malloc(M_RSA_KEYSTR_BUFSZ)) == NULL) {
			snprintf(pszErr, err_bufsz, "WARNING: out of memory");
			goto err_out;
		}

		if (rsa_convert_key_der2str(pRsa, 1, pBuf, M_RSA_KEYSTR_BUFSZ, &nSize)
			!= 0 || nSize != BIO_write(bio, pBuf, nSize)) {
			snprintf(pszErr, err_bufsz,
					 "Convert RSA key to DER hex format failed");
			goto err_out;
		}
	}

	destSize = BIO_get_mem_data(bio, &pMem);
	if (destSize >= out_bufsz) {
		snprintf(pszErr, err_bufsz, "Destination buffer too small");
		goto err_out;
	}
	if (pOutBytes) {
		*pOutBytes = destSize;
	}
	memcpy(pszOut, pMem, destSize);
	pszOut[destSize] = '\0';
	if ('\n' == pszOut[destSize - 1]) {
		pszOut[destSize - 1] = '\0';
	}
	nRet = 0;

  err_out:
	if (NULL != bio) {
		BIO_free(bio);
	}
	if (NULL != pBuf) {
		free(pBuf);
	}
	return nRet;
}

int
rsa_pri_convert_format(RSA * pRsa, char *pszType,
					   char *pszEnc, char *pszPassword,
					   char *pszOut, size_t out_bufsz,
					   char *pszErr, size_t err_bufsz, size_t * pOutBytes)
{
	BIO *bio = NULL;
	int nRet = -1;
	int nSize = 0;
	char *pBuf = NULL;
	const EVP_CIPHER *pEnc = NULL;	/* don't free it */
	char *pMem = NULL;
	long destSize;

	if (NULL == pRsa || NULL == pszType || NULL == pszOut) {
		return -1;
	}

	if (strcasecmp(pszType, "pem")
		&& strcasecmp(pszType, "der-hex")
		&& strcasecmp(pszType, "der")) {
		snprintf(pszErr, err_bufsz, "Invalid format type: %s", pszType);
		return -1;
	}

	/* prepare bio */
	if ((bio = BIO_new(BIO_s_mem())) == NULL) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}

	/* get Encrypt algorithm */
	if (NULL != pszEnc) {
		if (!strcasecmp("3des", pszEnc)) {
			pEnc = EVP_get_cipherbyname("des3");
		} else {
			pEnc = EVP_get_cipherbyname(pszEnc);
		}
		if (NULL == pEnc) {
			snprintf(pszErr, err_bufsz, "Unsupported encrypt algorithm: %s\n",
					 pszEnc);
			goto err_out;
		}
	}

	if (!strcasecmp(pszType, "pem")) {
		if (!PEM_write_bio_RSAPrivateKey(bio, pRsa,
										 pEnc, NULL, 0, NULL, pszPassword)) {
			snprintf(pszErr, err_bufsz,
					 "Convert RSA key to PEM format failed");
			goto err_out;
		}
	} else if (!strcasecmp(pszType, "der")) {
		if (!i2d_RSAPrivateKey_bio(bio, pRsa)) {
			snprintf(pszErr, err_bufsz,
					 "Convert RSA key to DER format failed");
			goto err_out;
		}
	} else if (!strcasecmp(pszType, "der-hex")) {
		if ((pBuf = malloc(M_RSA_KEYSTR_BUFSZ)) == NULL) {
			snprintf(pszErr, err_bufsz, "WARNING: out of memory");
			goto err_out;
		}

		if (rsa_convert_key_der2str(pRsa, 0, pBuf, M_RSA_KEYSTR_BUFSZ, &nSize)
			!= 0 || nSize != BIO_write(bio, pBuf, nSize)) {
			snprintf(pszErr, err_bufsz,
					 "Convert RSA key to DER hex format failed");
			goto err_out;
		}
	} else {
		snprintf(pszErr, err_bufsz, "Unsupported export type");
		assert(0);
		goto err_out;
	}

	destSize = BIO_get_mem_data(bio, &pMem);
	if (destSize >= out_bufsz) {
		snprintf(pszErr, err_bufsz, "Destination buffer too small");
		goto err_out;
	}
	if (pOutBytes) {
		*pOutBytes = destSize;
	}
	memcpy(pszOut, pMem, destSize);
	pszOut[destSize] = '\0';
	if ('\n' == pszOut[destSize - 1]) {
		pszOut[destSize - 1] = '\0';
	}
	nRet = 0;

  err_out:
	if (NULL != bio) {
		BIO_free(bio);
	}
	if (NULL != pBuf) {
		free(pBuf);
	}
	return nRet;
}

int
export_rsa_to_file(RSA * pRsa, int bPrivate, char *pszEnc, char *pszPassword,
				   char *pszDestType, char *pszFilename, char *pszErr,
				   size_t err_bufsz)
{
	int nRet = -1;
	FILE *fp = NULL;
	int nResult;
	char *pszOut = NULL;

	if (NULL == (fp = fopen(pszFilename, "w"))) {
		snprintf(pszErr, err_bufsz, "open file failed: %s", strerror(errno));
		goto err_out;
	}
	if (NULL == (pszOut = malloc(M_RSA_KEYSTR_BUFSZ))) {
		snprintf(pszErr, err_bufsz, "Out of memory");
		goto err_out;
	}

	if (bPrivate) {
		nResult =
			rsa_pri_convert_format(pRsa, pszDestType, pszEnc, pszPassword,
								   pszOut, M_RSA_KEYSTR_BUFSZ, pszErr,
								   err_bufsz, NULL);
	} else {
		nResult = rsa_pub_convert_format(pRsa, pszDestType, pszOut,
										 M_RSA_KEYSTR_BUFSZ, pszErr,
										 err_bufsz, NULL);
	}
	if (0 != nResult) {
		goto err_out;
	}

	fwrite(pszOut, strlen(pszOut), 1, fp);
	fprintf(fp, "\n");

	nRet = 0;

  err_out:
	if (NULL != pszOut) {
		free(pszOut);
	}
	if (NULL != fp) {
		fclose(fp);
	}
	if (0 != nRet) {
		unlink(pszFilename);
	} else {
		chmod(pszFilename, S_IRUSR | S_IWUSR);
	}
	return nRet;
}

int
rsa_load_der_file(char *pszFileName, int bPublicKey,
				  RSA ** ppRsa, char *pszErr, size_t err_bufsz)
{
	int nRet = -1;
	BIO *bio = NULL;
	EVP_PKEY *pEvp = NULL;
	RSA *pRsa = NULL;

	if (NULL == pszFileName || NULL == pszErr) {
		return -1;
	}

	/* prepare input bio */
	if ((bio = BIO_new(BIO_s_file())) == NULL) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}

	if (BIO_read_filename(bio, pszFileName) < 0) {
		snprintf(pszErr, err_bufsz, "Open file for read failed: %s\n",
				 strerror(errno));
		goto err_out;
	}

	/* load it as private key */
	pEvp = d2i_PrivateKey_bio(bio, NULL);
	if (NULL == pEvp && bPublicKey) {
        /* load it as public key */
	    BIO_seek(bio, 0);
	    pEvp = d2i_PUBKEY_bio(bio, NULL);
	}
	if (NULL != pEvp) {
		pRsa = EVP_PKEY_get1_RSA(pEvp);
	}
	if (NULL == pRsa) {
		snprintf(pszErr, err_bufsz, "Invalid key file");
		goto err_out;
	}

	if (rsa_key_bits_check(pRsa, pszErr, err_bufsz) != 0) {
		goto err_out;
	}

    if (ppRsa) {
        *ppRsa = pRsa;
        pRsa = NULL;
    }
	nRet = 0;

 err_out:
	if (pRsa) {
		RSA_free(pRsa);
	}
	if (pEvp) {
		EVP_PKEY_free(pEvp);
	}
	if (bio) {
		BIO_free(bio);
	}
	return nRet;
}

int
rsa_load_derhex_file(char *pszFileName, int bPublicKey,
					 RSA ** ppRsa, char *pszErr, size_t err_bufsz)
{
	int nRet = -1;
	BIO *bio = NULL;
	BIO *btmp = NULL;
	char *pBuf = NULL;
	char *pBufDer;
	EVP_PKEY *pEvp = NULL;
	RSA *pRsa = NULL;
	char szLine[M_RSA_BIO_LINE_BUFSZ];
	char *p = NULL;
	int nLeftLen = 0;
	int nSize = 0;

	if (NULL == pszErr || NULL == pszFileName) {
		goto err_out;
	}

	/* prepare input bio */
	if ((bio = BIO_new(BIO_s_file())) == NULL) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}

	if (BIO_read_filename(bio, pszFileName) < 0) {
		snprintf(pszErr, err_bufsz, "Open file for read failed: %s\n",
				 strerror(errno));
		goto err_out;
	}

	/* load key sting */
	if ((pBuf = malloc(M_RSA_KEYSTR_BUFSZ * 2)) == NULL ) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}
	pBufDer = pBuf + M_RSA_KEYSTR_BUFSZ;
	p = pBuf;
	nLeftLen = M_RSA_KEYSTR_BUFSZ;
	while ((nSize = BIO_gets(bio, szLine, M_RSA_BIO_LINE_BUFSZ)) > 0) {
		if ((nSize + 1) >= nLeftLen) {
			snprintf(pszErr, err_bufsz,
					 "DER HEX string too long. It is length range is [1, %d)",
					 M_RSA_KEYSTR_BUFSZ);
			goto err_out;
		}
		memcpy(p, szLine, nSize);
		p[nSize] = '\0';
		p += nSize;
		nLeftLen -= nSize;
	}

	/* convert it to der format */
	if (rsa_convert_str2der(pBuf, strlen(pBuf),
							pBufDer, M_RSA_KEYSTR_BUFSZ, &nSize) != 0) {
		/* invalid key string */
		snprintf(pszErr, err_bufsz, "Invalid DER HEX key string");
		goto err_out;
	}
	pBufDer[nSize] = '\0';

	/* try load it short mode, private key */
	p = pBufDer;
	pRsa = d2i_RSAPrivateKey(NULL, (const unsigned char **) &p, nSize);
    if (NULL == pRsa && bPublicKey) {
        /* try load it as public key */
        p = pBufDer;
        pRsa = d2i_RSAPublicKey(NULL, (const unsigned char **)&p, nSize);
    }
   
	if (NULL == pRsa) {
		/* try long format, private key */
		if ((btmp = BIO_new(BIO_s_mem())) == NULL
			|| nSize != BIO_write(btmp, pBufDer, nSize)) {
			snprintf(pszErr, err_bufsz, "WARNING: out of memory");
			goto err_out;
		}
		pEvp = d2i_PrivateKey_bio(btmp, NULL);
		if (NULL == pEvp && bPublicKey) {
		    /* try load it as public key */
		    pEvp = d2i_PUBKEY_bio(btmp, NULL);
		}
		if (NULL != pEvp) {
			pRsa = EVP_PKEY_get1_RSA(pEvp);
		}
	}

	if (NULL == pRsa) {
		snprintf(pszErr, err_bufsz, "Invalid key file");
		goto err_out;
	}

	if (rsa_key_bits_check(pRsa, pszErr, err_bufsz) != 0) {
		goto err_out;
	}

	if (ppRsa) {
	    *ppRsa = pRsa;
		pRsa = NULL;
	}

	nRet = 0;

  err_out:
	if (pRsa) {
		RSA_free(pRsa);
	}
	if (pEvp) {
		EVP_PKEY_free(pEvp);
	}
	if (pBuf) {
		free(pBuf);
	}
	if (btmp) {
		BIO_free(btmp);
	}
	if (bio) {
		BIO_free(bio);
	}
	return nRet;
}

int
rsa_load_pem_file(char *pszFileName, int bPublicKey,
				  char *pszPassword, RSA ** ppRsa, char *pszErr,
				  size_t err_bufsz)
{
	RSA *pRsa = NULL;
	EVP_PKEY *pkey = NULL;
	BIO *bio = NULL;
	int ret = -1;

	if (NULL == pszErr || NULL == pszFileName) {
		return -1;
	}

	/* prepare bio */
	if (NULL == (bio = BIO_new(BIO_s_file()))) {
		snprintf(pszErr, err_bufsz, "WARNING: out of memory");
		goto err_out;
	}
	if (BIO_read_filename(bio, pszFileName) <= 0) {
		snprintf(pszErr, err_bufsz, "Open file for read failed: %s",
				strerror(errno));
		goto err_out;
	}

	/* load as private key */
	pkey = PEM_read_bio_PrivateKey(bio, NULL, rsa_password_callback, pszPassword);
	if (NULL == pkey && bPublicKey) {
	    /* load as public key */
	    BIO_seek(bio, 0);
        pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
	}

	if (NULL == pkey) {
		if (PEM_R_BAD_PASSWORD_READ == ERR_GET_REASON(ERR_peek_last_error())) {
			snprintf(pszErr, err_bufsz, "Please specify password");
		} else if (PEM_R_BAD_DECRYPT == ERR_GET_REASON(ERR_peek_last_error())) {
			snprintf(pszErr, err_bufsz, "Invalid password");
		} else {
			snprintf(pszErr, err_bufsz, "Invalid key file");
		}
		goto err_out;
	}

	if ((pRsa = EVP_PKEY_get1_RSA(pkey)) == NULL) {
		snprintf(pszErr, err_bufsz, "Extract RSA key failed");
		goto err_out;
	}

    if (ppRsa) {
        *ppRsa = pRsa;
	    pRsa = NULL;
	}

	ret = 0;

  err_out:
	if (NULL != pRsa) {
		RSA_free(pRsa);
	}
	if (NULL != bio) {
		BIO_free(bio);
	}
	if (NULL != pkey) {
		EVP_PKEY_free(pkey);
	}
	return ret;
}


