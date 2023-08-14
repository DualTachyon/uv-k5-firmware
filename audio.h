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

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

enum BEEP_Type_t {
	BEEP_1KHZ_60MS_OPTIONAL = 1U,
	BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL = 2U,
	BEEP_440HZ_500MS = 3U,
	BEEP_500HZ_60MS_DOUBLE_BEEP = 4U,
};

typedef enum BEEP_Type_t BEEP_Type_t;

enum VOICE_ID_t {
	VOICE_ID_ENG_0				= 0x60U,
	VOICE_ID_ENG_1				= 0x61U,
	VOICE_ID_ENG_2				= 0x62U,
	VOICE_ID_ENG_3				= 0x63U,
	VOICE_ID_ENG_4				= 0x64U,
	VOICE_ID_ENG_5				= 0x65U,
	VOICE_ID_ENG_6				= 0x66U,
	VOICE_ID_ENG_7				= 0x67U,
	VOICE_ID_ENG_8				= 0x68U,
	VOICE_ID_ENG_9				= 0x69U,
	VOICE_ID_ENG_10				= 0x6AU,
	VOICE_ID_ENG_100			= 0x6BU,
	VOICE_ID_ENG_WELCOME			= 0x6CU,
	VOICE_ID_ENG_LOCK			= 0x6DU,
	VOICE_ID_ENG_M_LOCK			= 0x6EU,
	VOICE_ID_ENG_SCANNING_BEGIN		= 0x6FU,
	VOICE_ID_ENG_SCANNING_STOP		= 0x70U,
	VOICE_ID_ENG_SCRAMBLER_ON		= 0x71U,
	VOICE_ID_ENG_SCRAMBLER_OFF		= 0x72U,
	VOICE_ID_ENG_FUNCTION			= 0x73U,
	VOICE_ID_ENG_CTCSS			= 0x74U,
	VOICE_ID_ENG_DCS			= 0x75U,
	VOICE_ID_ENG_POWER			= 0x76U,
	VOICE_ID_ENG_SAVE_MODE			= 0x77U,
	VOICE_ID_ENG_MEMORY_CHANNEL		= 0x78U,
	VOICE_ID_ENG_DELETE_CHANNEL		= 0x79U,
	VOICE_ID_ENG_FREQUENCY_STEP		= 0x7AU,
	VOICE_ID_ENG_SQUELCH			= 0x7BU,
	VOICE_ID_ENG_TRANSMIT_OVER_TIME		= 0x7CU,
	VOICE_ID_ENG_BACKLIGHT_SELECTION	= 0x7DU,
	VOICE_ID_ENG_VOX			= 0x7EU,
	VOICE_ID_ENG_FREQUENCY_DIRECTION	= 0x7FU,
	VOICE_ID_ENG_OFFSET_FREQUENCY		= 0x80U,
	VOICE_ID_ENG_TRANSMITING_MEMORY		= 0x81U,
	VOICE_ID_ENG_RECEIVING_MEMORY		= 0x82U,
	VOICE_ID_ENG_EMERGENCY_CALL		= 0x83U,
	VOICE_ID_ENG_LOW_VOLTAGE		= 0x84U,
	VOICE_ID_ENG_CHANNEL_MODE		= 0x85U,
	VOICE_ID_ENG_FREQUENCY_MODE		= 0x86U,
	VOICE_ID_ENG_VOICE_PROMPT		= 0x87U,
	VOICE_ID_ENG_BAND_SELECTION		= 0x88U,
	VOICE_ID_ENG_DUAL_STANDBY		= 0x89U,
	VOICE_ID_ENG_CHANNEL_BANDWIDTH		= 0x8AU,
	VOICE_ID_ENG_OPTIONAL_SIGNAL		= 0x8BU,
	VOICE_ID_ENG_MUTE_MODE			= 0x8CU,
	VOICE_ID_ENG_BUSY_LOCKOUT		= 0x8DU,
	VOICE_ID_ENG_BEEP_PROMPT		= 0x8EU,
	VOICE_ID_ENG_ANI_CODE			= 0x8FU,
	VOICE_ID_ENG_INITIALISATION		= 0x90U,
	VOICE_ID_ENG_CONFIRM			= 0x91U,
	VOICE_ID_ENG_CANCEL			= 0x92U,
	VOICE_ID_ENG_ON				= 0x93U,
	VOICE_ID_ENG_OFF			= 0x94U,
	VOICE_ID_ENG_2_TONE			= 0x95U,
	VOICE_ID_ENG_5_TONE			= 0x96U,
	VOICE_ID_ENG_DIGITAL_SIGNAL		= 0x97U,
	VOICE_ID_ENG_REPEATER			= 0x98U,
	VOICE_ID_ENG_MENU			= 0x99U,
	VOICE_ID_ENG_11				= 0x9AU,
	VOICE_ID_ENG_12				= 0x9BU,
	VOICE_ID_ENG_13				= 0x9CU,
	VOICE_ID_ENG_14				= 0x9DU,
	VOICE_ID_ENG_15				= 0x9EU,
	VOICE_ID_ENG_16				= 0x9FU,
	VOICE_ID_ENG_17				= 0xA0U,
	VOICE_ID_ENG_18				= 0xA1U,
	VOICE_ID_ENG_19				= 0xA2U,
	VOICE_ID_ENG_20				= 0xA3U,
	VOICE_ID_ENG_30				= 0xA4U,
	VOICE_ID_ENG_40				= 0xA5U,
	VOICE_ID_ENG_50				= 0xA6U,
	VOICE_ID_ENG_60				= 0xA7U,
	VOICE_ID_ENG_70				= 0xA8U,
	VOICE_ID_ENG_80				= 0xA9U,
	VOICE_ID_ENG_90				= 0xAAU,
	VOICE_ID_ENG_END			= 0xABU,
};

typedef enum VOICE_ID_t VOICE_ID_t;

extern uint8_t gVoiceID[8];
extern uint8_t gVoiceReadIndex;
extern uint8_t gVoiceWriteIndex;
extern volatile uint16_t gCountdownToPlayNextVoice;
extern volatile bool gFlagPlayQueuedVoice;

void AUDIO_PlayBeep(BEEP_Type_t Beep);
void AUDIO_PlayVoice(VOICE_ID_t VoiceID);
void AUDIO_PlaySingleVoice(bool bFlag);
void AUDIO_SetVoiceID(uint8_t Index, VOICE_ID_t VoiceID);
uint8_t AUDIO_SetDigitVoice(uint8_t Index, uint32_t Value);

#endif

