#ifndef __LACP_DEFINE_H__
#define __LACP_DEFINE_H__

#include "lacp_config.h"

/* 30.7.2.1.20 aAggPortActorAdminState, 43.4.2.2 LACPDU structure m) Actor_State. */
#define STATE_FLAG_LACP_ACTIVITY            0x01    /* 1: Active LACP       0: Passive LACP */
#define STATE_FLAG_LACP_TIMEOUT             0x02    /* 1: Short Timeout     0: Long Timeout */
#define STATE_FLAG_AGGREGATION              0x04    /* 1: Aggregatable      0: Individual */
#define STATE_FLAG_SYNCHRONIZATION          0x08    /* 1: IN_SYNC           0: OUT_OF_SYNC */
#define STATE_FLAG_COLLECTING               0x10
#define STATE_FLAG_DISTRIBUTING             0x20
#define STATE_FLAG_DEFAULTED                0x40
#define STATE_FLAG_EXPIRED                  0x80    /* 1: Recv SM EXPIRED   0: Recv SM not EXPIRED */

#define GET_LACP_ACTIVITY(flags)            (flags & STATE_FLAG_LACP_ACTIVITY)
#define GET_LACP_TIMEOUT(flags)             (flags & STATE_FLAG_LACP_TIMEOUT)
#define GET_AGGREGATION(flags)              (flags & STATE_FLAG_AGGREGATION)
#define GET_SYNCHRONIZATION(flags)          (flags & STATE_FLAG_SYNCHRONIZATION)
#define GET_COLLECTING(flags)               (flags & STATE_FLAG_COLLECTING)
#define GET_DISTRIBUTING(flags)             (flags & STATE_FLAG_DISTRIBUTING)
#define GET_DEFAULTED(flags)                (flags & STATE_FLAG_DEFAULTED)
#define GET_EXPIRED(flags)                  (flags & STATE_FLAG_EXPIRED)

#define LACP_ACTIVE_LACP                    STATE_FLAG_LACP_ACTIVITY
#define LACP_PASSIVE_LACP                   0
#define LACP_SHORT_TIMEOUT                  STATE_FLAG_LACP_TIMEOUT
#define LACP_LONG_TIMEOUT                   0
#define LACP_AGGREGATABLE                   STATE_FLAG_AGGREGATION
#define LACP_INDIVIDUAL                     0
#define LACP_IN_SYNC                        STATE_FLAG_SYNCHRONIZATION
#define LACP_OUT_OF_SYNC                    0
#define LACP_COLLECTING_ENABLE              STATE_FLAG_COLLECTING
#define LACP_COLLECTING_DISABLE             0
#define LACP_DISTRIBUTING_ENABLE            STATE_FLAG_DISTRIBUTING
#define LACP_DISTRIBUTING_DISABLE           0
#define LACP_DEFAULTED                      STATE_FLAG_DEFAULTED
#define LACP_NOT_DEFAULTED                  0
#define LACP_EXPIRED                        STATE_FLAG_EXPIRED
#define LACP_NOT_EXPIRED                    0

/* 43.4.4 Timer Period */
#define FAST_PERIODIC_TIME                  1
#define SLOW_PERIODIC_TIME                  30
#define SHORT_TIMEOUT_TIME                  3
#define LONG_TIMEOUT_TIME                   90
#define CHURN_DETECTION_TIME                60
#define AGGREGATE_WAIT_TIME                 2

/* PDU */
/* 43.4.2.2.d */
#define LACP_SUBTYPE                        0x01
/* 43.4.2.2.e */
#define LACP_VERSION                        0x01
/* 43.4.2.2.f */
#define LACP_ACTOR_INFORMATION_TLV          0x01
/* 43.4.2.2.g */
#define LACP_ACTOR_INFORMATION_LENGTH       0x14
/* 43.4.2.2.o */
#define LACP_PARTNER_INFORMATION_TLV        0x02
/* 43.4.2.2.p */
#define LACP_PARTNER_INFORMATION_LENGTH     0x14
/* 43.4.2.2.o */
#define LACP_COLLECTOR_INFORMATION_TLV      0x03
/* 43.4.2.2.p */
#define LACP_COLLECTOR_INFORMATION_LENGTH   0x10
/* 43.4.2.2.z */
#define LACP_COLLECTOR_MAX_DELAY            0x05
/* 43.4.2.2.ab */
#define LACP_TERMINATOR_TLV                 0x00
/* 43.4.2.2.ac */
#define LACP_TERMINATOR_LENGTH              0x00

