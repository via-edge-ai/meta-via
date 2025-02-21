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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
//#include "viacommon.h"
//#include "nvtgpio.h"
#include "at_tok.h"
#include "xmlparser.h"
//#include "wifi_api.h"
#include "lte_api.h"


extern "C" {

/***************************************************************************************************
Function   : lte_check_sim_card_exist
Description: Send AT command "AT+CPIN?" to check whether the SIM card is plugged into the SIM card slot.
Parameter  : fd: The file handler of /dev/ttyUSB2.
Return     :   0: The SIM card is NOT plugged into the card slot.
               1: The SIM card is plugged into the card slot.
             < 0: Error
***************************************************************************************************/
int lte_check_sim_card_exist(int fd)
{
	int ret;

	ret = send_at_command(fd, "AT+CPIN?", 5000, 0, "OK", "+CME ERROR", "+CPIN: ", NULL, 0);
	return ret;
}

/***************************************************************************************************
Function   : lte_get_signal_strength
Description: Send AT command "AT+CSQ" to obtain the signal strength of LTE.
Parameter  : fd: The file handler of /dev/ttyUSB2.
Return     : >=0: The returned signal strength.
             <0 : Failed to get the signal strength.
***************************************************************************************************/
int lte_get_signal_strength(int fd)
{
	int i, ret, len, tmp_len, signal;
	char buf[64], tmp[64], *p;

	ret = send_at_command(fd, "AT+CSQ", 1000, 0, "OK", "ERROR", "+CSQ: ", buf, sizeof(buf));
	if(ret == 0)
	{
		p = buf;

		p += strlen("+CSQ: ");
		len = strlen(p);
		tmp_len = sizeof(tmp);
		if(len >= tmp_len)
			len = tmp_len - 1;

		memset(tmp, 0, tmp_len);
		for(i = 0; i < len; i++)
		{
			if(p[i] >= '0' && p[i] <= '9')
				tmp[i] = p[i];
			else
				break;
		}

		signal = atoi(tmp);

		return signal;
	}
	else
		return ret;
}

/***************************************************************************************************
Function   : lte_get_mcc_mnc
Description: Send AT command "AT+CIMI" to read MCC and MNC of the SIM card.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_mcc: The returned MCC.
             p_mnc: The returned MNC.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
/*
root@NVTEVM:~$ microcom /dev/ttyUSB2
AT+CIMI
460020209682714

OK
*/
int lte_get_mcc_mnc(int fd, int *p_mcc, int *p_mnc, char *quirk)
{
	int i, ret, len, tmp_len, mcc, mnc;
	char buf[64], tmp[64], *p;

	ret = send_at_command(fd, "ATE", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
	if(ret != 0)
		return ret;

	if (strcmp(quirk, "telit")) {
		ret = send_at_command(fd, "AT+CIMI", 1000, 0, "OK", "ERROR", "AT+CIMI", buf, sizeof(buf));
	} else {
		ret = send_at_command(fd, "AT+CIMI", 1000, 0, "OK", "ERROR", NULL, buf, sizeof(buf));
	}
	if(ret == 0)
	{
		p = buf;

		if (strcmp(quirk, "telit")) {
			p += strlen("AT+CIMI");
			len = strlen(p);
		}

		//Find the first number
		for(i = 0; i < len; i++)
		{
			if(p[i] >= '0' && p[i] <= '9')
				break;
		}

		if(i == len)
		{
			printf("%s: Not Found Number\n", __func__);
			return -1;
		}

		p = p + i;
		len = strlen(p);
		if(len < 6)
		{
			printf("%s: Error: mcc and mnc is too short. len = %d\n", __func__, len);
			return -1;
		}

		for(i = 0; i < 6; i++)
		{
			if(p[i] < '0' || p[i] > '9')
			{
				printf("%s: Error: mcc and mnc are NOT numbers\n", __func__);
				return -1;
			}
		}

		tmp_len = sizeof(tmp);
		memset(tmp, 0, tmp_len);
		memcpy(tmp, p, 3);
		mcc = atoi(tmp);

		memset(tmp, 0, tmp_len);
		memcpy(tmp, p + 3, 2);
		mnc = atoi(tmp);

		//printf("%s: mcc = %d, mnc = %d\n", __func__, mcc, mnc);

		*p_mcc = mcc;
		*p_mnc = mnc;

		return 0;
	}
	else
		return -1;
}

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
int lte_get_apn(int fd, char **p_dialup_param, char *quirk)
{
	int i, ret, len;
	int mcc, mnc;
	char mcc_str[4], mnc_str[4];
	apn_t *p_apn;
	int retry = 0;
	int max_retry_times = 20; //Maximum retry 20 times, 1 time per 1000 millseconds.

	//Retry read MCC and MNC
	retry = 0;
	while(retry < max_retry_times)
	{
		ret = lte_get_mcc_mnc(fd, &mcc, &mnc, quirk);
		if(ret == 0)
			break;
		else
			printf("%s: Read MCC and MNC failed. retry = %d\n", __func__, retry);

		retry++;
		usleep(1000000);
	}

	if(retry == max_retry_times)
	{
		printf("%s: Reach the maximum retry times(%d). Read MCC and MNC failed. Exit!\n", __func__, retry);
		return -1;
	}
	else
		printf("%s: Successfully Read MCC and MNC. retry = %d\n", __func__, retry);

	len = sprintf(mcc_str, "%03d", mcc);
	mcc_str[len] = 0;

	len = sprintf(mnc_str, "%02d", mnc);
	mnc_str[len] = 0;

	printf("%s: search mcc = %s, mnc = %s\n", __func__, mcc_str, mnc_str);

	p_apn = findApn(mcc_str, mnc_str);

	if(p_dialup_param)
	{
		for(i = 0; i < 3; i++)
			p_dialup_param[i][0] = 0;
	}

	if(strlen(p_apn->apn) > 0)
	{
		printf("apn = %s\n", p_apn->apn);
		if(p_dialup_param)
		{
			len = sprintf(p_dialup_param[0], "%s", p_apn->apn);
			p_dialup_param[0][len] = 0;
		}

		if(strlen(p_apn->user) > 0)
		{
			printf("user = %s\n", p_apn->user);
			if(p_dialup_param)
			{
				len = sprintf(p_dialup_param[1], "%s", p_apn->user);
				p_dialup_param[1][len] = 0;
			}

			if(strlen(p_apn->password) > 0)
			{
				printf("password = %s\n", p_apn->password);
				if(p_dialup_param)
				{
					len = sprintf(p_dialup_param[2], "%s", p_apn->password);
					p_dialup_param[2][len] = 0;
				}
			}
		}
	}
	else
	{
		printf("%s: Not Found APN for mcc = %s, mnc = %s\n", __func__, mcc_str, mnc_str);
	}

	freeApn();

	return 0;
}

/***************************************************************************************************
Function   : lte_get_imei
Description: Send AT command "AT+GSN" to read the IMEI number of the SIM card.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_imei: The IMEI number of the SIM card.
             imei_len: The length of the buffer that store the IMEI number.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_imei(int fd, char *p_imei, int imei_len)
{
	int i, ret, len;
	char buf[64], *p;

	memset(p_imei, 0, imei_len);

	ret = send_at_command(fd, "ATE", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
	if(ret != 0)
		return ret;

	ret = send_at_command(fd, "AT+GSN", 1000, 0, "OK", "ERROR", "AT+GSN", buf, sizeof(buf));

	if(ret != 0)
		return -1;

	p = buf;

	p += strlen("AT+GSN");
	len = strlen(p);

	//Skip '\r', '\n', space and tab key
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n' && p[i] != 0x20 && p[i] != 0x09)
			break;
		//else
		//	printf("%s: p[%d] = 0x%02x\n", __func__, i, p[i]);
	}

	//printf("%s: i = %d\n", __func__, i);

	if(i == len)
	{
		printf("%s: Not Found IMEI\n", __func__);
		return -1;
	}

	p = p + i;
	len = strlen(p);

	if(len > imei_len)
		len = imei_len;

	//Found '\r', '\n'
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n')
			p_imei[i] = p[i];
		else
			break;
	}

	len = strlen(p_imei);
	//printf("%s: strlen(p_imei) = %d\n", __func__, len);
	if(len < 15)
	{
		printf("%s:Warning: IMEI's length (%d) < 15\n", __func__, len);
	}

	return 0;
}

/***************************************************************************************************
Function   : lte_get_iccid
Description: Send AT command "AT+ICCID" to read the ICCID number of the SIM card.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_ccid: The ICCID number of the SIM card.
             iccid_len: The length of the buffer that store the ICCID nmber.
Return     :  0: Success
             <0: Fail
***************************************************************************************************/
int lte_get_iccid(int fd, char *p_iccid, int iccid_len)
{
	int i, ret, len;
	char buf[64], *p;

	memset(p_iccid, 0, iccid_len);

	ret = send_at_command(fd, "ATE", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
	if(ret != 0)
		return ret;

	ret = send_at_command(fd, "AT+ICCID", 1000, 0, "OK", "ERROR", "AT+ICCID", buf, sizeof(buf));

	if(ret != 0)
		return -1;

	p = buf;

	p += strlen("AT+ICCID");
	len = strlen(p);

	//Skip '\r', '\n', space and tab key
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n' && p[i] != 0x20 && p[i] != 0x09)
			break;
		//else
		//	printf("%s: p[%d] = 0x%02x\n", __func__, i, p[i]);
	}

	//printf("%s: i = %d\n", __func__, i);

	if(i == len)
	{
		printf("%s: Not Found ICCID\n", __func__);
		return -1;
	}

	p = p + i;
	len = strlen(p);

	if(len > iccid_len)
		len = iccid_len;

	char subbuff[iccid_len];
	memset(subbuff, 0, iccid_len);

	//Found '\r', '\n'
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n')
			subbuff[i] = p[i];
		else
			break;
	}

	memcpy(p_iccid, &subbuff[7], len-7);
	//subbuff[iccid_len-7] = '\0';

	return 0;
}

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
int lte_get_fw_version(int fd, char *p_version, int version_len)
{
	int i, ret, len;
	char buf[64], *p;

	memset(p_version, 0, version_len);

	ret = send_at_command(fd, "ATE", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
	if(ret != 0)
		return ret;

	ret = send_at_command(fd, "AT+QGMR", 1000, 0, "OK", "ERROR", "AT+QGMR", buf, sizeof(buf));

	if(ret != 0)
		return -1;

	p = buf;

	p += strlen("AT+QGMR");
	len = strlen(p);

	//Skip '\r', '\n', space and tab key
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n' && p[i] != 0x20 && p[i] != 0x09)
			break;
		//else
		//	printf("%s: p[%d] = 0x%02x\n", __func__, i, p[i]);
	}

	//printf("%s: i = %d\n", __func__, i);

	if(i == len)
	{
		printf("%s: Not Found FW Version\n", __func__);
		return -1;
	}

	p = p + i;
	len = strlen(p);

	if(len > version_len)
		len = version_len;

	//Found '\r', '\n'
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n')
			p_version[i] = p[i];
		else
			break;
	}

	//printf("%s: strlen(p_version) = %d\n", __func__, strlen(p_version));

	return 0;
}

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
int lte_get_fw_simple_version(int fd, char *p_version, int version_len)
{
	int i, ret, len;
	char buf[64], *p;

	memset(p_version, 0, version_len);

	ret = send_at_command(fd, "ATE", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
	if(ret != 0)
		return ret;

	ret = send_at_command(fd, "AT+GMR", 1000, 0, "OK", "ERROR", "AT+GMR", buf, sizeof(buf));

	if(ret != 0)
		return -1;

	p = buf;

	p += strlen("AT+GMR");
	len = strlen(p);

	//Skip '\r', '\n', space and tab key
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n' && p[i] != 0x20 && p[i] != 0x09)
			break;
		//else
		//	printf("%s: p[%d] = 0x%02x\n", __func__, i, p[i]);
	}

	//printf("%s: i = %d\n", __func__, i);

	if(i == len)
	{
		printf("%s: Not Found FW Version\n", __func__);
		return -1;
	}

	p = p + i;
	len = strlen(p);

	if(len > version_len)
		len = version_len;

	//Found '\r', '\n'
	for(i = 0; i < len; i++)
	{
		if(p[i] != '\r' && p[i] != '\n')
			p_version[i] = p[i];
		else
			break;
	}

	//printf("%s: strlen(p_version) = %d\n", __func__, strlen(p_version));

	return 0;
}

