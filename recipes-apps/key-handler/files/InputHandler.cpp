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
#define INPUT_HANDLER_C

#include <stdio.h>      /* for printf() */
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <time.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <errno.h>
#include <unistd.h>


#include "InputHandler.h"


#define INPUT_TRACE

#undef TRACE
#ifdef INPUT_TRACE
  #define TRACE(fmt, args...)    printf("{%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...)
#endif

#undef DBG_ERR
#define DBG_ERR(fmt, args...)   printf("*E* [%s]: " fmt, __FUNCTION__ , ## args)


extern void key_notify(void *priv, int code, int value);

static struct pollfd *ufds;
static char **device_names;
static int nfds;


static int open_device(const char *device)
{
    int version;
    int fd;
    int clkid = CLOCK_MONOTONIC;
    struct pollfd *new_ufds;
    char **new_device_names;
    char name[80];
    char location[80];
    char idstr[80];
    struct input_id id;

    fd = open(device, O_RDWR);
    if(fd < 0) {
        //printf("[%d]*E* could not open %s (%s)\n", __LINE__, device, strerror(errno));
        return -1;
    }
    
    if(ioctl(fd, EVIOCGVERSION, &version)) {
        printf("[%d]*E* could not get driver version for %s (%s)\n",
                 __LINE__, device, strerror(errno));
        return -1;
    }
    if(ioctl(fd, EVIOCGID, &id)) {
        fprintf(stderr, "[%d]*E* could not get driver id for %s (%s)\n", __LINE__, device, strerror(errno));
        return -1;
    }
    name[sizeof(name) - 1] = '\0';
    location[sizeof(location) - 1] = '\0';
    idstr[sizeof(idstr) - 1] = '\0';
    if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
        //fprintf(stderr, "could not get device name for %s, %s\n", device, strerror(errno));
        name[0] = '\0';
    }
    if(ioctl(fd, EVIOCGPHYS(sizeof(location) - 1), &location) < 1) {
        //fprintf(stderr, "could not get location for %s, %s\n", device, strerror(errno));
        location[0] = '\0';
    }
    if(ioctl(fd, EVIOCGUNIQ(sizeof(idstr) - 1), &idstr) < 1) {
        //fprintf(stderr, "could not get idstring for %s, %s\n", device, strerror(errno));
        idstr[0] = '\0';
    }

    if (ioctl(fd, EVIOCSCLOCKID, &clkid) != 0) {
        fprintf(stderr, "[%d] Can't enable monotonic clock reporting: %s\n", __LINE__, strerror(errno));
        // a non-fatal error
    }

    new_ufds = (struct pollfd *)realloc(ufds, sizeof(ufds[0]) * (nfds + 1));
    if(new_ufds == NULL) {
        fprintf(stderr, "out of memory\n");
        return -1;
    }
    ufds = new_ufds;
//    new_device_names = realloc(device_names, sizeof(device_names[0]) * (nfds + 1));
    void *ptr = realloc(device_names, sizeof(device_names[0]) * (nfds + 1));
    new_device_names = (char **)ptr;
    if(new_device_names == NULL) {
        fprintf(stderr, "out of memory\n");
        return -1;
    }
    device_names = new_device_names;

    ufds[nfds].fd = fd;
    ufds[nfds].events = POLLIN;
    device_names[nfds] = strdup(device);
    nfds++;

    return 0;
}

int close_device(const char *device)
{
    int i;

    for(i = 1; i < nfds; i++) {
        if(strcmp(device_names[i], device) == 0) {
            int count = nfds - i - 1;
            free(device_names[i]);
            memmove(device_names + i, device_names + i + 1, sizeof(device_names[0]) * count);
            memmove(ufds + i, ufds + i + 1, sizeof(ufds[0]) * count);
            nfds--;
            return 0;
        }
    }
    fprintf(stderr, "remote device: %s not found\n", device);

    return -1;
}

static int read_notify(const char *dirname, int nfd)
{
    int res;
    char devname[PATH_MAX];
    char *filename;
    char event_buf[512];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;

    res = read(nfd, event_buf, sizeof(event_buf));
    if(res < (int)sizeof(*event)) {
        if(errno == EINTR)
            return 0;
        fprintf(stderr, "could not get event, %s\n", strerror(errno));
        return 1;
    }
    //printf("got %d bytes of event information\n", res);
    printf("[%d] dirname: %s\n", __LINE__, dirname);
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    printf("[%d] filename: %s\n", __LINE__, filename);

    while(res >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        printf("%d: %08x \"%s\"\n", event->wd, event->mask, event->len ? event->name : "");
        if(event->len) {
            strcpy(filename, event->name);
            if(event->mask & IN_CREATE)
                open_device(devname);
            else
                close_device(devname);
        }
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }
    printf("[%d] read_notify end\n", __LINE__);
    return 0;
}

static int scan_dir(const char *dirname)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;

    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';

    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        open_device(devname);
    }
    closedir(dir);

    return 0;
}

void InputHandler::HandleInputEvent()
{
    int res;
    struct input_event ev;
    const char *device_path = "/dev/input";

    nfds = 1;
    void *ptr = calloc(1, sizeof(ufds[0]));
    ufds = (struct pollfd *)ptr;
    ufds[0].fd = inotify_init();
    ufds[0].events = POLLIN;

    res = inotify_add_watch(ufds[0].fd, device_path, IN_DELETE | IN_CREATE);
    if(res < 0) {
        fprintf(stderr, "could not add watch for %s, %s\n", device_path, strerror(errno));
        return;
    }
    res = scan_dir(device_path);
    if(res < 0) {
        fprintf(stderr, "scan dir failed for %s\n", device_path);
        return;
    }

    mThreadRun = true;
    while (mThreadRun) {
        poll(ufds, nfds, -1);  // block mode
        if (!mThreadRun)
            break;
        if(ufds[0].revents & POLLIN)
            read_notify(device_path, ufds[0].fd);

        for(int i = 1; i < nfds; i++) {
            if(ufds[i].revents) {
                if(ufds[i].revents & POLLIN) {
                    res = read(ufds[i].fd, &ev, sizeof(ev));
                    if(res < (int)sizeof(ev)) {
                        fprintf(stderr, "could not get event\n");
                        return;
                    }
                    if (ev.type == EV_KEY)
                        key_notify(mPriv, ev.code, ev.value);
                }
            }
        }
    }
}

void InputHandler::StartThread(void)
{
    if (mThreadRun) {
        DBG_ERR("Tread was already running! (%d)\n", mThreadRun);
        return;
    }
    TRACE("InputHandler thread was spawn\n");
    HandleInputEvent();
    TRACE("InputHandler thread exited \n");
}

static void * InputHandler_Thread(void *arg)
{
    InputHandler *pThis = (InputHandler *)arg;
    pThis->StartThread();
    return 0;
}

InputHandler::InputHandler(void *priv) : mThreadRun(false)
{
    mPriv = priv;
    pthread_create(&mEventThread, 0, InputHandler_Thread, (void *)this);
    while (!mThreadRun) {};
}

InputHandler::~InputHandler()
{
    StopThread();
}

void InputHandler::StopThread()
{
    if (mThreadRun) {
        TRACE("Stop Input Handler thread...\n");
        mThreadRun = false;
        pthread_join(mEventThread, 0);
    }
}

/*------------------------------------------------------------------------------*/
/*--------------------End of Function Body -----------------------------------*/

#undef INPUT_HANDLER_C
