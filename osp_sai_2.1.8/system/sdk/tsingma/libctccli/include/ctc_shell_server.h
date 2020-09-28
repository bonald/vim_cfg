/**
 @file ctc_master_cli_server.h

 @date 2009-10-27

 @version v2.0

The file define  chipset  independent common Macros and constant.
*/

#ifndef _CTC_MASTER_CLI_SERVER_H
#define _CTC_MASTER_CLI_SERVER_H
#ifdef __cplusplus
extern "C" {
#endif

extern int
ctc_vty_socket();

extern void 
ctc_vty_close();

#ifdef __cplusplus
}
#endif

#endif /* _CTC_MASTER_CLI_SERVER_H*/
