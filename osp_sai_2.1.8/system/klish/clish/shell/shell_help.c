/*
 * shell_help.c
 */
#include "private.h"
#include "clish/types.h"
#include "lub/string.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "lib_fs.h"

/* Added by Alexander 2014-09-02 16:04 */
#define M_CMD_BUF_SZ        1024
#define M_CMD_MODIFIER_SZ   1024

typedef struct {
    int bIsFile; /* is GFILENAME */
    char *pszType;
    char *pszHelp;
    char *pszKeyword;
    char *pszDesc;
} clish_modifier_t;

typedef struct {
    void *pArg;
    int bHelp;
    int nCnt;
} clish_match_arg_t;

static int g_show_match_status = 0;

static clish_modifier_t g_clish_modifies[] = {
    {0, "begin", "Begin with the line that matches", "STRING", "Expression"},
    {0, "exclude", "Exclude lines that match", "STRING", "Expression"},
    {0, "include", "Include lines that match", "STRING", "Expression"},
    {1, "redirect", "Redirect output", "GFILENAME", "Output file"},
    {0, NULL, NULL},
};

static clish_modifier_t g_clish_modifies_tofile[] = {
    {1, "flash:/", "Output to file", "GFILENAME", "Output file"},
    {0, NULL, NULL},
};

void set_show_match_status(int state)
{
    g_show_match_status = state;
}

int get_show_match_status()
{
    return g_show_match_status;
}

/*--------------------------------------------------------- */
/*
 * Provide a detailed list of the possible command completions
 */
static void available_commands(clish_shell_t *this,
	clish_help_t *help, const char *line, size_t *max_width)
{
	const clish_command_t *cmd;
	clish_shell_iterator_t iter;

	if (max_width)
		*max_width = 0;
	/* Search for COMMAND completions */
	clish_shell_iterator_init(&iter, CLISH_NSPACE_HELP);
	while ((cmd = clish_shell_find_next_completion(this, line, &iter))) {
        /* Hidden command should not be shown in help message */
        if (clish_command__get_hidden(cmd)) {
            continue;
        }
        
		size_t width;
		const char *name = clish_command__get_suffix(cmd);
		if (max_width) {
			width = strlen(name);
			if (width > *max_width)
				*max_width = width;
		}
		lub_argv_add(help->name, name);
		lub_argv_add(help->help, clish_command__get_text(cmd));
		lub_argv_add(help->detail, clish_command__get_detail(cmd));
	}
}

