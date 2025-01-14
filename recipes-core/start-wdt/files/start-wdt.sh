#!/bin/sh
# check can read MCU
TIMEOUT=5
dev_path=$1
if [ -f /nohup.out ]; then
    rm /nohup.out
fi
getVersion() {
    MCU_version=$(/usr/bin/mcu-utils ${dev_path} info | sed -n 3p | cut -d " " -f 2 | sed 's/[[:punct:]]//g')
}
startWDT() {
    echo -en "s_wdt\n\r" > ${dev_path}
    printf "s_wdt\n\r" > ${dev_path}
}
checkStatus() {
    MCU_WDT=$(/usr/bin/mcu-utils ${dev_path} wdt | grep "enable")
}

while [ -z "$MCU_version" ]  # Can`t get version
do
    getVersion
    sleep 1
    TIMEOUT=$(( TIMEOUT - 1 ))
    #echo -n "Timeout = $TIMEOUT"
    if [ $TIMEOUT -eq 0 ]; then
       echo -n "MCU READ ERROR"
       exit 0
    fi
done

echo "MCU version:$MCU_version"

# version should be later 1.0.0
if [ $MCU_version -ge 100 ]; then
    startWDT
    checkStatus
    # check wdt enable or not
    TIMEOUT=5
    while [ -z "$MCU_WDT" ]  # WDT not enable
    do
        startWDT
        checkStatus
        sleep 1
        TIMEOUT=$(( TIMEOUT - 1 ))
        #echo -n "Timeout = $TIMEOUT"
        if [ $TIMEOUT -eq 0 ]; then
           echo -n "MCU WDT ENABLE ERROR"
           exit 0
        fi
    done
    echo -n "MCU start WDT"
else
    echo -n "MCU not support WDT"
fi
