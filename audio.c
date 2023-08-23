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

#include "audio.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"
#include "fm.h"
#include "functions.h"
#include "gui.h"
#include "misc.h"
#include "settings.h"

static const uint8_t VoiceClipLengthChinese[58] = {
	0x32, 0x32, 0x32, 0x37, 0x37, 0x32, 0x32, 0x32,
	0x32, 0x37, 0x37, 0x32, 0x64, 0x64, 0x64, 0x64,
	0x64, 0x69, 0x64, 0x69, 0x5A, 0x5F, 0x5F, 0x64,
	0x64, 0x69, 0x64, 0x64, 0x69, 0x69, 0x69, 0x64,
	0x64, 0x6E, 0x69, 0x5F, 0x64, 0x64, 0x64, 0x69,
	0x69, 0x69, 0x64, 0x69, 0x64, 0x64, 0x55, 0x5F,
	0x5A, 0x4B, 0x4B, 0x46, 0x46, 0x69, 0x64, 0x6E,
	0x5A, 0x64,
};

static const uint8_t VoiceClipLengthEnglish[76] = {
	0x50, 0x32, 0x2D, 0x2D, 0x2D, 0x37, 0x37, 0x37,
	0x32, 0x32, 0x3C, 0x37, 0x46, 0x46, 0x4B, 0x82,
	0x82, 0x6E, 0x82, 0x46, 0x96, 0x64, 0x46, 0x6E,
	0x78, 0x6E, 0x87, 0x64, 0x96, 0x96, 0x46, 0x9B,
	0x91, 0x82, 0x82, 0x73, 0x78, 0x64, 0x82, 0x6E,
	0x78, 0x82, 0x87, 0x6E, 0x55, 0x78, 0x64, 0x69,
	0x9B, 0x5A, 0x50, 0x3C, 0x32, 0x55, 0x64, 0x64,
	0x50, 0x46, 0x46, 0x46, 0x4B, 0x4B, 0x50, 0x50,
	0x55, 0x4B, 0x4B, 0x32, 0x32, 0x32, 0x32, 0x37,
	0x41, 0x32, 0x3C, 0x37,
};

uint8_t gVoiceID[8];
uint8_t gVoiceReadIndex;
uint8_t gVoiceWriteIndex;
volatile uint16_t gCountdownToPlayNextVoice;
volatile bool gFlagPlayQueuedVoice;

void AUDIO_PlayBeep(BEEP_Type_t Beep)
{
	uint16_t ToneConfig;
	uint16_t ToneFrequency;
	uint16_t Duration;

	if (Beep != BEEP_500HZ_60MS_DOUBLE_BEEP && Beep != BEEP_440HZ_500MS && !gEeprom.BEEP_CONTROL) {
		return;
	}

	if (gScreenToDisplay == DISPLAY_AIRCOPY) {
		return;
	}
	if (gCurrentFunction == FUNCTION_4) {
		return;
	}
	if (gCurrentFunction == FUNCTION_2) {
		return;
	}

	ToneConfig = BK4819_GetRegister(BK4819_REG_71);

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

	if (gCurrentFunction == FUNCTION_POWER_SAVE && gThisCanEnable_BK4819_Rxon) {
		BK4819_RX_TurnOn();
	}

	if (gFmMute == true) {
		BK1080_Mute(true);
	}
	SYSTEM_DelayMs(20);
	switch (Beep) {
	case BEEP_1KHZ_60MS_OPTIONAL:
		ToneFrequency = 1000;
		break;
	case BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
	case BEEP_500HZ_60MS_DOUBLE_BEEP:
		ToneFrequency = 500;
		break;
	default:
		ToneFrequency = 440;
		break;
	}
	BK4819_PlayTone(ToneFrequency, true);
	SYSTEM_DelayMs(2);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	SYSTEM_DelayMs(60);

	switch (Beep) {
	case BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
	case BEEP_500HZ_60MS_DOUBLE_BEEP:
		BK4819_ExitTxMute();
		SYSTEM_DelayMs(60);
		BK4819_EnterTxMute();
		SYSTEM_DelayMs(20);
		// Fallthrough
	case BEEP_1KHZ_60MS_OPTIONAL:
		BK4819_ExitTxMute();
		Duration = 60;
		break;
	case BEEP_440HZ_500MS:
	default:
		BK4819_ExitTxMute();
		Duration = 500;
		break;
	}

	SYSTEM_DelayMs(Duration);
	BK4819_EnterTxMute();
	SYSTEM_DelayMs(20);
	GPIO_ClearBit(&GPIOC->DATA,4);

	g_200003B6 = 80;

	SYSTEM_DelayMs(5);
	BK4819_TurnsOffTones_TurnsOnRX();
	SYSTEM_DelayMs(5);
	BK4819_WriteRegister(BK4819_REG_71, ToneConfig);
	if (g_2000036B == 1) {
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	}
	if (gFmMute == true) {
		BK1080_Mute(false);
	}
	if (gCurrentFunction == FUNCTION_POWER_SAVE && gThisCanEnable_BK4819_Rxon) {
		BK4819_Sleep();
	}
}

void AUDIO_PlayVoice(VOICE_ID_t VoiceID)
{
	uint8_t i;

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
	SYSTEM_DelayMs(7);
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
	for (i = 0; i < 8; i++) {
		if ((VoiceID & 0x80U) == 0) {
			GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_1);
		} else {
			GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_VOICE_1);
		}
		SYSTICK_DelayUs(1200);
		GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
		SYSTICK_DelayUs(1200);
		GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
		VoiceID <<= 1;
	}
}

