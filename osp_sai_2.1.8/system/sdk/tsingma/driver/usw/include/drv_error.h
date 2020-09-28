/**
 @file drv_error.h

 @date 2010-02-25

 @version v5.1

 The file contains driver error code definition
*/

#ifndef _DRV_ERROR_H
#define _DRV_ERROR_H

/**********************************************************************************
              Define Typedef, define and Data Structure
***********************************************************************************/
enum drv_err_e
{
    DRV_E_NONE = 0,
    DRV_E_NO_MEMORY = -10000,
    DRV_E_OCCPANCY,/* -9999 */
    DRV_E_TIME_OUT,/* -9998 */
    DRV_E_EXCEED_MAX_SIZE,/* -9997 */
    DRV_E_INVALID_HASH_MODULE_TYPE,/* -9996 */
    DRV_E_INVALID_ADDR,/* -9995 */
    DRV_E_FIELD_OVER,/* -9994 */
    DRV_E_INVALID_TBL,/* -9993 */
    DRV_E_INVALID_FLD,/* -9992 */
    DRV_E_HASH_CONFLICT,/* -9991 */
    DRV_E_INVALID_CHIP,/* -9990 */
    DRV_E_INVALID_MEM,/* -9989 */
    DRV_E_INVALID_PTR,/* -9988 */
    DRV_E_WRONG_SIZE,/* -9987 */
    DRV_E_HARD_DISCARD,/* -9986 */
    DRV_E_TCAM_RESET,/* -9985 */
    DRV_E_NOT_FOUND,/* -9984 */
    DRV_E_INVAILD_TYPE,/* -9983 */
    DRV_E_ILLEGAL_LENGTH,/* -9982 */
    DRV_E_FAIL_CREATE_MUTEX,/* -9981 */
    DRV_E_INIT_FAILED,/* -9980 */
    DRV_E_INVALID_ALLOC_INFO,/* -9979 */
    DRV_E_FILE_OPEN_FAILED,/* -9978 */
    DRV_E_INVALID_POINT,/* -9977 */
    DRV_E_INVALID_CHANID,/* -9976 */
    DRV_E_SEND_CONDITION,/* -9975 */
    DRV_E_NOT_FOUND_TBL,/* -9974 */
    DRV_E_TBL_ENTRY_SIZE_NOT_MATCH,/* -9973 */
    DRV_E_TBL_ENTRY_DATA_NOT_MATCH,/* -9972 */
    DRV_E_CHECK_CMODEL_LINKLIST_IS_NOT_EMPTY,/* -9971 */
    DRV_E_CMODEL_AND_RTL_PKT_NOT_EQUAL,/* -9970 */
    DRV_E_INVALID_TCAM_TYPE,/* -9969 */
    DRV_E_FIELD_VALUE_ERROR,/* -9968 */
    DRV_E_RESERVD_VALUE_ERROR,/* -9967 */
    DRV_E_CREATE_TIMER,/* -9966 */
    DRV_E_CREATE_EVENT,/* -9965 */
    DRV_E_WAIT_EVENT,/* -9964 */
    DRV_E_INVALID_PARAMETER,/* -9963 */
    DRV_E_BIT_OUT_RANGE,/* -9962 */
    DRV_E_STORE_BUS_NO_OPEN_FILE,/* -9961 */
    DRV_E_STORE_BUS_INTER_OPERATION,/* -9960 */
    DRV_E_TCAM_KEY_DATA_ADDRESS,/* -9959 */
    DRV_E_TCAM_KEY_MASK_ADDRESS,/* -9958 */
    DRV_E_TCAM_KEY_CONFIG_ERROR,/* -9957 */
    DRV_E_INVALID_TCAM_INDEX,/* -9956 */
    DRV_E_INVALID_TCAM_ALLOC,/* -9955 */
    DRV_E_INVALID_HASH_ALLOC,/* -9954 */
    DRV_E_INVALID_HASH_FREE,/* -9953 */
    DRV_E_INVALID_DYNIC_TBL_ALLOC,/* -9952 */
    DRV_E_INVALID_ACTION_TBL_ALLOC,/* -9951 */
    DRV_E_INVALID_CFG_TBL_ID,/* -9950 */
    DRV_E_INVALID_CFG_NO_MATCH,/* -9949 */
    DRV_E_INVALID_CFG_TBL_BASE,/* -9948 */
    DRV_E_INVALID_HASH_TABLE_SIZE,/* -9947 */
    DRV_E_DATAPATH_CHECK_PLL_LOCK_FAIL,/* -9946 */
    DRV_E_DATAPATH_CHECK_DL_LOCK_FAIL,/* -9945 */
    DRV_E_INVALD_RUNNING_ENV_TYPE,/* -9944 */
    DRV_E_INVALD_RUNNING_PKT_LEN,/* -9943 */
    DRV_E_READ_NOT_CONFIG_TBL_ENTRY,/* -9942 */
    DRV_E_FATAL_EXCEP,/* -9941 */
    DRV_E_INVALID_HASH_TYPE,/* -9940 */
    DRV_E_INVALID_SIM_MODULE,/* -9939 */
    DRV_E_INVALID_SIM_BUS,/* -9938 */
    DRV_CHECK_LINKLIST_IS_NOT_EMPTY,/* -9937 */
    DRV_E_INVALID_CRC_POLYNOMIAL_TYPE,/* -9936 */
    DRV_E_CALL_WRONG_FUNCTION,/* -9935 */
    DRV_E_INVALID_INDEX,/* -9934 */
    DRV_E_PKT_MISMATCH,/* -9933 */
    DRV_E_INVALID_ACCESS_TYPE,/* -9932 */
    DRV_E_CMD_NOT_DONE,/* -9931 */
    DRV_E_PCI_CMD_ERROR,/* -9930 */
    DRV_E_I2C_CMD_ERROR,/* -9929 */
    DRV_E_MDIO_CMD_ERROR,/* -9928 */
    DRV_E_MDIO_CMD_TIMEOUT,/* -9927 */
    DRV_E_I2C_MASTER_READ_NOT_CLEAR,/* -9926 */
    DRV_E_I2C_MASTER_POLLING_NOT_DONE,/* -9925 */
    DRV_E_I2C_MASTER_CMD_ERROR,/* -9924 */
    DRV_E_DATAPATH_INIT_DATAPATH_FAIL,/* -9923 */
    DRV_E_DATAPATH_INVALID_HSS_MODE,/* -9922 */
    DRV_E_DATAPATH_HSS_PLL_LOCK_FAIL,/* -9921 */
    DRV_E_DATAPATH_HSS_INVALID_RATE,/* -9920 */
    DRV_E_DATAPATH_CHANNEL_ENABLE_CONFLICT,/* -9919 */
    DRV_E_DATAPATH_WRITE_CHIP_FAIL,/* -9918 */
    DRV_E_DATAPATH_READ_CHIP_FAIL,/* -9917 */
    DRV_E_DATAPATH_RESET_CHIP_FAIL,/* -9916 */
    DRV_E_DATAPATH_INIT_PLL_FAIL,/* -9915 */
    DRV_E_DATAPATH_INIT_HSS_FAIL,/* -9914 */
    DRV_E_FIB_MAC_ACCELERATE_FAIL,/* -9913 */
    DRV_E_ACCESS_HSS12G_FAIL,/* -9912 */
    DRV_E_MAC_IS_NOT_USED,/* -9911 */
    DRV_E_HSS_LANE_IS_NOT_USED,/* -9910 */
    DRV_E_DATAPATH_EXCEED_MAX_POINTER,/* -9909 */
    DRV_E_DATAPATH_INVALID_CHANNEL_ID,/* -9908 */
    DRV_E_DATAPATH_INVALID_PORT_ID,/* -9907 */
    DRV_E_I2C_MASTER_NACK_ERROR,/* -9906 */
    DRV_E_I2C_MASTER_CRC_ERROR,/* -9905 */
    DRV_E_NULL_WBIT,/* -9904 */
    DRV_E_CapwapPreReassemble_Hold_Pkt,/* -9903 */
    DRV_E_CapwapPreReassemble_Drop_Pkt,/* -9902 */
    DRV_E_NOT_INIT,/* -9901 */


    _DRV_E_LAST
};
typedef enum drv_err_e drv_e_t;

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/

#endif /*end of _DRV_ERROR_H*/

