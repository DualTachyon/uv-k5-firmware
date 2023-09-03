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
#include "app/dtmf.h"
#include "app/fm.h"
#include "bsp/dp32g030/gpio.h"
#include "dcs.h"
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/status.h"
#include "ui/ui.h"

FUNCTION_Type_t gCurrentFunction;

void FUNCTION_Init(void)
{
	if (IS_NOT_NOAA_CHANNEL(gRxInfo->CHANNEL_SAVE)) {
		gCopyOfCodeType = gCodeType;
		if (g_20000381 == 0) {
			if (gRxInfo->IsAM) {
				gCopyOfCodeType = CODE_TYPE_OFF;
			} else {
				gCopyOfCodeType = gRxInfo->pCurrent->CodeType;
			}
		}
	} else {
		gCopyOfCodeType = CODE_TYPE_CONTINUOUS_TONE;
	}
	gDTMF_RequestPending = false;
	gDTMF_WriteIndex = 0;
	memset(gDTMF_Received, 0, sizeof(gDTMF_Received));
	g_CxCSS_TAIL_Found = false;
	g_CDCSS_Lost = false;
	g_CTCSS_Lost = false;
	g_VOX_Lost = false;
	g_SquelchLost = false;
	g_20000342 = 0;
	gSystickFlag10 = false;
	gFoundCTCSS = false;
	gFoundCDCSS = false;
	gFoundCTCSSCountdown = 0;
	gFoundCDCSSCountdown = 0;
	g_20000377 = 0;
	gSystickCountdown2 = 0;
}

void FUNCTION_Select(FUNCTION_Type_t Function)
{
	FUNCTION_Type_t PreviousFunction;
	bool bWasPowerSave;

	PreviousFunction = gCurrentFunction;
	bWasPowerSave = (PreviousFunction == FUNCTION_POWER_SAVE);
	gCurrentFunction = Function;

	if (bWasPowerSave) {
		if (Function != FUNCTION_POWER_SAVE) {
			BK4819_Conditional_RX_TurnOn_and_GPIO6_Enable();
			gThisCanEnable_BK4819_Rxon = false;
			UI_DisplayStatus();
		}
	}

	if (Function == FUNCTION_0) {
		if (gDTMF_ReplyState) {
			RADIO_Something();
		}
		if (PreviousFunction == FUNCTION_TRANSMIT) {
			gVFO_RSSI_Level[0] = 0;
			gVFO_RSSI_Level[1] = 0;
		} else if (PreviousFunction != FUNCTION_TRANSMIT) {
			gBatterySaveCountdown = 1000;
			gSchedulePowerSave = false;
			return;
		}
		if (gFmRadioMode) {
			g_2000038E = 500;
		}
		if (gDTMF_CallState != DTMF_CALL_STATE_CALL_OUT && gDTMF_CallState != DTMF_CALL_STATE_RECEIVED) {
			gBatterySaveCountdown = 1000;
			gSchedulePowerSave = false;
			return;
		}
		gBatterySaveCountdown = 1000;
		gSchedulePowerSave = false;
		gDTMF_AUTO_RESET_TIME = 1 + (gEeprom.DTMF_AUTO_RESET_TIME * 2);
		return;
	}

	if (Function == FUNCTION_MONITOR || Function == FUNCTION_3 || Function == FUNCTION_RECEIVE) {
		gBatterySaveCountdown = 1000;
		gSchedulePowerSave = false;
		g_2000038E = 0;
		return;
	}

	if (Function == FUNCTION_POWER_SAVE) {
		gBatterySave = gEeprom.BATTERY_SAVE * 10;
		gThisCanEnable_BK4819_Rxon = true;
		BK4819_DisableVox();
		BK4819_Sleep();
		BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2, false);
		gBatterySaveCountdownExpired = false;
		gUpdateStatus = true;
		GUI_SelectNextDisplay(DISPLAY_MAIN);
		return;
	}

	if (Function != FUNCTION_TRANSMIT) {
		return;
	}

	if (gFmRadioMode) {
		BK1080_Init(0, false);
	}

	if (gAlarmState == ALARM_STATE_TXALARM && gEeprom.ALARM_MODE != ALARM_MODE_TONE) {
		gAlarmState = ALARM_STATE_ALARM;
		GUI_DisplayScreen();
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		SYSTEM_DelayMs(20);
		BK4819_PlayTone(500, 0);
		SYSTEM_DelayMs(2);
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		gEnableSpeaker = true;
		SYSTEM_DelayMs(60);
		BK4819_ExitTxMute();
		gBatterySaveCountdown = 1000;
		gSchedulePowerSave = false;
		g_2000038E = 0;
		gAlarmToneCounter = 0;
		return;
	}

	GUI_DisplayScreen();
	RADIO_PrepareTransmit();
	BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_RED, true);

	DTMF_Reply();

	if (gAlarmState != ALARM_STATE_OFF) {
		if (gAlarmState == ALARM_STATE_TX1750) {
			BK4819_TransmitTone(true, 1750);
		} else {
			BK4819_TransmitTone(true, 500);
		}
		SYSTEM_DelayMs(2);
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		gAlarmToneCounter = 0;
		g_2000038E = 0;
		gEnableSpeaker = true;
		gSchedulePowerSave = false;
		gBatterySaveCountdown = 1000;
		return;
	}
	if (gCrossTxRadioInfo->SCRAMBLING_TYPE && gSetting_ScrambleEnable) {
		BK4819_EnableScramble(gCrossTxRadioInfo->SCRAMBLING_TYPE - 1U);
		gBatterySaveCountdown = 1000;
		gSchedulePowerSave = false;
		g_2000038E = 0;
		return;
	}
	BK4819_DisableScramble();
	gBatterySaveCountdown = 1000;
	gSchedulePowerSave = false;
	g_2000038E = 0;
}

