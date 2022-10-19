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

struct dataSet myData;


int main(void) {

    char buffer[80];
    GUI_Init();
    GUI_Clear();
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetTextAlign(GUI_TA_CENTER);
    GUI_DispStringAt("CITY3032", 120, 10);

}