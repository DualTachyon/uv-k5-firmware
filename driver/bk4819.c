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

#include "bk4819.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "driver/systick.h"

uint16_t gBK4819_GpioOutState;

void BK4819_Init(void)
{
	GPIO_SetBit(&GPIOC->DATA, 0);
	GPIO_SetBit(&GPIOC->DATA, 1);
	GPIO_SetBit(&GPIOC->DATA, 2);

	BK4819_WriteRegister(BK4819_REG_00, 0x8000);
	BK4819_WriteRegister(BK4819_REG_00, 0x0000);
	BK4819_WriteRegister(BK4819_REG_37_VOLTATE_LDO_AND_MISC_ENABLE, 0x1D0F);
	BK4819_WriteRegister(BK4819_REG_36_POWER_AMPLIFIER_BIAS_AND_GAIN, 0x0022);
	BK4819_SetAGC(0);
	BK4819_WriteRegister(BK4819_REG_19, 0x1041);
	BK4819_WriteRegister(BK4819_REG_7D_MIC_SENSITIVITY_TUNING, 0xE940);
	BK4819_WriteRegister(BK4819_REG_48_AF_RF_GAIN_DAC_GAIN, 0xB3A8);
	BK4819_WriteRegister(BK4819_REG_09, 0x006F);
	BK4819_WriteRegister(BK4819_REG_09, 0x106B);
	BK4819_WriteRegister(BK4819_REG_09, 0x2067);
	BK4819_WriteRegister(BK4819_REG_09, 0x3062);
	BK4819_WriteRegister(BK4819_REG_09, 0x4050);
	BK4819_WriteRegister(BK4819_REG_09, 0x5047);
	BK4819_WriteRegister(BK4819_REG_09, 0x603A);
	BK4819_WriteRegister(BK4819_REG_09, 0x702C);
	BK4819_WriteRegister(BK4819_REG_09, 0x8041);
	BK4819_WriteRegister(BK4819_REG_09, 0x9037);
	BK4819_WriteRegister(BK4819_REG_09, 0xA025);
	BK4819_WriteRegister(BK4819_REG_09, 0xB017);
	BK4819_WriteRegister(BK4819_REG_09, 0xC0E4);
	BK4819_WriteRegister(BK4819_REG_09, 0xD0CB);
	BK4819_WriteRegister(BK4819_REG_09, 0xE0B5);
	BK4819_WriteRegister(BK4819_REG_09, 0xF09F);
	BK4819_WriteRegister(BK4819_REG_1F, 0x5454);
	BK4819_WriteRegister(BK4819_REG_3E, 0xA037);
	gBK4819_GpioOutState = 0x9000;
	BK4819_WriteRegister(BK4819_REG_33, 0x9000);
	BK4819_WriteRegister(BK4819_REG_3F_INTERRUPT_ENABLE, 0);
}

void BK4819_WriteRegister(BK4819_REGISTER_t Register, uint16_t Data)
{
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_0);
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
	SYSTICK_DelayUs(1);
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_0);
	BK4819_WriteU8(Register);
	SYSTICK_DelayUs(1);
	BK4819_WriteU16(Data);
	SYSTICK_DelayUs(1);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_0);
	SYSTICK_DelayUs(1);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);
}

void BK4819_WriteU8(uint8_t Data)
{
	uint8_t i;

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
	for (i = 0; i < 8; i++) {
		if ((Data & 0x80U) == 0) {
			GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);
		} else {
			GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);
		}
		SYSTICK_DelayUs(1);
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
		SYSTICK_DelayUs(1);
		Data <<= 1;
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
		SYSTICK_DelayUs(1);
	}
}

void BK4819_WriteU16(uint16_t Data)
{
	uint8_t i;

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
	for (i = 0; i < 16; i++) {
		if ((Data & 0x8000U) == 0U) {
			GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);
		} else {
			GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);
		}
		SYSTICK_DelayUs(1);
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
		Data <<= 1;
		SYSTICK_DelayUs(1);
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
		SYSTICK_DelayUs(1);
	}
}

void BK4819_SetAGC(uint8_t Value)
{
	if (Value == 0) {
		BK4819_WriteRegister(BK4819_REG_13, 0x03BE);
		BK4819_WriteRegister(BK4819_REG_12, 0x037B);
		BK4819_WriteRegister(BK4819_REG_11, 0x027B);
		BK4819_WriteRegister(BK4819_REG_10, 0x007A);
		BK4819_WriteRegister(BK4819_REG_14, 0x0019);
		BK4819_WriteRegister(BK4819_REG_49, 0x2A38);
		BK4819_WriteRegister(BK4819_REG_7B, 0x8420);
	} else if (Value == 1) {
		uint8_t i;

		BK4819_WriteRegister(BK4819_REG_13, 0x03BE);
		BK4819_WriteRegister(BK4819_REG_12, 0x037C);
		BK4819_WriteRegister(BK4819_REG_11, 0x027B);
		BK4819_WriteRegister(BK4819_REG_10, 0x007A);
		BK4819_WriteRegister(BK4819_REG_14, 0x0018);
		BK4819_WriteRegister(BK4819_REG_49, 0x2A38);
		BK4819_WriteRegister(BK4819_REG_7B, 0x318C);
		BK4819_WriteRegister(BK4819_REG_7C, 0x595E);
		BK4819_WriteRegister(BK4819_REG_20, 0x8DEF);
		for (i = 0; i < 8; i++) {
			// Bug? The bit 0x2000 below overwrites the (i << 13)
			BK4819_WriteRegister(BK4819_REG_06, ((i << 13) | 0x2500U) + 0x36U);
		}
	}
}

