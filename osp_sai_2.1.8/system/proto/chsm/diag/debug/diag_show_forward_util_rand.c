/*****************************************************************************
 * ctcutil_rand.h    random number generate
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:    R0.01
 * Author:      Jack Yang
 * Date:        2005-08-24
 * Reason:      Initial version
 *****************************************************************************/

/* todo: error code */

#include "pal.h"
#include <stdlib.h>
#include <time.h>

//#include "ctcutil_rand.h"


/****************************************************************************
 * Name		: ctcutil_rand
 * Purpose	: generate a random number
 * Input	: min: the minimum number
 *                max: the maximum number
 * Output	: n/a
 * Return	: 
 * Note		:
 ****************************************************************************/
int32
diag_show_forward_ctcutil_rand(uint32 min, uint32 max, uint32 *p_value)
{
    if (min >= max)
        return -1;

//    srand(time(NULL));
    *p_value = ((uint32)((float)(max - min + 1) * rand() / (RAND_MAX + 1.0))
                + min);

    return 0;
}

