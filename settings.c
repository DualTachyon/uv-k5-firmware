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

