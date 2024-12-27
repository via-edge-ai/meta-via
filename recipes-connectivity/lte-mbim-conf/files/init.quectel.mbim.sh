#!/bin/bash

atcmdfile=/dev/ttyUSB2
atlogfile=/tmp/init_quectel_sim.log
apnlogfile=/tmp/apn.log
LTE_APN=/usr/bin/lte-apn
netintf=wwan0

# Get LTE APN
get_lte_apn()
{
    if [ -f $LTE_APN ]; then
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
                fi
            else
                break
            fi
        done

        if [ ${ret} -ne 0 ]; then
            echo -n "$0: SIM is not ready "
            exit 0
        fi

        $LTE_APN apn -f ${atcmdfile} > ${apnlogfile}
    fi
}

# Check if SIM card exists
check_sim_status()
{
    i=0
    while [ $i -lt 3 ]
    do
        echo -en "AT+CPIN?\r\n" > ${atcmdfile}
        sleep 1
        sim_ready=`grep "+CPIN:" ${atlogfile} | tail -1 | wc -l`
        if [ "${sim_ready}" == "0" ]; then
            ((i=i+1))
            if [ $i -lt 3 ]; then
                echo -n "Retry SIM card detect... "
                sleep 3
            fi
        else
            i=3
        fi
    done
    
    if [ "${sim_ready}" == "0" ]; then
        echo -n "$0: SIM is not ready "
        stop_logging
        exit 0
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

# stop logging
stop_logging()
{
    pid=`ps x | grep ${atcmdfile} | grep cat | awk '{print $1}'`
    if [ "${pid}" != "" ]; then
        kill ${pid}
        sleep 1
    fi
}

# check if AT CMD file is exist and start logging
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
        sleep 5
        exit 1
    else
        echo -n "${atcmdfile} does not exist "
        #hw_reset
        sleep 5
        exit 1
    fi
}

# check if AT CMD file is exist and start logging
try_start_logging()
{
    pid=`ps x | grep ${atcmdfile} | grep cat | awk '{print $1}'`
    if [ "${pid}" == "" ]; then
        start_logging $1
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

# stop dhclient
stop_dhcp()
{
    #pid=`ps x | grep dhclient | grep ${netintf} | awk '{print $1}'`
    #if [ "${pid}" != "" ]; then
        #kill -9 ${pid}
    #fi
    ifconfig ${netintf} 0.0.0.0
    ifconfig ${netintf} down
}

case "$1" in
  start)
    echo -n "Starting init_quectel_mbim: "

    check_quectel_module
    start_logging ${atlogfile}
    wait_atcmd_ready 30
    check_sim_status
    stop_logging
    get_lte_apn
    set_quectel_atcmd.sh start

    if [ $? -eq 0 ]; then
        i=0
        while [ "$i" == "0" ]
        do
            sleep 60
            set_quectel_atcmd.sh status
            if [ $? -ne 0 ]; then
                set_quectel_atcmd.sh stop
                stop_dhcp
                set_quectel_atcmd.sh connect
            fi
        done
    else
        #sleep 15
        exit $?
    fi
    ;;
  stop)
    echo -n "Stopping init_quectel_mbim: "
    quectel_exist=""
    if lsusb | grep -q "2c7c:030b"; then
        quectel_exist=1
    fi
    if lsusb | grep -q "2c7c:0801"; then
        quectel_exist=1
    fi
    if [ "${quectel_exist}" == "1" ]; then
        set_quectel_atcmd.sh stop
    fi
    ;;
  *)
    echo -n "Incorrect option specified "
    ;;
esac
exit 0
