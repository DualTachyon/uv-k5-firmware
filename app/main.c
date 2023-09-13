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
#include "app/action.h"
#include "app/app.h"
#if defined(ENABLE_FMRADIO)
#include "app/fm.h"
#endif
#include "app/generic.h"
#include "app/main.h"
#include "app/scanner.h"
#include "audio.h"
#include "dtmf.h"
#include "frequencies.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/inputbox.h"
#include "ui/ui.h"

static void MAIN_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint8_t Vfo;
	uint8_t Band;

	Vfo = gEeprom.TX_CHANNEL;

	if (bKeyHeld) {
		return;
	}
	if (!bKeyPressed) {
		return;
	}

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (!gWasFKeyPressed) {
		INPUTBOX_Append(Key);
		gRequestDisplayScreen = DISPLAY_MAIN;
		if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			uint16_t Channel;

			if (gInputBoxIndex != 3) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gRequestDisplayScreen = DISPLAY_MAIN;
				return;
			}
			gInputBoxIndex = 0;
			Channel = ((gInputBox[0] * 100) + (gInputBox[1] * 10) + gInputBox[2]) - 1;
			if (!RADIO_CheckValidChannel(Channel, false, 0)) {
				gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
				return;
			}
			gAnotherVoiceID = (VOICE_ID_t)Key;
			gEeprom.MrChannel[Vfo] = (uint8_t)Channel;
			gEeprom.ScreenChannel[Vfo] = (uint8_t)Channel;
			gRequestSaveVFO = true;
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			return;
		}
		if (IS_NOT_NOAA_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			uint32_t Frequency;

			if (gInputBoxIndex < 6) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				return;
			}
			gInputBoxIndex = 0;
			NUMBER_Get(gInputBox, &Frequency);
			if (gSetting_350EN || (Frequency < 35000000 || Frequency > 39999990)) {
				uint8_t i;

				for (i = 0; i < 7; i++) {
					if (Frequency <= UpperLimitFrequencyBandTable[i] && (LowerLimitFrequencyBandTable[i] <= Frequency)) {
						gAnotherVoiceID = (VOICE_ID_t)Key;
						if (gTxVfo->Band != i) {
							gTxVfo->Band = i;
							gEeprom.ScreenChannel[Vfo] = i + FREQ_CHANNEL_FIRST;
							gEeprom.FreqChannel[Vfo] = i + FREQ_CHANNEL_FIRST;
							SETTINGS_SaveVfoIndices();
							RADIO_ConfigureChannel(Vfo, 2);
						}
						Frequency += 75;
						gTxVfo->ConfigRX.Frequency = FREQUENCY_FloorToStep(
								Frequency,
								gTxVfo->StepFrequency,
								LowerLimitFrequencyBandTable[gTxVfo->Band]
								);
						gRequestSaveChannel = 1;
						return;
					}
				}
			}
		} else {
#if defined(ENABLE_NOAA)
			uint8_t Channel;

			if (gInputBoxIndex != 2) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gRequestDisplayScreen = DISPLAY_MAIN;
				return;
			}
			gInputBoxIndex = 0;
			Channel = (gInputBox[0] * 10) + gInputBox[1];
			if (Channel >= 1 && Channel <= 10) {
				Channel += NOAA_CHANNEL_FIRST;
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gEeprom.NoaaChannel[Vfo] = Channel;
				gEeprom.ScreenChannel[Vfo] = Channel;
				gRequestSaveVFO = true;
				gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
				return;
			}
