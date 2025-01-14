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
#define VIA_KEY_HANDLER_C

#include <stdio.h>      /* for printf() */
#include <stdlib.h>
#include <unistd.h>     /* for sleep() */
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>

#include "ViaKeyHandler.h"

#define  SOUND_CARD   "mt8395evk"   /* For SOM-7000 */
//#define  SOUND_CARD   "mt8390evk"   /* For SOM-5000 */


#define ERROR_HERE   ((-1) * __LINE__)\

#define VOL_UP        1
#define VOL_DOWN      0

#define VOL_MAX       30
#define VOL_MIN       0


struct MsgInfo
{
    long int id;  // the first item must be "long int" for msgsnd
    int  code;
    int  value;
};

enum {
    BTN_UP = 0,
    BTN_DOWN
};

static int SYSTEM(const char *pCmd, bool dump)
{
    char buf[256];
    int err = 0;
    FILE *fp = popen(pCmd, "r");
    if (!fp) {
        printf("*E* \"%s\" failed (errno: %d)\n", pCmd, errno);
        return ERROR_HERE;
    }
//    if (dump)
//        printf("%s\n", pCmd);
    while (fgets(buf, 255, fp) != NULL) {
//        printf("%s", buf);
        if (!strncmp(buf, "  : values=", 11)) {
            int code1 = buf[11] - 48;
            int code2 = buf[12] - 48;
            if ((code1 < 0) || (code1 > 9))
                code1 = 0;
            if ((code2 < 0) || (code2 > 9))
                err = code1;
            else
                err = code1 * 10 + code2;
        }
    }
    pclose(fp);
    return err;
}

void key_notify(void *priv, int code, int value)
{
    ViaKeyHandler *vkh = (ViaKeyHandler *)priv;
    vkh->KeyEventNotify(code, value);
}

ViaKeyHandler::ViaKeyHandler()
{
    printf("\n>>> ViaKeyHandler Start (built: %s %s)\n",
            __DATE__, __TIME__);

    mInputHandler = new InputHandler(this);
    mMsgId = msgget((key_t)1125, 0666 | IPC_CREAT);
    int mWakeFd = open("/sys/power/wakeup_count", O_RDWR);
    if (mWakeFd < 0)
        printf("*E* open \"/sys/power/wakeup_count\" failed!\n");

    if (mMsgId < 0)
        printf("*E* msgget failed (errno: %d)\n", errno);

    SetVolume(GetCurVolume());
};

ViaKeyHandler::~ViaKeyHandler()
{
    if (mWakeFd >= 0)
        close(mWakeFd);
    if (mInputHandler)
        delete mInputHandler;
    printf(" ViaKeyHandler exit <<<\n");
}

void ViaKeyHandler::SuspendProc()
{
    char buf[16];
    int ret;
    printf("Enter %s\n", __FUNCTION__);
    if (mWakeFd < 0)
        printf("*E* Illegal wake up fd (%d)\n", mWakeFd);

    do {
        ret = read(mWakeFd, buf, 16);
        if (ret) {
            ret = write(mWakeFd, buf, 16);
            if (ret)
                printf("*E* Write failed (ret: %d)\n", ret);
        } else
            printf("*E* Read failed (ret: %d)\n", ret);
    } while (!ret);
    SYSTEM("echo mem > /sys/power/state", true);
    printf("Leave %s\n", __FUNCTION__);
}

int ViaKeyHandler::GetCurVolume()
{
    char buf[128];
    sprintf(buf, "amixer -c %s cget name='Headset Volume'", SOUND_CARD);
    return SYSTEM(buf, true);
}

void ViaKeyHandler::SetVolume(int vol)
{
    char buf[128];
    /* transfer volume to hardware value */
    int volume = vol * 100 / (VOL_MAX - VOL_MIN);
    sprintf(buf, "amixer -c %s cset name='Headset Volume' %d", SOUND_CARD, vol);
    SYSTEM(buf, true);
    printf("\n[VKH] Set Headphone Volume: %d <%d>\n", volume, vol);
}

void ViaKeyHandler::VolumeChange(int delta)
{
    int vol = GetCurVolume();
    int volume = vol * 100 / (VOL_MAX - VOL_MIN);
    /* The real audio volume is inverse with vol */
    if (delta == VOL_DOWN) {  // vol-
        if (vol > VOL_MIN)
            vol--;
    } else {  // vol+
        if (vol < VOL_MAX)
            vol++;
    }
    SetVolume(vol);
}

void ViaKeyHandler::KeyEventNotify(int code, int value)
{
    MsgInfo msg;
    msg.id = 1;
    msg.code = code;
    msg.value = value;
    if (msgsnd(mMsgId, (void *)&msg, sizeof(MsgInfo), 0) == -1)
        printf("[%d]*E* msgsnd fail (errno: %d)\n", __LINE__, errno);
}

void ViaKeyHandler::KeyEventHandler()
{
    MsgInfo msg;
    long int _receive = 0;
    int ret;
    do {
        ret = msgrcv(mMsgId, (void *)&msg, sizeof(MsgInfo), _receive, 0 /*IPC_NOWAIT*/);
        if (ret == -1) {
            printf("[%d]*E* msgrcv fail (errno: %d)\n", __LINE__, errno);
            continue;
        }
        //printf("[%d] Receive Msg (code: %d, value: %d)\n", __LINE__, msg.code, msg.value);
        switch (msg.code) {
        case KEY_VOLUMEDOWN:
            if (msg.value == BTN_DOWN)
                VolumeChange(VOL_DOWN);
            break;
        case KEY_VOLUMEUP:
            if (msg.value == BTN_DOWN)
                VolumeChange(VOL_UP);
            break;
        case KEY_SUSPEND:
            printf("\n ==> <KEY_SUSPEND>::");
            if (msg.value == BTN_DOWN)
                printf("<Key Down>\n");
            else
                printf("<Key Up>\n");
            break;
        case KEY_POWER:
            printf("\n ==> <KEY_POWER>::");
            if (msg.value == BTN_DOWN) {
                printf("<Key Down>\n");
/* Not supported yet
                SuspendProc();
*/
            } else
                printf("<Key Up>\n");
            break;
        case BTN_LEFT:   /* 0x110 = 272 */
        case BTN_RIGHT:  /* 0x111 = 273 */
        case BTN_TOUCH:  /* 0x14a = 330 */
            break;
        default:
            if ((msg.code >= KEY_ESC) && (msg.code <= KEY_F12)) {
                 ; // ignore normal key event
            } else
                printf("<Unhandled> (code: %d, value: %d)\n", msg.code, msg.value);
            break;
        }
    } while (1);
}

#undef VIA_KEY_HANDLER_C
