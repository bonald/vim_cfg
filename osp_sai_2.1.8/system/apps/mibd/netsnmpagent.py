#
# python-netsnmpagent module
# Copyright (c) 2013 Pieter Hollants <pieter@hollants.com>
# Licensed under the GNU Public License (GPL) version 3
#
# Main module
#

"""Allows to write net-snmp subagents in Python.

The Python bindings that ship with net-snmp support client operations
only. I fixed a couple of issues in the existing python-agentx module
but eventually decided to write a new module from scratch due to design
issues. For example, it implemented its own handler for registered SNMP
variables, which requires re-doing a lot of stuff which net-snmp
actually takes care of in its API's helpers.

This module, by contrast, concentrates on wrapping the net-snmp C API
for SNMP subagents in an easy manner. It is still under heavy
development and some features are yet missing."""

#import pkg_resources, sys, os, socket, struct, re
import sys, os, socket, struct, re
from collections import defaultdict
from netsnmpapi import *
import Queue
import time, signal
import binascii
import threading

# Make version string from setuptools available as a property variable that
# can be queried for by agents that require a specific version of
# python-netsnmpagent
#__version__ = pkg_resources.require("netsnmpagent")[0].version

# Maximum string size supported by python-netsnmpagent
MAX_STRING_SIZE                 = 1024

# Helper function courtesy of Alec Thomas and taken from
# http://stackoverflow.com/questions/36932/how-can-i-represent-an-enum-in-python
def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    enums["Names"] = dict((value,key) for key, value in enums.iteritems())
    return type("Enum", (), enums)

# Indicates the status of a netsnmpAgent object
netsnmpAgentStatus = enum(
    "REGISTRATION",     # Unconnected, SNMP object registrations possible
    "FIRSTCONNECT",     # No more registrations, first connection attempt
    "CONNECTFAILED",    # Error connecting to snmpd
    "CONNECTED",        # Connected to a running snmpd instance
    "RECONNECTING",     # Got disconnected, trying to reconnect
)