extern int
check_if_name_str(char *pszName, int len);
/*--------------------------------------------------------- */
static int available_params(clish_shell_t *this,
	clish_help_t *help, const clish_command_t *cmd,
	const char *line, size_t *max_width)
{
	unsigned index = lub_argv_wordcount(line);
	unsigned idx = lub_argv_wordcount(clish_command__get_name(cmd));
	lub_argv_t *argv;
	clish_pargv_t *completion, *pargv;
	unsigned i;
	unsigned cnt = 0;
	clish_pargv_status_t status = CLISH_LINE_OK;
	clish_context_t context;

	/* Empty line */
	if (0 == index)
		return -1;

	if (line[strlen(line) - 1] != ' ')
		index--;

	argv = lub_argv_new(line, 0);

	/* get the parameter definition */
	completion = clish_pargv_new();
	pargv = clish_pargv_new();
	context.shell = this;
	context.cmd = cmd;
	context.pargv = pargv;
	status = clish_shell_parse_pargv(pargv, cmd, &context,
		clish_command__get_paramv(cmd),
		argv, &idx, completion, index);
	clish_pargv_delete(pargv);
	cnt = clish_pargv__get_count(completion);

    /* IF_SPLITTING */
    clish_param_intf_help_finish();
    
	/* Calculate the longest name */
	for (i = 0; i < cnt; i++) {
		const clish_param_t *param;
		const char *name = NULL;
		unsigned clen = 0;

		param = clish_pargv__get_param(completion, i);
		if (CLISH_PARAM_SUBCOMMAND == clish_param__get_mode(param))
			name = clish_param__get_value(param);
        /* IF_SPLITTING */
        else if (CLISH_PTYPE_INTF == clish_ptype__get_method(clish_param__get_ptype(param)))
        {   if (NULL != clish_pargv__get_parg(completion, i) 
                && NULL != clish_parg__get_value(clish_pargv__get_parg(completion, i)))
            {
                if (0 != strlen(clish_parg__get_value(clish_pargv__get_parg(completion, i)))
                    && (0 != check_if_name_str((char *)clish_parg__get_value(clish_pargv__get_parg(completion, i)),
                    strlen(clish_parg__get_value(clish_pargv__get_parg(completion, i))))))
                {
                    return -1;
                }
            }

            clish_param_intf_help_init(
                    clish_parg__get_value(clish_pargv__get_parg(completion, i)));
        }
		else
			name = clish_ptype__get_text(clish_param__get_ptype(param));
		if (name)
			clen = strlen(name);
		if (max_width && (clen > *max_width))
			*max_width = clen;
        __helpstr_init_maxwidth(*max_width);
        if (NULL != strstr(line, "%"))
        {
            return -1;
        }
        /* IF_SPLITTING */
		clish_param_help(param, help);
        *max_width = __helpstr_get_maxwidth();
	}
	clish_pargv_delete(completion);
	lub_argv_delete(argv);

	/* It's a completed command */
	if (CLISH_LINE_OK == status)
		return 0;

	/* Incompleted command */
	return -1;
}

int clish_is_show_cmd(const char *line)
{
    if (!strncasecmp(line, "show ", 5) || !strncasecmp(line, "do show ", 8)) {
        return 1;
    }

    if (!strncasecmp(line, "display ", strlen("display "))) {
        return 1;
    }

    return 0;
}

void clish_split_show_cmd(const char *line, char *pszCmd, size_t nCmdSize, 
                                  char *pszModifier, size_t nModSize)
{
    char *p = strchr(line, '|');
    int nCnt;
    
    if (NULL == p) {
        p = strchr(line, '>');
    }

    if (NULL == p) {
        *pszModifier = '\0';
        snprintf(pszCmd, nCmdSize, "%s", line);
        return;
    }

    nCnt = p - line;
    if (nCnt >= nCmdSize) {
        assert(0);
        *pszModifier = '\0';
        snprintf(pszCmd, nCmdSize, "%s", line);
        return;
    }

    memcpy(pszCmd, line, (p - line));
    pszCmd[p - line] = '\0';
    snprintf(pszModifier, nModSize, "%s", p);
    return;
}

static void __split_modifier_line(const char *pszModifier, char *pszType, size_t nTypeSize, 
    char *pszParam, size_t nParamSize, char **ppszLeft)
{
    char *p = strchr(pszModifier, '|');
    char *q;
    int nCnt;
    int isRdir = 0;

    pszType[0] = '\0';
    pszParam[0] = '\0';
    *ppszLeft = NULL;
    
    if (NULL == p) {
        p = strchr(pszModifier, '>');
        if (NULL != p) {
            isRdir = 1;
        }
    }

    if (NULL == p) {
        return;
    }

    /* locate type */
    p++;
    while (' ' == *p) {
        p++;
    }
    if ('\0' == *p) {
        /* all spaces */
        return;
    }

    /* extract type */
    q = strchr(p, ' ');
    if (NULL == q) {
        /* no param */
        snprintf(pszType, nTypeSize, "%s", p);
        return;
    }

    /* have param */
    nCnt = q - p;
    if (nCnt >= nTypeSize) {
        assert(0);
        snprintf(pszType, nTypeSize, "%s", p);
        return;
    }
    memcpy(pszType, p, (q - p));
    pszType[q - p] = '\0';

    p = q; 
    while (' ' == *q) {
        q++;
    }
    if ('\0' == *q) {
        /* all space */
        snprintf(pszParam, nParamSize, "%s", p);
        return;
    } 

    if (isRdir || !strcmp("redirect", pszType)) {
        snprintf(pszParam, nParamSize, "%s", q);
        p = strchr(pszParam, ' ');
        if (NULL != p) {
            *p = '\0';
        }
        
        p = strrchr(q, ' ');
        if (NULL != p) {
            *ppszLeft = p; /* have content after the param */
        }
    } else {
        snprintf(pszParam, nParamSize, "%s", q);
    }
    return;
}

