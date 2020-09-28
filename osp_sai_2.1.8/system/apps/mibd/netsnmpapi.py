#
# python-netsnmpagent module
# Copyright (c) 2013 Pieter Hollants <pieter@hollants.com>
# Licensed under the GNU Public License (GPL) version 3
#
# net-snmp C API abstraction module
#

import ctypes, ctypes.util

c_sizet_p = ctypes.POINTER(ctypes.c_size_t)

# Make libnetsnmpagent available via Python's ctypes module. We do this globally
# so we can define C function prototypes

# Workaround for net-snmp 5.4.x that has a bug with unresolved dependencies
# in its libraries (http://sf.net/p/net-snmp/bugs/2107): load netsnmphelpers
# first
#try:
#    libnsh = ctypes.cdll.LoadLibrary(ctypes.util.find_library("netsnmphelpers"))
#except:
#    raise Exception("Could not load libnetsnmphelpers! Is net-snmp installed?")
try:
    #libnsa = ctypes.cdll.LoadLibrary(ctypes.util.find_library("netsnmpagent"))
    libnsa = ctypes.cdll.LoadLibrary('libnetsnmpagent.so')
except:
    raise Exception("Could not load libnetsnmpagent! Is net-snmp installed?")

# net-snmp <5.6.x had various functions in libnetsnmphelpers.so that were moved
# to libnetsnmpagent.so in later versions. Use netsnmp_create_watcher_info as
# a test and define a libnsX handle to abstract from the actually used library
# version.
#try:
#    libnsa.netsnmp_create_watcher_info
libnsX = libnsa
#except AttributeError:
#    libnsX = libnsh

# include/net-snmp/library/callback.h

# Callback major types 
SNMP_CALLBACK_LIBRARY                   = 0
SNMP_CALLBACK_APPLICATION               = 1

# SNMP_CALLBACK_LIBRARY minor types 
SNMP_CALLBACK_LOGGING                   = 4
SNMP_CALLBACK_USER_FUNC_1               = 9

SNMPCallback = ctypes.CFUNCTYPE(
    ctypes.c_int,                       # result type
    ctypes.c_int,                       # int majorID
    ctypes.c_int,                       # int minorID
    ctypes.c_void_p,                    # void *serverarg
    ctypes.c_void_p                     # void *clientarg
)

for f in [ libnsa.snmp_register_callback ]:
    f.argtypes = [
        ctypes.c_int,                   # int major
        ctypes.c_int,                   # int minor
        SNMPCallback,                   # SNMPCallback *new_callback
        ctypes.c_void_p                 # void *arg
    ]
    f.restype = int

# include/net-snmp/agent/agent_callbacks.h
SNMPD_CALLBACK_INDEX_STOP               = 11

# include/net-snmp/library/snmp_logging.h
LOG_EMERG                               = 0 # system is unusable
LOG_ALERT                               = 1 # action must be taken immediately
LOG_CRIT                                = 2 # critical conditions
LOG_ERR                                 = 3 # error conditions
LOG_WARNING                             = 4 # warning conditions
LOG_NOTICE                              = 5 # normal but significant condition
LOG_INFO                                = 6 # informational
LOG_DEBUG                               = 7 # debug-level messages

class snmp_log_message(ctypes.Structure): pass
snmp_log_message_p = ctypes.POINTER(snmp_log_message)
snmp_log_message._fields_ = [
    ("priority",            ctypes.c_int),
    ("msg",                 ctypes.c_char_p)
]

for f in [ libnsa.snmp_log ]:
    f.restype = ctypes.c_int

# include/net-snmp/library/snmp_api.h
SNMPERR_SUCCESS                         = 0

# include/net-snmp/library/default_store.h
NETSNMP_DS_LIBRARY_ID                   = 0
NETSNMP_DS_APPLICATION_ID               = 1
NETSNMP_DS_LIB_PERSISTENT_DIR           = 8

for f in [ libnsa.netsnmp_ds_set_boolean ]:
    f.argtypes = [
        ctypes.c_int,                   # int storeid
        ctypes.c_int,                   # int which
        ctypes.c_int                    # int value
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.netsnmp_ds_set_string ]:
    f.argtypes = [
        ctypes.c_int,                   # int storeid
        ctypes.c_int,                   # int which
        ctypes.c_char_p                 # const char *value
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.netsnmp_ds_set_int ]:
    f.argtypes = [
        ctypes.c_int,                # int storeid
        ctypes.c_int,                # int which
        ctypes.c_int                 # int value
    ]
    f.restype = ctypes.c_int

