/*
 * clish_script_callback.c
 *
 * Callback hook to action a shell script.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/queue.h>
#include <stdarg.h>

#include "lib_fs.h"
#include "lub/string.h"
#include "konf/buf.h"
#include "internal.h"

/* execute in pipe line mode */
#define M_PAGER_PROG    "ctcmore"

#ifndef TAILQ_HEAD_INITIALIZER
#define	TAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define	TAILQ_EMPTY(head)	((head)->tqh_first == NULL)

#define	TAILQ_FIRST(head)	((head)->tqh_first)

#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define	TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define	TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))

#endif

typedef struct clish_process
{
  TAILQ_ENTRY(clish_process) entry;

  const char **argv; /* Child execute argv */

  pid_t pid; /* id of child process */

  command_fn_t *pCmdFunc; /* callback function */
  void *pArg; /* argument for callback */

  char *filename; /* filename for redirection */

  int status;
  int completed;
} clish_process_t;

/* process queue header */
typedef TAILQ_HEAD(tagClishProcessHdr, clish_process) clish_process_hdr_t;

static clish_process_hdr_t clish_process_hdr = TAILQ_HEAD_INITIALIZER(clish_process_hdr);
extern u_int16_t g_term_lines;

/* Allocate a new process for pipeline.  */
static clish_process_t *__clish_pipe_add (command_fn_t *pCmdFunc, void *pArg, 
    const char **argv, const char *filename)
{
    clish_process_t *proc;

    proc = calloc (1, sizeof (clish_process_t));
    if (NULL != proc) {
        TAILQ_INSERT_TAIL(&clish_process_hdr, proc, entry);
        if (NULL != pCmdFunc) {
            proc->pCmdFunc = pCmdFunc;
            proc->pArg = pArg;
        } else if (NULL != argv) {
            proc->argv = argv;
        }
        if (NULL != filename) {
            proc->filename = lub_string_dup(filename);
        }
    }
    
    return proc;
}

/* Free pipeline memory.  */
static void __clish_pipe_free ()
{
    clish_process_t *proc;

    while (!TAILQ_EMPTY(&clish_process_hdr)) {
        proc = (clish_process_t *)TAILQ_FIRST(&clish_process_hdr);
        TAILQ_REMOVE(&clish_process_hdr, proc, entry);

        if (NULL != proc->pCmdFunc) {
            proc->pCmdFunc(proc->pArg, 1); /* free callback argument */
        }
        if (NULL != proc->filename) {
            lub_string_free(proc->filename);
        }
        free(proc);
    }
    return;
}

/* Free pipeline memory.  */
void __clish_pipe_term ()
{
    clish_process_t *proc;

    TAILQ_FOREACH(proc, &clish_process_hdr, entry) {
        if (proc->pid >= 1) {
            kill(proc->pid, SIGHUP);
        }
    }

    return;
}


/* Return 1 when all child processes are completed.  */
static int __clish_process_completed ()
{
    clish_process_t *proc;

    TAILQ_FOREACH(proc, &clish_process_hdr, entry) {
        if (!proc->completed) {
            return 0;
        }
    }
    
    return 1;
}

/* Mark process stautus.  */
static int __clish_mark_process_status (pid_t pid, int status)
{
    clish_process_t *proc;

    TAILQ_FOREACH(proc, &clish_process_hdr, entry) {
        if (pid == proc->pid) {
            proc->status = status;

            if (!WIFSTOPPED(status)) {
                proc->completed = 1;
            }
            return 0;
        }
    }
    
    return -1;
}

/* Signale wrapper. */
void *__signal_set (int signo, signal_handler_fn_t *func)
{
    int ret;
    struct sigaction sig;
    struct sigaction osig;

    sig.sa_handler = func;
    sigemptyset (&sig.sa_mask);
    sig.sa_flags = 0;
#ifdef SA_RESTART
    sig.sa_flags |= SA_RESTART;
#endif

    ret = sigaction (signo, &sig, &osig);

    if (ret < 0) {
        return (SIG_ERR);
    }
    
    return (osig.sa_handler);
}

