'''Wrapper for saiacl.h

Generated with:
/ctcdata/users/sw/kcao/p4/switch-master/third-party/ctypesgen/ctypesgen.py ./saiacl.h ./saibuffer.h ./saifdb.h ./sai.h ./saihash.h ./saihostintf.h ./saiisolation.h ./sailag.h ./saimirror.h ./saineighbor.h ./sainexthopgroup.h ./sainexthop.h ./saipolicer.h ./saiport.h ./saiqosmaps.h ./saiqueue.h ./sairoute.h ./sairouter.h ./sairouterintf.h ./saisamplepacket.h ./saischedulergroup.h ./saischeduler.h ./saistatus.h ./saistp.h ./saiswitch.h ./saitypes.h ./saiudf.h ./saivlan.h ./saiwred.h -o ./sai_headers.py

Do not modify this file.
'''

__docformat__ =  'restructuredtext'

# Begin preamble

import ctypes, os, sys
from ctypes import *

_int_types = (c_int16, c_int32)
if hasattr(ctypes, 'c_int64'):
    # Some builds of ctypes apparently do not have c_int64
    # defined; it's a pretty good bet that these builds do not
    # have 64-bit pointers.
    _int_types += (c_int64,)
for t in _int_types:
    if sizeof(t) == sizeof(c_size_t):
        c_ptrdiff_t = t
del t
del _int_types

class c_void(Structure):
    # c_void_p is a buggy return type, converting to int, so
    # POINTER(None) == c_void_p is actually written as
    # POINTER(c_void), so it can be treated as a real pointer.
    _fields_ = [('dummy', c_int)]

def POINTER(obj):
    p = ctypes.POINTER(obj)

    # Convert None to a real NULL pointer to work around bugs
    # in how ctypes handles None on 64-bit platforms
    if not isinstance(p.from_param, classmethod):
        def from_param(cls, x):
            if x is None:
                return cls()
            else:
                return x
        p.from_param = classmethod(from_param)

    return p

class UserString:
    def __init__(self, seq):
        if isinstance(seq, basestring):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else:
            self.data = str(seq)
    def __str__(self): return str(self.data)
    def __repr__(self): return repr(self.data)
    def __int__(self): return int(self.data)
    def __long__(self): return long(self.data)
    def __float__(self): return float(self.data)
    def __complex__(self): return complex(self.data)
    def __hash__(self): return hash(self.data)

    def __cmp__(self, string):
        if isinstance(string, UserString):
            return cmp(self.data, string.data)
        else:
            return cmp(self.data, string)
    def __contains__(self, char):
        return char in self.data

    def __len__(self): return len(self.data)
    def __getitem__(self, index): return self.__class__(self.data[index])
    def __getslice__(self, start, end):
        start = max(start, 0); end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, basestring):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other))
    def __radd__(self, other):
        if isinstance(other, basestring):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other) + self.data)
    def __mul__(self, n):
        return self.__class__(self.data*n)
    __rmul__ = __mul__
    def __mod__(self, args):
        return self.__class__(self.data % args)

    # the following methods are defined in alphabetical order:
    def capitalize(self): return self.__class__(self.data.capitalize())
    def center(self, width, *args):
        return self.__class__(self.data.center(width, *args))
    def count(self, sub, start=0, end=sys.maxint):
        return self.data.count(sub, start, end)
    def decode(self, encoding=None, errors=None): # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.decode(encoding, errors))
            else:
                return self.__class__(self.data.decode(encoding))
        else:
            return self.__class__(self.data.decode())
    def encode(self, encoding=None, errors=None): # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else:
            return self.__class__(self.data.encode())
    def endswith(self, suffix, start=0, end=sys.maxint):
        return self.data.endswith(suffix, start, end)
    def expandtabs(self, tabsize=8):
        return self.__class__(self.data.expandtabs(tabsize))
    def find(self, sub, start=0, end=sys.maxint):
        return self.data.find(sub, start, end)
    def index(self, sub, start=0, end=sys.maxint):
        return self.data.index(sub, start, end)
    def isalpha(self): return self.data.isalpha()
    def isalnum(self): return self.data.isalnum()
    def isdecimal(self): return self.data.isdecimal()
    def isdigit(self): return self.data.isdigit()
    def islower(self): return self.data.islower()
    def isnumeric(self): return self.data.isnumeric()
    def isspace(self): return self.data.isspace()
    def istitle(self): return self.data.istitle()
    def isupper(self): return self.data.isupper()
    def join(self, seq): return self.data.join(seq)
    def ljust(self, width, *args):
        return self.__class__(self.data.ljust(width, *args))
    def lower(self): return self.__class__(self.data.lower())
    def lstrip(self, chars=None): return self.__class__(self.data.lstrip(chars))
    def partition(self, sep):
        return self.data.partition(sep)
    def replace(self, old, new, maxsplit=-1):
        return self.__class__(self.data.replace(old, new, maxsplit))
    def rfind(self, sub, start=0, end=sys.maxint):
        return self.data.rfind(sub, start, end)
    def rindex(self, sub, start=0, end=sys.maxint):
        return self.data.rindex(sub, start, end)
    def rjust(self, width, *args):
        return self.__class__(self.data.rjust(width, *args))
    def rpartition(self, sep):
        return self.data.rpartition(sep)
    def rstrip(self, chars=None): return self.__class__(self.data.rstrip(chars))
    def split(self, sep=None, maxsplit=-1):
        return self.data.split(sep, maxsplit)
    def rsplit(self, sep=None, maxsplit=-1):
        return self.data.rsplit(sep, maxsplit)
    def splitlines(self, keepends=0): return self.data.splitlines(keepends)
    def startswith(self, prefix, start=0, end=sys.maxint):
        return self.data.startswith(prefix, start, end)
    def strip(self, chars=None): return self.__class__(self.data.strip(chars))
    def swapcase(self): return self.__class__(self.data.swapcase())
    def title(self): return self.__class__(self.data.title())
    def translate(self, *args):
        return self.__class__(self.data.translate(*args))
    def upper(self): return self.__class__(self.data.upper())
    def zfill(self, width): return self.__class__(self.data.zfill(width))

class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""
    def __init__(self, string=""):
        self.data = string
    def __hash__(self):
        raise TypeError("unhashable type (it is mutable)")
    def __setitem__(self, index, sub):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data): raise IndexError
        self.data = self.data[:index] + sub + self.data[index+1:]
    def __delitem__(self, index):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data): raise IndexError
        self.data = self.data[:index] + self.data[index+1:]
    def __setslice__(self, start, end, sub):
        start = max(start, 0); end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start]+sub.data+self.data[end:]
        elif isinstance(sub, basestring):
            self.data = self.data[:start]+sub+self.data[end:]
        else:
            self.data =  self.data[:start]+str(sub)+self.data[end:]
    def __delslice__(self, start, end):
        start = max(start, 0); end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]
    def immutable(self):
        return UserString(self.data)
    def __iadd__(self, other):
        if isinstance(other, UserString):
            self.data += other.data
        elif isinstance(other, basestring):
            self.data += other
        else:
            self.data += str(other)
        return self
    def __imul__(self, n):
        self.data *= n
        return self

class String(MutableString, Union):

    _fields_ = [('raw', POINTER(c_char)),
                ('data', c_char_p)]

    def __init__(self, obj=""):
        if isinstance(obj, (str, unicode, UserString)):
            self.data = str(obj)
        else:
            self.raw = obj

    def __len__(self):
        return self.data and len(self.data) or 0

    def from_param(cls, obj):
        # Convert None or 0
        if obj is None or obj == 0:
            return cls(POINTER(c_char)())

        # Convert from String
        elif isinstance(obj, String):
            return obj

        # Convert from str
        elif isinstance(obj, str):
            return cls(obj)

        # Convert from c_char_p
        elif isinstance(obj, c_char_p):
            return obj

        # Convert from POINTER(c_char)
        elif isinstance(obj, POINTER(c_char)):
            return obj

        # Convert from raw pointer
        elif isinstance(obj, int):
            return cls(cast(obj, POINTER(c_char)))

        # Convert from object
        else:
            return String.from_param(obj._as_parameter_)
    from_param = classmethod(from_param)

def ReturnString(obj, func=None, arguments=None):
    return String.from_param(obj)

# As of ctypes 1.0, ctypes does not support custom error-checking
# functions on callbacks, nor does it support custom datatypes on
# callbacks, so we must ensure that all callbacks return
# primitive datatypes.
#
# Non-primitive return values wrapped with UNCHECKED won't be
# typechecked, and will be converted to c_void_p.
def UNCHECKED(type):
    if (hasattr(type, "_type_") and isinstance(type._type_, str)
        and type._type_ != "P"):
        return type
    else:
        return c_void_p

# ctypes doesn't have direct support for variadic functions, so we have to write
# our own wrapper class
class _variadic_function(object):
    def __init__(self,func,restype,argtypes):
        self.func=func
        self.func.restype=restype
        self.argtypes=argtypes
    def _as_parameter_(self):
        # So we can pass this variadic function as a function pointer
        return self.func
    def __call__(self,*args):
        fixed_args=[]
        i=0
        for argtype in self.argtypes:
            # Typecheck what we can
            fixed_args.append(argtype.from_param(args[i]))
            i+=1
        return self.func(*fixed_args+list(args[i:]))

# End preamble

_libs = {}
_libdirs = []

# Begin loader

# ----------------------------------------------------------------------------
# Copyright (c) 2008 David James
# Copyright (c) 2006-2008 Alex Holkner
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

import os.path, re, sys, glob
import platform
import ctypes
import ctypes.util

def _environ_path(name):
    if name in os.environ:
        return os.environ[name].split(":")
    else:
        return []

class LibraryLoader(object):
    def __init__(self):
        self.other_dirs=[]

    def load_library(self,libname):
        """Given the name of a library, load it."""
        paths = self.getpaths(libname)

        for path in paths:
            if os.path.exists(path):
                return self.load(path)

        raise ImportError("%s not found." % libname)

    def load(self,path):
        """Given a path to a library, load it."""
        try:
            # Darwin requires dlopen to be called with mode RTLD_GLOBAL instead
            # of the default RTLD_LOCAL.  Without this, you end up with
            # libraries not being loadable, resulting in "Symbol not found"
            # errors
            if sys.platform == 'darwin':
                return ctypes.CDLL(path, ctypes.RTLD_GLOBAL)
            else:
                return ctypes.cdll.LoadLibrary(path)
        except OSError,e:
            raise ImportError(e)

    def getpaths(self,libname):
        """Return a list of paths where the library might be found."""
        if os.path.isabs(libname):
            yield libname
        else:
            # FIXME / TODO return '.' and os.path.dirname(__file__)
            for path in self.getplatformpaths(libname):
                yield path

            path = ctypes.util.find_library(libname)
            if path: yield path

    def getplatformpaths(self, libname):
        return []

# Darwin (Mac OS X)

class DarwinLibraryLoader(LibraryLoader):
    name_formats = ["lib%s.dylib", "lib%s.so", "lib%s.bundle", "%s.dylib",
                "%s.so", "%s.bundle", "%s"]

    def getplatformpaths(self,libname):
        if os.path.pathsep in libname:
            names = [libname]
        else:
            names = [format % libname for format in self.name_formats]

        for dir in self.getdirs(libname):
            for name in names:
                yield os.path.join(dir,name)

    def getdirs(self,libname):
        '''Implements the dylib search as specified in Apple documentation:

        http://developer.apple.com/documentation/DeveloperTools/Conceptual/
            DynamicLibraries/Articles/DynamicLibraryUsageGuidelines.html

        Before commencing the standard search, the method first checks
        the bundle's ``Frameworks`` directory if the application is running
        within a bundle (OS X .app).
        '''

        dyld_fallback_library_path = _environ_path("DYLD_FALLBACK_LIBRARY_PATH")
        if not dyld_fallback_library_path:
            dyld_fallback_library_path = [os.path.expanduser('~/lib'),
                                          '/usr/local/lib', '/usr/lib']

        dirs = []

        if '/' in libname:
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
        else:
            dirs.extend(_environ_path("LD_LIBRARY_PATH"))
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))

        dirs.extend(self.other_dirs)
        dirs.append(".")
        dirs.append(os.path.dirname(__file__))

        if hasattr(sys, 'frozen') and sys.frozen == 'macosx_app':
            dirs.append(os.path.join(
                os.environ['RESOURCEPATH'],
                '..',
                'Frameworks'))

        dirs.extend(dyld_fallback_library_path)

        return dirs

# Posix

class PosixLibraryLoader(LibraryLoader):
    _ld_so_cache = None

    def _create_ld_so_cache(self):
        # Recreate search path followed by ld.so.  This is going to be
        # slow to build, and incorrect (ld.so uses ld.so.cache, which may
        # not be up-to-date).  Used only as fallback for distros without
        # /sbin/ldconfig.
        #
        # We assume the DT_RPATH and DT_RUNPATH binary sections are omitted.

        directories = []
        for name in ("LD_LIBRARY_PATH",
                     "SHLIB_PATH", # HPUX
                     "LIBPATH", # OS/2, AIX
                     "LIBRARY_PATH", # BE/OS
                    ):
            if name in os.environ:
                directories.extend(os.environ[name].split(os.pathsep))
        directories.extend(self.other_dirs)
        directories.append(".")
        directories.append(os.path.dirname(__file__))

        try: directories.extend([dir.strip() for dir in open('/etc/ld.so.conf')])
        except IOError: pass

        unix_lib_dirs_list = ['/lib', '/usr/lib', '/lib64', '/usr/lib64']
        if sys.platform.startswith('linux'):
            # Try and support multiarch work in Ubuntu
            # https://wiki.ubuntu.com/MultiarchSpec
            bitage = platform.architecture()[0]
            if bitage.startswith('32'):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ['/lib/i386-linux-gnu', '/usr/lib/i386-linux-gnu']
            elif bitage.startswith('64'):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ['/lib/x86_64-linux-gnu', '/usr/lib/x86_64-linux-gnu']
            else:
                # guess...
                unix_lib_dirs_list += glob.glob('/lib/*linux-gnu')
        directories.extend(unix_lib_dirs_list)

        cache = {}
        lib_re = re.compile(r'lib(.*)\.s[ol]')
        ext_re = re.compile(r'\.s[ol]$')
        for dir in directories:
            try:
                for path in glob.glob("%s/*.s[ol]*" % dir):
                    file = os.path.basename(path)

                    # Index by filename
                    if file not in cache:
                        cache[file] = path

                    # Index by library name
                    match = lib_re.match(file)
                    if match:
                        library = match.group(1)
                        if library not in cache:
                            cache[library] = path
            except OSError:
                pass

        self._ld_so_cache = cache

    def getplatformpaths(self, libname):
        if self._ld_so_cache is None:
            self._create_ld_so_cache()

        result = self._ld_so_cache.get(libname)
        if result: yield result

        path = ctypes.util.find_library(libname)
        if path: yield os.path.join("/lib",path)

