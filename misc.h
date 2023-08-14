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

extern uint16_t gEEPROM_1F8A;
extern uint16_t gEEPROM_1F8C;
extern uint8_t gEEPROM_1F8E;

extern uint8_t gMR_ChannelParameters[207];

extern uint16_t g_2000032E;
extern uint8_t g_20000342;
extern uint8_t g_20000356;
extern uint8_t g_2000036B;
extern uint8_t g_2000036E;
extern uint8_t g_2000036F;
extern uint8_t g_20000370;
extern uint8_t g_20000375;
extern uint8_t g_20000376;
extern uint8_t g_20000377;
extern uint8_t g_20000378;
extern uint8_t g_20000379;
extern uint8_t g_2000037E;
extern uint8_t g_20000381;
extern uint16_t g_2000038E;
extern uint8_t g_200003A9;
extern uint8_t g_200003AA;
extern uint8_t g_200003AB;
extern uint8_t g_200003AD;
extern uint8_t g_200003AE;
extern uint8_t g_200003AF;
extern uint8_t g_200003B0;
extern uint16_t g_200003B6;
extern uint8_t g_200003BC;
extern uint8_t g_200003BE;
extern uint8_t g_20000400;
extern uint8_t g_2000042F;
extern uint8_t g_2000044C;
extern uint8_t g_20000458;
extern uint8_t g_2000045A;
extern uint8_t g_2000045B;
extern uint8_t g_2000045C;
extern uint8_t g_20000461;
extern uint8_t g_20000464;
extern uint8_t g_20000D0C[16];

extern bool gIs_A_Scan;
extern bool gIsNoaaMode;
extern bool gMaybeVsync;
extern bool gNoaaChannel;
extern bool gUpdateDisplay;
extern uint8_t gA_Scan_Channel;
extern uint8_t gCode;
extern uint8_t gCodeType;
extern uint8_t gCopyOfCodeType;
extern uint8_t gDebounceCounter;
extern uint8_t gDTMF_AUTO_RESET_TIME;
extern uint8_t gF_LOCK;
extern char gNumberForPrintf[8];
extern uint8_t gNumberOffset;
extern uint8_t gScanChannel;
extern uint32_t gScanFrequency;
extern uint8_t gScanState;
extern uint8_t gShowChPrefix;
extern uint8_t gSystickCountdown2;
extern uint8_t gSystickCountdown3;
extern uint8_t gSystickCountdown4;
extern uint8_t gSystickFlag10;
extern uint8_t gSystickFlag5;
extern uint8_t gSystickFlag6;
extern uint8_t gSystickFlag8;

// --------

void NUMBER_Append(char Digit);
void NUMBER_Get(char *pDigits, uint32_t *pInteger);
void NUMBER_ToDigits(uint32_t Value, char *pDigits);

#endif

