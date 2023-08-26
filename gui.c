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
#include "dtmf.h"
#include "external/printf/printf.h"
#include "font.h"
#include "fm.h"
#include "functions.h"
#include "gui.h"
#include "helper.h"
#include "misc.h"
#include "settings.h"

static const char MenuList[][7] = {
	// 0x00
	"SQL",    "STEP",    "TXP",    "R_DCS",
	"R_CTCS", "T_DCS",   "T_CTCS", "SFT-D",
	// 0x08
	"OFFSET", "W/N",     "SCR",    "BCL",
	"MEM-CH", "SAVE",    "VOX",    "ABR",
	// 0x10
	"TDR",    "WX",      "BEEP",   "TOT",
	"VOICE",  "SC-REV",  "MDF",    "AUTOLK",
	// 0x18
	"S-ADD1", "S-ADD2",  "STE",    "RP-STE",
	"MIC",    "1-CALL",  "S-LIST", "SLIST1",
	// 0x20
	"SLIST2", "AL-MOD",  "ANI-ID", "UPCODE",
	"DWCODE", "D-ST",    "D-RSP",  "D-HOLD",
	// 0x28
	"D-PRE",  "PTT-ID",  "D-DCD",  "D-LIST",
	"PONMSG", "ROGER",   "VOL",    "AM",
	// 0x30
	"NOAA_S", "DEL-CH",  "RESET",  "350TX",
	"F-LOCK", "200TX",   "500TX",  "350EN",
	// 0x38
	"SCREN",
};

static const uint16_t gSubMenu_Step[] = {
	250,
	500,
	625,
	1000,
	1250,
	2500,
	833,
};

static const char gSubMenu_TXP[3][5] = {
	"LOW",
	"MID",
	"HIGH",
};

static const char gSubMenu_SFT_D[3][4] = {
	"OFF",
	"+",
	"-",
};

static const char gSubMenu_W_N[2][7] = {
	"WIDE",
	"NARROW",
};

static const char gSubMenu_OFF_ON[2][4] = {
	"OFF",
	"ON",
};

static const char gSubMenu_SAVE[5][4] = {
	"OFF",
	"1:1",
	"1:2",
	"1:3",
	"1:4",
};

static const char gSubMenu_CHAN[3][7] = {
	"OFF",
	"CHAN_A",
	"CHAN_B",
};

static const char gSubMenu_VOICE[3][4] = {
	"OFF",
	"CHI",
	"ENG",
};

static const char gSubMenu_SC_REV[3][3] = {
	"TO",
	"CO",
	"SE",
};

static const char gSubMenu_MDF[3][5] = {
	"FREQ",
	"CHAN",
	"NAME",
};

static const char gSubMenu_AL_MOD[2][5] = {
	"SITE",
	"TONE",
};

static const char gSubMenu_D_RSP[4][6] = {
	"NULL",
	"RING",
	"REPLY",
	"BOTH",
};

static const char gSubMenu_PTT_ID[4][5] = {
	"OFF",
	"BOT",
	"EOT",
	"BOTH",
};

static const char gSubMenu_PONMSG[3][5] = {
	"FULL",
	"MSG",
	"VOL",
};

static const char gSubMenu_ROGER[3][6] = {
	"OFF",
	"ROGER",
	"MDC",
};

static const char gSubMenu_RESET[2][4] = {
	"VFO",
	"ALL",
};

static const char gSubMenu_F_LOCK[6][4] = {
	"OFF",
	"FCC",
	"CE",
	"GB",
	"430",
	"438",
};

GUI_DisplayType_t gScreenToDisplay;
uint8_t g_200003C6;
volatile uint8_t gCurrentStep;
GUI_DisplayType_t gRequestDisplayScreen;
uint8_t g_200003BA;
uint8_t g_200003BB;
uint8_t gWasFKeyPressed;