# include/net-snmp/agent/ds_agent.h
NETSNMP_DS_AGENT_ROLE                   = 1
NETSNMP_DS_AGENT_X_SOCKET               = 1
NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL    = 3
NETSNMP_DS_AGENT_AGENTX_RETRIES            = 5

# include/net-snmp/library/snmp.h
SNMP_ERR_NOERROR                        = 0
SNMP_ERR_TOOBIG                            = 1
SNMP_ERR_NOSUCHNAME                        = 2
SNMP_ERR_BADVALUE                        = 3
SNMP_ERR_READONLY                        = 4
SNMP_ERR_GENERR                            = 5
SNMP_ERR_NOACCESS                        = 6
SNMP_ERR_WRONGTYPE                        = 7
SNMP_ERR_WRONGLENGTH                    = 8
SNMP_ERR_WRONGENCODING                    = 9
SNMP_ERR_WRONGVALUE                        = 10
SNMP_ERR_NOCREATION                        = 11
SNMP_ERR_INCONSISTENTVALUE                = 12
SNMP_ERR_RESOURCEUNAVAILABLE            = 13
SNMP_ERR_COMMITFAILED                    = 14
SNMP_ERR_UNDOFAILED                        = 15
SNMP_ERR_AUTHORIZATIONERROR                = 16
SNMP_ERR_NOTWRITABLE                    = 17
SNMP_ERR_INCONSISTENTNAME                = 18

for f in [ libnsa.get_enterprise_id, libnsa.oem_init ]:
    f.argtypes = None
    f.restype = ctypes.c_int

for f in [ libnsa.init_snmp ]:
    f.argtypes = [
        ctypes.c_char_p                 # const char *type
    ]
    f.restype = None

for f in [ libnsa.snmp_shutdown ]:
    f.argtypes = [
        ctypes.c_char_p                 # const char *type
    ]
    f.restype = None

# include/net-snmp/library/oid.h
c_oid   = ctypes.c_ulong
c_oid_p = ctypes.POINTER(c_oid)

# include/net-snmp/types.h
MAX_OID_LEN                             = 128

# include/net-snmp/agent/snmp_vars.h
for f in [ libnsa.init_agent ]:
    f.argtypes = [
        ctypes.c_char_p                 # const char *app
    ]
    f.restype = ctypes.c_int

# include/net-snmp/library/parse.h
class tree(ctypes.Structure): pass

# include/net-snmp/mib_api.h
for f in [ libnsa.init_mib ]:
    f.argtypes = None
    f.restype = None

for f in [ libnsa.read_mib ]:
    f.argtypes = [
        ctypes.c_char_p                 # const char *filename
    ]
    f.restype = ctypes.POINTER(tree)

for f in [ libnsa.read_objid ]:
    f.argtypes = [
        ctypes.c_char_p,                # const char *input
        c_oid_p,                        # oid *output
        c_sizet_p                       # size_t *out_len
    ]
    f.restype = ctypes.c_int

# include/net-snmp/agent/agent_handler.h
HANDLER_CAN_GETANDGETNEXT               = 0x01
HANDLER_CAN_SET                         = 0x02
HANDLER_CAN_RONLY                       = HANDLER_CAN_GETANDGETNEXT
HANDLER_CAN_RWRITE                      = (HANDLER_CAN_GETANDGETNEXT | \
                                           HANDLER_CAN_SET)

# snmp_impl.h
RONLY        = 0x1
RWRITE         = 0x2
NOACCESS    = 0x0000

class netsnmp_mib_handler(ctypes.Structure): pass
netsnmp_mib_handler_p = ctypes.POINTER(netsnmp_mib_handler)
netsnmp_mib_handler._fields_ = [
    ("handler_name",        ctypes.c_char_p),
    ("myvoid",                 ctypes.c_void_p),
    ("flags",                 ctypes.c_int),
    ("access_method",       ctypes.c_void_p), # Netsnmp_Node_Handler *
    ("data_clone",          ctypes.c_void_p), # void *(*data_clone)(void *myvoid);
    ("data_free",           ctypes.c_void_p), # void (*data_free)(void *myvoid)
    ("next",                netsnmp_mib_handler_p),
    ("prev",                 netsnmp_mib_handler_p),
]

