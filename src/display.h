#ifndef DISPLAY_H
#define DISPLAY_H

#define TEMP 0
#define TEMP_LOW_THRESH 1
#define TEMP_HIGH_THRESH 2
#define TEMP_SET_VALUE 3
#define HEATER_STATUS 4
#define LIGHT 10
#define LIGHT_LOW_THRESH 11
#define LIGHT_HIGH_THRESH 12
#define LIGHT_SET_VALUE 13
#define LIGHT_STATUS 14
#define HUMID 20
#define HUMID_LOW_THRESH 21
#define HUMID_HIGH_THRESH 22
#define HUMID_SET_VALUE 23
#define WIFI_STATUS 30
#define MQTT_STATUS 31
#define SERVICE_STATUS 32


struct dataSet{
    float temperature;
    float tempSet = 24;
    float tempThresh = 0.5;
    bool heaterStatus = false;
    float lightLevel;
    float lightSet = 40;
    float lightThresh = 5;
    char gpsLat[32];
    char gpsLong[32];
    float pressure;
    float motion;
    bool lightStatus = false;
    bool wifiStatus = false;
    bool mqttStatus = false;
    bool updateDisplay = false;
    bool serviceStatus = false;
};

void displayThread();
void displayText(char*, int, int);
void initDisplay(void);
void updateDisplay(void);


#endif