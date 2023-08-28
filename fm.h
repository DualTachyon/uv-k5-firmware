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

#ifndef FM_H
#define FM_H

#include <stdbool.h>
#include <stdint.h>

#define FM_CHANNEL_UP	0x01
#define FM_CHANNEL_DOWN	0xFF

extern uint16_t gFM_Channels[20];
extern bool gFmRadioMode;

bool FM_CheckValidChannel(uint8_t Channel);
uint8_t FM_FindNextChannel(uint8_t Channel, uint8_t Direction);
int FM_ConfigureChannelState(void);
void FM_TurnOff(void);
void FM_EraseChannels(void);

#endif

