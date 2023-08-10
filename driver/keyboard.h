//
// Created by Manuel Jedinger on 8/10/23.
//

#ifndef UV_K5_FIRMWARE_KEYBOARD_H
#define UV_K5_FIRMWARE_KEYBOARD_H

#include "gpio.h"
#include "systick.h"
#include "../bsp/dp32g030/gpio.h"

#define KEY_NOKEY   24
#define KEY_M       10
#define KEY_UP      11
#define KEY_DOWN    12
#define KEY_EXIT    13
#define KEY_0       0
#define KEY_1       1
#define KEY_2       2
#define KEY_3       3
#define KEY_4       4
#define KEY_5       5
#define KEY_6       6
#define KEY_7       7
#define KEY_8       8
#define KEY_9       9
#define KEY_STAR    14
#define KEY_F       15
#define KEY_FN1     23
#define KEY_FN2     22
#define KEY_PPT     21


void Keyboard_init();
unsigned int PollKeyboard();


#endif //UV_K5_FIRMWARE_KEYBOARD_H
