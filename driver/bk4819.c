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
#include "bsp/dp32g030/portcon.h"
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
	BK4819_WriteRegister(BK4819_REG_37_VOLTAGE_LDO_AND_MISC_ENABLE, 0x1D0F);
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

static uint16_t BK4819_ReadU16(void)
{
	uint8_t i;
	uint16_t Value;

	PORTCON_PORTC_IE = (PORTCON_PORTC_IE & ~PORTCON_PORTC_IE_C2_MASK) | PORTCON_PORTC_IE_C2_BITS_ENABLE;
	GPIOC->DIR = (GPIOC->DIR & ~GPIO_DIR_2_MASK) | GPIO_DIR_2_BITS_INPUT;
	SYSTICK_DelayUs(1);

	Value = 0;
	for (i = 0; i < 16; i++) {
		Value <<= 1;
		Value |= GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
		SYSTICK_DelayUs(1);
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
		SYSTICK_DelayUs(1);
	}
	PORTCON_PORTC_IE = (PORTCON_PORTC_IE & ~PORTCON_PORTC_IE_C2_MASK) | PORTCON_PORTC_IE_C2_BITS_DISABLE;
	GPIOC->DIR = (GPIOC->DIR & ~GPIO_DIR_2_MASK) | GPIO_DIR_2_BITS_OUTPUT;

	return Value;
}

uint16_t BK4819_GetRegister(BK4819_REGISTER_t Register)
{
	uint16_t Value;

	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_0);
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
	SYSTICK_DelayUs(1);
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_0);

	BK4819_WriteU8(Register | 0x80);

	Value = BK4819_ReadU16();

	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_0);
	SYSTICK_DelayUs(1);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_1);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_2);

	return Value;
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

void BK4819_ToggleGpioOut(BK4819_GPIO_PIN_t Pin, bool bSet)
{
	if (bSet) {
		gBK4819_GpioOutState |= (0x40U >> Pin);
	} else {
		gBK4819_GpioOutState &= ~(0x40U >> Pin);
	}

	BK4819_WriteRegister(BK4819_REG_33, gBK4819_GpioOutState);
}

void BK4819_EnableCDCSS(uint32_t CodeWord)
{
	// Enable CDCSS
	// Transmit positive CDCSS code
	// CDCSS Mode
	// CDCSS 23bit
	// Enable Auto CDCSS Bw Mode
	// Enable Auto CTCSS Bw Mode
	// CTCSS/CDCSS Tx Gain1 Tuning = 51
	BK4819_WriteRegister(BK4819_REG_51_CTCSS_CDCSS_CONFIG, 0x8033);

	// CTC1 Frequency Control Word = 2775
	BK4819_WriteRegister(BK4819_REG_07_CTC_FREQUENCY_CDCSS_BAUD_RATE, (0U << 13) | 2775);

	// Set the code word
	BK4819_WriteRegister(BK4819_REG_08_CDCSS_CALL_SYMBOL_NUMBER_AND_COEFFICIENT, 0x0000 | ((CodeWord >>  0) & 0xFFF));
	BK4819_WriteRegister(BK4819_REG_08_CDCSS_CALL_SYMBOL_NUMBER_AND_COEFFICIENT, 0x8000 | ((CodeWord >> 12) & 0xFFF));
}

void BK4819_EnableCTCSS(uint32_t BaudRate)
{
	uint16_t Config;

	if (BaudRate == 2625) {
		// Enable TxCTCSS
		// CTCSS Mode
		// 1050/4 Detect Enable
		// Enable Auto CDCSS Bw Mode
		// Enable Auto CTCSS Bw Mode
		// CTCSS/CDCSS Tx Gain1 Tuning = 74
		Config = 0x944A;
	} else {
		// Enable TxCTCSS
		// CTCSS Mode
		// Enable Auto CDCSS Bw Mode
		// Enable Auto CTCSS Bw Mode
		// CTCSS/CDCSS Tx Gain1 Tuning = 74
		Config = 0x904A;
	}
	BK4819_WriteRegister(BK4819_REG_51_CTCSS_CDCSS_CONFIG, Config);
	// CTC1 Frequency Control Word
	BK4819_WriteRegister(BK4819_REG_07_CTC_FREQUENCY_CDCSS_BAUD_RATE, (0U << 13) | (BaudRate * 2065) / 1000);
}

void BK4819_Set55HzTailDetection(void)
{
	// CTC2 Frequency Control Word = round_nearest(25391 / 55) = 462
	BK4819_WriteRegister(BK4819_REG_07_CTC_FREQUENCY_CDCSS_BAUD_RATE, (1U << 13) | 462);
}

void BK4819_EnableVox(uint16_t Vox1Threshold, uint16_t Vox0Threshold)
{
	uint16_t Value;

	Value = BK4819_GetRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE);
	// 0xA000 is undocumented?
	BK4819_WriteRegister(BK4819_REG_46_VOICE_AMPLITUDE_THRESHOLD_VOX1, 0xA000 | (Vox1Threshold & 0x07FF));
	// 0x1800 is undocumented?
	BK4819_WriteRegister(BK4819_REG_79_VOX1_DETECTION_INTERVAL_TIME_VOICE_AMPLITUDE_THRESHOLD_VOX0, 0x1800 | (Vox0Threshold & 0x07FF));
	// Bottom 12 bits are undocumented?
	BK4819_WriteRegister(BK4819_REG_7A_VOX0_DETECTION_DELAY, 0x289A);
	// Enable VOX
	BK4819_WriteRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE, Value | 4);
}

