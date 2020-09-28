
#include <string.h>
#include <signal.h>
#include <pthread.h>


#include "rest_api.h"
#include "insight.h"
#include "log_api.h"
#include "modulemgr.h"
#include "packet_multiplexer.h"
extern INSIGHT_STATUS insight_system_init ();
extern INSIGHT_STATUS insight_bst_init ();

  pthread_t rest_thread;

void insight_agent_init(void *param)
{


  /*Initialize logging, Not handling error as openlog() does not return anything*/
  logging_init();

  /*Initialize Module manager*/
  if (modulemgr_init() != INSIGHT_STATUS_SUCCESS)
  {
    LOG_POST (INSIGHT_LOG_CRITICAL, "Failed to initialize Module Manager\r\n");
  }

  /* Initialize Packet Multiplexer application*/
  if (pkt_mux_init() != INSIGHT_STATUS_SUCCESS)
  {
    LOG_POST (INSIGHT_LOG_CRITICAL,
             "Failed to initialize Packet Multiplexerr\n");
  }

  mem_mgr_init();


  /*Initialize system utils application*/
  if (insight_system_init() != INSIGHT_STATUS_SUCCESS)
  {
    LOG_POST (INSIGHT_LOG_CRITICAL, "Failed to initialize system_utils application\r\n");
  }

  insight_bst_init();

#if 0
/*Initialize REST*/
if (rest_init() != INSIGHT_STATUS_SUCCESS)
{
    LOG_POST (INSIGHT_LOG_CRITICAL, "Failed to initialize REST \n All components must be De-initialized\r\n");
}
#else

/* create pthread for system_utils application */
if (0 != pthread_create (&rest_thread, NULL, (void *) &rest_init, NULL))
{
    LOG_POST (INSIGHT_LOG_EMERGENCY, "Failed to rest_init %d\r\n", errno);
}
#endif

}

extern INSIGHT_STATUS rest_deinit(void);

STATIC void
insight_ctrl_c_handler(int sig)
{

    sal_printf("insight_ctrl_c_handler\n");
    rest_deinit();
    exit(0);
}

extern int32
userinit(uint8 ctc_shell_mode, uint8 ctcs_api_en, void* p_init_config);
extern int32
ctc_cli_start(int32 ctc_shell_mode);

int main(int argc, char **argv)
{

    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_handler = insight_ctrl_c_handler;
    sigact.sa_flags = SA_NODEFER;
    if (sigaction(SIGINT, &sigact, NULL) < 0)
    {
        return -1;
    }

    userinit(0, 1, NULL);
    insight_agent_init(NULL);
    ctc_cli_start(0);


    return 0;
}
