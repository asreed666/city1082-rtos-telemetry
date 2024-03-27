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
//InterruptIn vibration(P1_3);
DigitalOut vcc(P9_6);
DigitalOut trigger(P1_3);
InterruptIn echo(P1_4);
DigitalOut led(LED1);
DigitalOut led2(LED4);
static int timeEchoUs;
static Timer echoTime, trigTime;


void echoed() {
    echoTime.stop();
    timeEchoUs = echoTime.read_us();
    led = !led;
}

int main(void) {


    vcc = true;
    float als;
    echo.rise(&echoed);
    echo.enable_irq();
//    bool vibDetected = false;
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
        sprintf(buffer, "Echo Time %2d uS  ", timeEchoUs);
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
        GUI_FillRect(220, 230, 250, 230-(int)timeEchoUs) ;
        GUI_SetColor(GUI_RED);
        GUI_FillRect(220, 230-(int)timeEchoUs, 250, 230 );
        GUI_SetColor(GUI_WHITE);
        timeEchoUs = 0;
        trigger = 1; // start measurements
        trigTime.start();
        trigger = 0;
        while (trigTime.read_us() < 11) {;}
        echoTime.start();
        ThisThread::sleep_for(100);

    }
    printf("Done\n");

}