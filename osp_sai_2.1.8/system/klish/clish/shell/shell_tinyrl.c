/*
 * shell_tinyrl.c
 *
 * This is a specialisation of the tinyrl_t class which maps the readline
 * functionality to the CLISH environment.
 */
#include "private.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <net/if.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/stat.h>

#include "tinyrl/tinyrl.h"
#include "tinyrl/history.h"

#include "lub/string.h"
#include "sal.h"

#define TRIM_CHAR(C)  ((C)=='\t' || (C)==' ' || (C)=='\r' || (C)=='\n')

extern int glb_pri;
extern int isStartupCfg();

static void clish_shell_trim_hostname(char *hostname)
{
    char* p = hostname;

    /*trim left space*/
    while (*p != '\0')
    {
        if (TRIM_CHAR(*p))
        {
            *p = '\0';
            break;
        }
        p++;
    }
    return;
}

static void clish_shell_get_hostname(char *hostname, int size)
{
    FILE* hn_fp = NULL;
    memset(hostname, 0, size);
    hn_fp = fopen("/proc/sys/kernel/hostname", "r");
    if (NULL == hn_fp)
    {
        hostname[0] = 0;
        return;
    }
    fgets(hostname, size, hn_fp);
    fclose(hn_fp);
    clish_shell_trim_hostname(hostname);
    return;
}

/*-------------------------------------------------------- */
static void clish_shell_renew_prompt(clish_shell_t *this)
{
	clish_context_t prompt_context;
	char *prompt = NULL;
	const clish_view_t *view;
	char *str = NULL;
    char hostname[64];
    char prompt_buf[16];

	/* Create appropriate context */
	memset(&prompt_context, 0, sizeof(prompt_context));
	prompt_context.shell = this;

	/* Obtain the prompt */
	view = clish_shell__get_view(this);
	assert(view);
    clish_shell_get_hostname(hostname, 64);
    lub_string_cat(&str, hostname);
	//lub_string_cat(&str, "${_PROMPT_PREFIX}");
    if (0 == strcmp(clish_view__get_name(view), "Exec"))
    {
        strncpy(prompt_buf, clish_view__get_prompt(view), 16);
        if (1 == glb_pri)
        {
            prompt_buf[0] = '>';
        }
        lub_string_cat(&str, prompt_buf);
    }
    else
    {
        lub_string_cat(&str, clish_view__get_prompt(view));
    }
	lub_string_cat(&str, "${_PROMPT_SUFFIX}");
	prompt = clish_shell_expand(str, SHELL_VAR_NONE, &prompt_context);
	assert(prompt);
	lub_string_free(str);
	tinyrl__set_prompt(this->tinyrl, prompt);
	lub_string_free(prompt);
}

/*-------------------------------------------------------- */
//#define DEBUG_HELP
#ifndef DEBUG_HELP
extern int __clish_more (command_fn_t *pCmdFunc, void *pArg);
static int __help_more_func(void *pArg, int bFree)
{
    if (!bFree) {
        tinyrl_t * this = (tinyrl_t *) pArg;
    	/* get the context */
    	clish_context_t *context = tinyrl__get_context(this);
    	clish_shell_help(context->shell, tinyrl__get_line(this));
    }
    return 0;
}
#endif

static bool_t clish_shell_tinyrl_key_help(tinyrl_t * this, int key)
{
	bool_t result = BOOL_TRUE;

	if (tinyrl_is_quoting(this)) {
		/* if we are in the middle of a quote then simply enter a space */
		result = tinyrl_insert_text(this, "?");
	} else {
        /* Added by Alexander */
        const char *pline = tinyrl__get_line(this);
        if (NULL == pline || !strcasecmp(pline, "")) {
            printf("?");
	    }
	
#ifndef DEBUG_HELP
    	tinyrl_crlf(this);
        extern void tty_set_raw_mode(tinyrl_t * this);
        extern void tty_restore_mode(tinyrl_t * this);

        tty_restore_mode(this);
		__clish_more(__help_more_func, this);
        tty_set_raw_mode(this);
        tinyrl_crlf(this);
    	tinyrl_reset_line_state(this);
#else
    	/* get the context */
    	clish_context_t *context = tinyrl__get_context(this);
    	
    	tinyrl_crlf(this);
		clish_shell_help(context->shell, tinyrl__get_line(this));
        tinyrl_crlf(this);
    	tinyrl_reset_line_state(this);
#endif
	}
	/* keep the compiler happy */
	key = key;

	return result;
}

