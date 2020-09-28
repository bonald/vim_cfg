/****************************************************************************
 * $Id$
 *  Centec SyncE related global MACRO, ENUM, Date Structure defines file
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Zhen Jiang
 * Date          : 2011-03-05 16:00
 * Reason        : First Create.
 ****************************************************************************/
#ifndef __GLB_G8131_DEFINE_H__
#define __GLB_G8131_DEFINE_H__

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#define G8131_WTR_TIMER_MIN_VALUE                          0
#define G8131_WTR_TIMER_MAX_VALUE                          720
#define G8131_DEFAULT_WTR_TIMEOUT                          300

#define G8131_HOLDOFF_TIMER_MIN_VALUE                      0
#define G8131_HOLDOFF_TIMER_MAX_VALUE                      100

#define G8131_APS_FIRST_TLV_OFFSET                         4
#define G8131_INVALID_VAL                                  0

/* Possible values for Protection Type A bit - APS or no APS */
#define G8131_NO_APS_CHANNEL                               0
#define G8131_APS_CHANNEL                                  1

/* Possible values for Protection Type B bit - 1:1 or 1+1 */
#define G8131_PERMANENT_BRIDGE                             0
#define G8131_NO_PERMANENT_BRIDGE                          1

/* Possible values for Protection Type D bit - uni or bi-directional */
#define G8131_UNIDIRECTIONAL_SWITCHING                     0
#define G8131_BIDIRECTIONAL_SWITCHING                      1

/* Possible values for Protection Type R bit - revertive or non-revertive */
#define G8131_NON_REVERTIVE_SWITCHING                      0
#define G8131_REVERTIVE_SWITCHING                          1

#define G8131_NULL_SIGNAL                                  0
#define G8131_NORMAL_TRAFFIC_SIGNAL                        1

#define G8131_DEFAULT_APS_TX_INTERVAL                      5

#define G8131_NUM_OF_APS_FRAMES_AFTER_STATE_CHANGE         3
#define G8131_STATE_CHANGE_APS_PDU_INTERVAL_M_SECONDS      3

#define G8131_TRUE                                         1
#define G8131_FALSE                                        0

#define G8131_TIMER_WTR                                    0
#define G8131_TIMER_HOLD_OFF                               1

#define G8131_ACTIVE_NONE                                  0
#define G8131_ACTIVE_WORKING                               1
#define G8131_ACTIVE_PROTECTING                            2

#define G8131_APS_MAXIMUM_COUNT                            500
#define G8131_APS_MAXIMUM                                  4294967040UL

enum g8131_return_code
{
    G8131_SUCCESS = 0,
    G8131_FAILURE,
    G8131_INVALID_PTR,
    G8131_PROTECTION_GROUP_NOT_EXISTS,
    G8131_A_BIT_MISMATCH_ERROR,
    G8131_B_BIT_MISMATCH_ERROR,
    G8131_D_BIT_MISMATCH_ERROR,
    G8131_R_BIT_MISMATCH_ERROR,
    G8131_T_BIT_MISMATCH_ERROR,
    G8131_XCALLOC_FAILED,
    G8131_PDU_PARSING_FAILED,
    G8131_INVALID_ABDR_IN_PDU,
    G8131_INVALID_REQ_STATE_IN_PDU,
    G8131_HAND_OVER_REQ_TO_FAR_END_FSM,
    G8131_HAND_OVER_REQ_TO_EXER_DNR_LOGIC,
    G8131_DFOP_OCCURED,
    G8131_LOCKOUT_ALREADY_EXISTS,
    G8131_FORCE_SWITCH_ALREADY_EXISTS,
    G8131_MS_TO_P_ALREADY_EXISTS,
    G8131_MS_TO_W_ALREADY_EXISTS,
    G8131_EXERCISE_ALREADY_EXISTS,
    G8131_PG_ALREADY_INITIALIZED,
    G8131_LOWER_PRIO_CMD_NOT_ALLOWED,
    G8131_WTR_NOT_ALLOWED_FOR_NON_REVERTIVE,
    G8131_APS_PDU_RCVD_ON_WORKING_INTERFACE,
    G8131_PG_NOT_IN_ACTIVE_STATE,
    G8131_TUNNEL_HASH_NOT_INIT,
    G8131_TUNNEL_NOT_FOUND,
    G8131_TUNNEL_APS_NOT_ENABLE,
    G8131_VPWS_NOT_FOUND,
    G8131_VPWS_APS_NOT_ENABLE,
    G8131_VPLS_NOT_FOUND,
    G8131_VPLS_PEER_NOT_FOUND,
    G8131_VPLS_PEER_APS_NOT_ENABLE
};

