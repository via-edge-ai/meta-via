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
#ifndef LTE_API_H
#define LTE_API_H

#include "at_tok.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ANTENNA_TYPE{
    ANTENNA_ENABLE_BOTH, ANTENNA_ENABLE_MAIN, ANTENNA_ENABLE_BACKUP, ANTENNA_QUERY_SETTING,
}LTE_ANTENNA_TYPE;

/***************************************************************************************************
Function   : lte_check_sim_card_exist
Description: Send AT command "AT+CPIN?" to check whether the SIM card is plugged into the SIM card slot.
Parameter  : fd: The file handler of /dev/ttyUSB2.
Return     :   0: The SIM card is NOT plugged into the card slot.
               1: The SIM card is plugged into the card slot.
             < 0: Error
***************************************************************************************************/
int lte_check_sim_card_exist(int fd);

/***************************************************************************************************
Function   : lte_get_signal_strength
Description: Send AT command "AT+CSQ" to obtain the signal strength of LTE.
Parameter  : fd: The file handler of /dev/ttyUSB2.
Return     : >=0: The returned signal strength.
             <0 : Failed to get the signal strength.
***************************************************************************************************/
int lte_get_signal_strength(int fd);

/***************************************************************************************************
Function   : lte_get_mcc_mnc
Description: Send AT command "AT+CIMI" to read MCC and MNC of the SIM card.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_mcc: The returned MCC.
             p_mnc: The returned MNC.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_mcc_mnc(int fd, int *p_mcc, int *p_mnc, char *quirk);

/***************************************************************************************************
Function   : lte_get_apn
Description: Get the dialup parameters (APN, user and password) from /etc/apns-conf.xml according
             to the MCC and MNC of the SIM card.
             The MCC and MNC of the SIM card are read through the AT command "AT+CIMI" from the LTE module.
Parameter  : fd: The file handler of /dev/ttyUSB2.
			 p_dialup_param: The dialup parameters include APN, user and password.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_apn(int fd, char **p_dialup_param, char *quirk);

/***************************************************************************************************
Function   : lte_get_imei
Description: Send AT command "AT+GSN" to read the IMEI number of the SIM card.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_imei: The IMEI number of the SIM card.
             imei_len: The length of the buffer that store the IMEI number.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_imei(int fd, char *p_imei, int imei_len);

/***************************************************************************************************
Function   : lte_get_iccid
Description: Send AT command "AT+QCCID" to read the ICCID number of the SIM card.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_ccid: The ICCID number of the SIM card.
             iccid_len: The length of the buffer that store the ICCID nmber.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_iccid(int fd, char *p_iccid, int iccid_len);

/***************************************************************************************************
Function   : lte_get_fw_version
Description: Send AT command "AT+QGMR" to read the firmware version of the LTE module.
             The example of the firmware version: EC25AFFDR07A08M4G_01.002.01.002
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_version: The firmware version of the LTE module.
             version_len: The length of the buffer that store the firmware version.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_fw_version(int fd, char *p_version, int version_len);

/***************************************************************************************************
Function   : lte_get_fw_simple_version
Description: Send AT command "AT+GMR" to read the header of the firmware version of the LTE module.
             The example of header of the firmware version: EC25AFFDR07A08M4G
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_version: The firmware version of the LTE module.
             version_len: The length of the buffer that store the firmware version.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_fw_simple_version(int fd, char *p_version, int version_len);

/***************************************************************************************************
Function   : lte_get_CTZU
Description: Send AT command "AT+CTZU?" to check whether automatic time zone update via NITZ.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_ctzu: The mode of automatic time zone update.
                     0: Disable automatic time zone update via NITZ.
                     1: Enable automatic time zone update via NITZ and update GMT time to RTC.
                     3: Enable automatic time zone update via NITZ and update LOCAL time to RTC.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_CTZU(int fd, int *p_ctzu);

/***************************************************************************************************
Function   : lte_set_CTZU
Description: Send AT command "AT+CTZU=1" or "AT+CTZU=3" to enable automatic time zone update via NITZ.
             Send AT command "AT+CTZU=0" to disable automatic time zone update via NITZ.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             ctzu: The mode of automatic time zone update.
                   0: Disable automatic time zone update via NITZ.
                   1: Enable automatic time zone update via NITZ and update GMT time to RTC.
                   3: Enable automatic time zone update via NITZ and update LOCAL time to RTC.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_set_CTZU(int fd, int ctzu);

/***************************************************************************************************
Function   : lte_get_timezone_quarters
Description: Send AT command "AT+CCLK?" to query the time zone of the LTE module.
             The time zone is expressed in quarters of an hour, between the local time and GMT.
             Range -48...+56
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_quarters: The time zone expressed in quarters.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_timezone_quarters(int fd, int *p_quarters);

/***************************************************************************************************
Function   : lte_convert_timezone_quarters_to_string
Description: Convert the time zone to string.
             For example: The time zone quarters is 32. Covert it to string as GMT+08:00
Parameter  : quarters: The time zone expressed in quarters.
			 tz_buf: The time zone string.
			 tz_len: The length of the time zone string.
Return     : None
***************************************************************************************************/
void lte_convert_timezone_quarters_to_string(int quarters, char *tz_buf, int tz_len);

