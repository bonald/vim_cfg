
#include <limits.h>
#include <errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "radius.h"
#include <openssl/md5.h>

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
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "dot1x_api.h"
#include "dot1x.h"
#include "authd.h"
#include "authd_debug.h"
#include "sal_base64.h"

DEBUG_DECLAR(authd_debug);

#define MAXPASS 128

void
authd_delete_account_stop_session(tbl_auth_session_t  *p_auth_session);

/*
 * Do XOR of two buffers.
 */
unsigned char * xor(unsigned char *p, unsigned char *q, int length)
{
	int i;
	unsigned char *retval = p;

	for (i = 0; i < length; i++) {
		*(p++) ^= *(q++);
	}
	return retval;
}

/**************************************************************************
 * MID-LEVEL RADIUS CODE
 **************************************************************************/

/*
 * get a pseudo-random vector.
 */
void get_random_vector(unsigned char *vector)
{
#ifdef linux
	int fd = open("/dev/urandom",O_RDONLY); /* Linux: get *real* random numbers */
	int total = 0;
	if (fd >= 0) {
		while (total < AUTH_VECTOR_LEN) {
			int bytes = read(fd, vector + total, AUTH_VECTOR_LEN - total);
			if (bytes <= 0)
				break;			/* oops! Error */
			total += bytes;
		}
		close(fd);
	}

	if (total != AUTH_VECTOR_LEN)
#endif
	{				/* do this *always* on other platforms */
		MD5_CTX my_md5;
		struct timeval tv;
		struct timezone tz;
		static unsigned int session = 0; /* make the number harder to guess */

		/* Use the time of day with the best resolution the system can
	 	   give us -- often close to microsecond accuracy. */
		gettimeofday(&tv,&tz);

		if (session == 0) {
			session = getppid();	/* (possibly) hard to guess information */
		}

		tv.tv_sec ^= getpid() * session++;

		/* Hash things to get maybe cryptographically strong pseudo-random numbers */
		MD5_Init(&my_md5);
		MD5_Update(&my_md5, (unsigned char *) &tv, sizeof(tv));
		MD5_Update(&my_md5, (unsigned char *) &tz, sizeof(tz));
		MD5_Final(vector, &my_md5);				/* set the final vector */
	}
}

/*
 * RFC 2139 says to do generate the accounting request vector this way.
 * However, the Livingston 1.16 server doesn't check it.	The Cistron
 * server (http://home.cistron.nl/~miquels/radius/) does, and this code
 * seems to work with it.	It also works with Funk's Steel-Belted RADIUS.
 */
void get_accounting_vector(AUTH_HDR *request, char *secret)
{
	MD5_CTX my_md5;
	int secretlen = strlen(secret);
	int len = ntohs(request->length);

	memset(request->vector, 0, AUTH_VECTOR_LEN);
	MD5_Init(&my_md5);
	memcpy(((char *)request) + len, secret, secretlen);

	MD5_Update(&my_md5, (unsigned char *)request, len + secretlen);
	MD5_Final(request->vector, &my_md5);			/* set the final vector */
}

/*
 * Verify the response from the server
 */
int verify_packet(char *secret, AUTH_HDR *response, AUTH_HDR *request)
{
	MD5_CTX my_md5;
	unsigned char	calculated[AUTH_VECTOR_LEN];
	unsigned char	reply[AUTH_VECTOR_LEN];

	/*
	 * We could dispense with the memcpy, and do MD5's of the packet
	 * + vector piece by piece.	This is easier understand, and maybe faster.
	 */
	memcpy(reply, response->vector, AUTH_VECTOR_LEN); /* save the reply */
	memcpy(response->vector, request->vector, AUTH_VECTOR_LEN); /* sent vector */

	/* MD5(response packet header + vector + response packet data + secret) */
	MD5_Init(&my_md5);
	MD5_Update(&my_md5, (unsigned char *) response, ntohs(response->length));

	/*
	 * This next bit is necessary because of a bug in the original Livingston
	 * RADIUS server.	The authentication vector is *supposed* to be MD5'd
	 * with the old password (as the secret) for password changes.
	 * However, the old password isn't used.	The "authentication" vector
	 * for the server reply packet is simply the MD5 of the reply packet.
	 * Odd, the code is 99% there, but the old password is never copied
	 * to the secret!
	 */
	if (*secret) {
		MD5_Update(&my_md5, (unsigned char *) secret, strlen(secret));
	}

	MD5_Final(calculated, &my_md5);			/* set the final vector */

	/* Did he use the same random vector + shared secret? */
	if (memcmp(calculated, reply, AUTH_VECTOR_LEN) != 0) {
		return -1;
	}
	return 0;
}

/*
 * Find an attribute in a RADIUS packet.	Note that the packet length
 * is *always* kept in network byte order.
 */
attribute_t *find_attribute(AUTH_HDR *response, unsigned char type)
{
	attribute_t *attr = (attribute_t *) &response->data;

	int len = ntohs(response->length) - AUTH_HDR_LEN;

	while (attr->attribute != type) {
		if ((len -= attr->length) <= 0) {
			return NULL;		/* not found */
		}
		attr = (attribute_t *) ((char *) attr + attr->length);
	}

	return attr;
}

/*
 * Add an attribute to a RADIUS packet.
 */
void add_attribute(AUTH_HDR *request, unsigned char type, unsigned char *data, int length)
{
	attribute_t *p;

	p = (attribute_t *) ((unsigned char *)request + ntohs(request->length));
	p->attribute = type;
	p->length = length + 2;		/* the total size of the attribute */
	request->length = htons(ntohs(request->length) + p->length);
	memcpy(p->data, data, length);
}

