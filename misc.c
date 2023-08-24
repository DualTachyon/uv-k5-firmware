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
uint8_t gSetting_F_LOCK;
bool gSetting_ScrambleEnable;
uint8_t gSetting_F_LOCK;

uint32_t gCustomPasswordKey[4];

bool bIsCheckExistingPassword;

uint8_t gEEPROM_1EC0_0[8];
uint8_t gEEPROM_1EC0_1[8];
uint8_t gEEPROM_1EC0_2[8];
uint8_t gEEPROM_1EC0_3[8];

uint8_t gEEPROM_RSSI_CALIB[3][4];

uint16_t gEEPROM_1F8A;
uint16_t gEEPROM_1F8C;

uint8_t gMR_ChannelParameters[207];

volatile bool gNextTimeslice500ms;
volatile uint16_t g_2000032E;
volatile uint16_t g_2000033A;
volatile uint16_t gTxTimerCountdown = 1000;
volatile uint16_t g_20000342;
volatile uint16_t g_2000034C;
volatile uint16_t g_20000356;
uint16_t g_20000362;
uint8_t g_2000036B;
uint8_t g_2000036D;
uint8_t g_2000036E;
uint8_t g_2000036F;
uint8_t g_20000370;
uint8_t g_20000371[2];
uint8_t g_20000373;
uint8_t g_20000375;
uint8_t g_20000376;
uint8_t g_20000377;
uint8_t gVFO_RSSI_Level[2];
uint8_t g_2000037E;
volatile uint8_t g_20000381;
uint8_t g_20000382;
uint8_t g_20000383;
uint16_t g_2000038E;
uint8_t gKeypadLocked;
uint8_t g_200003AA;
bool g_CDCSS_Lost;
bool g_CTCSS_Lost;
bool g_CxCSS_TAIL_Found;
bool g_VOX_Lost;
uint8_t g_200003B0;
uint16_t g_200003B6;
uint8_t g_200003BC;
uint8_t g_200003BD;
uint8_t g_200003BE;
uint8_t g_200003C3;
volatile uint16_t g_200003E4;
uint8_t g_200003FD;
uint8_t g_20000400;
uint8_t g_20000410;
uint8_t g_20000411;
uint8_t g_20000413;
uint8_t g_20000415;
uint8_t g_20000416;
uint8_t g_2000041F;
uint8_t g_20000420;
uint8_t g_20000427;
uint8_t g_2000042C;
uint8_t g_2000042D;
uint8_t gDTMF_WriteIndex;
uint8_t g_20000438;
uint8_t gMenuListCount;
uint8_t g_20000458;
uint8_t g_2000045A;
uint8_t g_2000045B;
uint8_t g_2000045C;
uint8_t g_20000461;
uint8_t g_20000464;
uint8_t g_20000474;
char g_20000D1C[15];
char gDTMF_Received[16];
bool gIsDtmfContactValid;
char gDTMF_ID[4];
char gDTMF_Contact0[4];
char gDTMF_Contact1[4];
uint8_t g_CalloutAndDTMF_State;

bool gIs_A_Scan;
bool gIsNoaaMode;
volatile bool gNextTimeslice;
uint8_t gNoaaChannel;
bool gUpdateDisplay;
bool gIsFmRadioEnabled;
uint8_t gA_Scan_Channel;
uint8_t gDebounceCounter;
uint8_t gDTMF_AUTO_RESET_TIME;
uint8_t gF_LOCK;
char gNumberForPrintf[8];
uint8_t gNumberOffset;
uint8_t gScanChannel;
uint32_t gScanFrequency;
uint8_t gScanPauseMode;
uint8_t gScanState;
uint8_t gShowChPrefix;
volatile uint16_t gSystickCountdown2;
volatile uint8_t gSystickCountdown3;
volatile uint8_t gSystickCountdown4;
volatile uint16_t gSystickCountdown11;
volatile bool gSystickFlag0;
volatile bool gNextTimeslice40ms;
volatile bool gSystickFlag5;
volatile bool gBatterySaveCountdownExpired;
volatile bool gSystickFlag7;
volatile bool gSystickFlag8;
volatile bool gSystickFlag9;
volatile bool gSystickFlag10;
volatile bool gSystickFlag11;

volatile uint16_t ScanPauseDelayIn10msec;

// This probably should be signed! But Ghidra disgrees
uint16_t gFM_FrequencyDeviation;

uint16_t gCurrentRSSI;

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