/* Exec child process.  */
static void __clish_exec_process (clish_process_t *proc, int in_fd, int out_fd, int err_fd)
{
    /* Set the handler to the default.  */
    __signal_set (SIGINT, SIG_DFL);
    __signal_set (SIGQUIT, SIG_DFL);
    __signal_set (SIGCHLD, SIG_DFL);
    __signal_set (SIGALRM, SIG_DFL);
    __signal_set (SIGPIPE, SIG_DFL);
    __signal_set (SIGUSR1, SIG_DFL);

    /* No job control.  */
    __signal_set (SIGTSTP, SIG_IGN);
    __signal_set (SIGTTIN, SIG_IGN);
    __signal_set (SIGTTOU, SIG_IGN);

    /* connect standard input, output and error */
    if (in_fd != 0) {
        if (dup2 (in_fd, 0) < 0) {
            exit (1);
        }
        close (in_fd);
    }
    
    if (out_fd != 1) {
        if (dup2 (out_fd, 1) < 0) {
          exit (1);
        }
        close (out_fd);
    }

    if (err_fd != 2) {
        if (dup2 (err_fd, 2) < 0)  {
            exit (1);
        }
        close (err_fd);
    }

    /* Internal process.  */
    if (proc->pCmdFunc) {
        int ret = proc->pCmdFunc(proc->pArg, 0);
        exit (ret);
    }

    execvp (proc->argv[0], (char **)proc->argv);

    /* Not reached her unless error occurred.  */
    perror ("execvp");
    exit (1);
}

static void __clish_wait_all_process ()
{
    pid_t pid;
    int status;

    do {
        pid = waitpid (-1, &status, WUNTRACED);
    } while (__clish_mark_process_status (pid, status) == 0 
             && __clish_process_completed () == 0);

    if ((WEXITSTATUS(status) & 0xff) == 17) {
        /* more or other prog timed out */
    }

    __clish_pipe_free ();
    return;
}

/* Run pipe lines */
static void __clish_pipe_line ()
{
    int in_fd;
    int out_fd;
    int err_fd;
    int pipe_fd[2];
    int ret;
    pid_t pid;
    clish_process_t *proc;
    int bFirst = 1;
    signal_handler_fn_t *old_sigint;
    signal_handler_fn_t *old_sigtstp;

    in_fd = 0;
    err_fd = 2;

    /* ignore SIGINT and SIGTSTP */
    old_sigint = __signal_set (SIGINT, SIG_IGN);
    old_sigtstp = __signal_set (SIGTSTP, SIG_IGN);

    TAILQ_FOREACH(proc, &clish_process_hdr, entry) {
        /* create pipe */
        if (TAILQ_NEXT(proc, entry)) {
            ret = pipe (pipe_fd);
            if (ret < 0) {
                perror ("pipe");
      	        if (1 != bFirst) {
         	        close(in_fd);
      		        break;
      	        }
                return;
            }
            out_fd = pipe_fd[1];
        } else {
            /* Redirection of output.  */
            if (proc->filename) {
                char szAccessName[PATH_MAX];
                int _ret = gen_check_and_get_filename(proc->filename, szAccessName, PATH_MAX);
                if (0 != _ret) {
                    _ret = gen_check_and_get_filename_bycwd(proc->filename, szAccessName, PATH_MAX);
                }

                if (0 != _ret) {
                    fprintf (stderr, "%% Invalid output file name %s\n", proc->filename);
                    if (1 != bFirst) {
           		        close(in_fd);
          		        break;
          	        }
                    goto err_out;
                }

                if (0 != gen_file_proc_confirm("overwrite", szAccessName, proc->filename)) {
                    if (1 != bFirst) {
           		        close(in_fd);
          		        break;
          	        }
                    goto err_out;                    
                }

                out_fd = open (szAccessName, O_WRONLY | O_CREAT | O_TRUNC, 0600);
                if (out_fd < 0) {
                    fprintf (stderr, "%% Can't open output file %s: %s\n", 
                            proc->filename, strerror(errno));
                    if (1 != bFirst) {
           		        close(in_fd);
          		        break;
          	        }
                    goto err_out;
                }
    	    } else {
                out_fd = 1;
            }
        }
 

        /* fork */
        pid = fork ();
        if (pid < 0) {
            perror ("fork");
            if (1 != bFirst) {
                close(in_fd);
      	        break;
            }
            return;
        }

        bFirst++;
        __signal_set (SIGINT, SIG_IGN);
        __signal_set (SIGTSTP, SIG_IGN);

        if (pid == 0) {
            /* incoming fd is not used by child process */
            if (TAILQ_NEXT(proc, entry)) {
                close (pipe_fd[0]);
            }

            /* this is child process */
            __clish_exec_process (proc, in_fd, out_fd, err_fd);
        } else {
            /* parent process */
            proc->pid = pid;
        }
        
        if (in_fd != 0) {
            close (in_fd);
        }
        if (out_fd != 1) {
            close (out_fd);
        }
        in_fd = pipe_fd[0];
    }

err_out:
    /* Wait for all of child processes.  */
    __clish_wait_all_process ();

    __signal_set (SIGINT, old_sigint);
    __signal_set (SIGTSTP, old_sigtstp);
    return;
}