/*lint +e818 */
/*-------------------------------------------------------- */
/*
 * Expand the current line with any history substitutions
 */
static clish_pargv_status_t clish_shell_tinyrl_expand(tinyrl_t * this)
{
	clish_pargv_status_t status = CLISH_LINE_OK;
	int rtn;
	char *buffer;

	return status;
	/* first of all perform any history substitutions */
	rtn = tinyrl_history_expand(tinyrl__get_history(this),
		tinyrl__get_line(this), &buffer);

	switch (rtn) {
	case -1:
		/* error in expansion */
		status = CLISH_BAD_HISTORY;
		break;
	case 0:
		/*no expansion */
		break;
	case 1:
		/* expansion occured correctly */
		tinyrl_replace_line(this, buffer, 1);
		break;
	case 2:
		/* just display line */
		tinyrl_printf(this, "\n%s", buffer);
		free(buffer);
		buffer = NULL;
		break;
	default:
		break;
	}
	free(buffer);

	return status;
}

/*-------------------------------------------------------- */
/*
 * This is a CLISH specific completion function.
 * If the current prefix is not a recognised prefix then
 * an error is flagged.
 * If it is a recognisable prefix then possible completions are displayed
 * or a unique completion is inserted.
 */
static tinyrl_match_e clish_shell_tinyrl_complete(tinyrl_t * this)
{
/*	context_t *context = tinyrl__get_context(this); */
	tinyrl_match_e status;

	/* first of all perform any history expansion */
	(void)clish_shell_tinyrl_expand(this);
	/* perform normal completion */
	status = tinyrl_complete(this);
	switch (status) {
	case TINYRL_NO_MATCH:
		if (BOOL_FALSE == tinyrl_is_completion_error_over(this)) {
			/* The user hasn't even entered a valid prefix! */
/*			tinyrl_crlf(this);
			clish_shell_help(context->shell,
				tinyrl__get_line(this));
			tinyrl_crlf(this);
			tinyrl_reset_line_state(this);
*/		}
		break;
	default:
		/* the default completion function will have prompted for completions as
		 * necessary
		 */
		break;
	}
	return status;
}

static bool_t clish_shell_tinyrl_key_space(tinyrl_t * this, int key)
{
	bool_t result = BOOL_FALSE;
	tinyrl_match_e status;
	clish_context_t *context = tinyrl__get_context(this);
	const char *line = tinyrl__get_line(this);
	clish_pargv_status_t arg_status;
	const clish_command_t *cmd = NULL;
	clish_pargv_t *pargv = NULL;

    /* XXX: bypass input check for ovs utilities. */
    if (!strncmp(line, "ovs-ofctl", 9) || !strncmp(line, "ovs-vsctl", 9) ||
        !strncmp(line, "ovs-appctl", 10))
    {
    	tinyrl_insert_text(this, " ");

    	/* keep compiler happy */
    	key = key;

    	return result;        
    }
    
	if(tinyrl_is_empty(this)) {
		/* ignore space at the begining of the line, don't display commands */
		return BOOL_TRUE;
	} else if (tinyrl_is_quoting(this)) {
		/* if we are in the middle of a quote then simply enter a space */
		result = BOOL_TRUE;
	} else {
		/* Find out if current line is legal. It can be
		 * fully completed or partially completed.
		 */
		arg_status = clish_shell_parse(context->shell,
			line, &cmd, &pargv);
		if (pargv)
			clish_pargv_delete(pargv);
		switch (arg_status) {
		case CLISH_LINE_OK:
		case CLISH_LINE_PARTIAL:
			if (' ' != line[strlen(line) - 1])
				result = BOOL_TRUE;
			break;
		default:
			break;
		}
		/* If current line is illegal try to make auto-comletion. */
		if (!result) {
			/* perform word completion */
			status = clish_shell_tinyrl_complete(this);
			switch (status) {
			case TINYRL_NO_MATCH:
			case TINYRL_AMBIGUOUS:
				/* ambiguous result signal an issue */              
				break;
			case TINYRL_COMPLETED_AMBIGUOUS:
				/* perform word completion again in case we just did case
				   modification the first time */
				status = clish_shell_tinyrl_complete(this);
				if (status == TINYRL_MATCH_WITH_EXTENSIONS) {
					/* all is well with the world just enter a space */
					result = BOOL_TRUE;
				}
				break;
			case TINYRL_MATCH:
			case TINYRL_MATCH_WITH_EXTENSIONS:
			case TINYRL_COMPLETED_MATCH:
				/* all is well with the world just enter a space */
				result = BOOL_TRUE;
				break;
			}
		}
	}

	if (result)
		result = tinyrl_insert_text(this, " ");

	/* keep compiler happy */
	key = key;

	return result;
}