void BK4819_SetFilterBandwidth(uint8_t Bandwidth)
{
	if (Bandwidth == 0) {
		BK4819_WriteRegister(BK4819_REG_43_RF_AFT_LPF2_FILTER_BANDWIDTH, 0x3028);
	} else if (Bandwidth == 1) {
		BK4819_WriteRegister(BK4819_REG_43_RF_AFT_LPF2_FILTER_BANDWIDTH, 0x4048);
	}
}

void BK4819_SetupPowerAmplifier(uint16_t Bias, uint32_t Frequency)
{
	uint8_t Gain;

	if (Bias > 255) {
		Bias = 255;
	}
	if (Frequency < 28000000) {
		// Gain 1 = 1
		// Gain 2 = 0
		Gain = 0x08U;
	} else {
		// Gain 1 = 4
		// Gain 2 = 2
		Gain = 0x22U;
	}
	// Enable PACTLoutput
	BK4819_WriteRegister(BK4819_REG_36_POWER_AMPLIFIER_BIAS_AND_GAIN, (Bias << 8) | 0x80U | Gain);
}

void BK4819_SetFrequency(uint32_t Frequency)
{
	BK4819_WriteRegister(BK4819_REG_38_FREQUENCY_LOW,  (Frequency >>  0) & 0xFFFF);
	BK4819_WriteRegister(BK4819_REG_39_FREQUENCY_HIGH, (Frequency >> 16) & 0xFFFF);
}

void BK4819_SetupSquelch(uint8_t SQ0, uint8_t SQ1, uint8_t SQ2, uint8_t SQ3, uint8_t SQ4, uint8_t SQ5)
{
	BK4819_WriteRegister(BK4819_REG_70_TONE1_CONFIG, 0);
	BK4819_WriteRegister(BK4819_REG_4D_GLITCH_THR_SQ0, 0xA000 | SQ4);
	BK4819_WriteRegister(BK4819_REG_4E_SQ1_DELAY_SQ0_DELAY_GLITCH_THR_SQ1, 0x6F00 | SQ5);
	BK4819_WriteRegister(BK4819_REG_4F_EXNOISE_THR_SQ0_EXNOISE_THR_SQ1, (SQ3 << 8) | SQ2);
	BK4819_WriteRegister(BK4819_REG_78_SQUELCH_RSSI_THRESHOLD, (SQ0 << 8) | SQ1);
	BK4819_SetAF(0); // MUTE
	BK4819_RX_TurnOn();
}

void BK4819_SetAF(uint8_t AF)
{
	// AF Output Inverse Mode = Inverse
	// Undocumented bits 0x2040
	BK4819_WriteRegister(BK4819_REG_47_AF_OUTPUT_CONFIG, 0x6040 | (AF << 8));
}

void BK4819_RX_TurnOn(void)
{
	// DSP Voltage Setting = 1
	// ANA LDO = 2.7v
	// VCO LDO = 2.7v
	// RF LDO = 2.7v
	// PLL LDO = 2.7v
	// ANA LDO bypass
	// VCO LDO bypass
	// RF LDO bypass
	// PLL LDO bypass
	// Reserved bit is 1 instead of 0
	// Enable DSP
	// Enable XTAL
	// Enable Band Gap
	BK4819_WriteRegister(BK4819_REG_37_VOLTAGE_LDO_AND_MISC_ENABLE, 0x1F0F);

	// Turn off everything
	BK4819_WriteRegister(BK4819_REG_30_VARIOUS_ENABLE_BITS, 0);

	// Enable VCO Calibration
	// Enable RX Link
	// Enable AF DAC
	// Enable PLL/VCO
	// Disable PA Gain
	// Disable MIC ADC
	// Disable TX DSP
	// Enable RX DSP
	BK4819_WriteRegister(BK4819_REG_30_VARIOUS_ENABLE_BITS, 0xBFF1);
}

void BK4819_Configure_GPIO2_PIN30_GPIO3_PIN31(uint32_t Frequency)
{
	if (Frequency < 28000000) {
		BK4819_ToggleGpioOut(BK4819_GPIO2_PIN30, true);
		BK4819_ToggleGpioOut(BK4819_GPIO3_PIN31, false);
	} else if (Frequency == 0xFFFFFFFF) {
		BK4819_ToggleGpioOut(BK4819_GPIO2_PIN30, false);
		BK4819_ToggleGpioOut(BK4819_GPIO3_PIN31, false);
	} else {
		BK4819_ToggleGpioOut(BK4819_GPIO2_PIN30, false);
		BK4819_ToggleGpioOut(BK4819_GPIO3_PIN31, true);
	}
}

void BK4819_DisableScramble(void)
{
	uint16_t Value;

	Value = BK4819_GetRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE);
	BK4819_WriteRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE, Value & 0xFFFD);
}

void BK4819_EnableScramble(uint8_t Type)
{
	uint16_t Value;

	Value = BK4819_GetRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE);
	BK4819_WriteRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE, Value | 2);
	BK4819_WriteRegister(BK4819_REG_71_TONE1_SCRAMBLE_FREQ_CONTROL_WORD, (Type * 0x0408) + 0x68DC);
}

void BK4819_DisableVox(void)
{
	uint16_t Value;

	Value = BK4819_GetRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE);
	BK4819_WriteRegister(BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE, Value & 0xFFFB);
}

void BK4819_DisableDTMF_SelCall(void)
{
	BK4819_WriteRegister(BK4819_REG_24_DTMF_SELCALL_CONFIG, 0);
}

void BK4819_ConfigureDTMF_SelCall_and_UnknownRegister(void)
{
	BK4819_WriteRegister(BK4819_REG_21, 0x06D8);
	BK4819_WriteRegister(BK4819_REG_24_DTMF_SELCALL_CONFIG, 0x8C7E);
}