class netsnmpAgent(object):
    """ Implements an SNMP agent using the net-snmp libraries. """

    def __init__(self, **args):
        """Initializes a new netsnmpAgent instance.
        
        "args" is a dictionary that can contain the following
        optional parameters:
        
        - AgentName     : The agent's name used for registration with net-snmp.
        - MasterSocket  : The transport specification of the AgentX socket of
                          the running snmpd instance to connect to (see the
                          "LISTENING ADDRESSES" section in the snmpd(8) manpage).
                          Change this if you want to use eg. a TCP transport or
                          access a custom snmpd instance, eg. as shown in
                          run_simple_agent.sh, or for automatic testing.
        - PersistenceDir: The directory to use to store persistence information.
                          Change this if you want to use a custom snmpd
                          instance, eg. for automatic testing.
        - MIBFiles      : A list of filenames of MIBs to be loaded. Required if
                          the OIDs, for which variables will be registered, do
                          not belong to standard MIBs and the custom MIBs are not
                          located in net-snmp's default MIB path
                          (/usr/share/snmp/mibs).
        - LogHandler    : An optional Python function that will be registered
                          with net-snmp as a custom log handler. If specified,
                          this function will be called for every log message
                          net-snmp itself generates, with parameters as follows:
                          1. a string indicating the message's priority: one of
                          "Emergency", "Alert", "Critical", "Error", "Warning",
                          "Notice", "Info" or "Debug".
                          2. the actual log message. Note that heading strings
                          such as "Warning: " and "Error: " will be stripped off
                          since the priority level is explicitly known and can
                          be used to prefix the log message, if desired.
                          Trailing linefeeds will also have been stripped off.
                          If undefined, log messages will be written to stderr
                          instead. """

        # Default settings
        defaults = {
            "AgentName"     : os.path.splitext(os.path.basename(sys.argv[0]))[0],
            "MasterSocket"  : "tcp:127.0.0.1:705",
            "PersistenceDir": None,
            "MIBFiles"      : None,
            "LogHandler"    : None,
        }
        for key in defaults:
            setattr(self, key, args.get(key, defaults[key]))
        if self.MIBFiles != None and not type(self.MIBFiles) in (list, tuple):
            self.MIBFiles = (self.MIBFiles,)
        # Initialize status attribute -- until start() is called we will accept
        # SNMP object registrations
        self._status = netsnmpAgentStatus.REGISTRATION
        self._trapq = Queue.Queue(64)

        # Unfortunately net-snmp does not give callers of init_snmp() (used
        # in the start() method) any feedback about success or failure of
        # connection establishment. But for AgentX clients this information is
        # quite essential, thus we need to implement some more or less ugly
        # workarounds.

        # For net-snmp 5.7.x, we can derive success and failure from the log
        # messages it generates. Normally these go to stderr, in the absence
        # of other so-called log handlers. Alas we define a callback function
        # that we will register with net-snmp as a custom log handler later on,
        # hereby effectively gaining access to the desired information.
        def _py_log_handler(majorID, minorID, serverarg, clientarg):
            # "majorID" and "minorID" are the callback IDs with which this
            # callback function was registered. They are useful if the same
            # callback was registered multiple times.
            # Both "serverarg" and "clientarg" are pointers that can be used to
            # convey information from the calling context to the callback
            # function: "serverarg" gets passed individually to every call of
            # snmp_call_callbacks() while "clientarg" was initially passed to
            # snmp_register_callback().

            # In this case, "majorID" and "minorID" are always the same (see the
            # registration code below). "serverarg" needs to be cast back to
            # become a pointer to a "snmp_log_message" C structure (passed by
            # net-snmp's log_handler_callback() in snmplib/snmp_logging.c) while
            # "clientarg" will be None (see the registration code below).
            logmsg = ctypes.cast(serverarg, snmp_log_message_p)

            # Generate textual description of priority level
            priorities = {
                LOG_EMERG: "Emergency",
                LOG_ALERT: "Alert",
                LOG_CRIT: "Critical",
                LOG_ERR: "Error",
                LOG_WARNING: "Warning",
                LOG_NOTICE: "Notice",
                LOG_INFO: "Info",
                LOG_DEBUG: "Debug"
            }
            msgprio = priorities[logmsg.contents.priority]

            # Strip trailing linefeeds and in addition "Warning: " and "Error: "
            # from msgtext as these conditions are already indicated through
            # msgprio
            msgtext = re.sub(
                "^(Warning|Error): *",
                "",
                logmsg.contents.msg.rstrip("\n")
            )

            # Intercept log messages related to connection establishment and
            # failure to update the status of this netsnmpAgent object. This is
            # really an ugly hack, introducing a dependency on the particular
            # text of log messages -- hopefully the net-snmp guys won't
            # translate them one day.
            if  msgprio == "Warning" \
            or  msgprio == "Error" \
            and re.match("Failed to .* the agentx master agent.*", msgtext):
                # If this was the first connection attempt, we consider the
                # condition fatal: it is more likely that an invalid
                # "MasterSocket" was specified than that we've got concurrency
                # issues with our agent being erroneously started before snmpd.
                if self._status == netsnmpAgentStatus.FIRSTCONNECT:
                    self._status = netsnmpAgentStatus.CONNECTFAILED

                    # No need to log this message -- we'll generate our own when
                    # throwing a netsnmpAgentException as consequence of the
                    # ECONNECT
                    return 0

                # Otherwise we'll stay at status RECONNECTING and log net-snmp's
                # message like any other. net-snmp code will keep retrying to
                # connect.
            elif msgprio == "Info" \
            and  re.match("AgentX subagent connected", msgtext):
                self._status = netsnmpAgentStatus.CONNECTED
            elif msgprio == "Info" \
            and  re.match("AgentX master disconnected us.*", msgtext):
                self._status = netsnmpAgentStatus.RECONNECTING

            # If "LogHandler" was defined, call it to take care of logging.
            # Otherwise print all log messages to stderr to resemble net-snmp
            # standard behavior (but add log message's associated priority in
            # plain text as well)
            if self.LogHandler:
                self.LogHandler(msgprio, msgtext)
            else:
                print "[{0}] {1}".format(msgprio, msgtext)

            return 0

        # We defined a Python function that needs a ctypes conversion so it can
        # be called by C code such as net-snmp. That's what SNMPCallback() is
        # used for. However we also need to store the reference in "self" as it
        # will otherwise be lost at the exit of this function so that net-snmp's
        # attempt to call it would end in nirvana...
        self._log_handler = SNMPCallback(_py_log_handler)

        # Now register our custom log handler with majorID SNMP_CALLBACK_LIBRARY
        # and minorID SNMP_CALLBACK_LOGGING.
        if libnsa.snmp_register_callback(
            SNMP_CALLBACK_LIBRARY,
            SNMP_CALLBACK_LOGGING,
            self._log_handler,
            None
        ) != SNMPERR_SUCCESS:
            raise netsnmpAgentException(
                "snmp_register_callback() failed for _netsnmp_log_handler!"
            )

        # Finally the net-snmp logging system needs to be told to enable
        # logging through callback functions. This will actually register a
        # NETSNMP_LOGHANDLER_CALLBACK log handler that will call out to any
        # callback functions with the majorID and minorID shown above, such as
        # ours.
        libnsa.snmp_enable_calllog()

        # Unfortunately our custom log handler above is still not enough: in
        # net-snmp 5.4.x there were no "AgentX master disconnected" log
        # messages yet. So we need another workaround to be able to detect
        # disconnects for this release. Both net-snmp 5.4.x and 5.7.x support
        # a callback mechanism using the "majorID" SNMP_CALLBACK_APPLICATION and
        # the "minorID" SNMPD_CALLBACK_INDEX_STOP, which we can abuse for our
        # purposes. Again, we start by defining a callback function.
        def _py_index_stop_callback(majorID, minorID, serverarg, clientarg):
            # For "majorID" and "minorID" see our log handler above.
            # "serverarg" is a disguised pointer to a "netsnmp_session"
            # structure (passed by net-snmp's subagent_open_master_session() and
            # agentx_check_session() in agent/mibgroup/agentx/subagent.c). We
            # can ignore it here since we have a single session only anyway.
            # "clientarg" will be None again (see the registration code below).

            # We only care about SNMPD_CALLBACK_INDEX_STOP as our custom log
            # handler above already took care of all other events.
            if minorID == SNMPD_CALLBACK_INDEX_STOP:
                self._status = netsnmpAgentStatus.RECONNECTING

            return 0

        # Convert it to a C callable function and store its reference
        self._index_stop_callback = SNMPCallback(_py_index_stop_callback)

        # Register it with net-snmp
        if libnsa.snmp_register_callback(
            SNMP_CALLBACK_APPLICATION,
            SNMPD_CALLBACK_INDEX_STOP,
            self._index_stop_callback,
            None
        ) != SNMPERR_SUCCESS:
            raise netsnmpAgentException(
                "snmp_register_callback() failed for _netsnmp_index_callback!"
            )

        self.data_mutex = threading.Lock()
        def _py_index_data_mutex_callback(majorID, minorID, serverarg, clientarg):
            if majorID == SNMP_CALLBACK_LIBRARY and minorID == SNMP_CALLBACK_USER_FUNC_1:
                if serverarg != None:
                    self.data_mutex.acquire()
                else:
                    self.data_mutex.release()

            return 0

        # Convert it to a C callable function and store its reference
        self._index_data_mutex_callback = SNMPCallback(_py_index_data_mutex_callback)

        # Register it with net-snmp
        if libnsa.snmp_register_callback(
            SNMP_CALLBACK_LIBRARY,
            SNMP_CALLBACK_USER_FUNC_1,
            self._index_data_mutex_callback,
            None
        ) != SNMPERR_SUCCESS:
            raise netsnmpAgentException(
                "snmp_register_callback() failed for _netsnmp_index_callback!"
            )

        # No enabling necessary here

        # Make us an AgentX client
        if libnsa.netsnmp_ds_set_boolean(
            NETSNMP_DS_APPLICATION_ID,
            NETSNMP_DS_AGENT_ROLE,
            1
        ) != SNMPERR_SUCCESS:
            raise netsnmpAgentException(
                "netsnmp_ds_set_boolean() failed for NETSNMP_DS_AGENT_ROLE!"
            )

        # Use an alternative transport specification to connect to the master?
        # Defaults to "tcp:127.0.0.1:705".
        # (See the "LISTENING ADDRESSES" section in the snmpd(8) manpage)
        if self.MasterSocket:
            if libnsa.netsnmp_ds_set_string(
                NETSNMP_DS_APPLICATION_ID,
                NETSNMP_DS_AGENT_X_SOCKET,
                self.MasterSocket
            ) != SNMPERR_SUCCESS:
                raise netsnmpAgentException(
                    "netsnmp_ds_set_string() failed for NETSNMP_DS_AGENT_X_SOCKET!"
                )

        # Use an alternative persistence directory?
        if self.PersistenceDir:
            if libnsa.netsnmp_ds_set_string(
                NETSNMP_DS_LIBRARY_ID,
                NETSNMP_DS_LIB_PERSISTENT_DIR,
                ctypes.c_char_p(self.PersistenceDir)
            ) != SNMPERR_SUCCESS:
                raise netsnmpAgentException(
                    "netsnmp_ds_set_string() failed for NETSNMP_DS_LIB_PERSISTENT_DIR!"
                )

        # Initialize net-snmp library (see netsnmp_agent_api(3))
        libnsa.netsnmp_set_mib_directory('+/centec_switch/mibs')
        if libnsa.init_agent(self.AgentName) != 0:
            raise netsnmpAgentException("init_agent() failed!")

        # Initialize MIB parser
        libnsa.netsnmp_init_mib()

        # If MIBFiles were specified (ie. MIBs that can not be found in
        # net-snmp's default MIB directory /usr/share/snmp/mibs), read
        # them in so we can translate OID strings to net-snmp's internal OID
        # format.
        if self.MIBFiles:
            for mib in self.MIBFiles:
                if libnsa.read_mib(mib) == 0:
                    raise netsnmpAgentException("netsnmp_read_module({0}) " +
                                                "failed!".format(mib))

        # Initialize our SNMP object registry
        self._objs    = defaultdict(dict)

        # Initialize sigpipe
        libnsa.ctc_init_sigpipe()

    def _prepareRegistration(self, oidstr, writable = True, nodehandle = None):
        """ Prepares the registration of an SNMP object.

            "oidstr" is the OID to register the object at.
            "writable" indicates whether "snmpset" is allowed. """

        # Make sure the agent has not been start()ed yet
        if self._status != netsnmpAgentStatus.REGISTRATION:
            raise netsnmpAgentException("Attempt to register SNMP object " \
                                        "after agent has been started!")

        # We can't know the length of the internal OID representation
        # beforehand, so we use a MAX_OID_LEN sized buffer for the call to
        # read_objid() below
        oid = (c_oid * MAX_OID_LEN)()
        oid_len = ctypes.c_size_t(MAX_OID_LEN)

        # Let libsnmpagent parse the OID
        if libnsa.read_objid(
            oidstr,
            ctypes.cast(ctypes.byref(oid), c_oid_p),
            ctypes.byref(oid_len)
        ) == 0:
            raise netsnmpAgentException("read_objid({0}) failed!".format(oidstr))

        # Do we allow SNMP SETting to this OID?
        handler_modes = HANDLER_CAN_RWRITE if writable \
                                           else HANDLER_CAN_RONLY

        # Create the netsnmp_handler_registration structure. It notifies
        # net-snmp that we will be responsible for anything below the given
        # OID. We use this for leaf nodes only, processing of subtress will be
        # left to net-snmp.
        handler_reginfo = libnsa.netsnmp_create_handler_registration(
            oidstr,
            nodehandle,
            oid,
            oid_len,
            handler_modes
        )

        return handler_reginfo

    def VarTypeClass(property_func):
        """ Decorator that transforms a simple property_func into a class
            factory returning instances of a class for the particular SNMP
            variable type. property_func is supposed to return a dictionary with
            the following elements:
            - "ctype"           : A reference to the ctypes constructor method
                                  yielding the appropriate C representation of
                                  the SNMP variable, eg. ctypes.c_long or
                                  ctypes.create_string_buffer.
            - "flags"           : A net-snmp constant describing the C data
                                  type's storage behavior, currently either
                                  WATCHER_FIXED_SIZE or WATCHER_MAX_SIZE.
            - "max_size"        : The maximum allowed string size if "flags"
                                  has been set to WATCHER_MAX_SIZE.
            - "initval"         : The value to initialize the C data type with,
                                  eg. 0 or "".
            - "asntype"         : A constant defining the SNMP variable type
                                  from an ASN.1 perspective, eg. ASN_INTEGER.
            - "context"         : A string defining the context name for the
                                  SNMP variable
        
            The class instance returned will have no association with net-snmp
            yet. Use the Register() method to associate it with an OID. """

        # This is the replacement function, the "decoration"
        def create_vartype_class(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
            agent = self

            # Call the original property_func to retrieve this variable type's
            # properties. Passing "initval" to property_func may seem pretty
            # useless as it won't have any effect and we use it ourselves below.
            # However we must supply it nevertheless since it's part of
            # property_func's function signature which THIS function shares.
            # That's how Python's decorators work.
            props = property_func(self, initval)

            # Use variable type's default initval if we weren't given one
            if initval == None:
                initval = props["initval"]

            # Create a class to wrap ctypes' access semantics and enable
            # Register() to do class-specific registration work.
            #
            # Since the part behind the "class" keyword can't be a variable, we
            # use the proxy name "cls" and overwrite its __name__ property
            # after class creation.
            class cls(object):
                def __init__(self):
                    for prop in ["flags", "asntype"]:
                        setattr(self, "_{0}".format(prop), props[prop])

                    # Create the ctypes class instance representing the variable
                    # to be handled by the net-snmp C API. If this variable type
                    # has no fixed size, pass the maximum size as second
                    # argument to the constructor.
                    if props["flags"] == WATCHER_FIXED_SIZE:
                        self._cvar      = props["ctype"](initval)
                        self._data_size = ctypes.sizeof(self._cvar)
                        self._max_size  = self._data_size
                    else:
                        self._cvar      = props["ctype"](initval, props["max_size"])
                        vlen = len(initval) if initval != None else len(self._cvar.value)
                        self._data_size = vlen
                        self._max_size  = max(self._data_size, props["max_size"])

                    if oidstr:
                        # Prepare the netsnmp_handler_registration structure.
                        handler_reginfo = agent._prepareRegistration(oidstr, writable, nodehandle)
                        handler_reginfo.contents.contextName = context

                        # Create the netsnmp_watcher_info structure.
                        self._watcher = libnsX.netsnmp_create_watcher_info(
                            self.cref(),
                            self._data_size,
                            self._asntype,
                            self._flags
                        )

                        # Explicitly set netsnmp_watcher_info structure's
                        # max_size parameter. netsnmp_create_watcher_info6 would
                        # have done that for us but that function was not yet
                        # available in net-snmp 5.4.x.
                        self._watcher.contents.max_size = self._max_size

                        # Register handler and watcher with net-snmp.
                        result = libnsX.netsnmp_register_watched_scalar(
                            handler_reginfo,
                            self._watcher
                        )
                        if result != 0:
                            raise netsnmpAgentException("Error registering variable with net-snmp!")

                        # Finally, we keep track of all registered SNMP objects for the
                        # getRegistered() method.
                        agent._objs[context][oidstr] = self

                def value(self):
                    val = self._cvar.value
                    if val <= sys.maxint:
                        val = int(val)
                    return val

                def cref(self):
                    return ctypes.byref(self._cvar) if self._flags == WATCHER_FIXED_SIZE \
                                                    else self._cvar

                def update(self, val):
                    if self._asntype == ASN_COUNTER and val >> 32:
                        val = val & 0xFFFFFFFF
                    if self._asntype == ASN_COUNTER64 and val >> 64:
                        val = val & 0xFFFFFFFFFFFFFFFF
                    self._cvar.value = val
                    if props["flags"] == WATCHER_MAX_SIZE:
                        if len(val) > self._max_size:
                            raise netsnmpAgentException(
                                "Value passed to update() truncated: {0} > {1} "
                                "bytes!".format(len(val), self._max_size))
                        self._cvar.value = val
                        self._data_size  = self._watcher.contents.data_size = len(val)

                if props["asntype"] in [ASN_COUNTER, ASN_COUNTER64]:
                    def increment(self, count=1):
                        self.update(self.value() + count)

            cls.__name__ = property_func.__name__

            # Return an instance of the just-defined class to the agent
            return cls()

        return create_vartype_class

    @VarTypeClass
    def Integer32(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : ctypes.c_long,
            "flags"         : WATCHER_FIXED_SIZE,
            "initval"       : 0,
            "asntype"       : ASN_INTEGER
        }

    @VarTypeClass
    def Unsigned32(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : ctypes.c_ulong,
            "flags"         : WATCHER_FIXED_SIZE,
            "initval"       : 0,
            "asntype"       : ASN_UNSIGNED
        }

    @VarTypeClass
    def Counter32(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : ctypes.c_ulong,
            "flags"         : WATCHER_FIXED_SIZE,
            "initval"       : 0,
            "asntype"       : ASN_COUNTER
        }

    @VarTypeClass
    def Counter64(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : counter64,
            "flags"         : WATCHER_FIXED_SIZE,
            "initval"       : 0,
            "asntype"       : ASN_COUNTER64
        }

    @VarTypeClass
    def TimeTicks(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : ctypes.c_ulong,
            "flags"         : WATCHER_FIXED_SIZE,
            "initval"       : 0,
            "asntype"       : ASN_TIMETICKS
        }

    # Note we can't use ctypes.c_char_p here since that creates an immutable
    # type and net-snmp _can_ modify the buffer (unless writable is False).
    # Also note that while net-snmp 5.5 introduced a WATCHER_SIZE_STRLEN flag,
    # we have to stick to WATCHER_MAX_SIZE for now to support net-snmp 5.4.x
    # (used eg. in SLES 11 SP2 and Ubuntu 12.04 LTS).
    @VarTypeClass
    def OctetString(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : ctypes.create_string_buffer,
            "flags"         : WATCHER_MAX_SIZE,
            "max_size"      : MAX_STRING_SIZE,
            "initval"       : "",
            "asntype"       : ASN_OCTET_STR
        }

    # Whereas an OctetString can contain UTF-8 encoded characters, a
    # DisplayString is restricted to ASCII characters only.
    @VarTypeClass
    def DisplayString(self, initval = None, oidstr = None, writable = True, context = "", nodehandle = None):
        return {
            "ctype"         : ctypes.create_string_buffer,
            "flags"         : WATCHER_MAX_SIZE|WATCHER_SIZE_STRLEN,
            "max_size"      : MAX_STRING_SIZE,
            "initval"       : "",
            "asntype"       : ASN_OCTET_STR
        }

    # IP addresses are stored as unsigned integers, but the Python interface
    # should use strings. So we need a special class.
    def IpAddress(self, initval = "0.0.0.0", oidstr = None, writable = True, context = "", nodehandle = None):
        agent = self

        class IpAddress(object):
            def __init__(self):
                self._flags     = WATCHER_FIXED_SIZE
                self._asntype   = ASN_IPADDRESS
                self._cvar      = ctypes.c_uint(0)
                self._data_size = ctypes.sizeof(self._cvar)
                self._max_size  = self._data_size
                self.update(initval)
                
                if oidstr:
                    # Prepare the netsnmp_handler_registration structure.
                    handler_reginfo = agent._prepareRegistration(oidstr, writable, nodehandle)
                    handler_reginfo.contents.contextName = context

                    # Create the netsnmp_watcher_info structure.
                    watcher = libnsX.netsnmp_create_watcher_info(
                        self.cref(),
                        ctypes.sizeof(self._cvar),
                        ASN_IPADDRESS,
                        WATCHER_FIXED_SIZE
                    )
                    watcher._maxsize = ctypes.sizeof(self._cvar)

                    # Register handler and watcher with net-snmp.
                    result = libnsX.netsnmp_register_watched_scalar(
                        handler_reginfo,
                        watcher
                    )
                    if result != 0:
                        raise netsnmpAgentException("Error registering variable with net-snmp!")

                    # Finally, we keep track of all registered SNMP objects for the
                    # getRegistered() method.
                    agent._objs[context][oidstr] = self

            def value(self):
                return socket.inet_ntoa(
                    struct.pack("I", self._cvar.value)
                )

            def cref(self):
                return ctypes.byref(self._cvar)

            def update(self, val):
                self._cvar.value = struct.unpack(
                    "I",
                    socket.inet_aton(val)
                )[0]

        # Return an instance of the just-defined class to the agent
        return IpAddress()

    @VarTypeClass
    def ObjectIdentify(self, initval="0", oidstr=None, writable=True, context="", nodehandle=None):
        agent = self

        return {
            "ctype": ctypes.sizeof(c_oid)*MAX_OID_LEN,
            "flags": WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN,
            "max_size": MAX_OID_LEN,
            "initval": "",
            "asntype": ASN_OBJECT_ID
        }



    def Table(self, oidstr, indexes, columns, counterobj = None, extendable = False, context = "", nodehandle = None, no_idx_col = False):
        agent = self

        # Define a Python class to provide access to the table.
        class Table(object):
            def __init__(self, oidstr, idxobjs, coldefs, counterobj, extendable, context, no_idx_col):
                # Create a netsnmp_table_data_set structure, representing both
                # the table definition and the data stored inside it. We use the
                # oidstr as table name.
                self._dataset = libnsX.netsnmp_create_table_data_set(
                    ctypes.c_char_p(oidstr)
                )
                self._no_idx_col = no_idx_col
                
                # A RowStatus object may be used to control the creation of a new row
                self._dataset.contents.allow_creation = 0
                self._col_writable = {}

                # Define the table row's indexes
                colno = 1
                for idxobj in idxobjs:
                    libnsX.netsnmp_table_dataset_add_index(
                        self._dataset,
                        idxobj._asntype
                    )

                    if self._no_idx_col == False:
                        self._col_writable[colno] = 0
                    colno = colno + 1

                # Define the table's columns and their default values
                for coldef in coldefs:
                    colno    = coldef[0]
                    defobj   = coldef[1]
                    writable = coldef[2] if len(coldef) > 2 \
                                         else 0

                    self._col_writable[colno] = writable
                    
                    # netsnmp_table_set_add_default_row() ignores the ASN type,
                    # so it doesn't implement any special handling for the
                    # trailing zero byte in C strings
                    size = defobj._data_size + 1 if defobj._asntype == ASN_OCTET_STR \
                                                 else defobj._data_size
                    if defobj._asntype == ASN_OCTET_STR and defobj.value() == '':
                        result = libnsX.netsnmp_table_set_add_default_row(
                            self._dataset,
                            colno,
                            defobj._asntype,
                            writable,
                            None,
                            0
                        )
                    else:
                        result = libnsX.netsnmp_table_set_add_default_row(
                            self._dataset,
                            colno,
                            defobj._asntype,
                            writable,
                            defobj.cref(),
                            size
                        )
                    if result != SNMPERR_SUCCESS:
                        raise netsnmpAgentException(
                            "netsnmp_table_set_add_default_row() failed with "
                            "error code {0}!".format(result)
                        )

                # Register handler and table_data_set with net-snmp.
                self._handler_reginfo = agent._prepareRegistration(
                    oidstr,
                    extendable,
                    nodehandle
                )
                self._handler_reginfo.contents.contextName = context
                result = libnsX.netsnmp_register_table_data_set(
                    self._handler_reginfo,
                    self._dataset,
                    None
                )
                if result != SNMP_ERR_NOERROR:
                    raise netsnmpAgentException(
                        "Error code {0} while registering table with "
                        "net-snmp!".format(result)
                    )

                # Finally, we keep track of all registered SNMP objects for the
                # getRegistered() method.
                agent._objs[context][oidstr] = self

                # If "counterobj" was specified, use it to track the number
                # of table rows
                if counterobj:
                    counterobj.update(0)
                self._counterobj = counterobj

            def set_allow_creation(self, onoff):
                self._dataset.contents.allow_creation = onoff
                
            def set_rowstatus_column(self, col_idx):
                self._dataset.contents.rowstatus_column = col_idx
                
            def addRow(self, idxobjs):
                dataset = self._dataset
                writables = self._col_writable
                no_idx_col = self._no_idx_col
                # Define a Python class to provide access to the table row.
                class TableRow(object):
                    def __init__(self, table_row):
                        pass
                
                    def __init__(self, idxobjs):
                        # Create the netsnmp_table_set_storage structure for
                        # this row.
                        self._table_row = libnsX.netsnmp_table_data_set_create_row_from_defaults(
                            dataset.contents.default_row
                        )
                        #self._table_row = libnsX.netsnmp_create_table_data_row()
                        if self._table_row == None:
                            raise netsnmpAgentException("out of memory")

                        # Add the indexes
                        colid = 1
                        for idxobj in idxobjs:
                            result = libnsa.snmp_varlist_add_variable(
                                ctypes.pointer(self._table_row.contents.indexes),
                                None,
                                0,
                                idxobj._asntype,
                                idxobj.cref(),
                                idxobj._data_size
                            )
                            if result == None:
                                raise netsnmpAgentException("out of memory")
                            if no_idx_col == False:
                                self.setRowCell(colid, idxobj)
                            colid = colid + 1

                    def setRowCell(self, column, snmpobj):
                        # netsnmp_set_row_column() ignores the ASN type, so it doesn't
                        # do special handling for the trailing zero byte in C strings
                        #size = snmpobj._data_size + 1 if snmpobj._asntype == ASN_OCTET_STR \
                        #                              else snmpobj._data_size
                        size = snmpobj._data_size
                        result = libnsX.netsnmp_set_row_column(
                            self._table_row,
                            column,
                            snmpobj._asntype,
                            snmpobj.cref(),
                            size
                        )
                        if result != SNMPERR_SUCCESS:
                            raise netsnmpAgentException("netsnmp_set_row_column() failed with error code {0}!".format(result))
                        libnsX.netsnmp_mark_row_column_writable(
                            self._table_row,
                            column,
                            writables[column]
                        )

                row = TableRow(idxobjs)
                
                result = libnsX.netsnmp_table_data_add_row(
                    dataset.contents.table,
                    row._table_row
                )
                if result != SNMP_ERR_NOERROR:
                    # add row failed: exists or error
                    row_old = libnsX.netsnmp_table_data_get_from_oid(
                            dataset.contents.table,
                            row._table_row.contents.index_oid,
                            row._table_row.contents.index_oid_len
                        )
                    # delete new row instance
                    libnsX.netsnmp_table_dataset_delete_row(
                        row._table_row
                    )
                    if row_old == None:
                        # out of memory?
                        row._table_row = None
                        raise netsnmpAgentException("out of memory")
                    else:
                        # row exists
                        row._table_row = row_old

                if self._counterobj:
                    self._counterobj.update(self._counterobj.value() + 1)
               
                return row

            def delRow(self, idxobjs):
                dataset = self._dataset

                idxvar = netsnmp_variable_list_p()
                for idxobj in idxobjs:
                    result = libnsa.snmp_varlist_add_variable(
                            idxvar,
                            None,
                            0,
                            idxobj._asntype,
                            idxobj.cref(),
                            idxobj._data_size
                            )
                    if result == None:
                        raise netsnmpAgentException("out of memory")
                row = libnsX.netsnmp_table_data_row_get_byidx(
                            self._dataset.contents.table,
                            idxvar
                        )
                if row != None:
                    libnsX.netsnmp_table_dataset_remove_and_delete_row(
                        self._dataset,
                        row
                    )
                        
                libnsa.snmp_free_varbind(idxvar)
                if self._counterobj:
                    self._counterobj.update(self._counterobj.value() - 1)

            def getRow(self, idxobjs):
                idxvar = netsnmp_variable_list_p()
                for idxobj in idxobjs:
                    result = libnsa.snmp_varlist_add_variable(
                            idxvar,
                            None,
                            0,
                            idxobj._asntype,
                            idxobj.cref(),
                            idxobj._data_size
                            )
                    if result == None:
                        raise netsnmpAgentException("out of memory")
                row = libnsX.netsnmp_table_data_row_get_byidx(
                            self._dataset.contents.table,
                            idxvar
                        )

                libnsa.snmp_free_varbind(idxvar)
                return row

            def value(self):
                # Because tables are more complex than scalar variables, we
                # return a dictionary representing the table's structure and
                # contents instead of a simple string.
                retdict = {}

                # The first entry will contain the defined columns, their types
                # and their defaults, if set. We use array index 0 since it's
                # impossible for SNMP tables to have a row with that index.
                retdict[0] = {}
                col = self._dataset.contents.default_row
                while bool(col):
                    retdict[0][int(col.contents.column)] = {}

                    asntypes = {
                        ASN_INTEGER:    "Integer",
                        ASN_OCTET_STR:  "OctetString",
                        ASN_IPADDRESS:  "IPAddress",
                        ASN_COUNTER:    "Counter32",
                        ASN_COUNTER64:  "Counter64",
                        ASN_UNSIGNED:   "Unsigned32",
                        ASN_TIMETICKS:  "TimeTicks"
                    }
                    retdict[0][int(col.contents.column)]["type"] = asntypes[col.contents.type]
                    if bool(col.contents.data):
                        if col.contents.type == ASN_OCTET_STR:
                            retdict[0][int(col.contents.column)]["value"] = col.contents.data.string
                        else:
                            retdict[0][int(col.contents.column)]["value"] = col.contents.data.integer.contents.value
                    col = col.contents.next

                # Next we iterate over the table's rows, creating a dictionary
                # entry for each row after that row's index.
                row = self._dataset.contents.table.contents.first_row
                while bool(row):
                    # We want to return the row index in the same way it is
                    # shown when using "snmptable", eg. "aa" instead of 2.97.97.
                    # This conversion is actually quite complicated (see
                    # net-snmp's sprint_realloc_objid() in snmplib/mib.c and
                    # get*_table_entries() in apps/snmptable.c for details).
                    # All code below assumes eg. that the OID output format was
                    # not changed.
                    
                    # snprint_objid() below requires a _full_ OID whereas the
                    # table row contains only the current row's identifer.
                    # Unfortunately, net-snmp does not have a ready function to
                    # get the full OID. The following code was modelled after
                    # similar code in netsnmp_table_data_build_result().
                    fulloid = ctypes.cast(
                        ctypes.create_string_buffer(
                            MAX_OID_LEN * ctypes.sizeof(c_oid)
                        ),
                        c_oid_p
                    )

                    # Registered OID
                    rootoidlen = self._handler_reginfo.contents.rootoid_len
                    for i in range(0,rootoidlen):
                        fulloid[i] = self._handler_reginfo.contents.rootoid[i]

                    # Entry
                    fulloid[rootoidlen] = 1

                    # Fake the column number. Unlike the table_data and
                    # table_data_set handlers, we do not have one here. No
                    # biggie, using a fixed value will do for our purposes as
                    # we'll do away with anything left of the last dot below.
                    fulloid[rootoidlen+1] = 2

                    # Index data
                    rootoidlen = -2
                    indexoidlen = row.contents.index_oid_len
                    for i in range(0,indexoidlen):
                        fulloid[rootoidlen+2+i] = row.contents.index_oid[i]

                    # Convert the full oid to its string representation
                    oidcstr = ctypes.create_string_buffer(MAX_OID_LEN)
                    libnsa.snprint_objid(
                        oidcstr,
                        MAX_OID_LEN,
                        fulloid,
                        rootoidlen + 2 + indexoidlen
                    )

                    # And finally do away with anything left of the last dot
                    print "oidcstr {0}".format(oidcstr.value)
                    indices = oidcstr.value[1:] # skip leading dot
                    #indices = oidcstr.value.split(".")[-1].replace('"', '')

                    '''
                    # If it's a string, remove the double quotes. If it's a
                    # string containing an integer, make it one
                    try:
                        indices = int(indices)
                    except ValueError:
                        indices = indices.replace('"', '')
                    '''
                    print "indices {0}".format(indices)
                    
                    # Finally, iterate over all columns for this row and add
                    # stored data, if present
                    retdict[indices] = {}
                    data = ctypes.cast(row.contents.data, ctypes.POINTER(netsnmp_table_data_set_storage))
                    
                    while bool(data):
                        if bool(data.contents.data):
                            if data.contents.type == ASN_OCTET_STR:
                                if data.contents.data.string != None:
                                    strv = data.contents.data.string[0:data.contents.data_len]
                                    retdict[indices][int(data.contents.column)] = strv
                                else:
                                    retdict[indices][int(data.contents.column)] = data.contents.data.string
                            else:
                                retdict[indices][int(data.contents.column)] = data.contents.data.integer.contents.value
                        else:
                            retdict[indices] += {}
                        data = data.contents.next

                    row = row.contents.next
                return retdict

            def clear(self):
                row = self._dataset.contents.table.contents.first_row
                while bool(row):
                    nextrow = row.contents.next
                    libnsX.netsnmp_table_dataset_remove_and_delete_row(
                        self._dataset,
                        row
                    )
                    row = nextrow
                if self._counterobj:
                    self._counterobj.update(0)

            # return netsnmp_variable_list_p
            def parse_request_index(self, index_oid, index_oid_len):
                var = self._dataset.contents.table.contents.indexes_template
                loc = 0
                objs = []
                while bool(var):
                    if var.contents.var_type == ASN_INTEGER:
                        slen = index_oid[loc]
                        objs.append([ASN_INTEGER, slen])
                        loc = loc + 1
                    elif var.contents.var_type == ASN_OCTET_STR:
                        slen = index_oid[loc]
                        oidarr = index_oid[loc + 1 : loc + 1 + slen]
                        stmp = binascii.unhexlify(''.join(("%0.2X" % oid) for oid in oidarr)) 
                        objs.append([ASN_OCTET_STR, stmp])
                        loc = loc + slen + 1
                    elif var.contents.var_type == ASN_UNSIGNED:
                        slen = index_oid[loc]
                        objs.append([ASN_UNSIGNED, slen])
                        loc = loc + 1
                    elif var.contents.var_type == ASN_IPADDRESS:
                        oidarr = index_oid[loc: loc + 4]
                        stmp = int("0x" + ''.join(("%0.2X" % oid) for oid in oidarr), 16)         
                        #ipaddr = socket.htonl(stmp)
                        ipstr = socket.inet_ntoa(struct.pack("I", stmp))
                        objs.append([ASN_IPADDRESS, ipstr])
                        loc = loc + 4
                    else:
                        raise netsnmpAgentException("parse_request_index() please add code to process type {0}!".format(var.contents.var_type))
                    var = var.contents.next_variable
                return objs

            def parse_request_indexes(self, index_oid, index_oid_len):
                var = self._dataset.contents.table.contents.indexes_template
                loc = 0
                num = 0
                objs = []
                while bool(var):
                    if var.contents.var_type == ASN_INTEGER:
                        slen = index_oid[loc]
                        objs.append([ASN_INTEGER, slen])
                        loc = loc + 1
                        num = num + 1
                    elif var.contents.var_type == ASN_OCTET_STR:
                        slen = index_oid[loc]
                        oidarr = index_oid[loc + 1 : loc + 1 + slen]
                        stmp = binascii.unhexlify(''.join(("%0.2X" % oid) for oid in oidarr))
                        objs.append([ASN_OCTET_STR, stmp])
                        loc = loc + slen + 1
                        num = num + 1
                    elif var.contents.var_type == ASN_UNSIGNED:
                        slen = index_oid[loc]
                        objs.append([ASN_UNSIGNED, slen])
                        loc = loc + 1
                        num = num + 1
                    elif var.contents.var_type == ASN_IPADDRESS:
                        oidarr = index_oid[loc: loc + 4]
                        stmp = int("0x" + ''.join(("%0.2X" % oid) for oid in oidarr), 16)
                        #ipaddr = socket.htonl(stmp)
                        ipstr = socket.inet_ntoa(struct.pack("I", stmp))
                        objs.append([ASN_IPADDRESS, ipstr])
                        loc = loc + 4
                        num = num + 1
                    else:
                        raise netsnmpAgentException("parse_request_index() please add code to process type {0}!".format(var.contents.var_type))
                    var = var.contents.next_variable
                return objs, num

        # Return an instance of the just-defined class to the agent
        return Table(oidstr, indexes, columns, counterobj, extendable, context, no_idx_col)

    def getContexts(self):
        return self._objs.keys()

    def getRegistered(self, context = ""):
        """Returns a dictionary with the currently registered SNMP objects.
        Returned is a dictionary objects for the specified `context`, which
        defaults to the default context ""."""
        myobjs = {}
        for oidstr, snmpobj in self._objs[context].iteritems():
            myobjs[oidstr] = {
                "type": type(snmpobj).__name__,
                "value": snmpobj.value()
            }
        return dict(myobjs)

    def start(self):
        """ Starts the agent. Among other things, this means connecting
            to the master agent, if configured that way. """
        if  self._status != netsnmpAgentStatus.CONNECTED \
        and self._status != netsnmpAgentStatus.RECONNECTING:
            self._status = netsnmpAgentStatus.FIRSTCONNECT
            libnsa.init_snmp(self.AgentName)
            libnsa.oem_init() # setting enterprise id
            if self._status == netsnmpAgentStatus.CONNECTFAILED:
                msg = "Error connecting to snmpd instance at \"{0}\" -- " \
                      "incorrect \"MasterSocket\" or snmpd not running?"
                msg = msg.format(self.MasterSocket)
                raise netsnmpAgentException(msg)

    def check_and_process(self, block=True):
        """ Processes incoming SNMP requests.
            If optional "block" argument is True (default), the function
            will block until a SNMP packet is received. """
        libnsa.agent_check_and_process(int(bool(block)))
        while True:
            try:
                TrapVars = self._trapq.get_nowait()
                if TrapVars == None:
                    break
                libnsa.send_v2trap(TrapVars)
                libnsa.snmp_free_varbind(TrapVars)  
            except Queue.Empty:
                break
            except Exception as err:
                print "check_and_process: send trap failed: {1}.".format(str(err)) 
                break


    # send trap from within agentx module
    def Trapv2(self, oidstr, uptime, *args):
        sysUpTimeOID    = (c_oid * 9) (1, 3, 6, 1, 2, 1, 1, 3, 0)        # sysUpTimeInstance
        snmpTrapOID    = (c_oid * 11) (1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0)    # snmpTrapOID.0

        toid = (c_oid * MAX_OID_LEN)()
        toid_len = ctypes.c_size_t(MAX_OID_LEN)
        if libnsa.read_objid(
            oidstr,
            ctypes.cast(ctypes.byref(toid), c_oid_p),
            ctypes.byref(toid_len)
        ) == 0:
            raise netsnmpAgentException("read_objid({0}) failed!".format(oidstr))

        TrapVars = netsnmp_variable_list_p()
        client_uptime = ctypes.c_long(uptime)

        # agent uptime
        libnsa.snmp_varlist_add_variable(
            ctypes.byref(TrapVars),
            ctypes.cast(ctypes.byref(sysUpTimeOID), ctypes.POINTER(c_oid)), len(sysUpTimeOID),
            ASN_TIMETICKS,
            ctypes.cast(ctypes.byref(client_uptime), ctypes.c_char_p), ctypes.sizeof(client_uptime),
        )

        # agent trap
        libnsa.snmp_varlist_add_variable(
            ctypes.byref(TrapVars),
            ctypes.cast(ctypes.byref(snmpTrapOID), ctypes.POINTER(c_oid)), len(snmpTrapOID),
            ASN_OBJECT_ID,
            ctypes.cast(ctypes.byref(toid), ctypes.c_char_p), toid_len.value * ctypes.sizeof(c_oid),
        )

        # add variable (format: OID type value)
        idx = 0
        alist = args[0]
        while idx < len(alist):
            ArgOID = alist[idx]
            ArgType = alist[idx + 1]
            ArgData = alist[idx + 2]
            aoid = (c_oid * MAX_OID_LEN)()
            aoid_len = ctypes.c_size_t(MAX_OID_LEN)
            if libnsa.read_objid(
                ArgOID,
                ctypes.cast(ctypes.byref(aoid), c_oid_p),
                ctypes.byref(aoid_len)
            ) == 0:
                raise netsnmpAgentException("read_objid({0}) failed!".format(oidstr))
            """ TYPE: one of i, u, t, a, o, s
                i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
                o: OBJID, s: STRING """
            ObjType = None
            ObjVal = None
            ValLen = 0
            if 'i' == ArgType:
                ObjType = ASN_INTEGER
                intv = ctypes.c_int(int(ArgData))
                ObjVal = ctypes.byref(intv)
                ValLen = ctypes.sizeof(ctypes.c_int)
            elif 'u' == ArgType or 't' == ArgType or 'a' == ArgType:
                ulongv = ctypes.c_ulong(long(ArgData))
                ObjVal = ctypes.byref(ulongv)
                ValLen = ctypes.sizeof(ctypes.c_ulong)
                if 'u' == ArgType: 
                    ObjType = ASN_UNSIGNED
                elif 't' == ArgType:
                    ObjType = ASN_TIMETICKS
                else: 
                    ObjType = ASN_IPADDRESS
                ValLen = 4 # XXX for snmp_varlist_add_variable
            elif 's' == ArgType:
                ObjType = ASN_OCTET_STR
                ObjVal = ArgData
                ValLen = len(ArgData)
            elif 'o' == ArgType:
                ObjVal = (c_oid * MAX_OID_LEN)()
                ooid_len = ctypes.c_size_t(MAX_OID_LEN)
                if libnsa.read_objid(
                    ArgData,
                    ctypes.cast(ctypes.byref(ObjVal), c_oid_p),
                    ctypes.byref(ooid_len)
                ) == 0:
                    raise netsnmpAgentException("parse objid({0}) failed!".format(oidstr))
                ObjType = ASN_OBJECT_ID
                ValLen = ooid_len.value * ctypes.sizeof(c_oid)
            else: 
                raise netsnmpAgentException("unsupported var type ({0}) failed!".format(ArgType))

            # add variable
            libnsa.snmp_varlist_add_variable(
                ctypes.byref(TrapVars),
                ctypes.cast(ctypes.byref(aoid), ctypes.POINTER(c_oid)), aoid_len,
                ObjType,
                ctypes.cast(ObjVal, ctypes.c_char_p), ValLen,
            )
            idx = idx + 3

        # send trap
        self._trapq.put(TrapVars)
        libnsa.ctc_notify_main_thread()

    def awake_main_loop(self):
        libnsa.ctc_notify_main_thread()

    def set_ping_interval(self, sec):
        return libnsa.netsnmp_ds_set_int(
            NETSNMP_DS_APPLICATION_ID,
            NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL,
            sec)
        
    def get_enterprise_id(self):
        """ get snmp enterprise id, default is 27975 """
        return libnsa.get_enterprise_id()

    def set_request_error(self, reqinfo, request, error_value = SNMP_ERR_GENERR):
        """ netsnmp_set_request_error for callback """
        return libnsa.netsnmp_set_request_error(reqinfo, request, error_value)

    def set_var_typed_value(self, newvar, var_type, val_str, val_len):
        return libnsa.snmp_set_var_typed_value(newvar, var_type, val_str, val_len)

    def snmp_free_varbind(self, newvar):
        libnsa.snmp_free_varbind(newvar)

    def call_next_handler(self, handler, reginfo, reqinfo, requests):
        return libnsa.netsnmp_call_next_handler(handler, reginfo, reqinfo, requests)

    def snmp_log(self, priority, szformat, *args):
        return libnsa.snmp_log(priority, szformat, *args)

    def header_generic(self, vp, name, namelength, exact, var_len, write_method):
        return libnsa.header_generic(vp, name, namelength, exact, var_len, write_method)

    def get_uptime(self):
        return libnsa.netsnmp_get_agent_uptime()

    def register_mib(self, moduleName, oidstr, mibvars):
        oid = (c_oid * MAX_OID_LEN)()
        oid_len = ctypes.c_size_t(MAX_OID_LEN)

        if libnsa.read_objid(
            oidstr,
            ctypes.cast(ctypes.byref(oid), c_oid_p),
            ctypes.byref(oid_len)
        ) == 0:
            raise netsnmpAgentException("read_objid({0}) failed!".format(oidstr))

        numvars = ctypes.c_size_t(len(mibvars))
        mvar = (variable * len(mibvars))()
        varsize = ctypes.c_size_t(ctypes.sizeof(variable))
        my_idx = 0
        for vline in mibvars:
            mvar[my_idx].magic = vline[0]
            mvar[my_idx].var_type = vline[1]
            mvar[my_idx].acl = vline[2]
            mvar[my_idx].findVar = ctypes.cast(vline[3], ctypes.c_void_p)
            mvar[my_idx].namelen = 1
            var_oid_len = ctypes.c_size_t(MAX_OID_LEN)
            if libnsa.read_objid(
                vline[4],
                ctypes.cast(ctypes.byref(mvar[my_idx].name), c_oid_p),
                ctypes.byref(var_oid_len)
            ) == 0:
                raise netsnmpAgentException("read_objid({0}) failed!".format(oidstr))
            mvar[my_idx].namelen = var_oid_len.value
            my_idx = my_idx + 1
        return libnsa.register_mib(moduleName, ctypes.cast(ctypes.byref(mvar), variable_p), \
            varsize, numvars, ctypes.cast(ctypes.byref(oid), c_oid_p), oid_len)

    def __del__(self):
        libnsa.snmp_shutdown(self.AgentName)

class netsnmpAgentException(Exception):
    pass

# class for mib node set
class SnmpWriteBase(object):
    def __init__(self, want_type = None):
        def _writer(state, var_val, var_val_type, var_val_len, statP, name, namelength):
            # check oid type
            if self._want_type != None:
                if self._want_type != var_val_type:
                    return SNMP_ERR_WRONGTYPE

            if state == MODE_SET_RESERVE1:
                self._var = var_val
                self._var_type = var_val_type
                self._var_len = var_val_len
                self._name = name
                self._namelength = namelength

            return self._run(state)

        self._want_type = want_type
        self._var = None
        self._var_type = None
        self._var_len = None
        self._name = None
        self._namelength = None
        self._write_method = Netsnmp_Write_Method(_writer)
        self._bufp = None

    # return write method for SNMP SET
    def get_write_method(self):
        return self._write_method

    def caddr(self):
        if self._bufp == None:
            raise netsnmpAgentException("caddr() invalid buffer")
        return ctypes.addressof(self._bufp)

    def update(self, val):
        if self._bufp == None:
            raise netsnmpAgentException("caddr() invalid buffer")
        self._bufp.value = val

    def check(self, var_val, var_type, var_len, name, namelength):
        """ please override this function
        process MODE_SET_RESERVE1 to check parameters
        """
        return SNMP_ERR_WRONGVALUE

    def action(self, var_val, var_len, name, namelength):
        """ please override this function
        process MODE_SET_ACTION to do write
        """
        return SNMP_ERR_COMMITFAILED

    def undo(self, var_val, var_len, name, namelength):
        """ please override this function
        process MODE_SET_UNDO to restore old settings
        """
        return SNMP_ERR_NOERROR

    def get(self):
        """ please override this function"""
        return SNMP_ERR_GENERR

    def _run(self, state):
        ret = SNMP_ERR_NOERROR
        if  MODE_SET_RESERVE1 == state:
            # check parameters
            ret = self.check(self._var, self._var_type, self._var_len, \
                        self._name, self._namelength)
        elif MODE_SET_RESERVE2 == state: pass
        elif MODE_SET_ACTION == state:
            ret = self.action(self._var, self._var_len, self._name, self._namelength)
        elif MODE_SET_COMMIT == state: pass
        elif MODE_SET_FREE == state: pass
        elif MODE_SET_UNDO == state:
            ret = self.undo(self._var, self._var_len, self._name, self._namelength)
        return ret

# class for register_mib
class SnmpVarBase(object):
    def __init__(self, agentx):
        self._agent = agentx
        self._vp = None
        self._write_method = None
        self._var_len = 0
        self._ret_buf_ptr = None
        
        def _vhandler(vp, name, namelength, exact, var_len, write_method):
            if self._agent.header_generic(vp, name, namelength, \
                    exact, var_len, write_method) != MATCH_SUCCEEDED:
                return None
                
            self._vp = vp
            self._write_method = None
            self._var_len = 0
            self._ret_buf_ptr = None
            self.run(vp.contents.magic, exact)

            if self._write_method != None:
                write_method[0] = self._write_method
            var_len.contents.value = self._var_len
            return self._ret_buf_ptr
        self._var_handler = Netsnmp_Var_Handler(_vhandler)

    # APIs for retrieve variable attributes
    def get_var_type(self):
        if bool(self._vp):
            return self._vp.contents.var_type
        return None

    # return var name attributes (oid, oidlen)
    def get_var_name(self):
        if bool(self._vp):
            return (self._vp.contents.name, self._vp.contents.namelen)
        return (None, None)      

    # APIs for set return value & functions
    def set_return_value(self, ret_buf_ptr, ret_size):
        self._ret_buf_ptr = ret_buf_ptr
        self._var_len = ret_size
    
    def set_write_method(self, write_method):
        self._write_method = write_method

    # please override this function
    # magic: column id
    # exact: exact flag
    def run(self, magic, exact):
        return None

# class for table & node
class SnmpTableBase(object):
    def __init__(self, agentx):
        self._reqinfo = None
        self._cur_req = None
        self._cur_col = None
        self._agent = agentx
    
        def _nhandler(handler, reginfo, reqinfo, requests):
            # save request info and requests
            self._reqinfo = reqinfo
            rmode = reqinfo.contents.mode

            # process each request
            request = requests
            while bool(request):
                self._cur_req = request
                self._cur_col = libnsX.netsnmp_extract_table_info(request)
                if bool(self._cur_col):
                    colnum = self._cur_col.contents.colnum
                else:
                    colnum = 0
                cur_vb = request.contents.requestvb
                ret = SNMP_ERR_NOERROR
                if MODE_GET == rmode:
                    ret = self.get(colnum)
                elif MODE_SET_RESERVE1 == rmode:
                    ret = self._check(colnum, cur_vb.contents.var_type,
                                     cur_vb.contents.val,
                                     cur_vb.contents.val_len)
                elif MODE_SET_RESERVE2 == rmode:
                    ret = self.backup(colnum, cur_vb.contents.var_type, 
                                     cur_vb.contents.val,
                                     cur_vb.contents.val_len)
                elif MODE_SET_ACTION == rmode:
                    ret = self.action(colnum, cur_vb.contents.var_type, 
                                     cur_vb.contents.val,
                                     cur_vb.contents.val_len)
                elif MODE_SET_UNDO == rmode:
                    ret = self.undo(colnum)
                else: pass

                if SNMP_ERR_NOERROR != ret:
                    return ret
                request = request.contents.next

            #nexthandler = handler.contents.next
            #if bool(nexthandler) and bool(nexthandler.contents.access_method):
            #    self._agent.call_next_handler(handler, reginfo, reqinfo, requests)   
            return SNMP_ERR_NOERROR

        self.node_handler = Netsnmp_Node_Handler(_nhandler)

        def _sub_handler(tbl_id, operation, ds_id, ds_id2, field_id, data):
            self.subscriber(tbl_id, operation, ds_id, ds_id2, field_id, data)
        self.subscribe_handler = _sub_handler

    # APIs for getting request parameters
    def get_request_colnum(self):
        return self._cur_col.contents.colnum

    def get_request_index(self):
        return (self._cur_col.contents.index_oid, self._cur_col.contents.index_oid_len)

    def get_request_indexes(self, table):
        indexes = []
        objs, num = table.parse_request_indexes(self._cur_col.contents.index_oid, self._cur_col.contents.index_oid_len)
        for i in range(num):
            indexes.append(objs[i][1])
        return indexes

    # APIs for get table row colnum obj
    def set_table_row(self, colnum, snmpobj):
        t_row = None
        if bool(self._cur_req):
            t_row = libnsX.netsnmp_extract_table_row(self._cur_req)
        if bool(t_row) == False:
            return None
        size = snmpobj._data_size
        result = libnsX.netsnmp_set_row_column(
            t_row,
            colnum,
            snmpobj._asntype,
            snmpobj.cref(),
            size
        )
    
    def get_table_row(self, colnum):
        t_row = None
        if bool(self._cur_req):
            t_row = libnsX.netsnmp_extract_table_row(self._cur_req)
        if bool(t_row) == False:
            return None

        tds = ctypes.cast(t_row.contents.data, netsnmp_table_data_set_storage_p)
        while bool(tds):
            undata = tds.contents.data
            if tds.contents.column != colnum:
                tds = tds.contents.next
                continue
                
            if tds.contents.type == ASN_INTEGER or tds.contents.type == ASN_UNSIGNED:
                intv = ctypes.cast(undata.integer, ctypes.POINTER(ctypes.c_long))
                return intv[0]
            elif tds.contents.type == ASN_OCTET_STR:
                if tds.contents.data_len == 0:
                    return ""
                strv = ctypes.string_at(undata.voidp, tds.contents.data_len)
                return strv
            elif tds.contents.type == ASN_IPADDRESS:
                ipaddr = socket.inet_ntoa(struct.pack("I", undata.integer.contents.value))
                return ipaddr
            elif tds.contents.type == ASN_OBJECT_ID:
                oid_len = tds.contents.data_len
                oid = (c_oid * oid_len)()
                return ctypes.addressof(undata)

            else:
                raise netsnmpAgentException("get_table_row() please add code to process type {0}!".format(tds.contents.type))

    # APIs for setting result
    # setting return values for GET
    def set_var_typed_value(self, var_type, val_str, val_len):
        self._agent.set_var_typed_value(self._cur_req.contents.requestvb, 
        var_type, val_str, val_len)
        return SNMP_ERR_NOERROR

    # setting error id for this request
    def set_request_error(self, error_value = SNMP_ERR_GENERR):
        self._agent.set_request_error(self._reqinfo, self._cur_req, error_value)
        return SNMP_ERR_NOERROR
    
    # APIs need override by sub classes
    # colnum: table colnum id 
    def get(self, colnum): 
        # for GET
        return SNMP_ERR_NOERROR

    def check(self, colnum, var_type, data):
        # apply settings
        return SNMP_ERR_WRONGVALUE

    def _check(self, colnum, var_type, val, val_len):
        # check set parameters
        data = None

        if var_type == ASN_OCTET_STR:
            data = ctypes.string_at(val, val_len)
        elif var_type == ASN_INTEGER:
            data = ctypes.cast(val, ctypes.POINTER(ctypes.c_int))[0]
        elif var_type == ASN_IPADDRESS:
            if val_len != 4:
                return SNMP_ERR_WRONGVALUE
            ipaddr = ctypes.cast(val, ctypes.POINTER(ctypes.c_uint))[0]
            try:
                data = socket.inet_ntoa(struct.pack("I", ipaddr))
            except:
                return SNMP_ERR_WRONGVALUE

        return self.check(colnum, var_type, data)

    def backup(self, colnum, var_type, val, val_len):
        # back old setting for undo
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        # apply settings
        return SNMP_ERR_WRONGVALUE

    def action(self, colnum, var_type, val, val_len):
        # apply settings
        data = None

        if var_type == ASN_OCTET_STR:
            data = ctypes.string_at(val, val_len)
        elif var_type == ASN_INTEGER:
            data = ctypes.cast(val, ctypes.POINTER(ctypes.c_int))[0]
        elif var_type == ASN_IPADDRESS:
            if val_len != 4:
                return SNMP_ERR_WRONGVALUE
            ipaddr = ctypes.cast(val, ctypes.POINTER(ctypes.c_uint))[0]
            try:
                data = socket.inet_ntoa(struct.pack("I", ipaddr))
            except:
                return SNMP_ERR_WRONGVALUE
        elif var_type == ASN_OBJECT_ID:
            if val_len > MAX_OID_LEN:
                return SNMP_ERR_WRONGVALUE
            oid = (c_oid * val_len)()
            data = ctypes.cast(val,ctypes.POINTER(ctypes.c_oid))[0]
            #data = ctypes.addressof(oid)

        return self.set(colnum, data)

    def undo(self, colnum): 
        # undo settings if action failed
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        # subscribe data callback
        return SNMP_ERR_NOERROR

    def snmp_return_string(self, str):
        # return ASN_OCT_STRING
        return self.set_var_typed_value(ASN_OCTET_STR, str, len(str))

    def snmp_return_integer(self, value):
        # return ASN_INTEGER
        tmp = ctypes.c_ulong(0)
        tmp.value = value
        return self.set_var_typed_value(ASN_INTEGER, ctypes.cast(ctypes.byref(tmp), ctypes.c_char_p), 4)

    def snmp_return_counter64(self, value):
        # return ASN_COUNTER64
        tmp = counter64(value)
        return self.set_var_typed_value(ASN_COUNTER64, ctypes.cast(ctypes.byref(tmp), ctypes.c_char_p), 8)

    def snmp_return_timeticks(self, value):
        # return ASN_TIMETICKS
        tmp = ctypes.c_ulong(0)
        tmp.value = value
        return self.set_var_typed_value(ASN_TIMETICKS, ctypes.cast(ctypes.byref(tmp), ctypes.c_char_p), 4)

    def snmp_return_object_id(self, value):
        # return ASN_OBJECT_ID
        return self.set_var_typed_value(ASN_OBJECT_ID, value, ctypes.sizeof(value))