int clish_check_modifier_pargs(lub_argv_t *pModArg, const char *pszModifier)
{
    char szType[M_CMD_MODIFIER_SZ];
    char szParam[M_CMD_MODIFIER_SZ];
    clish_modifier_t *pModifier = NULL;
    char *pLeft = NULL;
    int fullmatch = 0;
    int partmatch = 0;
    int i;

    pModifier = ('|' == *pszModifier) ? g_clish_modifies : g_clish_modifies_tofile;
    __split_modifier_line(pszModifier, szType, M_CMD_MODIFIER_SZ,
        szParam, M_CMD_MODIFIER_SZ, &pLeft);

    if ('>' == *pszModifier) {
        if ('\0' == szType[0]) {
            return CLISH_LINE_PARTIAL;
        }

        if ('\0' != szParam[0]) {
            if (strspn(szParam, " \t\r\n") != strlen(szParam)) {
                return CLISH_BAD_CMD;
            }
        }

        if (pModArg) {
            lub_argv_add(pModArg, szType);
        }
        return CLISH_LINE_OK;
    }

    /* generate help */
    if ('\0' == szType[0]) {
        return CLISH_LINE_PARTIAL;
    } 

    for (i = 0; ; i++) {
        if (NULL == pModifier[i].pszType) {
            break;
        }
        if (!strcasecmp(szType, pModifier[i].pszType)) {
            /* full match */
            fullmatch = 1;
            break;
        } 

        if (!strncasecmp(szType, pModifier[i].pszType, strlen(szType))) {
            partmatch = 1;
            break;
        }
    }

    if (partmatch) {
        if ('\0' != szParam[0]) {
            return CLISH_BAD_CMD;
        }
        
        return CLISH_LINE_PARTIAL;
    }

    if (!fullmatch) {
        return CLISH_BAD_CMD;
    }

    if ('\0' == szParam[0]) {
        return CLISH_LINE_PARTIAL;
    }

    /* check chars after the param */
    if (pLeft) {
        if (strspn(pLeft, " \t\r\n") != strlen(pLeft)) {
            return CLISH_BAD_CMD;
        }
    }

    if (pModArg) {
        lub_argv_add(pModArg, szType);
        lub_argv_add(pModArg, szParam);
    }
    return CLISH_LINE_OK;
}

int _g_clish_modifier_matched = 0;
int clish_modifier_matched()
{
    return _g_clish_modifier_matched != 0;
}

void set_modifier_matched_flag(int onoff)
{
    _g_clish_modifier_matched = (0 == onoff) ? 0 : 1;
    return;
}


static int __matches_out_cb(void *pArg, const char *pszMatch, const char *pszHelp)
{
    clish_match_arg_t *pma = (clish_match_arg_t *)pArg;
    int nLen = strlen(pszMatch);
    char *pszUsage;
   
    if (pma->bHelp) {
        clish_help_t *help = (clish_help_t *)pma->pArg;
        if (lub_argv__item_exist(help->name, pszMatch)) {
            return 0;
        }

        lub_argv_add(help->name, pszMatch);
        if (NULL != (pszUsage = (char *)gen_get_url_helpstr(pszMatch))) {
            char *str = NULL;
            
            lub_string_cat(&str, pszHelp);
            lub_string_cat(&str, " (");
            lub_string_cat(&str, pszUsage);
            lub_string_cat(&str, ")");

            lub_argv_add(help->help, str);
            lub_string_free(str);
        } else {
            lub_argv_add(help->help, pszHelp);
        }
        lub_argv_add(help->detail, NULL);
    } else {
        lub_argv_t *matches = (lub_argv_t *)pma->pArg;
 
        if (lub_argv__item_exist(matches, pszMatch)) {
            return 0;
        }
        lub_argv_add(matches, pszMatch);
    }

    __helpstr_set_maxwidth(nLen);
    pma->nCnt++;
    set_modifier_matched_flag(1);
    return 0;
}