class netsnmp_handler_registration(ctypes.Structure): pass
netsnmp_handler_registration_p = ctypes.POINTER(netsnmp_handler_registration)
netsnmp_handler_registration._fields_ = [
    ("handlerName",         ctypes.c_char_p),
    ("contextName",         ctypes.c_char_p),
    ("rootoid",             c_oid_p),
    ("rootoid_len",         ctypes.c_size_t),
    ("handler",             netsnmp_mib_handler_p),
    ("modes",               ctypes.c_int),
    ("priority",            ctypes.c_int),
    ("range_subid",         ctypes.c_int),
    ("range_ubound",        c_oid),
    ("timeout",             ctypes.c_int),
    ("global_cacheid",      ctypes.c_int),
    ("my_reg_void",         ctypes.c_void_p)
]

class netsnmp_agent_request_info(ctypes.Structure): pass
netsnmp_agent_request_info_p = ctypes.POINTER(netsnmp_agent_request_info)
netsnmp_agent_request_info._fields_ = [
    ("mode",        ctypes.c_int),
    ("asp",         ctypes.c_void_p), # struct netsnmp_agent_session_s *asp
    ("agent_data",  ctypes.c_void_p)  # netsnmp_data_list *agent_data
]

# include/net-snmp/types.h
class netsnmp_variable_list(ctypes.Structure): pass
netsnmp_variable_list_p = ctypes.POINTER(netsnmp_variable_list)
netsnmp_variable_list_p_p = ctypes.POINTER(netsnmp_variable_list_p)
netsnmp_variable_list._fields_ = [
    ("next_variable",        netsnmp_variable_list_p),
    ("name",                c_oid_p), # Object identifier of variable
    ("name_length",            ctypes.c_size_t), # number of subid's in name
    ("var_type",            ctypes.c_uint8), # ASN type of variable
    ("val",                    ctypes.c_void_p), # netsnmp_vardata, value of variable
    ("val_len",                ctypes.c_size_t), # the length of the value to be copied into buf
    ("name_loc",             c_oid * MAX_OID_LEN), # buffer to hold the OID
    ("buf",                    ctypes.c_uint8 * 40), # 90 percentile < 40
    ("data",                 ctypes.c_void_p), # (Opaque) hook for additional data
    ("dataFreeHook",        ctypes.c_void_p), # callback to free above
    ("index",                ctypes.c_int)
]

class variable(ctypes.Structure): pass
variable_p = ctypes.POINTER(variable)
variable._fields_ = [
    ("magic",        ctypes.c_uint8), # passed to function as a hint
    ("var_type",    ctypes.c_uint8), # type of variable
    ("acl",            ctypes.c_ushort), # access control list for variable, See important comment in snmp_vars.c
    ("findVar",        ctypes.c_void_p), # FindVarMethod *, function that finds variable
    ("namelen",        ctypes.c_uint8), # length of above
    ("name",        c_oid * MAX_OID_LEN), # object identifier of variable
]

class netsnmp_subtree(ctypes.Structure): pass
netsnmp_subtree_p = ctypes.POINTER(netsnmp_subtree)
netsnmp_subtree._fields_ = [
    ("name_a",            c_oid_p), # objid prefix of registered subtree
    ("namelen",            ctypes.c_uint8), # number of subid's in name above
    ("start_a",          c_oid_p), # objid of start of covered range
    ("start_len",        ctypes.c_uint8), # number of subid's in start name
    ("end_a",            c_oid_p), # objid of end of covered range
    ("end_len",            ctypes.c_uint8), # number of subid's in end name
    ("variables",        variable_p), # pointer to variables array
    ("variables_len",    ctypes.c_int), # number of entries in above array
    ("variables_width",    ctypes.c_int), # sizeof each variable entry
    ("label_a",            ctypes.c_char_p), # calling module's label
    ("session",            ctypes.c_void_p), # netsnmp_session *
    ("flags",            ctypes.c_uint8), 
    ("priority",         ctypes.c_uint8),
    ("timeout",         ctypes.c_int), # netsnmp_agent_request_info *
    ("next",            netsnmp_subtree_p), # List of 'sibling' subtrees
    ("prev",             netsnmp_subtree_p), # (doubly-linked list)
    ("children",        netsnmp_subtree_p), # List of 'child' subtrees
    ("range_subid",     ctypes.c_int),
    ("range_ubound",    c_oid),
    ("reginfo",            netsnmp_handler_registration_p), # new API
    ("cacheid",            ctypes.c_int),
    ("global_cacheid",     ctypes.c_int),
    ("oid_off",            ctypes.c_size_t),
]

