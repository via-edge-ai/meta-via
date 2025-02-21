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
#ifndef VIA_KEY_HANDLER_H
#define VIA_KEY_HANDLER_H

#include "InputHandler.h"


class ViaKeyHandler
{
public:
    ViaKeyHandler();
    ~ViaKeyHandler();

    void KeyEventHandler();
    void KeyEventNotify(int code, int value);

private:
    void SuspendProc();
    
    /* Volume +/- handler */
    int GetCurVolume();
    void SetVolume(int vol);
    void VolumeChange(int value);

    InputHandler * mInputHandler;
    int mMsgId;
    int mWakeFd;
};

#endif /* ifndef VIA_KEY_HANDLER_H */