static int clish_modifier_matches_rdir(
    const char *pszModifier, void *pArg, clish_modifier_t *pModifier, int bHelp,
    char *pszType, char *pszParam)
{
    clish_match_arg_t matcharg;

    memset(&matcharg, 0x00, sizeof(clish_match_arg_t));
    matcharg.pArg = pArg;
    matcharg.bHelp = bHelp;
    
    /* generate matches */
    if ('\0' == *pszType) {
        int nModLen = strlen(pszModifier);

        if (1 == nModLen) { /* only > */
            __matches_out_cb(&matcharg, ">", "Output redirection");
            return 0;
        } 

        /* have space */
    }

    if ('\0' != *pszParam) {
        if (strspn(pszParam, " \t\r\n") != strlen(pszParam)) {
            /* invalid command */
            return 0;
        }
        
        if (bHelp) {
            __matches_out_cb(&matcharg, "<cr>", NULL);
        }  
        return 0;
    }

    matcharg.nCnt = 0; 
    param_token_e tid = gen_parse_token(pModifier->pszKeyword);
    if (E_TOKEN_UNKNOWN != tid) {
        int _status = 0;
        
        if ('\0' == *pszType) {
            gen_set_last_token("");
            gen_output_matchstr(__matches_out_cb, &matcharg, 
                    "", pModifier->pszKeyword);
        } else {
            gen_set_last_token(pszType);
            gen_output_matchstr(__matches_out_cb, &matcharg, 
                    pszType, pModifier->pszKeyword);
        }
        if (matcharg.nCnt > 0) {
            _status = gen_filename_match(gen_get_last_token_str(), tid, 0);
            gen_set_match_status(_status);
        }
    }

    if (bHelp) {
        /* modified by liwh for bug 53003, 2019-08-08 
            <cr> should not follow >*/
        if (memcmp(pszModifier, ">", 1))
        /* liwh end */
        {
            __matches_out_cb(&matcharg, "<cr>", NULL);
        }
    }    
    return 0;
}

