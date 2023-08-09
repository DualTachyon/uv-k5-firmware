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

#include <stdbool.h>
#include "ARMCM0.h"

#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/portcon.h"
#include "bsp/dp32g030/syscon.h"
#include "board.h"
#include "driver/crc.h"
#include "driver/eeprom.h"
#include "driver/flash.h"
#include "driver/gpio.h"
#include "driver/st7565.h"
#include "driver/systick.h"
#include "driver/uart.h"

static const char Version[] = "UV-K5 Firmware, v0.01 Open Edition\r\n";

static void FLASHLIGHT_Init(void)
{
	PORTCON_PORTC_IE = PORTCON_PORTC_IE_C5_ENABLE;
	PORTCON_PORTC_PU = PORTCON_PORTC_PU_C5_ENABLE;
	GPIOC->DIR |= GPIO_DIR_3_OUTPUT;

	GPIO_SetBit(&GPIOC->DATA, 10);
	GPIO_SetBit(&GPIOC->DATA, 11);
	GPIO_SetBit(&GPIOC->DATA, 12);
	GPIO_SetBit(&GPIOC->DATA, 13);
}

static void FLASHLIGHT_TurnOn(void)
{
	GPIO_SetBit(&GPIOC->DATA, 3);
}

static void BACKLIGHT_TurnOn(void)
{
	GPIO_SetBit(&GPIOB->DATA, 6);
}

void Main(void)
{
	// Enable clock gating of blocks we need.
	SYSCON_DEV_CLK_GATE = 0
		| SYSCON_DEV_CLK_GATE_GPIOA_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOB_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOC_ENABLE
		| SYSCON_DEV_CLK_GATE_UART1_ENABLE
		| SYSCON_DEV_CLK_GATE_SPI0_ENABLE
		| SYSCON_DEV_CLK_GATE_SARADC_ENABLE
		| SYSCON_DEV_CLK_GATE_CRC_ENABLE
		| SYSCON_DEV_CLK_GATE_AES_ENABLE
		;

	SYSTICK_Init();
	BOARD_Init();

	UART_Init();
	UART_Send(Version, sizeof(Version));

	// Show some signs of life
	FLASHLIGHT_Init();
	FLASHLIGHT_TurnOn();
	BACKLIGHT_TurnOn();

	static const uint8_t TestBitmap0[8] = { 0x80, 0x80, 0x80, 0x80, 0x08, 0x08, 0x08, 0x08 };
	static const uint8_t TestBitmap1[8] = { 0x10, 0x10, 0x10, 0x10, 0x01, 0x01, 0x01, 0x01 };
	ST7565_DrawLine(16, 1, 8, TestBitmap0, false);
	ST7565_DrawLine(24, 3, 8, TestBitmap1, false);

	uint8_t Test[8];
	EEPROM_ReadBuffer(0x0EB0, Test, 8);

	while (1) {
	}
}

