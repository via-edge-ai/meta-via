#!/bin/bash

atcmdfile=/dev/ttyUSB2
atlogfile=/tmp/chk_telit_mode.log
apnlogfile=/tmp/apn.log
LTE_APN=/usr/bin/lte-apn
QMICLI=/usr/bin/qmicli
TELIT_FASTSHDN=0
TELIT_BOOTOK=0
TELIT_SIMCFG=1
netintf=wwan0
qmicmdfile=/dev/cdc-wdm0

# telit module at command reset
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

# check if telit AT CMD file is exist and start logging
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
        echo -en "AT#USBCFG?\r\n" > ${atcmdfile}
        sleep 1
        usbnet=`cat ${atlogfile} | grep "#USBCFG:" | cut -d ':' -f 1`
        if [ "${usbnet}" != "" ]; then
            usbnet=`cat ${atlogfile} | grep "#USBCFG: 1" | cut -d ':' -f 1`
            if [ "${usbnet}" == "" ]; then
                echo -n "telit_qmi: usbnet conf != 1(QMI), conf to 1... "
                echo -en "AT#USBCFG=1\r\n" > ${atcmdfile}
                sleep 1
                echo -en "AT#USBCFG?\r\n" > ${atcmdfile}
                sleep 1
                reset=1
            fi
            break
        fi
        ((i=i+1))
    done

    platform=`fw_printenv boot_conf | cut -d '#' -f 2 | grep "conf-mediatek_" | sed "s/conf-mediatek_//g" | sed "s/.dtb//g"`
    echo -n "telit_qmi: platform name = ${platform} "
    if [ "${platform}" == "vab-5000" ]; then
        TELIT_SIMCFG=0
    else
        TELIT_SIMCFG=1
    fi

    i=0
    while [ $i -lt 10 ]
    do
        echo -en "AT#SIMINCFG?\r\n" > ${atcmdfile}
        sleep 1
        simincfg=`cat ${atlogfile} | grep "#SIMINCFG:" | cut -d ':' -f 1`
        if [ "${simincfg}" != "" ]; then
            simincfg=`cat ${atlogfile} | grep "#SIMINCFG: 1,${TELIT_SIMCFG}" | cut -d ':' -f 1`
            if [ "${simincfg}" == "" ]; then
                echo -n "telit_qmi: simincfg conf != ${TELIT_SIMCFG}, conf to ${TELIT_SIMCFG}... "
                echo -en "AT#SIMINCFG=1,${TELIT_SIMCFG}\r\n" > ${atcmdfile}
                sleep 1
                echo -en "AT#SIMINCFG?\r\n" > ${atcmdfile}
                sleep 1
                reset=1
            fi
            break
        fi
        ((i=i+1))
    done

    if [ "${TELIT_FASTSHDN}" == "1" ]; then
    i=0
    while [ $i -lt 10 ]
    do
        echo -en "AT#FASTSHDN?\r\n" > ${atcmdfile}
        sleep 1
        fastshdn=`cat ${atlogfile} | grep "#FASTSHDN:" | cut -d ':' -f 1`
        if [ "${fastshdn}" != "" ]; then
            fastshdn=`cat ${atlogfile} | grep "#FASTSHDN: 1,7" | cut -d ':' -f 1`
            if [ "${fastshdn}" == "" ]; then
                echo -n "telit_qmi: fastshdn conf != 1,7, conf to 1,7,0... "
                echo -en "AT#FASTSHDN=1,7,0\r\n" > ${atcmdfile}
                sleep 1
                echo -en "AT#FASTSHDN?\r\n" > ${atcmdfile}
                sleep 1
                reset=1
            fi
            break
        fi
        ((i=i+1))
    done
    fi

    if [ "${TELIT_BOOTOK}" == "1" ]; then
    i=0
    while [ $i -lt 10 ]
    do
        echo -en "AT#SHDNIND?\r\n" > ${atcmdfile}
        sleep 1
        shdnind=`cat ${atlogfile} | grep "#SHDNIND:" | cut -d ':' -f 1`
        if [ "${shdnind}" != "" ]; then
            shdnind=`cat ${atlogfile} | grep "#SHDNIND: 3,8" | cut -d ':' -f 1`
            if [ "${shdnind}" == "" ]; then
                echo -n "telit_qmi: shdnind conf != 3,8, conf to 3,8... "
                echo -en "AT#SHDNIND=3,8\r\n" > ${atcmdfile}
                sleep 1
                echo -en "AT#SHDNIND?\r\n" > ${atcmdfile}
                sleep 1
                reset=1
            fi
            break
        fi
        ((i=i+1))
    done
    fi

    if [ "${reset}" == "1" ]; then
        atcmd_reset
        stop_logging
        sleep 20
        exit 1
    fi
}

