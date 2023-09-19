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
#include "app/dtmf.h"
#include "bitmaps.h"
#include "dcs.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/menu.h"
#include "ui/ui.h"

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
	"SLIST2",
#if defined(ENABLE_ALARM)
	          "AL-MOD",
#endif
		             "ANI-ID", "UPCODE",
	"DWCODE", "D-ST",    "D-RSP",  "D-HOLD",
	// 0x28
	"D-PRE",  "PTT-ID",  "D-DCD",  "D-LIST",
	"PONMSG", "ROGER",   "VOL",    "AM",
	// 0x30
#if defined(ENABLE_NOAA)
	"NOAA_S",
#endif
	          "DEL-CH",  "RESET",  "350TX",
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

#if defined(ENABLE_ALARM)
static const char gSubMenu_AL_MOD[2][5] = {
	"SITE",
	"TONE",
};
#endif

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

bool gIsInSubMenu;

uint8_t gMenuCursor;
int8_t gMenuScrollDirection;
uint32_t gSubMenuSelection;

//Pixel where we split the screen: left is menu selection, right is menu item content
#define SPLIT   (50)

void UI_DisplayMenu(void)
{
	char String[16];
	char Contact[16];
	uint8_t i;

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));

        /* Print the 3 visible menu items */
	for (i = 0; i < 3; i++) {
		if (gMenuCursor || i) {
			if ((gMenuListCount - 1) != gMenuCursor || (i != 2)) {
				UI_PrintString(MenuList[gMenuCursor + i - 1], 0, SPLIT - 2, i * 2, 8, false);
			}
		}
	}

        /* Invert for the select item */
	for (i = 0; i < SPLIT - 2; i++) {
		gFrameBuffer[2][i] ^= 0xFF;
		gFrameBuffer[3][i] ^= 0xFF;
	}

	/* Create a vertical line line, 2 pixels wide */
	for (i = 0; i < ARRAY_SIZE(gFrameBuffer); i++) {
		gFrameBuffer[i][SPLIT - 2] = 0xFF;
		gFrameBuffer[i][SPLIT - 1] = 0xFF;
	}

        /* Print the number of the current menu item, 2 digits 1 pixel from split */
	NUMBER_ToDigits(gMenuCursor + 1, String);
	UI_DisplaySmallDigits(2, String + 6, SPLIT - (2 * 8) - 1, 6);

        /* Display submenu arrow */
	if (gIsInSubMenu) {
		memcpy(gFrameBuffer[0] + SPLIT, BITMAP_CurrentIndicator, sizeof(BITMAP_CurrentIndicator));
	}

	memset(String, 0, sizeof(String));

	/* Get menu item content as string */
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
			sprintf(String, "%.1fHz", CTCSS_Options[gSubMenuSelection - 1] * 0.1);
		}
		break;

	case MENU_SFT_D:
		strcpy(String, gSubMenu_SFT_D[gSubMenuSelection]);
		break;

	case MENU_OFFSET:
		if (!gIsInSubMenu || gInputBoxIndex == 0) {
			sprintf(String, "%.5f", gSubMenuSelection * 1e-05);
			break;
		}
		for (i = 0; i < 3; i++) {
			if (gInputBox[i] == 10) {
				String[i] = '-';
			} else {
				String[i] = gInputBox[i] + '0';
			}
		}
		String[3] = '.';
		for (i = 3; i < 6; i++) {
			if (gInputBox[i] == 10) {
				String[i + 1] = '-';
			} else {
				String[i + 1] = gInputBox[i] + '0';
			}
		}
		String[7] = '-';
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
#if defined(ENABLE_NOAA)
	case MENU_NOAA_S:
#endif
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
		UI_GenerateChannelStringEx(
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

#if defined(ENABLE_ALARM)
	case MENU_AL_MOD:
		sprintf(String, gSubMenu_AL_MOD[gSubMenuSelection]);
		break;
#endif

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
			// Ghidra being weird again...
			memcpy(String, "NULL\0\0\0", 8);
		} else {
			memcpy(String, Contact, 8);
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

	UI_PrintString(String, 50, 127, 2, 8, true);

	if (gMenuCursor == MENU_OFFSET) {
		UI_PrintString("MHz", 50, 127, 4, 8, true);
	}

	if ((gMenuCursor == MENU_RESET || gMenuCursor == MENU_MEM_CH || gMenuCursor == MENU_DEL_CH) && gAskForConfirmation) {
		if (gAskForConfirmation == 1) {
			strcpy(String, "SURE?");
		} else {
			strcpy(String, "WAIT!");
		}
		UI_PrintString(String, 50, 127, 4, 8, true);
	}

	if ((gMenuCursor == MENU_R_CTCS || gMenuCursor == MENU_R_DCS) && gCssScanMode != CSS_SCAN_MODE_OFF) {
		UI_PrintString("SCAN", 50, 127, 4, 8, true);
	}

	if (gMenuCursor == MENU_UPCODE) {
		if (strlen(gEeprom.DTMF_UP_CODE) > 8) {
			UI_PrintString(gEeprom.DTMF_UP_CODE + 8, 50, 127, 4, 8, true);
		}
	}
	if (gMenuCursor == MENU_DWCODE) {
		if (strlen(gEeprom.DTMF_DOWN_CODE) > 8) {
			UI_PrintString(gEeprom.DTMF_DOWN_CODE + 8, 50, 127, 4, 8, true);
		}
	}
	if (gMenuCursor == MENU_D_LIST && gIsDtmfContactValid) {
		Contact[11] = 0;
		memcpy(&gDTMF_ID, Contact + 8, 4);
		sprintf(String, "ID:%s", Contact + 8);
		UI_PrintString(String, 50, 127, 4, 8, true);
	}

	if (gMenuCursor == MENU_R_CTCS || gMenuCursor == MENU_T_CTCS ||
		gMenuCursor == MENU_R_DCS || gMenuCursor == MENU_T_DCS || gMenuCursor == MENU_D_LIST) {
			uint8_t Offset;

			NUMBER_ToDigits((uint8_t)gSubMenuSelection, String);
			Offset = (gMenuCursor == MENU_D_LIST) ? 2 : 3;
			UI_DisplaySmallDigits(Offset, String + (8 - Offset), 105, 0);
	}

	if (gMenuCursor == MENU_SLIST1 || gMenuCursor == MENU_SLIST2) {
		i = gMenuCursor - MENU_SLIST1;

		if (gSubMenuSelection == 0xFF) {
			sprintf(String, "NULL");
		} else {
			UI_GenerateChannelStringEx(String, true, (uint8_t)gSubMenuSelection);
		}

		if (gSubMenuSelection == 0xFF || !gEeprom.SCAN_LIST_ENABLED[i]) {
			UI_PrintString(String, 50, 127, 2, 8, 1);
		} else {
			UI_PrintString(String, 50, 127, 0, 8, 1);
			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH1[i])) {
				sprintf(String, "PRI1:%d", gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
				UI_PrintString(String, 50, 127, 2, 8, 1);
			}
			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH2[i])) {
				sprintf(String, "PRI2:%d", gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
				UI_PrintString(String, 50, 127, 4, 8, 1);
			}
		}
	}

	ST7565_BlitFullScreen();
}

