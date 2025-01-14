/*
 * Copyright (c) 2024 VIA Technologies, Inc. All Rights Reserved.
 *
 * This PROPRIETARY SOFTWARE is the property of VIA Technologies, Inc.
 * and may contain trade secrets and/or other confidential information of
 * VIA Technologies, Inc. This file shall not be disclosed to any
 * third party, in whole or in part, without prior written consent of VIA.
 *
 * THIS PROPRIETARY SOFTWARE AND ANY RELATED DOCUMENTATION ARE PROVIDED
 * AS IS, WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND EITHER EXPRESS
 * OR IMPLIED, AND VIA TECHNOLOGIES, INC. DISCLAIMS ALL EXPRESS
 * OR IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * QUIET ENJOYMENT OR NON-INFRINGEMENT.
 */
#define KEY_HANDLER_C

#include <stdio.h>      /* for printf() */
#include <stdlib.h>
#include <unistd.h>     /* for sleep() */
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>

#include "ViaKeyHandler.h"


int main(int argc, char **argv)
{
    ViaKeyHandler vkh;
    vkh.KeyEventHandler();
    return 0;
}

#undef KEY_HANDLER_C
