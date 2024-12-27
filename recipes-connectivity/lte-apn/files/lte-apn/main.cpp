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
#include "xmlparser.h"


#define has_more_argv() ((opt < argc) && (argv[opt][0] != '-'))
int main(int argc, char *argv[])
{
    char *xml_file = DEFAULT_XML_FILE;
    char mcc_str[4], mnc_str[4];
    apn_t *p_apn;

    printf("[%s] start!\n", argv[0]);

    if (argc == 2) {
        xml_file = argv[1];
    }

    strcpy(mcc_str, "466");
    strcpy(mnc_str, "92");

    printf("[%s] search mcc = %s, mnc = %s\n", argv[0], mcc_str, mnc_str);

    p_apn = findApn(mcc_str, mnc_str);

    return 0;
}
