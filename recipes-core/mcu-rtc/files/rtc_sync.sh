# get MCU time
#echo -n "SYNC SYSTEM DATE WITH MCU RTC\n"
MCU_RESULT=$(hwclock -f /dev/rtc0 | grep "20")
TIMEOUT=5
while [ -z "$MCU_RESULT" ]  # Can`t get rtc
do
    MCU_RESULT=$(hwclock -f /dev/rtc0 | grep "20")
    sleep 1
    TIMEOUT=$(( TIMEOUT - 1 ))
    #echo -n "Timeout = $TIMEOUT\n"
    if [ $TIMEOUT -eq 0 ]; then
       echo -n "RTC READ ERROR\n" 
       exit 0
    fi
done

# get Date
DATE_RESULT=$(date +%s)
SYNC_RESULT=$(cat /sys/class/rtc/rtc0/since_epoch)
if [ $DATE_RESULT -gt $SYNC_RESULT ]
then
    hwclock -f /dev/rtc0 -w
else
    hwclock -f /dev/rtc0 -s
fi

exit 0