/* for bug 35422 */
static char* ctc_clish_get_slash_err_msg(const char *line)
{
    char word[10000];
    bool_t slash_in_intf_word = BOOL_FALSE;
    int p_pos = 0;
    int p_word_start = 0;
    int word_len = 0;
    int i = 0;
    char *p = NULL;

    p = strchr(line, '/');
    p_pos = (p - line);
    p_word_start = p_pos;
    
    while (p_word_start > 0)
    {
        if (line[p_word_start] == ' ')
        {
            break;
        }
        p_word_start--;
    }

    p_word_start++;
    word_len = p_pos - p_word_start;
    for (i = 0; i < word_len; i++)
    {
        word[i] = line[p_word_start+i];
    }
    word[i] = '\0';

    if (0 == strncmp("eth-", word, 4))
    {
        slash_in_intf_word = BOOL_TRUE;
    }

    if (slash_in_intf_word)
    {
        return "Illegal parameter";
    }
    else
    {
        return "Illegal char '/'";
    }
}

/*-------------------------------------------------------- */
#define M_SCP_PREFIX    "scp -"
static bool_t clish_shell_tinyrl_key_enter(tinyrl_t *this, int key)
{
	clish_context_t *context = tinyrl__get_context(this);
	const clish_command_t *cmd = NULL;
	const char *line = tinyrl__get_line(this);
	bool_t result = BOOL_FALSE;
	bool_t invalid_cmd = BOOL_FALSE;
	char *errmsg = NULL;
	clish_pargv_status_t arg_status = CLISH_LINE_OK;

	/* Inc line counter */
	if (context->shell->current_file)
		context->shell->current_file->line++;

	/* Renew prompt */
	clish_shell_renew_prompt(context->shell);

	/* nothing to pass simply move down the screen */
	if (!*line) {
		tinyrl_multi_crlf(this);
		tinyrl_done(this);
		return BOOL_TRUE;
	}

    /* XXX: bypass all klish cli resolving and arg processing, will
       delegate command syntax to the ovs utilities. */
    if ((!strncmp(line, "ovs-ofctl", 9) || 
          !strncmp(line, "ovs-vsctl", 9) || 
          !strncmp(line, "ovs-appctl", 10)) &&
        !strcmp(clish_view__get_name(clish_shell__get_view(context->shell)), "Exec"))
    {
        /* OVS utilities command share EXACTLY the same hehavior, just
           use the "ovs-ofctl" cmd to cheat context. */
        context->cmd = clish_view_find_command(clish_shell__get_view(context->shell),
                                               "ovs-ofctl", BOOL_FALSE);
        if (NULL == context->cmd)
        {
            return BOOL_FALSE;
        }
        
        /* Put to whole line into the pargv which will be used in shell execution,
           use the command's unique param to cheat . */
        context->pargv = clish_pargv_new();
        clish_pargv_insert(context->pargv, clish_command__get_param(context->cmd, 0), line); 

        tinyrl_multi_crlf(this);
        tinyrl_done(this); 
        
        /* Always pass the check. */
    	return BOOL_TRUE;        
    }

	/* try and parse the command */
	cmd = clish_shell_resolve_command(context->shell, line);
	if (!cmd) {
		tinyrl_match_e status = clish_shell_tinyrl_complete(this);
		switch (status) {
		case TINYRL_MATCH:
		case TINYRL_MATCH_WITH_EXTENSIONS:
		case TINYRL_COMPLETED_MATCH:
			/* re-fetch the line as it may have changed
			 * due to auto-completion
			 */
			line = tinyrl__get_line(this);
			/* get the command to parse? */
			cmd = clish_shell_resolve_command(context->shell, line);
			/*
			 * We have had a match but it is not a command
			 * so add a space so as not to confuse the user
			 */
			if (!cmd)
				result = tinyrl_insert_text(this, " ");
			break;
		case TINYRL_NO_MATCH:
            invalid_cmd = BOOL_TRUE;
			break;
		default:
			/* failed to get a unique match... */
			if (!tinyrl__get_isatty(this)) {
				/* batch mode */
				tinyrl_multi_crlf(this);
				errmsg = "Unknown command";
			}
			break;
		}
	}
	if (cmd) {
		tinyrl_multi_crlf(this);
		/* we've got a command so check the syntax */
        if (!clish_is_show_cmd(line)) {
    		arg_status = clish_shell_parse(context->shell,
    			line, &context->cmd, &context->pargv);
        } else {
            char cbuf[1024];
            char modifier[1024];
            char *p = NULL;
            clish_pargv_status_t modifier_status = CLISH_LINE_OK;

            /* split command line */
            modifier[0] = '\0';
            if ((p = strrchr(line, '|')) || (p = strrchr(line, '>'))) {
                memcpy(cbuf, line, (p - line));
                cbuf[p - line] = '\0';
                snprintf(modifier, 1024, "%s", p);
        		arg_status = clish_shell_parse(context->shell,
        			cbuf, &context->cmd, &context->pargv);
                if (CLISH_LINE_OK == arg_status) {
                    modifier_status = clish_check_modifier_pargs(NULL, modifier);
                    if (CLISH_LINE_OK != modifier_status) {
                        clish_pargv_delete(context->pargv);
                        context->pargv = NULL;
                        arg_status = modifier_status;
                    }
                }
            } else {
        		arg_status = clish_shell_parse(context->shell,
        			line, &context->cmd, &context->pargv);
            }
        }
        switch (arg_status) {
		case CLISH_LINE_OK:
			tinyrl_done(this);
			result = BOOL_TRUE;
			break;
		case CLISH_BAD_HISTORY:
			errmsg = "Bad history entry";
			break;
		case CLISH_BAD_CMD:
			errmsg = "Illegal command line";
			break;
		case CLISH_BAD_PARAM:
            if (line && strchr(line, '/'))
            {
                if ((strlen(line) >= 9) && (0 == strncmp(line, "interface", 9)))
                {
                    invalid_cmd = BOOL_TRUE;
                }
                else
                {
                    errmsg = ctc_clish_get_slash_err_msg(line);
                }
            }
            else
            {
                errmsg = "Illegal parameter";
            }
            break;
		case CLISH_LINE_PARTIAL:
			errmsg = "The command is not completed";
			break;
		default:
			errmsg = "Unknown problem";
			break;
		}
	}

    if (invalid_cmd && !strncmp(line, M_SCP_PREFIX, strlen(M_SCP_PREFIX))) {
        /* it's scp command from sshd */
        char szAccName[PATH_MAX];
        char szScpCmd[PATH_MAX];
        char *pShowName = strrchr(line, ' ') + 1;
        
        if (sal_cmd_exec(szAccName, PATH_MAX, 
                "fnconvert -p \"%s\" -c accessname 2>/dev/null", 
                pShowName) != 0) {
            fprintf(stderr, "%% Invalid path: %s\n", pShowName);
            return result;
        }
        snprintf(szScpCmd, PATH_MAX, "%s", line);
        snprintf(szScpCmd + (pShowName - line), PATH_MAX - (pShowName - line), "%s", szAccName);
        if (system(szScpCmd) != 0) {
            return result;
        }
        return BOOL_TRUE;
    }
	
	/* If error then print message */
	if (errmsg) {
		if (tinyrl__get_isatty(this) ||
			!context->shell->current_file) {
            fprintf(stderr, "%% Syntax error: %s\n", errmsg);
            if (CLISH_LINE_PARTIAL == arg_status)
            {
                tinyrl_reset_line_state(this);
            }
            else
            {
                /* remove line */
                if (tinyrl__get_last_buffer(this)) {
                    tinyrl_history_add(tinyrl__get_history(this), 
                        tinyrl__get_last_buffer(this));
                }
                tinyrl_remove_line_ctc(this);
                tinyrl_done(this);
                return BOOL_TRUE;
            }
		} else {
			char *fname = "stdin";
			if (context->shell->current_file->fname)
				fname = context->shell->current_file->fname;
			fprintf(stderr, "Syntax error on line %s:%u \"%s\": "
			"%s\n", fname, context->shell->current_file->line,
			line, errmsg);
		}
	}

    if (invalid_cmd)
    {
//        if (!isStartupCfg())
//        {
            errmsg = "Invalid command";
            fprintf(stderr, "\n%% Syntax error: %s", errmsg);
//        }
        if (tinyrl__get_last_buffer(this)) {
            tinyrl_history_add(tinyrl__get_history(this), 
                tinyrl__get_last_buffer(this));
        }
        tinyrl_reset_line_state_ctc(this);
    }
    
	/* keep the compiler happy */
	key = key;

	return result;
}

