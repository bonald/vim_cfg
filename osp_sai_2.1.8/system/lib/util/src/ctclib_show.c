/****************************************************************************
 * hsrv_show.c      ctclib show file.
 *
 * Copyright     :(c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Xianting Huang
 * Date          :2010-07.
 * Reason        :First Create.
 ****************************************************************************/
#include "sal.h"

#ifdef _GLB_ENABLE_DBGSHOW_

#include "ctclib_error.h"
#include "ctclib_show.h"
#include "ctclib_list.h"

FILE * g_fp_dumpfile = NULL;

/*********************************************************************
 * Name    : ctclib_show_common_db
 * Purpose : This function will show a software database
 * Input   : void * p_database, 
             CTCLIB_SHOW_CB_LOOP_DATA pf_show_function_loop,
             CTCLIB_SHOW_CB_PRINT_ITEM pf_show_function_print
 * Output  : N/A
 * Return  : CTCLIB_E_NONE
 * Note    : 2010.8 init version
*********************************************************************/
int32
ctclib_show_common_db(void * p_database, 
                            CTCLIB_SHOW_CB_LOOP_DATA pf_show_function_loop,
                            CTCLIB_SHOW_CB_PRINT_ITEM pf_show_function_print)
{
    pf_show_function_loop(p_database, pf_show_function_print);
    
    return CTCLIB_E_NONE;
}
    
#endif /*_GLB_ENABLE_DBGSHOW_*/
