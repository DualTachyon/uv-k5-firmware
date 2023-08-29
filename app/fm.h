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

#ifndef APP_FM_H
#define APP_FM_H

#include "driver/keyboard.h"

#define FM_CHANNEL_UP	0x01
#define FM_CHANNEL_DOWN	0xFF

extern uint16_t gFM_Channels[20];
extern bool gFmRadioMode;

bool FM_CheckValidChannel(uint8_t Channel);
uint8_t FM_FindNextChannel(uint8_t Channel, uint8_t Direction);
int FM_ConfigureChannelState(void);
void FM_TurnOff(void);
void FM_EraseChannels(void);

void FM_Tune(uint16_t Frequency, int8_t Step, bool bFlag);
void FM_Play(void);

void FM_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void FM_Key_EXIT(bool bKeyPressed, bool bKeyHeld);
void FM_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Step);

#endif