/* execute command in child process */
int __clish_exec (char *command, ...)
{
    va_list args;
    int argc;
    const char **argv;
    int ret = -1;

    /* check argument numbers */
    va_start (args, command);
    for (argc = 1; va_arg (args, char *) != NULL; ++argc) {
        continue;
    }
    va_end (args);

    argv = calloc (1, (argc + 1) * sizeof (*argv));
    if (!argv) {
        return -1;
    }

    /* generate argument list */
    argv[0] = command;
    va_start (args, command);
    for (argc = 1; (argv[argc] = va_arg (args, char *)) != NULL; ++argc) {
        continue;
    }
    va_end (args);

    /* generate process list */ 
    if (NULL == __clish_pipe_add (NULL, NULL, argv, NULL)) {
        goto err_out;
    }

    /* execute commands */
    __clish_pipe_line();
    ret = 0;
    
err_out:
    free (argv);
    return ret;
}

/* execute command in child process */
int __clish_execvp (const char **argv)
{
    clish_process_t *proc;

    proc = __clish_pipe_add (NULL, NULL, argv, NULL);
    if (!proc) {
        return -1;
    }

    /* execute command */
    __clish_pipe_line (proc);
    return 0;
}

char *clish_get_timeout_str(void)
{
    static char g_timeout_str[128];
    
    snprintf(g_timeout_str, 128, "@%d", 300); /* XXX */
    return g_timeout_str;
}

int __clish_more (command_fn_t *pCmdFunc, void *pArg)
{
    const char *argv_more[3];

    /* Initialize more array. */
    argv_more[0] = M_PAGER_PROG;
    argv_more[1] = clish_get_timeout_str();
    argv_more[2] = NULL;

    /* action */
    if (NULL == __clish_pipe_add(pCmdFunc, pArg, NULL, NULL)) {
        return -1;
    }

    if (g_term_lines) {
        /* more */
        if (NULL == __clish_pipe_add(NULL, NULL, argv_more, NULL)) {
            __clish_pipe_free();
            return -1;
        }
    }

    __clish_pipe_line ();
    return 0;
}

static int __clish_begin (command_fn_t *pCmdFunc, void *pArg, char *regexp)
{
    const char *argv_more[4];
    char *option = NULL;

    /* Prepare string for "more +/regexp".  */
    option = calloc (1, strlen ("+/") + strlen (regexp) + 1);
    if (NULL == option) {
        return -1;
    }
    strcat (option, "+/");
    strcat (option, regexp);

    argv_more[0] = M_PAGER_PROG;
    argv_more[1] = clish_get_timeout_str();    
    argv_more[2] = option;
    argv_more[3] = NULL;
    
    /* action */
    if (NULL == __clish_pipe_add(pCmdFunc, pArg, NULL, NULL)) {
        return -1;
    }

    /* more */
    if (NULL == __clish_pipe_add(NULL, NULL, argv_more, NULL)) {
        goto err_out;
    }

    __clish_pipe_line ();
    free (option);
    return 0;
    
err_out:
    free (option);
    __clish_pipe_free();
    return -1;
}

