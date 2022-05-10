/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "GUI.h"
#include <ios>
#include <iostream>
#include "string.h"
#include "config.h"
#include <iomanip>
#include "display.h"
#include "vt100.h"

#define CLEAR "CLS"
typedef struct {
    int     type;           /* The type of data 0 = Temperature   */
                            /*                 10 = Light Level   */
                            /*                 20 = ToDo          */
    float   value;          /* AD result of measured temperature  */
} message_t;
typedef struct {
    int     x;              /* x position 1 to 80                 */
    int     y;              /* y position 1 to 24                 */

    char    txt[80];        /* Text array to display              */
}txt_t;

extern struct dataSet myData;
static MemoryPool<txt_t, 32> tpool;
static Queue<txt_t, 32> tqueue;
bool displayUp = false;

void displayText(char * text, int xPos, int yPos) {
    txt_t *txtMsg = tpool.try_alloc();
    if(txtMsg) {
        strcpy(txtMsg->txt, text);
        txtMsg->x = xPos;
        txtMsg->y = yPos;
        tqueue.try_put(txtMsg);
    }
}

void displayThread(void)
{
    cout << "\033c" ;  // Reset terminal
    ThisThread::sleep_for(200ms);
    cout << "\033)A";  // Select UK Character Set
    ThisThread::sleep_for(10ms);
    cout << "\033[?25l" ;  // Hide Cursor
#ifdef TARGET_CY8CKIT_062_WIFI_BT
    GUI_Init();
    GUI_Clear();
    GUI_SetFont(GUI_FONT_10_1);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetFont(GUI_FONT_13B_1);
//    GUI_SetTextAlign(GUI_TA_CENTER);
    GUI_DispStringAt("Telemetry Data", 0, 0);
    GUI_DispStringAt(THING_NAME, 320 - 55, 0);
    GUI_DispStringAt("MQTT Broker:", 320, 0);
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_DispStringAt(MQTT_BROKER, 320, 0);
#endif
    while (true) {
        txt_t *txtMsg;
        auto tevent = tqueue.try_get(&txtMsg);
        ThisThread::sleep_for(1ms);
        if (tevent) {
            if ((txtMsg->txt[0] == 'C')&&
                (txtMsg->txt[1] == 'L')&&
                (txtMsg->txt[2] == 'S')) 
            {
                std::cout << "\033[2J";
#ifdef TARGET_CY8CKIT_062_WIFI_BT
                GUI_Clear();

#endif
            } 
            else {
                std::cout << "\033[" << (txtMsg->y) << ";" << (txtMsg->x) << "H" << txtMsg->txt;
#ifdef TARGET_CY8CKIT_062_WIFI_BT
                GUI_DispStringAt(txtMsg->txt, (txtMsg->x)*6, (txtMsg->y) *12);
#endif

            }
            
        
            tpool.free(txtMsg);
 
        }
    }
}
void initDisplay() {
    displayText( CLEAR, 1, 1); // clear Screen
    updateDisplay(); // initialise readings/status display
    displayUp = true;
}
void updateDisplay() {
    char buffer[80];
    displayText( "Temperature:", 1, 2);
    displayText( "C", 21, 2);
    displayText( "Set Temp", 26, 2);
    displayText( "C", 43, 2);
//    displayText( "Heater Status:", 48, 2);
    displayText( "Light Level:", 1, 3);
    displayText( "%", 21, 3);
    displayText( "Set Light", 26, 3);
    displayText( "%", 43, 3);
 //   displayText( "Light Status:", 48, 3);
    displayText( "Sub Count:", 1, 12);
    displayText( "Pub Count:", 1, 13);
    sprintf(buffer, "%2.1f", myData.tempSet);
    displayText(buffer, 37, 2);
    sprintf(buffer, "%s", myData.heaterStatus?
                    "ON  ":"OFF");
//    displayText(buffer, 63, 2);
    sprintf(buffer, "%2.1f", myData.lightSet);
    displayText(buffer, 37, 3);
    sprintf(buffer, "%s", myData.lightStatus?
                    "ON  ":"OFF");
//    displayText(buffer, 63, 3);

    myData.updateDisplay = false;
}
