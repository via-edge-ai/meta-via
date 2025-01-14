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
#ifndef _XMLPARSER_H
#define _XMLPARSER_H

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

#define DEFAULT_XML_FILE "/etc/apns-conf.xml"
#define TEST_XML_FILE "/data/apns-conf.xml"
#define LARGE_APN_LEN 60
#define MIDDLE_APN_LEN 50
#define SMALL_APN_LEN 8

typedef struct apn_s {
    char carrier[LARGE_APN_LEN];
    char mcc[SMALL_APN_LEN];
    char mnc[SMALL_APN_LEN];
    char apn[MIDDLE_APN_LEN];
    char user[MIDDLE_APN_LEN];
    char password[MIDDLE_APN_LEN];
    char mmsc[LARGE_APN_LEN];
    char mmsproxy[MIDDLE_APN_LEN];
    char mmsport[SMALL_APN_LEN];
    char type[MIDDLE_APN_LEN];

    char server[LARGE_APN_LEN];
    char port[SMALL_APN_LEN];
    char protocol[MIDDLE_APN_LEN];
    char roaming_protocol[MIDDLE_APN_LEN];
}apn_t;

/***************************************************************************************************
Function   : findApn
Description: Find APN from /etc/apns-conf.xml according to MCC and MNC.
             The MCC and MNC are read from SIM card using AT command "AT+CIMI".
Parameter  : mcc: the MCC of SIM card
             mnc: the MNC of SIM card
Return     : apn_t: the apn_t structure which includes apn, user, password for dial-up
***************************************************************************************************/
apn_t* findApn(const char* mcc,const char* mnc);

/***************************************************************************************************
Function   : freeApn
Description: Free the memory allocated by findApn()
Return     : None
***************************************************************************************************/
void freeApn(void);

#endif // _XMLPARSER_H