/* Ctrl-Z */
extern bool_t tinyrl_key_tstp(tinyrl_t * this, int key);
static bool_t clish_shell_tinyrl_key_tstp(tinyrl_t *this, int key)
{
    tinyrl_key_tstp(this, key);
    return clish_shell_tinyrl_key_enter(this, '\n');
}

/*-------------------------------------------------------- */
static bool_t clish_shell_tinyrl_hotkey(tinyrl_t *this, int key)
{
	clish_view_t *view;
	const char *cmd = NULL;
	clish_context_t *context = tinyrl__get_context(this);
	clish_shell_t *shell = context->shell;
	int i;
	char *tmp = NULL;

	i = clish_shell__get_depth(shell);
	while (i >= 0) {
		view = clish_shell__get_pwd_view(shell, i);
		cmd = clish_view_find_hotkey(view, key);
		if (cmd)
			break;
		i--;
	}
	/* Check the global view */
	if (i < 0) {
		view = shell->global;
		cmd = clish_view_find_hotkey(view, key);
	}
	if (!cmd)
		return BOOL_FALSE;

	tmp = clish_shell_expand(cmd, SHELL_VAR_NONE, context);
	tinyrl_replace_line(this, tmp, 0);
	lub_string_free(tmp);
	clish_shell_tinyrl_key_enter(this, 0);

	return BOOL_TRUE;
}

