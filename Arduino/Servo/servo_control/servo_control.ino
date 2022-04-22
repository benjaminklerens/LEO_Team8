/*
 * Linux for Embedded Objects Mini Project
 * Group 8
 * 
 */


//libraries
#include <ESP8266WiFiMulti.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>

//WiFi
#define WIFI_SSID       "LEO1_TEAM_08"
#define WIFI_PASSWORD    "leoteam8"

//mqtt
#define MQTT_SERVER           "192.168.10.1"
#define MQTT_SERVERPORT       1883 
#define MQTT_USERNAME         "leoteam8"
#define MQTT_KEY              "leoteam8"
#define MQTT_TOPIC_SERVO_POS  "/position"
#define MQTT_TOPIC_SOUND      "/sound"
#define MQTT_TOPIC_STATUS     "/status"
#define MQTT_TOPIC_WIFI_SNR   "/snr"
#define MQTT_TOPIC_LED        "/led"


//GPIO
#define SERVO_PIN 14
#define LED_PIN 5
#define SOUND_SENSOR_PIN 4

//timing
#define SENSE_TIME 2000 // = Publish Time
#define MOVE_TIME 2000
#define SENSE_MOVE_SHIFT 1000
#define ANGLE_STEPS 45

//functions
void debug(const char *s);
void mqtt_connect();
void print_wifi_status();
void publish_data();
void subscribe_led();
void subscribe_status();


//variables
ESP8266WiFiMulti WiFiMulti;
Servo servo;
WiFiClient wifi_client;

Adafruit_MQTT_Client mqtt(&wifi_client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

Adafruit_MQTT_Publish pos_mqtt_publish = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME MQTT_TOPIC_SERVO_POS);
Adafruit_MQTT_Publish sound_mqtt_publish = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME MQTT_TOPIC_SOUND);
Adafruit_MQTT_Publish snr_mqtt_publish = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME MQTT_TOPIC_WIFI_SNR);

Adafruit_MQTT_Subscribe status_mqtt_subscribe = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME MQTT_TOPIC_STATUS);
Adafruit_MQTT_Subscribe led_mqtt_subscribe = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME MQTT_TOPIC_STATUS);

uint32_t move_prev_time;
uint32_t sense_prev_time;
int16_t servo_pos = 0;
int16_t sound = 0;
uint8_t active_sensing = 0;


const uint16_t conn_tout_ms = 5000;

void setup() {
  //init time
  move_prev_time = millis()+SENSE_MOVE_SHIFT;
  sense_prev_time = millis();
  //set PinMode
  pinMode(LED_PIN,OUTPUT);
  pinMode(SOUND_SENSOR_PIN,INPUT);
  servo.attach(SERVO_PIN);
  
  //subscribe to mqtt topics
  mqtt.subscribe(&status_mqtt_subscribe);
  mqtt.subscribe(&led_mqtt_subscribe);
  //enable serial connection and begin boot process
  Serial.begin(115200);
  delay(10);
  debug("Boot");
  //set up wifi connection
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  if(WiFiMulti.run(conn_tout_ms) == WL_CONNECTED)
  {
    print_wifi_status();
  }
  else
  {
    debug("Unable to connect");
  }

}

void loop() {
  //ensure that mqtt is connected
  mqtt_connect();
  //read topics from mqtt

  //reset position after sensing all positions
  if (servo_pos > 180 - ANGLE_STEPS)
  {
    servo_pos = 0;
    servo.write(servo_pos);
    move_prev_time = millis()+SENSE_MOVE_SHIFT;
    sense_prev_time = millis();
  }
  
  //check if sensing is enabled
  if(!active_sensing)
  {
    move_prev_time = millis()+SENSE_MOVE_SHIFT;
    sense_prev_time = millis();
    
  }else
  {
    //check sensing interval
    if (millis() - move_prev_time >= SENSE_TIME)
    {
      sense_prev_time = millis();
      sound = digitalRead(SOUND_SENSOR_PIN);
      publish_data();
    }    
    //check movement interval
    if (millis() - move_prev_time >= MOVE_TIME)
    {
      move_prev_time = millis();
      servo_pos += ANGLE_STEPS;
      servo.write(servo_pos);
    }

  }
  
  

}

