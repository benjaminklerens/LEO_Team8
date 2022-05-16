#! bin/bash
sound=$(mosquitto_sub -d -h localhost -p 1883 -u leoteam8 -P leoteam8 -C 1 -t leoteam8/sound | grep -v "Client" | grep -v "Subscribed")
threshold=700

if [ $sound -lt $threshold ]
then
mosquitto_pub -h localhost -p 1883 -u leoteam8 -P leoteam8 -t leoteam8/led -m 0
else
mosquitto_pub -h localhost -p 1883 -u leoteam8 -P leoteam8 -t leoteam8/led -m 1
fi

