/*
 * Edit this file updating your Thing name and Topic definitions
 */

#define THING_NAME "CCP06"  // change this to a unique IOT device name for the MQTT Broker
#define LIGHT_SET_TOPIC "/lightSet"
#define LIGHT_LEVEL_SET_TOPIC "CCP06/lightSet"
#define LIGHT_LEVEL_TOPIC "/lightLevel"
#define ANNOUNCE_TOPIC "announce"
#define TEMP_SET_TOPIC "/tempSet"
#define TEMPERATURE_SET_TOPIC "CCP06/tempSet"
#define TEMPERATURE_TOPIC "/temp"
#define HEATER_STATUS "/heaterStatus"
#define LIGHT_STATUS "/lightStatus"
#define GET_TIME_TOPIC "getTime"
#define TIME_TOPIC "timeStamp"
#define MQTT_BROKER "192.168.1.176"
#define MQTTClient_QOS2 1
#define LEDON 0
#define LEDOFF 1
