#!/bin/bash

atcmdfile=/dev/ttyUSB3
atlogfile=/tmp/chk_quectel_mode.log

# quectel module at command reset
atcmd_reset()
{
    echo -en "AT+CFUN=1,1\r\n" > ${atcmdfile}
    sleep 5
}

# stop logging
stop_logging()
{
    pid=`ps x | grep ${atcmdfile} | grep cat | awk '{print $1}'`
    if [ "${pid}" != "" ]; then
        kill ${pid}
        sleep 1
    fi
}

# check if quectel AT CMD file is exist and start logging
start_logging()
{
    stop_logging
    if [ -c ${atcmdfile} ]; then
        if [ "${debugprint}" == "console" ]; then
            cat ${atcmdfile} | tee $1 &
        else
            cat ${atcmdfile} > $1 &
        fi
        sleep 1
    elif [ -f ${atcmdfile} ]; then
        echo -n "${atcmdfile} not a char device file "
        rm ${atcmdfile}
        #hw_reset
        exit 1
    else
        echo -n "${atcmdfile} does not exist "
        #hw_reset
        exit 1
    fi
}

# Wait for AT cmd port ready
wait_atcmd_ready()
{
    i=0
    while [ $i -lt $1 ]
    do
        echo -en "ATE0\r\n" > ${atcmdfile}
        sleep 1
        cmd_ok=`grep "OK" ${atlogfile} | tail -1 | wc -l`
        if [ "${cmd_ok}" == "0" ]; then
            ((i=i+1))
            if [ $i -lt $1 ]; then
                echo -n "Retry AT cmd ready... "
                sleep 1
            fi
        else
            i=$1
        fi
    done

    if [ "${cmd_ok}" == "0" ]; then
        echo -n "$0: AT cmd ready failed "
    fi
}

# check/set if modem module USB profile & mode is correct
check_module_mode()
{
    #echo -en "ATE0\r\n" > ${atcmdfile}
    #sleep 1

    reset=0

    i=0
    while [ $i -lt 10 ]
    do
        echo -en 'AT+QCFG="usbnet"\r\n' > ${atcmdfile}
        sleep 1
        usbnet=`cat ${atlogfile} | grep '+QCFG: "usbnet"' | cut -d ':' -f 1`
        if [ "${usbnet}" != "" ]; then
            usbnet=`cat ${atlogfile} | grep '+QCFG: "usbnet",2' | cut -d ':' -f 1`
            if [ "${usbnet}" == "" ]; then
                echo -n "quectel_mbim: usbnet conf != 2(MBIM), conf to 2... "
                echo -en 'AT+QCFG="usbnet",2\r\n' > ${atcmdfile}
                sleep 1
                echo -en 'AT+QCFG="usbnet"\r\n' > ${atcmdfile}
                sleep 1
                reset=1
            fi
            break
        fi
        ((i=i+1))
    done

    if [ "${reset}" == "1" ]; then
        sleep 20
        atcmd_reset
        stop_logging
        sleep 20
        exit 1
    fi
}

# Check if quectel module exists
check_quectel_module()
{
    i=0
    quectel_exist=""
    while [ $i -lt 12 ]
    do
        #quectel_exist=`lsusb | sed 's/.*ID/ID/' | grep "1e2d:00b3"`
        #quectel_exist=`lsusb | grep "1e2d:00b3"`
        if lsusb | grep -q "2c7c:030b"; then
            echo "Quectel EM060K 4G module detected"
            quectel_exist=1
        fi

        if lsusb | grep -q "2c7c:0801"; then
            echo "Quectel RM520N 5G module detected"
            quectel_exist=1
        fi

        if [ "${quectel_exist}" != "1" ]; then
            echo -n "Retry quectel module detect... "
            ((i=i+1))
            sleep 8
        else
            break
        fi
    done

    if [ "${quectel_exist}" != "1" ]; then
        echo -n "$0: No quectel module exists."
        exit 0
    fi
}

case "$1" in
  start)
      echo -n "Starting set_quectel_mbim: "

      check_quectel_module
      start_logging ${atlogfile}
      wait_atcmd_ready 30
      check_module_mode
      stop_logging
      echo "."
    ;;
  stop)
      echo "."
    ;;
  *)
      echo "Usage: /usr/bin/set-quectel-mbim.sh {start|stop}"
      exit 1
    ;;
esac

exit 0
