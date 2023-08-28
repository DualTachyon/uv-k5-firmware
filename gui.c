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
#include "audio.h"
#include "battery.h"
#include "bitmaps.h"
#include "driver/eeprom.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "fm.h"
#include "functions.h"
#include "gui.h"
#include "helper.h"
#include "misc.h"
#include "settings.h"
#include "ui/aircopy.h"
#include "ui/fmradio.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/menu.h"
#include "ui/scanner.h"

GUI_DisplayType_t gScreenToDisplay;
volatile int8_t gStepDirection;
GUI_DisplayType_t gRequestDisplayScreen;
uint8_t g_200003BB;
bool gWasFKeyPressed;

uint8_t gAskForConfirmation;
bool gAskToSave;
bool gAskToDelete;

void GUI_DisplayBatteryLevel(uint8_t BatteryLevel)
{
	const uint8_t *pBitmap;
	bool bClearMode = false;

	if (gCurrentFunction != 1) {
		switch (BatteryLevel) {
		case 0:
			pBitmap = NULL;
			bClearMode = 1;
			break;
		case 1:
			pBitmap = BITMAP_BatteryLevel1;
			break;
		case 2:
			pBitmap = BITMAP_BatteryLevel2;
			break;
		case 3:
			pBitmap = BITMAP_BatteryLevel3;
			break;
		case 4:
			pBitmap = BITMAP_BatteryLevel4;
			break;
		default:
			pBitmap = BITMAP_BatteryLevel5;
			break;
		}
		ST7565_DrawLine(110, 0, 18, pBitmap, bClearMode);
	}
}

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
			sprintf(WelcomeString0, "VOLTAGE");
			sprintf(WelcomeString1, "%.2fV", gBatteryVoltageAverage * 0.01);
		} else {
			EEPROM_ReadBuffer(0x0EB0, WelcomeString0, 16);
			EEPROM_ReadBuffer(0x0EC0, WelcomeString1, 16);
		}
		UI_PrintString(WelcomeString0, 0, 127, 1, 10, true);
		UI_PrintString(WelcomeString1, 0, 127, 3, 10, true);
		ST7565_BlitStatusLine();
		ST7565_BlitFullScreen();
	}
}

