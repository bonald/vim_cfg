/****************************************************************************
 * ctc_vti.h :         vti header
 *
 * Copyright (C) 2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         V1.0
 * Date           :         2010-7-28
 * Reason         :         First Create
 ****************************************************************************/

#ifndef _CTC_VTI_H
#define _CTC_VTI_H

#ifdef __cplusplus
extern "C" {
#endif

#define CTC_VTI_BUFSIZ 1536
#define CTC_VTI_MAXHIST 50

enum ctc_shell_mode_type_e
{
    CTC_VTI_SHELL_MODE_DEFAULT = 0,
    CTC_VTI_SHELL_MODE_USER,
    CTC_VTI_SHELL_MODE_MAx,
};
typedef enum ctc_shell_mode_type_e shell_mode_type_t;

/* Is this vti connect to file or not */
enum ctc_terminal_type_e
{
    CTC_VTI_TERM,
    CTC_VTI_FILE,
    CTC_VTI_SHELL,
    CTC_VTI_SHELL_SERV
};
typedef enum ctc_terminal_type_e terminal_type_t;

/* terminal status*/
enum ctc_terminal_stats_e
{
    CTC_VTI_NORMAL,
    CTC_VTI_CLOSE,
    CTC_VTI_MORE,
    CTC_VTI_MORELINE,
    CTC_VTI_START,
    CTC_VTI_CONTINUE
};
typedef enum ctc_terminal_stats_e ctc_terminal_stats_t;

/* VTI struct. */
struct ctc_vti_struct_s
{
    /* File descripter of this vty. */
    int32 fd;

    int (*printf)(struct ctc_vti_struct_s* vti, const char *szPtr, const int szPtr_len);

    int (*grep)(struct ctc_vti_struct_s* vti, const char *szPtr, const int szPtr_len);

    int (*quit)(struct ctc_vti_struct_s* vti);    

    bool grep_en;

    bool fprintf;

    bool for_en;

    int32 for_cnt;

    int32 sort_lineth;
    
    uint64 cmd_usec;

    int (*output)(const char *szPtr, const int szPtr_len, void*);/*used in user define shell mode(ctc_shell_mode = 2)*/

    terminal_type_t type;

    /* Node status of this vty */
    int32 node;

    /* What address is this vty comming from. */
    char* address;

    /* Privilege level of this vty. */
    int32 privilege;

    /* Failure count */
    int32 fail;

    /* Command input buffer */
    char* buf;

    /* Command cursor point */
    int32 cp;

    /* Command length */
    int32 length;

    /* Command max length. */
    int32 max;

    /* Histry of command */
    char* hist[CTC_VTI_MAXHIST];

    /* History lookup current point */
    int32 hp;

    /* History insert end point */
    int32 hindex;

    /* For current referencing point of interface, route-map,
       access-list etc... */
    void* index;

    /* For multiple level index treatment such as key chain and key. */
    void* index_sub;

    /* For escape character. */
    unsigned char escape;

    /* Current vty status. */
    ctc_terminal_stats_t status;

    /* IAC handling */
    unsigned char iac;

    /* IAC SB handling */
    unsigned char iac_sb_in_progress;

    /* Window width/height. */
    int32 width;

    int32 height;

    int32 scroll_one;

    /* Configure lines. */
    int32 lines;

    /* Terminal monitor. */
    int32 monitor;

    /* In configure mode. */
    int32 config;

    /* Timeout seconds and thread. */
    unsigned long v_timeout;
        

};
typedef struct ctc_vti_struct_s ctc_vti_t;

/* Small macro to determine newline is newline only or linefeed needed. */
#define CTC_VTI_NEWLINE "\n\r"

/* Default time out value */
#define CTC_VTI_TIMEOUT_DEFAULT 600

/* Vty read buffer size. */
#define CTC_VTI_READ_BUFSIZ 1536

/* GCC have printf type attribute check.  */
#ifdef __GNUC__
#define PRINTF_ATTRIBUTE(a, b) __attribute__ ((__format__(__printf__, a, b)))
#else
#define PRINTF_ATTRIBUTE(a, b)
#endif /* __GNUC__ */
/* Prototypes. */
void ctc_vti_init(int32 mode);
ctc_vti_t* ctc_vti_new(void);
int32 ctc_vti_out(ctc_vti_t*, const char*, ...) PRINTF_ATTRIBUTE(2, 3);
int32 ctc_vti_config_lock(ctc_vti_t*);
int32 ctc_vti_config_unlock(ctc_vti_t*);
void ctc_vti_prompt(ctc_vti_t* vti);


typedef int32 (* CTC_CB_VTI_CMD_OUT_FUN_P)  (const char *szPtr, const int szPtr_len, void*); 
/**
 @brief input commands to SDK

 @param[in] buf   input string

 @param[in] buf_len   string length

 @remark  Used in ctc_shell_mode 2 which defined at userinit(). User can input commands to SDK by the function.

 @return 0(sucess) or -1(fail)

*/
int32 
ctc_vti_cmd_input(const int8* buf, const uint32 buf_len);

/**
 @brief register callback function for commands process result

 @remark  Used in ctc_shell_mode 2 which defined at userinit(). The string of process result will send to user by the callback. 

 @return 0(sucess) or -1(fail)

*/
int32 
ctc_vti_cmd_output_register(CTC_CB_VTI_CMD_OUT_FUN_P print);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_VTI_H */

