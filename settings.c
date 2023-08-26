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
#include "driver/eeprom.h"
#include "driver/uart.h"
#include "fm.h"
#include "misc.h"
#include "settings.h"

EEPROM_Config_t gEeprom;

void SETTINGS_SaveFM(void)
{
	uint8_t i;
	struct {
		uint16_t Frequency;
		uint8_t Channel;
		bool IsChannelSelected;
		uint8_t Padding[4];
	} State;

	UART_LogSend("sFm\r\n", 5);

	memset(&State, 0xFF, sizeof(State));
	State.Channel = gEeprom.FM_CurrentChannel;
	State.Frequency = gEeprom.FM_CurrentFrequency;
	State.IsChannelSelected = gEeprom.FM_IsChannelSelected;

	EEPROM_WriteBuffer(0x0E88, &State);
	for (i = 0; i < 5; i++) {
		EEPROM_WriteBuffer(0x0E40 + (i * 8), &gFM_Channels[i * 4]);
	}
}

void SETTINGS_SaveVfoIndices(void)
{
	uint8_t State[8];

	UART_LogSend("sidx\r\n", 6);

	State[0] = gEeprom.VfoChannel[0];
	State[1] = gEeprom.EEPROM_0E81_0E84[0];
	State[2] = gEeprom.EEPROM_0E82_0E85[0];
	State[3] = gEeprom.VfoChannel[1];
	State[4] = gEeprom.EEPROM_0E81_0E84[1];
	State[5] = gEeprom.EEPROM_0E82_0E85[1];
	State[6] = gEeprom.EEPROM_0E86;
	State[7] = gEeprom.EEPROM_0E87;

	EEPROM_WriteBuffer(0x0E80, State);
}

void SETTINGS_SaveSettings(void)
{
	uint8_t State[8];
	uint32_t Password[2];

	UART_LogSend("spub\r\n", 6);

	State[0] = gEeprom.CHAN_1_CALL;
	State[1] = gEeprom.SQUELCH_LEVEL;
	State[2] = gEeprom.TX_TIMEOUT_TIMER;
	State[3] = gEeprom.NOAA_AUTO_SCAN;
	State[4] = gEeprom.KEY_LOCK;
	State[5] = gEeprom.VOX_SWITCH;
	State[6] = gEeprom.VOX_LEVEL;
	State[7] = gEeprom.MIC_SENSITIVITY;

	EEPROM_WriteBuffer(0x0E70, State);

	State[0] = 0xFF;
	State[1] = gEeprom.CHANNEL_DISPLAY_MODE;
	State[2] = gEeprom.CROSS_BAND_RX_TX;
	State[3] = gEeprom.BATTERY_SAVE;
	State[4] = gEeprom.DUAL_WATCH;
	State[5] = gEeprom.BACKLIGHT;
	State[6] = gEeprom.TAIL_NOTE_ELIMINATION;
	State[7] = gEeprom.VFO_OPEN;

	EEPROM_WriteBuffer(0x0E78, State);

	State[0] = gEeprom.BEEP_CONTROL;
	State[1] = gEeprom.KEY_1_SHORT_PRESS_ACTION;
	State[2] = gEeprom.KEY_1_LONG_PRESS_ACTION;
	State[3] = gEeprom.KEY_2_SHORT_PRESS_ACTION;
	State[4] = gEeprom.KEY_2_LONG_PRESS_ACTION;
	State[5] = gEeprom.SCAN_RESUME_MODE;
	State[6] = gEeprom.AUTO_KEYPAD_LOCK;
	State[7] = gEeprom.POWER_ON_DISPLAY_MODE;

	EEPROM_WriteBuffer(0x0E90, State);

	memset(Password, 0xFF, sizeof(Password));

	Password[0] = gEeprom.POWER_ON_PASSWORD;

	EEPROM_WriteBuffer(0x0E98, State);

	memset(State, 0xFF, sizeof(State));

	State[0] = gEeprom.VOICE_PROMPT;

	EEPROM_WriteBuffer(0x0EA0, State);

	State[0] = gEeprom.ALARM_MODE;
	State[1] = gEeprom.ROGER;
	State[2] = gEeprom.REPEATER_TAIL_TONE_ELIMINATION;
	State[3] = gEeprom.TX_CHANNEL;

	EEPROM_WriteBuffer(0x0EA8, State);

	State[0] = gEeprom.DTMF_SIDE_TONE;
	State[1] = gEeprom.DTMF_SEPARATE_CODE;
	State[2] = gEeprom.DTMF_GROUP_CALL_CODE;
	State[3] = gEeprom.DTMF_DECODE_RESPONSE;
	State[4] = gEeprom.DTMF_AUTO_RESET_TIME;
	State[5] = gEeprom.DTMF_PRELOAD_TIME / 10U;
	State[6] = gEeprom.DTMF_FIRST_CODE_PERSIST_TIME / 10U;
	State[7] = gEeprom.DTMF_HASH_CODE_PERSIST_TIME / 10U;

	EEPROM_WriteBuffer(0x0ED0, State);

	memset(State, 0xFF, sizeof(State));

	State[0] = gEeprom.DTMF_CODE_PERSIST_TIME / 10U;
	State[1] = gEeprom.DTMF_CODE_INTERVAL_TIME / 10U;
	State[2] = gEeprom.PERMIT_REMOTE_KILL;

	EEPROM_WriteBuffer(0x0ED8, State);

	State[0] = gEeprom.SCAN_LIST_DEFAULT;
	State[1] = gEeprom.SCAN_LIST_ENABLED[0];
	State[2] = gEeprom.SCANLIST_PRIORITY_CH1[0];
	State[3] = gEeprom.SCANLIST_PRIORITY_CH2[0];
	State[4] = gEeprom.SCAN_LIST_ENABLED[1];
	State[5] = gEeprom.SCANLIST_PRIORITY_CH1[1];
	State[6] = gEeprom.SCANLIST_PRIORITY_CH2[1];
	State[7] = 0xFF;

	EEPROM_WriteBuffer(0x0F18, State);

	memset(State, 0xFF, sizeof(State));

	State[0] = gSetting_F_LOCK;
	State[1] = gSetting_350TX;
	State[2] = gSetting_KILLED;
	State[3] = gSetting_200TX;
	State[4] = gSetting_500TX;
	State[5] = gSetting_350EN;
	State[6] = gSetting_ScrambleEnable;

	EEPROM_WriteBuffer(0x0F40, State);
}

