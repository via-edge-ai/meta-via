#!/bin/bash

set -e
atcmdfile=/dev/ttyUSB2
atlogfile=/tmp/init_quectel_mbim.log
atstsfile=/tmp/sts_quectel_mbim.log
ccidfile=/tmp/ccid.txt
dhcplogfile=/tmp/dhclient.log
dhcpleasefile=/var/run/dhclient.leases
apnlogfile=/tmp/apn.log
dnsconfigfile=/etc/resolv.conf
dns1=8.8.8.8
dns2=8.8.4.4
netintf=wwan0
apn=
user=
password=
#auth=(one of PAP, CHAP or MSCHAPV2)
auth=
#debugprint=console
#mbim_connect=1
MBIMNETWORK=/usr/bin/mbim-network
MBIMCLI=/usr/bin/mbimcli
mbimcmdfile=/dev/cdc-wdm0
mbimdefaultprofile=/etc/mbim-network.conf
mbimprofile=/tmp/mbim-network.conf

# quectel module hw reset
#hw_reset()
#{
    #echo 500 > /sys/kernel/debug/6a00000.ssusb/ext_mpcie_reset
#}

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

# check if quectel AT CMD file is exist and start logging
try_start_logging()
{
    pid=`ps x | grep ${atcmdfile} | grep cat | awk '{print $1}'`
    if [ "${pid}" == "" ]; then
        start_logging $1
    fi
}

# check/set if modem module USB profile & mode is correct
check_module_mode()
{
    echo -en "ATE0\r\n" > ${atcmdfile}
    sleep 1

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
        atcmd_reset
        stop_logging
        sleep 20
        exit 1
    fi
}

# set APN
setapn()
{
    # get APN from auto-config ${apnlogfile}
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

    echo -n "quectel_mbim: apn:\"${apn}\" user:\"${user}\" password:\"${password}\" auth:\"${auth}\" "

    if [ -x ${MBIMNETWORK} ] && [ -f ${mbimdefaultprofile} ]; then
        echo -n "quectel_mbim: MBIM connect "
        cp ${mbimdefaultprofile} ${mbimprofile}
        sed -i "s/APN=.*/APN=${apn}/g" ${mbimprofile}
        if [ -n "${user}" ] && [ -n "${password}" ]; then
            sed -i "s/APN_USER=.*/APN_USER=${user}/g" ${mbimprofile}
            sed -i "s/APN_PASS=.*/APN_PASS=${password}/g" ${mbimprofile}
            if [ -n "${auth}" ]; then
                sed -i "s/APN_AUTH=.*/APN_AUTH=${auth}/g" ${mbimprofile}
            fi
        fi
    fi
}

# init
do_init()
{
    echo -n "quectel_mbim: init "
    start_logging ${atlogfile}
    #if [ "${quick_init}" == "1" ]; then
        #echo -en "ATE0\r\n" > ${atcmdfile}
        #sleep 1
    #else
        check_module_mode
    #fi

    if [ "$1" != "1" ]; then
        stop_logging
    fi
}

# disconnecting
disconnecting()
{
    #if [ "${mbim_connect}" == "1" ]; then
        ${MBIMNETWORK} --profile=${mbimprofile} ${mbimcmdfile} stop >> ${atlogfile}
    #fi
}

# check ndis status
ndis_status()
{
    if [ -f ${atstsfile} ]; then
        stsfile=${atstsfile}
    else
        stsfile=${atlogfile}
    fi
    status=0
    echo -en "AT+CSQ\r\n" > ${atcmdfile}
    sleep 1
    echo -en "AT+COPS?\r\n" > ${atcmdfile}
    sleep 1
    #if [ "${mbim_connect}" == "1" ]; then
        stop_logging
        echo -n "quectel_mbim: getting IP... "
        # ${MBIMCLI} -d ${mbimcmdfile} --device-open-proxy --query-ip-configuration >> ${stsfile}
        STATUS_OUT=`${MBIMCLI} -d ${mbimcmdfile} --device-open-proxy --query-ip-configuration`
        if [ -z "${STATUS_OUT}" ]; then
            sleep 3
            echo -n "quectel_mbim: getting IP retry... "
            STATUS_OUT=`${MBIMCLI} -d ${mbimcmdfile} --device-open-proxy --query-ip-configuration`
            if [ -z "${STATUS_OUT}" ]; then
                echo -n "quectel_mbim: getting IP failed! "
                exit 1
            fi
        fi
        echo "${STATUS_OUT}" >> ${stsfile}
        status=`grep "IP " ${stsfile} | head -n 1 | awk -F"'" '{print $2}' | cut -d '/' -f 1`
    #fi

    if [ "${status}" == "" ] || [ "${status}" == "0.0.0.0" ]; then
        echo -n "quectel_mbim: IP=\"${status}\" error! "
        stop_logging
        exit 1
    fi

    #if [ "${mbim_connect}" == "1" ]; then
        netif_ip=`ifconfig ${netintf} | grep "inet " | awk '{print $2}'`
    #fi

    if [ "${status}" != "${netif_ip}" ]; then
        echo -n "quectel_mbim: IP=\"${status}\" != \"${netif_ip}\"! "
        stop_logging
        exit 1
    fi

    #if [ "${mbim_connect}" == "1" ]; then
        echo -n "quectel_mbim: getting network status... "
        ${MBIMCLI} -d ${mbimcmdfile} --query-connection-state --device-open-proxy >> ${stsfile}
    #fi
}

