/*
 * Linux for Embedded Objects Mini Project
 * Group 8
 * 
 */


//libraries
#include <ESP8266WiFiMulti.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//WiFi
#define WIFI_SSID       "LEO1_TEAM_08"
#define WIFI_PASSWORD    "leoteam8"

//RFID
#define RST_PIN 0
#define SS_PIN D8

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

//functions
void debug(const char *s);
void mqtt_connect();
void print_wifi_status();
void publish_data();

//variables
ESP8266WiFiMulti WiFiMulti;
WiFiClient wifi_client;
MFRC522 mfrc522(SS_PIN, RST_PIN);
byte readCard[4];
String MasterTag = "399DBDA3"; //UID for the blue tag
String tagID = "";
int rfid_status = 0;

Adafruit_MQTT_Client mqtt(&wifi_client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);
Adafruit_MQTT_Publish status_mqtt_publish = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME MQTT_TOPIC_STATUS);

const uint16_t conn_tout_ms = 5000;

void setup() {  
  //enable serial connection and begin boot process
  Serial.begin(9600);
  while (!Serial);
  SPI.begin(); // SPI bus
  mfrc522.PCD_Init(); // MFRC522
  delay(100); // Optional delay. Some boards need some time after init to be ready.
  Serial.println(F("Please scan your card/tag..."));
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
  while (getID())
  {
    if (tagID == MasterTag)
    {
      Serial.println(F("Access Granted!"));
      rfid_status = 1;
      publish_data();
    }
    else
    {
      Serial.println(F("Access Denied.. SPY!"));
      rfid_status = 0;
      publish_data();
    }
  }
}

//Read new tag if available
boolean getID() 
{
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
  return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
  return false;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) { // The MIFARE PICCs that we use have 4 byte UID
  //readCard[i] = mfrc522.uid.uidByte[i];
  tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  debug("ok were stopping it now");
  tagID.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading
  return true;
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
  char status_payload[16];
  int snr = WiFi.RSSI();
  
  sprintf(status_payload, "%ld", rfid_status);
  
  Serial.print(millis());
  Serial.print(" ");
  Serial.println(" Publishing: ");
  
  Serial.print("RFID Status: ");
  Serial.println(rfid_status);

  Serial.print(millis());
  Serial.print(" ");
  Serial.println(" Connecting...");
  
  if((WiFiMulti.run(conn_tout_ms) == WL_CONNECTED))
  {
    print_wifi_status();

    delay(20);
    //publish to rfid status
    if (! status_mqtt_publish.publish(rfid_status))
    {
      debug("Position:MQTT failed");
    }
    else
    {
      debug("Position:MQTT ok");
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
