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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <viacommon.h>
//#include "lte_msg_handler.h"
#include "lte_api.h"

#define LTE_KILL_PROCESS_TIMEOUT 30      // 30 x 100ms = 3s

static void print_usage(char *cmd)
{
	printf("Usage: %s command [options]\n", cmd);
	printf("command:\n");
	printf("    sim: Check whether the SIM Card is ready\n");
	printf("    signal: Get LTE signal\n");
	printf("    apn: Get APN\n");
	//printf("  dialup: Dial Up\n");
	printf("    imei: Get IMEI String\n");
	printf("    iccid: Get ICCID String\n");
	//printf("    version: Get the full revision of software release\n");
	//printf("    simple_version: Get the simple revision of software release\n");
	//printf("    get_ctzu: Get the CTZU value\n");
	//printf("    set_ctzu [0,1,3]: Set CTZU\n");
	//printf("    get_timezone: Get TimeZone\n");
	//printf("    update_timezone: update timezone to /etc/localtime\n");
	printf("options:\n");
	printf("    -f ttyDevice: TTY device file for AT commands, default is %s\n", TTY_DEVICE);
	printf("    -q vendor: vendor quirk for AT commands\n");
}

#if 0
static int set_ctzu(int fd, int argc, char *argv[])
{
	int ret, current_ctzu, except_ctzu;

	if(argc < 3)
	{
		printf("Error: please input ctzu value\n");
		return -1;
	}

	if(argv[2][0] < '0' || argv[2][0] > '9')
	{
		printf("Error: The input ctzu is not an integer\n");
		return -2;
	}

	except_ctzu = atoi(argv[2]);
	if(except_ctzu != 0 && except_ctzu != 1 && except_ctzu != 3)
	{
		printf("Error: The input ctzu is not 0,1,3\n");
		return -3;
	}

	ret = lte_get_CTZU(fd, &current_ctzu);
	if(ret)
		printf("Fail to get CTZU. ret = %d\n", ret);
	else
	{
		if(current_ctzu == except_ctzu)
		{
			printf("The current CTZU is already %d\n", except_ctzu);
			return 0;
		}
	}

	ret = lte_set_CTZU(fd, except_ctzu);
	if(ret)
	{
		printf("Fail to set CTZU. ret = %d\n", ret);
		return -4;
	}

	ret = lte_get_CTZU(fd, &current_ctzu);
	if(ret)
	{
		printf("Fail to get CTZU for validation. ret = %d\n", ret);
		return -5;
	}

	if(current_ctzu == except_ctzu)
		printf("Set CTZU to %d Success!\n", except_ctzu);
	else
	{
		printf("Set CTZU to %d Failed!\n", except_ctzu);
		return -6;
	}

	return 0;
}

static int generate_read_cmd(char *p_w_cmd, char *p_r_cmd, int r_cmd_buf_len, char *p_r_value, int r_value_buf_len)
{
	int i, len;
	char *p, *q;

//	printf("%s\n", p_cmd);

	memset(p_r_cmd, 0, r_cmd_buf_len);
	memset(p_r_value, 0, r_value_buf_len);

	p = strstr(p_w_cmd, ",");
	q = NULL;

	while(1)
	{
		if(p)
		{
			q = p;
			p++;
			if(*p == 0)
			{
				//printf("last\n");
				break;
			}
			else
				p = strstr(p, ",");
		}
		else
			break;
	}

	p = q;

	if(p != NULL)
	{
		p++;
		if(*p == 0)
			p = NULL;
	}

	if(p == NULL)
	{
		printf("NOT Found ','\n");
		return -1;
	}

	len = p - p_w_cmd - 1;

	for(i = 0; i < len && i < r_cmd_buf_len - 1; i++)
		p_r_cmd[i] =  p_w_cmd[i];

	q = strstr(p_r_cmd, "AT+QNVFW");
	if(q != NULL)
		memcpy(q, "AT+QNVFR", strlen("AT+QNVFW"));

	q = strstr(p_r_cmd, "AT+QNVW");
	if(q != NULL)
		memcpy(q, "AT+QNVR", strlen("AT+QNVR"));

	//printf("%s\n", p);

	len = strlen(p);

	for(i = 0; i < len && i < r_value_buf_len - 1; i++)
		*(p_r_value + i) = *(p + i);

	if(strlen(p_r_value) == 0)
		return -2;

	return 0;
}