extern void clish_remove_duplicate_space(char *src, char *dest, int len);

char *lub_string_dupn_strip(const char *string, unsigned int len)
{
    char tmp[10240];
	char *res = NULL;

	if (!string)
		return res;
	res = malloc(len + 1);
    memset(res, 0, len+1);
	strncpy(tmp, string, len);
    clish_remove_duplicate_space(tmp, res, len);

	return res;
}

/*-------------------------------------------------------- */
/* This is the completion function provided for CLISH */
tinyrl_completion_func_t clish_shell_tinyrl_completion;
char **clish_shell_tinyrl_completion(tinyrl_t * tinyrl,
	const char *line, unsigned start, unsigned end)
{
	lub_argv_t *matches;
	clish_context_t *context = tinyrl__get_context(tinyrl);
	clish_shell_t *this = context->shell;
	clish_shell_iterator_t iter;
	const clish_command_t *cmd = NULL;
	char *text;
	char **result = NULL;
    int b_is_show_cmd = 0;
    int do_completion_partial = 1;
    char *p = NULL;

	if (tinyrl_is_quoting(tinyrl))
		return result;

    matches = lub_argv_new(NULL, 0);
    b_is_show_cmd = clish_is_show_cmd(line);
    if (b_is_show_cmd) {
        p = strrchr(line, '|');
        if (NULL == p) {
            p = strrchr(line, '>');
        }
    }
	
    if (NULL == p) {
	    text = lub_string_dupn_strip(line, end);
    } else {
        text = lub_string_dupn_strip(line, (p - line));
    }

    /* recalc start offset */
    start = strlen(text);
    while (start && !isspace(text[start - 1]))
        start--;    
    
	/* Don't bother to resort to filename completion */
	tinyrl_completion_over(tinyrl);

	/* Search for COMMAND completions */
	clish_shell_iterator_init(&iter, CLISH_NSPACE_COMPLETION);
	while ((cmd = clish_shell_find_next_completion(this, text, &iter))) {
        /* Hidden cmd should skip completion */
        if (clish_command__get_hidden(cmd)) {
            continue;
        }

        const char *pszCmd = clish_command__get_name(cmd);
        clish_ptype_t *ptype = NULL;

        /* process do ... */
        if (!strncasecmp(pszCmd, "do ", 3)) {
            pszCmd += 3;
        }
        if ('G' == *pszCmd && (ptype = clish_shell_find_ptype(context->shell, pszCmd))) {
            lub_argv_add(matches, clish_ptype__get_text(ptype));
        } else {
		    lub_argv_add(matches, clish_command__get_suffix(cmd));
        }
    }

	/* Try and resolve a command */
	cmd = clish_shell_resolve_command(this, text);
	/* Search for PARAM completion for non-hidden command. */
	if (cmd && !(clish_command__get_hidden(cmd))) {
        /* Modified by kcao 2015-03-25 for bug 37696
         * start maybe out-of-range of text, if has '|', need not do completion partial
         */
        if (b_is_show_cmd && NULL != p)
        {
            do_completion_partial = 0;
        }
        if (do_completion_partial) {
            /* IF_SPLITTING */
            bool_t completion_partial = BOOL_FALSE;
            clish_shell_param_generator(this, matches, cmd, text, start, &completion_partial);
            if (completion_partial)
                tinyrl_completion_partial(tinyrl);
        }
    }

	lub_string_free(text);

    if (b_is_show_cmd && NULL != p) {
        if (lub_argv__get_count(matches) > 0) {
            lub_argv_delete(matches);
            matches = lub_argv_new(NULL, 0); 
        }

        clish_modifier_matches(matches, p, 0);
    }

	/* Matches were found */
	if (lub_argv__get_count(matches) > 0) {
		unsigned i;
		char *subst = lub_string_dup(lub_argv__get_arg(matches, 0));
		/* Find out substitution */
		if(!tinyrl_is_agree_completion(tinyrl))
		{		
			if(' ' != line[strlen(line) - 1])
			{
				for (i = 1; i < lub_argv__get_count(matches); i++) {
					char *p = subst;
					const char *match = lub_argv__get_arg(matches, i);
					size_t match_len = strlen(p);
					/* identify the common prefix */
					while ((tolower(*p) == tolower(*match)) && match_len--) {
						p++;
						match++;
					}
					/* Terminate the prefix string */
					*p = '\0';
				}
				result = lub_argv__get_argv(matches, subst);
			}
		}else{
           for (i = 1; i < lub_argv__get_count(matches); i++) {
				char *p = subst;
				const char *match = lub_argv__get_arg(matches, i);
				size_t match_len = strlen(p);
				/* identify the common prefix */
				while ((tolower(*p) == tolower(*match)) && match_len--) {
					p++;
					match++;
				}
				/* Terminate the prefix string */
				*p = '\0';
			}
			result = lub_argv__get_argv(matches, subst);
		}
		lub_string_free(subst);
	}
	lub_argv_delete(matches);

	return result;
}

