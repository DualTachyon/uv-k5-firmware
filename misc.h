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

#define IS_MR_CHANNEL(x) ((x) >= MR_CHANNEL_FIRST && (x) <= MR_CHANNEL_LAST)
#define IS_FREQ_CHANNEL(x) ((x) >= FREQ_CHANNEL_FIRST && (x) <= FREQ_CHANNEL_LAST)
#define IS_NOAA_CHANNEL(x) ((x) >= NOAA_CHANNEL_FIRST && (x) <= NOAA_CHANNEL_LAST)
#define IS_NOT_NOAA_CHANNEL(x) ((x) >= MR_CHANNEL_FIRST && (x) <= FREQ_CHANNEL_LAST)
#define IS_VALID_CHANNEL(x) ((x) <= NOAA_CHANNEL_LAST)

enum {
	MR_CHANNEL_FIRST = 0U,
	MR_CHANNEL_LAST = 199U,
	FREQ_CHANNEL_FIRST = 200U,
	FREQ_CHANNEL_LAST = 206U,
	NOAA_CHANNEL_FIRST = 207U,
	NOAA_CHANNEL_LAST = 216U,
};

enum {
	FLASHLIGHT_OFF = 0U,
	FLASHLIGHT_ON = 1U,
	FLASHLIGHT_BLINK = 2U,
};

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

extern uint16_t gEEPROM_RSSI_CALIB[3][4];

extern uint16_t gEEPROM_1F8A;
extern uint16_t gEEPROM_1F8C;

extern uint8_t gMR_ChannelAttributes[207];

extern volatile bool gNextTimeslice500ms;
extern volatile uint16_t gBatterySaveCountdown;
extern volatile uint16_t g_2000033A;
extern volatile uint16_t gTxTimerCountdown;
extern volatile uint16_t g_20000342;
extern volatile uint16_t gFmPlayCountdown;
extern volatile uint16_t g_20000356;
extern uint16_t g_20000362;
extern uint8_t g_2000036B;
extern uint8_t gKeyLockCountdown;
extern uint8_t gRTTECountdown;
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
extern uint8_t gBatteryVoltageIndex;
extern volatile uint8_t g_20000381;
extern uint8_t g_20000382;
extern uint8_t g_20000383;
extern uint16_t g_2000038E;
extern volatile int8_t gFM_Step;
extern uint8_t g_20000393;
extern bool g_20000394;
extern uint8_t g_20000395;
extern uint8_t g_20000395;
extern uint8_t g_20000396;
extern uint8_t g_20000398;
extern uint8_t g_2000039A;
extern uint8_t g_2000039B;
extern bool gRequestSaveVFO;
extern uint8_t gRequestSaveChannel;
extern bool gRequestSaveSettings;
extern bool gRequestSaveFM;
extern uint8_t gKeypadLocked;
extern uint8_t g_200003A0;
extern bool gFlagStartScan;
extern bool gFlagStopScan;
extern bool gFlagAcceptSetting;
extern bool gFlagRefreshSetting;
extern bool gFlagSaveVfo;
extern bool gFlagSaveSettings;
extern uint8_t gFlagSaveChannel;
extern bool gFlagSaveFM;
extern uint8_t g_200003AA;
extern bool g_CDCSS_Lost;
extern uint8_t gCDCSSCodeReceived;
extern bool g_CTCSS_Lost;
extern bool g_CxCSS_TAIL_Found;
extern bool g_VOX_Lost;
extern bool g_SquelchLost;
extern uint8_t gFlashLightState;
extern uint8_t g_200003B4;
extern uint16_t g_200003B6;
extern uint16_t g_200003B8;
extern uint8_t g_200003BA;
extern uint8_t g_200003BB;
extern uint8_t g_200003BC;
extern uint8_t g_200003BD;
extern uint8_t g_200003BE;
extern uint8_t g_200003C0;
extern bool g_200003C1;
extern uint8_t g_200003C3;
extern uint8_t g_200003C4;
extern uint8_t gDTMFChosenContact;
extern uint16_t g_200003E2;
extern volatile uint16_t gFlashLightBlinkCounter;
extern uint8_t g_200003FD;
extern uint8_t g_20000400;
extern uint8_t g_20000410;
extern uint8_t g_20000411;
extern uint8_t g_20000413;
extern uint8_t g_20000414;
extern uint8_t g_20000415;
extern uint8_t g_20000416;
extern uint32_t g_20000418;
extern uint8_t g_2000041F;
extern uint8_t g_20000420;
extern uint16_t g_20000422;
extern uint8_t g_20000427;
extern bool gKeyBeingHeld;
extern bool gPttIsPressed;
extern uint8_t gPttDebounceCounter;
extern uint8_t gDTMF_WriteIndex;
extern uint8_t g_20000438;
extern bool g_20000439;
extern uint8_t gMenuListCount;
extern uint8_t g_20000442;
extern uint8_t g_20000458;
extern uint8_t gBackupCROSS_BAND_RX_TX;
extern uint8_t g_CxCSS_Type;
extern uint8_t g_CxCSS_Index;
extern uint8_t g_2000045C;
extern uint8_t g_2000045D;
extern uint8_t g_2000045F;
extern uint8_t gScannerEditState;
extern uint8_t g_20000464;
extern uint8_t gAircopySendCountdown;
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
extern uint8_t gFmRadioCountdown;
extern uint8_t gA_Scan_Channel;
extern uint8_t gDTMF_AUTO_RESET_TIME;
extern bool gF_LOCK;
extern uint8_t gScanChannel;
extern uint32_t gScanFrequency;
extern uint8_t gScanPauseMode;
extern uint8_t gScanState;
extern uint8_t gShowChPrefix;
extern volatile uint16_t gSystickCountdown2;
extern volatile uint8_t gSystickCountdown3;
extern volatile uint8_t gSystickCountdown4;
extern volatile uint16_t gSystickCountdown11;
extern volatile bool gTxTimeoutReached;
extern volatile bool gNextTimeslice40ms;
extern volatile bool gSchedulePowerSave;
extern volatile bool gBatterySaveCountdownExpired;
extern volatile bool gSystickFlag7;
extern volatile bool gSystickFlag8;
extern volatile bool gSystickFlag9;
extern volatile bool gSystickFlag10;
extern volatile bool gScheduleFM;

extern volatile uint16_t ScanPauseDelayIn10msec;

extern int16_t gFM_FrequencyDeviation;

extern uint16_t gCurrentRSSI;

extern volatile int8_t gStepDirection;

// --------

void NUMBER_Get(char *pDigits, uint32_t *pInteger);
void NUMBER_ToDigits(uint32_t Value, char *pDigits);
uint8_t NUMBER_AddWithWraparound(uint8_t Base, int8_t Add, uint8_t LowerLimit, uint8_t UpperLimit);

#endif

