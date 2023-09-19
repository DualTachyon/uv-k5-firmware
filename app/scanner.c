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

#include "app/generic.h"
#include "app/scanner.h"
#include "audio.h"
#include "driver/bk4819.h"
#include "frequencies.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/inputbox.h"
#include "ui/ui.h"

DCS_CodeType_t gScanCssResultType;
uint8_t gScanCssResultCode;
bool gFlagStartScan;
bool gFlagStopScan;
bool gScanSingleFrequency;
uint8_t gScannerEditState;
uint8_t gScanChannel;
uint32_t gScanFrequency;
bool gScanPauseMode;
SCAN_CssState_t gScanCssState;
volatile bool gScheduleScanListen = true;
volatile uint16_t ScanPauseDelayIn10msec;
uint8_t gScanProgressIndicator;
uint8_t gScanHitCount;
bool gScanUseCssResult;
uint8_t gScanState;
bool bScanKeepFrequency;

static void SCANNER_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		if (gScannerEditState == 1) {
			uint16_t Channel;

			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			INPUTBOX_Append(Key);
			gRequestDisplayScreen = DISPLAY_SCANNER;
			if (gInputBoxIndex < 3) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				return;
			}
			gInputBoxIndex = 0;
			Channel = ((gInputBox[0] * 100) + (gInputBox[1] * 10) + gInputBox[2]) - 1;
			if (IS_MR_CHANNEL(Channel)) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gShowChPrefix = RADIO_CheckValidChannel(Channel, false, 0);
				gScanChannel = (uint8_t)Channel;
				return;
			}
		}
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
	}
}

static void SCANNER_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

		switch (gScannerEditState) {
		case 0:
			gRequestDisplayScreen = DISPLAY_MAIN;
			gEeprom.CROSS_BAND_RX_TX = gBackupCROSS_BAND_RX_TX;
			gUpdateStatus = true;
			gFlagStopScan = true;
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos = true;
			gAnotherVoiceID = VOICE_ID_CANCEL;
			break;

		case 1:
			if (gInputBoxIndex) {
				gInputBoxIndex--;
				gInputBox[gInputBoxIndex] = 10;
				gRequestDisplayScreen = DISPLAY_SCANNER;
				break;
			}
			// Fallthrough

		case 2:
			gScannerEditState = 0;
			gAnotherVoiceID = VOICE_ID_CANCEL;
			gRequestDisplayScreen = DISPLAY_SCANNER;
			break;
		}
	}
}

static void SCANNER_Key_MENU(bool bKeyPressed, bool bKeyHeld)
{
	uint8_t Channel;

	if (bKeyHeld) {
		return;
	}
	if (!bKeyPressed) {
		return;
	}
	if (gScanCssState == SCAN_CSS_STATE_OFF && !gScanSingleFrequency) {
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	if (gScanCssState == SCAN_CSS_STATE_SCANNING) {
		if (gScanSingleFrequency) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			return;
		}
	}

	if (gScanCssState == SCAN_CSS_STATE_FAILED) {
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	switch (gScannerEditState) {
	case 0:
		if (!gScanSingleFrequency) {
			uint32_t Freq250;
			uint32_t Freq625;
			int16_t Delta250;
			int16_t Delta625;

			Freq250 = FREQUENCY_FloorToStep(gScanFrequency, 250, 0);
			Freq625 = FREQUENCY_FloorToStep(gScanFrequency, 625, 0);
			Delta250 = (short)gScanFrequency - (short)Freq250;
			if (125 < Delta250) {
				Delta250 = 250 - Delta250;
				Freq250 += 250;
			}
			Delta625 = (short)gScanFrequency - (short)Freq625;
			if (312 < Delta625) {
				Delta625 = 625 - Delta625;
				Freq625 += 625;
			}
			if (Delta625 < Delta250) {
				gStepSetting = STEP_6_25kHz;
				gScanFrequency = Freq625;
			} else {
				gStepSetting = STEP_2_5kHz;
				gScanFrequency = Freq250;
			}
		}
		if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			gScannerEditState = 1;
			gScanChannel = gTxVfo->CHANNEL_SAVE;
			gShowChPrefix = RADIO_CheckValidChannel(gTxVfo->CHANNEL_SAVE, false, 0);
		} else {
			gScannerEditState = 2;
		}
		gScanCssState = SCAN_CSS_STATE_FOUND;
		gAnotherVoiceID = VOICE_ID_MEMORY_CHANNEL;
		gRequestDisplayScreen = DISPLAY_SCANNER;
		break;

	case 1:
		if (gInputBoxIndex == 0) {
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			gRequestDisplayScreen = DISPLAY_SCANNER;
			gScannerEditState = 2;
		}
		break;

	case 2:
		if (!gScanSingleFrequency) {
			RADIO_InitInfo(gTxVfo, gTxVfo->CHANNEL_SAVE, FREQUENCY_GetBand(gScanFrequency), gScanFrequency);
			if (gScanUseCssResult) {
				gTxVfo->ConfigRX.CodeType = gScanCssResultType;
				gTxVfo->ConfigRX.Code = gScanCssResultCode;
			}
			gTxVfo->ConfigTX = gTxVfo->ConfigRX;
			gTxVfo->STEP_SETTING = gStepSetting;
		} else {
			RADIO_ConfigureChannel(0, 2);
			RADIO_ConfigureChannel(1, 2);
			gTxVfo->ConfigRX.CodeType = gScanCssResultType;
			gTxVfo->ConfigRX.Code = gScanCssResultCode;
			gTxVfo->ConfigTX.CodeType = gScanCssResultType;
			gTxVfo->ConfigTX.Code = gScanCssResultCode;
		}

		if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			Channel = gScanChannel;
			gEeprom.MrChannel[gEeprom.TX_CHANNEL] = Channel;
		} else {
			Channel = gTxVfo->Band + FREQ_CHANNEL_FIRST;
			gEeprom.FreqChannel[gEeprom.TX_CHANNEL] = Channel;
		}
		gTxVfo->CHANNEL_SAVE = Channel;
		gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] = Channel;
		gAnotherVoiceID = VOICE_ID_CONFIRM;
		gRequestDisplayScreen = DISPLAY_SCANNER;
		gRequestSaveChannel = 2;
		gScannerEditState = 0;
		break;

	default:
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		break;
	}
}

