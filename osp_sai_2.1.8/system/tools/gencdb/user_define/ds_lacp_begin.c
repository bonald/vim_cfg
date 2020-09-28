#include "lacp_define.h"

const char *lacp_selected_strs[SELECTED_STATE_MAX] =
{
    "UNSELECTED",
    "SELECTED",
    "STANDBY"
};

const char *lacp_rx_event_strs[RX_EVENT_MAX] =
{
    "begin",
    "port_moved",
    "portenabled_lacpenabled",
    "portenabled_lacpdisabled",
    "currwhile_timer_expired",
    "pdu_rx",
    "port_disabled"
};

const char *lacp_rx_state_strs[RX_STATE_MAX] =
{
    "INITIALIZE",
    "PORT_DISABLED",
    "LACP_DISABLED",
    "EXPIRED",
    "DEFAULTED",
    "CURRENT"
};

const char *lacp_pertx_event_strs[PERTX_EVENT_MAX] =
{
    "disable",
    "enable",
    "short_timeout",
    "long_timeout",
    "periodic_time_expired"
};

const char *lacp_pertx_state_strs[PERTX_STATE_MAX] =
{
    "NO_PERIODIC",
    "FAST_PERIODIC",
    "SLOW_PERIODIC",
    "PERIODIC_TX"
};

const char *lacp_mux_event_strs[MUX_EVENT_MAX] =
{
    "begin",
    "selected",
    "standby",
    "unselected",
    "selected+ready",
    "selected+partner_insync",
    "partner_outofsync"
};

const char *lacp_mux_state_strs[MUX_STATE_MAX] =
{
    "DETACHED",
    "WAITING",
    "ATTACHED",
    "COLLECTING_DISTRIBUTING"
};
