#include "fm.h"
#include "settings.h"

uint16_t gFM_Channels[20];

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

