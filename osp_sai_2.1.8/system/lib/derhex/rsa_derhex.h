/****************************************************************************
* $Id$
*  macros and defines for Genreate / convert RSA der hex
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-04-12 17:16
* Reason        : First Create.
****************************************************************************/

#ifndef _RSA_DERHEX_H_
#define _RSA_DERHEX_H_

#include <openssl/rsa.h>

#ifndef SSH_KEY_BITS_RESERVED
#define SSH_KEY_BITS_RESERVED		128	/* from ssh.h */
#endif

/* default password for encrypt/decrypt */
#define M_DEFAULT_PW_STR    "centecnetworks.com"
/* add by wus for bug52967 modify password "admin" in 2019-07-15*/
//#define M_DEFAULT_PW_STR    "suzhoucentecnetworks.com"

/* INIT VEECTOR for encrypt/decrypt */
#define INIT_VECTOR         "c1entec2"
#define M_ALG               EVP_des_ede3_cbc()

/* buffer size to hold length */
#define M_RSA_ASN1_LEN_BUFSZ    64

/* bio line buffer */
#define M_RSA_BIO_LINE_BUFSZ    512

/* key string buffer size */
#define M_RSA_KEYSTR_BUFSZ      16384

#ifndef M_RSA_MIN_PASSWORD_LEN
#define M_RSA_MIN_PASSWORD_LEN   6
#endif

#define M_RSA_IMPORT_KEY_MIN_MODULUS    768
#define M_RSA_IMPORT_KEY_MAX_MODULUS    4096

int rsa_init_crypto();

int rsa_asn1_length2str(int length, unsigned char *pOut, int nOutLen);
int rsa_print_bignumber(BIO * bio, BIGNUM * pnum,
						unsigned char *buf, int off);
int rsa_convert_key_der2str(RSA * prsa, u_int8_t bPubOnly,
							char *pBuf, u_int16_t nBufLen, int *pnRetLen);
int rsa_convert_str2der(char *pszKey, u_int16_t nKeyLen, char *pBuf,
						u_int16_t nBufLen, int *pnLen);

/* encrypt passwd */
int gen_passwd_crypt(char *pszPin, char *pszDestBuf, size_t bufsz);

/* convert password to nss cryot format */
int convert_passwd_for_nss(char *ibuf, size_t ilen, char *obuf, size_t olen);

/* validate passwod: old password is hex mode */
int gen_passwd_validate(char *pszOldPasswd, char *pszInput);

int do_encrypt(char *pw, char *ibuf, size_t ilen, char *obuf, size_t olen);

int do_decrypt(char *pw, char *ibuf, size_t ilen, char *obuf, size_t olen);

int rsa_key_bits_check(RSA *pRsa, char *pszErr, size_t err_bufsz);

int
validate_rsa_key_pem(const char *pszPasswd, char *pszKey, u_int16_t nKeyLen,
					 int bPrivateKey, RSA ** pprsa, char *pszErr,
					 size_t err_bufsz);
int
validate_rsa_key_der(char *pszKey, u_int16_t nKeyLen,
					 int bPrivateKey, RSA ** pprsa, char *pszErr,
					 size_t err_bufsz);

int rsa_pub_convert_format(RSA * pRsa, char *pszType,
						   char *pszOut, size_t out_bufsz, char *pszErr,
						   size_t err_bufsz, size_t * pOutBytes);
int rsa_pri_convert_format(RSA * pRsa, char *pszType, char *pszEnc,
						   char *pszPassword, char *pszOut, size_t out_bufsz,
						   char *pszErr, size_t err_bufsz,
						   size_t * pOutBytes);

int
export_rsa_to_file(RSA * pRsa, int bPrivate, char *pszEnc, char *pszPassword,
				   char *pszDestType, char *pszFilename, char *pszErr,
				   size_t err_bufsz);


int
rsa_load_der_file(char *pszFileName, int bPublicKey,
				  RSA ** ppRsa, char *pszErr, size_t err_bufsz);
int
rsa_load_derhex_file(char *pszFileName, int bPublicKey,
					 RSA ** ppRsa, char *pszErr, size_t err_bufsz);
int
rsa_load_pem_file(char *pszFileName, int bPublicKey,
				  char *pszPassword, RSA ** ppRsa, char *pszErr,
				  size_t err_bufsz);

#endif
