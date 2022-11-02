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

int main(void) {

    AnalogIn lightLevel(ALS_OUT);
    float als;
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
        sprintf(buffer, "Light level %3.0f%c", als, '%');
        GUI_DispStringAt(buffer, 50, 22);
//        GUI_DrawFrame(15, 235, 55, 10, 4);
        GUI_SetColor(GUI_GREEN);
        GUI_FillRect(20, 130, 50, 230-(int)als) ;
        GUI_SetColor(GUI_CYAN);
        GUI_FillRect(20, 230-(int)als, 50, 230 );
        GUI_SetColor(GUI_WHITE);
        ThisThread::sleep_for(100);

    }
    printf("Done\n");

}