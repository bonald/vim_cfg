/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "sal.h"
#include "sal_task.h"
#include <glib-object.h>
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

#include "gen-c_glib/switch_sai_rpc.h"
#include "ptf_agent.h"
#include "sai_handler.h"

PTF_HDLR_LOG_OUT_FUNC PTF_HDLR_LOG = sal_printf;

int32
sai_set_out_func(PTF_HDLR_LOG_OUT_FUNC func)
{
    if (func)
    {
        PTF_HDLR_LOG = func;
    }
    return 0;
}

static ThriftServer *server = NULL;
static int32 server_port = 0;

void
ptf_agent_init(void* user_param)
{
#if (!GLIB_CHECK_VERSION (2, 36, 0))
    g_type_init ();
#endif

    gchar *server_name            = "ctc_ptf_server";
    gchar *transport_name         = "buffered";
    GType  transport_factory_type = THRIFT_TYPE_BUFFERED_TRANSPORT_FACTORY;
    gchar *protocol_name          = "binary";
    GType  protocol_factory_type  = THRIFT_TYPE_BINARY_PROTOCOL_FACTORY;

    switch_sai_rpcHandler  *handler = NULL;
    ThriftProcessor        *processor = NULL;
    ThriftServerTransport  *server_transport = NULL;
    ThriftTransportFactory *transport_factory = NULL;
    ThriftProtocolFactory  *protocol_factory = NULL;
    GError *error = NULL;
    int *p_port = NULL;

    if (user_param)
    {
        p_port = (int *)user_param;
        server_port = *p_port;
    }

    /* Establish all our connection objects */
    handler           = g_object_new (CTC_TYPE_SAI_RPC_HANDLER, NULL);
    processor         = g_object_new (TYPE_SWITCH_SAI_RPC_PROCESSOR,
                                        "handler", handler,
                                        NULL);
    server_transport  = g_object_new (THRIFT_TYPE_SERVER_SOCKET,
                                        "port", server_port,
                                        NULL);
    transport_factory = g_object_new (transport_factory_type,
                                        NULL);
    protocol_factory  = g_object_new (protocol_factory_type,
                                        NULL);

    server = g_object_new (THRIFT_TYPE_SIMPLE_SERVER,
                         "processor",                processor,
                         "server_transport",         server_transport,
                         "input_transport_factory",  transport_factory,
                         "output_transport_factory", transport_factory,
                         "input_protocol_factory",   protocol_factory,
                         "output_protocol_factory",  protocol_factory,
                         NULL);

    PTF_HDLR_LOG("Starting \"%s\" server (%s/%s) listen on: %d\n",
          server_name,
          transport_name,
          protocol_name,
          server_port);

    /* Serve clients until SIGINT is received (Ctrl-C is pressed) */
    thrift_server_serve (server, &error);

    g_object_unref (server);
    g_object_unref (protocol_factory);
    g_object_unref (transport_factory);
    g_object_unref (server_transport);
    g_object_unref (processor);
    g_object_unref (handler);

    return;
}

void
ptf_agent_deinit()
{
    /* Shut down the server gracefully */
    if (server != NULL)
    {
        thrift_server_stop (server);
    }

    server = NULL;
    return;
}