# check dns status
dns_status()
{
    if [ -f ${atstsfile} ]; then
        stsfile=${atstsfile}
    else
        stsfile=${atlogfile}
    fi

    #if [ "${mbim_connect}" == "1" ]; then
        dns=`grep "DNS \[0\]" ${stsfile} | head -n 1 | awk -F"'" '{print $2}' | cut -d '/' -f 1`
        if [ "${dns}" != "" ]; then
            dns_set=`grep "nameserver" ${dnsconfigfile} | grep ${dns} | wc -l`
            if [ ${dns_set} -eq 0 ]; then
                echo nameserver ${dns} >> ${dnsconfigfile}
            fi
        fi
        dns=`grep "DNS \[1\]" ${stsfile} | head -n 1 | awk -F"'" '{print $2}' | cut -d '/' -f 1`
        if [ "${dns}" != "" ]; then
            dns_set=`grep "nameserver" ${dnsconfigfile} | grep ${dns} | wc -l`
            if [ ${dns_set} -eq 0 ]; then
                echo nameserver ${dns} >> ${dnsconfigfile}
            fi
        fi
    #fi

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

# connect
do_connect()
{
    echo -n "quectel_mbim: connect "
    try_start_logging ${atlogfile}

    setapn

    #if [ "${mbim_connect}" == "1" ]; then
        stop_logging
        ifconfig ${netintf} down
        echo -n "quectel_mbim: mbim network start... "
        ${MBIMNETWORK} --profile=${mbimprofile} ${mbimcmdfile} stop >> ${atlogfile}
        ${MBIMNETWORK} --profile=${mbimprofile} ${mbimcmdfile} start >> ${atlogfile}
        ifconfig ${netintf} up
        sleep 1
    #fi

    if [ "$1" != "1" ]; then
        stop_logging
    fi
}

# disconnect
do_disconnect()
{
    echo -n "quectel_mbim: disconnect "
    #try_start_logging ${atlogfile}
    disconnecting
    #if [ "$1" != "1" ]; then
    #    stop_logging
    #fi
}

# ndis_status
do_ndis_status()
{
    #echo -n "quectel_mbim: ndis_status "
    try_start_logging ${atstsfile}
    ndis_status
    if [ "$1" != "1" ]; then
        stop_logging
    fi
}

# dhcp
do_dhcp()
{
    echo -n "quectel_mbim: dhcp ${netintf} "
    dhclient -v -lf ${dhcpleasefile} ${netintf} >& ${dhcplogfile}
}

# stop dhclient
stop_dhcp()
{
    pid=`ps x | grep dhclient | grep ${netintf} | awk '{print $1}'`
    if [ "${pid}" != "" ]; then
        kill -9 ${pid}
    fi
}

case "$1" in
  init)
    do_init 1
    ;;
  connect)
    do_connect 0
    sleep 5
    do_ndis_status 0
    dns_status
    ;;
  disconnect)
    do_disconnect 0
    ;;
  start)
    do_init 1
    do_connect 0
    sleep 5
    do_ndis_status 0
    dns_status
    ;;
  status)
    do_ndis_status 0
    dns_status
    ;;
  dns)
    dns_status
    ;;
  stop)
    connected=0
    ${MBIMCLI} -d ${mbimcmdfile} --query-connection-state --device-open-proxy > ${atstsfile}
    if [ -f ${atstsfile} ]; then
        #if [ "${mbim_connect}" == "1" ]; then
            status=`cat ${atstsfile} | sed -n "s/.*Activation state:.*'\(.*\)'.*/\1/p"`
            if [ "${status}" == "activated" ]; then
                connected=1
            fi
        #fi
    fi
    if [ "${connected}" != "0" ]; then
        do_disconnect 0
        echo -n "Shutting down ${netintf} "
        ifconfig ${netintf} 0.0.0.0
        ifconfig ${netintf} down
    fi
    ;;
  *)
    echo -n "Incorrect option specified "
    exit 1
    ;;
esac
exit 0
