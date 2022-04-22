#! bin/bash
LOG_FILE=/home/pi/logdata/battery.txt
battery=$(mosquitto_sub -d -h localhost -p 1883 -u leoteam8 -P leoteam8 -C 1 -t leoteam8/voltage | grep -v "Client" | grep -v "Subscribed")
timestamp=$(date +%s)

#echo "$timestamp, $battery">>$LOG_FILE

sed -i "1i$timestamp, $battery" $LOG_FILE