int clish_modifier_matches(void *pArg, const char *pszModifier, int bHelp)
{
    char szType[M_CMD_MODIFIER_SZ];
    char szParam[M_CMD_MODIFIER_SZ];
    clish_modifier_t *pModifier = NULL;
    clish_match_arg_t matcharg;
    char *pszLeft = NULL;
    int fullmatch = 0;
    int partmatch = 0;
    int i;

    set_modifier_matched_flag(0);
    memset(&matcharg, 0x00, sizeof(clish_match_arg_t));
    matcharg.pArg = pArg;
    matcharg.bHelp = bHelp;
    if ('\0' == *pszModifier) {
        if (bHelp) {
            __matches_out_cb(&matcharg, "|", "Output modifiers");
            __matches_out_cb(&matcharg, ">", "Output redirection");
            __matches_out_cb(&matcharg, "<cr>", NULL);
        }
        return 0;
    }

    pModifier = ('|' == *pszModifier) ? g_clish_modifies : g_clish_modifies_tofile;
    __split_modifier_line(pszModifier, szType, M_CMD_MODIFIER_SZ,
        szParam, M_CMD_MODIFIER_SZ, &pszLeft);
    gen_set_last_token((char *)pszModifier);
    if ('>' == *pszModifier) {
        return clish_modifier_matches_rdir(pszModifier, pArg, pModifier, bHelp,
                    szType, szParam);
    }
    
    /* generate matches */
    if ('\0' == szType[0]) {
        int nModLen = strlen(pszModifier);

        if (1 == nModLen) { /* only | */
            __matches_out_cb(&matcharg, "|", "Output modifiers");
            return 0;
        } 

        /* have space */
        for (i = 0; ; i++) {
            if (NULL == pModifier[i].pszType) {
                break;
            }

            __matches_out_cb(&matcharg, pModifier[i].pszType, pModifier[i].pszHelp);
        }

        return 0;
    }

    for (i = 0; ; i++) {
        if (NULL == pModifier[i].pszType) {
            break;
        }

        if (!strcasecmp(szType, pModifier[i].pszType)) {
            fullmatch = 1;
            break;
        }
        
        if (!strncasecmp(szType, pModifier[i].pszType, strlen(szType))) {
            partmatch = 1;

            __matches_out_cb(&matcharg, pModifier[i].pszType, pModifier[i].pszHelp);
        }
    }

    if (partmatch) {
        return 0;
    }

    if (!fullmatch) {
        return 0;
    }

    if (NULL != pszLeft) {
        if (strspn(pszLeft, " \t\r\n") != strlen(pszLeft)) {
            /* invalid command */
            return 0;
        }
        if (bHelp) {
            __matches_out_cb(&matcharg, "<cr>", NULL);
        } 
        return 0;
    }

    if ('\0' == szParam[0]) {
        __matches_out_cb(&matcharg, pModifier[i].pszType, pModifier[i].pszHelp);
        return 0;
    }
    
    matcharg.nCnt = 0;
    if (pModifier[i].bIsFile) {   
        param_token_e tid = gen_parse_token(pModifier[i].pszKeyword);
        if (E_TOKEN_UNKNOWN != tid) {
            int _status = 0;
            
            if ('\0' == szParam[0]) {
                gen_set_last_token("");
                gen_output_matchstr(__matches_out_cb, &matcharg, 
                        "", pModifier[i].pszKeyword);
            } else {
                gen_set_last_token(szParam);
                gen_output_matchstr(__matches_out_cb, &matcharg, 
                        szParam, pModifier[i].pszKeyword);
            }
            if (matcharg.nCnt > 0) {
                _status = gen_filename_match(gen_get_last_token_str(), tid, 0);
                gen_set_match_status(_status);
            }
        }
    } else if (bHelp) {
        __matches_out_cb(&matcharg, pModifier[i].pszKeyword, pModifier[i].pszHelp);
    } else {
        __matches_out_cb(&matcharg, szParam, pModifier[i].pszHelp);
        set_show_match_status(1);
    }

    if (bHelp && strspn(szParam, " \t\r\n") != strlen(szParam)) {
        __matches_out_cb(&matcharg, "<cr>", NULL);
    }    
    return 0;
}

void clish_remove_duplicate_space(char *src, char *dest, int len)
{
    int i = 0;
    int j = 0;
    int first_space = 1;
    
    for (i = 0; i < len; i++)
    {
        if (src[i] != ' ')
        {
            dest[j] = src[i];
            j++;
            first_space = 1;
        }
        else
        {
            if (first_space)
            {
                dest[j] = src[i];
                j++;
                first_space = 0;
            }
        }
    }
    dest[j] = '\0';

}