void mqtt_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (! mqtt.connected())
  {
    debug("Connecting to MQTT... ");
    while ((ret = mqtt.connect()) != 0)
    { // connect will return 0 for connected
         Serial.println(mqtt.connectErrorString(ret));
         debug("Retrying MQTT connection in 5 seconds...");
         mqtt.disconnect();
         delay(5000);  // wait 5 seconds
    }
    debug("MQTT Connected");
  }
}


void publish_data()
{
  char pos_payload[16];
  char sound_payload[16];
  char snr_payload[16];
  
  int snr = WiFi.RSSI();
  
  sprintf(pos_payload, "%ld", servo_pos);
  sprintf(sound_payload, "%ld", sound);
  sprintf(snr_payload, "%ld", snr);
  
  Serial.print(millis());
  Serial.print(" ");
  Serial.println(" Publishing: ");
  
  Serial.print("Position: ");
  Serial.println(pos_payload);
  Serial.print("Sound: ");
  Serial.println(sound_payload);
  Serial.print("WiFi SNR: ");
  Serial.println(snr_payload);


  Serial.print(millis());
  Serial.print(" ");
  Serial.println(" Connecting...");
  
  if((WiFiMulti.run(conn_tout_ms) == WL_CONNECTED))
  {
    print_wifi_status();

    //publish to sound topic
    if (! sound_mqtt_publish.publish(sound_payload))
    {
      debug("Sound:MQTT failed");
    }
    else
    {
      debug("Sound:MQTT ok");
    }
    
    //publish to position topic
    if (! pos_mqtt_publish.publish(pos_payload))
    {
      debug("Position:MQTT failed");
    }
    else
    {
      debug("Position:MQTT ok");
    }

    //publish to snr topic
    if (! snr_mqtt_publish.publish(snr_payload))
    {
      debug("SNR:MQTT failed");
    }
    else
    {
      debug("SNR:MQTT ok");
    }
    
  }
}

void subscribe_led()
{
  int16_t sub_timeout = 10;
  Adafruit_MQTT_Subscribe* sub;
  
  if((WiFiMulti.run(conn_tout_ms) == WL_CONNECTED))
  {   
    while ((sub = mqtt.readSubscription(10))) 
    {
      if (sub == &led_mqtt_subscribe)
      {
        Serial.print("Received LED Mode: ");
        Serial.println((char*)led_mqtt_subscribe.lastread);      
      
        // Switch on the LED if an 1 was received as first character
        if ((int)(led_mqtt_subscribe.lastread[0]) == 49)
        {
          digitalWrite(LED_PIN, HIGH);
          
        }
        if (led_mqtt_subscribe.lastread[0] == 48) 
        {
          digitalWrite(LED_PIN, LOW);
        }

      }
    }
  }
}

void subscribe_status()
{
  int16_t sub_timeout = 10;
  Adafruit_MQTT_Subscribe* sub;
  
  if((WiFiMulti.run(conn_tout_ms) == WL_CONNECTED))
  {   
    while ((sub = mqtt.readSubscription(10))) 
    {
      if (sub == &status_mqtt_subscribe)
      {
        Serial.print("Received Status Message: ");
        Serial.println((char*)status_mqtt_subscribe.lastread);      
      
        //Setting system active depending on received message
        if ((int)(led_mqtt_subscribe.lastread[0]) == 49)
        {
          active_sensing = 1;  
        }
        if (led_mqtt_subscribe.lastread[0] == 48) 
        {
          active_sensing = 0;
        }

      }
    }
  }
}

void debug(const char *s)
{
  Serial.print (millis());
  Serial.print (" ");
  Serial.println(s);
}

void print_wifi_status()
{
  Serial.print (millis());
  Serial.print(" WiFi connected: ");
  Serial.print(WiFi.SSID());
  Serial.print(" ");
  Serial.print(WiFi.localIP());
  Serial.print(" RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}
