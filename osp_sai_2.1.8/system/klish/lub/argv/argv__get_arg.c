/*
 * argv__get_arg.c
 */
#include "private.h"

/*--------------------------------------------------------- */
const char *lub_argv__get_arg(const lub_argv_t * this, unsigned index)
{
	const char *result = NULL;

	if (this->argc > index)
		result = this->argv[index].arg;

	return result;
}

int
lub_argv__get_argc(const lub_argv_t * this)
{
    return (int)this->argc;
}

/*--------------------------------------------------------- */