/*
 * Add an integer attribute to a RADIUS packet.
 */
void add_int_attribute(AUTH_HDR *request, unsigned char type, int data)
{
	int value = htonl(data);

	add_attribute(request, type, (unsigned char *) &value, sizeof(int));
}

/*
 * Add a RADIUS password attribute to the packet.	Some magic is done here.
 *
 * If it's an PW_OLD_PASSWORD attribute, it's encrypted using the encrypted
 * PW_PASSWORD attribute as the initialization vector.
 *
 * If the password attribute already exists, it's over-written.	This allows
 * us to simply call add_password to update the password for different
 * servers.
 */
void add_password(AUTH_HDR *request, unsigned char type, char *password, char *secret)
{
	MD5_CTX md5_secret, my_md5;
	unsigned char misc[AUTH_VECTOR_LEN];
	int i;
	int length = strlen(password);
	unsigned char hashed[256 + AUTH_PASS_LEN];	/* can't be longer than this */
	unsigned char *vector;
	attribute_t *attr;

	if (length > MAXPASS) {				/* shorten the password for now */
		length = MAXPASS;
	}

	if (length == 0) {
		length = AUTH_PASS_LEN;			/* 0 maps to 16 */
	} if ((length & (AUTH_PASS_LEN - 1)) != 0) {
		length += (AUTH_PASS_LEN - 1);		/* round it up */
		length &= ~(AUTH_PASS_LEN - 1);		/* chop it off */
	}						/* 16*N maps to itself */

	memset(hashed, 0, length);
	memcpy(hashed, password, strlen(password));

	attr = find_attribute(request, PW_PASSWORD);

	if (type == PW_PASSWORD) {
		vector = request->vector;
	} else {
		vector = attr->data;			/* attr CANNOT be NULL here. */
	}

	/* ************************************************************ */
	/* encrypt the password */
	/* password : e[0] = p[0] ^ MD5(secret + vector) */
	MD5_Init(&md5_secret);
      MD5_Update(&md5_secret, (unsigned char *) secret, strlen(secret));
	my_md5 = md5_secret;				/* so we won't re-do the hash later */
	MD5_Update(&my_md5, vector, AUTH_VECTOR_LEN);
	MD5_Final(misc, &my_md5);			/* set the final vector */
	xor(hashed, misc, AUTH_PASS_LEN);

	/* For each step through, e[i] = p[i] ^ MD5(secret + e[i-1]) */
	for (i = 1; i < (length >> 4); i++) {
		my_md5 = md5_secret;			/* grab old value of the hash */
		MD5_Update(&my_md5, &hashed[(i-1) * AUTH_PASS_LEN], AUTH_PASS_LEN);
		MD5_Final(misc, &my_md5);			/* set the final vector */
		xor(&hashed[i * AUTH_PASS_LEN], misc, AUTH_PASS_LEN);
	}

	if (type == PW_OLD_PASSWORD) {
		attr = find_attribute(request, PW_OLD_PASSWORD);
	}

	if (!attr) {
		add_attribute(request, type, hashed, length);
	} else {
		memcpy(attr->data, hashed, length); /* overwrite the packet */
	}
}

/*
 * Helper function for building a radius packet.
 * It initializes *some* of the header, and adds common attributes.
 */
void build_radius_packet(radius_handle_t *rh, AUTH_HDR *request, char *user, char *password)
{
	char hostname[256];
	uint32_t ipaddr;

	hostname[0] = '\0';
	gethostname(hostname, sizeof(hostname) - 1);

	request->length = htons(AUTH_HDR_LEN);

	if (password) {		/* make a random authentication req vector */
		get_random_vector(request->vector);
	}

	add_attribute(request, PW_USER_NAME, (unsigned char *) user, strlen(user));

	/*
	 *	Add a password, if given.
	 */
	if (password) {
		add_password(request, PW_PASSWORD, password, rh->secret);

		/*
		 *	Add a NULL password to non-accounting requests.
		 */
	} else if (request->code != PW_ACCOUNTING_REQUEST) {
		add_password(request, PW_PASSWORD, "", rh->secret);
	}

	/* the packet is from localhost if on localhost, to make configs easier */
	if ((rh->ip.s_addr == ntohl(0x7f000001)) || (!hostname[0])) {
		ipaddr = 0x7f000001;
	} else {
		struct hostent *hp;

		if ((hp = gethostbyname(hostname)) == (struct hostent *) NULL) {
			ipaddr = 0x00000000;	/* no client IP address */
		} else {
			ipaddr = ntohl(*(uint32_t *) hp->h_addr); /* use the first one available */
		}
	}

	/* If we can't find an IP address, then don't add one */
	if (ipaddr) {
		add_int_attribute(request, PW_NAS_IP_ADDRESS, ipaddr);
	}

    /* modified by liwh for bug 38745, 2016-05-27 */
#if 0
	/* There's always a NAS identifier */
	if (rh->pClientId && *rh->pClientId) {
		add_attribute(request, PW_NAS_IDENTIFIER, 
		    (unsigned char *) rh->pClientId, strlen(rh->pClientId));
	}
#endif    
    add_attribute(request, PW_NAS_IDENTIFIER, (unsigned char *) &hostname, sal_strlen(hostname));
    /* liwh end */
    
	/*
	 *	Add in the port (pid) and port type (virtual).
	 *
	 *	We might want to give the TTY name here, too.
	 */
	add_int_attribute(request, PW_NAS_PORT_ID, getpid());
	add_int_attribute(request, PW_NAS_PORT_TYPE, PW_NAS_PORT_TYPE_VIRTUAL);
}