# Windows

class _WindowsLibrary(object):
    def __init__(self, path):
        self.cdll = ctypes.cdll.LoadLibrary(path)
        self.windll = ctypes.windll.LoadLibrary(path)

    def __getattr__(self, name):
        try: return getattr(self.cdll,name)
        except AttributeError:
            try: return getattr(self.windll,name)
            except AttributeError:
                raise

class WindowsLibraryLoader(LibraryLoader):
    name_formats = ["%s.dll", "lib%s.dll", "%slib.dll"]

    def load_library(self, libname):
        try:
            result = LibraryLoader.load_library(self, libname)
        except ImportError:
            result = None
            if os.path.sep not in libname:
                for name in self.name_formats:
                    try:
                        result = getattr(ctypes.cdll, name % libname)
                        if result:
                            break
                    except WindowsError:
                        result = None
            if result is None:
                try:
                    result = getattr(ctypes.cdll, libname)
                except WindowsError:
                    result = None
            if result is None:
                raise ImportError("%s not found." % libname)
        return result

    def load(self, path):
        return _WindowsLibrary(path)

    def getplatformpaths(self, libname):
        if os.path.sep not in libname:
            for name in self.name_formats:
                dll_in_current_dir = os.path.abspath(name % libname)
                if os.path.exists(dll_in_current_dir):
                    yield dll_in_current_dir
                path = ctypes.util.find_library(name % libname)
                if path:
                    yield path

# Platform switching

# If your value of sys.platform does not appear in this dict, please contact
# the Ctypesgen maintainers.

loaderclass = {
    "darwin":   DarwinLibraryLoader,
    "cygwin":   WindowsLibraryLoader,
    "win32":    WindowsLibraryLoader
}

loader = loaderclass.get(sys.platform, PosixLibraryLoader)()

def add_library_search_dirs(other_dirs):
    loader.other_dirs = other_dirs

load_library = loader.load_library

del loaderclass

# End loader

add_library_search_dirs([])

# No libraries

# No modules

enum__sai_acl_stage_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 59

SAI_ACL_STAGE_INGRESS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 59

