#ifndef __LCSH_DEBUG_H__
#define __LCSH_DEBUG_H__

#include "genlog.h"
#ifndef CNOS_OS
#include "ctclib_debug.h"
#endif

#ifdef CNOS_OS
#define LCSH_LOG_ERR(fmt, args...)   log_sys(M_MOD_INTERNAL, E_ERROR, fmt, ##args)
#else
#define LCSH_LOG_EMERG(fmt, args...)   log_sys(M_MOD_LCSH, E_EMERGENCY, fmt, ##args)
#define LCSH_LOG_ALERT(fmt, args...)   log_sys(M_MOD_LCSH, E_ALERT, fmt, ##args)
#define LCSH_LOG_CRIT(fmt, args...)   log_sys(M_MOD_LCSH, E_CRITICAL, fmt, ##args)
#define LCSH_LOG_ERR(fmt, args...)   log_sys(M_MOD_LCSH, E_ERROR, fmt, ##args)
#define LCSH_LOG_WARN(fmt, args...)   log_sys(M_MOD_LCSH, E_WARNING, fmt, ##args)
#define LCSH_LOG_NOTICE(fmt, args...)  log_sys(M_MOD_LCSH, E_NOTICE, fmt, ##args)
#define LCSH_LOG_INFO(fmt, args...)   log_sys(M_MOD_LCSH, E_INFORMATIONAL, fmt, ##args)
#endif

#endif /*__LCSH_DEBUG_H__*/