#define G8131_IN_SF_HOLD_OFF_STATE     (1<<0)
#define G8131_IN_SFP_HOLD_OFF_STATE    (1<<1)
#define G8131_IN_SD_HOLD_OFF_STATE     (1<<2)
#define G8131_IN_SDP_HOLD_OFF_STATE    (1<<3)


#define G8131_APS_CHANNEL_MASK        (1<<3)
#define G8131_NO_PERMANENT_BR         (1<<2)
#define G8131_REQUEST_SIGNAL_NULL      0
#define G8131_REQUEST_SIGNAL_NORMAL    1
#define G8131_DIRECTION_BI            (1<<1)
#define G8131_SELECT_BRIDGE            0
#define G8131_BROADCAST_BRIDGE         1
#define G8131_REVERTIVE               (1<<0)

#define G8131_APS_GROUP_FLAG_ADD_BY_FLOW  (1<<0)

enum g8131_aps_group_type_e
{
    G8131_APS_GROUP_TYPE_LSP = 0,
    G8131_APS_GROUP_TYPE_PW,
    G8131_APS_GROUP_TYPE_MAX
};
typedef enum g8131_aps_group_type_e g8131_aps_group_type_t;

/**/
struct fei_g8131_mpls_fast_aps_req_s
{
    uint32  aps_group_id;
    uint32  aps_type;
    uint32  fast_aps_en;
};
typedef struct fei_g8131_mpls_fast_aps_req_s fei_g8131_mpls_fast_aps_req_t;

/**/
struct fei_g8131_mpls_protection_en_req_s
{
    uint32  aps_group_id;
    uint32  aps_type;
    uint32  protection_en;
};
typedef struct fei_g8131_mpls_protection_en_req_s fei_g8131_mpls_protection_en_req_t;

/* G.8131 FSM States. */
enum
{
  G8131_FSM_STATE_NR_W = 0, /*(A)No Request,          Working active,  Protecting Standby*/
  G8131_FSM_STATE_NR_P,     /*(B)No Request,          Working standby, Protecting Active*/
  G8131_FSM_STATE_LO,       /*(C)Lockout,             Working active,  Protecting Standby*/
  G8131_FSM_STATE_FS,       /*(D)Forced Switch,       Working standby, Protecting Active*/
  G8131_FSM_STATE_SF_W,     /*(E)Signal Fail on W,    Working standby, Protecting Active*/
  G8131_FSM_STATE_SF_P,     /*(F)Signal Fail on P,    Working active,  Protecting Standby*/
  G8131_FSM_STATE_SD_W,     /*(P)Signal Degrade on W, Working standby, Protecting Active*/
  G8131_FSM_STATE_SD_P,     /*(Q)Signal Degrade on P, Working active,  Protecting Standby*/
  G8131_FSM_STATE_MS_P,     /*(G)Manual Switch to P,  Working standby, Protecting Active*/
  G8131_FSM_STATE_MS_W,     /*(H)Manual Switch to W,  Working active,  Protecting Standby*/
  G8131_FSM_STATE_WTR,      /*(I)Wait-to-Restore,     Working standby, Protecting Active*/
  G8131_FSM_STATE_DNR,      /*(J)Do Not Revert,       Working standby, Protecting Active*/
  G8131_FSM_STATE_EXER_W,   /*(K)Exercise on W,       Working active,  Protecting Standby*/
  G8131_FSM_STATE_EXER_P,   /*(L)Exercise on P,       Working Standby,  Protecting active*/
  G8131_FSM_STATE_RR_W,     /*(M)Reverse Request on W,Working active,  Protecting Standby*/
  G8131_FSM_STATE_RR_P,     /*(N)Reverse Request on P,Working Standby,  Protecting active*/
  G8131_FSM_STATE_MAX
};

