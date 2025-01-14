#!/bin/bash

atcmdfile=/dev/ttyUSB2
atlogfile=/tmp/init_quectel_cm.log
apnlogfile=/tmp/apn.log
dnsconfigfile=/etc/resolv.conf
dns1=8.8.8.8
dns2=8.8.4.4
netintf=wwan0
apn=internet
user=
password=
auth=
pincode=
LTE_APN=/usr/bin/lte-apn
#debugprint=console

# check network interface name
check_netintf_name()
{
    netif_name=`ifconfig -a | grep ${netintf} | cut -d ':' -f 1`
    if [ "${netif_name}" == "" ]; then
        echo -n "$0: NETIF=\"${netintf}\" not exist! "
        netif_name=`ifconfig -a | grep 'wwx' | cut -d ':' -f 1`
        if [ "${netif_name}" == "" ]; then
            echo -n "$0: NETIF=\"wwx<MAC>\" not exist! "
            exit 1
        else
            netintf=${netif_name}
            echo -n "$0: NETIF=\"${netintf}\" "
        fi
    fi
}

# Check if Quectel module exists
check_quectel_module()
{
    i=0
    quectel_exist=""
    while [ $i -lt 10 ]
    do
        #quectel_exist=`lsusb | sed 's/.*ID/ID/' | grep "2c7c:030e"`
        #quectel_exist=`lsusb | sed 's/.*ID/ID/' | grep "2c7c:"`
        if lsusb | grep -q "2c7c:030e"; then
            echo "Quectel EM05-G 4G module detected"
            quectel_exist=1
        fi

        if lsusb | grep -q "2c7c:030a"; then
            echo "Quectel EM05-G Rev.2 4G module detected"
            quectel_exist=1
        fi

        if [ "${quectel_exist}" != "1" ]; then
            ((i=i+1))
            if [ $i -lt 10 ]; then
                echo -n "Retry Quectel module detect... "
                sleep 10
            fi
        else
            break
        fi
    done
    
    if [ "${quectel_exist}" != "1" ]; then
        echo -n "$0: No Quectel module exists."
        exit 0
    fi
}

# Get LTE APN
get_lte_apn()
{
    if [ -f $LTE_APN ]; then
        i=0
        while [ $i -lt 3 ]
        do
            $LTE_APN sim -f ${atcmdfile}
            ret=$?
            if [ ${ret} -ne 0 ]; then
                ((i=i+1))
                if [ $i -lt 3 ]; then
                    echo -n "Retry $LTE_APN sim -f ${atcmdfile}... "
                    sleep 3
                fi
            else
                i=3
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

# init quectel modem state
init_quectel_mode()
{
    start_logging ${atlogfile}

    wait_atcmd_ready 30

    echo -en "ATI;+GSN\r\n" > ${atcmdfile}
    sleep 1

    check_sim_status

    if [ -n "${pincode}" ]; then
        echo -en "AT+CPIN=${pincode}\r\n" > ${atcmdfile}
        sleep 1
    fi

    echo -en "AT+CSQ\r\n" > ${atcmdfile}
    sleep 1

    echo -en "AT+CGATT?\r\n" > ${atcmdfile}
    sleep 1

    echo -en "AT+COPS?\r\n" > ${atcmdfile}
    sleep 1

    #echo -en "AT+CGDCONT?\r\n" > ${atcmdfile}
    #sleep 1

    echo -en "AT+CGACT?\r\n" > ${atcmdfile}
    sleep 1
}

# check dns status
dns_status()
{
    dns=${dns1}
    dns_set=`grep "nameserver" ${dnsconfigfile} | grep ${dns} | wc -l`
    if [ ${dns_set} -eq 0 ]; then
        echo nameserver ${dns} >> ${dnsconfigfile}
    fi
    dns=${dns2}
    dns_set=`grep "nameserver" ${dnsconfigfile} | grep ${dns} | wc -l`
    if [ ${dns_set} -eq 0 ]; then
        echo nameserver ${dns} >> ${dnsconfigfile}
    fi
}

# connect quectelcm 
connect_quectel_cm()
{
    if [ -f ${apnlogfile} ]; then
        auto_apn=`grep "+APN:" ${apnlogfile} | cut -d ':' -f 2 | awk '{print $1}' | cut -d ',' -f 1`
        auto_user=`grep "+APN:" ${apnlogfile} | cut -d ':' -f 2 | awk '{print $1}' | cut -d ',' -f 2`
        auto_password=`grep "+APN:" ${apnlogfile} | cut -d ':' -f 2 | awk '{print $1}' | cut -d ',' -f 3`
        if [ -n "${auto_apn}" ]; then
            apn=${auto_apn}
        fi
        if [ -n "${auto_user}" ]; then
            user=${auto_user}
        fi
        if [ -n "${auto_password}" ]; then
            password=${auto_password}
        fi
    fi

    args=
    if [ -n "${apn}" ]; then
        args="-s ${apn}"
        if [ -n "${user}" ] && [ -n "${password}" ]; then
            args="${args} ${user} ${password}"
            if [ -n "${auth}" ]; then
                args="${args} ${auth}"
            fi
        fi
        echo -en "AT+CGDCONT=1,\"IP\",\"${apn}\",,0,0\r\n" > ${atcmdfile}
        sleep 1
    fi
    if [ -n "${pincode}" ]; then
        args="${args} -p ${pincode}"
    fi

    echo -en "AT+CGDCONT?\r\n" > ${atcmdfile}
    sleep 1

    stop_logging
    ifconfig ${netintf} 0.0.0.0
    ifconfig ${netintf} down

    dns_status

    echo -n "Connecting ${netintf}... "
    quectelcm ${args} &
}

case "$1" in
  start)
      check_quectel_module
      check_netintf_name
      get_lte_apn
      init_quectel_mode
      echo -n "Starting up ${netintf}... "
      connect_quectel_cm
      i=0
      while [ "$i" == "0" ]
      do
          sleep 60
          pid=`ps x | grep quectelcm | grep "\-s" | awk '{print $1}'`
          if [ "${pid}" != "" ]; then
              dns_status
          else
              break
          fi
      done
      echo "."
      exit 1
    ;;
  stop)
      #quectel_exist=`lsusb | sed 's/.*ID/ID/' | grep "2c7c:030e"`
      #quectel_exist=`lsusb | sed 's/.*ID/ID/' | grep "2c7c:"`
      quectel_exist=""
      if lsusb | grep -q "2c7c:030e"; then
          quectel_exist=1
      fi
      if lsusb | grep -q "2c7c:030a"; then
          quectel_exist=1
      fi
      if [ "${quectel_exist}" == "1" ]; then
          check_netintf_name
          echo -n "Shutting down ${netintf}"
          ifconfig $netintf 0.0.0.0
          ifconfig $netintf down
      fi
      echo "."
    ;;
  *)
      echo "Usage: /usr/bin/init-quectel-cm.sh {start|stop}"
      exit 1
    ;;
esac

exit 0
