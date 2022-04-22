#!/bin/bash

LOG_FILE=/home/pi/logdata/health.txt
#exec > >(tee -a $LOG_LOCATION/health.txt)

temp=$(vcgencmd measure_temp)
timestamp=$(date +%s)
storage=$(df / -kh |grep -v "Filesystem" | awk '{ print $5 }' | sed 's/%//g')

#echo "$timestamp, $temp, storage used: $storage%"
sed -i "1i$timestamp, $temp, storage used: $storage%" $LOG_FILE

