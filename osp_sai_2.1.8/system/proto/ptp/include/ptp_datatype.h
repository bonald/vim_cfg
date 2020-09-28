
#ifndef __PTP_DATATYPE_H__
#define __PTP_DATATYPE_H__


/* refer to 13.6.1 General Sync and Delay_Req message specifications Table 26 */
struct ptp_msg_sync_s
{
    glb_ptp_timestamp_t origin_timestamp;
};
typedef struct ptp_msg_sync_s ptp_msg_sync_t;

struct ptp_msg_delay_req_s
{
    glb_ptp_timestamp_t origin_timestamp;
};
typedef struct ptp_msg_delay_req_s ptp_msg_delay_req_t;

/* refer to 13.7.1 General Follow_Up message specifications Table 27 */
struct ptp_msg_follow_up_s
{
    glb_ptp_timestamp_t precise_origin_timestamp;
};
typedef struct ptp_msg_follow_up_s ptp_msg_follow_up_t;

/* refer to 13.8.1 General Delay_Resp message specifications Table 28 */
struct ptp_msg_delay_resp_s
{
    glb_ptp_timestamp_t receive_timestamp;
    glb_ptp_port_id_t requesting_port_identity;
};
typedef struct ptp_msg_delay_resp_s ptp_msg_delay_resp_t;

/* refer to 13.9.1 General Pdelay_Req message specifications Table 29 */
struct ptp_msg_pdelay_req_s
{
    glb_ptp_timestamp_t origin_timestamp;
};
typedef struct ptp_msg_pdelay_req_s ptp_msg_pdelay_req_t;

/* refer to 13.10.1 General Pdelay_Resp message specifications Table 30 */
struct ptp_msg_pdelay_resp_s
{
    glb_ptp_timestamp_t request_receipt_timestamp;
    glb_ptp_port_id_t requesting_port_identity;
};
typedef struct ptp_msg_pdelay_resp_s ptp_msg_pdelay_resp_t;

/* refer to 13.11.1 General Pdelay_Resp_Follow_Up message specifications Table 31 */
struct ptp_msg_pdelay_resp_followup_s
{
    glb_ptp_timestamp_t response_origin_timestamp;
    glb_ptp_port_id_t requesting_port_identity;
};
typedef struct ptp_msg_pdelay_resp_followup_s ptp_msg_pdelay_resp_followup_t;


    
#endif /* !__PTP_DATATYPE_H__ */