/* 43.5.3.2.d */
#define MARKER_SUBTYPE                      0x02
/* 43.5.3.2.e */
#define MARKER_VERSION                      0x01
/* 43.5.3.2.f */
#define MARKER_INFORMATION_TLV              0x01
/* 43.5.3.2.f */
#define MARKER_RESPONSE_TLV                 0x02
/* 43.5.3.2.g */
#define MARKER_INFORMATION_LENGTH           0x10
/* 43.5.3.2.g */
#define MARKER_RESPONSE_LENGTH              0x10
/* 43.5.3.2.g */
#define MARKER_RESPONSE_LENGTH              0x10
/* 43.5.3.2.l */
#define MARKER_TERMINATOR_TLV               0x00
/* 43.5.3.2.m */
#define MARKER_TERMINATOR_LENGTH            0x00

#define LACP_MLAG_FLAG                      0x8000

typedef enum
{
    SELECTED_UNSELECTED,
    SELECTED_SELECTED,
    SELECTED_STANDBY,
    SELECTED_STATE_MAX
} lacp_selected_t;

typedef enum
{
    RX_EVENT_BEGIN,
    RX_EVENT_PORT_MOVED,
    RX_EVENT_PORTENABLED_LACPENABLED,
    RX_EVENT_PORTENABLED_LACPDISABLED,
    RX_EVENT_CURRWHILE_TIMER_EXPIRED,
    RX_EVENT_PDU_RX,
    RX_EVENT_PORT_DISABLED,
    RX_EVENT_MAX
} lacp_rx_event_t;

typedef enum
{
    RX_INITIALIZE = 0,
    RX_PORT_DISABLED,
    RX_LACP_DISABLED,
    RX_EXPIRED,
    RX_DEFAULTED,
    RX_CURRENT,
    RX_STATE_MAX
} lacp_rx_state_t;

typedef enum
{
    MUX_EVENT_BEGIN,
    MUX_EVENT_SELECTED,
    MUX_EVENT_STANDBY,
    MUX_EVENT_UNSELECTED,
    MUX_EVENT_SELECTED_READYTRUE,
    MUX_EVENT_SELECTED_PARTNER_INSYNC,
    MUX_EVENT_PARTNER_OUTOFSYNC,
    MUX_EVENT_MAX
} lacp_mux_event_t;

typedef enum
{
    MUX_DETACHED = 0,
    MUX_WAITING,
    MUX_ATTACHED,
    MUX_COLLECTING_DISTRIBUTING,
    MUX_STATE_MAX
} lacp_mux_state_t;

typedef enum
{
    PERTX_EVENT_DISABLE,
    PERTX_EVENT_ENABLE,
    PERTX_EVENT_SHORT_TIMEOUT,
    PERTX_EVENT_LONG_TIMEOUT,
    PERTX_EVENT_PERIODIC_TIME_EXPIRED,
    PERTX_EVENT_MAX
} lacp_pertx_event_t;

typedef enum
{
    PERTX_NO_PERIODIC = 0,
    PERTX_FAST_PERIODIC,
    PERTX_SLOW_PERIODIC,
    PERTX_PERIODIC_TX,
    PERTX_STATE_MAX
} lacp_pertx_state_t;

#define LACP_SELECTED_MAX       SELECTED_STATE_MAX
#define LACP_RX_EVENT_MAX       RX_EVENT_MAX
#define LACP_RX_STATE_MAX       RX_STATE_MAX
#define LACP_MUX_EVENT_MAX      MUX_EVENT_MAX
#define LACP_MUX_STATE_MAX      MUX_STATE_MAX
#define LACP_PERTX_EVENT_MAX    PERTX_EVENT_MAX
#define LACP_PERTX_STATE_MAX    PERTX_STATE_MAX

extern const char *lacp_selected_strs[SELECTED_STATE_MAX];
extern const char *lacp_rx_event_strs[RX_EVENT_MAX];
extern const char *lacp_rx_state_strs[RX_STATE_MAX];
extern const char *lacp_pertx_event_strs[PERTX_EVENT_MAX];
extern const char *lacp_pertx_state_strs[PERTX_STATE_MAX];
extern const char *lacp_mux_event_strs[MUX_EVENT_MAX];
extern const char *lacp_mux_state_strs[MUX_STATE_MAX];

#define LACP_DBG_INFO(FMT, ARGS...) \
do { \
    log_sys(M_MOD_LACP, E_INFORMATIONAL, FMT,  ##ARGS); \
} while (0)

#define LACP_DBG_WARN(FMT, ARGS...) \
do { \
    log_sys(M_MOD_LACP, E_WARNING,  FMT,  ##ARGS); \
} while (0)

#define LACP_DBG_ERR(FMT, ARGS...) \
do { \
    log_sys(M_MOD_LACP, E_ERROR,  FMT,  ##ARGS); \
} while (0)

#define LACP_DBG_CRIT(FMT, ARGS...) \
do { \
    log_sys(M_MOD_LACP, E_CRITICAL,  FMT,  ##ARGS); \
} while (0)

#endif /* !__LACP_DEFINE_H__ */
