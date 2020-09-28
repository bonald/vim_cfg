#ifndef __DOT1X_PACKET_H__
#define __DOT1X_PACKET_H__

/* 802.1X-2001 Section 7.5.4 Packet type.  */
#define AUTH_PACKET_EAP_PACKET      0
#define AUTH_PACKET_EAPOL_START     1
#define AUTH_PACKET_EAPOL_LOGOFF    2
#define AUTH_PACKET_EAPOL_KEY       3
#define AUTH_PACKET_EAPOL_ASF_ALERT 4
#define AUTH_PACKET_TYPE_MAX        5

/* 802.1X-2001 Section 7.7 EAP packet format (RFC 2284 2.2)  */
#define AUTH_EAP_HDR_LEN            4
#define AUTH_EAP_DATA_TYPE_LEN      1

/* 802.1X-2001 Section 7.7 EAP packet format (RFC 2284 2.2)  */
#define AUTH_EAP_CODE_REQUEST       1
#define AUTH_EAP_CODE_RESPONSE      2
#define AUTH_EAP_CODE_SUCCESS       3
#define AUTH_EAP_CODE_FAILURE       4
#define AUTH_EAP_CODE_MAX           5

/* RFC 2284 3 Initial EAP Request/Response Types.  */
#define AUTH_EAP_TYPE_IDENTITY      1
#define AUTH_EAP_TYPE_NOTIFICATION  2
#define AUTH_EAP_TYPE_NAK           3
#define AUTH_EAP_TYPE_MD5_CHALLENGE 4
#define AUTH_EAP_TYPE_OTP           5   /* One-Time Password (OTP) */
#define AUTH_EAP_TYPE_GTC           6   /* Generic Token Card */
#define AUTH_EAP_TYPE_TLS           13  /* Transport Level Security */
#define AUTH_EAP_TYPE_PEAP          25  /* Protected EAP */
#define AUTH_EAP_TYPE_MAX           26

#define RADIUS_MAX_PKT_LEN         4096
#define RADIUS_MIN_PKT_LEN         20

/* RFC 2865 4 Radius Packet Code.  */
#define RADIUS_ACCESS_REQUEST           1
#define RADIUS_ACCESS_ACCEPT            2
#define RADIUS_ACCESS_REJECT            3
#define RADIUS_ACCT_REQUEST             4
#define RADIUS_ACCT_RESPONSE            5
#define RADIUS_ACCESS_CHALLENGE         11
#define RADIUS_CODE_MAX                 12

/* RFC 2865 5 Radius Attributes.  */
#define RADIUS_USER_NAME                1
#define RADIUS_PASSWORD                 2
#define RADIUS_NAS_IP_ADDRESS           4
#define RADIUS_NAS_PORT                 5
#define RADIUS_SERVICE_TYPE             6
#define RADIUS_FRAMED_PROTOCOL          7
#define RADIUS_FRAMED_IP_ADDRESS        8
#define RADIUS_FRAMED_IP_NETMASK        9
#define RADIUS_FRAMED_ROUTING           10
#define RADIUS_FILTER_ID                11
#define RADIUS_FRAMED_MTU               12
#define RADIUS_FRAMED_COMPRESSION       13
#define RADIUS_LOGIN_IP_HOST            14
#define RADIUS_LOGIN_SERVICE            15
#define RADIUS_LOGIN_TCP_PORT           16
#define RADIUS_REPLY_MESSAGE            18
#define RADIUS_STATE                    24
#define RADIUS_SESSION_TIMEOUT          27
#define RADIUS_CALLED_STID              30
#define RADIUS_CALLING_STID             31
#define RADIUS_NAS_IDENTIFIER           32
#define RADIUS_NAS_PORTTYPE             61
#define RADIUS_CONNECT_INFO             77
#define RADIUS_EAP_MESSAGE              79
#define RADIUS_MSG_AUTHENTICATOR        80
#define RADIUS_TUNNEL_PVT_GRP_ID        81

/* 802.1X-2001 Section 7.2 EAPOL frame format for 802.3/Ethernet.  */
typedef struct eapol_hdr
{
  /* Protocol Version.  */
  uint8 protocol_version;

  /* Packet Type.  */
  uint8 packet_type;

  /* Packet Body Length.  */
  uint16 packet_length;
} eapol_hdr_t;

/* 802.1X-2001 Section 7.7 EAP packet format (RFC 2284 2.2)  */
typedef struct eap_packet
{
  /* Code.  */
  uint8 code;

  /* Identifier.  */
  uint8 identifier;

  /* Length.  */
  uint16 length;

  /* Type for Request and Response.  */
  uint8 type;

  /* Type-Data for Request and Response.  */
  uint8 type_data[AUTH_USER_NAMLEN+1];
} eap_packet_t;

typedef struct dot1x_sockaddr_vlan
{
  /* Destination Mac address */
  uint8 dest_mac[MAC_ADDR_LEN];

  /* Source Mac address */
  uint8 src_mac[MAC_ADDR_LEN];

  /* Outgoing/Incoming interface index */
  uint16 ifindex;

  /* Vlan id */
  uint16 vlanid;

  /* SVlan id */
  uint16 svlanid;

  /* pkt type */
  uint16 type;

  /* reserve */
  uint16 reserve;
} dot1x_sockaddr_vlan_t;

/* RFC 2865 3 Radius Packet Format.  */
typedef struct radius_hdr
{
  /* Code.  */
  u_char code;

  /* Identifier.  */
  u_char identifier;

  /* Length.  */
  u_int16_t length;

  /* Authenticator.  */
  u_char authenticator[RADIUS_AUTHENTICATOR_LEN];
} radius_hdr_t;

int32
dot1x_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
dot1x_mac_by_pass_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg); 

void
dot1x_send_eapol_req_id(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_send_eapol_req(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_send_eapol_success(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_send_eapol_fail(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_send_radius_eap(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_send_radius_user_authen(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_db_dot1x_mac);

#endif /* !__DOT1X_PACKET_H__ */
