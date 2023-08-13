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

#include "misc.h"

const uint32_t *gUpperLimitFrequencyBandTable;
const uint32_t *gLowerLimitFrequencyBandTable;

bool gSetting_350TX;
bool gSetting_KILLED;
bool gSetting_200TX;
bool gSetting_500TX;
bool gSetting_350EN;
bool gSetting_ScrambleEnable;
uint8_t gSetting_F_LOCK;

uint32_t gCustomPasswordKey[4];

bool bIsCheckExistingPassword;

uint8_t gEEPROM_1EC0_0[8];
uint8_t gEEPROM_1EC0_1[8];
uint8_t gEEPROM_1EC0_2[8];
uint8_t gEEPROM_1EC0_3[8];

uint8_t gEEPROM_1EC8_0[8];
uint8_t gEEPROM_1EC8_1[8];
uint8_t gEEPROM_1EC8_2[8];

uint16_t gEEPROM_1F8A;
uint16_t gEEPROM_1F8C;
uint8_t gEEPROM_1F8E;

uint8_t gMR_ChannelParameters[207];

uint8_t g_2000036B;
uint8_t g_2000036F;
uint8_t g_20000381;

uint8_t g_200003AA;
uint8_t g_2000042F;
uint8_t g_200003AE;
uint8_t g_200003AB;
uint8_t g_200003AD;
uint8_t g_200003AF;
uint8_t g_200003B0;
uint8_t g_20000342;
uint8_t gSystickFlag10;
uint8_t g_20000375;
uint8_t g_20000376;
uint8_t gSystickCountdown4;
uint8_t gSystickCountdown3;
uint8_t g_20000377;
uint8_t gSystickCountdown2;

uint8_t gCopyOfCodeType;

uint8_t g_20000D0C[16];

bool gIsNoaaMode;
bool gNoaaChannel;

uint8_t gCodeType;
uint8_t gCode;

uint16_t g_200003B6;