/*
 * Talk RADIUS to a server.
 * Send a packet and get the response
 */
int talk_radius(AUTH_HDR *request, AUTH_HDR *response,
		       char *password, char *old_password, int tries)
{
#if 0
	socklen_t salen;
	int total_length;
	fd_set set;
	struct timeval tv;
	time_t now, end;
	int rcode;
	struct sockaddr saremote;
	struct sockaddr_in *s_in = (struct sockaddr_in *) &saremote;
	radius_server_t *server = conf->server;
	int ok;
	int server_tries;
	int retval;

	/* ************************************************************ */
	/* Now that we're done building the request, we can send it */

	/*
	 Hmm... on password change requests, all of the found server information
	 could be saved with a pam_set_data(), which means even the radius_conf_t
	 information will have to be malloc'd at some point

	 On the other hand, we could just try all of the servers again in
	 sequence, on the off chance that one may have ended up fixing itself.

	 */

	/* loop over all available servers */
	while (server != NULL) {
		/* clear the response */
		memset(response, 0, sizeof(AUTH_HDR));

		/* only look up IP information as necessary */
		if ((retval = host2server(server)) != PAM_SUCCESS) {
			_pam_log(LOG_ERR,
				 "Failed looking up IP address for RADIUS server %s (errcode=%d)",
				 server->hostname, retval);
			ok = -1;
			goto next;		/* skip to the next server */
		}

		/* set up per-server IP && port configuration */
		memset ((char *) s_in, '\0', sizeof(struct sockaddr));
		s_in->sin_family = AF_INET;
		s_in->sin_addr.s_addr = htonl(server->ip.s_addr);
		s_in->sin_port = server->port;
		total_length = ntohs(request->length);

		if (!password) { 		/* make an RFC 2139 p6 request authenticator */
			get_accounting_vector(request, server);
		}

		server_tries = tries;
	send:
		/* send the packet */
		if (sendto(conf->sockfd, (char *) request, total_length, 0,
			   &saremote, sizeof(struct sockaddr_in)) < 0) {
			_pam_log(LOG_ERR, "Error sending RADIUS packet to server %s: %s",
				 server->hostname, strerror(errno));
			ok = -1;
			goto next;		/* skip to the next server */
		}

		/* ************************************************************ */
		/* Wait for the response, and verify it. */
		salen = sizeof(struct sockaddr);
		tv.tv_sec = server->timeout;	/* wait for the specified time */
		tv.tv_usec = 0;
		FD_ZERO(&set);			/* clear out the set */
		FD_SET(conf->sockfd, &set);	/* wait only for the RADIUS UDP socket */

		time(&now);
		end = now + tv.tv_sec;

		/* loop, waiting for the select to return data */
		ok = 0;
		while (ok) {
			rcode = select(conf->sockfd + 1, &set, NULL, NULL, &tv);

			/* select timed out */
			if (rcode == 0) {
				_pam_log(LOG_ERR, "RADIUS server %s failed to respond", server->hostname);
				if (--server_tries) {
					goto send;
				}
				ok = -1;
				break;			/* exit from the select loop */
			} else if (rcode < 0) {

				/* select had an error */
				if (errno == EINTR) {	/* we were interrupted */
					time(&now);

					if (now > end) {
						_pam_log(LOG_ERR, "RADIUS server %s failed to respond",
							 server->hostname);
						if (--server_tries) goto send;
						ok = -1;
						break;		/* exit from the select loop */
					}

					tv.tv_sec = end - now;
					if (tv.tv_sec == 0) {	/* keep waiting */
						tv.tv_sec = 1;
					}

				} else {			/* not an interrupt, it was a real error */
					_pam_log(LOG_ERR, "Error waiting for response from RADIUS server %s: %s",
						 server->hostname, strerror(errno));
					ok = -1;
					break;
				}

			/* the select returned OK */
			} else if (FD_ISSET(conf->sockfd, &set)) {

				/* try to receive some data */
				if ((total_length = recvfrom(conf->sockfd, (void *) response, BUFFER_SIZE,
						     	     0, &saremote, &salen)) < 0) {
					_pam_log(LOG_ERR, "error reading RADIUS packet from server %s: %s",
					 	 server->hostname, strerror(errno));
					ok = -1;
					break;

				/* there's data, see if it's valid */
				} else {
					char *p = server->secret;

					if ((ntohs(response->length) != total_length) ||
					    (ntohs(response->length) > BUFFER_SIZE)) {
						_pam_log(LOG_ERR, "RADIUS packet from server %s is corrupted",
						 	 server->hostname);
						ok = -1;
						break;
					}

					/* Check if we have the data OK. We should also check request->id */
					if (password) {
						if (old_password) {
#ifdef LIVINGSTON_PASSWORD_VERIFY_BUG_FIXED
							p = old_password;	/* what it should be */
#else
							p = "";			/* what it really is */
#endif
						}
					/*
					 * RFC 2139 p.6 says not do do this, but the Livingston 1.16
					 * server disagrees.	If the user says he wants the bug, give in.
					 */
					} else {		/* authentication request */
						if (conf->accounting_bug) {
							p = "";
						}
					}

					if (!verify_packet(p, response, request)) {
						_pam_log(LOG_ERR, "packet from RADIUS server %s failed verification: "
							 "The shared secret is probably incorrect.", server->hostname);
						ok = -1;
						break;
					}

					/*
					 * Check that the response ID matches the request ID.
					 */
					if (response->id != request->id) {
						_pam_log(LOG_WARNING, "Response packet ID %d does not match the "
							 "request packet ID %d: verification of packet fails",
							 response->id, request->id);
						ok = -1;
						break;
					}
				}

				/*
				 * Whew! The select is done. It hasn't timed out, or errored out.
				 * It's our descriptor.	We've got some data. It's the right size.
				 * The packet is valid.
				 * NOW, we can skip out of the select loop, and process the packet
				 */
				break;
			}
			/* otherwise, we've got data on another descriptor, keep select'ing */
		}

			/* go to the next server if this one didn't respond */
		next:
		if (!ok) {
			radius_server_t *old;	/* forget about this server */

			old = server;
			server = server->next;
			conf->server = server;

			_pam_forget(old->secret);
			free(old->hostname);
			free(old);

			if (server) {		/* if there's more servers to check */
				/* get a new authentication vector, and update the passwords */
				get_random_vector(request->vector);
				request->id = request->vector[0];

				/* update passwords, as appropriate */
				if (password) {
					get_random_vector(request->vector);
					if (old_password) {	/* password change request */
						add_password(request, PW_PASSWORD, password, old_password);
						add_password(request, PW_OLD_PASSWORD, old_password, old_password);
					} else {		/* authentication request */
						add_password(request, PW_PASSWORD, password, server->secret);
					}
				}
			}
			continue;

		} else {
			/* we've found one that does respond, forget about the other servers */
			server->next = NULL;
			break;
		}
	}

	if (!server) {
		_pam_log(LOG_ERR, "All RADIUS servers failed to respond.");
		if (conf->localifdown)
			retval = -1;
		else
			retval = -2;
	} else {
		retval = 0;
	}

	return retval;
#else
    return -1;
#endif
}

