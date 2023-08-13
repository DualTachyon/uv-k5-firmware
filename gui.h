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

extern GUI_DisplayType_t gScreenToDisplay;
extern uint8_t g_200003C6;
extern uint8_t g_20000380;
extern uint8_t g_20000390;
extern uint8_t g_200003BA;
extern uint8_t g_200003BB;
extern uint8_t g_20000367;

extern bool gAskForConfirmation;
extern bool gAskToSave;
extern bool gAskToDelete;

void GUI_Welcome(void);
void GUI_PrintString(const char *pString, uint8_t Start, uint8_t End, uint8_t Line, int Width, bool bCentered);
void GUI_PasswordScreen(void);
void GUI_LockScreen(void);

void GUI_SelectNextDisplay(GUI_DisplayType_t Display);

#endif

