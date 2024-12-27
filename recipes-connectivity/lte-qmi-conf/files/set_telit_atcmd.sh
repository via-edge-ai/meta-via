#!/bin/bash

set -e
atcmdfile=/dev/ttyUSB2
atlogfile=/tmp/init_telit_qmi.log
atstsfile=/tmp/sts_telit_qmi.log
ccidfile=/tmp/ccid.txt
dhcplogfile=/tmp/udhcpc.log
#dhcpleasefile=/var/run/dhclient.leases
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
#qmi_connect=1
QMINETWORK=/usr/bin/qmi-network
QMICLI=/usr/bin/qmicli
PROXY_OPT=--device-open-proxy
IP_TYPE=4
CID=
PDH=
START_NETWORK_ARGS=
qmicmdfile=/dev/cdc-wdm0
qmidefaultprofile=/etc/qmi-network.conf
qmiprofile=/tmp/qmi-network.conf
STATE_FILE=/tmp/qmi-network-state-`basename $qmicmdfile`

save_state ()
{
    KEY=$1
    VAL=$2

    echo "Saving state at ${STATE_FILE}... ($KEY: $VAL)"

    if [ -f "$STATE_FILE" ]; then
        PREVIOUS=`cat $STATE_FILE`
        PREVIOUS=`echo "$PREVIOUS" | grep -v $KEY`
        if [ -n "$PREVIOUS" ]; then
            echo $PREVIOUS > $STATE_FILE
        else
            rm $STATE_FILE
        fi
    fi

    if [ -n "$VAL" ]; then
        echo "$KEY=\"$VAL\"" >> $STATE_FILE
    fi
}

load_state ()
{
    if [ -f "$STATE_FILE" ]; then
        echo "Loading previous state from ${STATE_FILE}..."
        . $STATE_FILE

        if [ -n "$CID" ]; then
            echo "    Previous CID: $CID"
        fi
        if [ -n "$PDH" ]; then
            echo "    Previous PDH: $PDH"
        fi
    fi
}

clear_state ()
{
    echo "Clearing state at ${STATE_FILE}..."
    rm -f $STATE_FILE
}

# telit module hw reset
#hw_reset()
#{
    #echo 500 > /sys/kernel/debug/6a00000.ssusb/ext_mpcie_reset
#}

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

# check if telit AT CMD file is exist and start logging
try_start_logging()
{
    pid=`ps x | grep ${atcmdfile} | grep cat | awk '{print $1}'`
    if [ "${pid}" == "" ]; then
        start_logging $1
    fi
}

# check network interface name
check_netintf_name()
{
    netif_name=`ifconfig -a | grep ${netintf} | cut -d ':' -f 1`
    if [ "${netif_name}" == "" ]; then
        echo -n "telit_qmi: NETIF=\"${netintf}\" not exist! "
        netif_name=`ifconfig -a | grep 'wwx' | cut -d ':' -f 1`
        if [ "${netif_name}" == "" ]; then
            echo -n "telit_qmi: NETIF=\"wwx<MAC>\" not exist! "
            exit 1
        else
            netintf=${netif_name}
            echo -n "telit_qmi: NETIF=\"${netintf}\" "
        fi
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

    echo -n "telit_qmi: apn:\"${apn}\" user:\"${user}\" password:\"${password}\" auth:\"${auth}\" "

    if [ -x ${QMINETWORK} ] && [ -f ${qmidefaultprofile} ]; then
        echo -n "telit_qmi: QMI connect "
        cp ${qmidefaultprofile} ${qmiprofile}
        sed -i "s/APN=.*/APN=${apn}/g" ${qmiprofile}
        if [ -n "${user}" ] && [ -n "${password}" ]; then
            sed -i "s/APN_USER=.*/APN_USER=${user}/g" ${qmiprofile}
            sed -i "s/APN_PASS=.*/APN_PASS=${password}/g" ${qmiprofile}
            if [ -n "${auth}" ]; then
                sed -i "s/APN_AUTH=.*/APN_AUTH=${auth}/g" ${qmiprofile}
            fi
        fi
    fi

    if [ -n "${apn}" ]; then
        START_NETWORK_ARGS="apn='${apn}'"
        if [ -n "${user}" ]; then
            START_NETWORK_ARGS="${START_NETWORK_ARGS},username='${user}'"
            if [ -n "${password}" ]; then
                START_NETWORK_ARGS="${START_NETWORK_ARGS},password='${password}'"
            fi
        fi
        if [ -n "$IP_TYPE" ]; then
            START_NETWORK_ARGS="${START_NETWORK_ARGS},ip-type='$IP_TYPE'"
        fi
    fi
}

