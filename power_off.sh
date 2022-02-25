#!/bin/bash
count=0
while :
do
	sleep 1
	status=$(bash /home/pi/bin/read_switch.sh)
	echo "SWITCH STATUS: $status, COUNT STATUS: $count "
	
	if [ $status -eq 1 ]
	then
		let "count++"
		if [ $count -eq 3 ]
		then
			sudo poweroff
		fi
	fi
done