# Get LTE APN
get_lte_apn()
{
    if [ -f $LTE_APN ] && [ -f $QMICLI ]; then
        $QMICLI -d $qmicmdfile --dms-set-operating-mode='online'
        i=0
        while [ $i -lt 5 ]
        do
            $LTE_APN sim -f ${atcmdfile}
            ret=$?
            if [ ${ret} -ne 0 ]; then
                ((i=i+1))
                if [ $i -lt 5 ]; then
                    echo -n "Retry $LTE_APN sim -f ${atcmdfile}... "
                    sleep 3
                    $QMICLI -d $qmicmdfile --dms-set-operating-mode='online'
                fi
            else
                break
            fi
        done

        if [ ${ret} -ne 0 ]; then
            echo -n "$0: SIM is not ready "
            exit 0
        fi

        $LTE_APN apn -f ${atcmdfile} -q telit > ${apnlogfile}
    else
        echo -n "$0: $LTE_APN or $QMICLI not exist! "
        exit 0
    fi
}

# Check if telit module exists
check_telit_module()
{
    i=0
    telit_exist=""
    while [ $i -lt 12 ]
    do
        #telit_exist=`lsusb | sed 's/.*ID/ID/' | grep "1e2d:00b3"`
        #telit_exist=`lsusb | grep "1e2d:00b3"`
        if lsusb | grep -q "1bc7:1060"; then
            echo "Telit LN920 4G module detected"
            telit_exist=1
            TELIT_FASTSHDN=1
            TELIT_BOOTOK=1
        fi

        if lsusb | grep -q "1bc7:1070"; then
            echo "Telit FN990 5G module detected"
            telit_exist=1
            TELIT_FASTSHDN=1
        fi

        if [ "${telit_exist}" != "1" ]; then
            echo -n "Retry telit module detect... "
            ((i=i+1))
            sleep 8
        else
            break
        fi
    done

    if [ "${telit_exist}" != "1" ]; then
        echo -n "$0: No telit module exists."
        exit 0
    fi
}

# stop udhcpc
stop_dhcp()
{
    pid=`ps x | grep udhcpc | grep ${netintf} | awk '{print $1}'`
    if [ "${pid}" != "" ]; then
        kill -9 ${pid}
    fi
    ifconfig ${netintf} 0.0.0.0
    ifconfig ${netintf} down
}

case "$1" in
  start)
    echo -n "Starting init_telit_qmi: "

    check_telit_module
    start_logging ${atlogfile}
    wait_atcmd_ready 30
    check_module_mode
    stop_logging
    get_lte_apn
    set_telit_atcmd.sh start

    if [ $? -eq 0 ]; then
        i=0
        while [ "$i" == "0" ]
        do
            sleep 60
            set_telit_atcmd.sh status
            if [ $? -ne 0 ]; then
                set_telit_atcmd.sh stop
                stop_dhcp
                set_telit_atcmd.sh connect
            fi
        done
    else
        #sleep 15
        exit $?
    fi
    ;;
  stop)
    echo -n "Stopping init_telit_qmi: "
    telit_exist=""
    if lsusb | grep -q "1bc7:1060"; then
        telit_exist=1
    fi
    if lsusb | grep -q "1bc7:1070"; then
        telit_exist=1
    fi
    if [ "${telit_exist}" == "1" ]; then
        set_telit_atcmd.sh stop
    fi
    ;;
  *)
    echo -n "Incorrect option specified "
    ;;
esac
exit 0
