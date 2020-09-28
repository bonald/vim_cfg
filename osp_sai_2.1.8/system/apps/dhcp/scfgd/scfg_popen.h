/****************************************************************************
* $Id$
*  POpen interface for scfgd
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-25 11:28
* Reason        : First Create.
****************************************************************************/

#ifndef _SCFG_POPEN_H_
#define _SCFG_POPEN_H_

typedef struct {
    void (*release)(void *pArg);
    void (*proc)(char *szBuf, int len); /* use this to process msgs */
    void (*done)(unsigned short nResult, void *pArg);
} SCFG_SYSTEM_OPS_T;

typedef struct {
    FILE *fp;
    void *reader; /* thread handle */

    SCFG_SYSTEM_OPS_T op;
    void *pArg;

    int refcnt;
} SCFG_EXEC_T;

typedef struct {
    char *pszSrc; /* source file name (don't include path) */
    char *pszDest;
} SCFG_TFTP_ARG_T;

SCFG_EXEC_T *scfg_system(char *pszCmd, char *pszMode, 
        SCFG_SYSTEM_OPS_T *pFuncs, void *pArg);
void scfg_cancel_exec(SCFG_EXEC_T *p);
SCFG_EXEC_T *scfg_calloc_exec(void);

int scfg_popen_init(void);
void scfg_process_sigchild(pid_t pid, int pstat);

FILE *scfg_popen(const char *program, const char *type);
int scfg_pclose(FILE *iop);

SCFG_TFTP_ARG_T *scfg_tftp_init(char *pszSrc);
#define M_TMPFILE_TEMPLATE      "/tmp/.smartcfg-XXXXXX"

#endif /* !_SCFG_POPEN_H_ */
