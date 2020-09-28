/*
 * pargv.c
 */
#include "private.h"
#include "lub/string.h"
#include "lub/argv.h"
#include "lub/system.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

/*--------------------------------------------------------- */
/*
 * Search for the specified parameter and return its value
 */
static clish_parg_t *find_parg(clish_pargv_t * this, const char *name)
{
	unsigned i;
	clish_parg_t *result = NULL;
	int idx = 0; /* Added by Alexander for GXX2 */
	char szName[32];
	int cnt = 0;
	char *p;

	if (!this || !name)
		return NULL;

    p = (char *)name;
    szName[0] = '\0';
    if ('G' == *name) {    
        do {
            if (isdigit(*p)) {
                idx = atoi(p) - 1;
                snprintf(szName, 32, name);
                szName[p - name] = '\0';
                p = szName;
                break;
            }
            p++;
        } while ('\0' != *p);
    }

    if ('\0' == szName[0]) {
        p = (char *)name;
    }

	/* scan the parameters in this instance */
	for (i = 0; i < this->pargc; i++) {
		clish_parg_t *parg = this->pargv[i];
		const char *pname = clish_param__get_name(parg->param);

		if (0 == strcmp(pname, p)) {
		    if (cnt == idx) {
                result = parg;
			    break;
		    }
			cnt++;
		}
	}
	
	return result;
}

/*--------------------------------------------------------- */
int clish_pargv_insert(clish_pargv_t * this,
	const clish_param_t * param, const char *value)
{
	if (!this || !param)
		return -1;

	clish_parg_t *parg = find_parg(this, clish_param__get_name(param));

    if (parg) { /* Added by Alexander Liu */
    	char *p = (char *)clish_param__get_name(param);
    	if ('G' == *p) {
    	    if (parg->param != param) {
                parg = NULL;
    	    }
    	}
	}

	if (parg) {
		/* release the current value */
		lub_string_free(parg->value);
	} else {
		size_t new_size = ((this->pargc + 1) * sizeof(clish_parg_t *));
		clish_parg_t **tmp;

		/* resize the parameter vector */
		tmp = realloc(this->pargv, new_size);
		this->pargv = tmp;
		/* insert reference to the parameter */
		parg = malloc(sizeof(*parg));
		this->pargv[this->pargc++] = parg;
		parg->param = param;
	}
	parg->value = NULL;
	if (value)
		parg->value = lub_string_dup(value);

	return 0;
}

/*--------------------------------------------------------- */
clish_pargv_t *clish_pargv_new(void)
{
	clish_pargv_t *this;

	this = malloc(sizeof(clish_pargv_t));
	this->pargc = 0;
	this->pargv = NULL;

	return this;
}

/*--------------------------------------------------------- */
static void clish_pargv_fini(clish_pargv_t * this)
{
	unsigned i;

	/* cleanup time */
	for (i = 0; i < this->pargc; i++) {
		lub_string_free(this->pargv[i]->value);
		this->pargv[i]->value = NULL;
		free(this->pargv[i]);
	}
	free(this->pargv);
}

/*--------------------------------------------------------- */
void clish_pargv_delete(clish_pargv_t * this)
{
	if (!this)
		return;

	clish_pargv_fini(this);
	free(this);
}

/*--------------------------------------------------------- */
unsigned clish_pargv__get_count(clish_pargv_t * this)
{
	if (!this)
		return 0;
	return this->pargc;
}

/*--------------------------------------------------------- */
clish_parg_t *clish_pargv__get_parg(clish_pargv_t * this, unsigned index)
{
	if (!this)
		return NULL;
	if (index > this->pargc)
		return NULL;
	return this->pargv[index];
}

/*--------------------------------------------------------- */
const clish_param_t *clish_pargv__get_param(clish_pargv_t * this,
	unsigned index)
{
	clish_parg_t *tmp;

	if (!this)
		return NULL;
	if (index >= this->pargc)
		return NULL;
	tmp = this->pargv[index];
	return tmp->param;
}

/*--------------------------------------------------------- */
const char *clish_parg__get_value(const clish_parg_t * this)
{
	if (!this)
		return NULL;
	return this->value;
}

/*--------------------------------------------------------- */
const char *clish_parg__get_name(const clish_parg_t * this)
{
	if (!this)
		return NULL;
	return clish_param__get_name(this->param);
}

/*--------------------------------------------------------- */
const clish_ptype_t *clish_parg__get_ptype(const clish_parg_t * this)
{
	if (!this)
		return NULL;
	return clish_param__get_ptype(this->param);
}

/*--------------------------------------------------------- */
const clish_parg_t *clish_pargv_find_arg(clish_pargv_t * this, const char *name)
{
	if (!this)
		return NULL;
	return find_parg(this, name);
}

/*--------------------------------------------------------- */
