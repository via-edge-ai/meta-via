#!/bin/sh
#//#!/bin/bash

#2025.01.10
ver="1.1.0"
#//=========================================================
vplay_set()
{
    plv=
    plv0='GST_DEBUG="GST_TRACER:7" GST_TRACERS="leaks"'
    plv1="gst-launch-1.0 -v filesrc location="
    plv2="qtdemux name=demux"
    #----------
    plv3="demux.video_0 ! queue !"
    plv4="decodebin !"
    plv5="v4l2convert output-io-mode=dmabuf-import capture-io-mode=dmabuf !"
    plv6="waylandsink"
    #----------
    plv7="demux.audio_0 ! queue !"
    plv8="decodebin !"
    plv9="audioconvert ! audioresample !"
    plv10="autoaudiosink"
    #---------------
    plv11="sync=false async=false"
    plv12="video/x-raw,height=720,width=1920"
    #-------------------------
    if [ "${lvf}" != "" ]; then plv1="${plv1}${lvf} !"; else lvx=1; fi

    case "${lvc}" in
        0)
            plv4="decodebin !"
            ;;
        1)
            plv4="parsebin ! v4l2h264dec !"
            ;;
        2)
            plv4="parsebin ! v4l2h265dec !"
            ;;
        3)
            plv4="parsebin ! v4l2mpeg4dec !"
            ;;
        #*)
        #    ;;
    esac

    case "${lvd}" in
        0)
            plv6="waylandsink"
            ;;
        1)
            plv6="waylandsink fullscreen=true"
            ;;
    esac

    if [ "${lvu}" != "1" ]; then
    case "${lva}" in
        2)
            plv10="alsasink device=\"hw:0,0\""
            ;;
        1)
            plv10="alsasink device=\"hw:0,5\""
            ;;
        *)
            plv10="autoaudiosink"
            ;;
    esac
    fi
}
vplay_cmd()
{
    plv=

    plv="${plv} ${plv1}"
    plv="${plv} ${plv2}"

    plv="${plv} ${plv3} ${plv4} ${plv5} ${plv6}"
    if [ "${lva}" != "0" ]; then
    plv="${plv} ${plv7} ${plv8} ${plv9} ${plv10}"
    fi
}
vplay()
{
    echo "start run vplay (version: $ver)"
    vplay_set
    vplay_cmd
    if [ "$lvx" = "0" ]; then
        cnt="$lvl"
        if [ "$cnt" = "-1" ]; then
            while true
            do
                $plv
            done
        else
            while [ $cnt -gt 0 ]
            do
                #(( cnt -- ))
                cnt=$(( cnt - 1 ))
                $plv
            done
        fi
    else
        dbg1
        dbg2
        local lvdbg="1"; if [ "$lvdbg" = "1" ]; then echo $plv; echo; fi
        #local lvdbg="1"; if [[ "$lvdbg" == "1" ]]; then echo $plv; echo; fi
    fi
}
dbg1()
{
    echo "===> ${lvf}"
    echo "===> ${lvc}"
    echo "===> ${lvd}"
    echo "===> ${lva}"
    echo "===> ${lvl}"
    echo "===> ${lvx}"
    echo
}
dbg2()
{
    #//[R]for (( i=1; i<=9; i=i+1 ))
    for i in `seq 9`
    do
       eval echo "plv$i ==\> " \$plv$i
    done
    echo
}
#//=====================================
syntax_type_usage()
{
echo
echo "----------------------------------------"
cat << HELP
    <> must| compulsory
    [] need| optional
    {} deploy

    usage:
        . ${0} <OPTIONS {VALUE}> {[OPTIONS {VALUE}] ...}

    (e.g.)
    . ${0} <-f path_to_video> [-c codec_type] [-d video_mode] [-a audio_mode}] [-l repeat_times]
    . ${0} -f "/media/1.mp4"
    . ${0} -f "/media/1.mp4" -c 0 -d 1 -a 0 -l 10
    (help)
    . ${0} -h
    (demo)
    sh ${0} -f "/media/1.mp4" -c 0 -d 1 -a 0 -l -1 -x

    Description:options
    (must)
        -f, --file
            Set source file
            (args num 1)
            (String:[full video path])
            (Default: null)

    (need)
        -h, --help
            Display this message

        -c
            Set codec type
            (args num 1)
            (Value:[0,3] - 3:v4l2mpeg4dec 2:v4l2h265dec 1:v4l2h264dec 0:decodebin)
            (Default: 0)

        -d
            Set display with fullscreen
            (args num 1)
            (Value:[0,1] - 1:fullscreen 0:original)
            (Default: 1)

        -a
            Set audio output device #(2&1 => yocto only)
            (args num 1)
            (Value:[0,2] - 2:headphone 1:hdmi 0:non-audio)
            (Default: 1)

        -l, --loop
            Set repeat times
            (args num 1)
            (Value:[-1,1<=N] - -1:loop)
            (Default: 1)