void GUI_PasswordScreen(void)
{
	KEY_Code_t Key;
	BEEP_Type_t Beep;

	gUpdateDisplay = true;
	memset(gInputBox, 10, sizeof(gInputBox));

	while (1) {
		while (!gNextTimeslice) {
		}
		// TODO: Original code doesn't do the below, but is needed for proper key debounce.
		gNextTimeslice = false;
		Key = KEYBOARD_Poll();
		if (gKeyReading0 == Key) {
			gDebounceCounter++;
			if (gDebounceCounter == 2) {
				if (Key == KEY_INVALID) {
					gKeyReading1 = KEY_INVALID;
				} else {
					gKeyReading1 = Key;
					switch (Key) {
					case KEY_0: case KEY_1: case KEY_2: case KEY_3:
					case KEY_4: case KEY_5: case KEY_6: case KEY_7:
					case KEY_8: case KEY_9:
						INPUTBOX_Append(Key - KEY_0);
						if (gInputBoxIndex < 6) {
							Beep = BEEP_1KHZ_60MS_OPTIONAL;
						} else {
							uint32_t Password;

							gInputBoxIndex = 0;
							NUMBER_Get(gInputBox, &Password);
							if ((gEeprom.POWER_ON_PASSWORD * 100) == Password) {
								AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
								return;
							}
							memset(gInputBox, 10, sizeof(gInputBox));
							Beep = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
						}
						AUDIO_PlayBeep(Beep);
						gUpdateDisplay = true;
						break;
					case KEY_EXIT: // Delete digit
						if (gInputBoxIndex) {
							gInputBoxIndex -= 1;
							gInputBox[gInputBoxIndex] = 10;
							gUpdateDisplay = true;
						}
						AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
					default:
						break;
					}
				}
				gKeyBeingHeld = false;
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
	strcpy(String, "LOCK");
	UI_PrintString(String, 0, 127, 1, 10, true);
	for (i = 0; i < 6; i++) {
		if (gInputBox[i] == 10) {
			String[i] = '-';
		} else {
			String[i] = '*';
		}
	}
	String[6] = 0;
	UI_PrintString(String, 0, 127, 3, 12, true);
	ST7565_BlitStatusLine();
	ST7565_BlitFullScreen();
}

void GUI_DisplayStatusLine(void)
{
	memset(gStatusLine, 0, sizeof(gStatusLine));
	if (gCurrentFunction == FUNCTION_POWER_SAVE) {
		memcpy(gStatusLine, BITMAP_PowerSave, sizeof(BITMAP_PowerSave));
	}
	if (gBatteryDisplayLevel < 2) {
		if (gLowBatteryBlink == 1) {
			memcpy(gStatusLine + 110, BITMAP_BatteryLevel1, sizeof(BITMAP_BatteryLevel1));
		}
	} else {
		if (gBatteryDisplayLevel == 2) {
			memcpy(gStatusLine + 110, BITMAP_BatteryLevel2, sizeof(BITMAP_BatteryLevel2));
		} else if (gBatteryDisplayLevel == 3) {
			memcpy(gStatusLine + 110, BITMAP_BatteryLevel3, sizeof(BITMAP_BatteryLevel3));
		} else if (gBatteryDisplayLevel == 4) {
			memcpy(gStatusLine + 110, BITMAP_BatteryLevel4, sizeof(BITMAP_BatteryLevel4));
		} else {
			memcpy(gStatusLine + 110, BITMAP_BatteryLevel5, sizeof(BITMAP_BatteryLevel5));
		}
	}
	if (gChargingWithTypeC) {
		memcpy(gStatusLine + 100, BITMAP_USB_C, sizeof(BITMAP_USB_C));
	}
	if (gEeprom.KEY_LOCK) {
		memcpy(gStatusLine + 90, BITMAP_KeyLock, sizeof(BITMAP_KeyLock));
	} else if (gWasFKeyPressed) {
		memcpy(gStatusLine + 90, BITMAP_F_Key, sizeof(BITMAP_F_Key));
	}

	if (gEeprom.VOX_SWITCH) {
		memcpy(gStatusLine + 71, BITMAP_VOX, sizeof(BITMAP_VOX));
	}
	if (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) {
		memcpy(gStatusLine + 58, BITMAP_WX, sizeof(BITMAP_WX));
	}
	if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
		memcpy(gStatusLine + 45, BITMAP_TDR, sizeof(BITMAP_TDR));
	}
	if (gEeprom.VOICE_PROMPT != VOICE_PROMPT_OFF) {
		memcpy(gStatusLine + 34, BITMAP_VoicePrompt, sizeof(BITMAP_VoicePrompt));
	}
	if (gSetting_KILLED) {
		memset(gStatusLine + 21, 0xFF, 10);
	}
	else if (gFmRadioMode) {
		memcpy(gStatusLine + 21, BITMAP_FM, sizeof(BITMAP_FM));
	}
	if (gIsNoaaMode) {
		memcpy(gStatusLine + 7, BITMAP_NOAA, sizeof(BITMAP_NOAA));
	}
	ST7565_BlitStatusLine();
}

//

void GUI_DisplayScreen(void)
{
	switch (gScreenToDisplay) {
	case DISPLAY_MAIN:
		UI_DisplayMain();
		break;
	case DISPLAY_FM:
		UI_DisplayFM();
		break;
	case DISPLAY_MENU:
		UI_DisplayMenu();
		break;
	case DISPLAY_SCANNER:
		UI_DisplayScanner();
		break;
	case DISPLAY_AIRCOPY:
		UI_DisplayAircopy();
		break;
	default:
		break;
	}
}

void GUI_SelectNextDisplay(GUI_DisplayType_t Display)
{
	if (Display != DISPLAY_INVALID) {
		if (gScreenToDisplay != Display) {
			gInputBoxIndex = 0;
			gIsInSubMenu = false;
			g_20000381 = 0;
			gStepDirection = 0;
			g_20000390 = 0;
			gAskForConfirmation = 0;
			g_200003BA = 0;
			g_200003BB = 0;
			gF_LOCK = false;
			gAskToSave = false;
			gAskToDelete = false;
			if (gWasFKeyPressed) {
				gWasFKeyPressed = false;
				g_2000036F = 1;
			}
		}
		gUpdateDisplay = true;
		gScreenToDisplay = Display;
	}
}