uint8_t gAskForConfirmation;
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
	char String[16];
	char String2[16];
	uint8_t i;

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	if (gEeprom.KEY_LOCK && gKeypadLocked) {
		GUI_PrintString("Long Press #", 0, 127, 1, 8, true);
		GUI_PrintString("To Unlock", 0, 127, 3, 8, true);
		ST7565_BlitFullScreen();
		return;
	}

	for (i = 0; i < 2; i++) {
		uint8_t *pLine0;
		uint8_t *pLine1;
		uint8_t Line;
		uint8_t Channel;
		bool bIsSameVfo;

		if (i == 0) {
			pLine0 = gFrameBuffer[0];
			pLine1 = gFrameBuffer[1];
			Line = 0;
		} else {
			pLine0 = gFrameBuffer[4];
			pLine1 = gFrameBuffer[5];
			Line = 4;
		}

		Channel = gEeprom.TX_CHANNEL;
		bIsSameVfo = !!(Channel == i);

		if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF && g_2000041F == 1) {
			Channel = gEeprom.RX_CHANNEL;
		}

		if (Channel != i) {
			if (g_200003BC || g_200003BD || g_200003BA) {
				if (g_200003BA == 0) {
					if (g_200003BC == 1) {
						if (g_CalloutAndDTMF_State == 2) {
							strcpy(String, "CALL OUT(RSP)");
						} else {
							strcpy(String, "CALL OUT");
						}
					} else if (g_200003BC == 2) {
						if (DTMF_FindContact(gDTMF_Contact0, String2)) {
							sprintf(String, "CALL:%s", String2);
						} else {
							sprintf(String, "CALL:%s", gDTMF_Contact0);
						}
					} else if (g_200003BD == 1) {
						if (g_CalloutAndDTMF_State == 1) {
							strcpy(String, "DTMF TX(SUCC)");
						} else {
							strcpy(String, "DTMF TX");
						}
					}
				} else {
					sprintf(String, ">%s", g_20000D1C);
				}
				GUI_PrintString(String, 2, 127, i * 3, 8, false);

				memset(String, 0, sizeof(String));
				memset(String2, 0, sizeof(String2));

				if (g_200003BA == 0) {
					if (g_200003BC == 1) {
						if (DTMF_FindContact(gDTMF_String, String2)) {
							sprintf(String, ">%s", String2);
						} else {
							sprintf(String, ">%s", gDTMF_String);
						}
					} else if (g_200003BC == 2) {
						if (DTMF_FindContact(gDTMF_Contact1, String2)) {
							sprintf(String, ">%s", String2);
						} else {
							sprintf(String, ">%s", gDTMF_Contact1);
						}
					} else if (g_200003BD == 1) {
						sprintf(String, ">%s", gDTMF_String);
					}
				}
				GUI_PrintString(String, 2, 127, 2 + (i * 3), 8, false);
				continue;
			} else if (bIsSameVfo) {
				memcpy(pLine0 + 2, BITMAP_VFO_Default, sizeof(BITMAP_VFO_Default));
			}
    	} else {
			if (bIsSameVfo) {
				memcpy(pLine0 + 2, BITMAP_VFO_Default, sizeof(BITMAP_VFO_Default));
			} else {
				memcpy(pLine0 + 2, BITMAP_VFO_NotDefault, sizeof(BITMAP_VFO_NotDefault));
			}
		}

		// 0x8EE2
		uint32_t SomeValue = 0;

		if (gCurrentFunction == FUNCTION_TRANSMIT) {
			if (g_20000383 == 2) {
				SomeValue = 2;
			} else {
				Channel = gEeprom.RX_CHANNEL;
				if (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) {
					Channel = gEeprom.TX_CHANNEL;
				}
				if (Channel == i) {
					SomeValue = 1;
					memcpy(pLine0 + 14, BITMAP_TX, sizeof(BITMAP_TX));
				}
			}
		} else {
			SomeValue = 2;
			if ((gCurrentFunction == FUNCTION_4 || gCurrentFunction == FUNCTION_2) && gEeprom.RX_CHANNEL == i) {
				memcpy(pLine0 + 14, BITMAP_RX, sizeof(BITMAP_RX));
			}
		}

		// 0x8F3C
		if (gEeprom.VfoChannel[i] < 200) {
			memcpy(pLine1 + 2, BITMAP_M, sizeof(BITMAP_M));
			if (gNumberOffset == 0 || gEeprom.TX_CHANNEL != i) {
				NUMBER_ToDigits(gEeprom.VfoChannel[i] + 1, String);
			} else {
				memcpy(String + 5, gNumberForPrintf, 3);
			}
			GUI_DisplaySmallDigits(3, String + 5, 10, Line + 1);
		} else if (gEeprom.VfoChannel[i] < 207) {
			char c;

			memcpy(pLine1 + 14, BITMAP_F, sizeof(BITMAP_F));
			c = gEeprom.VfoChannel[i] - 199;
			GUI_DisplaySmallDigits(1, &c, 22, Line + 1);
		} else {
			memcpy(pLine1 + 7, BITMAP_NarrowBand, sizeof(BITMAP_NarrowBand));
			if (gNumberOffset == 0 || gEeprom.TX_CHANNEL != i) {
				NUMBER_ToDigits(gEeprom.VfoChannel[i] - 206, String);
			} else {
				String[6] = gNumberForPrintf[0];
				String[7] = gNumberForPrintf[1];
			}
			GUI_DisplaySmallDigits(2, String + 6, 15, Line + 1);
		}

		// 0x8FEC

		uint8_t g371 = g_20000371[i];
		if (gCurrentFunction == FUNCTION_TRANSMIT && g_20000383 == 2) {
			if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
				Channel = gEeprom.RX_CHANNEL;
			} else {
				Channel = gEeprom.TX_CHANNEL;
			}
			if (Channel == i) {
				g371 = 5;
			}
		}
		if (g371 != 0) {
			uint8_t Width = 10;

			memset(String, 0, sizeof(String));
			switch (g371) {
			case 1:
				strcpy(String, "BUSY");
				Width = 15;
				break;
			case 2:
				strcpy(String, "BAT LOW");
				break;
			case 3:
				strcpy(String, "DISABLE");
				break;
			case 4:
				strcpy(String, "TIMEOUT");
				break;
			case 5:
				strcpy(String, "ALARM");
				break;
			case 6:
				sprintf(String, "VOL HIGH");
				Width = 8;
				break;
			}
			GUI_PrintString(String, 31, 111, i * 4, Width, true);
		} else {
			if (gNumberOffset != 0 && (gEeprom.VfoChannel[i] - 200) < 7 && gEeprom.TX_CHANNEL == i) {
				GUI_DisplayFrequency(gNumberForPrintf, 31, i * 4, true, false);
			} else {
				if (gEeprom.VfoChannel[i] < 200) {
					if (gEeprom.CHANNEL_DISPLAY_MODE == 2 && (gEeprom.VfoInfo[i].Name[0] == 0 || gEeprom.VfoInfo[i].Name[0] == 0xFF)) {
						sprintf(String, "CH-%03d", gEeprom.VfoChannel[i] + 1);
						GUI_PrintString(String, 31, 112, i * 4, 8, true);
					} else {
						switch (gEeprom.CHANNEL_DISPLAY_MODE) {
						case 0:
							if (gCurrentFunction == FUNCTION_TRANSMIT) {
								if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
									Channel = gEeprom.RX_CHANNEL;
								} else {
									Channel = gEeprom.TX_CHANNEL;
								}
								if (Channel == i) {
									NUMBER_ToDigits(gEeprom.VfoInfo[i].pDCS_Reverse->Frequency, String);
								} else {
									NUMBER_ToDigits(gEeprom.VfoInfo[i].pDCS_Current->Frequency, String);
								}
							} else {
								NUMBER_ToDigits(gEeprom.VfoInfo[i].pDCS_Current->Frequency, String);
							}
							GUI_DisplayFrequency(String, 31, i * 4, false, false);
							if (gEeprom.VfoChannel[i] < 200) {
								const uint8_t Params = gMR_ChannelParameters[gEeprom.VfoChannel[i]];
								if (Params & MR_CH_SCANLIST1) {
									memcpy(pLine0 + 113, BITMAP_ScanList, sizeof(BITMAP_ScanList));
								}
								if (Params & MR_CH_SCANLIST2) {
									memcpy(pLine0 + 120, BITMAP_ScanList, sizeof(BITMAP_ScanList));
								}
							}
							GUI_DisplaySmallDigits(2, String + 6, 112, Line + 1);
							break;
						case 1:
							sprintf(String, "CH-%03d", gEeprom.VfoChannel[i] + 1);
							GUI_PrintString(String, 31, 112, i * 4, 8, true);
							break;
						case 2:
							GUI_PrintString(gEeprom.VfoInfo[i].Name, 31, 112, i * 4, 8, true);
							break;
						}
					}
				} else {
					if (gCurrentFunction == FUNCTION_TRANSMIT) {
						if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
							Channel = gEeprom.RX_CHANNEL;
						} else {
							Channel = gEeprom.TX_CHANNEL;
						}
						if (Channel == i) {
							NUMBER_ToDigits(gEeprom.VfoInfo[i].pDCS_Reverse->Frequency, String);
						} else {
							NUMBER_ToDigits(gEeprom.VfoInfo[i].pDCS_Current->Frequency, String);
						}
					} else {
						NUMBER_ToDigits(gEeprom.VfoInfo[i].pDCS_Current->Frequency, String);
					}
					GUI_DisplayFrequency(String, 31, i * 4, false, false);
					if (gEeprom.VfoChannel[i] < 200) {
						const uint8_t Params = gMR_ChannelParameters[gEeprom.VfoChannel[i]];
						if (Params & MR_CH_SCANLIST1) {
							memcpy(pLine0 + 113, BITMAP_ScanList, sizeof(BITMAP_ScanList));
						}
						if (Params & MR_CH_SCANLIST2) {
							memcpy(pLine0 + 120, BITMAP_ScanList, sizeof(BITMAP_ScanList));
						}
					}
					GUI_DisplaySmallDigits(2, String + 6, 112, Line + 1);
				}
			}
		}

		// 0x926E
		uint8_t Level = 0;

		if (SomeValue == 1) {
				if (gInfoCHAN_A->OUTPUT_POWER == OUTPUT_POWER_LOW) {
					Level = 2;
				} else if (gInfoCHAN_A->OUTPUT_POWER == OUTPUT_POWER_MID) {
					Level = 4;
				} else {
					Level = 6;
				}		
		} else if (SomeValue == 2) {
			if (gVFO_RSSI_Level[i]) {
				Level = gVFO_RSSI_Level[i];
			}
		}

		// TODO: not quite how the original does it, but it's quite entangled in Ghidra.
		if (Level) {
			memcpy(pLine1 + 128 + 0, BITMAP_Antenna, sizeof(BITMAP_Antenna));
			memcpy(pLine1 + 128 + 5, BITMAP_AntennaLevel1, sizeof(BITMAP_AntennaLevel1));
			if (Level >= 2) {
				memcpy(pLine1 + 128 + 8, BITMAP_AntennaLevel2, sizeof(BITMAP_AntennaLevel2));
			}
			if (Level >= 3) {
				memcpy(pLine1 + 128 + 11, BITMAP_AntennaLevel3, sizeof(BITMAP_AntennaLevel3));
			}
			if (Level >= 4) {
				memcpy(pLine1 + 128 + 14, BITMAP_AntennaLevel4, sizeof(BITMAP_AntennaLevel4));
			}
			if (Level >= 5) {
				memcpy(pLine1 + 128 + 17, BITMAP_AntennaLevel5, sizeof(BITMAP_AntennaLevel5));
			}
			if (Level >= 6) {
				memcpy(pLine1 + 128 + 20, BITMAP_AntennaLevel6, sizeof(BITMAP_AntennaLevel6));
			}
		}

		// 0x931E
		if (gEeprom.VfoInfo[i].IsAM == true) {
			memcpy(pLine1 + 128 + 27, BITMAP_AM, sizeof(BITMAP_AM));
		} else {
			const DCS_Info_t *pDCS;

			if (SomeValue == 1) {
				pDCS = gEeprom.VfoInfo[i].pDCS_Reverse;
			} else {
				pDCS = gEeprom.VfoInfo[i].pDCS_Current;
			}
			switch (pDCS->CodeType) {
			case CODE_TYPE_CONTINUOUS_TONE:
				memcpy(pLine1 + 128 + 27, BITMAP_CT, sizeof(BITMAP_CT));
				break;
			case CODE_TYPE_DIGITAL:
			case CODE_TYPE_REVERSE_DIGITAL:
				memcpy(pLine1 + 128 + 24, BITMAP_DCS, sizeof(BITMAP_DCS));
				break;
			default:
				break;
			}
		}

		// 0x936C
		switch (gEeprom.VfoInfo[i].OUTPUT_POWER) {
		case OUTPUT_POWER_LOW:
			memcpy(pLine1 + 128 + 44, BITMAP_PowerLow, sizeof(BITMAP_PowerLow));
			break;
		case OUTPUT_POWER_MID:
			memcpy(pLine1 + 128 + 44, BITMAP_PowerMid, sizeof(BITMAP_PowerMid));
			break;
		case OUTPUT_POWER_HIGH:
			memcpy(pLine1 + 128 + 44, BITMAP_PowerHigh, sizeof(BITMAP_PowerHigh));
			break;
		}

		if (gEeprom.VfoInfo[i].DCS[0].Frequency != gEeprom.VfoInfo[i].DCS[1].Frequency) {
			if (gEeprom.VfoInfo[i].FREQUENCY_DEVIATION_SETTING == FREQUENCY_DEVIATION_ADD) {
				memcpy(pLine1 + 128 + 54, BITMAP_Add, sizeof(BITMAP_Add));
			}
			if (gEeprom.VfoInfo[i].FREQUENCY_DEVIATION_SETTING == FREQUENCY_DEVIATION_ADD) {
				memcpy(pLine1 + 128 + 54, BITMAP_Sub, sizeof(BITMAP_Sub));
			}

		}

		if (gEeprom.VfoInfo[i].FrequencyReverse) {
			memcpy(pLine1 + 128 + 64, BITMAP_ReverseMode, sizeof(BITMAP_ReverseMode));
		}
		if (gEeprom.VfoInfo[i].CHANNEL_BANDWIDTH == BANDWIDTH_NARROW) {
			memcpy(pLine1 + 128 + 74, BITMAP_NarrowBand, sizeof(BITMAP_NarrowBand));
		}
		if (gEeprom.VfoInfo[i].DTMF_DECODING_ENABLE || gSetting_KILLED) {
			memcpy(pLine1 + 128 + 84, BITMAP_DTMF, sizeof(BITMAP_DTMF));
		}
		if (gEeprom.VfoInfo[i].SCRAMBLING_TYPE && gSetting_ScrambleEnable) {
			memcpy(pLine1 + 128 + 110, BITMAP_Scramble, sizeof(BITMAP_Scramble));
		}
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
			if (gIs_A_Scan == false) {
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

static void DisplayMenu(void)
{
	char String[16];
	char Contact[8];
	uint8_t i;

	for (i = 0; i < 3; i++) {
		if (gMenuCursor && i) {
			if ((gMenuListCount - 1) != gMenuCursor || (i != 2)) {
				GUI_PrintString(MenuList[i], 0, 127, i * 2, 8, false);
			}
		}
	}
	for (i = 0; i < 48; i++) {
		gFrameBuffer[2][i] ^= 0xFF;
		gFrameBuffer[3][i] ^= 0xFF;
	}
	for (i = 0; i < 7; i++) {
		gFrameBuffer[i][48] = 0xFF;
		gFrameBuffer[i][49] = 0xFF;
	}
	NUMBER_ToDigits(gMenuCursor + 1, String);
	GUI_DisplaySmallDigits(2, String + 6, 33, 6);
	if (g_200003C6) {
		memcpy(gFrameBuffer[0] + 50, BITMAP_CurrentIndicator, sizeof(BITMAP_CurrentIndicator));
	}

	memset(String, 0, sizeof(String));

	switch (gMenuCursor) {
	case MENU_SQL:
	case MENU_MIC:
		sprintf(String, "%d", gSubMenuSelection);
		break;

	case MENU_STEP:
		sprintf(String, "%.2fKHz", gSubMenu_Step[gSubMenuSelection] * 0.01);
		break;

	case MENU_TXP:
		strcpy(String, gSubMenu_TXP[gSubMenuSelection]);
		break;

	case MENU_R_DCS:
	case MENU_T_DCS:
		if (gSubMenuSelection == 0) {
			strcpy(String, "OFF");
		} else if (gSubMenuSelection < 105) {
			sprintf(String, "D%03oN", DCS_Options[gSubMenuSelection - 1]);
		} else {
			sprintf(String, "D%03oI", DCS_Options[gSubMenuSelection - 105]);
		}
		break;

	case MENU_R_CTCS:
	case MENU_T_CTCS:
		if (gSubMenuSelection == 0) {
			strcpy(String, "OFF");
		} else {
			sprintf(String, "%.1fHz", CTCSS_Options[gSubMenuSelection] * 0.1);
		}
		break;

	case MENU_SFT_D:
		strcpy(String, gSubMenu_SFT_D[gSubMenuSelection]);
		break;

	case MENU_OFFSET:
		if (g_200003C6 != 1 || gNumberOffset == 0) {
			sprintf(String, "%.5f", gSubMenuSelection * 1e-05);
			break;
		}
		for (i = 0; i < 3; i++) {
			if (gNumberForPrintf[i] == 10) {
				String[i] = '-';
			} else {
				String[i] = gNumberForPrintf[i] + '0';
			}
		}
		String[3] = '.';
		for (i = 3; i < 6; i++) {
			if (gNumberForPrintf[i] == 10) {
				String[i + 1] = '-';
			} else {
				String[i + 1] = gNumberForPrintf[i] + '0';
			}
		}
		String[7] = 0x2d;
		String[8] = '-';
		String[9] = 0;
		String[10] = 0;
		String[11] = 0;
		break;

	case MENU_W_N:
		strcpy(String, gSubMenu_W_N[gSubMenuSelection]);
		break;

	case MENU_SCR:
	case MENU_VOX:
	case MENU_ABR:
		if (gSubMenuSelection == 0) {
			strcpy(String, "OFF");
		} else {
			sprintf(String, "%d", gSubMenuSelection);
		}
		break;

	case MENU_BCL:
	case MENU_BEEP:
	case MENU_AUTOLK:
	case MENU_S_ADD1:
	case MENU_S_ADD2:
	case MENU_STE:
	case MENU_D_ST:
	case MENU_D_DCD:
	case MENU_AM:
	case MENU_NOAA_S:
	case MENU_350TX:
	case MENU_200TX:
	case MENU_500TX:
	case MENU_350EN:
	case MENU_SCREN:
		strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
		break;

	case MENU_MEM_CH:
	case MENU_1_CALL:
	case MENU_DEL_CH:
		GenerateChannelStringEx(
			String,
			RADIO_CheckValidChannel((uint16_t)gSubMenuSelection, false, 0),
			(uint8_t)gSubMenuSelection
			);
		break;

	case MENU_SAVE:
		strcpy(String, gSubMenu_SAVE[gSubMenuSelection]);
		break;

	case MENU_TDR:
	case MENU_WX:
		strcpy(String, gSubMenu_CHAN[gSubMenuSelection]);
		break;

	case MENU_TOT:
		if (gSubMenuSelection == 0) {
			strcpy(String, "OFF");
		} else {
			sprintf(String, "%dmin", gSubMenuSelection);
		}
		break;

	case MENU_VOICE:
		strcpy(String, gSubMenu_VOICE[gSubMenuSelection]);
		break;

	case MENU_SC_REV:
		strcpy(String, gSubMenu_SC_REV[gSubMenuSelection]);
		break;

	case MENU_MDF:
		strcpy(String, gSubMenu_MDF[gSubMenuSelection]);
		break;

	case MENU_RP_STE:
		if (gSubMenuSelection == 0) {
			strcpy(String, "OFF");
		} else {
			sprintf(String, "%d*100ms", gSubMenuSelection);
		}
		break;

	case MENU_S_LIST:
		sprintf(String, "LIST%d", gSubMenuSelection);
		break;

	case MENU_AL_MOD:
		sprintf(String, gSubMenu_AL_MOD[gSubMenuSelection]);
		break;

	case MENU_ANI_ID:
		strcpy(String, gEeprom.ANI_DTMF_ID);
		break;

	case MENU_UPCODE:
		strcpy(String, gEeprom.DTMF_UP_CODE);
		break;

	case MENU_DWCODE:
		strcpy(String, gEeprom.DTMF_DOWN_CODE);
		break;

	case MENU_D_RSP:
		strcpy(String, gSubMenu_D_RSP[gSubMenuSelection]);
		break;

	case MENU_D_HOLD:
		sprintf(String, "%ds", gSubMenuSelection);
		break;

	case MENU_D_PRE:
		sprintf(String, "%d*10ms", gSubMenuSelection);
		break;

	case MENU_PTT_ID:
		strcpy(String, gSubMenu_PTT_ID[gSubMenuSelection]);
		break;

	case MENU_D_LIST:
		gIsDtmfContactValid = DTMF_GetContact((uint8_t)gSubMenuSelection - 1, Contact);
		if (!gIsDtmfContactValid) {
			strcpy(String, "NULL");
		} else {
			strcpy(String, Contact);
		}
		break;

	case MENU_PONMSG:
		strcpy(String, gSubMenu_PONMSG[gSubMenuSelection]);
		break;

	case MENU_ROGER:
		strcpy(String, gSubMenu_ROGER[gSubMenuSelection]);
		break;

	case MENU_VOL:
		sprintf(String, "%.2fV", gBatteryVoltageAverage * 0.01);
		break;

	case MENU_RESET:
		strcpy(String, gSubMenu_RESET[gSubMenuSelection]);
		break;

	case MENU_F_LOCK:
		strcpy(String, gSubMenu_F_LOCK[gSubMenuSelection]);
		break;
	}

	GUI_PrintString(String, 50, 127, 2, 8, true);

	if (gMenuCursor == MENU_OFFSET) {
		GUI_PrintString("MHz", 50, 127, 4, 8, true);
	}

	if ((gMenuCursor == MENU_RESET || gMenuCursor == MENU_MEM_CH || gMenuCursor == MENU_DEL_CH) && gAskForConfirmation) {
		if (gAskForConfirmation == 1) {
			strcpy(String, "SURE?");
		} else {
			strcpy(String, "WAIT!");
		}
		GUI_PrintString(String, 50, 127, 4, 8, true);
	}

	if ((gMenuCursor == MENU_R_CTCS || gMenuCursor == MENU_R_DCS) && g_20000381 != 0) {
		GUI_PrintString("SCAN", 50, 127, 4, 8, true);
	}

	if (gMenuCursor == MENU_UPCODE) {
		if (strlen(gEeprom.DTMF_UP_CODE) > 8) {
			GUI_PrintString(gEeprom.DTMF_UP_CODE + 8, 50, 127, 4, 8, true);
		}
	}
	if (gMenuCursor == MENU_DWCODE) {
		if (strlen(gEeprom.DTMF_DOWN_CODE) > 8) {
			GUI_PrintString(gEeprom.DTMF_DOWN_CODE + 8, 50, 127, 4, 8, true);
		}
	}
	if (gMenuCursor == MENU_D_LIST && gIsDtmfContactValid) {
		memcpy(&gDTMF_ID, Contact, 4);
		sprintf(String,"ID:%s", Contact);
		GUI_PrintString(String, 50, 127, 4, 8, true);
	}

	if (gMenuCursor == MENU_R_CTCS || gMenuCursor == MENU_T_CTCS ||
		gMenuCursor == MENU_R_DCS || gMenuCursor == MENU_T_DCS || gMenuCursor == MENU_D_LIST) {
			uint8_t Offset;

			NUMBER_ToDigits((uint8_t)gSubMenuSelection, String);
			Offset = (gMenuCursor == MENU_D_LIST) ? 2 : 3;
			GUI_DisplaySmallDigits(Offset, String + (8 - Offset), 105, 0);
	}

	if (gMenuCursor != MENU_SLIST1 && gMenuCursor != MENU_SLIST2) {
		goto Skip;
	}

	i = gMenuCursor - MENU_SLIST1;

	if (gSubMenuSelection == 0xFF) {
		sprintf(String,"NULL");
	} else {
		GenerateChannelStringEx(String, true, (uint8_t)gSubMenuSelection);
	}

	if (gSubMenuSelection == 0xFF || gEeprom.SCAN_LIST_ENABLED[i] != true) {
		GUI_PrintString(String, 50, 127, 2, 8, 1);
	} else {
		GUI_PrintString(String, 50, 127, 0, 8, 1);
		if (gEeprom.SCANLIST_PRIORITY_CH1[i] < 200) {
			sprintf(String, "PRI1:%d", gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
			GUI_PrintString(String, 50, 127, 2, 8, 1);
		}
		if (gEeprom.SCANLIST_PRIORITY_CH2[i] < 200) {
			sprintf(String, "PRI2:%d", gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
			GUI_PrintString(String, 50, 127, 4, 8, 1);
		}
	}

Skip:
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
	} else if (g_CxCSS_Type == 1) {
		sprintf(String, "CTC:%.1fHz", CTCSS_Options[g_CxCSS_Index] * 0.1);
	} else {
		sprintf(String, "DCS:D%03oN", DCS_Options[g_CxCSS_Index]);
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
		DisplayMenu();
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