static int __clish_include (command_fn_t *pCmdFunc, void *pArg, char *regexp, int bInclude)
{
    const char *argv_grep[4];
    const char *argv_more[3];

    argv_grep[0] = "egrep";
    if (bInclude) {
        argv_grep[1] = regexp;
        argv_grep[2] = NULL;
    } else {
        argv_grep[1] = "-v";
        argv_grep[2] = regexp;
    }
    argv_grep[3] = NULL;

    argv_more[0] = M_PAGER_PROG;
    argv_more[1] = clish_get_timeout_str();
    argv_more[2] = NULL;

    /* action */
    if (NULL == __clish_pipe_add(pCmdFunc, pArg, NULL, NULL)) {
        return -1;
    }

    /* grep */
    if (NULL == __clish_pipe_add(NULL, NULL, argv_grep, NULL)) {
        goto err_out;
    }

    if (g_term_lines) {
        /* more */
        if (NULL == __clish_pipe_add(NULL, NULL, argv_more, NULL)) {
            goto err_out;
        }
    }

    __clish_pipe_line ();
    return 0;

err_out:
    __clish_pipe_free();
    return -1;
}

static int __clish_redirect (command_fn_t *pCmdFunc, void *pArg, char *filename)
{
    /* action */
    if (NULL == __clish_pipe_add(pCmdFunc, pArg, NULL, filename)) {
        return -1;
    }

    __clish_pipe_line ();
    return 0;
}

static int __script_exec(void *pArg, int bFree)
{
    int ret;
    const char *pCmd = (const char *)pArg;
    
    if (bFree) {
        return 0;
    }

    ret = system(pCmd);
    return WEXITSTATUS(ret);
}

#if 1 /* fix bug 42193 add by wangqj in 2017/1/5 */
typedef struct parse_cmd_t {
    char **argv;
    int argc;
} parse_cmd_t;

enum cmd_type_t {
    CMD_TYPE_SOLO,
    CMD_TYPE_PAIR
};

struct cmd_table_t {
    int length;
    char *str;
    enum cmd_type_t type;
    char *value;
};

#define REPLACE_STR_WITH_VALUE(cmd_table_index) do { \
    strcat(arg_tmp, cmd_table[cmd_table_index].value); \
    p += cmd_table[cmd_table_index].length - 1; \
    next = 1; \
} while(0)

#define ARG_TMP_MAX_LENGTH (2 * 1024 * 1024)

static struct cmd_table_t cmd_table[] = {
    {2, "\\\"", CMD_TYPE_SOLO, "\""},
    {2, "\\'",  CMD_TYPE_SOLO, "'" },
    {2, "\\ ",  CMD_TYPE_SOLO, " " },
    {1, "\"",   CMD_TYPE_PAIR, ""  },
    {1, "'",    CMD_TYPE_PAIR, ""  }
};

static void _push_argv(parse_cmd_t *pcmd, const char *str)
{
    pcmd->argc++;

    if (NULL == pcmd->argv) {
        pcmd->argv = (char **)malloc(sizeof(char *)); 
    }
    else {
        pcmd->argv = (char **)realloc(pcmd->argv, sizeof(char *) * pcmd->argc); 
    }

    if (str != NULL) {
        pcmd->argv[pcmd->argc - 1] = (char *)malloc(strlen(str) + 1);
        strcpy(pcmd->argv[pcmd->argc - 1], str);
    }
    else {
        pcmd->argv[pcmd->argc - 1] = NULL;
    }
}

/**
 * parse string to argv
 * for example:
 * a b "c d"  e -> "a" "b" "c d" "e" NULL
 *                  |   |    |    |
 *       argv[0] __/    |    |    | 
 *       argv[1] ______/     |    |
 *       argv[2] ___________/     |
 *       argv[3] ________________/
 * @return
 *    if parse failed, return -1, else return 0
 */
