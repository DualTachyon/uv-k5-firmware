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
#include "aircopy.h"
#include "audio.h"
#include "battery.h"
#include "bitmaps.h"
#include "dcs.h"
#include "driver/eeprom.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "font.h"
#include "fm.h"
#include "functions.h"
#include "gui.h"
#include "helper.h"
#include "misc.h"
#include "settings.h"

GUI_DisplayType_t gScreenToDisplay;
uint8_t g_200003C6;
volatile uint8_t gCurrentStep;
volatile int8_t g_20000390;
uint8_t g_200003BA;
uint8_t g_200003BB;
uint8_t gWasFKeyPressed;

bool gAskForConfirmation;
bool gAskToSave;
bool gAskToDelete;

uint8_t gMenuCursor;
uint8_t gMenuScrollDirection;
uint32_t gSubMenuSelection;

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
			memcpy(gFrameBuffer[Line + 0] + (i * Width) + Start, &gFontBig[Index][0], 8);
			memcpy(gFrameBuffer[Line + 1] + (i * Width) + Start, &gFontBig[Index][8], 8);
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
	strcpy(String, "LOCK");
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
	} else if (gWasFKeyPressed == 1) {
		memcpy(gStatusLine + 90, BITMAP_F_Key, sizeof(BITMAP_F_Key));
	}

	if (gEeprom.VOX_SWITCH) {
		memcpy(gStatusLine + 71, BITMAP_VOX, sizeof(BITMAP_VOX));
	}
	if (gEeprom.CROSS_BAND_RX_TX != 0) {
		memcpy(gStatusLine + 58, BITMAP_WX, sizeof(BITMAP_WX));
	}
	if (gEeprom.DUAL_WATCH != 0) {
		memcpy(gStatusLine + 45, BITMAP_TDR, sizeof(BITMAP_TDR));
	}
	if (gEeprom.KEYPAD_TONE != 0) {
		memcpy(gStatusLine + 34, BITMAP_KEYPAD_TONE, sizeof(BITMAP_KEYPAD_TONE));
	}
	if (gSetting_KILLED) {
		memset(gStatusLine + 21, 0xFF, 10);
	}
	else if (gFmMute) {
		memcpy(gStatusLine + 21, BITMAP_FM_Mute, sizeof(BITMAP_FM_Mute));
	}
	if (gIsNoaaMode) {
		memcpy(gStatusLine + 7, BITMAP_NOAA, sizeof(BITMAP_NOAA));
	}
	ST7565_BlitStatusLine();
}

//

static void GenerateChannelString(char *pString, uint8_t Channel)
{
	uint8_t i;

	if (gNumberOffset == 0) {
		sprintf(pString, "CH-%02d", Channel + 1);
		return;
	}

	pString[0] = 'C';
	pString[1] = 'H';
	pString[2] = '-';

	for (i = 0; i < 2; i++) {
		if (gNumberForPrintf[i] == 10) {
			pString[i + 3] = '-';
		} else {
			pString[i + 3] = gNumberForPrintf[i] + '0';
		}
	}

}

static void GenerateChannelStringEx(char *pString, bool bShowPrefix, uint8_t ChannelNumber)
{
	if (gNumberOffset) {
		uint8_t i;

		for (i = 0; i < 3; i++) {
			if (gNumberForPrintf[i] == 10) {
				pString[i] = '-';
			} else {
				pString[i] = gNumberForPrintf[i] + '0';
			}
		}
		return;
	}

	if (bShowPrefix) {
		sprintf(pString, "CH-%03d", ChannelNumber + 1);
	} else {
		if (ChannelNumber == 0xFF) {
			strcpy(pString, "NULL");
		} else {
			sprintf(pString, "%03d", ChannelNumber + 1);
		}
	}
}

void GUI_DisplayFrequency(const char *pDigits, uint8_t X, uint8_t Y, bool bDisplayLeadingZero, bool Flag1)
{
	uint8_t *pFb0, *pFb1;
	bool bCanDisplay;
	uint8_t i;

	pFb0 = gFrameBuffer[Y] + X;
	pFb1 = pFb0 + 128;

	bCanDisplay = false;
	for (i = 0; i < 3; i++) {
		uint8_t Digit;

		Digit = pDigits[i];
		if (bDisplayLeadingZero || bCanDisplay || Digit) {
			bCanDisplay = true;
			memcpy(pFb0 + (i * 13), gFontBigDigits[Digit] +  0, 13);
			memcpy(pFb1 + (i * 13), gFontBigDigits[Digit] + 13, 13);
		} else if (Flag1) {
			pFb1 = pFb1 - 6;
			pFb0 = pFb0 - 6;
		}
	}

	pFb1[0x27] = 0x60;
	pFb1[0x28] = 0x60;
	pFb1[0x29] = 0x60;

	for (i = 0; i < 3; i++) {
		const uint8_t Digit = pDigits[i + 3];

		memcpy(pFb0 + (i * 13) + 42, gFontBigDigits[Digit] +  0, 13);
		memcpy(pFb1 + (i * 13) + 42, gFontBigDigits[Digit] + 13, 13);
	}
}

