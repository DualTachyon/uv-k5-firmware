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

#include "fm.h"
#include "settings.h"

uint16_t gFM_Channels[20];
bool gFmMute;

bool FM_CheckValidChannel(uint8_t Channel)
{
	if (Channel < 20 && gFM_Channels[Channel] - 760 < 321) {
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