void parse_cmd_to_argv(const char *command, parse_cmd_t *pcmd)
{
    const char *p;
    char *arg_tmp = NULL;
    int expect_index = -1;
    int i;
    int next = 0;

    memset(pcmd, 0, sizeof(parse_cmd_t));

    if (strlen(command) <= 0) return;

    if (strlen(command) >= ARG_TMP_MAX_LENGTH) {
        printf("arg length exceed %d!\n", ARG_TMP_MAX_LENGTH);
        return;
    }

    arg_tmp = (char *)malloc(ARG_TMP_MAX_LENGTH);
    if (NULL == arg_tmp) return;
    *arg_tmp = 0;

    for(p = command; *p != 0; p++) {
        if (' ' == *p && -1 == expect_index) {
            if (*arg_tmp) { /* push argv */
                _push_argv(pcmd, arg_tmp);
                *arg_tmp = 0;
            }
            continue;
        }
        for (i = 0; i < sizeof(cmd_table) / sizeof(struct cmd_table_t); i++) {
            if (0 == strncmp(cmd_table[i].str, p, cmd_table[i].length)) {
                switch(cmd_table[i].type) {
                    case CMD_TYPE_SOLO:
                        REPLACE_STR_WITH_VALUE(i);
                        break;
                    case CMD_TYPE_PAIR:
                        if (-1 == expect_index) {
                            expect_index = i;
                            REPLACE_STR_WITH_VALUE(i);
                        }
                        else {
                            if (i == expect_index) {
                                expect_index = -1;
                                REPLACE_STR_WITH_VALUE(i);
                            }
                        }
                        break;
                    default:
                        break;
                }
                continue;
            }
        }

        if (next) {
            next = 0;
            continue;
        }

        strncat(arg_tmp, p, 1);
    }

    if (*arg_tmp) _push_argv(pcmd, arg_tmp);

    _push_argv(pcmd, NULL);

    free(arg_tmp);
}

void free_cmd_to_argv(parse_cmd_t *pcmd)
{
    int i;

    for (i = 0; i < pcmd->argc; i++) {
        free(pcmd->argv[i]);
    }

    free(pcmd->argv);
}

void catch_ctrl_c(int s) 
{
}

static int execute_ovs_command(const char *command)
{
    int pid = 0;
    int status = 0;
    parse_cmd_t pcmd;

    parse_cmd_to_argv(command, &pcmd);

    pid = fork();
    if (-1 == pid) {
        printf("cannot fork new process!");
        return -1;
    }
    if (0 == pid) { // execute ovs command
        exit(execvp(*pcmd.argv, pcmd.argv));
    }
    else {
        signal(SIGINT, catch_ctrl_c);
        while((pid = wait(&status)) > 0);
        free_cmd_to_argv(&pcmd);
        return 0;
    }
}
#endif

#define STARTUP_SPEEDUP
#define CDBCTL_STR      "cdbctl "
#define CDBCTL_STR_LEN  7

extern int isStartupCfg();
extern int isCdbClientOK();
extern int
cdbclt_process_cmd(char *raw_path);

int
ctc_exec_cmd(char *command)
{
    char *new_command = NULL;

    new_command = strstr(command, CDBCTL_STR);
    new_command += CDBCTL_STR_LEN;

    return cdbclt_process_cmd(new_command);
}

static int g_first_enter_agg = 1;
static int g_in_system = 0;

void ctc_kill_child()
{
    char szcmd[512];

    if (!g_in_system) {
        return;
    }

    snprintf(szcmd, 512,
        "pstree -p %d | sed -e 's/---/\\n/g' |"
        " sed 's/(/ /g' | sed -e 's/)//g' |"
        " grep -v klish | awk '{print \"kill -9 \"$2\" 2>/dev/null\"}' | sh",
        getpid());
#if 0
    {
        FILE *fp = fopen("/tmp/xxx", "w");
        fwrite(szcmd, strlen(szcmd), 1, fp);
        fclose(fp);
    }
#endif
    system(szcmd);
    return;
}

