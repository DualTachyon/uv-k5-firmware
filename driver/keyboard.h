/* Copyright 2023 Manuel Jedinger
 * https://github.com/manujedi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifndef UV_K5_FIRMWARE_KEYBOARD_H
#define UV_K5_FIRMWARE_KEYBOARD_H

#include "gpio.h"
#include "systick.h"
#include "../bsp/dp32g030/gpio.h"

#define KEY_NOKEY   0xFF
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