class netsnmp_request_info(ctypes.Structure): pass        
netsnmp_request_info_p = ctypes.POINTER(netsnmp_request_info)
netsnmp_request_info._fields_ = [
    ("requestvb",        netsnmp_variable_list_p), # variable bindings
    ("parent_data",        ctypes.c_void_p), # netsnmp_data_list *
    ("agent_req_info",  ctypes.c_void_p), # struct netsnmp_agent_request_info_s *
    ("range_end",        c_oid_p),           # oid
    ("range_end_len",    ctypes.c_size_t), 
    ("delegated",        ctypes.c_int),
    ("processed",        ctypes.c_int),
    ("inclusive",        ctypes.c_int),
    ("status",            ctypes.c_int),
    ("index",            ctypes.c_int),
    ("repeat",            ctypes.c_int),
    ("orig_repeat",        ctypes.c_int),
    ("requestvb_start", netsnmp_variable_list_p),
    ("next",             netsnmp_request_info_p),
    ("prev",             netsnmp_request_info_p),
    ("subtree",            netsnmp_subtree_p), # netsnmp_subtree_s * 
]
    
Netsnmp_Node_Handler = ctypes.CFUNCTYPE(# Netsnmp_Node_Handler
    ctypes.c_int,                       # result type
    netsnmp_mib_handler_p,              # netsnmp_mib_handler *handler
    netsnmp_handler_registration_p,     # netsnmp_handler_registration *reginfo
    netsnmp_agent_request_info_p,       # netsnmp_agent_request_info *reqinfo
    netsnmp_request_info_p              # netsnmp_request_info *requests
)

Netsnmp_Write_Method = ctypes.CFUNCTYPE( # handler for Netsnmp_Var_Handler
    ctypes.c_int,        # result type
    ctypes.c_int,        # action
    ctypes.c_char_p,    # var_val
    ctypes.c_uint8,        # var_val_type
    ctypes.c_size_t,    # var_val_len
    ctypes.c_char_p,    # statP
    variable_p,            # name
    ctypes.c_size_t        # length
)

Netsnmp_Var_Handler = ctypes.CFUNCTYPE( # Handler for register_mib
    ctypes.c_char_p,                    # result type: u_char *
    variable_p,                            # struct variable *vp
    c_oid_p,                            # name
    ctypes.POINTER(ctypes.c_size_t),    # length
    ctypes.c_int,                        # exact
    ctypes.POINTER(ctypes.c_size_t),    # var_len
    ctypes.POINTER(Netsnmp_Write_Method),    # WriteMethod ** write_method, Netsnmp_Write_Method
)

MATCH_FAILED     = -1
MATCH_SUCCEEDED    = 0

for f in [ libnsa.header_generic ]:
    f.argtypes = [
        variable_p,                         # struct variable *vp
        c_oid_p,                             # oid *name
        ctypes.POINTER(ctypes.c_size_t),       # size_t *length
        ctypes.c_int,                        # int exact
        ctypes.POINTER(ctypes.c_size_t),       # size_t *var_len
        ctypes.POINTER(Netsnmp_Write_Method),    # WriteMethod ** write_method
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.netsnmp_get_agent_uptime ]:
    f.argtypes = []
    f.restype = ctypes.c_ulong