int
ctc_system(char *command)
{
    int ret;
    g_in_system = 1;
    
#ifdef STARTUP_SPEEDUP
    int isStartup = isStartupCfg();
    int isCdbClient = isCdbClientOK();

    if (isStartup)
    {
        /* Added by kcao 2017-12-12 for bug 45841, wait 3s for first enter LAG interface */
        if (g_first_enter_agg)
        {
            if (strstr(command, "create/cdb/interface/agg"))
            {
                sleep(3);
                g_first_enter_agg = 0;
            }
        }
        if (isCdbClient)
        {
            if (0 == strncmp(command, CDBCTL_STR, CDBCTL_STR_LEN))
            {
                g_in_system = 0;
                return ctc_exec_cmd(command);
            }
        }
    }
#endif /* !STARTUP_SPEEDUP */

    ret = system(command);
    g_in_system = 0;
    return ret;
}

/*--------------------------------------------------------- */
int clish_script_callback(clish_context_t *context,
	clish_action_t *action, const char *script, char **out)
{
	clish_shell_t *this = context->shell;
	const char * shebang = NULL;
	pid_t cpid = -1;
	int res;
	const char *fifo_name;
	FILE *rpipe, *wpipe;
	char *command = NULL;
	bool_t is_sh = BOOL_FALSE;

	/* Signal vars */
	struct sigaction sig_old_int;
	struct sigaction sig_old_quit;
	struct sigaction sig_new;
	sigset_t sig_set;

	assert(this);
	if (!script) /* Nothing to do */
		return BOOL_TRUE;

	/* Find out shebang */
	if (action)
		shebang = clish_action__get_shebang(action);
	if (!shebang)
		shebang = clish_shell__get_default_shebang(this);
	assert(shebang);
	if (0 == lub_string_nocasecmp(shebang, "/bin/sh"))
		is_sh = BOOL_TRUE;

#ifdef DEBUG
	fprintf(stderr, "SHEBANG: #!%s\n", shebang);
	fprintf(stderr, "SCRIPT: %s\n", script);
#endif /* DEBUG */

	/* If /bin/sh we don't need FIFO */
	if (!is_sh) {
		/* Get FIFO */
		fifo_name = clish_shell__get_fifo(this);
		if (!fifo_name) {
			fprintf(stderr, "System error. Can't create temporary FIFO.\n"
				"The ACTION will be not executed.\n");
			return BOOL_FALSE;
		}

		/* Create process to write to FIFO */
		cpid = fork();
		if (cpid == -1) {
			fprintf(stderr, "System error. Can't fork the write process.\n"
				"The ACTION will be not executed.\n");
			return BOOL_FALSE;
		}

		/* Child: write to FIFO */
		if (cpid == 0) {
			wpipe = fopen(fifo_name, "w");
			if (!wpipe)
				_exit(-1);
			fwrite(script, strlen(script) + 1, 1, wpipe);
			fclose(wpipe);
			_exit(0);
		}
	}

	/* Parent */
	/* Prepare command */
	if (!is_sh) {
		lub_string_cat(&command, shebang);
		lub_string_cat(&command, " ");
		lub_string_cat(&command, fifo_name);
	} else {
		lub_string_cat(&command, script);
	}

	/* If the stdout of script is needed */
	if (out) {
		konf_buf_t *buf;

		/* Ignore SIGINT and SIGQUIT */
		sigemptyset(&sig_set);
		sig_new.sa_flags = 0;
		sig_new.sa_mask = sig_set;
		sig_new.sa_handler = SIG_IGN;
		sigaction(SIGINT, &sig_new, &sig_old_int);
		sigaction(SIGQUIT, &sig_new, &sig_old_quit);

		/* Execute shebang with FIFO as argument */
		rpipe = popen(command, "r");
		if (!rpipe) {
			fprintf(stderr, "System error. Can't fork the script.\n"
				"The ACTION will be not executed.\n");
			lub_string_free(command);
			if (!is_sh) {
				kill(cpid, SIGTERM);
				waitpid(cpid, NULL, 0);
			}

			/* Restore SIGINT and SIGQUIT */
			sigaction(SIGINT, &sig_old_int, NULL);
			sigaction(SIGQUIT, &sig_old_quit, NULL);

			return BOOL_FALSE;
		}
		/* Read the result of script execution */
		buf = konf_buf_new(fileno(rpipe));
		while (konf_buf_read(buf) > 0);
		*out = konf_buf__dup_line(buf);
		konf_buf_delete(buf);
		/* Wait for the writing process */
		if (!is_sh) {
			kill(cpid, SIGTERM);
			waitpid(cpid, NULL, 0);
		}
		/* Wait for script */
		res = pclose(rpipe);

		/* Restore SIGINT and SIGQUIT */
		sigaction(SIGINT, &sig_old_int, NULL);
		sigaction(SIGQUIT, &sig_old_quit, NULL);
	} else {
        tinyrl_t *trl = clish_shell__get_tinyrl(this);
        const char *line = trl ? tinyrl__get_last_buffer(trl) : NULL;
	    if (NULL != trl 
                && NULL != line 
                && clish_is_show_cmd(line)) {
            char *p = strchr(line, '|');
            if (NULL == p) {
                p = strchr(line, '>');
            }

            if (NULL != p) {
                char szModifier[1024];
                lub_argv_t *pModArg = lub_argv_new(NULL, 0);

                assert(NULL != pModArg);
                snprintf(szModifier, 1024, "%s", p);
                clish_check_modifier_pargs(pModArg, szModifier);
                if (0 != lub_argv__get_argc(pModArg))
                {
                    if (!strcasecmp(lub_argv__get_arg(pModArg, 0), "begin")) {
                        __clish_begin(__script_exec, command, 
                                (char *)lub_argv__get_arg(pModArg, 1));
                    } else if (!strcasecmp(lub_argv__get_arg(pModArg, 0), "exclude")) {
                        __clish_include(__script_exec, command, 
                                (char *)lub_argv__get_arg(pModArg, 1), 0);
                    } else if (!strcasecmp(lub_argv__get_arg(pModArg, 0), "include")) {
                        __clish_include(__script_exec, command, 
                                (char *)lub_argv__get_arg(pModArg, 1), 1);
                    } else if (!strcasecmp(lub_argv__get_arg(pModArg, 0), "redirect")) {
                        __clish_redirect(__script_exec, command, (char *)lub_argv__get_arg(pModArg, 1));
                    } else {
                        __clish_redirect(__script_exec, command, (char *)lub_argv__get_arg(pModArg, 0));
                    }
                }
                else
                {
                    fprintf(stderr, "%% Syntax error: Illegal command line\n");
                }
                lub_argv_delete(pModArg);
            } else {
                __clish_more(__script_exec, command);
            }
            res = 0;
        } else {
#if 1 /* fix bug 42193 add by wangqj in 2017/1/5 */
            if (!strcasecmp(clish_command__get_name(context->cmd), "ovs-ofctl"))
            {
                res = execute_ovs_command(command);
            }
            else
            {
                /* load startup-config speedup */
                res = ctc_system(command);
            }
#else
            res = system(command);
#endif
        }

		/* Wait for the writing process */
		if (!is_sh) {
			kill(cpid, SIGTERM);
			waitpid(cpid, NULL, 0);
		}
	}
	lub_string_free(command);

#ifdef DEBUG
	fprintf(stderr, "RETCODE: %d\n", WEXITSTATUS(res));
#endif /* DEBUG */
	return WEXITSTATUS(res);
}

/*--------------------------------------------------------- */
int clish_dryrun_callback(clish_context_t *context,
	clish_action_t *action, const char *script, char ** out)
{
#ifdef DEBUG
	fprintf(stderr, "DRY-RUN: %s\n", script);
#endif /* DEBUG */
	if (out)
		*out = NULL;

	return 0;
}

/*--------------------------------------------------------- */
