/****************************************************************************
 * ctckal_usrctrl.h :        user control module header
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         Bo Xiong
 * Date           :         2005-7-28
 * Reason         :         First Create
 ****************************************************************************/
#ifndef _CTCKAL_USRCTRL_H_
#define _CTCKAL_USRCTRL_H_

/*
 * exported functions
 */
int32_t ctckal_usrctrl_init(void);

/*
 * Bay
 */
extern int32_t dal_usrctrl_write_bay(uint32_t chip_id, uint32_t reg_offset, uint32_t value);
extern int32_t dal_usrctrl_read_bay(uint32_t chip_id, uint32_t reg_offset, uint32_t p_value);

#endif  /* _CTCKAL_USRCTRL_H_ */