void AUDIO_PlaySingleVoice(bool bFlag)
{
	VOICE_ID_t VoiceID;
	uint8_t Delay;

	VoiceID = gVoiceID[0];
	if (gEeprom.VOICE_PROMPT != VOICE_PROMPT_OFF && gVoiceWriteIndex) {
		if (gEeprom.VOICE_PROMPT == VOICE_PROMPT_CHINESE) {
			// Chinese
			if (VoiceID >= sizeof(VoiceClipLengthChinese)) {
				goto Bailout;
			}
			Delay = VoiceClipLengthChinese[VoiceID];
			VoiceID += VOICE_ID_CHI_BASE;
		} else {
			// English
			if (VoiceID >= sizeof(VoiceClipLengthEnglish)) {
				goto Bailout;
			}
			Delay = VoiceClipLengthEnglish[VoiceID];
			VoiceID += VOICE_ID_ENG_BASE;
		}

		if (gCurrentFunction == FUNCTION_4 || gCurrentFunction == FUNCTION_2) {
			BK4819_SetAF(BK4819_AF_MUTE);
		}
		if (gFmMute) {
			BK1080_Mute(gFmMute);
		}
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		g_200003B6 = 2000;
		SYSTEM_DelayMs(5);
		AUDIO_PlayVoice(VoiceID);
		if (gVoiceWriteIndex == 1) {
			Delay += 3;
		}
		if (bFlag) {
			SYSTEM_DelayMs(Delay * 10);
			if (gCurrentFunction == FUNCTION_4 || gCurrentFunction == FUNCTION_2) {
				if (gInfoCHAN_A->IsAM == true) {
					BK4819_SetAF(BK4819_AF_AM);
				} else {
					BK4819_SetAF(BK4819_AF_OPEN);
				}
			}
			if (gFmMute == true) {
				BK1080_Mute(false);
			}
			if (g_2000036B == 0) {
				GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
			}
			gVoiceWriteIndex = 0;
			gVoiceReadIndex = 0;
			g_200003B6 = 80;
			return;
		}
		gVoiceReadIndex = 1;
		gCountdownToPlayNextVoice = Delay;
		gFlagPlayQueuedVoice = false;
		return;
	}

Bailout:
	gVoiceReadIndex = 0;
	gVoiceWriteIndex = 0;
}

void AUDIO_SetVoiceID(uint8_t Index, VOICE_ID_t VoiceID)
{
	if (Index >= 8) {
		return;
	}
	if (Index == 0) {
		gVoiceWriteIndex = 0;
		gVoiceReadIndex = 0;
	}

	gVoiceID[Index] = VoiceID;
	gVoiceWriteIndex++;
}

uint8_t AUDIO_SetDigitVoice(uint8_t Index, uint32_t Value)
{
	uint16_t Remainder;
	uint8_t Result;
	uint8_t Count;

	if (Index == 0) {
		gVoiceWriteIndex = 0;
		gVoiceReadIndex = 0;
	}

	Count = 0;
	Result = Value / 1000;
	Remainder = Value % 1000;
	if (Remainder >= 100) {
		Result = Remainder / 100;
		gVoiceID[gVoiceWriteIndex++] = Result;
		Count++;
		Remainder -= Result * 100;
	}
	if (Remainder >= 10) {
		Result = Remainder / 10;
		gVoiceID[gVoiceWriteIndex++] = Result;
		Count += 1;
		Remainder -= Result * 10;
	}

	gVoiceID[gVoiceWriteIndex++] = Remainder;

	return Count + 1;
}

void AUDIO_PlayQueuedVoice(void)
{
	uint8_t VoiceID;
	uint8_t Delay;
	bool Skip;

	Skip = false;
	gVoiceReadIndex = gVoiceReadIndex;
	if (gVoiceReadIndex != gVoiceWriteIndex && gEeprom.VOICE_PROMPT != VOICE_PROMPT_OFF) {
		VoiceID = gVoiceID[gVoiceReadIndex];
		if (gEeprom.VOICE_PROMPT == VOICE_PROMPT_CHINESE) {
			if (VoiceID < 58) {
				Delay = VoiceClipLengthChinese[VoiceID];
				VoiceID += VOICE_ID_CHI_BASE;
			} else {
				Skip = true;
			}
		} else {
			if (VoiceID < 76) {
				Delay = VoiceClipLengthEnglish[VoiceID];
				VoiceID += VOICE_ID_ENG_BASE;
			} else {
				Skip = true;
			}
		}
		gVoiceReadIndex++;
		if (!Skip) {
			if (gVoiceReadIndex == gVoiceWriteIndex) {
				Delay += 3;
			}
			AUDIO_PlayVoice(VoiceID);
			gCountdownToPlayNextVoice = Delay;
			gFlagPlayQueuedVoice = false;
			g_200003B6 = 2000;
			return;
		}
	}

	if (gCurrentFunction == FUNCTION_4 || gCurrentFunction == FUNCTION_2) {
		if (gInfoCHAN_A->IsAM == true) {
			BK4819_SetAF(BK4819_AF_AM);
		} else {
			BK4819_SetAF(BK4819_AF_OPEN);
		}
	}
	if (gFmMute == true) {
		BK1080_Mute(false);
	}
	if (g_2000036B == 0) {
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	}
	g_200003B6 = 80;
	gVoiceWriteIndex = 0;
	gVoiceReadIndex = 0;
}