/***************************************************************************************************
Function   : lte_convert_timezone_quarters_to_string
Description: The function do the preparation for device suspend. Before the device suspend,
             the device sends AT commands to let the LTE module dial up. And then sends AT commands
             to let the LTE module connect to the server using TCP protocol. After the device suspend,
             the LTE module wakes up the device once it receives the wakeup packet from server.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             serverIp: The IP address of server.
             sPort: The port number of server.
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_suspend(int fd, const char *serverIp, int sPort);

/***************************************************************************************************
Function   : lte_reset
Description: Pull the reset pin of the LTE module to reset the LTE module.
Parameter  : None
Return     : None
***************************************************************************************************/
void lte_reset(void);

/***************************************************************************************************
Function   : lte_poweroff
Description: Pull the power pin of the LTE module to power off the LTE module.
Parameter  : None
Return     : None
***************************************************************************************************/
void lte_poweroff(void);

/***************************************************************************************************
Function   : lte_poweron
Description: Pull the power pin and the reset pin of the LTE module to power on the LTE module.
Parameter  : None
Return     : None
***************************************************************************************************/
void lte_poweron(void);

/***************************************************************************************************
Function   : lte_check_ip_valid
Description: Check whether the IP address of LTE is valid.
Parameter  : None
Return     : 0: The IP address is valid.
            <0: The IP address is NOT valid.
***************************************************************************************************/
int lte_check_ip_valid(void);

/***************************************************************************************************
Function   : lte_init
Description: Initialize the LTE module. It power on or reset the LTE module to make the LTE module work well.
Parameter  : None
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_init(void);

/***************************************************************************************************
Function   : lte_set_antenna
Description: Enable/Disable the RX of main antenna and diversity antenna.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             type: Enable which antenna. As follows:
                   0: Enable both main antenna and diversity antenna.
                   1: Enable main antenna and Disable diversity antenna.
                   2: Enalbe diversity antenna and Disable main antenna.
                   3: Query wich antenna is enabled.
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_set_antenna(int fd, LTE_ANTENNA_TYPE type);

/***************************************************************************************************
Function   : lte_set_cmd
Description: Send an AT command to the LTE module.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_cmd: The AT command.
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_set_cmd(int fd, char *p_cmd);

/***************************************************************************************************
Function   : lte_get_tx_power
Description: Send AT command "AT+QNVFR" or "AT+QNVR" to get the value of TX power from LTE module.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_cmd: The AT command.
             p_value: The returned value of TX power
             value_buf_len: The length of buffer that store the value of TX power.
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_get_tx_power(int fd, char *p_cmd, char *p_value, int value_buf_len);

#ifdef __cplusplus
}
#endif

#endif /* LTE_API_H */

