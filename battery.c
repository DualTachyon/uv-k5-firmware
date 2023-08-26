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

#include "battery.h"
#include "driver/backlight.h"
#include "gui.h"
#include "misc.h"

uint16_t gBatteryCalibration[6];
uint16_t gBatteryCurrentVoltage;
uint16_t gBatteryCurrent;
uint16_t gBatteryVoltages[4];
uint16_t gBatteryVoltageAverage;

uint8_t gBatteryDisplayLevel;

bool gChargingWithTypeC;
bool gLowBattery;
bool gLowBatteryBlink;

volatile uint16_t gBatterySave;

void BATTERY_GetReadings(bool bDisplayBatteryLevel)
{
	uint16_t Voltage;
	uint8_t PreviousBatteryLevel;

	PreviousBatteryLevel = gBatteryDisplayLevel;

	Voltage = (gBatteryVoltages[0] + gBatteryVoltages[1] + gBatteryVoltages[2] + gBatteryVoltages[3]) / 4;

	if (gBatteryCalibration[5] < Voltage) {
		gBatteryDisplayLevel = 6;
	} else if (gBatteryCalibration[4] < Voltage) {
		gBatteryDisplayLevel = 5;
	} else if (gBatteryCalibration[3] < Voltage) {
		gBatteryDisplayLevel = 4;
	} else if (gBatteryCalibration[2] < Voltage) {
		gBatteryDisplayLevel = 3;
	} else if (gBatteryCalibration[1] < Voltage) {
		gBatteryDisplayLevel = 2;
	} else if (gBatteryCalibration[0] < Voltage) {
		gBatteryDisplayLevel = 1;
	} else {
		gBatteryDisplayLevel = 0;
	}

	gBatteryVoltageAverage = (Voltage * 760) / gBatteryCalibration[3];

	if ((gScreenToDisplay == DISPLAY_MENU) && gMenuCursor == MENU_VOL) {
		g_20000370 = 1;
	}
	if (gBatteryCurrent < 501) {
		if (gChargingWithTypeC) {
			g_2000036F = 1;
		}
		gChargingWithTypeC = 0;
	} else {
		if (gChargingWithTypeC == false) {
			g_2000036F = 1;
			BACKLIGHT_TurnOn();
		}
		gChargingWithTypeC = 1;
	}

	if (PreviousBatteryLevel != gBatteryDisplayLevel) {
		if (gBatteryDisplayLevel < 2) {
			gLowBattery = true;
		} else {
			gLowBattery = false;
			if (bDisplayBatteryLevel) {
				GUI_DisplayBatteryLevel(gBatteryDisplayLevel);
			}
		}
		g_20000400 = 0;
	}
}

