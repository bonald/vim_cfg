
#ifndef __CTCLIB_SYS_CMD_H__
#define __CTCLIB_SYS_CMD_H__

typedef int (*sal_sys_cmd_clone_fn)(void*);

int ctclib_reconstruct_system_cmd_exec_str(void* sys_cmd);
int32 ctclib_reconstruct_system_cmd_chld_clone(sal_sys_cmd_clone_fn func,char *cmdstring);

int32 
ctclib_string_replace_substr(char* str, size_t strLen, const char* d, const char* s);

#endif

