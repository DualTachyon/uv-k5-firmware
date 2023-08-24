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
extern uint8_t gSetting_F_LOCK;
extern bool gSetting_ScrambleEnable;
extern uint8_t gSetting_F_LOCK;

extern uint32_t gCustomPasswordKey[4];

extern bool bIsCheckExistingPassword;

extern uint8_t gEEPROM_1EC0_0[8];
extern uint8_t gEEPROM_1EC0_1[8];
extern uint8_t gEEPROM_1EC0_2[8];
extern uint8_t gEEPROM_1EC0_3[8];

extern uint8_t gEEPROM_RSSI_CALIB[3][4];

extern uint16_t gEEPROM_1F8A;
extern uint16_t gEEPROM_1F8C;

extern uint8_t gMR_ChannelParameters[207];

extern volatile bool gNextTimeslice500ms;
extern volatile uint16_t g_2000032E;
extern volatile uint16_t g_2000033A;
extern volatile uint16_t gTxTimerCountdown;
extern volatile uint16_t g_20000342;
extern volatile uint16_t g_2000034C;
extern volatile uint16_t g_20000356;
extern uint16_t g_20000362;
extern uint8_t g_2000036B;
extern uint8_t g_2000036D;
extern uint8_t g_2000036E;
extern uint8_t g_2000036F;
extern uint8_t g_20000370;
extern uint8_t g_20000371[2];
extern uint8_t g_20000373;
extern uint8_t g_20000375;
extern uint8_t g_20000376;
extern uint8_t g_20000377;
extern uint8_t gVFO_RSSI_Level[2];
extern uint8_t g_2000037E;
extern volatile uint8_t g_20000381;
extern uint8_t g_20000382;
extern uint8_t g_20000383;
extern uint16_t g_2000038E;
extern uint8_t gKeypadLocked;
extern uint8_t g_200003AA;
extern bool g_CDCSS_Lost;
extern bool g_CTCSS_Lost;
extern bool g_CxCSS_TAIL_Found;
extern bool g_VOX_Lost;
extern uint8_t g_200003B0;
extern uint16_t g_200003B6;
extern uint8_t g_200003BC;
extern uint8_t g_200003BD;
extern uint8_t g_200003BE;
extern uint8_t g_200003C3;
extern volatile uint16_t g_200003E4;
extern uint8_t g_200003FD;
extern uint8_t g_20000400;
extern uint8_t g_20000410;
extern uint8_t g_20000411;
extern uint8_t g_20000413;
extern uint8_t g_20000415;
extern uint8_t g_20000416;
extern uint8_t g_2000041F;
extern uint8_t g_20000420;
extern uint8_t g_20000427;
extern uint8_t g_2000042C;
extern uint8_t g_2000042D;
extern uint8_t gDTMF_WriteIndex;
extern uint8_t g_20000438;
extern uint8_t gMenuListCount;
extern uint8_t g_20000458;
extern uint8_t g_2000045A;
extern uint8_t g_2000045B;
extern uint8_t g_2000045C;
extern uint8_t g_20000461;
extern uint8_t g_20000464;
extern uint8_t gFSKWriteIndex;
extern uint8_t g_20000474;
extern char g_20000D1C[15];
extern char gDTMF_Received[16];
extern bool gIsDtmfContactValid;
extern char gDTMF_ID[4];
extern char gDTMF_Contact0[4];
extern char gDTMF_Contact1[4];
extern uint8_t g_CalloutAndDTMF_State;

extern bool gIs_A_Scan;
extern bool gIsNoaaMode;
extern volatile bool gNextTimeslice;
extern uint8_t gNoaaChannel;
extern bool gUpdateDisplay;
extern bool gIsFmRadioEnabled;
extern uint8_t gA_Scan_Channel;
extern uint8_t gDebounceCounter;
extern uint8_t gDTMF_AUTO_RESET_TIME;
extern uint8_t gF_LOCK;
extern char gNumberForPrintf[8];
extern uint8_t gNumberOffset;
extern uint8_t gScanChannel;
extern uint32_t gScanFrequency;
extern uint8_t gScanPauseMode;
extern uint8_t gScanState;
extern uint8_t gShowChPrefix;
extern volatile uint16_t gSystickCountdown2;
extern volatile uint8_t gSystickCountdown3;
extern volatile uint8_t gSystickCountdown4;
extern volatile uint16_t gSystickCountdown11;
extern volatile bool gSystickFlag0;
extern volatile bool gNextTimeslice40ms;
extern volatile bool gSystickFlag5;
extern volatile bool gBatterySaveCountdownExpired;
extern volatile bool gSystickFlag7;
extern volatile bool gSystickFlag8;
extern volatile bool gSystickFlag9;
extern volatile bool gSystickFlag10;
extern volatile bool gSystickFlag11;

extern volatile uint16_t ScanPauseDelayIn10msec;

extern uint16_t gFM_FrequencyDeviation;

extern uint16_t gCurrentRSSI;

// --------

void NUMBER_Append(char Digit);
void NUMBER_Get(char *pDigits, uint32_t *pInteger);
void NUMBER_ToDigits(uint32_t Value, char *pDigits);

#endif