HELP
echo "----------------------------------------"
#exit 0
}
syntax_type()
{
    local lv_min=2
    local lv_max=11
    local lv_err=0
    local lv_get=1
    if [ $# -lt $lv_min ] || [ $# -gt $lv_max ] ; then
        way=1
    else
        #while true
        while [ -n "$1" ]
        do
        {
            local LOC_SEL
            case "${1}" in
                -f)
                    lv_get=$(($lv_get-1))
                    if [ "${2}" != "" ]; then lvf=${2}; shift; else lv_err=$(($lv_err+1)); fi
                    ;;
                -c)
                    if [ "${2}" = "" ]; then
                        lv_err=$(($lv_err+1))
                        #lv_err=$[$lv_err+1]
                    else
                        lvc="${2}"; shift
                    fi
                    ;;
                -d)
                    if [ "${2}" = "" ]; then lv_err=$(($lv_err+1)); else lvd=${2}; shift; fi
                    ;;
                -a)
                    if [ "${2}" != "" ]; then lva=${2}; shift; else lv_err=$(($lv_err+1)); fi
                    ;;
                -l)
                    if [ "${2}" != "" ]; then lvl=${2}; shift; else lv_err=$(($lv_err+1)); fi
                    ;;
                -x)
                    lvx=1
                    ;;
                #-h|--help)
                #    ;&
                *)
                    lv_err=$(($lv_err+1)); way=1
                    #return
                    #break
                    ;;
            esac
            shift
        }
        done
        #---------------------
        if [ "$lv_err" != "0" ]; then way=1; fi
        if [ $lv_get != 0 ]; then way=1; fi
        if [ "$way" = "1" ]; then
            echo "lv_err ===> ${lv_err}"
            echo "lv_get ===> ${lv_get}"
            syntax_type_usage
        else
            vplay
        fi
    fi
}
#//-------------------------------------
syntax_args_usage()
{
    echo "----------------------------------------"
    echo "\
    <> must| compulsory
    [] need| optional
    {} deploy

    syntax:
        . $0 <path_to_video> {[<codec_type> <video_mode> <audio_mode> {repeat_times}]}

    args:
        codec_type   (Default: 0) 3:v4l2mpeg4dec 2:v4l2h265dec 1:v4l2h264dec 0:decodebin
        video_mode   (Default: 1) 1:fullscreen 0:original
        audio_mode   (Default: 1) 2:headphone 1:hdmi 0:non-audio #(2&1 => yocto only)
        repeat_times -1:loop  (Default: 1)

    (e.g.)
    . ${0} /media/1.mp4
    . ${0} /media/1.mp4 1
    . ${0} /media/1.mp4 1 0
    . ${0} /media/1.mp4 1 0 2
    . ${0} /media/1.mp4 1 1 1 10
    (help)
    . ${0}
    (demo)
    sh ${0} /media/1.mp4 0 1 1 1 1"
    echo "----------------------------------------"
    #exit 0
}
syntax_args()
{
    local lv_min=1
    local lv_max=6 #//5+1
    #if [ "${1}" = "" ]
    #if [ -v $cnt ]
    if [ $# -lt $lv_min ] || [ $# -gt $lv_max ] ; then
        syntax_args_usage
    elif [ $# -ge $lv_min ] && [ $# -le $lv_max ] ; then
    #else
        ##set value (reset)
        if [ "${1}" != "" ]; then lvf="${1}"; fi
        if [ "${2}" != "" ]; then lvc="${2}"; fi
        if [ "${3}" != "" ]; then lvd="${3}"; fi
        if [ "${4}" != "" ]; then lva="${4}"; fi
        if [ "${5}" != "" ]; then lvl="${5}"; fi
        if [ "${6}" != "" ]; then lvx="${6}"; fi
    fi
    ##--------------
    vplay
}
#//=====================================
syntax_dbg()
{
    SETDBG="1"
    if [ "$SETDBG" = "1" ]; then
        echo "program name: ${0}"
        echo "parameter numbers:  \$# => "$#
        echo "parameter argument: \$@ => "$@
        echo "parameter argument: \$* => "$*
        #//[R]for (( i=1; i<=$#; i=i+1 ))
        for i in `seq $#`
        do
            eval echo "$i ==\>" \$$i
            #//[R]eval "lt$i=\$$i; echo $i ==\> lt$i \$lt$i"
            #eval "lt$i=\$$i"
            #eval echo "$i ==\> lt$i " \$lt$i
        done
        echo
    fi
}
syntax_default()
{
    ##--------------
    lvs=1
    lvu=0
    ##--------------
    #//local OSRV=`cat /etc/os-release | grep 'PRETTY_NAME=' | sed 's/PRETTY_NAME=//g'`;
    #//local OSRV=`cat /etc/os-release | grep 'ID=rity-vis' | sed 's/ID=//g'`;
    local OSRV=`cat /etc/os-release | grep 'ID=' | awk 'NR==1 {print $1}' | sed 's/ID=//g'`;
    OSRV=`echo $OSRV | xargs`;
    OSRV=`echo "$OSRV" | awk '{print $1}'`;
    echo "OSRV $OSRV";
    #//if [ "$OSRV" = "VIA" ]; then
    if [ "$OSRV" = "rity-vis" ]; then
        lvu=0; ## yocto
    elif [ "$OSRV" = "ubuntu" ]; then
        lvu=1;
    elif [ "$OSRV" = "debian" ]; then
        lvu=1;
    else
        lvu=1;
    fi;
    ##--------------
    lvf=
    lvc=0
    lvd=1
    lva=1
    lvl=1
    ##--------------
    lvx=0
    ##--------------
    way=0
}
syntax_check()
{
    syntax_default
    #syntax_dbg "$@"
    ##--------------
    if [ "$lvs" = "1" ]; then
        if [ -n "$1" ]; then
            case "${1}" in
                -v)
                    echo "version: $ver"
                    ;;
                -h)
                    syntax_type_usage
                    ;;
                *)
                    syntax_type "$@"
                    ;;
            esac
        else
            syntax_type_usage
        fi
    else
        if [ -n "$1" ]; then
            case "${1}" in
                -v)
                    echo "version: $ver"
                    ;;
                -h)
                    syntax_args_usage
                    ;;
                *)
                    syntax_args "$@"
                    ;;
            esac
        else
            syntax_args_usage
        fi
    fi
}
#//=========================================================
syntax_check "$@"