
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol_factory.h>
#include <thrift/c_glib/server/thrift_server.h>
#include <thrift/c_glib/server/thrift_simple_server.h>
#include <thrift/c_glib/transport/thrift_buffered_transport.h>
#include <thrift/c_glib/transport/thrift_buffered_transport_factory.h>
#include <thrift/c_glib/transport/thrift_framed_transport.h>
#include <thrift/c_glib/transport/thrift_framed_transport_factory.h>
#include <thrift/c_glib/transport/thrift_server_socket.h>
#include <thrift/c_glib/transport/thrift_server_transport.h>
#include <thrift/c_glib/transport/thrift_transport.h>
#include <thrift/c_glib/transport/thrift_transport_factory.h>
#include "sal.h"
#include "sai.h"
#include "ptf_agent.h"

extern void
ptf_agent_init(void* user_param);
extern void
ptf_agent_deinit();

/* A flag that indicates whether the server was interrupted with SIGINT
   (i.e. Ctrl-C) so we can tell whether its termination was abnormal */
int sigint_received = FALSE;

/* Handle SIGINT ("Ctrl-C") signals by gracefully stopping the server */
static void
sigint_handler (int signal_number)
{
    /* Take note we were called */
    sigint_received = TRUE;
    ptf_agent_deinit();
}

int
main (int argc, char **argv)
{
    GError *error = NULL;
    static int port = 9090;
    static
    GOptionEntry option_entries[] = 
    {
        { "port",            0, 0, G_OPTION_ARG_INT,      &port,
          "Port number to connect (=9090)", NULL },
        { NULL }
    };

    GOptionContext *option_context;
    struct sigaction sigint_action;

    /* Configure and parse our command-line options */
    option_context = g_option_context_new (NULL);
    g_option_context_add_main_entries (option_context,
                                     option_entries,
                                     NULL);
    if (g_option_context_parse (option_context,
                              &argc,
                              &argv,
                              &error) == FALSE) 
    {
        fprintf (stderr, "%s\n", error->message);
        return 255;
    }
    g_option_context_free (option_context);

    /* Install our SIGINT handler, which handles Ctrl-C being pressed by stopping
     the server gracefully */
    memset (&sigint_action, 0, sizeof (sigint_action));
    sigint_action.sa_handler = sigint_handler;
    sigint_action.sa_flags = SA_RESETHAND;
    sigaction (SIGINT, &sigint_action, NULL);
    
    ptf_agent_init(&port);
    return 0;
}

#ifdef PTF_STUB

sai_status_t
sai_api_query(sai_api_t sai_api_id, void** api_method_table)
{
    *api_method_table = NULL;
    return SAI_STATUS_SUCCESS;
}

#endif /* !PTF_STUB */

