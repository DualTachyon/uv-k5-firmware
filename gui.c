#include <string.h>
#include "driver/eeprom.h"
#include "driver/st7565.h"
#include "font.h"
#include "gui.h"
#include "settings.h"

void GUI_Welcome(void)
{
	char WelcomeString0[16];
	char WelcomeString1[16];

	memset(gStatusLine, 0, sizeof(gStatusLine));
	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));

	if (gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_FULL_SCREEN) {
		ST7565_FillScreen(0xFF);
	} else {
		memset(WelcomeString0, 0, sizeof(WelcomeString0));
		memset(WelcomeString1, 0, sizeof(WelcomeString1));
		if (gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_VOLTAGE) {
			// TODO: Add non bloated sprintf to support the following
			//sprintf(WelcomeString0, "VOLTAGE");
			//sprintf(WelcomeString1, "%.2fV", gBatteryVoltageAverage * 0.01);
		} else {
			EEPROM_ReadBuffer(0x0EB0, WelcomeString0, 16);
			EEPROM_ReadBuffer(0x0EC0, WelcomeString1, 16);
		}
		GUI_PrintString(WelcomeString0, 0, 127, 1, 10, true);
		GUI_PrintString(WelcomeString1, 0, 127, 3, 10, true);
		ST7565_BlitStatusLine();
		ST7565_BlitFullScreen();
	}
}

void GUI_PrintString(const char *pString, uint8_t Start, uint8_t End, uint8_t Line, int Width, bool bCentered)
{
	uint32_t i, Length;

	Length = strlen(pString);
	if (bCentered) {
		Start += (((End - Start) - (Length * Width)) + 1) / 2;
	}
	for (i = 0; i < Length; i++) {
		if (pString[i] - ' ' < 0x5F) {
			uint8_t Index = pString[i] - ' ';
			memcpy(gFrameBuffer[Line + 0] + (i * Width) + Start, &gBigFont[Index][0], 8);
			memcpy(gFrameBuffer[Line + 1] + (i * Width) + Start, &gBigFont[Index][8], 8);
		}
	}
}

