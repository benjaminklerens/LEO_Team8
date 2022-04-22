# !bin/sh
read INPUT </dev/ttyACM0
echo $INPUT

curl -d "api_key=62DAJQ7PT0ON4FR8&field1=$INPUT" https://api.thingspeak.com/update

