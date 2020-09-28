#ifndef __LC_DEFINE_H__
#define __LC_DEFINE_H__

/* for clish <--> psm */
#define SUPER_SOCK_PATH     "/tmp/.cfm_ctrl_super"

/* for lcm <--> lcsh */
#define LC_SOCK_PATH        "/tmp/.cfm_ctrl_lc"

#define FULL_NAME_FLASH              "/mnt/flash"
#define FULL_NAME_CFCARD             "/mnt/cf"
#define FULL_NAME_UDISK              "/mnt/udisk"
#define SHOW_NAME_FLASH              "flash:"

#define M_SYS_PRO_PID_FILE "/var/run/syspro.pid"
#define M_USR_SIG1_CMD_FILE "/tmp/usrsig1.cmd"
#define M_USR_SIG1_ACK_FILE "/tmp/sys_proc_sig"
/* full path name length */
#define M_FULLPATH_MAX_LEN      256
#define M_FULLPATH_DEPTH_MAX    32

#endif /* !__LC_DEFINE_H__ */