#endif
		}
		gRequestDisplayScreen = DISPLAY_MAIN;
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}
	gWasFKeyPressed = false;
	gUpdateStatus = true;
	switch (Key) {
	case KEY_0:
#if defined(ENABLE_FMRADIO)
		ACTION_FM();
#endif
		break;

	case KEY_1:
		if (!IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			gWasFKeyPressed = false;
			gUpdateStatus = true;
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			return;
		}
		Band = gTxVfo->Band + 1;
		if (gSetting_350EN || Band != BAND5_350MHz) {
			if (BAND7_470MHz < Band) {
				Band = BAND1_50MHz;
			}
		} else {
			Band = BAND6_400MHz;
		}
		gTxVfo->Band = Band;
		gEeprom.ScreenChannel[Vfo] = FREQ_CHANNEL_FIRST + Band;
		gEeprom.FreqChannel[Vfo] = FREQ_CHANNEL_FIRST + Band;
		gRequestSaveVFO = true;
		gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gRequestDisplayScreen = DISPLAY_MAIN;
		break;

	case KEY_2:
		if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_CHAN_A) {
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_CHAN_B;
		} else if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_CHAN_B) {
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_CHAN_A;
		} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_CHAN_A) {
			gEeprom.DUAL_WATCH = DUAL_WATCH_CHAN_B;
		} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_CHAN_B) {
			gEeprom.DUAL_WATCH = DUAL_WATCH_CHAN_A;
		} else {
			gEeprom.TX_CHANNEL = (Vfo == 0);
		}
		gRequestSaveSettings = 1;
		gFlagReconfigureVfos = true;
		gRequestDisplayScreen = DISPLAY_MAIN;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		break;

	case KEY_3:
		if (gEeprom.VFO_OPEN && IS_NOT_NOAA_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			uint8_t Channel;

			if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
				gEeprom.ScreenChannel[Vfo] = gEeprom.FreqChannel[gEeprom.TX_CHANNEL];
				gAnotherVoiceID = VOICE_ID_FREQUENCY_MODE;
				gRequestSaveVFO = true;
				gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
				break;
			}
			Channel = RADIO_FindNextChannel(gEeprom.MrChannel[gEeprom.TX_CHANNEL], 1, false, 0);
			if (Channel != 0xFF) {
				gEeprom.ScreenChannel[Vfo] = Channel;
				AUDIO_SetVoiceID(0, VOICE_ID_CHANNEL_MODE);
				AUDIO_SetDigitVoice(1, Channel + 1);
				gAnotherVoiceID = (VOICE_ID_t)0xFE;
				gRequestSaveVFO = true;
				gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
				break;
			}
		}
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		break;

	case KEY_4:
		gWasFKeyPressed = false;
		gUpdateStatus = true;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gFlagStartScan = true;
		gScanSingleFrequency = false;
		gBackupCROSS_BAND_RX_TX = gEeprom.CROSS_BAND_RX_TX;
		gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
		break;

	case KEY_5:
#if defined(ENABLE_NOAA)
		if (IS_NOT_NOAA_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			gEeprom.ScreenChannel[Vfo] = gEeprom.NoaaChannel[gEeprom.TX_CHANNEL];
		} else {
			gEeprom.ScreenChannel[Vfo] = gEeprom.FreqChannel[gEeprom.TX_CHANNEL];
			gAnotherVoiceID = VOICE_ID_FREQUENCY_MODE;
		}
		gRequestSaveVFO = true;
		gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
#endif
		break;

	case KEY_6:
		ACTION_Power();
		break;

	case KEY_7:
		ACTION_Vox();
		break;

	case KEY_8:
		gTxVfo->FrequencyReverse = gTxVfo->FrequencyReverse == false;
		gRequestSaveChannel = 1;
		break;

	case KEY_9:
		if (RADIO_CheckValidChannel(gEeprom.CHAN_1_CALL, false, 0)) {
			gEeprom.MrChannel[Vfo] = gEeprom.CHAN_1_CALL;
			gEeprom.ScreenChannel[Vfo] = gEeprom.CHAN_1_CALL;
			AUDIO_SetVoiceID(0, VOICE_ID_CHANNEL_MODE);
			AUDIO_SetDigitVoice(1, gEeprom.CHAN_1_CALL + 1);
			gAnotherVoiceID = (VOICE_ID_t)0xFE;
			gRequestSaveVFO = true;
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			break;
		}
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		break;

	default:
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gUpdateStatus = true;
		gWasFKeyPressed = false;
		break;
	}
}

static void MAIN_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
#if defined(ENABLE_FMRADIO)
		if (gFmRadioMode) {
			ACTION_FM();
			return;
		}
#endif
		if (gScanState == SCAN_OFF) {
			if (gInputBoxIndex == 0) {
				return;
			}
			gInputBoxIndex--;
			gInputBox[gInputBoxIndex] = 10;
			if (gInputBoxIndex == 0) {
				gAnotherVoiceID = VOICE_ID_CANCEL;
			}
		} else {
			SCANNER_Stop();
			gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
		}
		gRequestDisplayScreen = DISPLAY_MAIN;
	}
}

static void MAIN_Key_MENU(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		bool bFlag;

		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		bFlag = gInputBoxIndex == 0;
		gInputBoxIndex = 0;
		if (bFlag) {
			gFlagRefreshSetting = true;
			gRequestDisplayScreen = DISPLAY_MENU;
			gAnotherVoiceID = VOICE_ID_MENU;
		} else {
			gRequestDisplayScreen = DISPLAY_MAIN;
		}
	}
}

