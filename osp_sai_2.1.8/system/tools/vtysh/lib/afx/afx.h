#ifndef __AFX_H__
#define __AFX_H__

/*
 * Application Framework Library
 */
#include <sys/types.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/stddef.h>
#include <string.h>
#include <time.h>

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define afx_container_of(p, type, member) \
({const typeof(((type *)0)->member) *_p = p; (type *)((char *)_p - offsetof(type, member));})

#include "afx_list.h"
#include "afx_evtlp.h"

#endif /* !__AFX_H__ */
