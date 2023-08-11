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

#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdint.h>

extern const uint32_t *gUpperLimitFrequencyBandTable;
extern const uint32_t *gLowerLimitFrequencyBandTable;

extern bool gSetting_350TX;
extern bool gSetting_KILLED;
extern bool gSetting_200TX;
extern bool gSetting_500TX;
extern bool gSetting_350EN;
extern bool gSetting_ScrambleEnable;
extern uint8_t gSetting_F_LOCK;

extern uint32_t gCustomPasswordKey[4];

extern bool bIsCheckExistingPassword;

extern uint8_t gEEPROM_1EC0_0[8];
extern uint8_t gEEPROM_1EC0_1[8];
extern uint8_t gEEPROM_1EC0_2[8];
extern uint8_t gEEPROM_1EC0_3[8];

extern uint8_t gEEPROM_1EC8_0[8];
extern uint8_t gEEPROM_1EC8_1[8];
extern uint8_t gEEPROM_1EC8_2[8];

extern uint16_t gBatteryCalibration[6];
extern uint16_t gADC_CH4_BootValue;
extern uint16_t gADC_CH4[4];
extern uint16_t gADC_CH9;

extern uint16_t gEEPROM_1F8A;
extern uint16_t gEEPROM_1F8C;
extern uint8_t gEEPROM_1F8E;

extern uint8_t gMR_ChannelParameters[207];

extern uint8_t g_2000036B;
extern uint8_t g_2000036F;
extern uint8_t g_20000381;

extern bool gIsNoaaMode;
extern bool gNoaaChannel;

extern uint8_t gCodeType;
extern uint8_t gCode;

#endif