void GUI_DisplaySmallDigits(uint8_t Size, const char *pString, uint8_t x, uint8_t y)
{
	uint8_t i;

	for (i = 0; i < Size; i++) {
		memcpy(gFrameBuffer[y] + (i * 7) + x, gFontSmallDigits[(uint8_t)pString[i]], 7);
	}
}

static void DisplayMain(void)
{
	uint8_t i;

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	if (gEeprom.KEY_LOCK && g_200003A9) {
		GUI_PrintString("Long Press #", 0, 127, 1, 8, true);
		GUI_PrintString("To Unlock", 0, 127, 3, 8, true);
		ST7565_BlitFullScreen();
		return;
	}

	for (i = 0; i < 2; i++) {
		// TODO
	}

	ST7565_BlitFullScreen();
}

static void DisplayFM(void)
{
	uint8_t i;
	char String[16];
						        
	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	memset(String, 0, sizeof(String));
	strcpy(String, "FM");

	GUI_PrintString(String, 0, 127, 0, 12, true);
	memset(String, 0, sizeof(String));

	if (gAskToSave) {
		strcpy(String, "SAVE?");
	} else if (gAskToDelete) {
		strcpy(String, "DEL?");
	} else {
		if (g_20000390 == 0) {
			if (gEeprom.FM_IsChannelSelected == false) {
				for (i = 0; i < 20; i++) {
					if (gEeprom.FM_FrequencyToPlay == gFM_Channels[i]) {
						sprintf(String, "VFO(CH%02d)", i + 1);
						break;
					}
				}
				if (i == 20) {
					strcpy(String, "VFO");
				}
			} else {
				sprintf(String, "MR(CH%02d)", gEeprom.FM_CurrentChannel + 1);
			}
		} else {
			if (gIs_A_Scan == '\0') {
				strcpy(String, "M-SCAN");
			} else {
				sprintf(String, "A-SCAN(%d)", gA_Scan_Channel + 1);
			}
		}
	}

	GUI_PrintString(String, 0, 127, 2, 10, true);
	memset(String, 0, sizeof(String));

	if (gAskToSave || (gEeprom.FM_IsChannelSelected && gNumberOffset)) {
		GenerateChannelString(String, gA_Scan_Channel);
	} else if (gAskToDelete) {
		if (gNumberOffset == 0) {
			NUMBER_ToDigits(gEeprom.FM_FrequencyToPlay * 10000, String);
			GUI_DisplayFrequency(String, 23, 4, false, true);
		} else {
			GUI_DisplayFrequency(gNumberForPrintf, 23, 4, true, false);
		}
		ST7565_BlitFullScreen();
		return;
	} else {
		sprintf(String, "CH-%02d", gEeprom.FM_CurrentChannel + 1);
	}

	GUI_PrintString(String, 0, 127, 4, 10, true);
	ST7565_BlitFullScreen();
}

static void DisplayScanner(void)
{
	char String[16];
	bool bCentered;
	uint8_t Start;

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	memset(String, 0, sizeof(String));

	if (g_20000458 == 1 || (gScanState != 0 && gScanState != 3)) {
		sprintf(String, "FREQ:%.5f", gScanFrequency * 1e-05);
	} else {
		sprintf(String, "FREQ:**.*****");
	}
	GUI_PrintString(String, 2, 127, 1, 8, 0);
	memset(String, 0, sizeof(String));

	if (gScanState < 2 || g_2000045C != 1) {
		sprintf(String, "CTC:******");
	} else if (g_2000045A == 1) {
		sprintf(String, "CTC:%.1fHz", CTCSS_Options[g_2000045B + 1] * 0.1);
	} else {
		sprintf(String, "DCS:D%03oN", DCS_Options[g_2000045B]);
	}
	GUI_PrintString(String, 2, 127, 3, 8, 0);
	memset(String, 0, sizeof(String));

	if (g_20000461 == 2) {
		strcpy(String, "SAVE?");
		Start = 0;
		bCentered = 1;
	} else {
		if (g_20000461 == 1) {
			strcpy(String, "SAVE:");
			GenerateChannelStringEx(String + 5, gShowChPrefix, gScanChannel);
		} else if (gScanState < 2) {
			strcpy(String, "SCAN");
			memset(String + 4, '.', (g_20000464 & 7) + 1);
		} else {
			if (gScanState == 2) {
				strcpy(String, "SCAN CMP.");
			} else {
				strcpy(String, "SCAN FAIL.");
			}
		}
		Start = 2;
		bCentered = 0;
	}

	GUI_PrintString(String, Start, 127, 5, 8, bCentered);
	ST7565_BlitFullScreen();
}