static int lte_set_tx_power(int fd, int argc, char *argv[])
{
	int ret;
	char r_cmd[84], r_value[44], buf[44];

	if(argc < 3)
	{
		printf("%s: Error: please input command\n", __func__);
		return -1;
	}

	ret = generate_read_cmd(argv[2], r_cmd, sizeof(r_cmd), r_value, sizeof(r_value));
	if(ret)
	{
		printf("%s: Error: Fail to generate read command\n", __func__);
		return -2;
	}

	ret = lte_get_tx_power(fd, r_cmd, buf, sizeof(buf));
	if(ret == 0)
	{
		if(!strcmp(buf, r_value))
		{
			//printf("%s: Right\n", __func__);
			return 0;
		}
	}

	ret = lte_set_cmd(fd, argv[2]);
	if(ret)
	{
		printf("%s: Fail to set tx power: %s\n", __func__, argv[2]);
		return -3;
	}

	ret = lte_get_tx_power(fd, r_cmd, buf, sizeof(buf));
	if(ret)
	{
		printf("%s: Fail to get tx power for validation: %s\n", __func__, argv[2]);
		return -4;
	}

	if(!strcmp(buf, r_value))
	{
		//printf("%s: Set OK\n", __func__);
		return 1;
	}
	else
	{
		printf("%s: Set \"%s\" Failed. \n", __func__,  argv[2]);
		return -5;
	}
}
#endif