#if 0
#define PWRKEY_LTE 77
#define RESETn_LTE 86

/***************************************************************************************************
Function   : lte_reset
Description: Pull the reset pin of the LTE module to reset the LTE module.
Parameter  : None
Return     : None
***************************************************************************************************/
void lte_reset(void)
{
	nvt_gpio_set_direction(RESETn_LTE,1);

	nvt_gpio_output(RESETn_LTE,0);
	usleep(100000);
	nvt_gpio_output(PWRKEY_LTE,1);
	usleep(300000);  // >= 150ms and <= 460ms
	nvt_gpio_output(PWRKEY_LTE,0);
}

/***************************************************************************************************
Function   : lte_poweroff
Description: Pull the power pin of the LTE module to power off the LTE module.
Parameter  : None
Return     : None
***************************************************************************************************/
void lte_poweroff(void)
{
    nvt_gpio_set_direction(PWRKEY_LTE,1);

    nvt_gpio_output(PWRKEY_LTE,0);
    usleep(100000);
    nvt_gpio_output(PWRKEY_LTE,1);
    usleep(700000); // >= 650ms
    nvt_gpio_output(PWRKEY_LTE,0);
}

/***************************************************************************************************
Function   : lte_poweron
Description: Pull the power pin and the reset pin of the LTE module to power on the LTE module.
Parameter  : None
Return     : None
***************************************************************************************************/
void lte_poweron(void)
{
    nvt_gpio_set_direction(PWRKEY_LTE,1);
    nvt_gpio_set_direction(RESETn_LTE,1);

    nvt_gpio_output(PWRKEY_LTE,1);
    nvt_gpio_output(RESETn_LTE,1);
    usleep(300000);
    nvt_gpio_output(PWRKEY_LTE,0);
    nvt_gpio_output(RESETn_LTE,0);

    //Make sure that VBAT is stable before pulling high PWRKEY pin.
    //The time between them should be no less than 30ms.
    usleep(100000); // no less than 30ms
    nvt_gpio_output(PWRKEY_LTE,1);
    usleep(600000); // >= 500ms
    nvt_gpio_output(PWRKEY_LTE,0);
}


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
int lte_suspend(int fd, const char *serverIp, int sPort)
{
    char buf[100], *recvs;
    int i=0, ret=0, index = 5, loop = 3;
    char qiOpenCmd[100];

    if( serverIp == NULL || sPort == 0 )
        return -1;

    ret = send_at_command(fd, "AT+QISTATE=1,0", 1000, 0, "OK", "ERROR", "QISTATE:", buf, sizeof(buf));
    if( ret == 0 ){
        recvs = buf;
        while( index-- && strsep(&recvs, ",") );
        if( recvs[0]=='2' ){
            return 0;
        }
    }

    while(loop--){
        ret = send_at_command(fd, "AT+QSCLK=1", 1000, 0, "OK", "ERROR", "AT+QSCLK", buf, sizeof(buf));
        ret = send_at_command(fd, "AT+QURCCFG=\"urcport\",\"usbat\"", 1000, 0, "OK", "ERROR", "AT+QURCCFG", buf, sizeof(buf));
        ret = send_at_command(fd, "AT+QCFG=\"apready\",1,0,200", 1000, 0, "OK", "ERROR", "AT+QCFG", buf, sizeof(buf));
        ret = send_at_command(fd, "AT+QCFG=\"risignaltype\",\"physical\"", 1000, 0, "OK", "ERROR", "AT+QCFG", buf, sizeof(buf));
        ret = send_at_command(fd, "AT+QICSGP=1", 1000, 0, "OK", "ERROR", "AT+QICSGP", buf, sizeof(buf));
        ret = send_at_command(fd, "AT+QIACT=1", 1000, 0, "OK", "ERROR", "AT+QIACT", buf, sizeof(buf));
        if( ret == 0 ){
            snprintf(qiOpenCmd, 100, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1", serverIp, sPort);
            ret = send_at_command(fd, qiOpenCmd, 1000, 0, "OK", "ERROR", "AT+QIOPEN", buf, sizeof(buf));
            while( i++ < 3 ){
                usleep(200000);
                ret = send_at_command(fd, "AT+QISTATE=1,0", 1000, 0, "OK", "ERROR", "QISTATE:", buf, sizeof(buf));
                if( ret == 0 ){
                    index=5;
                    recvs = buf;
                    while( index-- && strsep(&recvs, ",") );
                    if( recvs[0]=='2' ){
                        return 0;
                    }
                }
            }
            if( i<=3 ) return -1;
        }else {
            lte_reset();
            usleep(30000);
        }
    }

    return ret;
}

static int lte_check_power_on(void)
{
	int count;

	//Check for 25 seconds
	count = 0;

	while(1)
	{
		//If the /dev/ttyUSB2 appear, it means the LTE is initialized successfully.
		if ((access(TTY_DEVICE, F_OK)) == 0)
		{
			printf("lte power on success\n");
			break;
		}

		//If the LTE is still off at the 15th seconds, reset it.
		if (count == 15)
		{
			printf("lte reset\n");
			lte_reset();
		}

		if (count == 25)
			break;

		count++;

		sleep(1);
	}

	if(count == 25)
	{
		printf("lte power on failed\n");
		return -1;
	}

	return 0;
}

static int lte_check_power_off(void)
{
	int count;

	//Check for 35 seconds
	count = 0;

	while (1)
	{
		//If the /dev/ttyUSB2 disappear, it means the LTE power off successfully.
		if((access(TTY_DEVICE, F_OK)) != 0)
		{
			printf("lte power off success\n");
			break;
		}

		//If the LTE is still on at the 30th seconds, reset it.
		if(count == 30)
		{
			printf("lte reset\n");
			lte_reset();
		}

		if(count == 35)
			break;

		count++;

		sleep(1);
	}

	if(count == 35)
	{
		printf("lte power off failed\n");
		return -1;
	}

	return 0;
}

static int save_lte_info_to_tmp_folder(void)
{
	int ret, fd, times, len;
	char lte_module_name[7] = {0}, data[64], buf[80] = {0};

	fd = at_open_uart();
	if(fd < 0)
	{
		printf("[%s] Can NOT Open %s.\n", __func__, TTY_DEVICE);
		return -1;
	}

	//Get FW Versioin. Try 3 times.
	times = 0;
	while(1)
	{
		ret = lte_get_fw_version(fd, data, sizeof(data));
		if(ret == 0)
			break;
		else
		{
			times++;
			printf("[%s]Error: lte_get_fw_version failed. times = %d\n", __func__, times);
		}

		if(times == 3)
			break;

		sleep(1);
	}

	if(times == 3)
	{
		printf("[%s] failed.\n", __func__);
		close(fd);
		return -1;
	}

	len = 0;
	if(strlen(data) >= 6)
	{
		strncpy(lte_module_name, data, 6);
		len += sprintf(buf + len, "LTE_MODULE=%s\n", lte_module_name);
		buf[len] = 0;
	}
	else
	{
		printf("[%s]Error: couldn't get lte module name\n", __func__);
		return -1;
	}

	len += sprintf(buf + len, "FW_VERSION=%s\n", data);

	//Get IMEI. Try 3 times.
	times = 0;
	while(1)
	{
		ret = lte_get_imei(fd, data, sizeof(data));
		if(ret == 0)
			break;
		else
		{
			times++;
			printf("[%s]Error: lte_get_imei failed. times = %d\n", __func__, times);
		}

		if(times == 3)
			break;

		sleep(1);
	}

	if(times == 3)
	{
		printf("[%s] failed.\n", __func__);
		close(fd);
		return -1;
	}

	len += sprintf(buf + len, "IMEI=%s\n", data);

	close(fd);

	ret = write_string_to_file(buf, "/tmp/lte_info");
	if(ret)
	{
		printf("[%s]Error: Generate /tmp/lte_version failed.\n", __func__);
		return -2;
	}


	len = sprintf(buf, "LTE_MODULE=%s\n", lte_module_name);
	buf[len] = 0;
	ret = write_string_to_file(buf, "/tmp/.lte_existed");
	if(ret)
	{
		printf("[%s]Error: Generate /tmp/.lte_existed failed.\n", __func__);
		return -2;
	}

	printf("[%s] success.\n", __func__);

	return 0;
}

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
int lte_get_CTZU(int fd, int *p_ctzu)
{
	int i, ret, len;
	char buf[64], tmp[64] = {0}, *p;

	*p_ctzu = -1;

	ret = send_at_command(fd, "AT+CTZU?", 1000, 0, "OK", "ERROR", "+CTZU: ", buf, sizeof(buf));
	if(ret)
		return ret;

	p = buf;

	p += strlen("+CTZU: ");
	len = strlen(p);

	for(i = 0; i < len; i++)
	{
		if(p[i] >= '0' && p[i] <= '9')
			tmp[i] = p[i];
		else
			break;
	}

	len = strlen(tmp);
	if(len == 0)
		return -21;

	*p_ctzu = atoi(tmp);

	return 0;
}

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
int lte_set_CTZU(int fd, int ctzu)
{
	int ret;
	char at_cmd[16] = {0};

	sprintf(at_cmd, "AT+CTZU=%d", ctzu);

	ret = send_at_command(fd, at_cmd, 1000, 0, "OK", "ERROR", at_cmd, NULL, 0);
	return ret;
}

#endif

/***************************************************************************************************
Function   : lte_set_cmd
Description: Send an AT command to the LTE module.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             p_cmd: The AT command.
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_set_cmd(int fd, char *p_cmd)
{
	int ret;

	ret = send_at_command(fd, "ATE", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
	if(ret != 0)
		return ret;

	ret = send_at_command(fd, p_cmd, 1000, 0, "OK", "ERROR", p_cmd, NULL, 0);

	return ret;
}

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
int lte_get_tx_power(int fd, char *p_cmd, char *p_value, int value_buf_len)
{
	int i, ret, len, tmplen;
	char buf[64], tmp[64] = {0}, *p;

	memset(p_value, 0, value_buf_len);

	if(strstr(p_cmd, "AT+QNVFR"))
		ret = send_at_command(fd, p_cmd, 1000, 0, "OK", "ERROR", "+QNVFR: ", buf, sizeof(buf));
	else if(strstr(p_cmd, "AT+QNVR"))
		ret = send_at_command(fd, p_cmd, 1000, 0, "OK", "ERROR", "+QNVR: ", buf, sizeof(buf));
	else {
		printf("%s: Not Support command: %s\n", __func__, p_cmd);
		return -99;
	}

	if(ret)
		return ret;

	p = buf;

	if(strstr(p_cmd, "AT+QNVFR"))
		p += strlen("+QNVFR: ");
	else
		p += strlen("+QNVR: ");

	len = strlen(p);
	tmplen = sizeof(tmp);

	for(i = 0; i < len && i < tmplen - 1; i++)
	{
		if(p[i] != '\r' && p[i] != '\n')
			tmp[i] = p[i];
		else
			break;
	}

	len = strlen(tmp);
	for(i = 0; i < len && i < value_buf_len - 1; i++)
		p_value[i] = tmp[i];

	return 0;
}

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
/*
root@NVTEVM:~$ microcom /dev/ttyUSB2
AT+CCLK?
+CCLK: "80/01/06,00:49:25+32"

OK
*/
int lte_get_timezone_quarters(int fd, int *p_quarters)
{
	int i, ret, len, negative;
	char buf[64], tmp[64] = {0}, *p, *p_tmp;

	*p_quarters = 0;
	negative = 0;

	ret = send_at_command(fd, "AT+CCLK?", 1000, 0, "OK", "ERROR", "+CCLK: ", buf, sizeof(buf));
	if(ret)
		return ret;

	p = buf;

	p += strlen("+CCLK: ");
	p_tmp = strstr(p, "+");
	if(p_tmp == NULL)
	{
		p_tmp = strstr(p, "-");
		if(p_tmp == NULL)
		{
			printf("[%s]Error: Not found the character \"+\" or \"-\" in the returned string of \"AT+CCLK?\" command.\n", __func__);
			return -20;
		}
		else
			negative = 1;
	}

	p = p_tmp;

	p++;
	len = strlen(p);

	for(i = 0; i < len; i++)
	{
		if(p[i] >= '0' && p[i] <= '9')
			tmp[i] = p[i];
		else
			break;
	}

	len = strlen(tmp);
	if(len == 0)
		return -21;

	if(negative == 0)
		*p_quarters = atoi(tmp);
	else
		*p_quarters = atoi(tmp) * -1;

	return 0;
}

/***************************************************************************************************
Function   : lte_convert_timezone_quarters_to_string
Description: Convert the time zone to string.
             For example: The time zone quarters is 32. Covert it to string as GMT+08:00
Parameter  : quarters: The time zone expressed in quarters.
			 tz_buf: The time zone string.
			 tz_len: The length of the time zone string.
Return     : None
***************************************************************************************************/
void lte_convert_timezone_quarters_to_string(int quarters, char *tz_buf, int tz_len)
{
	memset(tz_buf, 0, tz_len);

	sprintf(tz_buf, "GMT%c%02d:%02d", quarters >= 0 ? '+':'-', quarters / 4, (quarters % 4) * 15);
}

#if 0
/***************************************************************************************************
Function   : lte_check_ip_valid
Description: Check whether the IP address of LTE is valid.
Parameter  : None
Return     : 0: The IP address is valid.
            <0: The IP address is NOT valid.
***************************************************************************************************/
int lte_check_ip_valid(void)
{
	int ret;
	unsigned int ipaddr;

	ret = get_ip_addr("wwan0", &ipaddr);
	if (ret)
		return -1;

	//Invalid IP Address: 0.0.0.0 and 169.254.xx.xx
	if (ipaddr == 0 || (((ipaddr & 0xFF) == 0xA9) && (((ipaddr >> 8)) & 0xFF) == 0xFE))
		return -1;

	return 0;
}

/***************************************************************************************************
Function   : lte_init
Description: Initialize the LTE module. It power on or reset the LTE module to make the LTE module work well.
Parameter  : None
Return     : 0: Success
            <0: Fail
***************************************************************************************************/
int lte_init(void)
{
	int fd, ret, count;

	//Check whether the node (/dev/ttyUSB2) of LTE module exist. Check 2 times.
	count = 0;

	while(1)
	{
		if(access(TTY_DEVICE, F_OK) != 0)
		{
			if(count == 0)
				printf("[%s] Not Found %s.\n", __func__, TTY_DEVICE);
		}
		else
			break;

		count++;

		if(count == 2)
			break;

		sleep(1);
	}

	//If not found /dev/ttyUSB2, it means the LTE is powered off. we power on the LTE.
	if(count == 2)
	{
		printf("[%s] LTE power on.\n", __func__);
		lte_poweron();

		ret = lte_check_power_on();
		if(ret)
		{
			printf("[%s] LTE init failed.\n", __func__);
			return -1;
		}
		else
		{
			save_lte_info_to_tmp_folder();
			printf("[%s] LTE init success.\n", __func__);
			return 0;
		}
	}

	//If get LTE info failed, it means the LTE module doesn't work normally. we need restart the LTE module.
	ret = save_lte_info_to_tmp_folder();
	if(ret == -1)
	{
		printf("[%s] LTE power off because getting lte info failed.\n", __func__);
		lte_poweroff();

		ret = lte_check_power_off();
		if(ret)
		{
			printf("[%s] LTE init failed.\n", __func__);
			return -3;
		}

		printf("[%s] LTE power on.\n", __func__);
		lte_poweron();

		ret = lte_check_power_on();
		if(ret)
		{
			printf("[%s] LTE init failed.\n", __func__);
			return -1;
		}
		else
			printf("[%s] LTE init success.\n", __func__);

		save_lte_info_to_tmp_folder();

		return 0;
	}
	//If found the /dev/ttyUSB2, it means the LTE isn't powered off by MCU.
	//Because the hardware doesn't support the SIM card hotplug, if the customer hotplug the SIM card, we need restart the LTE module to identify the SIM card.

	//Check whther the SIM card is inserted. Check 2 times.
	fd = at_open_uart();
	if(fd < 0)
	{
		printf("[%s] Can NOT Open %s.\n", __func__, TTY_DEVICE);
		printf("[%s] LTE init failed.\n", __func__);
		return -2;
	}

	count = 0;

	while(1)
	{
		ret = lte_check_sim_card_exist(fd);
		if(ret != 0)
		{
			if(count == 0)
			{
				printf("[%s] SIM is NOT inserted.\n", __func__);
			}
		}
		else
		{
			printf("[%s] SIM is inserted.\n", __func__);
			break;
		}

		count++;

		if(count == 2)
			break;

		sleep(1);
	}

	close(fd);

	//If the SIM card is found, it means the customer doesn't hot pulg the SIM card.
	if(count < 2)
	{
		printf("[%s] LTE init success.\n", __func__);
		return 0;
	}

	//If the SIM card is not found, it means the customer may have hot-plugged the SIM card, we need restart the LTE module to identify the SIM card.
	//We power off the LTE, and then power on the LTE for restarting the LTE.

	printf("[%s] LTE power off.\n", __func__);
	lte_poweroff();

	ret = lte_check_power_off();
	if(ret)
	{
		printf("[%s] LTE init failed.\n", __func__);
		return -3;
	}

	printf("[%s] LTE power on.\n", __func__);
	lte_poweron();

	ret = lte_check_power_on();
	if(ret)
	{
		printf("[%s] LTE init failed.\n", __func__);
		return -1;
	}
	else
		printf("[%s] LTE init success.\n", __func__);

	return 0;
}

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
int lte_set_antenna(int fd, LTE_ANTENNA_TYPE type)
{
	int i, j, ret, len;
	char buf[128], tmp[128], *p;
	switch(type){
		case ANTENNA_ENABLE_BOTH:
		ret = send_at_command(fd, "AT+QNVFW=\"/nv/item_files/modem/lte/ML1/rx_tuning_chan\",00", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
		break;
		case ANTENNA_ENABLE_MAIN:
		ret = send_at_command(fd, "AT+QNVFW=\"/nv/item_files/modem/lte/ML1/rx_tuning_chan\",06", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
		break;
		case ANTENNA_ENABLE_BACKUP:
		ret = send_at_command(fd, "AT+QNVFW=\"/nv/item_files/modem/lte/ML1/rx_tuning_chan\",05", 1000, 0, "OK", "ERROR", NULL, NULL, 0);
		break;
		case ANTENNA_QUERY_SETTING:
		ret = send_at_command(fd, "AT+QNVFR=\"/nv/item_files/modem/lte/ML1/rx_tuning_chan\"", 1000, 0, "OK", "ERROR", "AT+QNVFR=", buf, sizeof(buf));
		break;
		default:
		return -1;
	}
	if(ret == 0)
	{
		ret = send_at_command(fd, "AT+QNVFR=\"/nv/item_files/modem/lte/ML1/rx_tuning_chan\"", 1000, 0, "OK", "ERROR", "AT+QNVFR=", buf, sizeof(buf));
		printf("%s buf = %s \n", __func__, buf);

		//format example:
		//
		//AT+QNVFR="/nv/item_files/modem/lte/ML1/rx_tuning_chan"
		//+QNVFR: 06
		//
		//OK

		if(ret == 0)
		{
			p = buf;

			p += strlen("AT+QNVFR=\"/nv/item_files/modem/lte/ML1/rx_tuning_chan\"");
			len = strlen(p);

			memset(tmp ,0 , 128);
			j = 0;
			for(i = 0; i < len; i++)
			{
				if(p[i] >= '0' && p[i] <= '9')
				{
					tmp[j] = p[i];
					j++;
				}
			}
			if(strlen(tmp) == 0)
			{
				printf("%s: lte_set_antenna fail!\n", __func__);
				return -1;
			}

			switch(type){
				case ANTENNA_ENABLE_BOTH:
				if(strcmp(tmp, "00") == 0)
				{
					printf("%s: ANTENNA_ENABLE_BOTH!\n", __func__);
					return 0;
				}
				break;
				case ANTENNA_ENABLE_MAIN:
				if(strcmp(tmp, "06") == 0)
				{
					printf("%s: ANTENNA_ENABLE_MAIN!\n", __func__);
					return 0;
				}
				break;
				case ANTENNA_ENABLE_BACKUP:
				if(strcmp(tmp, "05") == 0)
				{
					printf("%s: ANTENNA_ENABLE_BACKUP!\n", __func__);
					return 0;
				}
				break;

				default:
					break;
			}
			return -1;
		}

		return ret;
	}
	else
		return ret;
}
#endif

} // extern "C"

