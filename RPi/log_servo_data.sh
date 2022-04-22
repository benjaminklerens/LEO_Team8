#! bin/bash
LOG_FILE=/home/pi/logdata/servo_data.log
sound=$(mosquitto_sub -d -h localhost -p 1883 -u leoteam8 -P leoteam8 -C 1 -t leoteam8/sound | grep -v "Client" | grep -v "Subscribed")
position=$(mosquitto_sub -d -h localhost -p 1883 -u leoteam8 -P leoteam8 -C 1 -t leoteam8/position | grep -v "Client" | grep -v "Subscribed")
timestamp=$(date +%s)

#echo "$timestamp, $battery, $position">>$LOG_FILE

sed -i "1i$timestamp, $sound, $position" $LOG_FILE
