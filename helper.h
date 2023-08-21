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

#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include "driver/keyboard.h"

extern KEY_Code_t gKeyReading0;
extern KEY_Code_t gKeyReading1;
extern uint8_t g_2000042A;

uint8_t HELPER_GetKey(void);
void HELPER_CheckBootKey(uint8_t KeyType);

#endif

