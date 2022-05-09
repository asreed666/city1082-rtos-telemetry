
#include "mbed.h"
#include "display.h"
#include "actuators.h"

extern struct dataSet myData;
extern bool displayUp;

void actuatorsThread() {
    char buffer[80];
#ifdef TARGET_CY8CKIT_062_WIFI_BT
    DigitalOut lightIndicator(P11_1);
    DigitalOut heatIndicator(P0_3);
    bool LED_ON = 0;
    bool LED_OFF = 1;
#else
    DigitalOut lightIndicator(P12_3);
    DigitalOut heatIndicator(P0_5);
    bool LED_ON = 1;
    bool LED_OFF = 0;
#endif
    while (displayUp == false) {
        ThisThread::sleep_for(10ms);
    }
    while(true){
        if (myData.lightLevel < myData.lightSet - myData.lightThresh) {
            lightIndicator = LED_ON;
        }
        else if (myData.lightLevel > myData.lightSet + myData.lightThresh) {
            lightIndicator = LED_OFF;
        }
        if (myData.temperature < myData.tempSet - myData.tempThresh) {
            heatIndicator = LED_ON;
        }
        else if (myData.temperature > myData.tempSet + myData.tempThresh) {
            heatIndicator = LED_OFF;
        }
        if (myData.lightStatus != (lightIndicator ^ TARGET_CY8CKIT_062_WIFI_BT)) {
            myData.lightStatus = (lightIndicator ^ TARGET_CY8CKIT_062_WIFI_BT);
            sprintf(buffer, "%s", (lightIndicator ^ TARGET_CY8CKIT_062_WIFI_BT)?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
            displayText(buffer, 63, 3);
        }
        if (myData.heaterStatus != (heatIndicator ^ TARGET_CY8CKIT_062_WIFI_BT)) {
            myData.heaterStatus = (heatIndicator ^ TARGET_CY8CKIT_062_WIFI_BT);
            sprintf(buffer, "%s", (heatIndicator ^ TARGET_CY8CKIT_062_WIFI_BT)?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
            displayText(buffer, 63, 2);
        }
        ThisThread::sleep_for(100ms);
    }
}