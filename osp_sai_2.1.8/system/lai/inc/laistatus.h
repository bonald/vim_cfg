/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-11-25
* Reason        : First Create.
****************************************************************************/

#if !defined (__LAISTATUS_H_)
#define __LAISTATUS_H_


#ifdef _WIN32

#define LAI_STATUS_CODE(_S_)    ((lai_status_t)(_S_)) 

#else
//
// Any mapping schema from _S_ to error codes convenient on *nix can be used.
//
#define LAI_STATUS_CODE(_S_)    (-(_S_))
#endif


/** 
 *  Status success 
 */
#define LAI_STATUS_SUCCESS                                  0x00000000L

/** 
 *  General failure 
 */
#define LAI_STATUS_FAILURE                  LAI_STATUS_CODE(0x00000001L)  

/** 
 *  The request is not supported 
 */
#define LAI_STATUS_NOT_SUPPORTED            LAI_STATUS_CODE(0x00000002L)

/** 
 *  Not enough memory to complete the operation 
 */
#define LAI_STATUS_NO_MEMORY                LAI_STATUS_CODE(0x00000003L)

/** 
 *  Insufficient system resources exist to complete the operation 
 */
#define LAI_STATUS_INSUFFICIENT_RESOURCES   LAI_STATUS_CODE(0x00000004L)

/**  
 *  An invalid parameter was passed to a function 
 */
#define LAI_STATUS_INVALID_PARAMETER        LAI_STATUS_CODE(0x00000005L)

/**  
 *  An item already exists. For example, create a route which already
 *  exists.
 */
#define LAI_STATUS_ITEM_ALREADY_EXISTS      LAI_STATUS_CODE(0x00000006L)

/**  
 *  An item was not found. For example, remove a route which does not
 *  exist.
 */
#define LAI_STATUS_ITEM_NOT_FOUND           LAI_STATUS_CODE(0x00000007L)

/** 
 *  The data was too large to fit into the specified buffer. 
 */ 
#define LAI_STATUS_BUFFER_OVERFLOW          LAI_STATUS_CODE(0x00000008L)

/**  
 *  Invalid port number
 */
#define LAI_STATUS_INVALID_PORT_NUMBER      LAI_STATUS_CODE(0x00000009L)

/**  
 *  Invalid port member
 */
#define LAI_STATUS_INVALID_PORT_MEMBER      LAI_STATUS_CODE(0x0000000AL)

/**  
 *  Invalid VLAN id
 */
#define LAI_STATUS_INVALID_VLAN_ID          LAI_STATUS_CODE(0x0000000BL)

/**  
 *  Object is uninitialized 
 */
#define LAI_STATUS_UNINITIALIZED            LAI_STATUS_CODE(0x0000000CL)

/**  
 *  Table is full
 */
#define LAI_STATUS_TABLE_FULL               LAI_STATUS_CODE(0x0000000DL)

/**
 *  Mandatory attribute is missing 
 */
#define LAI_STATUS_MANDATORY_ATTRIBUTE_MISSING     LAI_STATUS_CODE(0x0000000EL)

/**  
 *  Function is not implemented
 */
#define LAI_STATUS_NOT_IMPLEMENTED          LAI_STATUS_CODE(0x0000000FL)

/**  
 *  Address not found 
 */
#define LAI_STATUS_ADDR_NOT_FOUND           LAI_STATUS_CODE(0x00000010L)

/**
 *  SAI Object is in use
 */
#define LAI_STATUS_OBJECT_IN_USE            LAI_STATUS_CODE(0x00000011L)

/**
 * Invalid SAI Object type passed to a function. If the object id
 * is passed as an attribute value in list, then 
 * LAI_STATUS_INVALID_ATTR_VALUE_0 status code should be used
 */
#define LAI_STATUS_INVALID_OBJECT_TYPE      LAI_STATUS_CODE(0x00000012L)


