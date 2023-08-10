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

#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include <stdint.h>

enum GPIOA_PINS {
    GPIOA_PIN_KEYBOARD_ROW1 = 3,
    GPIOA_PIN_KEYBOARD_ROW2 = 4,
    GPIOA_PIN_KEYBOARD_ROW3 = 5,
    GPIOA_PIN_KEYBOARD_ROW4 = 6,
    GPIOA_PIN_KEYBOARD_LINE1 = 10,
    GPIOA_PIN_I2C_SCL = 10,
    GPIOA_PIN_KEYBOARD_LINE2 = 11,
    GPIOA_PIN_I2C_SDA = 11,
    GPIOA_PIN_KEYBOARD_LINE3 = 12,
    GPIOA_PIN_KEYBOARD_LINE4 = 13,
};

enum GPIOB_PINS {
	GPIOB_PIN_FLASHLIGHT = 3,
	GPIOB_PIN_BACKLIGHT = 6,
	GPIOB_PIN_ST7565_0 = 9,
	GPIOB_PIN_ST7565_1 = 11,
	GPIOB_PIN_BK1080 = 15,
};

enum GPIOC_PINS {
	GPIOC_PIN_BK4819_0 = 0,
	GPIOC_PIN_BK4819_1 = 1,
	GPIOC_PIN_BK4819_2 = 2,
	GPIOC_PIN_KEYBOARD_PPT = 5,
};

void GPIO_ClearBit(volatile uint32_t *pReg, uint8_t Bit);
uint8_t GPIO_CheckBit(volatile uint32_t *pReg, uint8_t Bit);
void GPIO_FlipBit(volatile uint32_t *pReg, uint8_t Bit);
void GPIO_SetBit(volatile uint32_t *pReg, uint8_t Bit);

#endif