static void MAIN_Key_STAR(bool bKeyPressed, bool bKeyHeld)
{
	if (gInputBoxIndex) {
		if (!bKeyHeld && bKeyPressed) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
		return;
	}
	if (bKeyHeld || !bKeyPressed) {
		if (bKeyHeld || bKeyPressed) {
			if (!bKeyHeld) {
				return;
			}
			if (!bKeyPressed) {
				return;
			}
			ACTION_Scan(false);
			return;
		}
		if (gScanState == SCAN_OFF && IS_NOT_NOAA_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			gDTMF_InputMode = true;
			memcpy(gDTMF_InputBox, gDTMF_String, 15);
			gDTMF_InputIndex = 0;
			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}
	} else {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		if (!gWasFKeyPressed) {
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			return;
		}
		gWasFKeyPressed = false;
		gUpdateStatus = true;
		if (IS_NOT_NOAA_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
			gFlagStartScan = true;
			gScanSingleFrequency = true;
			gBackupCROSS_BAND_RX_TX = gEeprom.CROSS_BAND_RX_TX;
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
		} else {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
		gPttWasReleased = true;
	}
}

static void MAIN_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction)
{
	uint8_t Channel;

	Channel = gEeprom.ScreenChannel[gEeprom.TX_CHANNEL];
	if (bKeyHeld || !bKeyPressed) {
		if (gInputBoxIndex) {
			return;
		}
		if (!bKeyPressed) {
			if (!bKeyHeld) {
				return;
			}
			if (IS_FREQ_CHANNEL(Channel)) {
				return;
			}
			AUDIO_SetDigitVoice(0, gTxVfo->CHANNEL_SAVE + 1);
			gAnotherVoiceID = (VOICE_ID_t)0xFE;
			return;
		}
	} else {
		if (gInputBoxIndex) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			return;
		}
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
	}

	if (gScanState == SCAN_OFF) {
		if (IS_NOT_NOAA_CHANNEL(Channel)) {
			uint8_t Next;

			if (IS_FREQ_CHANNEL(Channel)) {
				APP_SetFrequencyByStep(gTxVfo, Direction);
				gRequestSaveChannel = 1;
				return;
			}
			Next = RADIO_FindNextChannel(Channel + Direction, Direction, false, 0);
			if (Next == 0xFF) {
				return;
			}
			if (Channel == Next) {
				return;
			}
			gEeprom.MrChannel[gEeprom.TX_CHANNEL] = Next;
			gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] = Next;
			if (!bKeyHeld) {
				AUDIO_SetDigitVoice(0, Next + 1);
				gAnotherVoiceID = (VOICE_ID_t)0xFE;
			}
		} else {
#if defined(ENABLE_NOAA)
			Channel = NOAA_CHANNEL_FIRST + NUMBER_AddWithWraparound(gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] - NOAA_CHANNEL_FIRST, Direction, 0, 9);
			gEeprom.NoaaChannel[gEeprom.TX_CHANNEL] = Channel;
			gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] = Channel;
#endif
		}
		gRequestSaveVFO = true;
		gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
		return;
	}
	CHANNEL_Next(false, Direction);
	gPttWasReleased = true;
}

void MAIN_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
#if defined(ENABLE_FMRADIO)
	if (gFmRadioMode && Key != KEY_PTT && Key != KEY_EXIT) {
		if (!bKeyHeld && bKeyPressed) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
		return;
	}
#endif
	if (gDTMF_InputMode && !bKeyHeld && bKeyPressed) {
		char Character = DTMF_GetCharacter(Key);
		if (Character != 0xFF) {
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			DTMF_Append(Character);
			gRequestDisplayScreen = DISPLAY_MAIN;
			gPttWasReleased = true;
			return;
		}
	}

	// TODO: ???
	if (KEY_PTT < Key) {
		Key = KEY_SIDE2;
	}

	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		MAIN_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		MAIN_Key_MENU(bKeyPressed, bKeyHeld);
		break;
	case KEY_UP:
		MAIN_Key_UP_DOWN(bKeyPressed, bKeyHeld, 1);
		break;
	case KEY_DOWN:
		MAIN_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
		break;
	case KEY_EXIT:
		MAIN_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	case KEY_STAR:
		MAIN_Key_STAR(bKeyPressed, bKeyHeld);
		break;
	case KEY_F:
		GENERIC_Key_F(bKeyPressed, bKeyHeld);
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