extern int auth_get_shared_secret(tbl_auth_server_t *pSrv, char *pszOut, size_t nBufLen);
extern int auth_get_timeout(tbl_auth_server_t * pSrv);

int radius_op_init(auth_handle_t *h)
{
    radius_handle_t *pObj = NULL;

    pObj = calloc(1, sizeof(radius_handle_t));
    if (NULL == pObj) {
        return -1;
    }

    AUTHD_EVENT_DEBUG(
            "initialize radius backend handle %p", pObj);

    pObj->pSendBuf = malloc(AUTH_PKT_BUF_SZ);
    if (NULL == pObj->pSendBuf) {
        if (NULL != pObj->pSendBuf) {
            free(pObj->pSendBuf);
        }
        free(pObj);
        return -1;
    }
    
    h->pObj = (void *)pObj;
    return 0;
}

void radius_op_release(auth_handle_t *h)
{
    radius_handle_t *pObj = (radius_handle_t *)h->pObj;

    AUTHD_EVENT_DEBUG(
            "release radius backend handle %p", pObj);

    if (NULL != pObj) {
        if (NULL != pObj->pSendBuf) {
            free(pObj->pSendBuf);
        }
        free(pObj);
        h->pObj = NULL;
    }

    return;
}

int radius_op_connect(auth_handle_t *h, tbl_auth_server_t *pSrv)
{
    struct sockaddr_in dest;
    struct sockaddr_in6 dest6;
    struct sockaddr *psa;
    socklen_t salen;
    int flags;
    int fd = -1;

    AUTHD_EVENT_DEBUG(
            "radius op connect %p", h->pObj);

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
        dest.sin_port = 0;
        dest.sin_addr.s_addr = INADDR_ANY;
    } else {
        psa = (struct sockaddr *)&dest6;
        salen = sizeof(struct sockaddr_in6);
        memset(psa, 0x00, salen);
        dest6.sin6_family = AF_INET6;
        dest6.sin6_port = 0;
        dest6.sin6_addr = in6addr_any;
    }

    if ((fd = socket(pSrv->key.addr.family, SOCK_DGRAM, 0)) == -1) {
        goto err_out;
    }
    
    if (bind(fd, psa, salen) != 0) {
        goto err_out;
    }

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1 ||
            fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        goto err_out;
    }

    pSrv->fd = fd;
    pSrv->connected = 1;
    return M_AUTH_CONNECT_SUCCESS;

err_out:
    if (-1 != fd) {
        close(fd);
    }
    return M_AUTH_CONNECT_FAILED;
}


static int radius_send_pkt(int fd, tbl_auth_server_t *pSrv, AUTH_HDR *request)
{
    struct sockaddr_in dest;
    struct sockaddr_in6 dest6;
    struct sockaddr *psa;
    socklen_t salen;
    int total_length = ntohs(request->length);

    AUTHD_EVENT_DEBUG(
            "radius op send pkt");

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

resend:
    if (sendto(fd, (char *) request, total_length, 0, psa, salen) < 0) {
        if (EINTR == errno || EAGAIN == errno) {
            goto resend;
        }
        return -1;
    }

    return 0;
}

