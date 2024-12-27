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
#include <pthread.h>

//#include "nvtgpio.h"
#include "at_tok.h"


static char s_tty_device[256]= {0};
static pthread_mutex_t s_at_cmd_lock = PTHREAD_MUTEX_INITIALIZER;
static int s_show_at_cmd_log = 0;

extern "C" {

/***************************************************************************************************
Function   : at_open_uart
Description: Open TTY device for AT commands
Parameter  : ttyDevice: The TTY device file for opening, if NULL, open /dev/ttyUSB2
Return     : The file handler of ttyDevice
***************************************************************************************************/
int at_open_uart(char *ttyDevice)
{
	int fd;
	struct termios cfg;

	if(ttyDevice != NULL)
	{
		strncpy(s_tty_device, ttyDevice, sizeof(s_tty_device)-1);
	}
	else
	{
		strcpy(s_tty_device, TTY_DEVICE);
	}

	if((access(s_tty_device, F_OK)) == -1)
	{
		printf("%s: %s does not exist\n", __func__, s_tty_device);
		return -1;
	}

	if(s_show_at_cmd_log)
		printf("%s: %s\n", __func__, s_tty_device);

	fd = open(s_tty_device, O_RDWR);
	if (fd < 0)
	{
		printf("%s: open %s error\n", __func__, s_tty_device);
		return -1;
	}
	else
	{
		if(s_show_at_cmd_log)
			printf("%s: open %s success, fd = %d\n", __func__, s_tty_device, fd);
	}

 	if(isatty(fd))
	{
		if(tcgetattr(fd, &cfg))
		{
			printf("%s: tcgetattr() failed. fd = %d\n", __func__, fd);
			close(fd);
			return -1;
		}

		cfmakeraw(&cfg);
		cfsetispeed(&cfg, TTY_SPEED);
		cfsetospeed(&cfg, TTY_SPEED);

		if (tcsetattr(fd, TCSANOW, &cfg))
		{
			printf("%s: tcsetattr() failed. fd = %d\n", __func__, fd);
			close(fd);
			return -1;
		}
 	}
	else
	{
		printf("%s: %s is not an isatty. fd = %d\n", __func__, s_tty_device, fd);
		close(fd);
		return -1;
	}

	return fd;
}

/***************************************************************************************************
Function   : at_close_uart
Description: Close the file handler of /dev/ttyUSB2 opened by at_open_uart()
Parameter  : fd: The file handler of /dev/ttyUSB2
Return     : None
***************************************************************************************************/
void at_close_uart(int fd)
{
	if(s_show_at_cmd_log)
    	printf("%s\n", __func__);

    if (fd >= 0)
	{
        close(fd);
        fd  = -1;
    }
}

/***************************************************************************************************
Function   : send_at_command
Description: Send an AT command to LTE module and receive the response and data from LTE module.
Parameter  : fd: The file handler of /dev/ttyUSB2.
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
int send_at_command(int fd, const char *cmd, int max_response_ms, int delay_ms, const char *ok_str, const char *error_str, const char *expect_str, char *rcv_str, int rcv_len)
{
	int len, ret;
	int sec, usec;
	char buf[512], *p;
	struct timeval timeout;
	fd_set fds;
	int rcv_times;

	pthread_mutex_lock(&s_at_cmd_lock);

	len = sprintf(buf, "%s\r", cmd);
	buf[len] = 0;

	ret = write(fd, buf, len);
	if(ret != len)
	{
		printf("%s: %s. Fail to write cmd. Exit!\n", __func__, cmd);
		pthread_mutex_unlock(&s_at_cmd_lock);
		return -3;
	}

	sec = max_response_ms / 1000;
	usec = (max_response_ms % 1000) * 1000;

	rcv_times = 100;
	len = 0;
	memset(buf, 0, sizeof(buf));

	while(rcv_times)
	{
		timeout.tv_sec = sec;
		timeout.tv_usec = usec;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;

			printf("%s: %s. select(): %s. Exit!\n", __func__, cmd, strerror(errno));
			pthread_mutex_unlock(&s_at_cmd_lock);
			return -4;
		}
		else if(ret == 0)
		{
			printf("%s: %s. select() timeout! Exit!\n", __func__, cmd);
			pthread_mutex_unlock(&s_at_cmd_lock);
			return -5;
		}

		read(fd, buf + len, sizeof(buf) - len);
		len = strlen(buf);

		if(strstr(buf, ok_str))
		{
			if(delay_ms)
			{
				sec = delay_ms / 1000;
				usec = (delay_ms % 1000) * 1000;

				timeout.tv_sec = sec;
				timeout.tv_usec = usec;
				FD_ZERO(&fds);
				FD_SET(fd, &fds);

				while(1)
				{
					ret = select(fd + 1, &fds, NULL, NULL, &timeout);
					if(ret < 0)
					{
						if(errno == EINTR)
							continue;
					}
					else if(ret == 0)
					{
						printf("%s: %s. delay select() timeout!\n", __func__, cmd);
						break;
					}

					read(fd, buf + len, sizeof(buf) - len);
					len = strlen(buf);
					break;
				}
			}

			break;
		}
		else if(strstr(buf, error_str))
		{
			if(s_show_at_cmd_log)
				printf("%s: %s. Error.\n", __func__, cmd);
			pthread_mutex_unlock(&s_at_cmd_lock);
			return -6;
		}

		usleep(100000);
		rcv_times--;
	}

	if(rcv_times == 0)
	{
		printf("%s: %s. Receive LTE data too often. Exit!\n", __func__, cmd);
		pthread_mutex_unlock(&s_at_cmd_lock);
		return -7;
	}

	if(expect_str == NULL)
	{
		if(s_show_at_cmd_log)
			printf("%s: %s. OK.\n", __func__, cmd);

		if(rcv_str != NULL)
		{
			p = buf;
			len = strlen(p);
			if(len >= rcv_len)
				len = rcv_len - 1;

			strncpy(rcv_str, p, len);
			rcv_str[len] = 0;
			if(s_show_at_cmd_log)
				printf("%s: %s. rcv_len = %d, rcv_str: \"%s\".\n", __func__, cmd, len, rcv_str);
		}

		pthread_mutex_unlock(&s_at_cmd_lock);
		return 0;
	}
	else
	{
		p = strstr(buf, expect_str);
		if(p)
		{
			if(s_show_at_cmd_log)
				printf("%s: %s. Find \"%s\".\n", __func__, cmd, expect_str);

			if(rcv_str != NULL)
			{
				len = strlen(p);
				if(len >= rcv_len)
					len = rcv_len - 1;

				strncpy(rcv_str, p, len);
				rcv_str[len] = 0;
				if(s_show_at_cmd_log)
					printf("%s: %s. rcv_len = %d, rcv_str: \"%s\".\n", __func__, cmd, len, rcv_str);
			}

			pthread_mutex_unlock(&s_at_cmd_lock);
			return 0;
		}
		else
		{
			printf("%s: %s. content start ---\n", __func__, cmd);
			printf("%s", buf);
			printf("%s: %s. content end ---\n", __func__, cmd);
			printf("%s: %s. Not Found \"%s\".\n", __func__, cmd, expect_str);
			pthread_mutex_unlock(&s_at_cmd_lock);
			return -8;
		}
	}

	pthread_mutex_unlock(&s_at_cmd_lock);
	return 0;
}

#if 0
#define AP_READY_GPIO  91

/***************************************************************************************************
Function   : receive_at_urc
Description: Receive the wakeup package from the LTE module when the device is waked up by LTE module.
Parameter  : fd: The file handler of /dev/ttyUSB2.
             max_response_ms: The maximum time that LTE module return the wakeup package.
             rcv_times: The retry times.
             rcv_str: The wakeup package.
             rcv_len: The length of wakeup package.
Return     :  0: Receive the wakeup package successfully
             <0: Error
***************************************************************************************************/
int receive_at_urc(int fd, int max_response_ms, int rcv_times, char *rcv_str, int rcv_len)
{
	int len=0, ret;
	int sec, usec;
	char buf[512], *p;
	struct timeval timeout;
	fd_set fds;

	pthread_mutex_lock(&s_at_cmd_lock);

	sec = max_response_ms / 1000;
	usec = (max_response_ms % 1000) * 1000;

    nvt_gpio_set_direction(AP_READY_GPIO,1);
    nvt_gpio_output(AP_READY_GPIO,1);

	while(rcv_times)
	{
		timeout.tv_sec = sec;
		timeout.tv_usec = usec;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
		if(ret < 0) {
			if(errno == EINTR)
				continue;
			printf("%s: select(): %s. Exit!\n", __func__, strerror(errno));
			pthread_mutex_unlock(&s_at_cmd_lock);
            return -4;
		} else if(ret == 0) {
			printf("%s: select() timeout! Exit!\n", __func__);
		} else {
    		read(fd, buf + len, sizeof(buf) - len);
    		len = strlen(buf);
            if( strstr(buf, "QIURC")){
                break;
            }
        }
		rcv_times--;
	}

	if(rcv_times == 0) {
		printf("%s: Receive timeout. Exit!\n", __func__);
		pthread_mutex_unlock(&s_at_cmd_lock);
		return -7;
	}

	p = strstr(buf, "QIURC");
	if(p) {
		printf("%s: Find \"%s\".\n", __func__, "QIURC");

		if(rcv_str != NULL) {
			len = strlen(p);
			if(len >= rcv_len)
				len = rcv_len - 1;

			strncpy(rcv_str, p, len);
			rcv_str[len] = 0;
			printf("%s: rcv_len = %d, rcv_str: \"%s\".\n", __func__, len, rcv_str);
		}
		pthread_mutex_unlock(&s_at_cmd_lock);
		return 0;
	} else {
		printf("%s: Not Found \"%s\".\n", __func__, "QIURC");
		pthread_mutex_unlock(&s_at_cmd_lock);
		return -8;
	}

	pthread_mutex_unlock(&s_at_cmd_lock);
	return 0;
}
#endif

/***************************************************************************************************
Function   : enable_at_cmd_log
Description: Whether to print the detail log when executing the AT command.
Parameter  : Enable=1: Print log.
             Enable=0: Don't print log.
Return     : None
***************************************************************************************************/
void enable_at_cmd_log(int enable)
{
	s_show_at_cmd_log = enable;
}

} // extern "C"

