/****************************************************************************
 * hsrv_show.c      ctclib show header file.
 *
 * Copyright     :(c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Xianting Huang
 * Date          :2010-07.
 * Reason        :First Create.
 ****************************************************************************/
#ifndef __CTCLIB_SHOW_H__
#define __CTCLIB_SHOW_H__
#include "sal.h"

#ifdef _GLB_ENABLE_DBGSHOW_
#include "glb_tempfile_define.h"

#define CTCLIB_DUMP_FILE_NAME  GLB_DUMP_FILE

#define CTCLIB_DUMP_OPEN_FILE()                          \
extern FILE * g_fp_dumpfile;                             \
do{                                                      \
    if(g_fp_dumpfile)                                    \
    {                                                    \
        fclose(g_fp_dumpfile);                           \
        unlink(CTCLIB_DUMP_FILE_NAME);                   \
        g_fp_dumpfile = NULL;                            \
    }                                                    \
                                                         \
    g_fp_dumpfile = fopen(CTCLIB_DUMP_FILE_NAME, "w+");  \
    if(NULL == g_fp_dumpfile)                            \
    {                                                    \
        SAL_ASSERT(0);                                   \
    }                                                    \
}while(0);

#define CTCLIB_DUMP_CLOSE_FILE()                         \
extern FILE * g_fp_dumpfile;                             \
do{                                                      \
    if(g_fp_dumpfile)                                    \
    {                                                    \
        fclose(g_fp_dumpfile);                           \
        g_fp_dumpfile = NULL;                            \
    }                                                    \
}while(0);

#define CTCLIB_DUMP_DELETE_FILE()                        \
extern FILE * g_fp_dumpfile;                             \
do{                                                      \
    if(g_fp_dumpfile)                                    \
    {                                                    \
        unlink(CTCLIB_DUMP_FILE_NAME);                   \
        g_fp_dumpfile = NULL;                            \
    }                                                    \
}while(0)

#define CTCLIB_DUMP_CLOSE_DELETE_FILE()                  \
extern FILE * g_fp_dumpfile;                             \
do{                                                      \
    if(g_fp_dumpfile)                                    \
    {                                                    \
        fclose(g_fp_dumpfile);                           \
        unlink(CTCLIB_DUMP_FILE_NAME);                   \
        g_fp_dumpfile = NULL;                            \
    }                                                    \
}while(0)

#define CTCLIB_DUMP_OUT_FILE_LINE(cli)                   \
extern FILE * g_fp_dumpfile;                             \
do{                                                      \
    char buf[256];                                       \
                                                         \
    g_fp_dumpfile = fopen(CTCLIB_DUMP_FILE_NAME, "r");   \
    if (g_fp_dumpfile == NULL)                           \
    {                                                    \
        cli_out(cli, "%% Internal error\n");             \
        return CLI_ERROR;                                \
    }                                                    \
                                                         \
    while (fgets (buf, 256, g_fp_dumpfile))              \
    {                                                    \
        cli_out (cli, "%s", buf);                        \
    }                                                    \
}while(0);

#define CTCLIB_DUMP_OUT_TO_FILE(fmt, arg...)             \
{                                                        \
    extern FILE * g_fp_dumpfile;                         \
    do{                                                  \
        sal_fprintf(g_fp_dumpfile, fmt, ##arg);          \
    }while(0);                                           \
}

#define CTCLIB_DUMP_OUT CTCLIB_DUMP_OUT_TO_FILE

#define CTCLIB_DUMP_PREPARE()                            \
CTCLIB_DUMP_OPEN_FILE();

#define CTCLIB_DUMP_FINISH(cli)                          \
CTCLIB_DUMP_CLOSE_FILE();                                \
CTCLIB_DUMP_OUT_FILE_LINE(cli);                          \
CTCLIB_DUMP_CLOSE_DELETE_FILE();

/* ======= debug call back functions part: =======*/

/*call back function to print out each item in table.
Arg1 "void *" is the pointer to the data node*/
typedef int32(*CTCLIB_SHOW_CB_PRINT_ITEM) (void*);

/*call back function to print out each item in table.
Arg1 "void *" is the pointer to the database.
Arg2 "CTCLIB_SHOW_CB_PRINT_ITEM" is the call back function to print each node*/
typedef int32(*CTCLIB_SHOW_CB_LOOP_DATA) (void*, CTCLIB_SHOW_CB_PRINT_ITEM);

/* ======= function declare part: =======*/
int32
ctclib_show_common_db(void * p_database, 
                            CTCLIB_SHOW_CB_LOOP_DATA pf_show_function_loop,
                            CTCLIB_SHOW_CB_PRINT_ITEM pf_show_function_print);

#endif /*_GLB_ENABLE_DBGSHOW_*/
#endif /*__CTCLIB_SHOW_H__*/
