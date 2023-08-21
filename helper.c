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

#include "aircopy.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk4819.h"
#include "driver/keyboard.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "gui.h"
#include "helper.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

KEY_Code_t gKeyReading0;
KEY_Code_t gKeyReading1;
uint8_t g_2000042A;

uint8_t HELPER_GetKey(void)
{
	KEY_Code_t Keys[2];
	uint8_t i;
	uint8_t ret;

	ret = 0;
	for (i = 0; i < 2; i++) {
		if (GPIO_CheckBit(&GPIOC->DATA, 5)) {
			return 0;
		}
		Keys[i] = KEYBOARD_Poll();
		SYSTEM_DelayMs(20);
	}
	if (Keys[0] == Keys[1]) {
		gKeyReading0 = Keys[0];
		gKeyReading1 = Keys[0];
		g_2000042A = 2;
		if (i == KEY_SIDE1) {
			ret = 1;
		} else if (i == KEY_SIDE2) {
			ret = 2;
		}
	}

	return ret;
}

void HELPER_CheckBootKey(uint8_t KeyType)
{
	if (KeyType == 1) {
		gMenuCursor = MENU_350TX;
		gSubMenuSelection = gSetting_350TX;
		GUI_SelectNextDisplay(DISPLAY_MENU);
		gMenuListCount = 57;
		gF_LOCK = 1;
	} else if (KeyType == 2) {
		gEeprom.DUAL_WATCH = DUAL_WATCH_OFF;
		gEeprom.BATTERY_SAVE = 0;
		gEeprom.VOX_SWITCH = false;
		gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
		gEeprom.AUTO_KEYPAD_LOCK = false;
		gEeprom.KEY_1_SHORT_PRESS_ACTION = 0;
		gEeprom.KEY_1_LONG_PRESS_ACTION = 0;
		gEeprom.KEY_2_SHORT_PRESS_ACTION = 0;
		gEeprom.KEY_2_LONG_PRESS_ACTION = 0;

		RADIO_InitInfo(gInfoCHAN_A, 205, 5, 41002500);
		gInfoCHAN_A->CHANNEL_BANDWIDTH = BANDWIDTH_NARROW;
		gInfoCHAN_A->OUTPUT_POWER = 0;
		RADIO_ConfigureSquelchAndOutputPower(gInfoCHAN_A);
		gCrossTxRadioInfo = gInfoCHAN_A;
		RADIO_SetupRegisters(true);
		BK4819_SetupAircopy();
		BK4819_ResetFSK();
		gAircopyState = AIRCOPY_READY;
		GUI_SelectNextDisplay(DISPLAY_AIRCOPY);
	} else {
		GUI_SelectNextDisplay(DISPLAY_MAIN);
	}
}

