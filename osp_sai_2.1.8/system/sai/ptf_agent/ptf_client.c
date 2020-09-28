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

#include <glib-object.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>

#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol.h>
#include <thrift/c_glib/transport/thrift_buffered_transport.h>
#include <thrift/c_glib/transport/thrift_framed_transport.h>
#include <thrift/c_glib/transport/thrift_socket.h>
#include <thrift/c_glib/transport/thrift_transport.h>

#include "gen-c_glib/switch_sai_rpc.h"
#include "sai.h"

/* Handle SIGPIPE signals (indicating the server has closed the
   connection prematurely) by outputting an error message before
   exiting. */
static void
sigpipe_handler (int signal_number)
{
  THRIFT_UNUSED_VAR (signal_number);

  /* Flush standard output to make sure the test results so far are
     logged */
  fflush (stdout);

  fputs ("Broken pipe (server closed connection prematurely)\n", stderr);
  fflush (stderr);

  /* Re-raise the signal, this time invoking the default signal
     handler, to terminate the program */
  raise (SIGPIPE);
}

/* Compare two gint32 values. Used for sorting and finding integer
   values within a GList. */
gint
gint32_compare (gconstpointer a, gconstpointer b)
{
  gint32 int32_a = *(gint32 *)a;
  gint32 int32_b = *(gint32 *)b;
  int result = 0;

  if (int32_a < int32_b)
    result = -1;
  else if (int32_a > int32_b)
    result = 1;

  return result;
}

