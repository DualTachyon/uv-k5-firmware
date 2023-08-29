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
#include "app/fm.h"
#include "audio.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk1080.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "misc.h"
#include "settings.h"
#include "ui/inputbox.h"
#include "ui/ui.h"

extern void APP_StartScan(bool bFlag);
extern void APP_SwitchToFM(void);

uint16_t gFM_Channels[20];
bool gFmRadioMode;

bool FM_CheckValidChannel(uint8_t Channel)
{
	if (Channel < 20 && (gFM_Channels[Channel] - 760) < 321) {
		return true;
	}

	return false;
}

uint8_t FM_FindNextChannel(uint8_t Channel, uint8_t Direction)
{
	uint8_t i;

	for (i = 0; i < 20; i++) {
		if (Channel == 0xFF) {
			Channel = 19;
		} else if (Channel >= 20) {
			Channel = 0;
		}
		if (FM_CheckValidChannel(Channel)) {
			return Channel;
		}
		Channel += Direction;
	}

	return 0xFF;
}

int FM_ConfigureChannelState(void)
{
	uint8_t Channel;

	gEeprom.FM_FrequencyToPlay = gEeprom.FM_CurrentFrequency;
	if (gEeprom.FM_IsChannelSelected) {
		Channel = FM_FindNextChannel(gEeprom.FM_CurrentChannel, FM_CHANNEL_UP);
		if (Channel == 0xFF) {
			gEeprom.FM_IsChannelSelected = false;
			return -1;
		}
		gEeprom.FM_CurrentChannel = Channel;
		gEeprom.FM_FrequencyToPlay = gFM_Channels[Channel];
	}

	return 0;
}

void FM_TurnOff(void)
{
	gFmRadioMode = false;
	gFM_Step = 0;
	g_2000038E = 0;
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	g_2000036B = 0;
	BK1080_Init(0, false);
	g_2000036F = 1;
}

void FM_EraseChannels(void)
{
	uint8_t i;
	uint8_t Template[8];

	memset(Template, 0xFF, sizeof(Template));
	for (i = 0; i < 5; i++) {
		EEPROM_WriteBuffer(0x0E40 + (i * 8), Template);
	}

	memset(gFM_Channels, 0xFF, sizeof(gFM_Channels));
}

void FM_Tune(uint16_t Frequency, int8_t Step, bool bFlag)
{
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	g_2000036B = 0;
	if (gFM_Step == 0) {
		gFmPlayCountdown = 120;
	} else {
		gFmPlayCountdown = 10;
	}
	gScheduleFM = false;
	g_20000427 = 0;
	gAskToSave = false;
	gAskToDelete = false;
	gEeprom.FM_FrequencyToPlay = Frequency;
	if (!bFlag) {
		Frequency += Step;
		if (Frequency < gEeprom.FM_LowerLimit) {
			Frequency = gEeprom.FM_UpperLimit;
		} else if (Frequency > gEeprom.FM_UpperLimit) {
			Frequency = gEeprom.FM_LowerLimit;
		}
		gEeprom.FM_FrequencyToPlay = Frequency;
	}

	gFM_Step = Step;
	BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
}

void FM_Play(void)
{
	gFM_Step = 0;
	if (gIs_A_Scan) {
		gEeprom.FM_IsChannelSelected = true;
		gEeprom.FM_CurrentChannel = 0;
	}
	FM_ConfigureChannelState();
	BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
	SETTINGS_SaveFM();
	gFmPlayCountdown = 0;
	gScheduleFM = false;
	gAskToSave = false;
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	g_2000036B = 1;
}

