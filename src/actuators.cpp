#include "mbed.h"
#include "display.h"
#include "actuators.h"

extern struct dataSet myData;
extern bool displayUp;

void actuatorsThread() {
    char buffer[80];
    std::DigitalOut lightIndicator(LED3);
    std::DigitalOut heatIndicator(LED4);
    while (displayUp == false) {
        ThisThread::sleep_for(10ms);
    }
    while(true){
        if (myData.lightLevel < myData.lightSet - myData.lightThresh) {
            lightIndicator = true;
        }
        else if (myData.lightLevel > myData.lightSet + myData.lightThresh) {
            lightIndicator = false;
        }
        if (myData.temperature < myData.tempSet - myData.tempThresh) {
            heatIndicator = true;
        }
        else if (myData.temperature > myData.tempSet + myData.tempThresh) {
            heatIndicator = false;
        }
        if (myData.lightStatus != lightIndicator) {
            myData.lightStatus = lightIndicator;
            sprintf(buffer, "%s", lightIndicator?
                    "ON  ":"OFF");
            displayText(buffer, 63, 3);
        }
        if (myData.heaterStatus != heatIndicator) {
            myData.heaterStatus = heatIndicator;
            sprintf(buffer, "%s", heatIndicator?
                    "ON  ":"OFF");
            displayText(buffer, 63, 2);
        }
        ThisThread::sleep_for(100ms);
    }
}