#define has_more_argv() ((opt < argc) && (argv[opt][0] != '-'))
int main(int argc, char *argv[])
{
	int opt = 2;
	int i, ret = 0, fd, len, count;
	char *ttyfilename = NULL;
	char *quirkvendor = NULL;
	char *dialup_param[3], buf[140], quirk[32];
	int need_open_uart;

	if (!strcmp(argv[argc-1], "&"))
		argc--;

	if(argc == 1)
	{
		print_usage(argv[0]);
		return -1;
	}

	//enable_at_cmd_log(1);

	//if(strcmp(argv[1], "reset") && strcmp(argv[1], "poweron") && strcmp(argv[1], "poweroff") && strcmp(argv[1], "init"))
		need_open_uart = 1;
	//else
	//	need_open_uart = 0;

	opt = 2;
	while  (opt < argc) {
		if (argv[opt][0] != '-') {
			print_usage(argv[0]);
			return -1;
		}

		switch (argv[opt++][1])
		{
		case 'f':
			if (has_more_argv())
			{
				ttyfilename = argv[opt++];
			}
		break;

		case 'q':
			if (has_more_argv())
			{
				quirkvendor = argv[opt++];
				strncpy(quirk, quirkvendor, sizeof(quirk));
			}
		break;

		default:
			print_usage(argv[0]);
    			return -1;
		break;
		}
	}

	fd = -1;

	if(need_open_uart)
	{
		fd = at_open_uart(ttyfilename);
		if(fd < 0)
			return -2;
	}

	if(!strcmp(argv[1], "sim"))
	{
		ret = lte_check_sim_card_exist(fd);
		if(ret == 0)
			printf("SIM Ready!\n");
		else
			printf("SIM is Not Ready!\n");
	}
	else if(!strcmp(argv[1], "signal"))
	{
		ret = lte_get_signal_strength(fd);
		if(ret < 0)
			printf("[%s %s] Fail to get lte signal\n", argv[0], argv[1]);
		else
			printf("lte signal = %d\n", ret);
	}
	else if(!strcmp(argv[1], "apn"))
	{
		//Execute command: "quectel-CM -s apn user password &" or "quectel-CM -s apn &" or "quectel-CM &"
		len = 0;

		for(i = 0; i < 3; i++)
		{
			dialup_param[i] = (char *)malloc(60);
			dialup_param[i][0] = 0;
		}

		ret = lte_get_apn(fd, dialup_param, quirk);
		if(ret == 0)
		{
			len += sprintf(buf + len, "+APN: %s,", dialup_param[0]);
			len += sprintf(buf + len, "%s,", dialup_param[1]);
			len += sprintf(buf + len, "%s", dialup_param[2]);
		}

		buf[len] = 0;

		for(i = 0; i < 3; i++)
			free(dialup_param[i]);

		printf("%s\n", buf);
	}
#if 0
	else if(!strcmp(argv[1], "dialup"))
	{
		//Execute command: "quectel-CM -s apn user password &" or "quectel-CM -s apn &" or "quectel-CM &"
		len = 0;
		len += sprintf(buf + len, "quectel-CM");

		for(i = 0; i < 3; i++)
		{
			dialup_param[i] = (char *)malloc(60);
			dialup_param[i][0] = 0;
		}

		ret = lte_get_apn(fd, dialup_param, quirk);
		if(ret == 0)
		{
			if(strlen(dialup_param[0]) > 0) // append apn
			{
				len += sprintf(buf + len, " -s %s", dialup_param[0]);

				if(strlen(dialup_param[1]) > 0) // append user
				{
					len += sprintf(buf + len, " %s", dialup_param[1]);

					if(strlen(dialup_param[2]) > 0) // append password
						len += sprintf(buf + len, " %s", dialup_param[2]);
				}
			}
		}

		len += sprintf(buf + len, " &");

		buf[len] = 0;

		for(i = 0; i < 3; i++)
			free(dialup_param[i]);

		printf("[%s %s] kill quectel-CM\n", argv[0], argv[1]);

		kill_process("quectel-CM");

		count = 0;
		while(count < LTE_KILL_PROCESS_TIMEOUT && check_process_exist("quectel-CM") >= 0)
		{
			usleep(100000);
			count++;
		}

		if(count == LTE_KILL_PROCESS_TIMEOUT)
		{
			printf("[%s %s]Error: Fail to kill quectel-CM\n", argv[0], argv[1]);
			at_close_uart(fd);
			return -1;
		}

		printf("[%s %s] %s\n", argv[0], argv[1], buf);
		system(buf);
	}
#endif
	else if(!strcmp(argv[1], "imei"))
	{
		ret = lte_get_imei(fd, buf, sizeof(buf));
		if(ret == 0)
		{
			printf("%s\n", buf);
		}
		else
		{
			printf("[%s %s]Error: Fail to get IMEI\n", argv[0], argv[1]);
		}
	}
	else if(!strcmp(argv[1], "iccid"))
	{
		ret = lte_get_iccid(fd, buf, sizeof(buf));
		if(ret == 0)
		{
			printf("%s\n", buf);
		}
		else
		{
			printf("[%s %s]Error: Fail to get ICCID\n", argv[0], argv[1]);
		}
	}
#if 0
	else if(!strcmp(argv[1], "version"))
	{
		ret = lte_get_fw_version(fd, buf, sizeof(buf));
		if(ret == 0)
		{
			printf("%s\n", buf);
		}
		else
		{
			printf("[%s %s]Error: Fail to get FW Version\n", argv[0], argv[1]);
		}
	}
	else if(!strcmp(argv[1], "simple_version"))
	{
		ret = lte_get_fw_simple_version(fd, buf, sizeof(buf));
		if(ret == 0)
		{
			printf("%s\n", buf);
		}
		else
		{
			printf("[%s %s]Error: Fail to get FW Version\n", argv[0], argv[1]);
		}
	}
    else if(!strcmp(argv[1], "suspend"))
    {
        if( argc == 4 ){
            lte_suspend(fd, argv[2], atoi(argv[3]));
        }else{
            lte_suspend(fd, "120.24.173.127", 11123);
        }
    }
    else if(!strcmp(argv[1], "geturc"))
    {
        memset(buf,0,sizeof(buf));
        receive_at_urc(fd, 1000, 5, buf, sizeof(buf));
        printf("geturc: %s\r\n",buf);
    }
    else if(!strcmp(argv[1], "reset"))
    {
        lte_reset();
    }
    else if(!strcmp(argv[1], "poweron"))
    {
        lte_poweron();
    }
    else if(!strcmp(argv[1], "poweroff"))
    {
        lte_poweroff();
    }
    else if(!strcmp(argv[1], "init"))
    {
        lte_init();
    }
    else if(!strcmp(argv[1], "get_ctzu"))
    {
    	int ctzu;

        ret = lte_get_CTZU(fd, &ctzu);
        if(ret)
            printf("Fail to get CTZU. ret = %d\n", ret);
        else
            printf("%d\n", ctzu);
    }
    else if(!strcmp(argv[1], "set_ctzu"))
    {
        set_ctzu(fd, argc, argv);
    }
    else if(!strcmp(argv[1], "get_timezone"))
    {
        int quarters;
        char tz_buf[32];

        ret = lte_get_timezone_quarters(fd, &quarters);
        if(ret)
            printf("Fail to get timezone quarters. ret = %d\n", ret);
        else
        {
            lte_convert_timezone_quarters_to_string(quarters, tz_buf, sizeof(tz_buf));
            printf("%s\n", tz_buf);
            generate_timezone_file("/tmp/localtime", quarters, "GMT");
        }
    }
    else if(!strcmp(argv[1], "update_timezone"))
    {
        lte_msg_set_timezone();
    }
	else if(!strcmp(argv[1], "set_tx_power"))
	{
		//lte set_tx_power AT+QNVFW=\"/nv/item_files/rfnv/00020993\",0100F500
		ret = lte_set_tx_power(fd, argc, argv);
		at_close_uart(fd);
		return ret;
	}
	else if(!strcmp(argv[1], "get_tx_power"))
	{
		 //lte get_tx_power AT+QNVFW=\"/nv/item_files/rfnv/00020993\",0100F500
		 if( argc > 2 ){
			ret = lte_get_tx_power(fd, argv[2], buf, sizeof(buf));
			if(ret == 0)
				printf("%s\n", buf);
			else
				printf("Fail to get tx power: %s. ret = %d\n", argv[2], ret);
		 }
		 else
		 	 print_usage(argv[0]);
	}
#endif
    else
        print_usage(argv[0]);

    if(need_open_uart)
    {
        at_close_uart(fd);
    }

    return ret;
}

