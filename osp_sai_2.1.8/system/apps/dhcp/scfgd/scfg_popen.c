/****************************************************************************
* $Id$
*  POpen interface for scfgd
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-25 11:27
* Reason        : First Create.
****************************************************************************/
#include <sys/param.h>
#include <sys/wait.h>

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include <pthread.h>
#include "pal.h"
#include "glb_hw_define.h"
#include "scfg_popen.h"
#include "thread.h"

#define M_INPUTBUF_SIZE     256

static struct pid {
    struct pid *next;
    FILE *fp;
    pid_t pid;
    int pstat; /* store process stat of SIGCHILD */
} *pidlist;

static pthread_mutex_t pidlist_lock;

/* success return 1, otherwise return 0 */
int scfg_popen_init(void)
{
    pidlist = NULL;
    if (pthread_mutex_init(&pidlist_lock, NULL) != 0)
    {
        return 0;
    }

    return 1;
}

/* setting process shutdown status */
void
scfg_process_sigchild(pid_t pid, int pstat)
{
    struct pid *cur;

    /* Find the appropriate file pointer. */
    pthread_mutex_lock(&pidlist_lock);
    for (cur = pidlist; cur; cur = cur->next)
    {
        if (pid == cur->pid)
            break;
    }

    if (NULL != cur) 
    {
        cur->pstat = pstat;
    }

    pthread_mutex_unlock(&pidlist_lock);
    return; 
}

FILE *
scfg_popen(const char *program, const char *type)
{
    struct pid * volatile cur;
    FILE *iop;
    int pdes[2];
    pid_t pid;

    if ((*type != 'r' && *type != 'w') || type[1] != '\0') 
    {
        errno = EINVAL;
        return (NULL);
    }

    if ((cur = XMALLOC(MTYPE_TMP, sizeof(struct pid))) == NULL)
    {
        return (NULL);
    }

    if (pipe(pdes) < 0) 
    {
        XFREE(MTYPE_TMP, cur);
        return (NULL);
    }

    pthread_mutex_lock(&pidlist_lock);
    switch (pid = vfork()) 
    {
    case -1:            /* Error. */
        pthread_mutex_unlock(&pidlist_lock);
        (void)close(pdes[0]);
        (void)close(pdes[1]);
        XFREE(MTYPE_TMP, cur);
        return (NULL);
        /* NOTREACHED */
    case 0:                /* Child. */
        {
        struct pid *pcur;
        /*
         * because vfork() instead of fork(), must leak FILE *,
         * but luckily we are terminally headed for an execl()
         */
        for (pcur = pidlist; pcur; pcur = pcur->next)
            close(fileno(pcur->fp));

        if (*type == 'r') 
        {
            int tpdes1 = pdes[1];

            (void) close(pdes[0]);
            /*
             * We must NOT modify pdes, due to the
             * semantics of vfork.
             */
            if (tpdes1 != STDOUT_FILENO) 
            {
                (void)dup2(tpdes1, STDOUT_FILENO);
                (void)close(tpdes1);
                tpdes1 = STDOUT_FILENO;
            }
        } 
        else 
        {
            (void)close(pdes[1]);
            if (pdes[0] != STDIN_FILENO) 
            {
                (void)dup2(pdes[0], STDIN_FILENO);
                (void)close(pdes[0]);
            }
        }
        execl(_PATH_BSHELL, "sh", "-c", program, (char *)NULL);
        _exit(127);
        /* NOTREACHED */
        }
    }
    pthread_mutex_unlock(&pidlist_lock);

    /* Parent; assume fdopen can't fail. */
    if (*type == 'r') 
    {
        iop = fdopen(pdes[0], type);
        (void)close(pdes[1]);
    } 
    else 
    {
        iop = fdopen(pdes[1], type);
        (void)close(pdes[0]);
    }

    /* Link into list of file descriptors. */
    cur->fp = iop;
    cur->pid =  pid;
    cur->pstat = -1;
    pthread_mutex_lock(&pidlist_lock);
    cur->next = pidlist;
    pidlist = cur;
    pthread_mutex_unlock(&pidlist_lock);

    return (iop);
}

/*
 *    returns -1 if stream is not associated with a `popened' command,
 *    if already `pclosed', or waitpid returns an error.
 */