for f in [ libnsa.register_mib ]:
    f.argtypes = [
        ctypes.c_char_p,    # moduleName
        variable_p,         # struct variable *var
        ctypes.c_size_t,     # varsize
        ctypes.c_size_t,       # numvars
        c_oid_p,            # mibloc
        ctypes.c_size_t        # mibloclen
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.netsnmp_call_next_handler ]:
    f.argtypes = [
        netsnmp_mib_handler_p,             # current
        netsnmp_handler_registration_p, # reginfo
        netsnmp_agent_request_info_p,   # reqinfo
        netsnmp_request_info_p,            # requests
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.netsnmp_create_handler_registration ]:
    f.argtypes = [
        ctypes.c_char_p,           # const char *name
        ctypes.c_void_p,           # Netsnmp_Node_Handler *handler_access_method
        c_oid_p,                   # const oid *reg_oid
        ctypes.c_size_t,           # size_t reg_oid_len
        ctypes.c_int               # int modes
    ]
    f.restype = netsnmp_handler_registration_p

for f in [libnsa.netsnmp_set_request_error]:
    f.argtypes = [
        netsnmp_agent_request_info_p,      # reqinfo
        netsnmp_request_info_p,            # request
        ctypes.c_int                       # error value
    ]
    f.restype = ctypes.c_int

for f in [libnsa.snmp_set_var_typed_value]:
    f.argtypes = [
        netsnmp_variable_list_p,      # netsnmp_variable_list *newvar
        ctypes.c_uint8,                # u_char type
        ctypes.c_char_p,               # const u_char * val_str
        ctypes.c_size_t                # size_t val_len
    ]
    f.restype = ctypes.c_int

# include/net-snmp/library/asn1.h
ASN_INTEGER             = 0x02
ASN_OCTET_STR           = 0x04
ASN_OBJECT_ID            = 0x06
ASN_CONSTRUCTOR            = 0x20
ASN_APPLICATION         = 0x40
ASN_CONTEXT                = 0x80

SNMP_MSG_GET            = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x0
SNMP_MSG_GETNEXT        = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x1
SNMP_MSG_RESPONSE        = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x2
SNMP_MSG_SET            = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x3
SNMP_MSG_TRAP            = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4
SNMP_MSG_GETBULK        = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x5
SNMP_MSG_INFORM            = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x6
SNMP_MSG_TRAP2            = ASN_CONTEXT | ASN_CONSTRUCTOR | 0x7

SNMP_MSG_INTERNAL_SET_BEGIN            = -1
SNMP_MSG_INTERNAL_SET_RESERVE1         = 0    # these should match snmp.h
SNMP_MSG_INTERNAL_SET_RESERVE2         = 1
SNMP_MSG_INTERNAL_SET_ACTION           = 2
SNMP_MSG_INTERNAL_SET_COMMIT           = 3
SNMP_MSG_INTERNAL_SET_FREE             = 4
SNMP_MSG_INTERNAL_SET_UNDO             = 5
SNMP_MSG_INTERNAL_SET_MAX              = 6

MODE_GET                = SNMP_MSG_GET
MODE_GETNEXT            = SNMP_MSG_GETNEXT
MODE_GETBULK              = SNMP_MSG_GETBULK
MODE_GET_STASH            = 131 #SNMP_MSG_INTERNAL_GET_STASH

MODE_SET_BEGIN            = SNMP_MSG_INTERNAL_SET_BEGIN
MODE_SET_RESERVE1         = SNMP_MSG_INTERNAL_SET_RESERVE1
MODE_SET_RESERVE2         = SNMP_MSG_INTERNAL_SET_RESERVE2
MODE_SET_ACTION           = SNMP_MSG_INTERNAL_SET_ACTION
MODE_SET_COMMIT           = SNMP_MSG_INTERNAL_SET_COMMIT
MODE_SET_FREE             = SNMP_MSG_INTERNAL_SET_FREE
MODE_SET_UNDO             = SNMP_MSG_INTERNAL_SET_UNDO


# row status values
SNMP_ROW_NONEXISTENT    = 0
SNMP_ROW_ACTIVE         = 1
SNMP_ROW_NOTINSERVICE   = 2
SNMP_ROW_NOTREADY       = 3
SNMP_ROW_CREATEANDGO    = 4
SNMP_ROW_CREATEANDWAIT  = 5
SNMP_ROW_DESTROY        = 6