# init
do_init()
{
    echo -n "telit_qmi: init "
    start_logging ${atlogfile}

    if [ "$1" != "1" ]; then
        stop_logging
    fi
}

# disconnecting
disconnecting()
{
    #if [ "${qmi_connect}" == "1" ]; then
        #${QMINETWORK} --profile=${qmiprofile} ${qmicmdfile} stop >> ${atlogfile}
        ${QMICLI} -d ${qmicmdfile} --wds-stop-network=$PDH --client-cid=$CID --device-open-proxy >> ${atlogfile}
        clear_state
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
    stop_logging
    ifconfig ${netintf} >> ${stsfile}
    
    #if [ "${qmi_connect}" == "1" ]; then
    #    stop_logging
    #    echo -n "telit_qmi: getting IP... "
    #    # ${QMICLI} -d ${qmicmdfile} --device-open-proxy --query-ip-configuration >> ${stsfile}
    #    STATUS_OUT=`${QMICLI} -d ${qmicmdfile} --device-open-proxy --query-ip-configuration`
    #    if [ -z "${STATUS_OUT}" ]; then
    #        sleep 3
    #        echo -n "telit_qmi: getting IP retry... "
    #        STATUS_OUT=`${QMICLI} -d ${qmicmdfile} --device-open-proxy --query-ip-configuration`
    #        if [ -z "${STATUS_OUT}" ]; then
    #            echo -n "telit_qmi: getting IP failed! "
    #            exit 1
    #        fi
    #    fi
    #    echo "${STATUS_OUT}" >> ${stsfile}
    #    status=`grep "IP " ${stsfile} | head -n 1 | awk -F"'" '{print $2}' | cut -d '/' -f 1`
    #fi

    #if [ "${status}" == "" ] || [ "${status}" == "0.0.0.0" ]; then
    #    echo -n "telit_qmi: IP=\"${status}\" error! "
    #    stop_logging
    #    exit 1
    #fi

    #if [ "${qmi_connect}" == "1" ]; then
    #    netif_ip=`ifconfig ${netintf} | grep "inet " | awk '{print $2}'`
    #fi

    #if [ "${status}" != "${netif_ip}" ]; then
    #    echo -n "telit_qmi: IP=\"${status}\" != \"${netif_ip}\"! "
    #    stop_logging
    #    exit 1
    #fi

    #if [ "${qmi_connect}" == "1" ]; then
    #    echo -n "telit_qmi: getting network status... "
    #    ${QMICLI} -d ${qmicmdfile} --query-connection-state --device-open-proxy >> ${stsfile}
    #fi
}