static void DisplayAircopy(void)
{
	char String[16];

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	if (gAircopyState == AIRCOPY_READY) {
		strcpy(String, "AIR COPY(RDY)");
	} else if (gAircopyState == AIRCOPY_TRANSFER) {
		strcpy(String, "AIR COPY");
	} else {
		strcpy(String, "AIR COPY(CMP)");
	}
	GUI_PrintString(String, 2, 127, 0, 8, true);

	if (gNumberOffset == 0) {
		NUMBER_ToDigits(gInfoCHAN_A->DCS[0].Frequency, String);
		GUI_DisplayFrequency(String, 16, 2, 0, 0);
		GUI_DisplaySmallDigits(2, String + 6, 97, 3);
	} else {
		GUI_DisplayFrequency(gNumberForPrintf, 16, 2, 1, 0);
	}

	memset(String, 0, sizeof(String));

	if (gAirCopyIsSendMode == 0) {
		sprintf(String, "RCV:%d E:%d", gAirCopyBlockNumber, gErrorsDuringAirCopy);
	} else if (gAirCopyIsSendMode == 1) {
		sprintf(String, "SND:%d", gAirCopyBlockNumber);
	}
	GUI_PrintString(String, 2, 127, 4, 8, true);
	ST7565_BlitFullScreen();
}

void GUI_DisplayScreen(void)
{
	switch (gScreenToDisplay) {
	case DISPLAY_MAIN:
		DisplayMain();
		break;
	case DISPLAY_FM:
		DisplayFM();
		break;
	case DISPLAY_MENU:
//		DisplayMenu();
		break;
	case DISPLAY_SCANNER:
		DisplayScanner();
		break;
	case DISPLAY_AIRCOPY:
		DisplayAircopy();
		break;
	default:
		break;
	}
}

void GUI_SelectNextDisplay(GUI_DisplayType_t Display)
{
	if (Display != DISPLAY_INVALID) {
		if (gScreenToDisplay != Display) {
			gNumberOffset = 0;
			g_200003C6 = 0;
			g_20000381 = 0;
			gCurrentStep = 0;
			g_20000390 = 0;
			gAskForConfirmation = 0;
			g_200003BA = 0;
			g_200003BB = 0;
			gF_LOCK = 0;
			gAskToSave = false;
			gAskToDelete = false;
			if (gWasFKeyPressed == 1) {
				gWasFKeyPressed = 0;
				g_2000036F = 1;
			}
		}
		gUpdateDisplay = true;
		gScreenToDisplay = Display;
	}
}

void GUI_RenderRSSI(uint8_t RssiLevel, uint8_t VFO)
{
	uint8_t *pLine;
	uint8_t Line;
	bool bIsClearMode;

	if (gCurrentFunction == FUNCTION_TRANSMIT || gScreenToDisplay != DISPLAY_MAIN) {
		return;
	}

	if (VFO == 0) {
		pLine = gFrameBuffer[2];
		Line = 3;
	} else {
		pLine = gFrameBuffer[6];
		Line = 7;
	}

	memset(pLine, 0, 23);
	if (RssiLevel == 0) {
		pLine = NULL;
		bIsClearMode = true;
	} else {
		memcpy(pLine, BITMAP_Antenna,5);
		memcpy(pLine + 5,BITMAP_AntennaLevel1, sizeof(BITMAP_AntennaLevel1));
		if (RssiLevel >= 2) {
			memcpy(pLine + 8, BITMAP_AntennaLevel2, sizeof(BITMAP_AntennaLevel2));
		}
		if (RssiLevel >= 3) {
			memcpy(pLine + 11, BITMAP_AntennaLevel3, sizeof(BITMAP_AntennaLevel3));
		}
		if (RssiLevel >= 4) {
			memcpy(pLine + 14, BITMAP_AntennaLevel4, sizeof(BITMAP_AntennaLevel4));
		}
		if (RssiLevel >= 5) {
			memcpy(pLine + 17, BITMAP_AntennaLevel5, sizeof(BITMAP_AntennaLevel5));
		}
		if (RssiLevel >= 6) {
			memcpy(pLine + 20, BITMAP_AntennaLevel6, sizeof(BITMAP_AntennaLevel6));
		}
		bIsClearMode = false;
	}
	ST7565_DrawLine(0, Line, 23 , pLine, bIsClearMode);
}

void GUI_DisplayRSSI(uint16_t RSSI)
{
	uint8_t Level;

	if (RSSI >= gEEPROM_RSSI_CALIB[gInfoCHAN_A->Band][3]) {
		Level = 6;
	} else if (RSSI >= gEEPROM_RSSI_CALIB[gInfoCHAN_A->Band][2]) {
		Level = 4;
	} else if (RSSI >= gEEPROM_RSSI_CALIB[gInfoCHAN_A->Band][1]) {
		Level = 2;
	} else if (RSSI >= gEEPROM_RSSI_CALIB[gInfoCHAN_A->Band][0]) {
		Level = 1;
	} else {
		Level = 0;
	}

	if (gVFO_RSSI_Level[gEeprom.RX_CHANNEL] != Level) {
		gVFO_RSSI_Level[gEeprom.RX_CHANNEL] = Level;
		GUI_RenderRSSI(Level, gEeprom.RX_CHANNEL);
	}
}