# counter64 requires some extra work because it can't be reliably represented
# by a single C data type
class counter64(ctypes.Structure):
    @property
    def value(self):
        return self.high << 32 | self.low

    @value.setter
    def value(self, val):
        self.high = val >> 32
        self.low  = val & 0xFFFFFFFF

    def __init__(self, initval=0):
        ctypes.Structure.__init__(self, 0, 0)
        self.value = initval
counter64_p = ctypes.POINTER(counter64)
counter64._fields_ = [
    ("high",                ctypes.c_ulong),
    ("low",                 ctypes.c_ulong)
]

# include/net-snmp/library/snmp_impl.h
ASN_IPADDRESS                           = ASN_APPLICATION | 0
ASN_COUNTER                             = ASN_APPLICATION | 1
ASN_UNSIGNED                            = ASN_APPLICATION | 2
ASN_TIMETICKS                           = ASN_APPLICATION | 3
ASN_COUNTER64                           = ASN_APPLICATION | 6

# include/net-snmp/agent/watcher.h
WATCHER_FIXED_SIZE                      = 0x01
WATCHER_MAX_SIZE                        = 0x02
WATCHER_SIZE_STRLEN                     = 0x08

class netsnmp_watcher_info(ctypes.Structure): pass
netsnmp_watcher_info_p = ctypes.POINTER(netsnmp_watcher_info)
netsnmp_watcher_info._fields_ = [
    ("data",                ctypes.c_void_p),
    ("data_size",           ctypes.c_size_t),
    ("max_size",            ctypes.c_size_t),
    ("type",                ctypes.c_ubyte),
    ("flags",               ctypes.c_int)
    # net-snmp 5.7.x knows data_size_p here as well but we ignore it for
    # backwards compatibility with net-snmp 5.4.x.
] 

for f in [ libnsX.netsnmp_create_watcher_info ]:
    f.argtypes = [
        ctypes.c_void_p,                # void *data
        ctypes.c_size_t,                # size_t size
        ctypes.c_ubyte,                 # u_char type
        ctypes.c_int                    # int flags
    ]
    f.restype = netsnmp_watcher_info_p

for f in [ libnsX.netsnmp_register_watched_instance ]:
    f.argtypes = [
        netsnmp_handler_registration_p, # netsnmp_handler_registration *reginfo
        netsnmp_watcher_info_p          # netsnmp_watcher_info *winfo
    ]
    f.restype = ctypes.c_int

for f in [ libnsX.netsnmp_register_watched_scalar ]:
    f.argtypes = [
        netsnmp_handler_registration_p, # netsnmp_handler_registration *reginfo
        netsnmp_watcher_info_p          # netsnmp_watcher_info *winfo
    ]
    f.restype = ctypes.c_int

# include/net-snmp/varbind_api.h
for f in [ libnsa.snmp_varlist_add_variable ]:
    f.argtypes = [
        netsnmp_variable_list_p_p,       # netsnmp_variable_list **varlist
        c_oid_p,                         # const oid *name
        ctypes.c_size_t,                 # size_t name_length
        ctypes.c_ubyte,                  # u_char type
        ctypes.c_void_p,                 # const void *value
        ctypes.c_size_t                  # size_t len
    ]
    f.restype = netsnmp_variable_list_p

# include/net-snmp/agent/table_data.h
class netsnmp_table_row(ctypes.Structure): pass
netsnmp_table_row_p = ctypes.POINTER(netsnmp_table_row)
netsnmp_table_row._fields_ = [
    ("indexes",             netsnmp_variable_list_p),
    ("index_oid",           c_oid_p),
    ("index_oid_len",       ctypes.c_size_t),
    ("data",                ctypes.c_void_p),
    ("next",                netsnmp_table_row_p),
    ("prev",                netsnmp_table_row_p)
]

class netsnmp_table_data(ctypes.Structure): pass
netsnmp_table_data_p = ctypes.POINTER(netsnmp_table_data)
netsnmp_table_data._fields_ = [
    ("indexes_template",    netsnmp_variable_list_p),
    ("name",                ctypes.c_char_p),
    ("flags",                ctypes.c_int),
    ("store_indexes",        ctypes.c_int),
    ("first_row",            netsnmp_table_row_p),
    ("last_row",            netsnmp_table_row_p)
]

