#ifndef __CHSM_FM_H__
#define __CHSM_FM_H__

#ifdef HAVE_HA_CTC

#include "chsm_client.h"
#include "fm_client.h"

#define FM_SERVER_RESTART_INTERVAL					5
#define FM_SERVER_RESTART_COUNTER					20

/* Reset parser pointer and size. */
#define FM_RECV_BUFFER_RESET(C)                                             \
  do {                                                                        \
    (C)->size_in = 0;                                                         \
    (C)->pnt_in = (C)->buf_in;                                                \
  } while (0)

/*  fm server structure.  */
struct fm_server
{
  /* Lib globals. */
  struct lib_globals *zg;

  /* Pointer to protocol instance. */
  void *parent;

  /* Message handler. */
  struct message_handler *ms;

  /* Client session list. */
  struct list *session_list;

  /* Tie-break method. */
  u_char tie_break;

  /* Default retry interval. */
  u_int16_t def_retry_interval;

  /* Restart threads.  */
  struct thread *t_restart;

  /* Restart counter and interval.  */
  int restart_counter;
  int restart_interval;
};
  
struct fm_full_dump_sequence
{
    int module_id;
    int select;
};

struct fm_server_global
{
    int redundancyMode;
    int fulldumpStatus;
    int currentFulldumpPm;
    int fmSvrfd;
    struct  fm_server *fs;
};

extern struct fm_server_global g_fmServerGlobal;

int chsm_fm_get_redundancy_mode();
void chsm_fm_set_redundancy_mode(int mode);
int chsm_msg_tx_full_dump_status(int status);

int fm_server_send_ha_overswitch();
int fm_server_send_pm_full_dump_start();
int fm_server_init (struct lib_globals *zg);
void chsm_fm_init_fulldump_para();
void fm_server_control_full_dump_sequence();
int chsm_start_fulldump();
int fm_server_send_redundancy_mode(int mode);
#endif 
#endif /* __CHSM_FM_H__ */
