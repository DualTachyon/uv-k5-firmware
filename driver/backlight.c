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

#include "backlight.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "settings.h"

uint8_t gBacklightCountdown;

void BACKLIGHT_TurnOn(void)
{
	if (gEeprom.BACKLIGHT) {
		GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
		gBacklightCountdown = 1 + (gEeprom.BACKLIGHT * 2);
	}
}

