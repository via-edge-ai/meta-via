/******************************************************************************

 Copyright (c) 2020 VIA Technologies, Inc. All Rights Reserved.

 This PROPRIETARY SOFTWARE is the property of VIA Technologies, Inc.
 and may contain trade secrets and/or other confidential information of
 VIA Technologies, Inc. This file shall not be disclosed to any third
 party, in whole or in part, without prior written consent of VIA.

 THIS PROPRIETARY SOFTWARE AND ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
 WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND EITHER EXPRESS OR IMPLIED,
 AND VIA TECHNOLOGIES, INC. DISCLAIMS ALL EXPRESS OR IMPLIED
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 ENJOYMENT OR NON-INFRINGEMENT.

******************************************************************************/
#ifndef AT_TOK_H
#define AT_TOK_H 1

#define TTY_DEVICE "/dev/ttyUSB0"
#define TTY_SPEED  B115200

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
Function   : at_open_uart
Description: Open TTY device for AT commands
Parameter  : ttyDevice: The TTY device file for opening, if NULL, open /dev/ttyUSB2
Return     : The file handler of ttyDevice
***************************************************************************************************/
int at_open_uart(char *ttyDevice);

/***************************************************************************************************
Function   : at_close_uart
Description: Close the file handler of ttyDevice opened by at_open_uart()
Parameter  : fd: The file handler of ttyDevice
Return     : None
***************************************************************************************************/
void at_close_uart(int fd);

/***************************************************************************************************
Function   : send_at_command
Description: Send an AT command to LTE module and receive the response and data from LTE module.
Parameter  : fd: The file handler of ttyDevice.
             cmd: The AT command.
             max_response_ms: The maximum response time of LTE module for the AT command.
             The wait time begins to receive the expected string from LTE module.
             ok_str: A string that indicates that the LTE module executes the AT command correctly.
             error_str: A string indicates that the LTE module executes the AT command incorrectly.
             expect_str: The header of expected string received from LTE module.
             rcv_str: The complete string contains the header represented by expect_str.
             rcv_len: The length of rcv_str.
Return     :  0: The LTE module executes the AT command correctly.
             <0: Error
***************************************************************************************************/
int send_at_command(int fd, const char *cmd, int max_response_ms, int delay_ms, const char *ok_str, const char *error_str, const char *expect_str, char *rcv_str, int rcv_len);

/***************************************************************************************************
Function   : receive_at_urc
Description: Receive the wakeup package from the LTE module when the device is waked up by LTE module.
Parameter  : fd: The file handler of ttyDevice.
             max_response_ms: The maximum time that LTE module return the wakeup package.
             rcv_times: The retry times.
             rcv_str: The wakeup package.
             rcv_len: The length of wakeup package.
Return     :  0: Receive the wakeup package successfully
             <0: Error
***************************************************************************************************/
int receive_at_urc(int fd, int max_response_ms, int rcv_times, char *rcv_str, int rcv_len);

/***************************************************************************************************
Function   : enable_at_cmd_log
Description: Whether to print the detail log when executing the AT command.
Parameter  : Enable=1: print log.
             Enable=0: Don't print log.
Return     : None
***************************************************************************************************/
void enable_at_cmd_log(int enable);

#ifdef __cplusplus
}
#endif

#endif /* AT_TOK_H */