# include/net-snmp/agent/table_dataset.h
class netsnmp_table_data_set_storage_udata(ctypes.Union): pass
netsnmp_table_data_set_storage_udata._fields_ = [
    ("voidp",                ctypes.c_void_p),
    ("integer",              ctypes.POINTER(ctypes.c_ulong)),
    ("string",               ctypes.c_char_p),
    ("objid",                c_oid_p),
    ("bitstring",            ctypes.POINTER(ctypes.c_ubyte)),
    ("counter64",            ctypes.POINTER(counter64)),
    ("floatVal",            ctypes.POINTER(ctypes.c_float)),
    ("doubleVal",            ctypes.POINTER(ctypes.c_double))
]

class netsnmp_table_data_set_storage(ctypes.Structure): pass
netsnmp_table_data_set_storage_p = ctypes.POINTER(netsnmp_table_data_set_storage)
netsnmp_table_data_set_storage._fields_ = [
    ("column",              ctypes.c_uint),
    ("writable",            ctypes.c_byte),
    ("change_ok_fn",        ctypes.c_void_p),
    ("my_change_data",      ctypes.c_void_p),
    ("type",                ctypes.c_ubyte),
    ("data",                netsnmp_table_data_set_storage_udata),
    ("data_len",            ctypes.c_ulong),
    ("next",                netsnmp_table_data_set_storage_p)
]

class netsnmp_table_data_set(ctypes.Structure): pass
netsnmp_table_data_set_p = ctypes.POINTER(netsnmp_table_data_set)
netsnmp_table_data_set._fields_ = [
    ("table",               netsnmp_table_data_p),
    ("default_row",         netsnmp_table_data_set_storage_p),
    ("allow_creation",      ctypes.c_int),
    ("rowstatus_column",    ctypes.c_uint)
]

class netsnmp_table_request_info(ctypes.Structure): pass
netsnmp_table_request_info_p = ctypes.POINTER(netsnmp_table_request_info)
netsnmp_table_request_info._fields_ = [
    ("colnum",          ctypes.c_uint),
    ("number_indexes",  ctypes.c_uint),
    ("indexes",         netsnmp_variable_list_p),
    ("index_oid",       c_oid * MAX_OID_LEN),
    ("index_oid_len",   ctypes.c_size_t),
    ("reg_info",        ctypes.c_void_p) # netsnmp_table_registration_info*
]

for f in [ libnsX.netsnmp_mark_row_column_writable ]:
    f.argtypes = [
        netsnmp_table_row_p, # netsnmp_table_row *row
        ctypes.c_int, # column
        ctypes.c_int, # writable
    ]
    f.restype = ctypes.c_int

for f in [ libnsX.netsnmp_extract_table_row ]:
    f.argtypes = [
        netsnmp_request_info_p, # netsnmp_request_info *request
    ]
    f.restype = netsnmp_table_row_p

for f in [ libnsX.netsnmp_extract_table_info ]:
    f.argtypes = [
        netsnmp_request_info_p, # netsnmp_request_info *request
    ]
    f.restype = netsnmp_table_request_info_p

for f in [ libnsX.netsnmp_tdata_extract_entry ]:
    f.argtypes = [
        netsnmp_request_info_p, # netsnmp_request_info *request
    ]
    f.restype = ctypes.c_void_p

for f in [ libnsX.netsnmp_create_table_data_set ]:
    f.argtypes = [
        ctypes.c_char_p                 # const char *table_name
    ]
    f.restype = netsnmp_table_data_set_p

for f in [ libnsX.netsnmp_table_dataset_add_row ]:
    f.argtypes = [
        netsnmp_table_data_set_p,       # netsnmp_table_data_set *table
        netsnmp_table_row_p,            # netsnmp_table_row *row
    ]
    f.restype = None

for f in [ libnsX.netsnmp_table_data_row_get_byidx]:
    f.argtypes = [
        netsnmp_table_data_p,
        netsnmp_variable_list_p
    ]
    f.restype = netsnmp_table_row_p

for f in [ libnsX.netsnmp_table_data_get_from_oid]:
    f.argtypes = [
        netsnmp_table_data_p,       #  netsnmp_table_data *table
        c_oid_p,                    # oid *
        ctypes.c_size_t             # size_t *
    ]
    f.restype = netsnmp_table_row_p