void
auth_hmac_md5 (unsigned char *text, int text_len,
    unsigned char *key, int key_len, unsigned char *digest)
{
    MD5_CTX context;
    u_char k_ipad[65];    /* inner padding - key XORd with ipad.  */
    u_char k_opad[65];    /* outer padding - key XORd with opad.  */
    u_char tk[16];
    int i;
  
    /* if key is longer than 64 bytes reset it to key=MD5(key).  */
    if (key_len > 64)
    {
        MD5_CTX      tctx;
  
        MD5_Init (&tctx);
        MD5_Update (&tctx, key, key_len);
        MD5_Final((u_char *)tk, &tctx);
  
        key = (unsigned char *)tk;
        key_len = 16;
    }
  
    /* The HMAC_MD5 transform looks like:
  
       MD5(K XOR opad, MD5(K XOR ipad, text))
  
       where K is an n byte key ipad is the byte 0x36 repeated 64 times
  
       opad is the byte 0x5c repeated 64 times
       and text is the data being protected.  */
  
    /* Start out by storing key in pads.  */
    sal_memset (k_ipad, 0, sizeof (k_ipad));
    sal_memset (k_opad, 0, sizeof (k_opad));
    sal_memcpy (k_ipad, key, key_len);
    sal_memcpy (k_opad, key, key_len);
  
    /* XOR key with ipad and opad values.  */
    for (i = 0; i < 64; i++)
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }
    
    /* Perform inner MD5.  */
    MD5_Init (&context);			/* Init context for 1st pass */
    MD5_Update (&context, k_ipad, 64);	/* Start with inner pad.  */
    MD5_Update (&context, text, text_len);	/* Then text of datagram.  */
    MD5_Final (digest, &context);		/* Finish up 1st pass.  */
    
    /* Perform outer MD5.  */
    MD5_Init (&context);			/* Init context for 2nd pass */
    MD5_Update (&context, k_opad, 64);	/* Start with outer pad. */
    MD5_Update (&context, digest, 16);	/* Then results of 1st hash. */
    MD5_Final (digest, &context);		/* Finish up 2nd pass.  */
}

