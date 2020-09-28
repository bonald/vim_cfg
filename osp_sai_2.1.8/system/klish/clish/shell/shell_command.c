/*
 * shell_word_generator.c
 */
#include <string.h>
#include <ctype.h>

#include "private.h"
#include "lub/string.h"
#include "lub/argv.h"

#include "lib_fs.h"

/*-------------------------------------------------------- */
void
clish_shell_iterator_init(clish_shell_iterator_t * iter,
	clish_nspace_visibility_t field)
{
	iter->last_cmd = NULL;
	iter->field = field;
}

/*--------------------------------------------------------- */
const clish_command_t *clish_shell_resolve_command(const clish_shell_t * this,
	const char *line)
{
	clish_command_t *cmd, *result;

	/* Search the current view */
	result = clish_view_resolve_command(clish_shell__get_view(this), line, BOOL_TRUE);
	/* Search the global view */
	cmd = clish_view_resolve_command(this->global, line, BOOL_TRUE);

#if 0
    if (!result && !cmd) {
        /* search configure terminal view */
        unsigned int depth = clish_shell__get_depth(this);
        clish_view_t *ctview = NULL;
        if (depth >= 1 && (ctview = clish_shell__get_view_depth(this, 0))) {
            if (!strcasecmp(clish_view__get_name(ctview), "Configure")) {
                cmd = clish_view_resolve_command(ctview, line, BOOL_TRUE);
            }
        }
    }
#endif

	result = clish_command_choose_longest(result, cmd);

	return result;
}

/*--------------------------------------------------------- */
const clish_command_t *clish_shell_resolve_prefix(const clish_shell_t * this,
	const char *line)
{
	clish_command_t *cmd, *result;

	/* Search the current view */
	result = clish_view_resolve_prefix(clish_shell__get_view(this), line, BOOL_TRUE);
	/* Search the global view */
	cmd = clish_view_resolve_prefix(this->global, line, BOOL_TRUE);

#if 0
    if (!result && !cmd) {
        /* search configure terminal view */
        unsigned int depth = clish_shell__get_depth(this);
        clish_view_t *ctview = NULL;
        if (depth >= 1 && (ctview = clish_shell__get_view_depth(this, 0))) {
            if (!strcasecmp(clish_view__get_name(ctview), "Configure")) {
                cmd = clish_view_resolve_prefix(ctview, line, BOOL_TRUE);
            }
        }
    }
#endif
	result = clish_command_choose_longest(result, cmd);

	return result;
}

/*-------------------------------------------------------- */
const clish_command_t *clish_shell_find_next_completion(const clish_shell_t *
	this, const char *line, clish_shell_iterator_t * iter)
{
	const clish_command_t *result, *cmd;

	/* ask the local view for next command */
	result = clish_view_find_next_completion(clish_shell__get_view(this),
		iter->last_cmd, line, iter->field, BOOL_TRUE);
	/* ask the global view for next command */
	cmd = clish_view_find_next_completion(this->global,
		iter->last_cmd, line, iter->field, BOOL_TRUE);
#if 0
    if (!result && !cmd) {
        /* search configure terminal view */
        unsigned int depth = clish_shell__get_depth(this);
        clish_view_t *ctview = NULL;
        if (depth >= 1 && (ctview = clish_shell__get_view_depth(this, 0))) {
            if (!strcasecmp(clish_view__get_name(ctview), "Configure")) {
                cmd = clish_view_find_next_completion(ctview,
		                iter->last_cmd, line, iter->field, BOOL_TRUE);
            }
        }
    }
#endif
	if (clish_command_diff(result, cmd) > 0)
		result = cmd;

	if (!result)
		iter->last_cmd = NULL;
	else
		iter->last_cmd = clish_command__get_name(result);

	return result;
}

static int __match_out_cb(void *pArg, const char *pszMatch, const char *pszHelp)
{
    lub_argv_t *matches = (lub_argv_t *)pArg; 
    
    if (!lub_argv__item_exist(matches, pszMatch)) {
        lub_argv_add(matches, pszMatch);
    }
    return 0;
}

/*--------------------------------------------------------- */
void clish_shell_param_generator(clish_shell_t *this, lub_argv_t *matches,
	const clish_command_t *cmd, const char *line, unsigned offset, bool_t *completion_partial/* IF_SPLITTING */)
{
	const char *name = clish_command__get_name(cmd);
	char *text = lub_string_dup(&line[offset]);
	clish_ptype_t *ptype;
	unsigned idx = lub_argv_wordcount(name);
	/* get the index of the current parameter */
	unsigned index = lub_argv_wordcount(line) - idx;
	clish_context_t context;

    gen_set_last_token((char *)line);

	if ((0 != index) || (offset && line[offset - 1] == ' ')) {
		lub_argv_t *argv = lub_argv_new(line, 0);
		clish_pargv_t *pargv = clish_pargv_new();
		clish_pargv_t *completion = clish_pargv_new();
		unsigned completion_index = 0;
		const clish_param_t *param = NULL;

		/* if there is some text for the parameter then adjust the index */
		if ((0 != index) && (text[0] != '\0'))
			index--;

		/* Parse command line to get completion pargv's */
		context.shell = this;
		context.cmd = cmd;
		context.pargv = pargv;
		clish_shell_parse_pargv(pargv, cmd, &context,
			clish_command__get_paramv(cmd),
			argv, &idx, completion, index + idx);
		lub_argv_delete(argv);

		while ((param = clish_pargv__get_param(completion,
			completion_index++))) {
			char *result;
            const char *pname = clish_param__get_name(param);
            
            if ('G' == *pname) {     
                param_token_e tid = gen_parse_token(pname);
                if (E_TOKEN_UNKNOWN != tid) {
                    int _cnt = lub_argv__get_count(matches);
                    gen_output_matchstr(__match_out_cb, matches, "", (char *)pname);
                    if (_cnt < lub_argv__get_count(matches)) {
                        int _status = 0;
                        if (E_TOKEN_GURLNAME == tid) {
                            _status = gen_url_match(gen_get_last_token_str());
                        } else {
                            _status = gen_filename_match(gen_get_last_token_str(), tid, 0);
                        }
                        gen_set_match_status(_status);
                    }
                    continue;
                }
            }
            
            
			/* The param is args so it has no completion */
			if (param == clish_command__get_args(cmd))
				continue;
			/* The switch has no completion string */
			if (CLISH_PARAM_SWITCH == clish_param__get_mode(param))
				continue;
			/* The subcommand is identified by it's value */
			if (CLISH_PARAM_SUBCOMMAND ==
				clish_param__get_mode(param)) {
				result = clish_param__get_value(param);
				if (result)
					lub_argv_add(matches, result);
			}
			/* The 'completion' field of PARAM */
			if (clish_param__get_completion(param)) {
				char *str, *q;
				char *saveptr;
				str = clish_shell_expand(
					clish_param__get_completion(param), SHELL_VAR_ACTION, &context);
				if (str) {
					for (q = strtok_r(str, " \n", &saveptr);
						q; q = strtok_r(NULL, " \n", &saveptr)) {
						if (q == strstr(q, text))
							lub_argv_add(matches, q);
					}
					lub_string_free(str);
				}
			}
			/* The common PARAM. Let ptype do the work */
			if ((ptype = clish_param__get_ptype(param)))
				clish_ptype_word_generator(ptype, matches, text, completion_partial/* IF_SPLITTING */);
		}
		clish_pargv_delete(completion);
		clish_pargv_delete(pargv);
	}

	lub_string_free(text);
}

/*--------------------------------------------------------- */
