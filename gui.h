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

#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include <stdint.h>

enum GUI_DisplayType_t {
	DISPLAY_MAIN	= 0x00U,
	DISPLAY_FM	= 0x01U,
	DISPLAY_MENU	= 0x02U,
	DISPLAY_SCANNER	= 0x03U,
	DISPLAY_AIRCOPY	= 0x04U,
	DISPLAY_INVALID	= 0xFFU,
};

typedef enum GUI_DisplayType_t GUI_DisplayType_t;

enum {
	MENU_SQL		= 0,
	MENU_STEP		= 1,
	MENU_TXP		= 2,
	MENU_R_DCS		= 3,
	MENU_R_CTCS		= 4,
	MENU_T_DCS		= 5,
	MENU_T_CTCS		= 6,
	MENU_SFT_D		= 7,
	MENU_OFFSET		= 8,
	MENU_W_N		= 9,
	MENU_SCR		= 10,
	MENU_BCL		= 11,
	MENU_MEM_CH		= 12,
	MENU_SAVE		= 13,
	MENU_VOX		= 14,
	MENU_ABR		= 15,
	MENU_TDR		= 16,
	MENU_WX			= 17,
	MENU_BEEP		= 18,
	MENU_TOT		= 19,
	MENU_VOICE		= 20,
	MENU_SC_REV		= 21,
	MENU_MDF		= 22,
	MENU_AUTOLK		= 23,
	MENU_S_ADD1		= 24,
	MENU_S_ADD2		= 25,
	MENU_STE		= 26,
	MENU_RP_STE		= 27,
	MENU_MIC		= 28,
	MENU_1_CALL		= 29,
	MENU_S_LIST		= 30,
	MENU_SLIST1		= 31,
	MENU_SLIST2		= 32,
	MENU_AL_MOD		= 33,
	MENU_ANI_ID		= 34,
	MENU_UPCODE		= 35,
	MENU_DWCODE		= 36,
	MENU_D_ST		= 37,
	MENU_D_RSP		= 38,
	MENU_D_HOLD		= 39,
	MENU_D_PRE		= 40,
	MENU_PTT_ID		= 41,
	MENU_D_DCD		= 42,
	MENU_D_LIST		= 43,
	MENU_PONMSG		= 44,
	MENU_ROGER		= 45,
	MENU_VOL		= 46,
	MENU_AM			= 47,
	MENU_NOAA_S		= 48,
	MENU_DEL_CH		= 49,
	MENU_RESET		= 50,
	MENU_350TX		= 51,
	MENU_F_LOCK		= 52,
	MENU_200TX		= 53,
	MENU_500TX		= 54,
	MENU_350EN		= 55,
	MENU_SCREN		= 56,
};

extern GUI_DisplayType_t gScreenToDisplay;
extern bool gIsInSubMenu;
extern volatile int8_t gStepDirection;
extern GUI_DisplayType_t gRequestDisplayScreen;
extern uint8_t g_200003BA;
extern uint8_t g_200003BB;
extern bool gWasFKeyPressed;

extern uint8_t gAskForConfirmation;
extern bool gAskToSave;
extern bool gAskToDelete;

extern uint8_t gMenuCursor;
extern int8_t gMenuScrollDirection;
extern uint32_t gSubMenuSelection;

void GUI_DisplayBatteryLevel(uint8_t BatteryLevel);
void GUI_Welcome(void);
void GUI_PrintString(const char *pString, uint8_t Start, uint8_t End, uint8_t Line, int Width, bool bCentered);
void GUI_PasswordScreen(void);
void GUI_LockScreen(void);
void GUI_DisplayStatusLine(void);

void GUI_DisplayFrequency(const char *pDigits, uint8_t X, uint8_t Y, bool bDisplayLeadingZero, bool Flag1);
void GUI_DisplaySmallDigits(uint8_t Size, const char *pString, uint8_t x, uint8_t y);

void GUI_DisplayMenu(void);
void GUI_DisplayScreen(void);
void GUI_SelectNextDisplay(GUI_DisplayType_t Display);

void GUI_RenderRSSI(uint8_t RssiLevel, uint8_t VFO);
void GUI_DisplayRSSI(uint16_t RSSI);

#endif