void build_dot1x_radius_packet(radius_handle_t *rh, AUTH_HDR *request, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{  
    uint32 mtu = sal_hton32(RADIUS_FRAMED_MTU_DEFAULT);
    char   authenticator[RADIUS_AUTHENTICATOR_LEN];
    uint32              index = 0;
    unsigned char *buf = NULL;
    uint16 len = 0;
    uint16 total_len = 0;   
    char hostname[256];
     
    hostname[0] = '\0';
    gethostname(hostname, sizeof(hostname) - 1);

    sal_memset(authenticator, 0, RADIUS_AUTHENTICATOR_LEN);
    
    request->length = htons(AUTH_HDR_LEN);
    
    get_random_vector(request->vector);

    sal_memcpy(pSess->oauthenticator, request->vector, RADIUS_AUTHENTICATOR_LEN);
    
    add_attribute(request, PW_USER_NAME, (unsigned char *) pSess->user.name, sal_strlen(pSess->user.name));

    add_attribute(request, PW_FRAMED_MTU, (unsigned char *) &mtu, sizeof(mtu));

    add_attribute(request, PW_NAS_IDENTIFIER, (unsigned char *) &hostname, sal_strlen(hostname));

    index = sal_ntoh32(pSess->key.index);
    add_attribute(request, PW_NAS_PORT_ID, (unsigned char *) &index, sizeof(uint32));

    if (pSess->radius_state_len)
    {
        add_attribute(request, PW_STATE, (unsigned char *)pSess->radius_state_buf, pSess->radius_state_len);
    }

    /* Handle EAP fragmentation.  */
    while (total_len < pSess->eap_length)
    {
        len = pSess->eap_length - total_len;

        if (len > RADIUS_ATTR_MAX_LEN)
        {
            len = RADIUS_ATTR_MAX_LEN;
        }

        add_attribute(request, PW_EAP_MESSAGE, (unsigned char *)(pSess->eap_buf + total_len), len); 
        total_len = total_len + len;     
    }
  
    add_attribute(request, PW_MSG_AUTHENTICATOR, (unsigned char *)&authenticator, 
        RADIUS_AUTHENTICATOR_LEN);

    buf = (unsigned char *)request;
    buf = buf + (sal_ntoh16(request->length) - RADIUS_AUTHENTICATOR_LEN);
    auth_hmac_md5 ((unsigned char *)request, sal_ntoh16(request->length),
        (unsigned char *)rh->secret, sal_strlen(rh->secret), buf);

    return;
}

void build_dot1x_mac_radius_packet(radius_handle_t *rh, AUTH_HDR *request, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{  
    char      authenticator[RADIUS_AUTHENTICATOR_LEN];
    char      hostname[256];
     
    hostname[0] = '\0';
    gethostname(hostname, sizeof(hostname) - 1);

    sal_memset(authenticator, 0, RADIUS_AUTHENTICATOR_LEN);
    
    request->length = htons(AUTH_HDR_LEN);
    
    get_random_vector(request->vector);

    sal_memcpy(pSess->oauthenticator, request->vector, RADIUS_AUTHENTICATOR_LEN);
    
    add_attribute(request, PW_USER_NAME, (unsigned char *) pSess->user.name, sal_strlen(pSess->user.name));

    add_attribute(request, PW_NAS_IDENTIFIER, (unsigned char *) &hostname, sal_strlen(hostname));

    add_password(request, PW_PASSWORD, pSess->user.name, rh->secret);

    return;
}

int radius_op_send_account_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    radius_handle_t   *pObj = (radius_handle_t *)h->pObj;
    AUTH_HDR          *request = (AUTH_HDR *) pObj->pSendBuf;
    uint16            port = 0;
    int               ret = -1; 
    char              hostname[256];
    uint32            value = 0;
    char              tmp_value[256];
    tbl_vty_t         *p_vty = NULL;
    tbl_vty_t         vty;
    struct timeval    now;
 
    sal_memset(tmp_value, 0, 256);

    if (NULL == pObj || !pSrv->connected) {
        return M_AUTH_SEND_FAILED;
    }

    pObj->secret[0] = '\0';
    auth_get_shared_secret(pSrv, pObj->secret, RADIUS_SECRET_BUFSZ);
    
    /* XXX: ipv4 only */
    if (AF_INET == pSrv->key.addr.family) {
        memcpy(&pObj->ip.s_addr, &pSrv->key.addr.u.prefix4, sizeof(struct in_addr));
    } else if (AF_INET6 == pSrv->key.addr.family) {
        memcpy(&pObj->ipv6, &pSrv->key.addr.u.prefix6, sizeof(struct in6_addr));
    } else {
        return M_AUTH_SEND_FAILED;
    }

    /* build RADIUS account packet */
    request->code = PW_ACCOUNTING_REQUEST;

    request->length = htons(AUTH_HDR_LEN);
    
    sal_memset(request->vector, 0, AUTH_VECTOR_LEN);
    request->id = dot1x_allocate_session_id(); /* this should be evenly distributed */

    if (pSess->acct_start)
    {
        value = PW_STATUS_START;
    }
    else
    {
        value = PW_STATUS_STOP;
    }
    value = htonl(value);
    add_attribute(request, PW_ACCT_STATUS_TYPE, (unsigned char *) &value, sizeof(value));

    sprintf(tmp_value, "%d", pSess->key.index);
    add_attribute(request, PW_ACCT_SESSION_ID, (unsigned char *) &tmp_value, sal_strlen(tmp_value));

    /* modified by liwh for bug 50704, 2019-01-08 */
    if (sal_strcmp(pSess->user.name, "null"))
    /* liwh end */
    {
        add_attribute(request, PW_USER_NAME, (unsigned char *) pSess->user.name, strlen(pSess->user.name));
    }
    
    if (AUTH_SERVER_TYPE_RADIUS == pSess->key.type)
    {
        value = PW_AUTH_RADIUS;
        value = htonl(value);
        add_attribute(request, PW_ACCT_AUTHENTIC, (unsigned char *) &value, sizeof(value));
    }

    if (pSess->acct_stop)
    {
        value = 0;

        sal_memset(&vty, 0, sizeof(tbl_vty_t));
        vty.key.id = pSess->key.index - 1;
        
        p_vty = tbl_vty_get_vty(&vty.key);
        if (p_vty)
        {
            gettimeofday (&now, (struct timezone *)0);
            value = now.tv_sec - p_vty->aaa_start_time;
            value = htonl(value);                   
        }

        add_attribute(request, PW_ACCT_SESSION_TIME, (unsigned char *) &value, sizeof(value));
    }

    hostname[0] = '\0';
    gethostname(hostname, sizeof(hostname) - 1);
    add_attribute(request, PW_NAS_IDENTIFIER, (unsigned char *) &hostname, sal_strlen(hostname));

    get_accounting_vector(request, pObj->secret);
        
    port = pSrv->port;
    pSrv->port = RADIUS_ACCT_PORT;
    ret = radius_send_pkt(pSrv->fd, pSrv, request);
    if (ret < 0) {
        /* auth failed */
        return M_AUTH_SEND_FAILED;
    }
    
    pSrv->port = port;
    pSess->uCurSessId = request->id;
    
    return M_AUTH_SEND_SUCCESS;  
}

int radius_op_send_request(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    radius_handle_t *pObj = (radius_handle_t *)h->pObj;
    AUTH_HDR *request = (AUTH_HDR *) pObj->pSendBuf;
    int ret = -1;

    if (NULL == pObj || !pSrv->connected) {
        return M_AUTH_SEND_FAILED;
    }

    if (pSess->acct_start || pSess->acct_stop) 
    {
        ret = radius_op_send_account_request(h, pSess, pSrv);
        return ret;    
    }

    pObj->secret[0] = '\0';
    auth_get_shared_secret(pSrv, pObj->secret, RADIUS_SECRET_BUFSZ);
    /* XXX: ipv4 only */
    if (AF_INET == pSrv->key.addr.family) {
        memcpy(&pObj->ip.s_addr, &pSrv->key.addr.u.prefix4, sizeof(struct in_addr));
    } else if (AF_INET6 == pSrv->key.addr.family) {
        memcpy(&pObj->ipv6, &pSrv->key.addr.u.prefix6, sizeof(struct in6_addr));
    } else {
        return M_AUTH_SEND_FAILED;
    }

    /* build and initialize the RADIUS packet */
    request->code = PW_AUTHENTICATION_REQUEST;

    if (!pSess->is_dot1x_session)
    {
        char szPasswd[PASSWD_SIZE];
        get_random_vector(request->vector);
        request->id = request->vector[0]; /* this should be evenly distributed */

        sal_b64_pton(pSess->passwd, (unsigned char *)szPasswd, PASSWD_SIZE);
        build_radius_packet(pObj, request, pSess->user.name, szPasswd); 
        
        /* not all servers understand this service type, but some do */
        add_int_attribute(request, PW_USER_SERVICE_TYPE, PW_AUTHENTICATE_ONLY);
    }
    else
    {
        if (!pSess->is_auth_bypass)
        {
            request->id = pSess->uCurSessId;
            build_dot1x_radius_packet(pObj, request, pSess, pSrv); 
        }
        else
        {
            request->id = pSess->uCurSessId;
            build_dot1x_mac_radius_packet(pObj, request, pSess, pSrv);     
        } 

        dot1x_radius_set_identifier(pSrv, pSess);
    }
   
    ret = radius_send_pkt(pSrv->fd, pSrv, request);
    if (ret < 0) {
        /* auth failed */
        return M_AUTH_SEND_FAILED;
    }

    if (!pSess->is_dot1x_session)
    {
        pSess->uCurSessId = (u_int32_t)request->id;
    }
    return M_AUTH_SEND_SUCCESS;
}

#if 0
#define CONSOLE_PRINT(fmt, args...)                      \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    fprintf(fp_console, fmt, ##args);   \
    fclose(fp_console);                                        \
}

