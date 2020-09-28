/****************************************************************************
* $Id$
*  process serial parameter
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-10-23 08:29
* Reason        : First Create.
****************************************************************************/

#ifndef _APP_SERIAL_H_
#define _APP_SERIAL_H_

int check_serial_baudrate(int value);
int set_serial_baudrate(int speed);
int set_serial_parity(int parity);
int set_serial_databits(int databits);
int set_serial_stopbits(int stopbits);
int get_serial_buadrate(void);
int get_serial_parity(void);
int get_serial_databits(void);
int get_serial_stopbits(void);

#endif /* !_APP_SERIAL_H_ */