static void SCANNER_Key_STAR(bool bKeyPressed, bool bKeyHeld)
{
	if ((!bKeyHeld) && (bKeyPressed)) {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gFlagStartScan = true;
	}
	return;
}

static void SCANNER_Key_UP_DOWN(bool bKeyPressed, bool pKeyHeld, int8_t Direction)
{
	if (pKeyHeld) {
		if (!bKeyPressed) {
			return;
		}
	} else {
		if (!bKeyPressed) {
			return;
		}
		gInputBoxIndex = 0;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
	}
	if (gScannerEditState == 1) {
		gScanChannel = NUMBER_AddWithWraparound(gScanChannel, Direction, 0, MR_CHANNEL_LAST);
		gShowChPrefix = RADIO_CheckValidChannel(gScanChannel, false, 0);
		gRequestDisplayScreen = DISPLAY_SCANNER;
	} else {
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
	}
}

void SCANNER_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		SCANNER_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		SCANNER_Key_MENU(bKeyPressed, bKeyHeld);
		break;
	case KEY_UP:
		SCANNER_Key_UP_DOWN(bKeyPressed, bKeyHeld, 1);
		break;
	case KEY_DOWN:
		SCANNER_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
		break;
	case KEY_EXIT:
		SCANNER_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	case KEY_STAR:
		SCANNER_Key_STAR(bKeyPressed, bKeyHeld);
		break;
	case KEY_PTT:
		GENERIC_Key_PTT(bKeyPressed);
		break;
	default:
		if (!bKeyHeld && bKeyPressed) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
		break;
	}
}

void SCANNER_Start(void)
{
	uint8_t BackupStep;
	uint16_t BackupFrequency;

	BK4819_StopScan();
	RADIO_SelectVfos();

	if (IS_NOAA_CHANNEL(gRxVfo->CHANNEL_SAVE)) {
		gRxVfo->CHANNEL_SAVE = FREQ_CHANNEL_FIRST + BAND6_400MHz;
	}

	BackupStep = gRxVfo->STEP_SETTING;
	BackupFrequency = gRxVfo->StepFrequency;

	RADIO_InitInfo(gRxVfo, gRxVfo->CHANNEL_SAVE, gRxVfo->Band, gRxVfo->pRX->Frequency);

	gRxVfo->STEP_SETTING = BackupStep;
	gRxVfo->StepFrequency = BackupFrequency;

	RADIO_SetupRegisters(true);

#if defined(ENABLE_NOAA)
	gIsNoaaMode = false;
#endif
	if (gScanSingleFrequency) {
		gScanCssState = SCAN_CSS_STATE_SCANNING;
		gScanFrequency = gRxVfo->pRX->Frequency;
		gStepSetting = gRxVfo->STEP_SETTING;
		BK4819_PickRXFilterPathBasedOnFrequency(gScanFrequency);
		BK4819_SetScanFrequency(gScanFrequency);
	} else {
		gScanCssState = SCAN_CSS_STATE_OFF;
		gScanFrequency = 0xFFFFFFFF;
		BK4819_PickRXFilterPathBasedOnFrequency(0xFFFFFFFF);
		BK4819_EnableFrequencyScan();
	}
	gScanDelay = 21;
	gScanCssResultCode = 0xFF;
	gScanCssResultType = 0xFF;
	gScanHitCount = 0;
	gScanUseCssResult = false;
	gDTMF_RequestPending = false;
	g_CxCSS_TAIL_Found = false;
	g_CDCSS_Lost = false;
	gCDCSSCodeType = 0;
	g_CTCSS_Lost = false;
	g_VOX_Lost = false;
	g_SquelchLost = false;
	gScannerEditState = 0;
	gScanProgressIndicator = 0;
}

void SCANNER_Stop(void)
{
	uint8_t Previous;

	Previous = gRestoreMrChannel;
	gScanState = SCAN_OFF;

	if (!bScanKeepFrequency) {
		if (IS_MR_CHANNEL(gNextMrChannel)) {
			gEeprom.MrChannel[gEeprom.RX_CHANNEL] = gRestoreMrChannel;
			gEeprom.ScreenChannel[gEeprom.RX_CHANNEL] = Previous;
			RADIO_ConfigureChannel(gEeprom.RX_CHANNEL, 2);
		} else {
			gRxVfo->ConfigRX.Frequency = gRestoreFrequency;
			RADIO_ApplyOffset(gRxVfo);
			RADIO_ConfigureSquelchAndOutputPower(gRxVfo);
		}
		RADIO_SetupRegisters(true);
		gUpdateDisplay = true;
		return;
	}

	if (!IS_MR_CHANNEL(gRxVfo->CHANNEL_SAVE)) {
		RADIO_ApplyOffset(gRxVfo);
		RADIO_ConfigureSquelchAndOutputPower(gRxVfo);
		SETTINGS_SaveChannel(gRxVfo->CHANNEL_SAVE, gEeprom.RX_CHANNEL, gRxVfo, 1);
		return;
	}

	SETTINGS_SaveVfoIndices();
}

