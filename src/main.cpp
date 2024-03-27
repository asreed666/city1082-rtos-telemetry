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
#include "platform.h"

#define ALS_OUT P10_0

AnalogIn lightLevel(ALS_OUT);
AnalogIn temperatureVoltage(P10_6);
InterruptIn vibration(P1_3);
DigitalOut vcc(P9_6);
DigitalOut vcc2(P1_4);
DigitalOut led(LED1);
DigitalOut led2(LED4);

static int vibCount = 0;

void vibrated() {
    vibCount++;
    led = !led;
    led2 = 1;
}

int main(void) {


    vcc = true;
    vcc2 = true;
    led2 = 0;
    float als;
    vibration.rise(&vibrated);
    vibration.enable_irq();
    bool vibDetected = false;
//    int vibCount = 0;
    float temperature;
    char buffer[80];
    printf("Starting\n");
    GUI_Init();
    GUI_Clear();
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextAlign(GUI_TA_CENTER);
    GUI_DispStringAt("CITY3032", 160, 10);
//    GUI_DrawCircle(100, 100, 20) ;
//    GUI_DrawRect(1,1,20,120);
//    GUI_DrawFrame(120,120,220,220, 5);
    while(true) {
        als = lightLevel.read()*100;

        temperature = (temperatureVoltage.read() * 220) - 50;
        sprintf(buffer, "Light level %3.0f%c  ", als, '%');
        GUI_DispStringAt(buffer, 50, 40);
        sprintf(buffer, "Temperature is %2.1fC  ", temperature);
        GUI_DispStringAt(buffer, 50, 60);
        sprintf(buffer, "Vibrations/ sec detected %2d  ", vibCount*10);
        GUI_DispStringAt(buffer, 50, 80);

//        GUI_DrawFrame(15, 235, 55, 10, 4);
        GUI_SetColor(GUI_GREEN);
        GUI_FillRect(20, 130, 50, 230-(int)als) ;
        GUI_SetColor(GUI_CYAN);
        GUI_FillRect(20, 230-(int)als, 50, 230 );
        GUI_SetColor(GUI_BLUE);
        GUI_FillRect(120, 130, 150, 230-(int)temperature*2) ;
        GUI_SetColor(GUI_RED);
        GUI_FillRect(120, 230-(int)temperature*2, 150, 230 );
        GUI_SetColor(GUI_GREEN);
        GUI_FillRect(220, 230, 250, 230-(int)vibCount) ;
        GUI_SetColor(GUI_RED);
        GUI_FillRect(220, 230-(int)vibCount, 250, 230 );
        GUI_SetColor(GUI_WHITE);
        vibCount = 0;
        ThisThread::sleep_for(100);

    }
    printf("Done\n");

}