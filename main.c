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

#include <string.h>
#include "app/app.h"
#include "app/dtmf.h"
#include "audio.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"
#include "board.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"
#if defined(ENABLE_UART)
#include "driver/uart.h"
#endif
#include "helper/battery.h"
#include "helper/boot.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/lock.h"
#include "ui/welcome.h"
#include "version.h"

void _putchar(char c)
{
#if defined(ENABLE_UART)
	UART_Send((uint8_t *)&c, 1);
#endif
}

void Main(void)
{
	uint8_t i;

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

#if defined(ENABLE_UART)
	UART_Init();
	UART_Send(UART_Version, sizeof(UART_Version));
#endif

	// Not implementing authentic device checks

	memset(&gEeprom, 0, sizeof(gEeprom));
	memset(gDTMF_String, '-', sizeof(gDTMF_String));
	gDTMF_String[14] = 0;

	BK4819_Init();
	BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);
	BOARD_EEPROM_Init();
	BOARD_EEPROM_LoadMoreSettings();

	RADIO_ConfigureChannel(0, 2);
	RADIO_ConfigureChannel(1, 2);
	RADIO_SelectVfos();
	RADIO_SetupRegisters(true);

	for (i = 0; i < 4; i++) {
		BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);
	}

	BATTERY_GetReadings(false);
	if (!gChargingWithTypeC && !gBatteryDisplayLevel) {
		FUNCTION_Select(FUNCTION_POWER_SAVE);
		GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
		gReducedService = true;
	} else {
		BOOT_Mode_t BootMode;
		uint8_t Channel;

		UI_DisplayWelcome();
		BACKLIGHT_TurnOn();
		SYSTEM_DelayMs(1000);
		gMenuListCount = 49;
#if defined(ENABLE_ALARM)
		gMenuListCount++;
#endif
#if defined(ENABLE_NOAA)
		gMenuListCount++;
#endif

		BootMode = BOOT_GetMode();
		if (gEeprom.POWER_ON_PASSWORD < 1000000) {
			bIsInLockScreen = true;
			UI_DisplayLock();
			bIsInLockScreen = false;
		}

		BOOT_ProcessMode(BootMode);

		GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
		gUpdateStatus = true;
		AUDIO_SetVoiceID(0, VOICE_ID_WELCOME);
		Channel = gEeprom.ScreenChannel[gEeprom.TX_CHANNEL];
		if (IS_MR_CHANNEL(Channel)) {
			AUDIO_SetVoiceID(1, VOICE_ID_CHANNEL_MODE);
			AUDIO_SetDigitVoice(2, Channel + 1);
		} else if (IS_FREQ_CHANNEL(Channel)) {
			AUDIO_SetVoiceID(1, VOICE_ID_FREQUENCY_MODE);
		}
		AUDIO_PlaySingleVoice(0);
#if defined(ENABLE_NOAA)
		RADIO_ConfigureNOAA();
#endif
	}

	while (1) {
		APP_Update();
		if (gNextTimeslice) {
			APP_TimeSlice10ms();
			gNextTimeslice = false;
		}
		if (gNextTimeslice500ms) {
			APP_TimeSlice500ms();
			gNextTimeslice500ms = false;
		}
	}
}