int
main (int argc, char **argv)
{
  static gchar *host = NULL;
  static gint   port = 9090;
  static gchar *transport_option = NULL;
  static gchar *protocol_option = NULL;

  static
    GOptionEntry option_entries[] ={
    { "host",            0, 0, G_OPTION_ARG_STRING,   &host,
      "Host to connect (=localhost)", NULL },
    { "port",            0, 0, G_OPTION_ARG_INT,      &port,
      "Port number to connect (=9090)", NULL },
    { "transport",       0, 0, G_OPTION_ARG_STRING,   &transport_option,
      "Transport: buffered, framed (=buffered)", NULL },
    { "protocol",        0, 0, G_OPTION_ARG_STRING,   &protocol_option,
      "Protocol: binary (=binary)", NULL },
    { NULL }
  };

  struct sigaction sigpipe_action;

#if (!GLIB_CHECK_VERSION (2, 36, 0))
  g_type_init ();
#endif

  GType  transport_type = THRIFT_TYPE_BUFFERED_TRANSPORT;
  gchar *transport_name = "buffered";
  GType  protocol_type  = THRIFT_TYPE_BINARY_PROTOCOL;
  gchar *protocol_name  = "binary";

  ThriftSocket    *socket;
  ThriftTransport *transport;
  ThriftProtocol  *protocol;

  switch_sai_rpcIf *test_client;

  struct timeval time_start, time_stop, time_elapsed;
  guint64 time_elapsed_usec, time_total_usec = 0;
  guint64 time_min_usec = G_MAXUINT64, time_max_usec = 0;

  GOptionContext *option_context;
  gboolean options_valid = TRUE;
  int fail_count = 0;
  GError *error = NULL;


  /* Configure and parse our command-line options */
  option_context = g_option_context_new (NULL);
  g_option_context_add_main_entries (option_context,
                                     option_entries,
                                     NULL);
  if (!g_option_context_parse (option_context,
                               &argc,
                               &argv,
                               &error)) {
    fprintf (stderr, "%s\n", error->message);
    return 255;
  }
  g_option_context_free (option_context);

  /* Set remaining default values for unspecified options */
  if (host == NULL)
    host = g_strdup ("localhost");

  /* Validate the parsed options */
  if (protocol_option != NULL &&
      strncmp (protocol_option, "binary", 7) != 0) {
    fprintf (stderr, "Unknown protocol type %s\n", protocol_option);
    options_valid = FALSE;
  }

  if (transport_option != NULL) {
    if (strncmp (transport_option, "framed", 7) == 0) {
      transport_type = THRIFT_TYPE_FRAMED_TRANSPORT;
      transport_name = "framed";
    }
    else if (strncmp (transport_option, "buffered", 9) != 0) {
      fprintf (stderr, "Unknown transport type %s\n", transport_option);
      options_valid = FALSE;
    }
  }

  if (!options_valid)
    return 254;

  printf ("Connecting (%s/%s) to: %s:%d\n",
          transport_name,
          protocol_name,
          host,
          port);

  /* Install our SIGPIPE handler, which outputs an error message to
     standard error before exiting so testers can know what
     happened */
  memset (&sigpipe_action, 0, sizeof (sigpipe_action));
  sigpipe_action.sa_handler = sigpipe_handler;
  sigpipe_action.sa_flags = SA_RESETHAND;
  sigaction (SIGPIPE, &sigpipe_action, NULL);

  /* Establish all our connection objects */
  socket = g_object_new (THRIFT_TYPE_SOCKET,
                         "hostname", host,
                         "port",     port,
                         NULL);
  transport = g_object_new (transport_type,
                            "transport", socket,
                            NULL);
  protocol = g_object_new (protocol_type,
                           "transport", transport,
                           NULL);
  test_client = g_object_new (TYPE_SWITCH_SAI_RPC_CLIENT,
                              "input_protocol",  protocol,
                              "output_protocol", protocol,
                              NULL);

    /* Execute the actual tests */
    if (thrift_transport_open (transport, &error)) 
    {

        gettimeofday (&time_start, NULL);

#if 0
        gchar  *string = NULL;
        UserId user_id;

        /* Thrift basic test */
        printf ("testVoid()");
        if (switch_sai_rpc_if_test_void (test_client, &error)) 
        {
            printf (" = void\n");
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        printf ("testString(\"Test\")");
        if (switch_sai_rpc_if_test_string (test_client,
                                         &string,
                                         "Test",
                                         &error))
        {
            printf (" = \"%s\"\n", string);
            if (strncmp (string, "Test", 5) != 0)
                fail_count++;

            g_free (string);
            string = NULL;
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        printf ("testTypedef(309858235082523)");
        if (switch_sai_rpc_if_test_typedef (test_client,
                                          &user_id,
                                          309858235082523LL,
                                          &error))
        {
            printf(" = %" PRId64 "\n", user_id);
            if (user_id != 309858235082523LL)
              fail_count++;
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }
        /* Thrift basic test end */
#endif

        gint32 rc = 0;
        sai_thrift_object_id_t vr_id = 0;
        sai_thrift_attribute_list_t *rc_attr_list = NULL;

        sai_thrift_attribute_t *attr[3];
        sai_thrift_attribute_value_t *value[3];

        sai_thrift_vlan_port_t *vlan_ports[3];
        GPtrArray *ptr_array = NULL;
        sai_thrift_fdb_entry_t *fdb_entry = NULL;
        char mac[6];
        gint64 port = 0;

        mac[0] = 0x11;
        mac[1] = 0x22;
        mac[2] = 0x33;
        mac[3] = 0x44;
        mac[4] = 0x55;
        mac[5] = 0x66;

        fdb_entry = g_object_new (TYPE_SAI_THRIFT_FDB_ENTRY_T,
                                "mac_address", mac,
                                "vlan_id", 10,
                                NULL);


#define CTC_SAI_OBJECT_TYPE_SET(objtype,index)  (((guint64)objtype << 32) | (guint64)index)
        port = CTC_SAI_OBJECT_TYPE_SET(1, 10);
    
        /* 1. create_fdb_entry */
        printf ("1. create_fdb_entry()");
        value[0] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "u8", 1,
                                NULL);
        value[1] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "oid", port,
                                NULL);
        value[2] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "u8", 2,
                                NULL);
      
        attr[0] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 0,  /* SAI_FDB_ENTRY_ATTR_TYPE */
                                "value", value[0],
                                NULL);
        attr[1] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 1,  /* SAI_FDB_ENTRY_ATTR_PORT_ID */
                                "value", value[1],
                                NULL);
        attr[2] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 2,  /* SAI_FDB_ENTRY_ATTR_PACKET_ACTION */
                                "value", value[2],
                                NULL);

        ptr_array = g_ptr_array_new();
        g_ptr_array_add (ptr_array, attr[0]);
        g_ptr_array_add (ptr_array, attr[1]);
        g_ptr_array_add (ptr_array, attr[2]);
        if (switch_sai_rpc_if_sai_thrift_create_fdb_entry (test_client,
                                              &rc,
                                              fdb_entry,
                                              ptr_array,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }
        
        printf ("2. delete_fdb_entry()");
        if (switch_sai_rpc_if_sai_thrift_delete_fdb_entry (test_client,
                                              &rc,
                                              fdb_entry,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }


        value[0] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "u16", 9,
                                NULL);
        value[1] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "u8", 1, /* SAI_FDB_FLUSH_ENTRY_STATIC */
                                NULL);

        attr[0] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 1,  /* SAI_FDB_FLUSH_ATTR_VLAN_ID */
                                "value", value[0],
                                NULL);
        attr[1] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 2,  /* SAI_FDB_FLUSH_ATTR_ENTRY_TYPE */
                                "value", value[1],
                                NULL);
        
        ptr_array = g_ptr_array_new();
        g_ptr_array_add (ptr_array, attr[0]);
        g_ptr_array_add (ptr_array, attr[1]);

        printf ("3. flush_fdb_entries()");
        if (switch_sai_rpc_if_sai_thrift_flush_fdb_entries (test_client,
                                              &rc,
                                              ptr_array,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        printf ("4. create_vlan()");
        if (switch_sai_rpc_if_sai_thrift_create_vlan (test_client,
                                              &rc,
                                              10,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }
        
        printf ("5. delete_vlan()");
        if (switch_sai_rpc_if_sai_thrift_delete_vlan (test_client,
                                              &rc,
                                              10,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        ptr_array = g_ptr_array_new();        
        vlan_ports[0] = g_object_new (TYPE_SAI_THRIFT_VLAN_PORT_T,
                                "port_id", 0x10000000A,
                                "tagging_mode", 1,
                                NULL);
        vlan_ports[1] = g_object_new (TYPE_SAI_THRIFT_VLAN_PORT_T,
                                "port_id", 0x10000000B,
                                "tagging_mode", 2,
                                NULL);
        vlan_ports[2] = g_object_new (TYPE_SAI_THRIFT_VLAN_PORT_T,
                                "port_id", 0x10000000C,
                                "tagging_mode", 3,
                                NULL);

        g_ptr_array_add (ptr_array, vlan_ports[0]);
        g_ptr_array_add (ptr_array, vlan_ports[1]);
        g_ptr_array_add (ptr_array, vlan_ports[2]);
        
        printf ("6. add_ports_to_vlan()");
        if (switch_sai_rpc_if_sai_thrift_add_ports_to_vlan (test_client,
                                              &rc,
                                              10,
                                              ptr_array,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }


        printf ("7. remove_ports_from_vlan()");
        if (switch_sai_rpc_if_sai_thrift_remove_ports_from_vlan (test_client,
                                              &rc,
                                              11,
                                              ptr_array,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        value[0] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "booldata", 1,
                                NULL);
        value[1] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T,
                                "booldata", 1,
                                NULL);
        attr[0] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 0,  /* SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE */
                                "value", value[0],
                                NULL);
        attr[1] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T,
                                "id", 1,  /* SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE */
                                "value", value[1],
                                NULL);

        ptr_array = g_ptr_array_new();
        g_ptr_array_add (ptr_array, attr[0]);
        g_ptr_array_add (ptr_array, attr[1]);
        
        printf ("8. create_virtual_router()\n");
        if (switch_sai_rpc_if_sai_thrift_create_virtual_router (test_client,
                                              &vr_id,
                                              ptr_array,
                                              &error))
        {
            //printf(" rc vr_id = 0x%lx\n", (uint32_t)vr_id);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        vr_id = 0x300000001;
        printf ("9. remove_virtual_router()");
        if (switch_sai_rpc_if_sai_thrift_remove_virtual_router (test_client,
                                              &rc,
                                              vr_id,
                                              &error))
        {
            printf(" rc = %d\n", rc);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        printf ("30. get_switch_attribute()");
        if (switch_sai_rpc_client_sai_thrift_get_switch_attribute (test_client,
                                              &rc_attr_list,
                                              &error))
        {
            printf(" attr list count = %d\n", rc_attr_list->attr_count);
        }
        else
        {
            printf ("%s\n", error->message);
            g_error_free (error);
            error = NULL;

            fail_count++;
        }

        gettimeofday (&time_stop, NULL);
        timersub (&time_stop, &time_start, &time_elapsed);
        time_elapsed_usec = time_elapsed.tv_sec * 1000 * 1000 + time_elapsed.tv_usec;

        printf("Total time: %" PRIu64 " us\n", time_elapsed_usec);

        time_total_usec += time_elapsed_usec;
        if (time_elapsed_usec < time_min_usec)
            time_min_usec = time_elapsed_usec;
        if (time_elapsed_usec > time_max_usec)
            time_max_usec = time_elapsed_usec;
        thrift_transport_close (transport, &error);
    }
    else
    {
        printf ("Connect failed: %s\n", error->message);
        g_error_free (error);
        error = NULL;

        return 1;
    }

    /* All done---output statistics */
    puts ("\nAll tests done.");

    g_object_unref (test_client);
    g_object_unref (protocol);
    g_object_unref (transport);
    g_free (host);

    return fail_count;
}

#ifdef PTF_STUB

sai_status_t
sai_api_query(sai_api_t sai_api_id, void** api_method_table)
{
    *api_method_table = NULL;
    return SAI_STATUS_SUCCESS;
}

#endif /* !PTF_STUB */