/*-------------------------------------------------------- */
static void clish_shell_tinyrl_init(tinyrl_t * this)
{
	bool_t status;
	/* bind the '?' key to the help function */
	status = tinyrl_bind_key(this, '?', clish_shell_tinyrl_key_help);
	assert(status);

	/* bind the <RET> key to the help function */
	status = tinyrl_bind_key(this, '\r', clish_shell_tinyrl_key_enter);
	assert(status);
	status = tinyrl_bind_key(this, '\n', clish_shell_tinyrl_key_enter);
	assert(status);

	/* for Ctrl-Z / SIGTSTP */
	status = tinyrl_bind_key(this, 26, clish_shell_tinyrl_key_tstp);

	/* bind the <SPACE> key to auto-complete if necessary */
	status = tinyrl_bind_key(this, ' ', clish_shell_tinyrl_key_space);
	assert(status);

	/* Set external hotkey callback */
	tinyrl__set_hotkey_fn(this, clish_shell_tinyrl_hotkey);

	/* Assign timeout callback */
	tinyrl__set_timeout_fn(this, clish_shell_timeout_fn);
	/* Assign keypress callback */
	tinyrl__set_keypress_fn(this, clish_shell_keypress_fn);
}

/*-------------------------------------------------------- */
/*
 * Create an instance of the specialised class
 */
