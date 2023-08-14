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

#include <string.h>
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

uint16_t g_2000032E;
uint8_t g_20000342;
uint8_t g_20000356;
uint8_t g_2000036B;
uint8_t g_2000036E;
uint8_t g_2000036F;
uint8_t g_20000370;
uint8_t g_20000375;
uint8_t g_20000376;
uint8_t g_20000377;
uint8_t g_20000378;
uint8_t g_20000379;
uint8_t g_2000037E;
uint8_t g_20000381;
uint16_t g_2000038E;
uint8_t g_200003A9;
uint8_t g_200003AA;
uint8_t g_200003AB;
uint8_t g_200003AD;
uint8_t g_200003AE;
uint8_t g_200003AF;
uint8_t g_200003B0;
uint16_t g_200003B6;
uint8_t g_200003BC;
uint8_t g_200003BE;
uint8_t g_20000400;
uint8_t g_2000042F;
uint8_t g_2000044C;
uint8_t g_20000458;
uint8_t g_2000045A;
uint8_t g_2000045B;
uint8_t g_2000045C;
uint8_t g_20000461;
uint8_t g_20000464;
uint8_t g_20000D0C[16];

bool gIs_A_Scan;
bool gIsNoaaMode;
bool gMaybeVsync;
bool gNoaaChannel;
bool gThisCanEnable_BK4819_Rxon;
bool gUpdateDisplay;
uint8_t gA_Scan_Channel;
uint8_t gCode;
uint8_t gCodeType;
uint8_t gCopyOfCodeType;
uint8_t gDebounceCounter;
uint8_t gDTMF_AUTO_RESET_TIME;
uint8_t gF_LOCK;
char gNumberForPrintf[8];
uint8_t gNumberOffset;
uint8_t gScanChannel;
uint32_t gScanFrequency;
uint8_t gScanState;
uint8_t gShowChPrefix;
uint8_t gSystickCountdown2;
uint8_t gSystickCountdown3;
uint8_t gSystickCountdown4;
uint8_t gSystickFlag10;
uint8_t gSystickFlag5;
uint8_t gSystickFlag6;
uint8_t gSystickFlag8;

// --------

void NUMBER_Append(char Digit)
{
	if (gNumberOffset == 0) {
		memset(gNumberForPrintf, 10, sizeof(gNumberForPrintf));
	} else if (gNumberOffset >= sizeof(gNumberForPrintf)) {
		return;
	}
	gNumberForPrintf[gNumberOffset++] = Digit;
}

void NUMBER_Get(char *pDigits, uint32_t *pInteger)
{
	uint32_t Value;
	uint32_t Multiplier;
	uint8_t i;

	Multiplier = 10000000;
	Value = 0;
	for (i = 0; i < 8; i++) {
		if (pDigits[i] > 9) {
			break;
		}
		Value += pDigits[i] * Multiplier;
		Multiplier /= 10U;
	}
	*pInteger = Value;
}

void NUMBER_ToDigits(uint32_t Value, char *pDigits)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		uint32_t Result = Value / 10U;

		pDigits[7 - i] = Value - (Result * 10U);
		Value = Result;
	}
}