/* G.8131 FSM Events. */
enum
{
  /*Local Events*/
  G8131_FSM_L_EVENT_LO = 0,            /*0  (a)(a)Lockout*/
  G8131_FSM_L_EVENT_FS,                /*1  (b)(b)Forced Switch*/
  G8131_FSM_L_EVENT_SF_W,              /*2  (c)(c)Signal Fail on Working*/
  G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF, /*3  (d)(d)Working recovers from SF*/
  G8131_FSM_L_EVENT_SF_P,              /*4  (e)(e)Signal Fail on Protection*/
  G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF, /*5  (f)(f)Protection recovers from SF*/
  G8131_FSM_L_EVENT_SD_W,              /*6  (g)(g)Signal Degrade on Working*/
  G8131_FSM_L_EVENT_W_RCOVERS_FROM_SD, /*7  (h)(h)Working recovers from SD*/
  G8131_FSM_L_EVENT_SD_P,              /*8  (i)(i)Signal Degrade on Protection*/
  G8131_FSM_L_EVENT_P_RCOVERS_FROM_SD, /*9  (j)(j)Protection recovers from SD*/
  G8131_FSM_L_EVENT_MS_P,              /*10 (k)(k)Manual Switch to protection*/
  G8131_FSM_L_EVENT_MS_W,              /*11 (l)(l)Manual Switch to working*/
  G8131_FSM_L_EVENT_CLEAR,             /*12 (m)(m)Clear*/
  G8131_FSM_L_EVENT_EXER,              /*13 (n)(n)Exercise*/
  G8131_FSM_L_EVENT_WTR,               /*14 (o)(/)WTR timer expires*/

  /*Remote Events*/
  G8131_FSM_R_EVENT_LO,            /*15 (p)(o)Lockout*/
  G8131_FSM_R_EVENT_SF_P,          /*16 (q)(p)Signal Fail on Protection*/
  G8131_FSM_R_EVENT_FS,            /*17 (r)(q)Force switch*/
  G8131_FSM_R_EVENT_SF_W,          /*18 (s)(r)Signal Fail on working*/
  G8131_FSM_R_EVENT_SD_W,          /*19 (t)(s)Signal Degrade on working*/
  G8131_FSM_R_EVENT_SD_P,          /*20 (u)(t)Signal Degrade on protection*/
  G8131_FSM_R_EVENT_MS_P,          /*21 (v)(u)Manual Switch to protection*/
  G8131_FSM_R_EVENT_MS_W,          /*22 (w)(v)Manual Switch to working*/
  G8131_FSM_R_EVENT_WTR,           /*23 (x)(w)Wait-to-Restore*/
  G8131_FSM_R_EVENT_EXER_W,        /*24 (y)(x)Exercise on working*/
  G8131_FSM_R_EVENT_EXER_P,        /*25 (/)(y)Exercise on protection*/
  G8131_FSM_R_EVENT_RR_W,          /*26 (z)(z)Reverse Request on working*/
  G8131_FSM_R_EVENT_RR_P,          /*27 (/)(aa)Reverse Request on protection*/
  G8131_FSM_R_EVENT_NR_NULL,       /*28 (aa)(ab)No Request, r/b = null*/
  G8131_FSM_R_EVENT_NR_NORMAL,     /*29 (ab)(ac)No Request, r/b = normal*/
  G8131_FSM_R_EVENT_DNR,           /*30 (ac)(ad)Do Not Revert*/
  G8131_FSM_EVENT_MAX
};

/* G.8131 Request States. */
enum
{
  G8131_REQ_STATE_NR   =  0, /*No Request*/
  G8131_REQ_STATE_DNR  =  1, /*Do Not Revert*/
  G8131_REQ_STATE_RR   =  2, /*Reverse Request*/
  G8131_REQ_STATE_EXER =  4, /*Exercise*/
  G8131_REQ_STATE_WTR  =  5, /*Wait-to-Restore*/
  G8131_REQ_STATE_MS   =  7, /*Manual Switch*/
  G8131_REQ_STATE_SD   =  9, /*Signal Degrade*/
  G8131_REQ_STATE_SF_W = 11, /*Signal Fail for Working*/
  G8131_REQ_STATE_FS   = 13, /*Force Switch*/
  G8131_REQ_STATE_SF_P = 14, /*Signal Fail for Protection*/
  G8131_REQ_STATE_LO   = 15, /*Lockout of Protection*/
  G8131_REQ_STATE_MAX
};

#endif /*!__GLB_G8131_DEFINE_H__*/