SAI_ACL_STAGE_EGRESS = (SAI_ACL_STAGE_INGRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 59

SAI_ACL_SUBSTAGE_INGRESS_PRE_L2 = (SAI_ACL_STAGE_EGRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 59

SAI_ACL_SUBSTAGE_INGRESS_POST_L3 = (SAI_ACL_SUBSTAGE_INGRESS_PRE_L2 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 59

sai_acl_stage_t = enum__sai_acl_stage_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 59

enum__sai_acl_ip_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_ANY = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_IP = (SAI_ACL_IP_TYPE_ANY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_NON_IP = (SAI_ACL_IP_TYPE_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_IPv4ANY = (SAI_ACL_IP_TYPE_NON_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_NON_IPv4 = (SAI_ACL_IP_TYPE_IPv4ANY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_IPv6ANY = (SAI_ACL_IP_TYPE_NON_IPv4 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_NON_IPv6 = (SAI_ACL_IP_TYPE_IPv6ANY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_ARP = (SAI_ACL_IP_TYPE_NON_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_ARP_REQUEST = (SAI_ACL_IP_TYPE_ARP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

SAI_ACL_IP_TYPE_ARP_REPLY = (SAI_ACL_IP_TYPE_ARP_REQUEST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

sai_acl_ip_type_t = enum__sai_acl_ip_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 93

enum__sai_acl_ip_frag_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

SAI_ACL_IP_FRAG_ANY = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

SAI_ACL_IP_FRAG_NON_FRAG = (SAI_ACL_IP_FRAG_ANY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

SAI_ACL_IP_FRAG_NON_FRAG_OR_HEAD = (SAI_ACL_IP_FRAG_NON_FRAG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

SAI_ACL_IP_FRAG_HEAD = (SAI_ACL_IP_FRAG_NON_FRAG_OR_HEAD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

SAI_ACL_IP_FRAG_NON_HEAD = (SAI_ACL_IP_FRAG_HEAD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

sai_acl_ip_frag_t = enum__sai_acl_ip_frag_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 112

enum__sai_acl_table_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_STAGE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_PRIORITY = (SAI_ACL_TABLE_ATTR_STAGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_SIZE = (SAI_ACL_TABLE_ATTR_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_GROUP_ID = (SAI_ACL_TABLE_ATTR_SIZE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_START = 4096 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_SRC_IPv6 = SAI_ACL_TABLE_ATTR_FIELD_START # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_DST_IPv6 = (SAI_ACL_TABLE_ATTR_FIELD_SRC_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC = (SAI_ACL_TABLE_ATTR_FIELD_DST_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_DST_MAC = (SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_SRC_IP = (SAI_ACL_TABLE_ATTR_FIELD_DST_MAC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_DST_IP = (SAI_ACL_TABLE_ATTR_FIELD_SRC_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS = (SAI_ACL_TABLE_ATTR_FIELD_DST_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS = (SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IN_PORT = (SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT = (SAI_ACL_TABLE_ATTR_FIELD_IN_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID = (SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI = (SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI = (SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID = (SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI = (SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI = (SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT = (SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT = (SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE = (SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL = (SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_DSCP = (SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_ECN = (SAI_ACL_TABLE_ATTR_FIELD_DSCP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_TTL = (SAI_ACL_TABLE_ATTR_FIELD_ECN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_TOS = (SAI_ACL_TABLE_ATTR_FIELD_TTL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS = (SAI_ACL_TABLE_ATTR_FIELD_TOS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS = (SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IP_TYPE = (SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IP_FRAG = (SAI_ACL_TABLE_ATTR_FIELD_IP_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_IPv6_FLOW_LABEL = (SAI_ACL_TABLE_ATTR_FIELD_IP_FRAG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_TC = (SAI_ACL_TABLE_ATTR_FIELD_IPv6_FLOW_LABEL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE = (SAI_ACL_TABLE_ATTR_FIELD_TC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE = (SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META = (SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META = (SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META = (SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META = (SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META = (SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META = (SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_NPU_META_HIT = (SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_NPU_META_HIT = (SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_NPU_META_HIT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_FIELD_END = SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_NPU_META_HIT # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

SAI_ACL_TABLE_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

sai_acl_table_attr_t = enum__sai_acl_table_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 330

enum__sai_acl_entry_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_TABLE_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_PRIORITY = (SAI_ACL_ENTRY_ATTR_TABLE_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ADMIN_STATE = (SAI_ACL_ENTRY_ATTR_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_START = 4096 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPv6 = SAI_ACL_ENTRY_ATTR_FIELD_START # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_DST_IPv6 = (SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC = (SAI_ACL_ENTRY_ATTR_FIELD_DST_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC = (SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP = (SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_DST_IP = (SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS = (SAI_ACL_ENTRY_ATTR_FIELD_DST_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS = (SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT = (SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT = (SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID = (SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI = (SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI = (SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID = (SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI = (SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI = (SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT = (SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT = (SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE = (SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL = (SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_DSCP = (SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_ECN = (SAI_ACL_ENTRY_ATTR_FIELD_DSCP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_TTL = (SAI_ACL_ENTRY_ATTR_FIELD_ECN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_TOS = (SAI_ACL_ENTRY_ATTR_FIELD_TTL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS = (SAI_ACL_ENTRY_ATTR_FIELD_TOS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS = (SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IP_TYPE = (SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IP_FRAG = (SAI_ACL_ENTRY_ATTR_FIELD_IP_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_IPv6_FLOW_LABEL = (SAI_ACL_ENTRY_ATTR_FIELD_IP_FRAG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_TC = (SAI_ACL_ENTRY_ATTR_FIELD_IPv6_FLOW_LABEL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE = (SAI_ACL_ENTRY_ATTR_FIELD_TC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE = (SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META = (SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META = (SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_USER_META = (SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META = (SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META = (SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META = (SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT = (SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT = (SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_FIELD_END = SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_START = 8192 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT = SAI_ACL_ENTRY_ATTR_ACTION_START # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_PACKET_ACTION = (SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_FLOOD = (SAI_ACL_ENTRY_ATTR_PACKET_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_COUNTER = (SAI_ACL_ENTRY_ATTR_ACTION_FLOOD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS = (SAI_ACL_ENTRY_ATTR_ACTION_COUNTER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS = (SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER = (SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL = (SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_TC = (SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_COLOR = (SAI_ACL_ENTRY_ATTR_ACTION_SET_TC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID = (SAI_ACL_ENTRY_ATTR_ACTION_SET_COLOR + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI = (SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID = (SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI = (SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC = (SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC = (SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP = (SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP = (SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPv6 = (SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPv6 = (SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP = (SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPv6 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN = (SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT = (SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT = (SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE = (SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE = (SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE = (SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA = (SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

SAI_ACL_ENTRY_ATTR_ACTION_END = SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

sai_acl_entry_attr_t = enum__sai_acl_entry_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 607

enum__sai_acl_counter_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

SAI_ACL_COUNTER_ATTR_TABLE_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT = (SAI_ACL_COUNTER_ATTR_TABLE_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT = (SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

SAI_ACL_COUNTER_ATTR_PACKETS = (SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

SAI_ACL_COUNTER_ATTR_BYTES = (SAI_ACL_COUNTER_ATTR_PACKETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

sai_acl_counter_attr_t = enum__sai_acl_counter_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiacl.h: 640

enum__sai_ingress_priority_group_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 47

SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 47

sai_ingress_priority_group_attr_t = enum__sai_ingress_priority_group_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 47

enum__sai_buffer_pool_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 88

SAI_BUFFER_POOL_INGRESS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 88

SAI_BUFFER_POOL_EGRESS = (SAI_BUFFER_POOL_INGRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 88

sai_buffer_pool_type_t = enum__sai_buffer_pool_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 88

enum__sai_buffer_threshold_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 101

SAI_BUFFER_THRESHOLD_MODE_STATIC = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 101

SAI_BUFFER_THRESHOLD_MODE_DYNAMIC = (SAI_BUFFER_THRESHOLD_MODE_STATIC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 101

sai_buffer_threshold_mode_t = enum__sai_buffer_threshold_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 101

enum__sai_buffer_pool_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 127

SAI_BUFFER_POOL_ATTR_SHARED_SIZE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 127

SAI_BUFFER_POOL_ATTR_TYPE = (SAI_BUFFER_POOL_ATTR_SHARED_SIZE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 127

SAI_BUFFER_POOL_ATTR_SIZE = (SAI_BUFFER_POOL_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 127

SAI_BUFFER_POOL_ATTR_TH_MODE = (SAI_BUFFER_POOL_ATTR_SIZE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 127

sai_buffer_pool_attr_t = enum__sai_buffer_pool_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 127

enum__sai_buffer_profile_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

SAI_BUFFER_PROFILE_ATTR_POOL_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE = (SAI_BUFFER_PROFILE_ATTR_POOL_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH = (SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH = (SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

SAI_BUFFER_PROFILE_ATTR_XOFF_TH = (SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

SAI_BUFFER_PROFILE_ATTR_XON_TH = (SAI_BUFFER_PROFILE_ATTR_XOFF_TH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

sai_buffer_profile_attr_t = enum__sai_buffer_profile_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saibuffer.h: 218

enum__sai_fdb_entry_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 58

SAI_FDB_ENTRY_DYNAMIC = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 58

SAI_FDB_ENTRY_STATIC = (SAI_FDB_ENTRY_DYNAMIC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 58

SAI_FDB_ENTRY_DYNAMIC_WO_AGING = (SAI_FDB_ENTRY_STATIC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 58

SAI_FDB_ENTRY_DYNAMIC_SECURITY = (SAI_FDB_ENTRY_DYNAMIC_WO_AGING + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 58

sai_fdb_entry_type_t = enum__sai_fdb_entry_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 58

enum_sai_fdb_event_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 85

SAI_FDB_EVENT_LEARNED = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 85

SAI_FDB_EVENT_AGED = (SAI_FDB_EVENT_LEARNED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 85

SAI_FDB_EVENT_FLUSHED = (SAI_FDB_EVENT_AGED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 85

sai_fdb_event_t = enum_sai_fdb_event_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 85

enum__sai_fdb_entry_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

SAI_FDB_ENTRY_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

SAI_FDB_ENTRY_ATTR_PORT_ID = (SAI_FDB_ENTRY_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

SAI_FDB_ENTRY_ATTR_PACKET_ACTION = (SAI_FDB_ENTRY_ATTR_PORT_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

SAI_FDB_ENTRY_ATTR_META_DATA = (SAI_FDB_ENTRY_ATTR_PACKET_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

SAI_FDB_ENTRY_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

sai_fdb_entry_attr_t = enum__sai_fdb_entry_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 117

enum__sai_fdb_flush_entry_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 130

SAI_FDB_FLUSH_ENTRY_DYNAMIC = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 130

SAI_FDB_FLUSH_ENTRY_STATIC = (SAI_FDB_FLUSH_ENTRY_DYNAMIC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 130

sai_fdb_flush_entry_type_t = enum__sai_fdb_flush_entry_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 130

enum__sai_fdb_flush_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 158

SAI_FDB_FLUSH_ATTR_PORT_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 158

SAI_FDB_FLUSH_ATTR_VLAN_ID = (SAI_FDB_FLUSH_ATTR_PORT_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 158

SAI_FDB_FLUSH_ATTR_ENTRY_TYPE = (SAI_FDB_FLUSH_ATTR_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 158

sai_fdb_flush_attr_t = enum__sai_fdb_flush_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saifdb.h: 158

enum__sai_api_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_UNSPECIFIED = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_SWITCH = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_PORT = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_FDB = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_VLAN = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_VIRTUAL_ROUTER = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_ROUTE = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_NEXT_HOP = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_NEXT_HOP_GROUP = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_ROUTER_INTERFACE = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_NEIGHBOR = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_ACL = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_HOST_INTERFACE = 12 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_MIRROR = 13 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_SAMPLEPACKET = 14 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_STP = 15 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_LAG = 16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_POLICER = 17 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_WRED = 18 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_QOS_MAPS = 19 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_QUEUE = 20 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_SCHEDULER = 21 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_SCHEDULER_GROUP = 22 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_BUFFERS = 23 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_HASH = 24 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_UDF = 25 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

SAI_API_ISOLATION = 26 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

sai_api_t = enum__sai_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 104

enum__sai_log_level_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

SAI_LOG_DEBUG = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

SAI_LOG_INFO = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

SAI_LOG_NOTICE = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

SAI_LOG_WARN = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

SAI_LOG_ERROR = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

SAI_LOG_CRITICAL = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

sai_log_level_t = enum__sai_log_level_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sai.h: 114

enum__sai_native_hash_field = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_SRC_IP = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_DST_IP = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_INNER_SRC_IP = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_INNER_DST_IP = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_VLAN_ID = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_IP_PROTOCOL = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_ETHERTYPE = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_L4_SRC_PORT = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_L4_DST_PORT = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_SRC_MAC = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_DST_MAC = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

SAI_NATIVE_HASH_FIELD_IN_PORT = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

sai_native_hash_field_t = enum__sai_native_hash_field # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 85

enum__sai_hash_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 102

SAI_HASH_NATIVE_FIELD_LIST = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 102

SAI_HASH_UDF_GROUP_LIST = (SAI_HASH_NATIVE_FIELD_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 102

sai_hash_attr_t = enum__sai_hash_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihash.h: 102

enum__sai_hostif_trap_group_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

SAI_HOSTIF_TRAP_GROUP_ATTR_PRIO = (SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE = (SAI_HOSTIF_TRAP_GROUP_ATTR_PRIO + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER = (SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

SAI_HOSTIF_TRAP_GROUP_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

sai_hostif_trap_group_attr_t = enum__sai_hostif_trap_group_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 63

enum__sai_hostif_trap_id_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_STP = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_LACP = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_EAPOL = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_LLDP = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_PVRST = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_SAMPLEPACKET = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_SWITCH_CUSTOM_RANGE_BASE = 4096 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_ARP_REQUEST = 8192 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_ARP_RESPONSE = 8193 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_DHCP = 8194 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_OSPF = 8195 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_PIM = 8196 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_VRRP = 8197 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_BGP = 8198 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_DHCPV6 = 8199 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_OSPFV6 = 8200 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_VRRPV6 = 8201 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_BGPV6 = 8202 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY = 8203 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_V2 = 8204 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_REPORT = 8205 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_DONE = 8206 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_MLD_V2_REPORT = 8207 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_ROUTER_CUSTOM_RANGE_BASE = 12288 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_L3_MTU_ERROR = 16384 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_TTL_ERROR = 16385 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_IPMC_RPF_FAIL = 16386 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT = 16387 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_RANGE_BASE = 20480 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

sai_hostif_trap_id_t = enum__sai_hostif_trap_id_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 256

enum__sai_hostif_trap_channel_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 271

SAI_HOSTIF_TRAP_CHANNEL_FD = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 271

SAI_HOSTIF_TRAP_CHANNEL_CB = (SAI_HOSTIF_TRAP_CHANNEL_FD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 271

SAI_HOSTIF_TRAP_CHANNEL_NETDEV = (SAI_HOSTIF_TRAP_CHANNEL_CB + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 271

SAI_HOSTIF_TRAP_CHANNEL_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 271

sai_hostif_trap_channel_t = enum__sai_hostif_trap_channel_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 271

enum__sai_hostif_trap_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY = (SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_TRAP_CHANNEL = (SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_FD = (SAI_HOSTIF_TRAP_ATTR_TRAP_CHANNEL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_PORT_LIST = (SAI_HOSTIF_TRAP_ATTR_FD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP = (SAI_HOSTIF_TRAP_ATTR_PORT_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

SAI_HOSTIF_TRAP_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

sai_hostif_trap_attr_t = enum__sai_hostif_trap_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 307

enum__sai_hostif_user_defined_trap_id_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_SAMPLEPACKET = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_ACL_MIN = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_ACL_MAX = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_ACL_MIN + 0) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_ROUTER_MIN = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_ACL_MAX + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_ROUTER_MAX = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_ROUTER_MIN + 0) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_NEIGH_MIN = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_ROUTER_MAX + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_NEIGH_MAX = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_NEIGH_MIN + 0) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_FDB_MIN = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_NEIGH_MAX + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_USER_DEFINED_TRAP_ID_FDB_MAX = (SAI_HOSTIF_USER_DEFINED_TRAP_ID_FDB_MIN + 0) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

SAI_HOSTIF_TRAP_ID_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

sai_hostif_user_defined_trap_id_t = enum__sai_hostif_user_defined_trap_id_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 377

enum__sai_hostif_user_defined_trap_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 391

SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_CHANNEL = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 391

SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_FD = (SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_CHANNEL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 391

sai_hostif_user_defined_trap_attr_t = enum__sai_hostif_user_defined_trap_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 391

enum__sai_hostif_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 440

SAI_HOSTIF_TYPE_NETDEV = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 440

SAI_HOSTIF_TYPE_FD = (SAI_HOSTIF_TYPE_NETDEV + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 440

sai_hostif_type_t = enum__sai_hostif_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 440

enum__sai_hostif_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 469

SAI_HOSTIF_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 469

SAI_HOSTIF_ATTR_RIF_OR_PORT_ID = (SAI_HOSTIF_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 469

SAI_HOSTIF_ATTR_NAME = (SAI_HOSTIF_ATTR_RIF_OR_PORT_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 469

SAI_HOSTIF_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 469

sai_hostif_attr_t = enum__sai_hostif_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 469

enum__sai_hostif_tx_type = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 552

SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 552

SAI_HOSTIF_TX_TYPE_PIPELINE_LOOKUP = (SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 552

SAI_HOSTIF_TX_TYPE_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 552

sai_hostif_tx_type_t = enum__sai_hostif_tx_type # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 552

enum__sai_hostif_packet_attr = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

SAI_HOSTIF_PACKET_TRAP_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

SAI_HOSTIF_PACKET_INGRESS_PORT = (SAI_HOSTIF_PACKET_TRAP_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

SAI_HOSTIF_PACKET_INGRESS_LAG = (SAI_HOSTIF_PACKET_INGRESS_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

SAI_HOSTIF_PACKET_TX_TYPE = (SAI_HOSTIF_PACKET_INGRESS_LAG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

SAI_HOSTIF_PACKET_EGRESS_PORT_OR_LAG = (SAI_HOSTIF_PACKET_TX_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

SAI_HOSTIF_PACKET_VLAN_ID = (SAI_HOSTIF_PACKET_EGRESS_PORT_OR_LAG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

sai_hostif_packet_attr_t = enum__sai_hostif_packet_attr # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 575

enum__sai_isolaton_group_drop_pkt_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_UC = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_UC = (SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_UC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_MC = (SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_UC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_MC = (SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_MC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_BC = (SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_MC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL = (SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_BC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

sai_isolaton_group_drop_pkt_type_t = enum__sai_isolaton_group_drop_pkt_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 63

enum__sai_isolaton_group_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 79

SAI_ISOLATION_GROUP_MODE_ISOLATION = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 79

SAI_ISOLATION_GROUP_MODE_MLAG = (SAI_ISOLATION_GROUP_MODE_ISOLATION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 79

SAI_ISOLATION_GROUP_MODE_PVLAN = (SAI_ISOLATION_GROUP_MODE_MLAG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 79

sai_isolaton_group_mode_t = enum__sai_isolaton_group_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 79

enum__sai_isolation_group_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

SAI_ISOLATION_ATTR_EGRESS_BLOCK_PORT_LIST = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

SAI_ISOLATION_ATTR_INRESS_BLOCK_PORT_LIST = (SAI_ISOLATION_ATTR_EGRESS_BLOCK_PORT_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

SAI_ISOLATION_ATTR_DROP_PACKET_TYPE = (SAI_ISOLATION_ATTR_INRESS_BLOCK_PORT_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

SAI_ISOLATION_ATTR_MODE = (SAI_ISOLATION_ATTR_DROP_PACKET_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

SAI_ISOLATION_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

sai_isolation_group_attr_t = enum__sai_isolation_group_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiisolation.h: 107

enum__sai_lag_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 46

SAI_LAG_ATTR_PORT_LIST = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 46

SAI_LAG_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 46

sai_lag_attr_t = enum__sai_lag_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 46

enum__sai_lag_member_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

SAI_LAG_MEMBER_ATTR_LAG_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

SAI_LAG_MEMBER_ATTR_PORT_ID = (SAI_LAG_MEMBER_ATTR_LAG_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE = (SAI_LAG_MEMBER_ATTR_PORT_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE = (SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

SAI_LAG_MEMBER_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

sai_lag_member_attr_t = enum__sai_lag_member_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sailag.h: 122

bool = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 86

sai_status_t = c_int32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 94

sai_switch_profile_id_t = c_uint32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 95

sai_vlan_id_t = c_uint16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 96

sai_attr_id_t = c_uint32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 97

sai_cos_t = c_uint8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 98

sai_queue_index_t = c_uint8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 99

sai_mac_t = c_uint8 * 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 100

sai_ip4_t = c_uint32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 101

sai_ip6_t = c_uint8 * 16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 102

sai_switch_hash_seed_t = c_uint32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 103

sai_uint64_t = c_uint64 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 116

sai_int64_t = c_int64 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 117

sai_uint32_t = c_uint32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 118

sai_int32_t = c_int32 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 119

sai_uint16_t = c_uint16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 120

sai_int16_t = c_int16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 121

sai_uint8_t = c_uint8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 122

sai_int8_t = c_int8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 123

sai_size_t = c_size_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 124

sai_object_id_t = c_uint64 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 125

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 148
class struct__sai_object_list_t(Structure):
    pass

struct__sai_object_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_object_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(sai_object_id_t)),
]

sai_object_list_t = struct__sai_object_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 148

enum__sai_object_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_NULL = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_PORT = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_LAG = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_VIRTUAL_ROUTER = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_NEXT_HOP = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_NEXT_HOP_GROUP = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_ROUTER_INTERFACE = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_ACL_TABLE = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_ACL_ENTRY = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_ACL_COUNTER = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_HOST_INTERFACE = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_MIRROR = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_SAMPLEPACKET = 12 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_STP_INSTANCE = 13 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_TRAP_GROUP = 14 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_ACL_TABLE_GROUP = 15 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_POLICER = 16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_WRED = 17 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_QOS_MAPS = 18 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_QUEUE = 19 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_SCHEDULER = 20 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_SCHEDULER_GROUP = 21 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_BUFFER_POOL = 22 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_BUFFER_PROFILE = 23 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_PRIORITY_GROUP = 24 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_LAG_MEMBER = 25 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_HASH = 25 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_UDF = 26 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_UDF_MATCH = 27 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_UDF_GROUP = 28 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_ISOLATION_GROUP = 29 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

SAI_OBJECT_TYPE_MAX = 30 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

sai_object_type_t = enum__sai_object_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 186

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 191
class struct__sai_u8_list_t(Structure):
    pass

struct__sai_u8_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_u8_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(c_uint8)),
]

sai_u8_list_t = struct__sai_u8_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 191

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 196
class struct__sai_s8_list_t(Structure):
    pass

struct__sai_s8_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_s8_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(c_int8)),
]

sai_s8_list_t = struct__sai_s8_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 196

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 201
class struct__sai_u16_list_t(Structure):
    pass

struct__sai_u16_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_u16_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(c_uint16)),
]

sai_u16_list_t = struct__sai_u16_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 201

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 206
class struct__sai_s16_list_t(Structure):
    pass

struct__sai_s16_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_s16_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(c_int16)),
]

sai_s16_list_t = struct__sai_s16_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 206

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 211
class struct__sai_u32_list_t(Structure):
    pass

struct__sai_u32_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_u32_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(c_uint32)),
]

sai_u32_list_t = struct__sai_u32_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 211

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 216
class struct__sai_s32_list_t(Structure):
    pass

struct__sai_s32_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_s32_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(c_int32)),
]

sai_s32_list_t = struct__sai_s32_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 216

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 221
class struct__sai_u32_range_t(Structure):
    pass

struct__sai_u32_range_t.__slots__ = [
    'min',
    'max',
]
struct__sai_u32_range_t._fields_ = [
    ('min', c_uint32),
    ('max', c_uint32),
]

sai_u32_range_t = struct__sai_u32_range_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 221

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 226
class struct__sai_s32_range_t(Structure):
    pass

struct__sai_s32_range_t.__slots__ = [
    'min',
    'max',
]
struct__sai_s32_range_t._fields_ = [
    ('min', c_int32),
    ('max', c_int32),
]

sai_s32_range_t = struct__sai_s32_range_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 226

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 239
class struct__sai_vlan_list_t(Structure):
    pass

struct__sai_vlan_list_t.__slots__ = [
    'vlan_count',
    'vlan_list',
]
struct__sai_vlan_list_t._fields_ = [
    ('vlan_count', c_uint32),
    ('vlan_list', POINTER(sai_vlan_id_t)),
]

sai_vlan_list_t = struct__sai_vlan_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 239

enum__sai_vlan_tagging_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 252

SAI_VLAN_PORT_UNTAGGED = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 252

SAI_VLAN_PORT_TAGGED = (SAI_VLAN_PORT_UNTAGGED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 252

SAI_VLAN_PORT_PRIORITY_TAGGED = (SAI_VLAN_PORT_TAGGED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 252

sai_vlan_tagging_mode_t = enum__sai_vlan_tagging_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 252

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 263
class struct__sai_vlan_port_t(Structure):
    pass

struct__sai_vlan_port_t.__slots__ = [
    'port_id',
    'tagging_mode',
]
struct__sai_vlan_port_t._fields_ = [
    ('port_id', sai_object_id_t),
    ('tagging_mode', sai_vlan_tagging_mode_t),
]

sai_vlan_port_t = struct__sai_vlan_port_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 263

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 276
class struct__sai_vlan_port_list_t(Structure):
    pass

struct__sai_vlan_port_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_vlan_port_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(sai_vlan_port_t)),
]

sai_vlan_port_list_t = struct__sai_vlan_port_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 276

enum__sai_ip_addr_family_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 282

SAI_IP_ADDR_FAMILY_IPV4 = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 282

SAI_IP_ADDR_FAMILY_IPV6 = (SAI_IP_ADDR_FAMILY_IPV4 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 282

sai_ip_addr_family_t = enum__sai_ip_addr_family_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 282

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 286
class union_anon_15(Union):
    pass

union_anon_15.__slots__ = [
    'ip4',
    'ip6',
]
union_anon_15._fields_ = [
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
]

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 290
class struct__sai_ip_address_t(Structure):
    pass

struct__sai_ip_address_t.__slots__ = [
    'addr_family',
    'addr',
]
struct__sai_ip_address_t._fields_ = [
    ('addr_family', sai_ip_addr_family_t),
    ('addr', union_anon_15),
]

sai_ip_address_t = struct__sai_ip_address_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 290

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 294
class union_anon_16(Union):
    pass

union_anon_16.__slots__ = [
    'ip4',
    'ip6',
]
union_anon_16._fields_ = [
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
]

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 298
class union_anon_17(Union):
    pass

union_anon_17.__slots__ = [
    'ip4',
    'ip6',
]
union_anon_17._fields_ = [
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
]

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 302
class struct__sai_ip_prefix_t(Structure):
    pass

struct__sai_ip_prefix_t.__slots__ = [
    'addr_family',
    'addr',
    'mask',
]
struct__sai_ip_prefix_t._fields_ = [
    ('addr_family', sai_ip_addr_family_t),
    ('addr', union_anon_16),
    ('mask', union_anon_17),
]

sai_ip_prefix_t = struct__sai_ip_prefix_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 302

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 319
class union_anon_18(Union):
    pass

union_anon_18.__slots__ = [
    'u8',
    's8',
    'u16',
    's16',
    'u32',
    's32',
    'mac',
    'ip4',
    'ip6',
]
union_anon_18._fields_ = [
    ('u8', sai_uint8_t),
    ('s8', sai_int8_t),
    ('u16', sai_uint16_t),
    ('s16', sai_int16_t),
    ('u32', sai_uint32_t),
    ('s32', sai_int32_t),
    ('mac', sai_mac_t),
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
]

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 334
class union_anon_19(Union):
    pass

union_anon_19.__slots__ = [
    'u8',
    's8',
    'u16',
    's16',
    'u32',
    's32',
    'mac',
    'ip4',
    'ip6',
    'oid',
    'objlist',
]
union_anon_19._fields_ = [
    ('u8', sai_uint8_t),
    ('s8', sai_int8_t),
    ('u16', sai_uint16_t),
    ('s16', sai_int16_t),
    ('u32', sai_uint32_t),
    ('s32', sai_int32_t),
    ('mac', sai_mac_t),
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
    ('oid', sai_object_id_t),
    ('objlist', sai_object_list_t),
]

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 347
class struct__sai_acl_field_data_t(Structure):
    pass

struct__sai_acl_field_data_t.__slots__ = [
    'enable',
    'mask',
    'data',
]
struct__sai_acl_field_data_t._fields_ = [
    ('enable', bool),
    ('mask', union_anon_18),
    ('data', union_anon_19),
]

sai_acl_field_data_t = struct__sai_acl_field_data_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 347

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 363
class union_anon_20(Union):
    pass

union_anon_20.__slots__ = [
    'u8',
    's8',
    'u16',
    's16',
    'u32',
    's32',
    'mac',
    'ip4',
    'ip6',
    'oid',
    'objlist',
]
union_anon_20._fields_ = [
    ('u8', sai_uint8_t),
    ('s8', sai_int8_t),
    ('u16', sai_uint16_t),
    ('s16', sai_int16_t),
    ('u32', sai_uint32_t),
    ('s32', sai_int32_t),
    ('mac', sai_mac_t),
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
    ('oid', sai_object_id_t),
    ('objlist', sai_object_list_t),
]

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 376
class struct__sai_acl_action_data_t(Structure):
    pass

struct__sai_acl_action_data_t.__slots__ = [
    'enable',
    'parameter',
]
struct__sai_acl_action_data_t._fields_ = [
    ('enable', bool),
    ('parameter', union_anon_20),
]

sai_acl_action_data_t = struct__sai_acl_action_data_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 376

enum__sai_port_breakout_mode_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 395

SAI_PORT_BREAKOUT_MODE_1_LANE = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 395

SAI_PORT_BREAKOUT_MODE_2_LANE = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 395

SAI_PORT_BREAKOUT_MODE_4_LANE = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 395

SAI_PORT_BREAKOUT_MODE_MAX = (SAI_PORT_BREAKOUT_MODE_4_LANE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 395

sai_port_breakout_mode_type_t = enum__sai_port_breakout_mode_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 395

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 412
class struct__sai_port_breakout_t(Structure):
    pass

struct__sai_port_breakout_t.__slots__ = [
    'breakout_mode',
    'port_list',
]
struct__sai_port_breakout_t._fields_ = [
    ('breakout_mode', sai_port_breakout_mode_type_t),
    ('port_list', sai_object_list_t),
]

sai_port_breakout_t = struct__sai_port_breakout_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 412

enum__sai_packet_color_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 422

SAI_PACKET_COLOR_GREEN = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 422

SAI_PACKET_COLOR_YELLOW = (SAI_PACKET_COLOR_GREEN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 422

SAI_PACKET_COLOR_RED = (SAI_PACKET_COLOR_YELLOW + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 422

sai_packet_color_t = enum__sai_packet_color_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 422

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 459
class struct__sai_qos_map_params_t(Structure):
    pass

struct__sai_qos_map_params_t.__slots__ = [
    'tc',
    'dscp',
    'dot1p',
    'prio',
    'pg',
    'queue_index',
    'color',
]
struct__sai_qos_map_params_t._fields_ = [
    ('tc', sai_cos_t),
    ('dscp', sai_uint8_t),
    ('dot1p', sai_uint8_t),
    ('prio', sai_uint8_t),
    ('pg', sai_uint8_t),
    ('queue_index', sai_queue_index_t),
    ('color', sai_packet_color_t),
]

sai_qos_map_params_t = struct__sai_qos_map_params_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 459

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 469
class struct__sai_qos_map_t(Structure):
    pass

struct__sai_qos_map_t.__slots__ = [
    'key',
    'value',
]
struct__sai_qos_map_t._fields_ = [
    ('key', sai_qos_map_params_t),
    ('value', sai_qos_map_params_t),
]

sai_qos_map_t = struct__sai_qos_map_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 469

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 477
class struct__sai_qos_map_list_t(Structure):
    pass

struct__sai_qos_map_list_t.__slots__ = [
    'count',
    'list',
]
struct__sai_qos_map_list_t._fields_ = [
    ('count', c_uint32),
    ('list', POINTER(sai_qos_map_t)),
]

sai_qos_map_list_t = struct__sai_qos_map_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 477

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 515
class union_anon_21(Union):
    pass

union_anon_21.__slots__ = [
    'booldata',
    'chardata',
    'u8',
    's8',
    'u16',
    's16',
    'u32',
    's32',
    'u64',
    's64',
    'mac',
    'ip4',
    'ip6',
    'ipaddr',
    'oid',
    'objlist',
    'u8list',
    's8list',
    'u16list',
    's16list',
    'u32list',
    's32list',
    'u32range',
    's32range',
    'vlanlist',
    'vlanportlist',
    'aclfield',
    'aclaction',
    'portbreakout',
    'qosmap',
]
union_anon_21._fields_ = [
    ('booldata', bool),
    ('chardata', c_char * 32),
    ('u8', sai_uint8_t),
    ('s8', sai_int8_t),
    ('u16', sai_uint16_t),
    ('s16', sai_int16_t),
    ('u32', sai_uint32_t),
    ('s32', sai_int32_t),
    ('u64', sai_uint64_t),
    ('s64', sai_int64_t),
    ('mac', sai_mac_t),
    ('ip4', sai_ip4_t),
    ('ip6', sai_ip6_t),
    ('ipaddr', sai_ip_address_t),
    ('oid', sai_object_id_t),
    ('objlist', sai_object_list_t),
    ('u8list', sai_u8_list_t),
    ('s8list', sai_s8_list_t),
    ('u16list', sai_u16_list_t),
    ('s16list', sai_s16_list_t),
    ('u32list', sai_u32_list_t),
    ('s32list', sai_s32_list_t),
    ('u32range', sai_u32_range_t),
    ('s32range', sai_s32_range_t),
    ('vlanlist', sai_vlan_list_t),
    ('vlanportlist', sai_vlan_port_list_t),
    ('aclfield', sai_acl_field_data_t),
    ('aclaction', sai_acl_action_data_t),
    ('portbreakout', sai_port_breakout_t),
    ('qosmap', sai_qos_map_list_t),
]

sai_attribute_value_t = union_anon_21 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 515

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 520
class struct__sai_attribute_t(Structure):
    pass

struct__sai_attribute_t.__slots__ = [
    'id',
    'value',
]
struct__sai_attribute_t._fields_ = [
    ('id', sai_attr_id_t),
    ('value', sai_attribute_value_t),
]

sai_attribute_t = struct__sai_attribute_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 520

enum__sai_mirror_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 53

SAI_MIRROR_TYPE_LOCAL = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 53

SAI_MIRROR_TYPE_REMOTE = (SAI_MIRROR_TYPE_LOCAL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 53

SAI_MIRROR_TYPE_ENHANCED_REMOTE = (SAI_MIRROR_TYPE_REMOTE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 53

sai_mirror_type_t = enum__sai_mirror_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 53

enum__sai_erspan_encapsulation_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 65

SAI_MIRROR_L3_GRE_TUNNEL = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 65

sai_erspan_encapsulation_type_t = enum__sai_erspan_encapsulation_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 65

enum__sai_mirror_session_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_MONITOR_PORT = (SAI_MIRROR_SESSION_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_TC = (SAI_MIRROR_SESSION_ATTR_MONITOR_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_VLAN_TPID = (SAI_MIRROR_SESSION_ATTR_TC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_VLAN_ID = (SAI_MIRROR_SESSION_ATTR_VLAN_TPID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_VLAN_PRI = (SAI_MIRROR_SESSION_ATTR_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE = (SAI_MIRROR_SESSION_ATTR_VLAN_PRI + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION = (SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_TOS = (SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_TTL = (SAI_MIRROR_SESSION_ATTR_TOS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS = (SAI_MIRROR_SESSION_ATTR_TTL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS = (SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS = (SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS = (SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE = (SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

sai_mirror_session_attr_t = enum__sai_mirror_session_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 140

sai_create_mirror_session_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 151

sai_remove_mirror_session_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 164

sai_set_mirror_session_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 175

sai_get_mirror_session_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 188

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 203
class struct__sai_mirror_api_t(Structure):
    pass

struct__sai_mirror_api_t.__slots__ = [
    'create_mirror_session',
    'remove_mirror_session',
    'set_mirror_session_attribute',
    'get_mirror_session_attribute',
]
struct__sai_mirror_api_t._fields_ = [
    ('create_mirror_session', sai_create_mirror_session_fn),
    ('remove_mirror_session', sai_remove_mirror_session_fn),
    ('set_mirror_session_attribute', sai_set_mirror_session_attribute_fn),
    ('get_mirror_session_attribute', sai_get_mirror_session_attribute_fn),
]

sai_mirror_api_t = struct__sai_mirror_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 203

enum__sai_neighbor_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

SAI_NEIGHBOR_ATTR_PACKET_ACTION = (SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

SAI_NEIGHBOR_ATTR_NO_HOST_ROUTE = (SAI_NEIGHBOR_ATTR_PACKET_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

SAI_NEIGHBOR_ATTR_META_DATA = (SAI_NEIGHBOR_ATTR_NO_HOST_ROUTE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

SAI_NEIGHBOR_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

sai_neighbor_attr_t = enum__sai_neighbor_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 69

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 79
class struct__sai_neighbor_entry_t(Structure):
    pass

struct__sai_neighbor_entry_t.__slots__ = [
    'rif_id',
    'ip_address',
]
struct__sai_neighbor_entry_t._fields_ = [
    ('rif_id', sai_object_id_t),
    ('ip_address', sai_ip_address_t),
]

sai_neighbor_entry_t = struct__sai_neighbor_entry_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 79

sai_create_neighbor_entry_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_neighbor_entry_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 97

sai_remove_neighbor_entry_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_neighbor_entry_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 116

sai_set_neighbor_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_neighbor_entry_t), POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 132

sai_get_neighbor_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_neighbor_entry_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 150

sai_remove_all_neighbor_entries_fn = CFUNCTYPE(UNCHECKED(sai_status_t), ) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 167

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 180
class struct__sai_neighbor_api_t(Structure):
    pass

struct__sai_neighbor_api_t.__slots__ = [
    'create_neighbor_entry',
    'remove_neighbor_entry',
    'set_neighbor_attribute',
    'get_neighbor_attribute',
    'remove_all_neighbor_entries',
]
struct__sai_neighbor_api_t._fields_ = [
    ('create_neighbor_entry', sai_create_neighbor_entry_fn),
    ('remove_neighbor_entry', sai_remove_neighbor_entry_fn),
    ('set_neighbor_attribute', sai_set_neighbor_attribute_fn),
    ('get_neighbor_attribute', sai_get_neighbor_attribute_fn),
    ('remove_all_neighbor_entries', sai_remove_all_neighbor_entries_fn),
]

sai_neighbor_api_t = struct__sai_neighbor_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 180

enum__sai_next_hop_group_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 50

SAI_NEXT_HOP_GROUP_ECMP = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 50

sai_next_hop_group_type_t = enum__sai_next_hop_group_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 50

enum__sai_next_hop_group_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 76

SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 76

SAI_NEXT_HOP_GROUP_ATTR_TYPE = (SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 76

SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST = (SAI_NEXT_HOP_GROUP_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 76

SAI_NEXT_HOP_GROUP_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 76

sai_next_hop_group_attr_t = enum__sai_next_hop_group_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 76

sai_create_next_hop_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 91

sai_remove_next_hop_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 108

sai_set_next_hop_group_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 124

sai_get_next_hop_group_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 143

sai_add_next_hop_to_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_object_id_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 162

sai_remove_next_hop_from_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_object_id_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 182

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 201
class struct__sai_next_hop_group_api_t(Structure):
    pass

struct__sai_next_hop_group_api_t.__slots__ = [
    'create_next_hop_group',
    'remove_next_hop_group',
    'set_next_hop_group_attribute',
    'get_next_hop_group_attribute',
    'add_next_hop_to_group',
    'remove_next_hop_from_group',
]
struct__sai_next_hop_group_api_t._fields_ = [
    ('create_next_hop_group', sai_create_next_hop_group_fn),
    ('remove_next_hop_group', sai_remove_next_hop_group_fn),
    ('set_next_hop_group_attribute', sai_set_next_hop_group_attribute_fn),
    ('get_next_hop_group_attribute', sai_get_next_hop_group_attribute_fn),
    ('add_next_hop_to_group', sai_add_next_hop_to_group_fn),
    ('remove_next_hop_from_group', sai_remove_next_hop_from_group_fn),
]

sai_next_hop_group_api_t = struct__sai_next_hop_group_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 201

enum__sai_next_hop_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 54

SAI_NEXT_HOP_IP = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 54

SAI_NEXT_HOP_PORT = (SAI_NEXT_HOP_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 54

sai_next_hop_type_t = enum__sai_next_hop_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 54

enum__sai_next_hop_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

SAI_NEXT_HOP_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

SAI_NEXT_HOP_ATTR_IP = (SAI_NEXT_HOP_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID = (SAI_NEXT_HOP_ATTR_IP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

SAI_NEXT_HOP_ATTR_PORT = (SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

SAI_NEXT_HOP_ATTR_VLAN = (SAI_NEXT_HOP_ATTR_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

SAI_NEXT_HOP_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

sai_next_hop_attr_t = enum__sai_next_hop_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 89

sai_create_next_hop_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 106

sai_remove_next_hop_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 123

sai_set_next_hop_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 139

sai_get_next_hop_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 158

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 174
class struct__sai_next_hop_api_t(Structure):
    pass

struct__sai_next_hop_api_t.__slots__ = [
    'create_next_hop',
    'remove_next_hop',
    'set_next_hop_attribute',
    'get_next_hop_attribute',
]
struct__sai_next_hop_api_t._fields_ = [
    ('create_next_hop', sai_create_next_hop_fn),
    ('remove_next_hop', sai_remove_next_hop_fn),
    ('set_next_hop_attribute', sai_set_next_hop_attribute_fn),
    ('get_next_hop_attribute', sai_get_next_hop_attribute_fn),
]

sai_next_hop_api_t = struct__sai_next_hop_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 174

enum__sai_meter_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 46

SAI_METER_TYPE_PACKETS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 46

SAI_METER_TYPE_BYTES = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 46

SAI_METER_TYPE_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 46

sai_meter_type_t = enum__sai_meter_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 46

enum__sai_policer_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 64

SAI_POLICER_MODE_Sr_TCM = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 64

SAI_POLICER_MODE_Tr_TCM = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 64

SAI_POLICER_MODE_STORM_CONTROL = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 64

SAI_POLICER_MODE_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 64

sai_policer_mode_t = enum__sai_policer_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 64

enum__sai_policer_color_source_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 79

SAI_POLICER_COLOR_SOURCE_BLIND = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 79

SAI_POLICER_COLOR_SOURCE_AWARE = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 79

SAI_POLICER_COLOR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 79

sai_policer_color_source_t = enum__sai_policer_color_source_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 79

enum__sai_policer_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_METER_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_MODE = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_COLOR_SOURCE = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_CBS = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_CIR = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_PBS = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_PIR = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_GREEN_PACKET_ACTION = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_YELLOW_PACKET_ACTION = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_RED_PACKET_ACTION = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_ENABLE_COUNTER_LIST = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

SAI_POLICER_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

sai_policer_attr_t = enum__sai_policer_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 141

enum__sai_policer_stat_counter_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_PACKETS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_ATTR_BYTES = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_GREEN_PACKETS = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_GREEN_BYTES = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_YELLOW_PACKETS = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_YELLOW_BYTES = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_RED_PACKETS = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_RED_BYTES = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

SAI_POLICER_STAT_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

sai_policer_stat_counter_t = enum__sai_policer_stat_counter_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 176

sai_create_policer_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 192

sai_remove_policer_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 206

sai_set_policer_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 220

sai_get_policer_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 237

sai_get_policer_stats_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_policer_stat_counter_t), c_uint32, POINTER(c_uint64)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 255

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 272
class struct__sai_policer_api_t(Structure):
    pass

struct__sai_policer_api_t.__slots__ = [
    'create_policer',
    'remove_policer',
    'set_policer_attribute',
    'get_policer_attribute',
    'get_policer_statistics',
]
struct__sai_policer_api_t._fields_ = [
    ('create_policer', sai_create_policer_fn),
    ('remove_policer', sai_remove_policer_fn),
    ('set_policer_attribute', sai_set_policer_attribute_fn),
    ('get_policer_attribute', sai_get_policer_attribute_fn),
    ('get_policer_statistics', sai_get_policer_stats_fn),
]

sai_policer_api_t = struct__sai_policer_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 272

enum__sai_port_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 51

SAI_PORT_TYPE_LOGICAL = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 51

SAI_PORT_TYPE_CPU = (SAI_PORT_TYPE_LOGICAL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 51

sai_port_type_t = enum__sai_port_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 51

enum__sai_port_oper_status_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

SAI_PORT_OPER_STATUS_UNKNOWN = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

SAI_PORT_OPER_STATUS_UP = (SAI_PORT_OPER_STATUS_UNKNOWN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

SAI_PORT_OPER_STATUS_DOWN = (SAI_PORT_OPER_STATUS_UP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

SAI_PORT_OPER_STATUS_TESTING = (SAI_PORT_OPER_STATUS_DOWN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

SAI_PORT_OPER_STATUS_NOT_PRESENT = (SAI_PORT_OPER_STATUS_TESTING + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

sai_port_oper_status_t = enum__sai_port_oper_status_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 73

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 86
class struct__sai_port_oper_status_notification_t(Structure):
    pass

struct__sai_port_oper_status_notification_t.__slots__ = [
    'port_id',
    'port_state',
]
struct__sai_port_oper_status_notification_t._fields_ = [
    ('port_id', sai_object_id_t),
    ('port_state', sai_port_oper_status_t),
]

sai_port_oper_status_notification_t = struct__sai_port_oper_status_notification_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 86

enum__sai_port_flow_control_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 105

SAI_PORT_FLOW_CONTROL_DISABLE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 105

SAI_PORT_FLOW_CONTROL_TX_ONLY = (SAI_PORT_FLOW_CONTROL_DISABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 105

SAI_PORT_FLOW_CONTROL_RX_ONLY = (SAI_PORT_FLOW_CONTROL_TX_ONLY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 105

SAI_PORT_FLOW_CONTROL_BOTH_ENABLE = (SAI_PORT_FLOW_CONTROL_RX_ONLY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 105

sai_port_flow_control_mode_t = enum__sai_port_flow_control_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 105

enum__sai_port_internal_loopback_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 121

SAI_PORT_INTERNAL_LOOPBACK_NONE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 121

SAI_PORT_INTERNAL_LOOPBACK_PHY = (SAI_PORT_INTERNAL_LOOPBACK_NONE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 121

SAI_PORT_INTERNAL_LOOPBACK_MAC = (SAI_PORT_INTERNAL_LOOPBACK_PHY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 121

sai_port_internal_loopback_mode_t = enum__sai_port_internal_loopback_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 121

enum__sai_port_fdb_learning_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

SAI_PORT_LEARN_MODE_DROP = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

SAI_PORT_LEARN_MODE_DISABLE = (SAI_PORT_LEARN_MODE_DROP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

SAI_PORT_LEARN_MODE_HW = (SAI_PORT_LEARN_MODE_DISABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

SAI_PORT_LEARN_MODE_CPU_TRAP = (SAI_PORT_LEARN_MODE_HW + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

SAI_PORT_LEARN_MODE_CPU_LOG = (SAI_PORT_LEARN_MODE_CPU_TRAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

sai_port_fdb_learning_mode_t = enum__sai_port_fdb_learning_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 143

enum__sai_port_event_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 156

SAI_PORT_EVENT_ADD = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 156

SAI_PORT_EVENT_DELETE = (SAI_PORT_EVENT_ADD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 156

sai_port_event_t = enum__sai_port_event_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 156

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 169
class struct__sai_port_event_notification_t(Structure):
    pass

struct__sai_port_event_notification_t.__slots__ = [
    'port_id',
    'port_event',
]
struct__sai_port_event_notification_t._fields_ = [
    ('port_id', sai_object_id_t),
    ('port_event', sai_port_event_t),
]

sai_port_event_notification_t = struct__sai_port_event_notification_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 169

enum__sai_port_media_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

SAI_PORT_MEDIA_TYPE_NOT_PRESENT = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

SAI_PORT_MEDIA_TYPE_UNKNONWN = (SAI_PORT_MEDIA_TYPE_NOT_PRESENT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

SAI_PORT_MEDIA_TYPE_QSFP_FIBER = (SAI_PORT_MEDIA_TYPE_UNKNONWN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

SAI_PORT_MEDIA_TYPE_QSFP_COPPER = (SAI_PORT_MEDIA_TYPE_QSFP_FIBER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

SAI_PORT_MEDIA_TYPE_SFP_FIBER = (SAI_PORT_MEDIA_TYPE_QSFP_COPPER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

SAI_PORT_MEDIA_TYPE_SFP_COPPER = (SAI_PORT_MEDIA_TYPE_SFP_FIBER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

sai_port_media_type_t = enum__sai_port_media_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 193

enum__sai_port_isolation_direction_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 210

SAI_PORT_ISOLATION_DIRECTION_BOTH = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 210

SAI_PORT_ISOLATION_DIRECTION_INGRESS = (SAI_PORT_ISOLATION_DIRECTION_BOTH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 210

SAI_PORT_ISOLATION_DIRECTION_EGRESS = (SAI_PORT_ISOLATION_DIRECTION_INGRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 210

sai_port_isolation_direction_t = enum__sai_port_isolation_direction_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 210

enum__sai_port_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_OPER_STATUS = (SAI_PORT_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_HW_LANE_LIST = (SAI_PORT_ATTR_OPER_STATUS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE = (SAI_PORT_ATTR_HW_LANE_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE = (SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES = (SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_QUEUE_LIST = (SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS = (SAI_PORT_ATTR_QOS_QUEUE_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST = (SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_SUPPORTED_SPEED = (SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS = (SAI_PORT_ATTR_SUPPORTED_SPEED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_PRIORITY_GROUP_LIST = (SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_SPEED = (SAI_PORT_ATTR_PRIORITY_GROUP_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_FULL_DUPLEX_MODE = (SAI_PORT_ATTR_SPEED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_AUTO_NEG_MODE = (SAI_PORT_ATTR_FULL_DUPLEX_MODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_ADMIN_STATE = (SAI_PORT_ATTR_AUTO_NEG_MODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_MEDIA_TYPE = (SAI_PORT_ATTR_ADMIN_STATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_PORT_VLAN_ID = (SAI_PORT_ATTR_MEDIA_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY = (SAI_PORT_ATTR_PORT_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_INGRESS_FILTERING = (SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_DROP_UNTAGGED = (SAI_PORT_ATTR_INGRESS_FILTERING + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_DROP_TAGGED = (SAI_PORT_ATTR_DROP_UNTAGGED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_INTERNAL_LOOPBACK = (SAI_PORT_ATTR_DROP_TAGGED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_FDB_LEARNING = (SAI_PORT_ATTR_INTERNAL_LOOPBACK + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_UPDATE_DSCP = (SAI_PORT_ATTR_FDB_LEARNING + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_MTU = (SAI_PORT_ATTR_UPDATE_DSCP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID = (SAI_PORT_ATTR_MTU + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID = (SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID = (SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL = (SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES = (SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION = (SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_INGRESS_MIRROR_SESSION = (SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_EGRESS_MIRROR_SESSION = (SAI_PORT_ATTR_INGRESS_MIRROR_SESSION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE = (SAI_PORT_ATTR_EGRESS_MIRROR_SESSION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE = (SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_POLICER_ID = (SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_EGRESS_POLICER_ID = (SAI_PORT_ATTR_POLICER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DEFAULT_TC = (SAI_PORT_ATTR_EGRESS_POLICER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP = (SAI_PORT_ATTR_QOS_DEFAULT_TC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP = (SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP = (SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP = (SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP = (SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP = (SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP = (SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP = (SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP = (SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP = (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP = (SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP = (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_PRIORITY_GROUP_TO_PFC_PRIORITY_MAP = (SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP = (SAI_PORT_ATTR_QOS_PRIORITY_GROUP_TO_PFC_PRIORITY_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_WRED_PROFILE_ID = (SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID = (SAI_PORT_ATTR_QOS_WRED_PROFILE_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_QOS_BUFFER_PROFILE_ID = (SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL = (SAI_PORT_ATTR_QOS_BUFFER_PROFILE_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_META_DATA = (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST = (SAI_PORT_ATTR_META_DATA + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_ISOLATION_GROUP = (SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_ISOLATION_DIRECTION = (SAI_PORT_ATTR_ISOLATION_GROUP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

SAI_PORT_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

sai_port_attr_t = enum__sai_port_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 487

enum__sai_port_stat_counter_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_OCTETS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_UCAST_PKTS = (SAI_PORT_STAT_IF_IN_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS = (SAI_PORT_STAT_IF_IN_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_DISCARDS = (SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_ERRORS = (SAI_PORT_STAT_IF_IN_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS = (SAI_PORT_STAT_IF_IN_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_BROADCAST_PKTS = (SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_MULTICAST_PKTS = (SAI_PORT_STAT_IF_IN_BROADCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_VLAN_DISCARDS = (SAI_PORT_STAT_IF_IN_MULTICAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_OCTETS = (SAI_PORT_STAT_IF_IN_VLAN_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_UCAST_PKTS = (SAI_PORT_STAT_IF_OUT_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS = (SAI_PORT_STAT_IF_OUT_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_DISCARDS = (SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_ERRORS = (SAI_PORT_STAT_IF_OUT_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_QLEN = (SAI_PORT_STAT_IF_OUT_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS = (SAI_PORT_STAT_IF_OUT_QLEN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS = (SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS = (SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS = (SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS = (SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS = (SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_FRAGMENTS = (SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS = (SAI_PORT_STAT_ETHER_STATS_FRAGMENTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS = (SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS = (SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS = (SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS = (SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_JABBERS = (SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_OCTETS = (SAI_PORT_STAT_ETHER_STATS_JABBERS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_PKTS = (SAI_PORT_STAT_ETHER_STATS_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_COLLISIONS = (SAI_PORT_STAT_ETHER_STATS_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS = (SAI_PORT_STAT_ETHER_STATS_COLLISIONS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS = (SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS = (SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_IN_RECEIVES = (SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_IN_OCTETS = (SAI_PORT_STAT_IP_IN_RECEIVES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_IN_UCAST_PKTS = (SAI_PORT_STAT_IP_IN_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS = (SAI_PORT_STAT_IP_IN_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_IN_DISCARDS = (SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_OUT_OCTETS = (SAI_PORT_STAT_IP_IN_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_OUT_UCAST_PKTS = (SAI_PORT_STAT_IP_OUT_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS = (SAI_PORT_STAT_IP_OUT_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IP_OUT_DISCARDS = (SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_IN_RECEIVES = (SAI_PORT_STAT_IP_OUT_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_IN_OCTETS = (SAI_PORT_STAT_IPV6_IN_RECEIVES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_IN_UCAST_PKTS = (SAI_PORT_STAT_IPV6_IN_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS = (SAI_PORT_STAT_IPV6_IN_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_IN_MCAST_PKTS = (SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_IN_DISCARDS = (SAI_PORT_STAT_IPV6_IN_MCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_OUT_OCTETS = (SAI_PORT_STAT_IPV6_IN_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS = (SAI_PORT_STAT_IPV6_OUT_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS = (SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS = (SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IPV6_OUT_DISCARDS = (SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_GREEN_DISCARD_DROPPED_PACKETS = (SAI_PORT_STAT_IPV6_OUT_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_GREEN_DISCARD_DROPPED_BYTES = (SAI_PORT_STAT_GREEN_DISCARD_DROPPED_PACKETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_PACKETS = (SAI_PORT_STAT_GREEN_DISCARD_DROPPED_BYTES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_BYTES = (SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_PACKETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_RED_DISCARD_DROPPED_PACKETS = (SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_BYTES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_RED_DISCARD_DROPPED_BYTES = (SAI_PORT_STAT_RED_DISCARD_DROPPED_PACKETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_DISCARD_DROPPED_PACKETS = (SAI_PORT_STAT_RED_DISCARD_DROPPED_BYTES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_DISCARD_DROPPED_BYTES = (SAI_PORT_STAT_DISCARD_DROPPED_PACKETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS = (SAI_PORT_STAT_DISCARD_DROPPED_BYTES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS = (SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS = (SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_PKTS = (SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_OAM_PKTS = (SAI_PORT_STAT_IF_IN_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_RUNTS_PKTS = (SAI_PORT_STAT_IF_IN_OAM_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_GIANTS_PKTS = (SAI_PORT_STAT_IF_IN_RUNTS_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS = (SAI_PORT_STAT_IF_IN_GIANTS_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS = (SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_FRAME_PKTS = (SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_PAUSE_PKTS = (SAI_PORT_STAT_IF_IN_FRAME_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_BAD_PKTS = (SAI_PORT_STAT_IF_IN_PAUSE_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_IN_BAD_OCTETS = (SAI_PORT_STAT_IF_IN_BAD_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_PKTS = (SAI_PORT_STAT_IF_IN_BAD_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_OAM_PKTS = (SAI_PORT_STAT_IF_OUT_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS = (SAI_PORT_STAT_IF_OUT_OAM_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

SAI_PORT_STAT_IF_OUT_PAUSE_PKTS = (SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

sai_port_stat_counter_t = enum__sai_port_stat_counter_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 618

sai_set_port_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 633

sai_get_port_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 652

sai_get_port_stats_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_port_stat_counter_t), c_uint32, POINTER(c_uint64)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 672

sai_clear_port_stats_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_port_stat_counter_t), c_uint32) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 692

sai_clear_port_all_stats_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 709

sai_port_state_change_notification_fn = CFUNCTYPE(UNCHECKED(None), c_uint32, POINTER(sai_port_oper_status_notification_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 725

sai_port_event_notification_fn = CFUNCTYPE(UNCHECKED(None), c_uint32, POINTER(sai_port_event_notification_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 741

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 756
class struct__sai_port_api_t(Structure):
    pass

struct__sai_port_api_t.__slots__ = [
    'set_port_attribute',
    'get_port_attribute',
    'get_port_stats',
    'clear_port_stats',
    'clear_port_all_stats',
]
struct__sai_port_api_t._fields_ = [
    ('set_port_attribute', sai_set_port_attribute_fn),
    ('get_port_attribute', sai_get_port_attribute_fn),
    ('get_port_stats', sai_get_port_stats_fn),
    ('clear_port_stats', sai_clear_port_stats_fn),
    ('clear_port_all_stats', sai_clear_port_all_stats_fn),
]

sai_port_api_t = struct__sai_port_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 756

enum__sai_qos_map_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_DOT1P_TO_TC = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_DOT1P_TO_COLOR = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_DSCP_TO_TC = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_DSCP_TO_COLOR = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_TC_TO_QUEUE = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_TC_TO_DSCP = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_TC_TO_DOT1P = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_TC_TO_PRIORITY_GROUP = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_PRIORITY_GROUP_TO_PFC_PRIORITY = 12 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_PFC_PRIORITY_TO_QUEUE = 13 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

SAI_QOS_MAP_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

sai_qos_map_type_t = enum__sai_qos_map_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 82

enum__sai_qos_map_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 106

SAI_QOS_MAP_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 106

SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 106

SAI_QOS_MAP_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 106

sai_qos_map_attr_t = enum__sai_qos_map_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 106

sai_create_qos_map_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 118

sai_remove_qos_map_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 132

sai_set_qos_map_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 146

sai_get_qos_map_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 163

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 179
class struct__sai_qos_map_api_t(Structure):
    pass

struct__sai_qos_map_api_t.__slots__ = [
    'create_qos_map',
    'remove_qos_map',
    'set_qos_map_attribute',
    'get_qos_map_attribute',
]
struct__sai_qos_map_api_t._fields_ = [
    ('create_qos_map', sai_create_qos_map_fn),
    ('remove_qos_map', sai_remove_qos_map_fn),
    ('set_qos_map_attribute', sai_set_qos_map_attribute_fn),
    ('get_qos_map_attribute', sai_get_qos_map_attribute_fn),
]

sai_qos_map_api_t = struct__sai_qos_map_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 179

enum__sai_queue_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 50

SAI_QUEUE_TYPE_ALL = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 50

SAI_QUEUE_TYPE_UNICAST = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 50

SAI_QUEUE_TYPE_MULTICAST = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 50

SAI_QUEUE_TYPE_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 50

sai_queue_type_t = enum__sai_queue_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 50

enum__sai_queue_drop_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 56

SAI_DROP_TYPE_TAIL = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 56

SAI_DROP_TYPE_WRED = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 56

sai_queue_drop_type_t = enum__sai_queue_drop_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 56

enum__sai_queue_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

SAI_QUEUE_ATTR_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

SAI_QUEUE_ATTR_WRED_PROFILE_ID = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

SAI_QUEUE_ATTR_BUFFER_PROFILE_ID = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

SAI_QUEUE_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

SAI_QUEUE_ATTR_DROP_TYPE = (SAI_QUEUE_ATTR_CUSTOM_RANGE_BASE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

sai_queue_attr_t = enum__sai_queue_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 86

enum__sai_queue_stat_counter_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_PACKETS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_BYTES = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_DROPPED_PACKETS = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_DROPPED_BYTES = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_GREEN_PACKETS = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_GREEN_BYTES = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_GREEN_DROPPED_BYTES = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_YELLOW_PACKETS = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_YELLOW_BYTES = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_RED_PACKETS = 12 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_RED_BYTES = 13 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_RED_DROPPED_PACKETS = 14 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_RED_DROPPED_BYTES = 15 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_PACKETS = 16 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_BYTES = 17 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_PACKETS = 18 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_BYTES = 19 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_RED_DISCARD_DROPPED_PACKETS = 20 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_RED_DISCARD_DROPPED_BYTES = 21 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_DISCARD_DROPPED_PACKETS = 22 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_DISCARD_DROPPED_BYTES = 23 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

SAI_QUEUE_STAT_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

sai_queue_stat_counter_t = enum__sai_queue_stat_counter_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 172

sai_set_queue_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 183

sai_get_queue_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 197

sai_get_queue_stats_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_queue_stat_counter_t), c_uint32, POINTER(c_uint64)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 215

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 231
class struct__sai_queue_api_t(Structure):
    pass

struct__sai_queue_api_t.__slots__ = [
    'set_queue_attribute',
    'get_queue_attribute',
    'get_queue_stats',
]
struct__sai_queue_api_t._fields_ = [
    ('set_queue_attribute', sai_set_queue_attribute_fn),
    ('get_queue_attribute', sai_get_queue_attribute_fn),
    ('get_queue_stats', sai_get_queue_stats_fn),
]

sai_queue_api_t = struct__sai_queue_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 231

enum__sai_route_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

SAI_ROUTE_ATTR_PACKET_ACTION = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

SAI_ROUTE_ATTR_TRAP_PRIORITY = (SAI_ROUTE_ATTR_PACKET_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

SAI_ROUTE_ATTR_NEXT_HOP_ID = (SAI_ROUTE_ATTR_TRAP_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

SAI_ROUTE_ATTR_META_DATA = (SAI_ROUTE_ATTR_NEXT_HOP_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

SAI_ROUTE_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

sai_route_attr_t = enum__sai_route_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 71

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 82
class struct__sai_unicast_route_entry_t(Structure):
    pass

struct__sai_unicast_route_entry_t.__slots__ = [
    'vr_id',
    'destination',
]
struct__sai_unicast_route_entry_t._fields_ = [
    ('vr_id', sai_object_id_t),
    ('destination', sai_ip_prefix_t),
]

sai_unicast_route_entry_t = struct__sai_unicast_route_entry_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 82

sai_create_route_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_unicast_route_entry_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 100

sai_remove_route_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_unicast_route_entry_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 119

sai_set_route_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_unicast_route_entry_t), POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 135

sai_get_route_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_unicast_route_entry_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 153

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 170
class struct__sai_route_api_t(Structure):
    pass

struct__sai_route_api_t.__slots__ = [
    'create_route',
    'remove_route',
    'set_route_attribute',
    'get_route_attribute',
]
struct__sai_route_api_t._fields_ = [
    ('create_route', sai_create_route_fn),
    ('remove_route', sai_remove_route_fn),
    ('set_route_attribute', sai_set_route_attribute_fn),
    ('get_route_attribute', sai_get_route_attribute_fn),
]

sai_route_api_t = struct__sai_route_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 170

enum__sai_virtual_router_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE = (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS = (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION = (SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS = (SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

SAI_VIRTUAL_ROUTER_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

sai_virtual_router_attr_t = enum__sai_virtual_router_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 73

sai_create_virtual_router_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 89

sai_remove_virtual_router_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 106

sai_set_virtual_router_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 122

sai_get_virtual_router_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 140

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 156
class struct__sai_virtual_router_api_t(Structure):
    pass

struct__sai_virtual_router_api_t.__slots__ = [
    'create_virtual_router',
    'remove_virtual_router',
    'set_virtual_router_attribute',
    'get_virtual_router_attribute',
]
struct__sai_virtual_router_api_t._fields_ = [
    ('create_virtual_router', sai_create_virtual_router_fn),
    ('remove_virtual_router', sai_remove_virtual_router_fn),
    ('set_virtual_router_attribute', sai_set_virtual_router_attribute_fn),
    ('get_virtual_router_attribute', sai_get_virtual_router_attribute_fn),
]

sai_virtual_router_api_t = struct__sai_virtual_router_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 156

enum__sai_router_interface_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 51

SAI_ROUTER_INTERFACE_TYPE_PORT = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 51

SAI_ROUTER_INTERFACE_TYPE_VLAN = (SAI_ROUTER_INTERFACE_TYPE_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 51

sai_router_interface_type_t = enum__sai_router_interface_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 51

enum__sai_router_interface_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_TYPE = (SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_PORT_ID = (SAI_ROUTER_INTERFACE_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_VLAN_ID = (SAI_ROUTER_INTERFACE_ATTR_PORT_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS = (SAI_ROUTER_INTERFACE_ATTR_VLAN_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE = (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE = (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_MTU = (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

SAI_ROUTER_INTERFACE_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

sai_router_interface_attr_t = enum__sai_router_interface_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 97

sai_create_router_interface_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 112

sai_remove_router_interface_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 129

sai_set_router_interface_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 145

sai_get_router_interface_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 164

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 180
class struct__sai_router_interface_api_t(Structure):
    pass

struct__sai_router_interface_api_t.__slots__ = [
    'create_router_interface',
    'remove_router_interface',
    'set_router_interface_attribute',
    'get_router_interface_attribute',
]
struct__sai_router_interface_api_t._fields_ = [
    ('create_router_interface', sai_create_router_interface_fn),
    ('remove_router_interface', sai_remove_router_interface_fn),
    ('set_router_interface_attribute', sai_set_router_interface_attribute_fn),
    ('get_router_interface_attribute', sai_get_router_interface_attribute_fn),
]

sai_router_interface_api_t = struct__sai_router_interface_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 180

enum__sai_samplepacket_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 47

SAI_SAMPLEPACKET_SLOW_PATH = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 47

sai_samplepacket_type_t = enum__sai_samplepacket_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 47

enum__sai_samplepacket_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 72

SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 72

SAI_SAMPLEPACKET_ATTR_TYPE = (SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 72

sai_samplepacket_attr_t = enum__sai_samplepacket_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 72

sai_create_samplepacket_session_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 83

sai_remove_samplepacket_session_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 96

sai_set_samplepacket_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 107

sai_get_samplepacket_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 120

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 134
class struct__sai_samplepacket_api_t(Structure):
    pass

struct__sai_samplepacket_api_t.__slots__ = [
    'create_samplepacket_session',
    'remove_samplepacket_session',
    'set_samplepacket_attribute',
    'get_samplepacket_attribute',
]
struct__sai_samplepacket_api_t._fields_ = [
    ('create_samplepacket_session', sai_create_samplepacket_session_fn),
    ('remove_samplepacket_session', sai_remove_samplepacket_session_fn),
    ('set_samplepacket_attribute', sai_set_samplepacket_attribute_fn),
    ('get_samplepacket_attribute', sai_get_samplepacket_attribute_fn),
]

sai_samplepacket_api_t = struct__sai_samplepacket_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 134

enum__sai_scheduler_group_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_MAX_SUPPORTED_CHILDS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_PORT_ID = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_LEVEL = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

SAI_SCHEDULER_GROUP_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

sai_scheduler_group_attr_t = enum__sai_scheduler_group_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 66

sai_create_scheduler_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 79

sai_remove_scheduler_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 93

sai_set_scheduler_group_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 107

sai_get_scheduler_group_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 123

sai_add_child_object_to_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_object_id_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 139

sai_remove_child_object_from_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_object_id_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 156

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 174
class struct__sai_scheduler_group_api_t(Structure):
    pass

struct__sai_scheduler_group_api_t.__slots__ = [
    'create_scheduler_group',
    'remove_scheduler_group',
    'set_scheduler_group_attribute',
    'get_scheduler_group_attribute',
    'add_child_object_to_group',
    'remove_child_object_from_group',
]
struct__sai_scheduler_group_api_t._fields_ = [
    ('create_scheduler_group', sai_create_scheduler_group_fn),
    ('remove_scheduler_group', sai_remove_scheduler_group_fn),
    ('set_scheduler_group_attribute', sai_set_scheduler_group_attribute_fn),
    ('get_scheduler_group_attribute', sai_get_scheduler_group_attribute_fn),
    ('add_child_object_to_group', sai_add_child_object_to_group_fn),
    ('remove_child_object_from_group', sai_remove_child_object_from_group_fn),
]

sai_scheduler_group_api_t = struct__sai_scheduler_group_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 174

enum__sai_scheduling_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 45

SAI_SCHEDULING_STRICT = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 45

SAI_SCHEDULING_WRR = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 45

SAI_SCHEDULING_DWRR = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 45

sai_scheduling_type_t = enum__sai_scheduling_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 45

enum__sai_scheduler_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_SHAPER_TYPE = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

SAI_SCHEDULER_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

sai_scheduler_attr_t = enum__sai_scheduler_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 84

sai_create_scheduler_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 97

sai_remove_scheduler_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 111

sai_set_scheduler_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 125

sai_get_scheduler_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 141

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 157
class struct__sai_scheduler_api_t(Structure):
    pass

struct__sai_scheduler_api_t.__slots__ = [
    'create_scheduler_profile',
    'remove_scheduler_profile',
    'set_scheduler_attribute',
    'get_scheduler_attribute',
]
struct__sai_scheduler_api_t._fields_ = [
    ('create_scheduler_profile', sai_create_scheduler_fn),
    ('remove_scheduler_profile', sai_remove_scheduler_fn),
    ('set_scheduler_attribute', sai_set_scheduler_attribute_fn),
    ('get_scheduler_attribute', sai_get_scheduler_attribute_fn),
]

sai_scheduler_api_t = struct__sai_scheduler_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 157

enum__sai_port_stp_port_state_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 53

SAI_PORT_STP_STATE_LEARNING = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 53

SAI_PORT_STP_STATE_FORWARDING = (SAI_PORT_STP_STATE_LEARNING + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 53

SAI_PORT_STP_STATE_BLOCKING = (SAI_PORT_STP_STATE_FORWARDING + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 53

sai_port_stp_port_state_t = enum__sai_port_stp_port_state_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 53

enum__sai_stp_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 67

SAI_STP_ATTR_VLAN_LIST = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 67

sai_stp_attr_t = enum__sai_stp_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 67

sai_create_stp_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 78

sai_remove_stp_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 91

sai_set_stp_port_state_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, sai_object_id_t, sai_port_stp_port_state_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 103

sai_get_stp_port_state_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, sai_object_id_t, POINTER(sai_port_stp_port_state_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 117

sai_set_stp_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 130

sai_get_stp_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 143

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 158
class struct__sai_stp_api_t(Structure):
    pass

struct__sai_stp_api_t.__slots__ = [
    'create_stp',
    'remove_stp',
    'set_stp_attribute',
    'get_stp_attribute',
    'set_stp_port_state',
    'get_stp_port_state',
]
struct__sai_stp_api_t._fields_ = [
    ('create_stp', sai_create_stp_fn),
    ('remove_stp', sai_remove_stp_fn),
    ('set_stp_attribute', sai_set_stp_attribute_fn),
    ('get_stp_attribute', sai_get_stp_attribute_fn),
    ('set_stp_port_state', sai_set_stp_port_state_fn),
    ('get_stp_port_state', sai_get_stp_port_state_fn),
]

sai_stp_api_t = struct__sai_stp_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 158

enum__sai_switch_oper_status_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 63

SAI_SWITCH_OPER_STATUS_UNKNOWN = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 63

SAI_SWITCH_OPER_STATUS_UP = (SAI_SWITCH_OPER_STATUS_UNKNOWN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 63

SAI_SWITCH_OPER_STATUS_DOWN = (SAI_SWITCH_OPER_STATUS_UP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 63

SAI_SWITCH_OPER_STATUS_FAILED = (SAI_SWITCH_OPER_STATUS_DOWN + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 63

sai_switch_oper_status_t = enum__sai_switch_oper_status_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 63

enum__sai_packet_action_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_DROP = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_FORWARD = (SAI_PACKET_ACTION_DROP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_COPY = (SAI_PACKET_ACTION_FORWARD + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_COPY_CANCEL = (SAI_PACKET_ACTION_COPY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_TRAP = (SAI_PACKET_ACTION_COPY_CANCEL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_LOG = (SAI_PACKET_ACTION_TRAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_DENY = (SAI_PACKET_ACTION_LOG + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

SAI_PACKET_ACTION_TRANSIT = (SAI_PACKET_ACTION_DENY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

sai_packet_action_t = enum__sai_packet_action_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 110

enum__sai_switch_switching_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 123

SAI_SWITCHING_MODE_CUT_THROUGH = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 123

SAI_SWITCHING_MODE_STORE_AND_FORWARD = (SAI_SWITCHING_MODE_CUT_THROUGH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 123

sai_switch_switching_mode_t = enum__sai_switch_switching_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 123

enum__sai_hash_algorithm_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 140

SAI_HASH_ALGORITHM_CRC = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 140

SAI_HASH_ALGORITHM_XOR = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 140

SAI_HASH_RANDOM = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 140

sai_hash_algorithm_t = enum__sai_hash_algorithm_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 140

enum__sai_isolation_mode_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 153

SAI_ISOLATION_MODE_L2 = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 153

SAI_ISOLATION_MODE_ALL = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 153

sai_isolation_mode_t = enum__sai_isolation_mode_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 153

enum__sai_switch_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_PORT_NUMBER = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_PORT_LIST = (SAI_SWITCH_ATTR_PORT_NUMBER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_PORT_MAX_MTU = (SAI_SWITCH_ATTR_PORT_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_CPU_PORT = (SAI_SWITCH_ATTR_PORT_MAX_MTU + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS = (SAI_SWITCH_ATTR_CPU_PORT + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_FDB_TABLE_SIZE = (SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED = (SAI_SWITCH_ATTR_FDB_TABLE_SIZE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_OPER_STATUS = (SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_MAX_TEMP = (SAI_SWITCH_ATTR_OPER_STATUS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY = (SAI_SWITCH_ATTR_MAX_TEMP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY = (SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY = (SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY = (SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_FDB_DST_USER_META_DATA_RANGE = (SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE = (SAI_SWITCH_ATTR_FDB_DST_USER_META_DATA_RANGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE = (SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE = (SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_VLAN_USER_META_DATA_RANGE = (SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ACL_USER_META_DATA_RANGE = (SAI_SWITCH_ATTR_VLAN_USER_META_DATA_RANGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID = (SAI_SWITCH_ATTR_ACL_USER_META_DATA_RANGE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID = (SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES = (SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS = (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL = (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_MAX_LAG_MEMBERS = (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_MAX_LAG_NUMBER = (SAI_SWITCH_ATTR_MAX_LAG_MEMBERS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE = (SAI_SWITCH_ATTR_MAX_LAG_NUMBER + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM = (SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM = (SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP = (SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ECMP_HASH = (SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_LAG_HASH = (SAI_SWITCH_ATTR_ECMP_HASH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_SWITCHING_MODE = (SAI_SWITCH_ATTR_LAG_HASH + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE = (SAI_SWITCH_ATTR_SWITCHING_MODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE = (SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_SRC_MAC_ADDRESS = (SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES = (SAI_SWITCH_ATTR_SRC_MAC_ADDRESS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_FDB_AGING_TIME = (SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION = (SAI_SWITCH_ATTR_FDB_AGING_TIME + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION = (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION = (SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM = (SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED = (SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ECMP_HASH_IPV4 = (SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4 = (SAI_SWITCH_ATTR_ECMP_HASH_IPV4 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM = (SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED = (SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_LAG_HASH_IPV4 = (SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4 = (SAI_SWITCH_ATTR_LAG_HASH_IPV4 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ECMP_MAX_PATHS = (SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL = (SAI_SWITCH_ATTR_ECMP_MAX_PATHS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DEFAULT_TC = (SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP = (SAI_SWITCH_ATTR_QOS_DEFAULT_TC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP = (SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP = (SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP = (SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP = (SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP = (SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP = (SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_TC_TO_DOT1P_MAP = (SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP = (SAI_SWITCH_ATTR_QOS_TC_TO_DOT1P_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_TC_TO_DSCP_MAP = (SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP = (SAI_SWITCH_ATTR_QOS_TC_TO_DSCP_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_ISOLATION_MODE = (SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_PORT_BREAKOUT = (SAI_SWITCH_ATTR_ISOLATION_MODE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

SAI_SWITCH_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

sai_switch_attr_t = enum__sai_switch_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 436

sai_switch_shutdown_request_fn = CFUNCTYPE(UNCHECKED(None), ) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 450

sai_switch_state_change_notification_fn = CFUNCTYPE(UNCHECKED(None), sai_switch_oper_status_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 465

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 477
for _lib in _libs.values():
    try:
        on_port_state_change = (sai_port_state_change_notification_fn).in_dll(_lib, 'on_port_state_change')
        break
    except:
        pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 478
for _lib in _libs.values():
    try:
        on_port_event = (sai_port_event_notification_fn).in_dll(_lib, 'on_port_event')
        break
    except:
        pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 479
for _lib in _libs.values():
    try:
        on_switch_shutdown_request = (sai_switch_shutdown_request_fn).in_dll(_lib, 'on_switch_shutdown_request')
        break
    except:
        pass

sai_shutdown_switch_fn = CFUNCTYPE(UNCHECKED(None), bool) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 519

sai_disconnect_switch_fn = CFUNCTYPE(UNCHECKED(None), ) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 552

sai_set_switch_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 568

sai_get_switch_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 585

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 596
for _lib in _libs.values():
    try:
        shutdown_switch = (sai_shutdown_switch_fn).in_dll(_lib, 'shutdown_switch')
        break
    except:
        pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 598
for _lib in _libs.values():
    try:
        disconnect_switch = (sai_disconnect_switch_fn).in_dll(_lib, 'disconnect_switch')
        break
    except:
        pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 599
for _lib in _libs.values():
    try:
        set_switch_attribute = (sai_set_switch_attribute_fn).in_dll(_lib, 'set_switch_attribute')
        break
    except:
        pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 600
for _lib in _libs.values():
    try:
        get_switch_attribute = (sai_get_switch_attribute_fn).in_dll(_lib, 'get_switch_attribute')
        break
    except:
        pass

enum__sai_udf_base_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 54

SAI_UDF_BASE_L2 = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 54

SAI_UDF_BASE_L3 = (SAI_UDF_BASE_L2 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 54

SAI_UDF_BASE_L4 = (SAI_UDF_BASE_L3 + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 54

sai_udf_base_t = enum__sai_udf_base_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 54

enum__sai_udf_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

SAI_UDF_ATTR_MATCH_ID = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

SAI_UDF_ATTR_GROUP_ID = (SAI_UDF_ATTR_MATCH_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

SAI_UDF_ATTR_BASE = (SAI_UDF_ATTR_GROUP_ID + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

SAI_UDF_ATTR_OFFSET = (SAI_UDF_ATTR_BASE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

SAI_UDF_ATTR_HASH_MASK = (SAI_UDF_ATTR_OFFSET + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

sai_udf_attr_t = enum__sai_udf_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 83

enum__sai_udf_match_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 106

SAI_UDF_MATCH_ATTR_L2_TYPE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 106

SAI_UDF_MATCH_ATTR_L3_TYPE = (SAI_UDF_MATCH_ATTR_L2_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 106

SAI_UDF_MATCH_ATTR_GRE_TYPE = (SAI_UDF_MATCH_ATTR_L3_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 106

SAI_UDF_MATCH_ATTR_PRIORITY = (SAI_UDF_MATCH_ATTR_GRE_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 106

sai_udf_match_attr_t = enum__sai_udf_match_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 106

enum__sai_udf_group_type_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 119

SAI_UDF_GROUP_GENERIC = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 119

SAI_UDF_GROUP_HASH = (SAI_UDF_GROUP_GENERIC + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 119

sai_udf_group_type_t = enum__sai_udf_group_type_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 119

enum__sai_udf_group_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 139

SAI_UDF_GROUP_ATTR_UDF_LIST = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 139

SAI_UDF_GROUP_ATTR_TYPE = (SAI_UDF_GROUP_ATTR_UDF_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 139

SAI_UDF_ATTR_LENGTH = (SAI_UDF_GROUP_ATTR_TYPE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 139

sai_udf_group_attr_t = enum__sai_udf_group_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 139

sai_create_udf_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 155

sai_remove_udf_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 172

sai_set_udf_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 188

sai_get_udf_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 206

sai_create_udf_match_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 226

sai_remove_udf_match_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 243

sai_set_udf_match_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 259

sai_get_udf_match_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 277

sai_create_udf_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 297

sai_remove_udf_group_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 314

sai_set_udf_group_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 330

sai_get_udf_group_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 348

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 372
class struct__sai_udf_api_t(Structure):
    pass

struct__sai_udf_api_t.__slots__ = [
    'create_udf',
    'remove_udf',
    'set_udf_attribute',
    'get_udf_attribute',
    'create_udf_match',
    'remove_udf_match',
    'set_udf_match_attribute',
    'get_udf_match_attribute',
    'create_udf_group',
    'remove_udf_group',
    'set_udf_group_attribute',
    'get_udf_group_attribute',
]
struct__sai_udf_api_t._fields_ = [
    ('create_udf', sai_create_udf_fn),
    ('remove_udf', sai_remove_udf_fn),
    ('set_udf_attribute', sai_set_udf_attribute_fn),
    ('get_udf_attribute', sai_get_udf_attribute_fn),
    ('create_udf_match', sai_create_udf_match_fn),
    ('remove_udf_match', sai_remove_udf_match_fn),
    ('set_udf_match_attribute', sai_set_udf_match_attribute_fn),
    ('get_udf_match_attribute', sai_get_udf_match_attribute_fn),
    ('create_udf_group', sai_create_udf_group_fn),
    ('remove_udf_group', sai_remove_udf_group_fn),
    ('set_udf_group_attribute', sai_set_udf_group_attribute_fn),
    ('get_udf_group_attribute', sai_get_udf_group_attribute_fn),
]

sai_udf_api_t = struct__sai_udf_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 372

enum__sai_vlan_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

SAI_VLAN_ATTR_PORT_LIST = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES = (SAI_VLAN_ATTR_PORT_LIST + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

SAI_VLAN_ATTR_STP_INSTANCE = (SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

SAI_VLAN_ATTR_LEARN_DISABLE = (SAI_VLAN_ATTR_STP_INSTANCE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

SAI_VLAN_ATTR_META_DATA = (SAI_VLAN_ATTR_LEARN_DISABLE + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

SAI_VLAN_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

sai_vlan_attr_t = enum__sai_vlan_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 80

enum__sai_vlan_stat_counter_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_IN_OCTETS = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_IN_UCAST_PKTS = (SAI_VLAN_STAT_IN_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_IN_NON_UCAST_PKTS = (SAI_VLAN_STAT_IN_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_IN_DISCARDS = (SAI_VLAN_STAT_IN_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_IN_ERRORS = (SAI_VLAN_STAT_IN_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_IN_UNKNOWN_PROTOS = (SAI_VLAN_STAT_IN_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_OUT_OCTETS = (SAI_VLAN_STAT_IN_UNKNOWN_PROTOS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_OUT_UCAST_PKTS = (SAI_VLAN_STAT_OUT_OCTETS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_OUT_NON_UCAST_PKTS = (SAI_VLAN_STAT_OUT_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_OUT_DISCARDS = (SAI_VLAN_STAT_OUT_NON_UCAST_PKTS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_OUT_ERRORS = (SAI_VLAN_STAT_OUT_DISCARDS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

SAI_VLAN_STAT_OUT_QLEN = (SAI_VLAN_STAT_OUT_ERRORS + 1) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

sai_vlan_stat_counter_t = enum__sai_vlan_stat_counter_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 101

sai_create_vlan_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 114

sai_remove_vlan_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 129

sai_set_vlan_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 145

sai_get_vlan_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 163

sai_remove_all_vlans_fn = CFUNCTYPE(UNCHECKED(sai_status_t), ) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 180

sai_add_ports_to_vlan_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t, c_uint32, POINTER(sai_vlan_port_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 197

sai_remove_ports_from_vlan_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t, c_uint32, POINTER(sai_vlan_port_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 216

sai_get_vlan_stats_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_vlan_id_t, POINTER(sai_vlan_stat_counter_t), c_uint32, POINTER(c_uint64)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 236

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 257
class struct__sai_vlan_api_t(Structure):
    pass

struct__sai_vlan_api_t.__slots__ = [
    'create_vlan',
    'remove_vlan',
    'set_vlan_attribute',
    'get_vlan_attribute',
    'add_ports_to_vlan',
    'remove_ports_from_vlan',
    'remove_all_vlans',
    'get_vlan_stats',
]
struct__sai_vlan_api_t._fields_ = [
    ('create_vlan', sai_create_vlan_fn),
    ('remove_vlan', sai_remove_vlan_fn),
    ('set_vlan_attribute', sai_set_vlan_attribute_fn),
    ('get_vlan_attribute', sai_get_vlan_attribute_fn),
    ('add_ports_to_vlan', sai_add_ports_to_vlan_fn),
    ('remove_ports_from_vlan', sai_remove_ports_from_vlan_fn),
    ('remove_all_vlans', sai_remove_all_vlans_fn),
    ('get_vlan_stats', sai_get_vlan_stats_fn),
]

sai_vlan_api_t = struct__sai_vlan_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 257

enum__sai_wred_attr_t = c_int # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_GREEN_ENABLE = 0 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_GREEN_MIN_THRESHOLD = 1 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_GREEN_MAX_THRESHOLD = 2 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_GREEN_DROP_PROBABILITY = 3 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_YELLOW_ENABLE = 4 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD = 5 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD = 6 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY = 7 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_RED_ENABLE = 8 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_RED_MIN_THRESHOLD = 9 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_RED_MAX_THRESHOLD = 10 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_RED_DROP_PROBABILITY = 11 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_WEIGHT = 12 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_ECN_MARK_ENABLE = 13 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

SAI_WRED_ATTR_CUSTOM_RANGE_BASE = 268435456 # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

sai_wred_attr_t = enum__sai_wred_attr_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 101

sai_create_wred_fn = CFUNCTYPE(UNCHECKED(sai_status_t), POINTER(sai_object_id_t), c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 117

sai_remove_wred_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 131

sai_set_wred_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 147

sai_get_wred_attribute_fn = CFUNCTYPE(UNCHECKED(sai_status_t), sai_object_id_t, c_uint32, POINTER(sai_attribute_t)) # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 162

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 177
class struct__sai_wred_api_t(Structure):
    pass

struct__sai_wred_api_t.__slots__ = [
    'create_wred_profile',
    'remove_wred_profile',
    'set_wred_attribute',
    'get_wred_attribute',
]
struct__sai_wred_api_t._fields_ = [
    ('create_wred_profile', sai_create_wred_fn),
    ('remove_wred_profile', sai_remove_wred_fn),
    ('set_wred_attribute', sai_set_wred_attribute_fn),
    ('get_wred_attribute', sai_get_wred_attribute_fn),
]

sai_wred_api_t = struct__sai_wred_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 177

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 43
try:
    HOSTIF_NAME_SIZE = 16
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saihostintf.h: 345
try:
    SAI_HOSTIF_USER_DEFINED_TRAP_ID_TABLE_RANGE = 0
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 89
try:
    true = 1
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 90
try:
    false = 0
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 127
try:
    SAI_NULL_OBJECT_ID = 0L
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 46
def SAI_STATUS_CODE(_S_):
    return (-_S_)

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 53
try:
    SAI_STATUS_SUCCESS = 0L
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 58
try:
    SAI_STATUS_FAILURE = (SAI_STATUS_CODE (1L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 63
try:
    SAI_STATUS_NOT_SUPPORTED = (SAI_STATUS_CODE (2L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 68
try:
    SAI_STATUS_NO_MEMORY = (SAI_STATUS_CODE (3L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 73
try:
    SAI_STATUS_INSUFFICIENT_RESOURCES = (SAI_STATUS_CODE (4L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 78
try:
    SAI_STATUS_INVALID_PARAMETER = (SAI_STATUS_CODE (5L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 84
try:
    SAI_STATUS_ITEM_ALREADY_EXISTS = (SAI_STATUS_CODE (6L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 90
try:
    SAI_STATUS_ITEM_NOT_FOUND = (SAI_STATUS_CODE (7L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 95
try:
    SAI_STATUS_BUFFER_OVERFLOW = (SAI_STATUS_CODE (8L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 100
try:
    SAI_STATUS_INVALID_PORT_NUMBER = (SAI_STATUS_CODE (9L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 105
try:
    SAI_STATUS_INVALID_PORT_MEMBER = (SAI_STATUS_CODE (10L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 110
try:
    SAI_STATUS_INVALID_VLAN_ID = (SAI_STATUS_CODE (11L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 115
try:
    SAI_STATUS_UNINITIALIZED = (SAI_STATUS_CODE (12L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 120
try:
    SAI_STATUS_TABLE_FULL = (SAI_STATUS_CODE (13L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 125
try:
    SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING = (SAI_STATUS_CODE (14L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 130
try:
    SAI_STATUS_NOT_IMPLEMENTED = (SAI_STATUS_CODE (15L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 135
try:
    SAI_STATUS_ADDR_NOT_FOUND = (SAI_STATUS_CODE (16L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 140
try:
    SAI_STATUS_OBJECT_IN_USE = (SAI_STATUS_CODE (17L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 147
try:
    SAI_STATUS_INVALID_OBJECT_TYPE = (SAI_STATUS_CODE (18L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 155
try:
    SAI_STATUS_INVALID_OBJECT_ID = (SAI_STATUS_CODE (19L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 167
try:
    SAI_STATUS_INVALID_ATTRIBUTE_0 = (SAI_STATUS_CODE (65536L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 168
try:
    SAI_STATUS_INVALID_ATTRIBUTE_MAX = (SAI_STATUS_CODE (131071L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 173
try:
    SAI_STATUS_INVALID_ATTR_VALUE_0 = (SAI_STATUS_CODE (131072L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 174
try:
    SAI_STATUS_INVALID_ATTR_VALUE_MAX = (SAI_STATUS_CODE (196607L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 181
try:
    SAI_STATUS_ATTR_NOT_IMPLEMENTED_0 = (SAI_STATUS_CODE (196608L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 182
try:
    SAI_STATUS_ATTR_NOT_IMPLEMENTED_MAX = (SAI_STATUS_CODE (262143L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 189
try:
    SAI_STATUS_UNKNOWN_ATTRIBUTE_0 = (SAI_STATUS_CODE (262144L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 190
try:
    SAI_STATUS_UNKNOWN_ATTRIBUTE_MAX = (SAI_STATUS_CODE (327679L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 197
try:
    SAI_STATUS_ATTR_NOT_SUPPORTED_0 = (SAI_STATUS_CODE (327680L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 198
try:
    SAI_STATUS_ATTR_NOT_SUPPORTED_MAX = (SAI_STATUS_CODE (393215L))
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 203
def SAI_STATUS_IS_INVALID_ATTRIBUTE(x):
    return (x & ((~65535) == SAI_STATUS_INVALID_ATTRIBUTE_0))

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 204
def SAI_STATUS_IS_INVALID_ATTR_VALUE(x):
    return (x & ((~65535) == SAI_STATUS_INVALID_ATTR_VALUE_0))

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 205
def SAI_STATUS_IS_ATTR_NOT_IMPLEMENTED(x):
    return (x & ((~65535) == SAI_STATUS_ATTR_NOT_IMPLEMENTED_0))

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 206
def SAI_STATUS_IS_UNKNOWN_ATTRIBUTE(x):
    return (x & ((~65535) == SAI_STATUS_INVALID_ATTRIBUTE_0))

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistatus.h: 207
def SAI_STATUS_IS_ATTR_NOT_SUPPORTED(x):
    return (x & ((~65535) == SAI_STATUS_ATTR_NOT_SUPPORTED_0))

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiswitch.h: 43
try:
    SAI_MAX_HARDWARE_ID_LEN = 255
except:
    pass

# /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 40
try:
    VLAN_COUNTER_SET_DEFAULT = 0
except:
    pass

_sai_object_list_t = struct__sai_object_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 148

_sai_u8_list_t = struct__sai_u8_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 191

_sai_s8_list_t = struct__sai_s8_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 196

_sai_u16_list_t = struct__sai_u16_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 201

_sai_s16_list_t = struct__sai_s16_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 206

_sai_u32_list_t = struct__sai_u32_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 211

_sai_s32_list_t = struct__sai_s32_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 216

_sai_u32_range_t = struct__sai_u32_range_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 221

_sai_s32_range_t = struct__sai_s32_range_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 226

_sai_vlan_list_t = struct__sai_vlan_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 239

_sai_vlan_port_t = struct__sai_vlan_port_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 263

_sai_vlan_port_list_t = struct__sai_vlan_port_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 276

_sai_ip_address_t = struct__sai_ip_address_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 290

_sai_ip_prefix_t = struct__sai_ip_prefix_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 302

_sai_acl_field_data_t = struct__sai_acl_field_data_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 347

_sai_acl_action_data_t = struct__sai_acl_action_data_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 376

_sai_port_breakout_t = struct__sai_port_breakout_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 412

_sai_qos_map_params_t = struct__sai_qos_map_params_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 459

_sai_qos_map_t = struct__sai_qos_map_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 469

_sai_qos_map_list_t = struct__sai_qos_map_list_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 477

_sai_attribute_t = struct__sai_attribute_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saitypes.h: 520

_sai_mirror_api_t = struct__sai_mirror_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saimirror.h: 203

_sai_neighbor_entry_t = struct__sai_neighbor_entry_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 79

_sai_neighbor_api_t = struct__sai_neighbor_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saineighbor.h: 180

_sai_next_hop_group_api_t = struct__sai_next_hop_group_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthopgroup.h: 201

_sai_next_hop_api_t = struct__sai_next_hop_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sainexthop.h: 174

_sai_policer_api_t = struct__sai_policer_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saipolicer.h: 272

_sai_port_oper_status_notification_t = struct__sai_port_oper_status_notification_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 86

_sai_port_event_notification_t = struct__sai_port_event_notification_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 169

_sai_port_api_t = struct__sai_port_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiport.h: 756

_sai_qos_map_api_t = struct__sai_qos_map_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqosmaps.h: 179

_sai_queue_api_t = struct__sai_queue_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiqueue.h: 231

_sai_unicast_route_entry_t = struct__sai_unicast_route_entry_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 82

_sai_route_api_t = struct__sai_route_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairoute.h: 170

_sai_virtual_router_api_t = struct__sai_virtual_router_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouter.h: 156

_sai_router_interface_api_t = struct__sai_router_interface_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/sairouterintf.h: 180

_sai_samplepacket_api_t = struct__sai_samplepacket_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saisamplepacket.h: 134

_sai_scheduler_group_api_t = struct__sai_scheduler_group_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischedulergroup.h: 174

_sai_scheduler_api_t = struct__sai_scheduler_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saischeduler.h: 157

_sai_stp_api_t = struct__sai_stp_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saistp.h: 158

_sai_udf_api_t = struct__sai_udf_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiudf.h: 372

_sai_vlan_api_t = struct__sai_vlan_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saivlan.h: 257

_sai_wred_api_t = struct__sai_wred_api_t # /ctcdata/users/sw/kcao/osp_sai/system/sai/inc/saiwred.h: 177

# No inserted files