void print_attributes(AUTH_HDR *response)
{
	attribute_t *attr = (attribute_t *) &response->data;

	int len = ntohs(response->length) - AUTH_HDR_LEN;

	while (len > 0) {
		if ((len -= attr->length) <= 0) {
			return;		/* not found */
		}
		attr = (attribute_t *) ((char *) attr + attr->length);
        CONSOLE_PRINT("print_attributes: %d %d\r\n",
            attr->attribute, attr->length);
	}

	return;
}
#endif

tbl_auth_session_t *auth_session_get_by_id(u_int32_t uSessId);
int radius_op_process_reply(tbl_auth_server_t *pSrv, AUTH_RESULT_CB pCB)
{
    tbl_auth_session_t *pSess = NULL;
    auth_handle_t *h = NULL;
    radius_handle_t *pObj = NULL;
    AUTH_HDR *response = (AUTH_HDR *)pSrv->buf;
    int ret;
    int pktlen = 0;
    int leftlen;
    int minlen = sizeof(AUTH_HDR) - 2;
    struct timeval now;

    ret = read(pSrv->fd, pSrv->buf + pSrv->blen, GLB_AUTH_MSG_BUF_SZ - pSrv->blen);
    if (ret <= 0) {
        /* socket closed */
        return M_AUTH_ERROR;
    }

    AUTHD_EVENT_DEBUG(
            "radius op process reply");

    pSrv->blen += ret;
    leftlen = pSrv->blen;
    
    if (leftlen < minlen) {
        return M_AUTH_INPROGRESS;
    }

    while (leftlen >= minlen) {
        pktlen = ntohs(response->length);
        if (pktlen >= AUTH_PKT_BUF_SZ) {
            return M_AUTH_ERROR;
        }

        if (leftlen < pktlen) { /* incomplete */
            break;
        }

        if ((pSess = auth_session_get_by_id((u_int32_t)response->id)) == NULL) {
            /* session deleted, drop this message */
            leftlen -= pktlen;
            response = (AUTH_HDR *)((char *)response + pktlen);
            continue;
        }
        
        h = (auth_handle_t *)pSess->pHandle;

        if (!h) {
            return M_AUTH_ERROR;    
        }
        pObj = (radius_handle_t *)h->pObj;
        if (NULL == pObj) {
            if (pCB) {
                pCB(pSess, M_AUTH_ERROR, "Internal error, radius handle NULL");
            }
            return M_AUTH_ERROR;
        }

        /* process packet */
        if (verify_packet(pObj->secret, response, (AUTH_HDR *)(pObj->pSendBuf)) != 0) {
            if (pCB) {
                pCB(pSess, M_AUTH_ERROR, "packet from RADIUS server failed verification");
            }    
        } else {
            if (PW_ACCESS_CHALLENGE == response->code) {

                if (pSess->is_dot1x_session)
                {
                    dot1x_proc_radius(pSrv, pSess);
                }
                
            } else if (PW_AUTHENTICATION_ACK == response->code) {

                if (pSess->is_dot1x_session)
                {
                    dot1x_proc_radius(pSrv, pSess);
                }
                else
                {                   
                    /* modified by liwh for bug 40534, 2016-09-08 */                    
                    /* find vty */
                    if (0 != pSess->key.index)
                    {
                        tbl_vty_t     *p_vty = NULL;
                        tbl_vty_t     vty;
                        tbl_author_t  author;
                        tbl_author_t  *p_author = NULL;
                        char          *ptr = NULL;
                        uint8         type = 0;
                        uint8         length = 0;
                        uint32        system_id = 0;
                        uint32        privilege = PRIVILEGE_INVALID;               
                        uint32        length_reserved = 0;
#ifdef _GLB_UML_SYSTEM_
                        uint32        privilege1 = PRIVILEGE_INVALID;
#endif
                        
                        sal_memset(&vty, 0, sizeof(tbl_vty_t));
                        vty.key.id = pSess->key.index - 1;
                        
                        p_vty = tbl_vty_get_vty(&vty.key);
                        if (p_vty && (p_vty->inuse || p_vty->is_ssh_login))/*modified by yejl to fix bug 46694, 2018-04-03*/
                        {
                            /* check aaa authorized is configured */
                            /* get and reserve aaa privilege and set vty privilege */
                            memset(&author, 0x00, sizeof(tbl_author_t));
                            sal_memcpy(&author.key.name, p_vty->author_method, LINE_AUTH_METHOD_NAME_SIZE);

                            p_author = tbl_author_get_author(&author.key);
                            if (p_author && (sal_strstr(p_author->methods, "radius")))/*added by yejl to fix bug41250, 2017-01-05*/
                            {
                                /* modified by liwh for bug 52019, 2019-06-15 */
                                char   *tmpFile  = "/tmp/oem_info_XXX";
                                FILE    *fp      = NULL;
                                static char    line[128] = {};
                                uint32  centec_system_id = GLB_SYSTEM_OID;

                                sprintf(line, " more /tmp/oem_info | grep snmp_enterprise_oid | awk '{print $2}' > %s", tmpFile);            
                                if(0 == system(line))
                                {
                                    bzero(line, sizeof(line));
                                     
                                    fp = fopen(tmpFile, "r");
                                    if (fp)
                                    {
                                        fgets(line, sizeof(line), fp);
                                        if(strlen(line) > 1)
                                        {
                                            if(line[strlen(line)-1] == '\n')
                                            {
                                                line[strlen(line)-1] = '\0';
                                            } 

                                            centec_system_id = atoi(line);
                                        }

                                        fclose(fp);
                                        remove(tmpFile);                    
                                    }
                                }
                                /* liwh end */
                                    
                                ptr = (char*)&response->data;
                                type = *ptr;
                                ptr++;
                                length_reserved = response->length - 4 - AUTH_VECTOR_LEN;
                                while (type && length_reserved)
                                {
                                    if (PW_VENDOR_SPECIFIC != type)
                                    {
                                        length = *ptr;
                                        ptr++;
                                        ptr = ptr + (length - 2);
                                        type = *ptr;
                                        ptr++;
                                        if (length_reserved > length)
                                        {
                                            length_reserved = length_reserved - length;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                        continue;
                                    }
                            
                                    length = *ptr;
                                    ptr++;
                                    sal_memcpy(&system_id, ptr, 4);
                                    ptr += 4;
                                    system_id = htonl(system_id);

#ifndef _GLB_UML_SYSTEM_
                                    if (centec_system_id == system_id)
                                    {
                                        type = *ptr;
                                        ptr++;
                                        length = *ptr;
                                        ptr++;
                                        sal_memcpy(&privilege, ptr, 4);
                                        ptr += 4;

                                        /* modified by liwh for bug 50157, 2019-01-16 */
                                        if (privilege >= PRIVILEGE_INVALID)
                                        {
                                            privilege = htonl(privilege);
                                        }
                                        /* liwh end */

                                        AUTHD_EVENT_DEBUG("radius_op_process_reply system_id %d privilege %d", 
                                            system_id, privilege);
                                    
                                        break;
                                    }
#else
                                    if (9 == system_id)
                                    {
                                        type = *ptr;
                                        ptr++;
                                        length = *ptr;
                                        ptr++;
                                        sal_memcpy(&privilege, ptr, 4);
                                        ptr += 4;
                            
                                        if (privilege >= PRIVILEGE_INVALID)
                                        {                                        
                                            sscanf((char*)&privilege, "%d", &privilege1);
                                            privilege = privilege1;
                                        }

                                        AUTHD_EVENT_DEBUG("radius_op_process_reply system_id is %d privilege %d", 
                                            system_id, privilege);
                                    
                                        break;
                                    }
#endif

                                    if (length_reserved > length)
                                    {
                                        length_reserved = length_reserved - length;
                                    }
                                    else
                                    {
                                        break;
                                    }

                                    ptr = ptr + (length - 6);
                                    type = *ptr;
                                    ptr++;
                                }   
                                if (privilege < PRIVILEGE_INVALID)
                                {
                                    /* reserve aaa privilege and set vty privilege */
                                    p_vty->aaa_privilege = privilege;
                                    tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_AAA_PRIVILEGE);
                                    
                                    p_vty->config_privilege = p_vty->privilege;  
                                    tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_CONFIG_PRIVILEGE);
                                    
                                    p_vty->privilege = aaa_map_privilege(privilege);
                                    tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_PRIVILEGE);

                                    p_vty->is_ssh_login = FALSE;
                                    tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_IS_SSH_LOGIN);
                                }
                                
                                gettimeofday (&now, (struct timezone *)0);
                                p_vty->aaa_start_time = now.tv_sec;
                                tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_AAA_START_TIME);

                            }
                        }
                    }
                    /* liwh end */
                    
                    if (pCB) {
                        pCB(pSess, M_AUTH_SUCCESS, "access permited");
                    } 
                }
            } else if (PW_AUTHENTICATION_REJECT == response->code) {
                if (pSess->is_dot1x_session)
                {
                    logid_sys(LOG_DOT1X_4_FAIL, pSess->user.name, "Denied");
                    dot1x_proc_radius(pSrv, pSess);
                }
                else
                {
                    if (pCB) {
                        pCB(pSess, M_AUTH_FAILED, "access denied");
                    }
                }
            } else if (PW_ACCOUNTING_RESPONSE == response->code) {

                if (pSess->acct_start || pSess->acct_stop)
                {
                    if (pCB) {
                        dot1x_free_session(pSess->uCurSessId);
                        pCB(pSess, M_AUTH_SUCCESS, "access permited");
                    }
                }
            } 
        }

        leftlen -= pktlen;
        response = (AUTH_HDR *)((char *)response + pktlen);
    }

    /* uncomplete message */
    if (leftlen > 0 && (pSrv->buf != (char *)response)) {
        memmove(pSrv->buf, response, leftlen);
        pSrv->blen = leftlen;
    } else {
        pSrv->blen = 0;
    }

    return M_AUTH_INPROGRESS;
}

void radius_op_get_error_msg(auth_handle_t *h, char *pszErrBuf, size_t nBufSz)
{
    snprintf(pszErrBuf, nBufSz, "unknown error");
    return;
}

auth_be_op_t radius_be_ops = {
    .init = radius_op_init,
    .release = radius_op_release,
    .do_connect = radius_op_connect,
    .send_request = radius_op_send_request,
    .process_reply = radius_op_process_reply,
    .get_error_msg = radius_op_get_error_msg,
};  