# check dns status
dns_status()
{
    #if [ -f ${atstsfile} ]; then
    #    stsfile=${atstsfile}
    #else
    #    stsfile=${atlogfile}
    #fi

    #if [ "${qmi_connect}" == "1" ]; then
    #    dns=`grep "DNS \[0\]" ${stsfile} | head -n 1 | awk -F"'" '{print $2}' | cut -d '/' -f 1`
    #    if [ "${dns}" != "" ]; then
    #        dns_set=`grep "nameserver" ${dnsconfigfile} | grep ${dns} | wc -l`
    #        if [ ${dns_set} -eq 0 ]; then
    #            echo nameserver ${dns} >> ${dnsconfigfile}
    #        fi
    #    fi
    #    dns=`grep "DNS \[1\]" ${stsfile} | head -n 1 | awk -F"'" '{print $2}' | cut -d '/' -f 1`
    #    if [ "${dns}" != "" ]; then
    #        dns_set=`grep "nameserver" ${dnsconfigfile} | grep ${dns} | wc -l`
    #        if [ ${dns_set} -eq 0 ]; then
    #            echo nameserver ${dns} >> ${dnsconfigfile}
    #        fi
    #    fi
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
    echo -n "telit_qmi: connect "
    try_start_logging ${atlogfile}

    setapn

    #if [ "${qmi_connect}" == "1" ]; then
        stop_logging
        #${QMICLI} -d ${qmicmdfile} --dms-set-operating-mode='online' >> ${atlogfile}
        ifconfig ${netintf} down
        echo 'Y' | tee /sys/class/net/${netintf}/qmi/raw_ip
        ifconfig ${netintf} up
        ${QMICLI} -d ${qmicmdfile} --wda-get-data-format >> ${atlogfile}
        echo -n "telit_qmi: qmi network start... "
        ##sudo qmicli -p -d /dev/cdc-wdm0 -p --wds-start-network="apn='YOUR_APN',username='YOUR_USERNAME',password='YOUR_PASSWORD',ip-type=4" --client-no-release-cid
        #${QMINETWORK} --profile=${qmiprofile} ${qmicmdfile} stop >> ${atlogfile}
        #${QMINETWORK} --profile=${qmiprofile} ${qmicmdfile} start >> ${atlogfile}
        #${QMICLI} -d ${qmicmdfile} --device-open-net='net-raw-ip|net-no-qos-header' --wds-start-network="apn='internet',ip-type=4" --client-no-release-cid --device-open-proxy >> /tmp/init_telit_qmi.log
        ${QMICLI} -d ${qmicmdfile} --device-open-net='net-raw-ip|net-no-qos-header' --wds-start-network="${START_NETWORK_ARGS}" --client-no-release-cid --device-open-proxy >> ${atlogfile}
        #ifconfig ${netintf} up
        sleep 1
    #fi

    # Save the new CID if we didn't use any before
    if [ -z "$CID" ]; then
        CID=`cat ${atlogfile} | sed -n "s/.*CID.*'\(.*\)'.*/\1/p"`
        if [ -z "$CID" ]; then
            echo -n "error: network start failed, client not allocated " 1>&2
            exit 1
        else
            save_state "CID" $CID
        fi
    fi

    PDH=`cat ${atlogfile} | sed -n "s/.*handle.*'\(.*\)'.*/\1/p"`
    if [ -z "$PDH" ]; then
        echo -n "error: network start failed, no packet data handle " 1>&2
        # Cleanup the client
        ${QMICLI} -d ${qmicmdfile} --wds-noop --client-cid="$CID" $PROXY_OPT
        clear_state
        exit 2
    else
        save_state "PDH" $PDH
    fi

    if [ "$1" != "1" ]; then
        stop_logging
    fi
}

# disconnect
do_disconnect()
{
    echo -n "telit_qmi: disconnect "
    #try_start_logging ${atlogfile}
    disconnecting
    #if [ "$1" != "1" ]; then
    #    stop_logging
    #fi
}

# ndis_status
do_ndis_status()
{
    #echo -n "telit_qmi: ndis_status "
    try_start_logging ${atstsfile}
    ndis_status
    if [ "$1" != "1" ]; then
        stop_logging
    fi
}

# dhcp
do_dhcp()
{
    echo -n "telit_qmi: dhcp ${netintf} "
    udhcpc -i ${netintf} >& ${dhcplogfile}
}

# stop dhcp
stop_dhcp()
{
    pid=`ps x | grep udhcpc | grep ${netintf} | awk '{print $1}'`
    if [ "${pid}" != "" ]; then
        kill -9 ${pid}
    fi
}

case "$1" in
  init)
    do_init 1
    ;;
  connect)
    check_netintf_name
    do_connect 0
    do_dhcp
    sleep 5
    do_ndis_status 0
    #dns_status
    ;;
  disconnect)
    check_netintf_name
    load_state
    connected=1
    if [ -z "$CID" ] || [ -z "$PDH" ]; then
        connected=0
    fi
    if [ "${connected}" != "0" ]; then
        stop_dhcp
        do_disconnect 0
        echo -n "Shutting down ${netintf} "
        ifconfig ${netintf} 0.0.0.0
        ifconfig ${netintf} down
    fi
    ;;
  start)
    do_init 1
    check_netintf_name
    do_connect 0
    do_dhcp
    sleep 5
    do_ndis_status 0
    #dns_status
    ;;
  status)
    check_netintf_name
    do_ndis_status 0
    dns_status
    ;;
  dns)
    dns_status
    ;;
  stop)
    check_netintf_name
    load_state
    connected=1
    if [ -z "$CID" ] || [ -z "$PDH" ]; then
        connected=0
    fi
    if [ "${connected}" != "0" ]; then
        stop_dhcp
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