tinyrl_t *clish_shell_tinyrl_new(FILE * istream,
	FILE * ostream, unsigned stifle)
{
	/* call the parent constructor */
	tinyrl_t *this = tinyrl_new(istream,
		ostream, stifle, clish_shell_tinyrl_completion);
	/* now call our own constructor */
	if (this)
		clish_shell_tinyrl_init(this);
	return this;
}

/*-------------------------------------------------------- */
void clish_shell_tinyrl_fini(tinyrl_t * this)
{
	/* nothing to do... yet */
	this = this;
}

/*-------------------------------------------------------- */
void clish_shell_tinyrl_delete(tinyrl_t * this)
{
	/* call our destructor */
	clish_shell_tinyrl_fini(this);
	/* and call the parent destructor */
	tinyrl_delete(this);
}

extern lub_argv_t *g_p_interface_list;
extern char szCurIntfName[IF_NAMESIZE];
int clish_shell_intfrange(char *pszIntfList, 
    clish_context_t* context, char *pszCmdLine, char **out)
{
    char szBuf[64];
    int nCnt;
    int i;
    int res;

    
    if (NULL == g_p_interface_list) {
        return -1;
    }

    nCnt = lub_argv__get_count(g_p_interface_list);
    for (i = 0; i < nCnt; i++) {
        const char *pszIntfName = lub_argv__get_arg(g_p_interface_list, i);
        snprintf(szBuf, 64, "iface=%s", pszIntfName);
        putenv(szBuf);
        if ((res = clish_shell_execute(context, out))) {
    		context->shell->state = SHELL_STATE_SCRIPT_ERROR;
    		return res;
    	}
	}
    return 0;
}

extern char *find_viewid_var(const char *name, clish_context_t *context);

static int clish_shell_is_need_history(clish_shell_t *this, const char *str)
{
    clish_view_t *view = clish_shell__get_view(this);

    if (str)
    {
        if (!strcasecmp(str, "internal-debug"))
        {
            return 0;
        }
    }

    if (view)
    {
        if (!strcasecmp(clish_view__get_name(view), "Internal-debug-view"))
        {
            return 0;
        }
    }

    return 1;
}

/*-------------------------------------------------------- */
static int clish_shell_execline(clish_shell_t *this, const char *line, char **out)
{
	char *str;
	clish_context_t context;
	tinyrl_history_t *history;
	int lerror = 0;
	int is_need_history = 0;
	struct stat stat_data;

	assert(this);
	this->state = SHELL_STATE_OK;
	if (!line && !tinyrl__get_istream(this->tinyrl)) {
		this->state = SHELL_STATE_SYSTEM_ERROR;
		return -1;
	}

	/* Renew prompt */
	clish_shell_renew_prompt(this);

	/* Set up the context for tinyrl */
	context.cmd = NULL;
	context.pargv = NULL;
	context.shell = this;

	/* Push the specified line or interactive line */
	if (line)
		str = tinyrl_forceline(this->tinyrl, &context, line);
	else
		str = tinyrl_readline(this->tinyrl, &context);
	lerror = errno;
	if (!str) {
		switch (lerror) {
		case ENOENT:
			this->state = SHELL_STATE_EOF;
			break;
		case ENOEXEC:
			this->state = SHELL_STATE_SYNTAX_ERROR;
			break;
		default:
			this->state = SHELL_STATE_SYSTEM_ERROR;
			break;
		};
		return -1;
	}

    is_need_history = clish_shell_is_need_history(this, str);
    if (is_need_history)
    {
        /* Deal with the history list */
        if (str && strcasecmp(str, "")) { /* Modified by Alexander */
            if (tinyrl__get_isatty(this->tinyrl)) {
                history = tinyrl__get_history(this->tinyrl);
                tinyrl_history_add(history, str);
            }
        }
    }

	/* Let the client know the command line has been entered */
	if (this->client_hooks->cmd_line_fn)
		this->client_hooks->cmd_line_fn(&context, str);

	/* Execute the provided command */
	if (context.cmd && context.pargv) {
		int res;
        clish_view_t *cur_view = clish_shell__get_view(this);

	if (!strcasecmp(clish_view__get_name(cur_view), "Exec")
		&& (!strncasecmp(str, "rmdir", 5)
		 || !strncasecmp(str, "pwd", 3)
		 || !strncasecmp(str, "dir", 3)
		 || !strncasecmp(str, "ls", 2))) {

		stat(".", &stat_data);
		if (stat_data.st_nlink == 0) {
		    if (!access("/mnt/flash", 0)) {
			printf("%% Get current working directory failed: No such file or directory");
			printf("\n%% Change working directory to flash:\n");
			chdir("/mnt/flash");
		    } else {
			printf("%% Get current working directory failed: No such file or directory");
			printf("\n%% Change working directory to /mnt\n");
			chdir("/mnt");
		    }
    		    free(str);
		    return 0;
		}
	}

        if (!strcasecmp(clish_view__get_name(cur_view), "InterfaceRange")
                && strcasecmp(str, "exit") 
                && strcasecmp(str, "quit")
                && strcasecmp(str, "end")
                && strcasecmp(str, "logout")
                && strncasecmp(str, "interface", 9)
                && strncasecmp(str, "show ", 5)
                && strncasecmp(str, "do ", 3)) {
            char *pszIntfList = find_viewid_var("ifacelist", &context); 
            if (NULL != pszIntfList) {
                clish_shell_intfrange(pszIntfList, &context, str, out);
            } else {
                fprintf(stderr, "%% Internal error: can't found var ifacelist_name\r\n");
            }
        } else {
    		if ((res = clish_shell_execute(&context, out))) {
    			this->state = SHELL_STATE_SCRIPT_ERROR;
    			if (context.pargv)
    				clish_pargv_delete(context.pargv);
    		    free(str);
    			return res;
    		}
		}
	}

	free(str);

	if (context.pargv)
		clish_pargv_delete(context.pargv);

	return 0;
}