void FM_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		if (!gWasFKeyPressed) {
			uint8_t Value;

			if (gAskToDelete) {
				gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
				return;
			}
			if (gAskToSave) {
				Value = 2;
			} else {
				if (gFM_Step) {
					gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
					return;
				}
				if (gEeprom.FM_IsChannelSelected) {
					Value = 1;
				} else {
					Value = 0;
				}
			}
			INPUTBOX_Append(Key);
			gRequestDisplayScreen = DISPLAY_FM;
			if (Value == 0) {
				if (gInputBoxIndex == 1) {
					if (1 < gInputBox[0]) {
						gInputBox[1] = gInputBox[0];
						gInputBox[0] = 0;
						gInputBoxIndex = 2;
					}
				} else if (3 < gInputBoxIndex) {
					uint32_t Frequency;

					gInputBoxIndex = 0;
					NUMBER_Get(gInputBox, &Frequency);
					Frequency = Frequency / 10000;
					if (Frequency < gEeprom.FM_LowerLimit || gEeprom.FM_UpperLimit < Frequency) {
						gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
						gRequestDisplayScreen = DISPLAY_FM;
						return;
					}
					gEeprom.FM_CurrentFrequency = Frequency;
					gAnotherVoiceID = (VOICE_ID_t)Key;
					gEeprom.FM_FrequencyToPlay = gEeprom.FM_CurrentFrequency;
					BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
					gRequestSaveFM = true;
					return;
				}
			} else if (gInputBoxIndex == 2) {
				uint8_t Channel;

				gInputBoxIndex = 0;
				Channel = ((gInputBox[0] * 10) + gInputBox[1]) - 1;
				if (Value == 1) {
					if (FM_CheckValidChannel(Channel)) {
						gAnotherVoiceID = (VOICE_ID_t)Key;
						gEeprom.FM_CurrentChannel = Channel;
						gEeprom.FM_FrequencyToPlay = gFM_Channels[Channel];
						BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
						gRequestSaveFM = true;
						return;
					}
				} else if (Channel < 20) {
					gAnotherVoiceID = (VOICE_ID_t)Key;
					gRequestDisplayScreen = DISPLAY_FM;
					gInputBoxIndex = 0;
					gA_Scan_Channel = Channel;
					return;
				}
				gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
				return;
			}
			gAnotherVoiceID = (VOICE_ID_t)Key;
			return;
		}
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gWasFKeyPressed = false;
		g_2000036F = 1;
		gRequestDisplayScreen = DISPLAY_FM;
		switch (Key) {
		case KEY_0:
			APP_SwitchToFM();
			break;

		case KEY_1:
			gEeprom.FM_IsChannelSelected = !gEeprom.FM_IsChannelSelected;
			if (!FM_ConfigureChannelState()) {
				BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
				gRequestSaveFM = true;
				return;
			}
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			break;

		case KEY_2:
			APP_StartScan(true);
			break;

		case KEY_3:
			APP_StartScan(false);
			break;

		default:
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			break;
		}
	}
}

void FM_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (bKeyHeld) {
		return;
	}
	if (!bKeyPressed) {
		return;
	}
	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
	if (gFM_Step == 0) {
		if (gInputBoxIndex == 0) {
			if (!gAskToSave && !gAskToDelete) {
				APP_SwitchToFM();
				return;
			}
			gAskToSave = false;
			gAskToDelete = false;
		} else {
			gInputBoxIndex--;
			gInputBox[gInputBoxIndex] = 10;
			if (gInputBoxIndex) {
				if (gInputBoxIndex != 1) {
					gRequestDisplayScreen = DISPLAY_FM;
					return;
				}
				if (gInputBox[0] != 0) {
					gRequestDisplayScreen = DISPLAY_FM;
					return;
				}
			}
			gInputBoxIndex = 0;
		}
		gAnotherVoiceID = VOICE_ID_CANCEL;
	} else {
		FM_Play();
		gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
	}
	gRequestDisplayScreen = DISPLAY_FM;
}

void FM_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Step)
{
	if (bKeyHeld || !bKeyPressed) {
		if (gInputBoxIndex) {
			return;
		}
		if (!bKeyPressed) {
			return;
		}
	} else {
		if (gInputBoxIndex) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			return;
		}
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
	}
	if (gAskToSave) {
		gRequestDisplayScreen = DISPLAY_FM;
		gA_Scan_Channel = NUMBER_AddWithWraparound(gA_Scan_Channel, Step, 0, 19);
		return;
	}
	if (gFM_Step) {
		if (gIs_A_Scan) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			return;
		}
		FM_Tune(gEeprom.FM_FrequencyToPlay, Step, false);
		gRequestDisplayScreen = DISPLAY_FM;
		return;
	}
	if (gEeprom.FM_IsChannelSelected) {
		uint8_t Channel;

		Channel = FM_FindNextChannel(gEeprom.FM_CurrentChannel + Step, Step);
		if (Channel == 0xFF || gEeprom.FM_CurrentChannel == Channel) {
			goto Bail;
		}
		gEeprom.FM_CurrentChannel = Channel;
		gEeprom.FM_FrequencyToPlay = gFM_Channels[Channel];
	} else {
		uint16_t Frequency;

		Frequency = gEeprom.FM_CurrentFrequency + Step;
		if (Frequency < gEeprom.FM_LowerLimit) {
			Frequency = gEeprom.FM_UpperLimit;
		} else if (Frequency > gEeprom.FM_UpperLimit) {
			Frequency = gEeprom.FM_LowerLimit;
		}
		gEeprom.FM_FrequencyToPlay = Frequency;
		gEeprom.FM_CurrentFrequency = gEeprom.FM_FrequencyToPlay;
	}
	gRequestSaveFM = true;

Bail:
	BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
	gRequestDisplayScreen = DISPLAY_FM;
}