int
scfg_pclose(FILE *iop)
{
    struct pid *cur, *last;
    int pstat;
    pid_t pid;

    /* Find the appropriate file pointer. */
    pthread_mutex_lock(&pidlist_lock);
    for (last = NULL, cur = pidlist; cur; last = cur, cur = cur->next)
    {
        if (cur->fp == iop)
            break;
    }

    if (cur == NULL) 
    {
        pthread_mutex_unlock(&pidlist_lock);
        return (-1);
    }

    /* Remove the entry from the linked list. */
    if (last == NULL)
        pidlist = cur->next;
    else
        last->next = cur->next;
    pthread_mutex_unlock(&pidlist_lock);

    (void)fclose(iop);

    if (-1 == cur->pstat)
    {
        do 
        {
            pid = waitpid(cur->pid, &pstat, 0);
        } while (pid == -1 && errno == EINTR);
    }
    else
    {
        pid = cur->pid;
        pstat = cur->pstat;
    }

    XFREE(MTYPE_TMP, cur);

    return (pid == -1 ? -1 : pstat);
}


SCFG_EXEC_T *
scfg_calloc_exec(void)
{
    return XCALLOC(MTYPE_TMP, sizeof(SCFG_EXEC_T));
}

void
scfg_free_exec(SCFG_EXEC_T *p)
{
    if (NULL != p)
    {
        if (p->refcnt > 0)
        {
            return;
        }

        if (NULL != p->fp)
        {
            scfg_pclose(p->fp);
            p->fp = NULL;
        }

        if (NULL != p->reader)
        {
            thread_cancel((struct thread *)p->reader);
            p->reader = NULL;
        }

        if (NULL != p->pArg)
        {
            if (p->op.release)
            {
                p->op.release(p->pArg);
            }
        }

        XFREE(MTYPE_TMP, p);
        p = NULL;
    }
    return;
}

void
scfg_cancel_exec(SCFG_EXEC_T *p)
{
    if (NULL != p)
    {
        p->op.done = NULL;
        p->op.proc = NULL;

        p->refcnt--;
        if (p->refcnt > 0)
        {
            return;
        }

        scfg_free_exec(p);
    }
    return;
}

static int
scfg_exec_handle(struct thread *th)
{
    SCFG_EXEC_T *pExec = (SCFG_EXEC_T *)THREAD_ARG(th);
    int fd = THREAD_FD(th);
    SCFG_SYSTEM_OPS_T *pFuncs = NULL;
    char szBuf[M_INPUTBUF_SIZE];
    int nRet = -1;

    if (NULL == pExec || NULL == pExec->fp)
    {
        assert(0);
        return -1;
    }
    pFuncs = &pExec->op;

    pExec->reader = NULL;

    nRet = read(fd, szBuf, M_INPUTBUF_SIZE - 1);
    if (nRet <= 0)
    {
        if (nRet < 0)
        {
            log_sys(M_MOD_SCFG, E_ERROR, 
                    "Read pipe failed: %s\n", strerror(errno));
        }
        nRet = scfg_pclose(pExec->fp);
        pExec->fp = NULL;
        if (pFuncs->done)
        {
            if (-1 == nRet)
            {
                log_sys(M_MOD_SCFG, E_ERROR,
                        "Pclose failed. ret == -1");
            }
            pFuncs->done(ntohs(nRet), pExec->pArg);
        }
        if (pFuncs->release)
        {
            pFuncs->release(pExec->pArg);
        }
        pExec->pArg = NULL;
        pExec->refcnt--;
        scfg_free_exec(pExec);
        return -1;
    }

    szBuf[nRet] = 0x00;
    if (pFuncs->proc)
    {
        pFuncs->proc(szBuf, nRet);
    }
    pExec->reader = thread_add_read(lib_globals_get(), scfg_exec_handle, pExec, fd);
    return 0;
}

SCFG_EXEC_T *
scfg_system(char *pszCmd, char *pszMode, SCFG_SYSTEM_OPS_T *pFuncs, void *pArg)
{
    SCFG_EXEC_T *pExec = NULL;
    struct thread *rth = NULL;

    if (NULL == pszCmd || NULL == pFuncs) 
    {
        assert(0);
        return NULL;
    }
    
    if (NULL == (pExec = scfg_calloc_exec())) {
        log_sys(M_MOD_SCFG, E_ERROR, "Warning: Out of memory");
        return NULL;
    }

    pExec->pArg = pArg;
    if (NULL == (pExec->fp = scfg_popen(pszCmd, pszMode))) {
        log_sys(M_MOD_SCFG, E_ERROR, "Popen failed: %s", strerror(errno));
        goto err_out;
    }

    rth = thread_add_read(lib_globals_get(), scfg_exec_handle, pExec, fileno(pExec->fp));
    if (NULL == rth)
    {
        log_sys(M_MOD_SCFG, E_ERROR, "Warning: Out of memory");
        goto err_out;
    }
    
    pExec->reader = rth;
    pExec->refcnt++;
    pal_mem_cpy(&pExec->op, pFuncs, sizeof(SCFG_SYSTEM_OPS_T));
    return pExec;

err_out:
    if (NULL != pExec)
    {
        scfg_free_exec(pExec);
    }
    if (NULL != rth)
    {
        thread_cancel(rth);
    }
    return NULL;
}