/*-------------------------------------------------------- */
int clish_shell_forceline(clish_shell_t *this, const char *line, char **out)
{
	return clish_shell_execline(this, line, out);
}

/*-------------------------------------------------------- */
int clish_shell_readline(clish_shell_t *this, char **out)
{
	return clish_shell_execline(this, NULL, out);
}

/*-------------------------------------------------------- */
FILE * clish_shell__get_istream(const clish_shell_t * this)
{
	return tinyrl__get_istream(this->tinyrl);
}

/*-------------------------------------------------------- */
FILE * clish_shell__get_ostream(const clish_shell_t * this)
{
	return tinyrl__get_ostream(this->tinyrl);
}

/*-------------------------------------------------------- */
void clish_shell__set_interactive(clish_shell_t * this, bool_t interactive)
{
	assert(this);
	this->interactive = interactive;
}

/*-------------------------------------------------------- */
bool_t clish_shell__get_interactive(const clish_shell_t * this)
{
	assert(this);
	return this->interactive;
}

/*-------------------------------------------------------- */
bool_t clish_shell__get_utf8(const clish_shell_t * this)
{
	assert(this);
	return tinyrl__get_utf8(this->tinyrl);
}

/*-------------------------------------------------------- */
void clish_shell__set_utf8(clish_shell_t * this, bool_t utf8)
{
	assert(this);
	tinyrl__set_utf8(this->tinyrl, utf8);
}

/*-------------------------------------------------------- */
void clish_shell__set_timeout(clish_shell_t *this, int timeout)
{
	assert(this);
	this->idle_timeout = timeout;
}

/*--------------------------------------------------------- */
tinyrl_t *clish_shell__get_tinyrl(const clish_shell_t * this)
{
	return this->tinyrl;
}

/*----------------------------------------------------------*/
int clish_shell__save_history(const clish_shell_t *this, const char *fname)
{
	return tinyrl__save_history(this->tinyrl, fname);
}

/*----------------------------------------------------------*/
int clish_shell__restore_history(clish_shell_t *this, const char *fname)
{
	return tinyrl__restore_history(this->tinyrl, fname);
}

/*----------------------------------------------------------*/
void clish_shell__stifle_history(clish_shell_t *this, unsigned int stifle)
{
	tinyrl__stifle_history(this->tinyrl, stifle);
}

/*-------------------------------------------------------- */
