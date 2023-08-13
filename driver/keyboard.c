/* Copyright 2023 Manuel Jinger
 * Copyright 2023 Dual Tachyon
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

#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "driver/systick.h"

KEY_Code_t KEYBOARD_Poll(void)
{
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_4);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_5);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);

	SYSTICK_DelayUs(1);

	// Keys connected to gnd
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_0)) {
		return KEY_SIDE1;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_1)) {
		return KEY_SIDE2;
	}
	// Original doesn't do PTT

	// First row
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_4);
	SYSTICK_DelayUs(1);

	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_0)) {
		return KEY_MENU;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_1)) {
		return KEY_1;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_2)) {
		return KEY_4;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_3)) {
		return KEY_7;
	}

	// Second row
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_5);
	SYSTICK_DelayUs(1);

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_4);
	SYSTICK_DelayUs(1);

	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_0)) {
		return KEY_UP;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_1)) {
		return KEY_2;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_2)) {
		return KEY_5;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_3)) {
		return KEY_8;
	}

	// Third row
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_4);
	SYSTICK_DelayUs(1);

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_5);
	SYSTICK_DelayUs(1);

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_4);
	SYSTICK_DelayUs(1);

	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
	SYSTICK_DelayUs(1);

	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_0)) {
		return KEY_DOWN;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_1)) {
		return KEY_3;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_2)) {
		return KEY_6;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_3)) {
		return KEY_9;
	}

	// Fourth row
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);
	SYSTICK_DelayUs(1);

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
	SYSTICK_DelayUs(1);

	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_0)) {
		return KEY_EXIT;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_1)) {
		return KEY_STAR;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_2)) {
		return KEY_0;
	}
	if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_3)) {
		return KEY_F;
	}

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_4);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_5);
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);

	return KEY_INVALID;
}

