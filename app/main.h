/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
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

#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/keyboard.h"

void MAIN_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_EXIT(bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_MENU(bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_STAR(bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction);

#endif