for f in [ libnsX.netsnmp_table_dataset_delete_row ]:
    f.argtypes = [
        netsnmp_table_row_p,            # netsnmp_table_row *row
    ]
    f.restype = None

for f in [ libnsX.netsnmp_table_data_add_row ]:
    f.argtypes = [
        netsnmp_table_data_p,       #  netsnmp_table_data *table
        netsnmp_table_row_p,        # netsnmp_table_row *row
    ]
    f.restype = ctypes.c_int

for f in [ libnsX.netsnmp_table_data_set_create_row_from_defaults ]:
    f.argtypes = [
        netsnmp_table_data_set_storage_p # netsnmp_table_data_set_storage *defrow
    ]
    f.restype = netsnmp_table_row_p

for f in [ libnsX.netsnmp_create_table_data_row ]:
    f.argtypes = [
    ]
    f.restype = netsnmp_table_row_p

for f in [ libnsX.netsnmp_table_set_add_default_row ]:
    f.argtypes = [
        netsnmp_table_data_set_p,       # netsnmp_table_data_set *table_set
        ctypes.c_uint,                  # unsigned int column
        ctypes.c_int,                   # int type
        ctypes.c_int,                   # int writable
        ctypes.c_void_p,                # void *default_value
        ctypes.c_size_t                 # size_t default_value_len
    ]
    f.restype = ctypes.c_int

for f in [ libnsX.netsnmp_register_table_data_set ]:
    f.argtypes = [
        netsnmp_handler_registration_p, # netsnmp_handler_registration *reginfo
        netsnmp_table_data_set_p,       # netsnmp_table_data_set *data_set
        ctypes.c_void_p                 # netsnmp_table_registration_info *table_info
    ]
    f.restype = ctypes.c_int

for f in [ libnsX.netsnmp_set_row_column ]:
    f.argtypes = [
        netsnmp_table_row_p,            # netsnmp_table_row *row
        ctypes.c_uint,                  # unsigned int column
        ctypes.c_int,                   # int type
        ctypes.c_void_p,                # const void *value
        ctypes.c_size_t                 # size_t value_len
    ]
    f.restype = ctypes.c_int

for f in [ libnsX.netsnmp_table_dataset_add_index ]:
    f.argtypes = [
        netsnmp_table_data_set_p,       # netsnmp_table_data_set *table
        ctypes.c_ubyte                  # u_char type
    ]
    f.restype = None

for f in [ libnsX.netsnmp_table_dataset_remove_and_delete_row ]:
    f.argtypes = [
        netsnmp_table_data_set_p,       # netsnmp_table_data_set *table
        netsnmp_table_row_p             # netsnmp_table_row *row
    ]

# include/net-snmp/agent/snmp_agent.h
for f in [ libnsa.agent_check_and_process ]:
    f.argtypes = [
        ctypes.c_int                    # int block
    ]
    f.restype = ctypes.c_int

# for trap
for f in [ libnsa.send_v2trap, libnsa.snmp_free_varbind ]:
    f.argtypes = [
        netsnmp_variable_list_p # netsnmp_variable_list *var
    ]
    f.restype = None
    
for f in [ libnsa.send_v3trap ]:
    f.argtypes = [
        netsnmp_variable_list_p, # netsnmp_variable_list *var
        ctypes.c_char_p          # const char *context 
    ]
    f.restype = None

for f in [ libnsa.parse_oid_indexes ]:
    f.argtypes = [
        c_oid_p,                # oid *
        ctypes.c_size_t,        # size_t *
        netsnmp_variable_list_p,
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.snmp_clone_var ]:
    f.argtypes = [
        netsnmp_variable_list_p, # var
        netsnmp_variable_list_p, # newvar
    ]
    f.restype = ctypes.c_int

for f in [ libnsa.ctc_notify_main_thread, libnsa.ctc_init_sigpipe ]:
    f.argtypes = []
    f.restype = None

for f in [ libnsa.parse_one_oid_index ]:
    f.argtypes = [
        ctypes.POINTER(c_oid_p),             # oid **oidStart
        ctypes.POINTER(ctypes.c_size_t),    # size_t * oidLen
        netsnmp_variable_list_p,            # data
        ctypes.c_int,                       # complete
    ]
    f.restype = ctypes.c_int

    