/*--------------------------------------------------------- */
void clish_shell_help(clish_shell_t *this, const char *line)
{
	clish_help_t help;
	size_t max_width = 0;
	const clish_command_t *cmd;
    char tmp_cbuf[M_CMD_BUF_SZ];
    char cbuf[M_CMD_BUF_SZ];
    char modifier[M_CMD_MODIFIER_SZ];
    int is_show_cmd = clish_is_show_cmd(line);
	int i;
	int bFirst = 1;

    gen_set_last_token((char *)line);

    /* split show command line */
    if (is_show_cmd) {
        clish_split_show_cmd(line, tmp_cbuf, M_CMD_BUF_SZ, modifier, M_CMD_MODIFIER_SZ);
    } else {
        modifier[0] = '\0';
        snprintf(tmp_cbuf, M_CMD_BUF_SZ, "%s", line);
    }

    /* for bug46030 replace % to get correct available commands */
    int len = strlen(tmp_cbuf);
    for (i = 0; i < len; ++i)
    {
        if (tmp_cbuf[i] == '%')
        {
            tmp_cbuf[i] = '_';
        }
    }

    memset(cbuf, 0, sizeof(cbuf));
    clish_remove_duplicate_space(tmp_cbuf, cbuf, sizeof(tmp_cbuf));
	help.name = lub_argv_new(NULL, 0);
	help.help = lub_argv_new(NULL, 0);
	help.detail = lub_argv_new(NULL, 0);

	/* Get COMMAND completions */
	available_commands(this, &help, cbuf, &max_width);

	/* Resolve a command */
	cmd = clish_shell_resolve_command(this, cbuf);
	/* Search for PARAM completion for non-hidden command */
	if (cmd && !(clish_command__get_hidden(cmd))) {
		size_t width = 0;
		int status;
		status = available_params(this, &help, cmd, cbuf, &width);
		if (width > max_width)
			max_width = width;
		/* Add <cr> if command is completed */
		if (!status) {
            if (is_show_cmd) {
                /* process CLI modifier */            
                if ('\0' != modifier[0]) {
                    lub_argv_delete(help.name);
    	            lub_argv_delete(help.help);
    	            lub_argv_delete(help.detail);

                    help.name = lub_argv_new(NULL, 0);
    	            help.help = lub_argv_new(NULL, 0);
    	            help.detail = lub_argv_new(NULL, 0);
                    __helpstr_init_maxwidth(0);
                }
                
                clish_modifier_matches(&help, modifier, 1);
                /* Added by kcao for bug 38787 */
                for (i = 0; i < lub_argv__get_count(help.name); i++) {
                    const char *name = lub_argv__get_arg(help.name, i);
                    width = strlen(name);
                    if (width > max_width)
                        max_width = width;
                }                
            } else {
                      lub_argv_add(help.name, "<cr>");
        		lub_argv_add(help.help, NULL);
        		lub_argv_add(help.detail, NULL);
            }
		}
	} 
    
    if (lub_argv__get_count(help.name) == 0)
    {
        /* Added by kcao 2015-05-28 for bug 33487 */
        printf("%% There is no matched command\n");
        goto end;
    }

	/* Print help messages */
	for (i = 0; i < lub_argv__get_count(help.name); i++) {
        const char *pszName = lub_argv__get_arg(help.name, i);
        clish_ptype_t *ptype = NULL;
        
        if ('G' == *pszName) {
            ptype = clish_shell_find_ptype(this, pszName);
            if (NULL != ptype) {
                pszName = clish_ptype__get_text(ptype);
            }
        }

        /* ignore ! */
        if (!strcasecmp(pszName, "!")) {
            continue;
        }

        if (bFirst) {
            bFirst = 0;
            /* Added by Alexander */
            const clish_view_t *view = 
                clish_shell__get_view(this);
            if (NULL == line || !strcasecmp(line, "")) {
                printf("%s commands: \n", clish_view__get_name(view));
    	    }
        }
        
		printf("  %-*s  %s\n", (int)max_width,
			pszName,
			lub_argv__get_arg(help.help, i) ?
			lub_argv__get_arg(help.help, i) : "");
	}

	/* Print details */
	if ((lub_argv__get_count(help.name) == 1) &&
		(SHELL_STATE_HELPING == this->state)) {
		const char *detail = lub_argv__get_arg(help.detail, 0);
		if (detail)
			printf("%s\n", detail);
	}

	/* update the state */
	if (this->state == SHELL_STATE_HELPING)
		this->state = SHELL_STATE_OK;
	else
		this->state = SHELL_STATE_HELPING;

end:
	lub_argv_delete(help.name);
	lub_argv_delete(help.help);
	lub_argv_delete(help.detail);
}

/*--------------------------------------------------------- */
