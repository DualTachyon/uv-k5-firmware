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
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/flash.h"
#include "driver/gpio.h"
#include "driver/st7565.h"
#include "driver/system.h"
#include "driver/systick.h"
#include "driver/uart.h"
#include "driver/keyboard.h"

static const char Version[] = "UV-K5 Firmware, v0.01 Open Edition\r\n";

static void FLASHLIGHT_Init(void)
{
	PORTCON_PORTC_IE = PORTCON_PORTC_IE_C5_BITS_ENABLE;
	PORTCON_PORTC_PU = PORTCON_PORTC_PU_C5_BITS_ENABLE;
	GPIOC->DIR |= GPIO_DIR_3_BITS_OUTPUT;

	GPIO_SetBit(&GPIOC->DATA, 10);
	GPIO_SetBit(&GPIOC->DATA, 11);
	GPIO_SetBit(&GPIOC->DATA, 12);
	GPIO_SetBit(&GPIOC->DATA, 13);
}

static void FLASHLIGHT_TurnOff(void)
{
	GPIO_ClearBit(&GPIOC->DATA, GPIOB_PIN_FLASHLIGHT);
}

static void FLASHLIGHT_TurnOn(void)
{
	GPIO_SetBit(&GPIOC->DATA, GPIOB_PIN_FLASHLIGHT);
}

static void BACKLIGHT_TurnOn(void)
{
	GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
}

void Main(void)
{
	// Enable clock gating of blocks we need.
	SYSCON_DEV_CLK_GATE = 0
		| SYSCON_DEV_CLK_GATE_GPIOA_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOB_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_UART1_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_SPI0_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_CRC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_AES_BITS_ENABLE
		;

	SYSTICK_Init();
	BOARD_Init();

	UART_Init();
	UART_Send(Version, sizeof(Version));

	// Not implementing authentic device checks

	// TODO: EEPROM Init

	BK4819_Init();

	// Below this line is development/test area not conforming to the original firmware

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

    Keyboard_init();

	while (1) {
        UART_Send("\r\nKey: ",7);
        uint8_t key = PollKeyboard() + 0x40;
        UART_Send(&key, 1);

		SYSTEM_DelayMs(500);
		FLASHLIGHT_TurnOff();
		SYSTEM_DelayMs(500);
		FLASHLIGHT_TurnOn();
	}
}