/**
 * Invalid SAI Object ID passed to a function. Return LAI_STATUS_INVALID_OBJECT_TYPE
 * when the object type is invalid, otherwise return this LAI_STATUS_INVALID_OBJECT_ID.
 * This can happen if the object id is already deleted.
 */
#define LAI_STATUS_INVALID_OBJECT_ID        LAI_STATUS_CODE(0x00000013L)

#define LAI_STATUS_PORT_NOT_INIT        LAI_STATUS_CODE(0x00000014L) 
/**  
 *  Attribute is invalid (range from 0x00010000L to 0x0001FFFFL). 
 *  Return this when the property attached to the attribute does not match
 *  the API call. For example:
 *    - When a READ_ONLY attribute is passed set_attribute or create call
 *    - When a CREATE_ONLY attribute is passed to set_attribute call
 *
 *  The offset from LAI_STATUS_INVLAID_ATTRIBUTE_0 is the array index of
 *  the attribute list passed to the SAI API call. 
 */
#define LAI_STATUS_INVALID_ATTRIBUTE_0      LAI_STATUS_CODE(0x00010000L)
#define LAI_STATUS_INVALID_ATTRIBUTE_MAX    LAI_STATUS_CODE(0x0001FFFFL)

/**  
 *  Invalid attribute value (range from 0x00020000L to 0x0002FFFFL)
 */
#define LAI_STATUS_INVALID_ATTR_VALUE_0     LAI_STATUS_CODE(0x00020000L)
#define LAI_STATUS_INVALID_ATTR_VALUE_MAX   LAI_STATUS_CODE(0x0002FFFFL)

/**  
 *  Attribute is not implmented (range from 0x00030000L to 0x003FFFFL)
 *  Return this when the attribute is supported but not implemented on 
 *  the platform.
 */
#define LAI_STATUS_ATTR_NOT_IMPLEMENTED_0   LAI_STATUS_CODE(0x00030000L)
#define LAI_STATUS_ATTR_NOT_IMPLEMENTED_MAX LAI_STATUS_CODE(0x0003FFFFL)

/**  
 *  Attribute is unknown (range from 0x00040000L to 0x004FFFFL)
 *  Return this when the attribute is undefined, e.g., the attribute id 
 *  is out of range.
 */
#define LAI_STATUS_UNKNOWN_ATTRIBUTE_0      LAI_STATUS_CODE(0x00040000L)
#define LAI_STATUS_UNKNOWN_ATTRIBUTE_MAX    LAI_STATUS_CODE(0x0004FFFFL)

/**
 *  Attribute is not supported (range from 0x00050000L to 0x005FFFFL)
 *  Return this when the platform does not has the capability to support
 *  this attribute.
 */
#define LAI_STATUS_ATTR_NOT_SUPPORTED_0   LAI_STATUS_CODE(0x00050000L)
#define LAI_STATUS_ATTR_NOT_SUPPORTED_MAX LAI_STATUS_CODE(0x0005FFFFL)

/**
 *  Macros to check if attribute related error is within the specified range
 */
#define LAI_STATUS_IS_INVALID_ATTRIBUTE(x)      ((x) & ~0xFFFF == LAI_STATUS_INVALID_ATTRIBUTE_0)
#define LAI_STATUS_IS_INVALID_ATTR_VALUE(x)     ((x) & ~0xFFFF == LAI_STATUS_INVALID_ATTR_VALUE_0)
#define LAI_STATUS_IS_ATTR_NOT_IMPLEMENTED(x)   ((x) & ~0xFFFF == LAI_STATUS_ATTR_NOT_IMPLEMENTED_0)
#define LAI_STATUS_IS_UNKNOWN_ATTRIBUTE(x)      ((x) & ~0xFFFF == LAI_STATUS_INVALID_ATTRIBUTE_0)
#define LAI_STATUS_IS_ATTR_NOT_SUPPORTED(x)     ((x) & ~0xFFFF == LAI_STATUS_ATTR_NOT_SUPPORTED_0)


#endif // _LAISTATUS_H_

