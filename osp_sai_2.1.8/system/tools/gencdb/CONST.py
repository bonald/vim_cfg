########################################
# const for node.txt
NODE_PREFIX_DIR = 'DIR_'
NODE_PREFIX_TBL = 'TBL_'
NODE_PREFIX_DS  = 'DS_'
NODE_PREFIX_ACT = 'ACT_'
NODE_PREFIX_BAT = 'BAT_'

PRODUCT_PREFIX  = 'CDB_PRODUCT_'

PYDT_PTR_STR = 'I' # or 'P'

MAX_FIELD_NUM		= 196

PARAM_TYPE_NONE		= 0
PARAM_TYPE_TBL		= 1
PARAM_TYPE_TBL_KEY	= 2

CDB_ROOT_PATH	= './'
CDB_INPUT_PATH	= './define/'
CDB_H_PATH		= 'core/include/gen/'
CDB_C_PATH		= 'core/src/gen/'
CDB_PY_PATH		= 'py/gen/'
PBC_H_PATH		= 'protobuf/include/gen/'
PBC_C_PATH 		= 'protobuf/src/gen/'
PBC_PY_PATH		= 'protobuf/py/'

PBC_PROTO_COM   = CDB_ROOT_PATH + 'common.proto'
PBC_PROTO_TBL   = CDB_ROOT_PATH + 'tbl.proto'
PBC_PROTO_DS    = CDB_ROOT_PATH + 'ds.proto'

PROTO_H_FILE = 'proto.h'
CDB_CMP_H_FILE = 'cdb_data_cmp.h'
CMP_FUNC_FILES = 'cdb_data_cmp.h/cdb_data_cmp.c'
VAL2STR_FUNC_FILES = 'cdb_ds.h/cdb_ds.c'
MEMORY_FILES   = 'ctclib_memory.h/memory.c'

FUNC_REFERLIST_VAL2STR = 'cdb_reference_list_val2str()'

TBL_PREFIX_TBL		= '[TBL]'
TBL_PREFIX_DS		= '[DS]'
TBL_PREFIX_KEY		= '[KEY]'
TBL_PREFIX_DATA		= '[DATA]'
TBL_PREFIX_FLAGS	= '[FLAGS]'
TBL_PREFIX_MASTER	= '[MASTER]'

ACT_PREFIX_ACT		= '[ACT]'
ACT_PREFIX_SUB		= '[SUB]'

DT_PREFIX_DT_TYPE	= '[DT_TYPE]'
DT_PREFIX_DT		= '[DT_MAP]'

PYDT_OTHR_TYPES		= '[OTHR_TYPES]'
PYDT_UINT_TYPES		= '[UINT_TYPES]'
PYDT_STRING_KEY		= '[STRING_KEY]'
PYDT_CHAR_ARRAY		= '[CHAR_ARRAY]'

TBL_LINE_INVALID = 0
TBL_LINE_TBL     = 1
TBL_LINE_KEY     = 2
TBL_LINE_DATA    = 3
TBL_LINE_FLAGS   = 4
TBL_LINE_MASTER  = 5
TBL_LINE_END     = 6

DS_LINE_INVALID = 0
DS_LINE_DS      = 1
DS_LINE_KEY     = 2
DS_LINE_DATA    = 3
DS_LINE_FLAGS   = 4
DS_LINE_END     = 5

ACT_LINE_INVALID = 0
ACT_LINE_ACT     = 1
ACT_LINE_SUB     = 2
ACT_LINE_END     = 3

DT_LINE_INVALID	= 0
DT_LINE_DT_TYPE	= 1
DT_LINE_DT		= 2
DT_LINE_END		= 3

PYDT_LINE_INVALID		= 0
PYDT_LINE_OTHR_TYPES	= 1
PYDT_LINE_UINT_TYPES	= 2
PYDT_LINE_STRING_KEY	= 3
PYDT_LINE_CHAR_ARRAY	= 4
PYDT_LINE_END			= 5

LOG_LINE_INVALID        = 0
LOG_LINE_MODULE         = 1
LOG_ITEM_ID             = 2
LOG_ITEM_SENTENCE       = 3
LOG_ITEM_PARAM          = 4
LOG_ITEM_EXPLANATION    = 5
LOG_ITEM_ACTION         = 6
LOG_ITEM_PARAM_CN       = 7
LOG_ITEM_EXPLANATION_CN = 8
LOG_ITEM_ACTION_CN      = 9
LOG_ITEM_END            = 10

BUF				= 'buf'
BUF_SIZE		= 'MAX_CMD_STR_LEN'


REFERLIST_TYPE_ALL = 0
REFERLIST_TYPE_TBL = 1
REFERLIST_TYPE_DS  = 2

PM_ARRAY = ['all',
            'switch',
            'opm',
            'routed',
            'chsm',
            'appcfg',
            'fea',
            'openflow',
            'dhcrelay',
            'dhcsnooping',
            'dhclient',
			'authd',
			'ptp',
            'bhm',
            'emgr',
			'ssm',
			'sub',
			'ncs',
            'dhcpd6',
			'dhclient6',
		   ]

DEST_PM_ARRAY = [
        ['ctl', 'PM_ID_CTL'],
        ['ccs', 'PM_ID_CCS'],
		['cds', 'PM_ID_CDS'],
        ['chsm', 'PM_ID_CHSM'],
        ['switch', 'PM_ID_SWITCH'],
        ['routed', 'PM_ID_ROUTED'],
        ['opm', 'PM_ID_OPM'],
        ['appcfg', 'PM_ID_APPCFG'],
        ['fea', 'PM_ID_FEA'],
        ['openflow', 'PM_ID_OPENFLOW'],
        ['dhcrelay', 'PM_ID_DHCRELAY'],
        ['dhcsnooping', 'PM_ID_DHCSNOOPING'],
        ['dhclient', 'PM_ID_DHCLIENT'],
        ['authd', 'PM_ID_AUTHD'],
        ['ptp', 'PM_ID_PTP'],
        ['bhm', 'PM_ID_BHM'],
        ['emgr', 'PM_ID_EVENTMGR'],
        ['ssm', 'PM_ID_SSM'],
        ['sub', 'PM_ID_SUBSCRIBER'],
        ['ncs', 'PM_ID_NCS'],
        ['dhcpd6', 'PM_ID_DHCPD6'],
		['dhclient6', 'PM_ID_DHCLIENT6'],
		['all', 'PM_ID_MAX']
    ]

PRODUCT_ARRAY = ['ALL',
                 'L2PLUS',
                 'OPENFLOW',
                 'TAP',
                ]
