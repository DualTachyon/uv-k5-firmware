#include <string.h>
#include "audio.h"
#include "driver/eeprom.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "font.h"
#include "gui.h"
#include "helper.h"
#include "misc.h"
#include "settings.h"

GUI_DisplayType_t gScreenToDisplay;
uint8_t g_200003C6;
uint8_t g_20000380;
uint8_t g_20000390;
uint8_t g_200003BA;
uint8_t g_200003BB;
uint8_t g_20000367;

bool gAskForConfirmation;
bool gAskToSave;
bool gAskToDelete;


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

void GUI_PasswordScreen(void)
{
	KEY_Code_t Key;
	BEEP_Type_t Beep;

	gUpdateDisplay = true;
	memset(gNumberForPrintf, 10, sizeof(gNumberForPrintf));

	while (1) {
		while (!gMaybeVsync) {
		}
		Key = KEYBOARD_Poll();
		if (gKeyReading0 == Key) {
			gDebounceCounter++;
			if (gDebounceCounter == 2) {
				if (Key == KEY_INVALID) {
					gKeyReading1 = 0xFF;
				} else {
					gKeyReading1 = Key;
					switch (Key) {
					case KEY_0: case KEY_1: case KEY_2: case KEY_3:
					case KEY_4: case KEY_5: case KEY_6: case KEY_7:
					case KEY_8: case KEY_9:
						NUMBER_Append(Key - KEY_0);
						if (gNumberOffset < 6) {
							Beep = BEEP_1KHZ_60MS_OPTIONAL;
						} else {
							uint32_t Password;

							gNumberOffset = 0;
							NUMBER_Get(gNumberForPrintf, &Password);
							if ((gEeprom.POWER_ON_PASSWORD * 100) == Password) {
								AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
								return;
							}
							memset(gNumberForPrintf, 10, sizeof(gNumberForPrintf));
							Beep = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
						}
						AUDIO_PlayBeep(Beep);
						gUpdateDisplay = true;
						break;
					case KEY_EXIT: // Delete digit
						if (gNumberOffset != 0) {
							gNumberOffset -= 1;
							gNumberForPrintf[gNumberOffset] = 10;
							gUpdateDisplay = true;
						}
						AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
					default:
						break;
					}
				}
				//DAT_2000042c = 0;
			}
		} else {
			gDebounceCounter = 0;
			gKeyReading0 = Key;
		}
#if 0
		if (UART_CheckForCommand()) {
			__disable_irq();
			ProcessUartCommand();
			__enable_irq();
		}
#endif
		if (gUpdateDisplay) {
			GUI_LockScreen();
			gUpdateDisplay = false;
		}
	}
}

void GUI_LockScreen(void)
{
	char String[7];
	uint8_t i;

	memset(gStatusLine, 0, sizeof(gStatusLine));
	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	strcat(String, "LOCK");
	GUI_PrintString(String, 0, 127, 1, 10, true);
	for (i = 0; i < 6; i++) {
		if (gNumberForPrintf[i] == 10) {
			String[i] = '-';
		} else {
			String[i] = '*';
		}
	}
	String[6] = 0;
	GUI_PrintString(String, 0, 127, 3, 12, true);
	ST7565_BlitStatusLine();
	ST7565_BlitFullScreen();
}

void GUI_SelectNextDisplay(GUI_DisplayType_t Display)
{
	if (Display != DISPLAY_INVALID) {
		if (gScreenToDisplay != Display) {
			gNumberOffset = 0;
			g_200003C6 = 0;
			g_20000381 = 0;
			g_20000380 = 0;
			g_20000390 = 0;
			gAskForConfirmation = 0;
			g_200003BA = 0;
			g_200003BB = 0;
			gF_LOCK = 0;
			gAskToSave = false;
			gAskToDelete = false;
			if (g_20000367 == 1) {
				g_20000367 = 0;
				g_2000036F = 1;
			}
		}
		gUpdateDisplay = true;
		gScreenToDisplay = Display;
	}
}

