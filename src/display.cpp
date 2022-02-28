/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "display.h"
#include "mbed.h"
#include "vt100.h"
#include <iomanip>
#include <ios>
#include <iostream>

typedef struct {
  int type;    /* The type of data 0 = Temperature   */
               /*                 10 = Light Level   */
               /*                 20 = ToDo          */
  float value; /* AD result of measured temperature  */
} msg_val_t;
typedef struct {
  int type;    /* The topic        0 = Temperature   */
               /*                 10 = Light Level   */
               /*                 20 = ToDo          */
  string text; /* AD result of measured temperature  */
} msg_txt_t;

extern struct dataSet myData;
static MemoryPool<msg_val_t, 32> mpool;
static Queue<msg_val_t, 32> queue;
bool displayUp = false;

void displaySendUpdateSensor(int topic, float reading) {
  msg_val_t *message = mpool.try_alloc();
  if (message) {
    message->type = topic;
    message->value = reading;
    queue.try_put(message);
  }
}

void displayThread(void) {
  cout << "\033c"; // Reset terminal
  ThisThread::sleep_for(500ms);
  cout << "\033)A"; // Select UK Character Set
  ThisThread::sleep_for(100ms);
  //    cout << "\033(0";  // Select Graphics set 0
  //    ThisThread::sleep_for(10ms);
  cout << "\033[?25l"; // Hide Cursor
  ThisThread::sleep_for(100ms);
  std::cout << "\033[2;1H" // Cursor to 1, 1 (0, 0) HOME
            << "\033[1;37m"
            << "Temperature:         C   Set Temp:         C   Heater Status:  "
               "    \r\n"
            << "Light Level:         \%   Set Light:        \%   Light Status: "
               "      \r\n";
  displayUp = true;
  while (true) {
    msg_val_t *message;
    auto event = queue.try_get(&message);
    ThisThread::sleep_for(1ms);
    if (event) {
      switch (message->type) {
      case TEMP:
        std::cout << "\033[2;15H" << std::fixed << std::setw(6)
                  << std::setprecision(1) << (message->value);
        break;
      case TEMP_SET_VALUE:
        std::cout << "\033[2;37H" << std::fixed << std::setw(6)
                  << std::setprecision(1) << (message->value);
        break;
      case HEATER_STATUS:
        std::cout << "\033[2;63H"
                  << (static_cast<bool>(message->value)
                          ? "\033[1;31mON  \033[1;37m"
                          : "\033[1;32mOFF\033[1;37m");
        break;
      case LIGHT:
        std::cout << "\033[3;15H" << std::fixed << std::setw(6)
                  << std::setprecision(1) << (message->value);
        break;
      case LIGHT_SET_VALUE:
        std::cout << "\033[3;37H" << std::fixed << std::setw(6)
                  << std::setprecision(1) << (message->value);
        break;
      case LIGHT_STATUS:
        std::cout << "\033[3;63H"
                  << (static_cast<bool>(message->value)
                          ? "\033[1;31mON  \033[1;37m"
                          : "\033[1;32mOFF\033[1;37m");
        break;
      default:
        break;
      }
      mpool.free(message);
    }
  }
}
