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
#include "app/aircopy.h"
#include "app/app.h"
#include "app/dtmf.h"
#include "app/fm.h"
#include "app/generic.h"
#include "app/main.h"
#include "app/menu.h"
#include "app/scanner.h"
#include "app/uart.h"
#include "ARMCM0.h"
#include "audio.h"
#include "board.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/backlight.h"
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "driver/system.h"
#include "dtmf.h"
#include "external/printf/printf.h"
#include "frequencies.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "sram-overlay.h"
#include "ui/battery.h"
#include "ui/inputbox.h"
#include "ui/menu.h"
#include "ui/rssi.h"
#include "ui/status.h"
#include "ui/ui.h"

static void APP_ProcessKey(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void APP_StartListening(FUNCTION_Type_t Function);

static void FUN_00005144(void)
{
	if (!g_SquelchLost) {
		return;
	}
	if (gStepDirection == 0) {
		if (g_20000381 && g_20000411 == 0) {
			ScanPauseDelayIn10msec = 100;
			gSystickFlag9 = false;
			g_20000411 = 1;
		}
		if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF) {
			if (gIsNoaaMode) {
				gNOAA_Countdown = 20;
				gScheduleNOAA = false;
			}
			FUNCTION_Select(FUNCTION_3);
			return;
		}
		if (g_20000411) {
			FUNCTION_Select(FUNCTION_3);
			return;
		}
		g_2000033A = 100;
		gSystickFlag7 = false;
	} else {
		if (g_20000411) {
			FUNCTION_Select(FUNCTION_3);
			return;
		}
		ScanPauseDelayIn10msec = 20;
		gSystickFlag9 = false;
	}
	g_20000411 = 1;
	FUNCTION_Select(FUNCTION_3);
}

void FUN_000051e8(void)
{
	bool bFlag;

	if (!g_SquelchLost) {
		FUNCTION_Select(FUNCTION_0);
		gUpdateDisplay = true;
		return;
	}

	bFlag = (gStepDirection == 0 && gCopyOfCodeType == CODE_TYPE_OFF);
	if (gRxInfo->CHANNEL_SAVE >= NOAA_CHANNEL_FIRST && gSystickCountdown2) {
		bFlag = true;
		gSystickCountdown2 = 0;
	}
	if (g_CTCSS_Lost && gCopyOfCodeType == CODE_TYPE_CONTINUOUS_TONE) {
		bFlag = true;
		gFoundCTCSS = false;
	}
	if (g_CDCSS_Lost && gCDCSSCodeType == CDCSS_POSITIVE_CODE && (gCopyOfCodeType == CODE_TYPE_DIGITAL || gCopyOfCodeType == CODE_TYPE_REVERSE_DIGITAL)) {
		gFoundCDCSS = false;
	} else {
		if (!bFlag) {
			return;
		}
	}

	DTMF_HandleRequest();

	if (gStepDirection == 0 && g_20000381 == 0) {
		if (gRxInfo->DTMF_DECODING_ENABLE || gSetting_KILLED) {
			if (gDTMF_CallState == DTMF_CALL_STATE_NONE) {
				if (g_20000411 == 0x01) {
					g_2000033A = 500;
					gSystickFlag7 = false;
					g_20000411 = 2;
					return;
				}
			}
		}
	}

	APP_StartListening(FUNCTION_RECEIVE);
}

void FUN_0000773c(void)
{
	uint8_t Previous;

	Previous = g_20000414;
	gStepDirection = 0;

	if (g_20000413 != 1) {
		if (IS_MR_CHANNEL(g_20000410)) {
			gEeprom.MrChannel[gEeprom.RX_CHANNEL] = g_20000414;
			gEeprom.ScreenChannel[gEeprom.RX_CHANNEL] = Previous;
			RADIO_ConfigureChannel(gEeprom.RX_CHANNEL, 2);
		} else {
			gRxInfo->ConfigRX.Frequency = g_20000418;
			RADIO_ApplyOffset(gRxInfo);
			RADIO_ConfigureSquelchAndOutputPower(gRxInfo);
		}
		RADIO_SetupRegisters(true);
		gUpdateDisplay = true;
		return;
	}

	if (!IS_MR_CHANNEL(gRxInfo->CHANNEL_SAVE)) {
		RADIO_ApplyOffset(gRxInfo);
		RADIO_ConfigureSquelchAndOutputPower(gRxInfo);
		SETTINGS_SaveChannel(gRxInfo->CHANNEL_SAVE, gEeprom.RX_CHANNEL, gRxInfo, 1);
		return;
	}

	SETTINGS_SaveVfoIndices();
}

void FUN_000052f0(void)
{
	uint8_t Value;

	Value = 0;

	if (gSystickFlag10) {
		Value = 1;
	} else if (gStepDirection && IS_FREQ_CHANNEL(g_20000410)) {
		if (g_SquelchLost) {
			return;
		}
		Value = 1;
	} else if (gCopyOfCodeType == CODE_TYPE_CONTINUOUS_TONE && gFoundCTCSS && gFoundCTCSSCountdown == 0) {
		gFoundCTCSS = false;
		gFoundCDCSS = false;
		Value = 1;
	} else if ((gCopyOfCodeType == CODE_TYPE_DIGITAL || gCopyOfCodeType == CODE_TYPE_REVERSE_DIGITAL) && gFoundCDCSS && gFoundCDCSSCountdown == 0) {
		gFoundCTCSS = false;
		gFoundCDCSS = false;
		Value = 1;
	} else {
		if (g_SquelchLost) {
			if (g_20000377 == 0 && IS_NOT_NOAA_CHANNEL(gRxInfo->CHANNEL_SAVE)) {
				switch (gCopyOfCodeType) {
				case CODE_TYPE_OFF:
					if (gEeprom.SQUELCH_LEVEL) {
						if (g_CxCSS_TAIL_Found) {
							Value = 2;
							g_CxCSS_TAIL_Found = false;
						}
					}
					break;

				case CODE_TYPE_CONTINUOUS_TONE:
					if (g_CTCSS_Lost) {
						gFoundCTCSS = false;
					} else if (!gFoundCTCSS) {
						gFoundCTCSS = true;
						gFoundCTCSSCountdown = 100;
					}
					if (g_CxCSS_TAIL_Found) {
						Value = 2;
						g_CxCSS_TAIL_Found = false;
					}
					break;

				case CODE_TYPE_DIGITAL:
				case CODE_TYPE_REVERSE_DIGITAL:
					if (g_CDCSS_Lost && gCDCSSCodeType == CDCSS_POSITIVE_CODE) {
						gFoundCDCSS = false;
					} else if (!gFoundCDCSS) {
						gFoundCDCSS = true;
						gFoundCDCSSCountdown = 100;
					}
					if (g_CxCSS_TAIL_Found) {
						if (BK4819_GetCTCType() == 1) {
							Value = 2;
						}
						g_CxCSS_TAIL_Found = false;
					}
					break;

				default:
					break;
				}
			}
		} else {
			Value = 1;
		}
	}

	if (g_20000377 || Value || !gNextTimeslice40ms || !gEeprom.TAIL_NOTE_ELIMINATION || (gCopyOfCodeType != CODE_TYPE_DIGITAL && gCopyOfCodeType != CODE_TYPE_REVERSE_DIGITAL)) {
	} else {
		if (BK4819_GetCTCType() == 1) {
			Value = 2;
		}
	}

	gNextTimeslice40ms = false;

	switch (Value) {
	case 1:
		RADIO_SetupRegisters(true);
		if (IS_NOAA_CHANNEL(gRxInfo->CHANNEL_SAVE)) {
			gSystickCountdown2 = 300;
		}
		gUpdateDisplay = true;
		if (gStepDirection) {
			switch (gEeprom.SCAN_RESUME_MODE) {
			case SCAN_RESUME_CO:
				ScanPauseDelayIn10msec = 360;
				gSystickFlag9 = false;
				break;
			case SCAN_RESUME_SE:
				FUN_0000773c();
				break;
			}
		}
		break;
	case 2:
		if (gEeprom.TAIL_NOTE_ELIMINATION) {
			GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
			g_20000342 = 20;
			gSystickFlag10 = false;
			gEnableSpeaker = false;
			g_20000377 = 1;
		}
		break;
	}
}

void FUN_0000510c(void)
{
	switch (gCurrentFunction) {
	case FUNCTION_0:
		FUN_00005144();
		break;
	case FUNCTION_POWER_SAVE:
		if (!gThisCanEnable_BK4819_Rxon) {
			FUN_00005144();
		}
		break;
	case FUNCTION_3:
		FUN_000051e8();
		break;
	case FUNCTION_RECEIVE:
		FUN_000052f0();
		break;
	default:
		break;
	}
}

void APP_StartListening(FUNCTION_Type_t Function)
{
	if (!gSetting_KILLED) {
		if (gFmRadioMode) {
			BK1080_Init(0, false);
		}
		gVFO_RSSI_Level[gEeprom.RX_CHANNEL == 0] = 0;
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		gEnableSpeaker = true;
		BACKLIGHT_TurnOn();
		if (gStepDirection) {
			switch (gEeprom.SCAN_RESUME_MODE) {
			case SCAN_RESUME_TO:
				if (gScanPauseMode == 0) {
					ScanPauseDelayIn10msec = 500;
					gSystickFlag9 = false;
					gScanPauseMode = 1;
				}
				break;
			case SCAN_RESUME_CO:
			case SCAN_RESUME_SE:
				ScanPauseDelayIn10msec = 0;
				gSystickFlag9 = false;
				break;
			}
			g_20000413 = 1;
		}
		if (IS_NOAA_CHANNEL(gRxInfo->CHANNEL_SAVE) && gIsNoaaMode) {
			gRxInfo->CHANNEL_SAVE = gNoaaChannel + NOAA_CHANNEL_FIRST;
			gRxInfo->pCurrent->Frequency = NoaaFrequencyTable[gNoaaChannel];
			gRxInfo->pReverse->Frequency = NoaaFrequencyTable[gNoaaChannel];
			gEeprom.ScreenChannel[gEeprom.RX_CHANNEL] = gRxInfo->CHANNEL_SAVE;
			gNOAA_Countdown = 500;
			gScheduleNOAA = false;
		}
		if (g_20000381) {
			g_20000381 = 2;
		}
		if ((gStepDirection == 0 || g_20000381 == 0) && gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
			g_2000041F = 1;
			g_2000033A = 360;
			gSystickFlag7 = false;
		}
		if (gRxInfo->IsAM) {
			BK4819_WriteRegister(BK4819_REG_48, 0xB3A8);
			g_20000474 = 0;
		} else {
			BK4819_WriteRegister(BK4819_REG_48, 0xB000
					| (gEeprom.VOLUME_GAIN << 4)
					| (gEeprom.DAC_GAIN << 0)
					);
		}
		if (gVoiceWriteIndex == 0) {
			if (gRxInfo->IsAM) {
				BK4819_SetAF(BK4819_AF_AM);
			} else {
				BK4819_SetAF(BK4819_AF_OPEN);
			}
		}
		FUNCTION_Select(Function);
		if (Function == FUNCTION_MONITOR || gFmRadioMode) {
			GUI_SelectNextDisplay(DISPLAY_MAIN);
			return;
		}
		gUpdateDisplay = true;
	}
}

void APP_SetFrequencyByStep(VFO_Info_t *pInfo, int8_t Step)
{
	uint32_t Frequency;

	Frequency = pInfo->ConfigRX.Frequency + (Step * pInfo->StepFrequency);
	if (Frequency >= gUpperLimitFrequencyBandTable[pInfo->Band]) {
		pInfo->ConfigRX.Frequency = gLowerLimitFrequencyBandTable[pInfo->Band];
	} else if (Frequency < gLowerLimitFrequencyBandTable[pInfo->Band]) {
		pInfo->ConfigRX.Frequency = FREQUENCY_FloorToStep(gUpperLimitFrequencyBandTable[pInfo->Band], pInfo->StepFrequency, gLowerLimitFrequencyBandTable[pInfo->Band]);
	} else {
		pInfo->ConfigRX.Frequency = Frequency;
	}
}

void APP_MoreRadioStuff(void)
{
	APP_SetFrequencyByStep(gRxInfo, gStepDirection);
	RADIO_ApplyOffset(gRxInfo);
	RADIO_ConfigureSquelchAndOutputPower(gRxInfo);
	RADIO_SetupRegisters(true);
	gUpdateDisplay = true;
	ScanPauseDelayIn10msec = 10;
	g_20000413 = 0;
}

void FUN_00007dd4(void)
{
	uint8_t Ch1 = gEeprom.SCANLIST_PRIORITY_CH1[gEeprom.SCAN_LIST_DEFAULT];
	uint8_t Ch2 = gEeprom.SCANLIST_PRIORITY_CH2[gEeprom.SCAN_LIST_DEFAULT];
	uint8_t PreviousCh, Ch;
	bool bEnabled;

	PreviousCh = g_20000410;
	bEnabled = gEeprom.SCAN_LIST_ENABLED[gEeprom.SCAN_LIST_DEFAULT];
	if (bEnabled) {
		if (g_20000415 == 0) {
			g_20000416 = g_20000410;
			if (RADIO_CheckValidChannel(Ch1, false, 0)) {
				g_20000410 = Ch1;
			} else {
				g_20000415 = 1;
			}
		}
		if (g_20000415 == 1) {
			if (RADIO_CheckValidChannel(Ch2, false, 0)) {
				g_20000410 = Ch2;
			} else {
				g_20000415 = 2;
			}
		}
		if (g_20000415 == 2) {
			g_20000410 = g_20000416;
		}
	}

	Ch = RADIO_FindNextChannel(g_20000410 + gStepDirection, gStepDirection, true, gEeprom.SCAN_LIST_DEFAULT);
	if (Ch == 0xFF) {
		return;
	}

	g_20000410 = Ch;
	if (PreviousCh != g_20000410) {
		gEeprom.MrChannel[gEeprom.RX_CHANNEL] = g_20000410;
		gEeprom.ScreenChannel[gEeprom.RX_CHANNEL] = g_20000410;
		RADIO_ConfigureChannel(gEeprom.RX_CHANNEL, 2);
		RADIO_SetupRegisters(true);
		gUpdateDisplay = true;
	}
	ScanPauseDelayIn10msec = 20;
	g_20000413 = 0;
	if (bEnabled) {
		g_20000415++;
		if (g_20000415 >= 2) {
			g_20000415 = 0;
		}
	}
}

void NOAA_IncreaseChannel(void)
{
	gNoaaChannel++;
	if (gNoaaChannel > 9) {
		gNoaaChannel = 0;
	}
}

void FUN_00007f4c(void)
{
	if (gIsNoaaMode) {
		if (IS_NOT_NOAA_CHANNEL(gEeprom.ScreenChannel[0]) || IS_NOT_NOAA_CHANNEL(gEeprom.ScreenChannel[1])) {
			gEeprom.RX_CHANNEL = gEeprom.RX_CHANNEL == 0;
		} else {
			gEeprom.RX_CHANNEL = 0;
		}
		gRxInfo = &gEeprom.VfoInfo[gEeprom.RX_CHANNEL];
		if (gEeprom.VfoInfo[0].CHANNEL_SAVE >= NOAA_CHANNEL_FIRST) {
			NOAA_IncreaseChannel();
		}
	} else {
		gEeprom.RX_CHANNEL = gEeprom.RX_CHANNEL == 0;
		gRxInfo = &gEeprom.VfoInfo[gEeprom.RX_CHANNEL];
	}
	RADIO_SetupRegisters(false);
	if (gIsNoaaMode) {
		g_2000033A = 7;
	} else {
		g_2000033A = 10;
	}
}

void APP_PlayFM(void)
{
	if (!FM_CheckFrequencyLock(gEeprom.FM_FrequencyPlaying, gEeprom.FM_LowerLimit)) {
		if (!gFM_AutoScan) {
			gFmPlayCountdown = 0;
			g_20000427 = 1;
			if (!gEeprom.FM_IsMrMode) {
				gEeprom.FM_SelectedFrequency = gEeprom.FM_FrequencyPlaying;
			}
			GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
			gEnableSpeaker = true;
		} else {
			if (gFM_ChannelPosition < 20) {
				gFM_Channels[gFM_ChannelPosition++] = gEeprom.FM_FrequencyPlaying;
				if (gEeprom.FM_UpperLimit > gEeprom.FM_FrequencyPlaying) {
					FM_Tune(gEeprom.FM_FrequencyPlaying, gFM_Step, false);
				} else {
					FM_Play();
				}
			} else {
				FM_Play();
			}
		}
	} else if (gFM_AutoScan) {
		if (gEeprom.FM_UpperLimit > gEeprom.FM_FrequencyPlaying) {
			FM_Tune(gEeprom.FM_FrequencyPlaying, gFM_Step, false);
		} else {
			FM_Play();
		}
	} else {
		FM_Tune(gEeprom.FM_FrequencyPlaying, gFM_Step, false);
	}

	GUI_SelectNextDisplay(DISPLAY_FM);
}

void APP_StartFM(void)
{
	gFmRadioMode = true;
	gFM_Step = 0;
	g_2000038E = 0;
	BK1080_Init(gEeprom.FM_FrequencyPlaying, true);
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	gEnableSpeaker = true;
	gUpdateStatus = true;
}

void APP_CheckRadioInterrupts(void)
{
	if (gScreenToDisplay == DISPLAY_SCANNER) {
		return;
	}

	while (BK4819_GetRegister(BK4819_REG_0C) & 1U) {
		uint16_t Mask;

		BK4819_WriteRegister(BK4819_REG_02, 0);
		Mask = BK4819_GetRegister(BK4819_REG_02);
		if (Mask & BK4819_REG_02_DTMF_5TONE_FOUND) {
			gDTMF_RequestPending = true;
			gDTMF_RecvTimeout = 5;
			if (gDTMF_WriteIndex > 15) {
				uint8_t i;
				for (i = 0; i < sizeof(gDTMF_Received) - 1; i++) {
					gDTMF_Received[i] = gDTMF_Received[i + 1];
				}
				gDTMF_WriteIndex = 15;
			}
			gDTMF_Received[gDTMF_WriteIndex++] = DTMF_GetCharacter(BK4819_GetDTMF_5TONE_Code());
			if (gCurrentFunction == FUNCTION_RECEIVE) {
				DTMF_HandleRequest();
			}
		}
		if (Mask & BK4819_REG_02_CxCSS_TAIL) {
			g_CxCSS_TAIL_Found = true;
		}
		if (Mask & BK4819_REG_02_CDCSS_LOST) {
			g_CDCSS_Lost = true;
			gCDCSSCodeType = BK4819_GetCDCSSCodeType();
		}
		if (Mask & BK4819_REG_02_CDCSS_FOUND) {
			g_CDCSS_Lost = false;
		}
		if (Mask & BK4819_REG_02_CTCSS_LOST) {
			g_CTCSS_Lost = true;
		}
		if (Mask & BK4819_REG_02_CTCSS_FOUND) {
			g_CTCSS_Lost = false;
		}
		if (Mask & BK4819_REG_02_VOX_LOST) {
			g_VOX_Lost = true;
			g_200003B8 = 10;
			if (gEeprom.VOX_SWITCH) {
				if (gCurrentFunction == FUNCTION_POWER_SAVE && !gThisCanEnable_BK4819_Rxon) {
					gBatterySave = 20;
					gBatterySaveCountdownExpired = 0;
				}
				if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF && (gSystickFlag7 || g_2000033A < 20)) {
					g_2000033A = 20;
					gSystickFlag7 = false;
				}
			}
		}
		if (Mask & BK4819_REG_02_VOX_FOUND) {
			g_VOX_Lost = false;
			g_200003B8 = 0;
		}
		if (Mask & BK4819_REG_02_SQUELCH_LOST) {
			g_SquelchLost = true;
			BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28, true);
		}
		if (Mask & BK4819_REG_02_SQUELCH_FOUND) {
			g_SquelchLost = false;
			BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28, false);
		}
		if (Mask & BK4819_REG_02_FSK_FIFO_ALMOST_FULL && gScreenToDisplay == DISPLAY_AIRCOPY && gAircopyState == AIRCOPY_TRANSFER && gAirCopyIsSendMode == 0) {
			uint8_t i;

			for (i = 0; i < 4; i++) {
				g_FSK_Buffer[gFSKWriteIndex++] = BK4819_GetRegister(BK4819_REG_5F);
			}
			AIRCOPY_StorePacket();
		}
	}
}

void TalkRelatedCode(void)
{
	RADIO_SendEndOfTransmission();
	RADIO_EnableCxCSS();
	RADIO_SetupRegisters(false);
}

static void FUN_00008334(void)
{
	if (!gSetting_KILLED) {
		if (g_200003B6 == 0) {
			if (g_200003B8) {
				return;
			}
		} else {
			g_VOX_Lost = false;
			g_200003B8 = 0;
		}
		if (gCurrentFunction != FUNCTION_RECEIVE && gCurrentFunction != FUNCTION_MONITOR && gStepDirection == 0 && g_20000381 == 0 && !gFmRadioMode) {
			if (g_200003B4 == 1) {
				if (g_VOX_Lost) {
					gSystickCountdown11 = 100;
				} else if (gSystickCountdown11 == 0) {
					g_200003B4 = 0;
				}
				if (gCurrentFunction == FUNCTION_TRANSMIT && !gPttIsPressed && g_200003B4 == 0) {
					if (g_200003FD == 1) {
						FUNCTION_Select(FUNCTION_0);
					} else {
						TalkRelatedCode();
						if (gEeprom.REPEATER_TAIL_TONE_ELIMINATION == 0) {
							FUNCTION_Select(FUNCTION_0);
						} else {
							gRTTECountdown = gEeprom.REPEATER_TAIL_TONE_ELIMINATION * 10;
						}
					}
					gUpdateDisplay = true;
					g_200003FD = 0;
					return;
				}
			} else if (g_VOX_Lost) {
				g_200003B4 = 1;
				if (gCurrentFunction == FUNCTION_POWER_SAVE) {
					FUNCTION_Select(FUNCTION_0);
				}
				if (gCurrentFunction != FUNCTION_TRANSMIT) {
					gDTMF_ReplyState = DTMF_REPLY_UP_CODE;
					RADIO_SomethingWithTransmit();
					gUpdateDisplay = true;
				}
			}
		}
	}
}

void APP_Update(void)
{
	if (gFlagPlayQueuedVoice) {
		AUDIO_PlayQueuedVoice();
		gFlagPlayQueuedVoice = false;
	}

	if (gCurrentFunction == FUNCTION_TRANSMIT && gTxTimeoutReached) {
		gTxTimeoutReached = false;
		g_200003FD = 1;
		TalkRelatedCode();
		AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP);
		RADIO_SomethingElse(4);
		GUI_DisplayScreen();
	}
	if (gReducedService) {
		return;
	}
	if (gCurrentFunction != FUNCTION_TRANSMIT) {
		FUN_0000510c();
	}
	if (gFmRadioCountdown) {
		return;
	}

	if (gScreenToDisplay != DISPLAY_SCANNER && gStepDirection && gSystickFlag9 && !gPttIsPressed && gVoiceWriteIndex == 0) {
		if (IS_FREQ_CHANNEL(g_20000410)) {
			if (gCurrentFunction == FUNCTION_3) {
				APP_StartListening(FUNCTION_RECEIVE);
			} else {
				APP_MoreRadioStuff();
			}
		} else {
			if (gCopyOfCodeType == CODE_TYPE_OFF && gCurrentFunction == FUNCTION_3) {
				APP_StartListening(FUNCTION_RECEIVE);
			} else {
				FUN_00007dd4();
			}
		}
		gScanPauseMode = 0;
		g_20000411 = 0;
		gSystickFlag9 = false;
	}

	if (g_20000381 == 1 && gSystickFlag9 && gVoiceWriteIndex == 0) {
		MENU_SelectNextDCS();
		gSystickFlag9 = false;
	}

	if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF && gIsNoaaMode && gScheduleNOAA && gVoiceWriteIndex == 0) {
		NOAA_IncreaseChannel();
		RADIO_SetupRegisters(false);
		gScheduleNOAA = false;
		gNOAA_Countdown = 7;
	}

	if (gScreenToDisplay != DISPLAY_SCANNER && gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
		if (gSystickFlag7 && gVoiceWriteIndex == 0) {
			if (gStepDirection == 0 && g_20000381 == 0) {
				if (!gPttIsPressed && !gFmRadioMode && gDTMF_CallState == DTMF_CALL_STATE_NONE && gCurrentFunction != FUNCTION_POWER_SAVE) {
					FUN_00007f4c();
					if (g_2000041F == 1 && gScreenToDisplay == DISPLAY_MAIN) {
						GUI_SelectNextDisplay(DISPLAY_MAIN);
					}
					g_2000041F = 0;
					gScanPauseMode = 0;
					g_20000411 = 0;
					gSystickFlag7 = false;
				}
			}
		}
	}

	if (gFM_Step && gScheduleFM && gCurrentFunction != FUNCTION_MONITOR && gCurrentFunction != FUNCTION_RECEIVE && gCurrentFunction != FUNCTION_TRANSMIT) {
		APP_PlayFM();
		gScheduleFM = false;
	}

	if (gEeprom.VOX_SWITCH) {
		FUN_00008334();
	}

	if (gSchedulePowerSave) {
		if (gEeprom.BATTERY_SAVE == 0 || gStepDirection || g_20000381 || gFmRadioMode || gPttIsPressed || gScreenToDisplay != DISPLAY_MAIN || gKeyBeingHeld || gDTMF_CallState != DTMF_CALL_STATE_NONE) {
			gBatterySaveCountdown = 1000;
		} else {
			if ((IS_NOT_NOAA_CHANNEL(gEeprom.ScreenChannel[0]) && IS_NOT_NOAA_CHANNEL(gEeprom.ScreenChannel[1])) || !gIsNoaaMode) {
				FUNCTION_Select(FUNCTION_POWER_SAVE);
			} else {
				gBatterySaveCountdown = 1000;
			}
		}
		gSchedulePowerSave = false;
	}

	if (gBatterySaveCountdownExpired && gCurrentFunction == FUNCTION_POWER_SAVE && gVoiceWriteIndex == 0) {
		if (gThisCanEnable_BK4819_Rxon) {
			BK4819_Conditional_RX_TurnOn_and_GPIO6_Enable();
			if (gEeprom.VOX_SWITCH) {
				BK4819_EnableVox(gEeprom.VOX1_THRESHOLD, gEeprom.VOX0_THRESHOLD);
			}
			if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF && gStepDirection == 0 && g_20000381 == 0) {
				FUN_00007f4c();
				g_20000382 = 0;
			}
			FUNCTION_Init();
			gBatterySave = 10;
			gThisCanEnable_BK4819_Rxon = false;
		} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF || gStepDirection || g_20000381 || g_20000382) {
			gCurrentRSSI = BK4819_GetRSSI();
			UI_UpdateRSSI(gCurrentRSSI);
			gBatterySave = gEeprom.BATTERY_SAVE * 10;
			gThisCanEnable_BK4819_Rxon = true;
			BK4819_DisableVox();
			BK4819_Sleep();
			BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2, false);
			// Authentic device checked removed
		} else {
			FUN_00007f4c();
			g_20000382 = 1;
			gBatterySave = 10;
		}
		gBatterySaveCountdownExpired = false;
	}
}

void APP_CheckKeys(void)
{
	KEY_Code_t Key;

	if (gSetting_KILLED || (gScreenToDisplay == DISPLAY_AIRCOPY && gAircopyState != AIRCOPY_READY)) {
		return;
	}

	if (gPttIsPressed) {
		if (GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT)) {
			SYSTEM_DelayMs(20);
			if (GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT)) {
				APP_ProcessKey(KEY_PTT, false, false);
				gPttIsPressed = false;
				if (gKeyReading1 != KEY_INVALID) {
					g_20000394 = true;
				}
			}
		}
	} else {
		if (!GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT)) {
			gPttDebounceCounter = gPttDebounceCounter + 1;
			if (gPttDebounceCounter > 4) {
				gPttIsPressed = true;
				APP_ProcessKey(KEY_PTT, true, false);
			}
		} else {
			gPttDebounceCounter = 0;
		}
	}
	Key = KEYBOARD_Poll();
	if (gKeyReading0 != Key) {
		if (gKeyReading0 != KEY_INVALID && Key != KEY_INVALID) {
			APP_ProcessKey(gKeyReading1, false, gKeyBeingHeld);
		}
		gKeyReading0 = Key;
		gDebounceCounter = 0;
		return;
	}
	gDebounceCounter++;
	if (gDebounceCounter == 2) {
		if (Key == KEY_INVALID) {
			if (gKeyReading1 != KEY_INVALID) {
				APP_ProcessKey(gKeyReading1, false, gKeyBeingHeld);
				gKeyReading1 = KEY_INVALID;
			}
		} else {
			gKeyReading1 = Key;
			APP_ProcessKey(Key, true, false);
		}
		gKeyBeingHeld = false;
	} else if (gDebounceCounter == 128) {
		if (Key == KEY_STAR || Key == KEY_F || Key == KEY_SIDE2 || Key == KEY_SIDE1 || Key == KEY_UP || Key == KEY_DOWN) {
			gKeyBeingHeld = true;
			APP_ProcessKey(Key, true, true);
		}
	} else if (gDebounceCounter > 128) {
		if (Key == KEY_UP || Key == KEY_DOWN) {
			gKeyBeingHeld = true;
			if ((gDebounceCounter & 15) == 0) {
				APP_ProcessKey(Key, true, true);
			}
		}
		if (gDebounceCounter != 0xFFFF) {
			return;
		}
		gDebounceCounter = 128;
	}
}

void APP_TimeSlice10ms(void)
{
	gFlashLightBlinkCounter++;

	if (UART_IsCommandAvailable()) {
		__disable_irq();
		UART_HandleCommand();
		__enable_irq();
	}

	if (gReducedService) {
		return;
	}

	if (gCurrentFunction != FUNCTION_POWER_SAVE || !gThisCanEnable_BK4819_Rxon) {
		APP_CheckRadioInterrupts();
	}

	if (gCurrentFunction != FUNCTION_TRANSMIT) {
		if (gUpdateStatus) {
			UI_DisplayStatus();
			gUpdateStatus = false;
		}
		if (gUpdateDisplay) {
			GUI_DisplayScreen();
			gUpdateDisplay = false;
		}
	}

	// Skipping authentic device checks

	if (gFmRadioCountdown) {
		return;
	}

	if (gFlashLightState == FLASHLIGHT_BLINK && (gFlashLightBlinkCounter & 15U) == 0) {
		GPIO_FlipBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT);
	}
	if (g_200003B6) {
		g_200003B6--;
	}
	if (g_200003B8) {
		g_200003B8--;
	}
	if (gCurrentFunction == FUNCTION_TRANSMIT) {
		if (g_20000383 == 1 || g_20000383 == 2) {
			uint16_t Value;

			g_20000422++;
			g_20000420++;

			Value = 500 + (g_20000420 * 25);
			if (Value > 1500) {
				Value = 500;
				g_20000420 = 0;
			}
			BK4819_SetScrambleFrequencyControlWord(Value);
			if (gEeprom.ALARM_MODE == ALARM_MODE_TONE && g_20000422 == 512) {
				g_20000422 = 0;
				if (g_20000383 == 1) {
					g_20000383 = 2;
					RADIO_EnableCxCSS();
					BK4819_SetupPowerAmplifier(0, 0);
					BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1, false);
					BK4819_Enable_AfDac_DiscMode_TxDsp();
					BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29, false);
					GUI_DisplayScreen();
				} else {
					g_20000383 = 1;
					GUI_DisplayScreen();
					BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29, true);
					RADIO_PrepareTransmit();
					BK4819_TransmitTone(true, 500);
					SYSTEM_DelayMs(2);
					GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
					gEnableSpeaker = true;
					g_20000420 = 0;
				}
			}
		}
		if (gRTTECountdown) {
			gRTTECountdown--;
			if (gRTTECountdown == 0) {
				FUNCTION_Select(FUNCTION_0);
				gUpdateDisplay = true;
			}
		}
	}
	if (gFmRadioMode && g_2000038E) {
		g_2000038E--;
		if (g_2000038E == 0) {
			APP_StartFM();
			GUI_SelectNextDisplay(DISPLAY_FM);
		}
	}
	if (gScreenToDisplay == DISPLAY_SCANNER) {
		uint32_t Result;
		int32_t Delta;
		BK4819_CssScanResult_t ScanResult;
		uint16_t CtcssFreq;

		if (g_2000045D) {
			g_2000045D--;
			APP_CheckKeys();
			return;
		}
		if (gScannerEditState != 0) {
			APP_CheckKeys();
			return;
		}

		switch (gScanState) {
		case 0:
			if (!BK4819_GetFrequencyScanResult(&Result)) {
				break;
			}

			Delta = Result - gScanFrequency;
			gScanFrequency = Result;
			if (Delta < 0) {
				Delta = -Delta;
			}
			if (Delta < 100) {
				g_2000045F++;
			} else {
				g_2000045F = 0;
			}
			BK4819_DisableFrequencyScan();
			if (g_2000045F < 3) {
				BK4819_EnableFrequencyScan();
			} else {
				BK4819_SetScanFrequency(gScanFrequency);
				gCS_ScannedIndex = 0xFF;
				gCS_ScannedType = 0xFF;
				g_2000045F = 0;
				g_2000045C = 0;
				g_20000464 = 0;
				gScanState = 1;
				GUI_SelectNextDisplay(DISPLAY_SCANNER);
			}
			g_2000045D = 0x15;
			break;

		case 1:
			ScanResult = BK4819_GetCxCSSScanResult(&Result, &CtcssFreq);
			if (ScanResult == 0) {
				break;
			}
			BK4819_Disable();
			if (ScanResult == BK4819_CSS_RESULT_CDCSS) {
				uint8_t Index;

				Index = DCS_GetCdcssIndex(Result);
				if (Index != 0xFF) {
					gCS_ScannedIndex = Index;
					gCS_ScannedType = CODE_TYPE_DIGITAL;
					gScanState = 2;
					g_2000045C = 1;
				}
			} else if (ScanResult == BK4819_CSS_RESULT_CTCSS) {
				uint8_t Index;

				Index = DCS_GetCtcssIndex(CtcssFreq);
				if (Index != 0xFF) {
					if (Index == gCS_ScannedIndex && gCS_ScannedType == CODE_TYPE_CONTINUOUS_TONE) {
						g_2000045F++;
						if (1 < g_2000045F) {
							gScanState = 2;
							g_2000045C = 1;
						}
					} else {
						g_2000045F = 0;
					}
					gCS_ScannedType = CODE_TYPE_CONTINUOUS_TONE;
					gCS_ScannedIndex = Index;
				}
			}
			if (gScanState < 2) {
				BK4819_SetScanFrequency(gScanFrequency);
				g_2000045D = 0x15;
				break;
			}
			GUI_SelectNextDisplay(DISPLAY_SCANNER);
			break;
		}
	}

	if (gScreenToDisplay == DISPLAY_AIRCOPY && gAircopyState == AIRCOPY_TRANSFER && gAirCopyIsSendMode == 1) {
		if (gAircopySendCountdown) {
			gAircopySendCountdown--;
			if (gAircopySendCountdown == 0) {
				AIRCOPY_SendMessage();
				GUI_DisplayScreen();
			}
		}
	}

	APP_CheckKeys();
}

void APP_TimeSlice500ms(void)
{
	// Skipped authentic device check

	if (gKeypadLocked) {
		gKeypadLocked--;
		if (!gKeypadLocked) {
			gUpdateDisplay = true;
		}
	}

	// Skipped authentic device check

	if (gFmRadioCountdown) {
		gFmRadioCountdown--;
		return;
	}
	if (gReducedService) {
		BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);
		if ((gBatteryCurrent < 0x1f5) && (gBatteryCurrentVoltage <= gBatteryCalibration[3])) {
			return;
		}
		overlay_FLASH_RebootToBootloader();
		return;
	}

	g_200003E2++;

	// Skipped authentic device check

	if (gCurrentFunction != FUNCTION_TRANSMIT) {
		if ((g_200003E2 & 1) == 0) {
			BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[gBatteryVoltageIndex++], &gBatteryCurrent);
			if (gBatteryVoltageIndex > 3) {
				gBatteryVoltageIndex = 0;
			}
			BATTERY_GetReadings(true);
		}
		if (gCurrentFunction != FUNCTION_POWER_SAVE) {
			gCurrentRSSI = BK4819_GetRSSI();
			UI_UpdateRSSI(gCurrentRSSI);
			if (gCurrentFunction == FUNCTION_TRANSMIT) {
				goto LAB_00004b08;
			}
		}
		if ((gFM_Step == 0 || gAskToSave) && gStepDirection == 0 && g_20000381 == 0) {
			if (gBacklightCountdown) {
				gBacklightCountdown--;
				if (gBacklightCountdown == 0) {
					GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
				}
			}
			if (gScreenToDisplay != DISPLAY_AIRCOPY && (gScreenToDisplay != DISPLAY_SCANNER || (1 < gScanState))) {
				if (gEeprom.AUTO_KEYPAD_LOCK && gKeyLockCountdown && !gDTMF_InputMode) {
					gKeyLockCountdown--;
					if (gKeyLockCountdown == 0) {
						gEeprom.KEY_LOCK = true;
					}
					gUpdateStatus = true;
				}
				if (g_20000393) {
					g_20000393--;
					if (g_20000393 == 0) {
						if (gInputBoxIndex || gDTMF_InputMode || gScreenToDisplay == DISPLAY_MENU) {
							AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
						}
						if (gScreenToDisplay == DISPLAY_SCANNER) {
							BK4819_StopScan();
							RADIO_ConfigureChannel(0, 2);
							RADIO_ConfigureChannel(1, 2);
							RADIO_SetupRegisters(true);
						}
						gWasFKeyPressed = false;
						gUpdateStatus = true;
						gInputBoxIndex = 0;
						gDTMF_InputMode = false;
						gDTMF_InputIndex = 0;
						gAskToSave = false;
						gAskToDelete = false;
						if (gFmRadioMode && gCurrentFunction != FUNCTION_RECEIVE && gCurrentFunction != FUNCTION_MONITOR && gCurrentFunction != FUNCTION_TRANSMIT) {
							GUI_SelectNextDisplay(DISPLAY_FM);
						} else {
							GUI_SelectNextDisplay(DISPLAY_MAIN);
						}
					}
				}
			}
		}
	}

LAB_00004b08:
	if (!gPttIsPressed && g_20000373) {
		g_20000373--;
		if (g_20000373 == 0) {
			RADIO_SomethingElse(0);
			if (gCurrentFunction != FUNCTION_RECEIVE && gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_MONITOR && gFmRadioMode) {
				APP_StartFM();
				GUI_SelectNextDisplay(DISPLAY_FM);
			}
		}
	}

	if (gLowBattery) {
		gLowBatteryBlink = ++gLowBatteryCountdown & 1;
		UI_DisplayBattery(gLowBatteryCountdown);
		if (gCurrentFunction != FUNCTION_TRANSMIT) {
			if (gLowBatteryCountdown < 30) {
				if (gLowBatteryCountdown == 29 && !gChargingWithTypeC) {
					AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP);
				}
			} else {
				gLowBatteryCountdown = 0;
				if (!gChargingWithTypeC) {
					AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP);
					AUDIO_SetVoiceID(0, VOICE_ID_LOW_VOLTAGE);
					if (gBatteryDisplayLevel == 0) {
						AUDIO_PlaySingleVoice(true);
						gReducedService = true;
						FUNCTION_Select(FUNCTION_POWER_SAVE);
						ST7565_Configure_GPIO_B11();
						GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
					} else {
						AUDIO_PlaySingleVoice(false);
					}
				}
			}
		}
	}

	if (gScreenToDisplay == DISPLAY_SCANNER && gScannerEditState == 0 && gScanState < 2) {
		g_20000464++;
		if (0x20 < g_20000464) {
			if (gScanState == 1 && g_20000458 == 0) {
				gScanState = 2;
			} else {
				gScanState = 3;
			}
		}
		gUpdateDisplay = true;
	}

	if (gDTMF_CallState != DTMF_CALL_STATE_NONE && gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_RECEIVE) {
		if (gDTMF_AUTO_RESET_TIME) {
			gDTMF_AUTO_RESET_TIME--;
			if (gDTMF_AUTO_RESET_TIME == 0) {
				gDTMF_CallState = DTMF_CALL_STATE_NONE;
				gUpdateDisplay = true;
			}
		}
		if (gDTMF_DecodeRing && gDTMF_DecodeRingCountdown) {
			gDTMF_DecodeRingCountdown--;
			if ((gDTMF_DecodeRingCountdown % 3) == 0) {
				AUDIO_PlayBeep(BEEP_440HZ_500MS);
			}
			if (gDTMF_DecodeRingCountdown == 0) {
				gDTMF_DecodeRing = false;
			}
		}
	}

	if (g_200003BD && g_200003C3) {
		g_200003C3--;
		if (g_200003C3 == 0) {
			g_200003BD = 0;
			gUpdateDisplay = true;
		}
	}

	if (gDTMF_RecvTimeout) {
		gDTMF_RecvTimeout--;
		if (gDTMF_RecvTimeout == 0) {
			gDTMF_WriteIndex = 0;
			memset(gDTMF_Received, 0, sizeof(gDTMF_Received));
		}
	}
}

void FUN_00001150(void)
{
	g_20000383 = 0;
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	gEnableSpeaker = false;
	if (gEeprom.ALARM_MODE == ALARM_MODE_TONE) {
		RADIO_SendEndOfTransmission();
		RADIO_EnableCxCSS();
	}
	g_200003B6 = 0x50;
	SYSTEM_DelayMs(5);
	RADIO_SetupRegisters(true);
	gRequestDisplayScreen = DISPLAY_MAIN;
}

void FUN_000075b0(void)
{
	uint8_t StepSetting;
	uint16_t StepFrequency;

	BK4819_StopScan();
	RADIO_ConfigureTX();

	if (IS_NOAA_CHANNEL(gRxInfo->CHANNEL_SAVE)) {
		gRxInfo->CHANNEL_SAVE = FREQ_CHANNEL_FIRST + 5;
	}
	StepSetting = gRxInfo->STEP_SETTING;
	StepFrequency = gRxInfo->StepFrequency;
	RADIO_InitInfo(gRxInfo, gRxInfo->CHANNEL_SAVE, gRxInfo->Band, gRxInfo->pCurrent->Frequency);

	gRxInfo->STEP_SETTING = StepSetting;
	gRxInfo->StepFrequency = StepFrequency;
	RADIO_SetupRegisters(true);

	gIsNoaaMode = false;
	if (g_20000458 == 1) {
		gScanState = 1;
		gScanFrequency = gRxInfo->pCurrent->Frequency;
		gStepSetting = gRxInfo->STEP_SETTING;
		BK4819_PickRXFilterPathBasedOnFrequency(gScanFrequency);
		BK4819_SetScanFrequency(gScanFrequency);
	} else {
		gScanState = 0;
		gScanFrequency = 0xFFFFFFFF;
		BK4819_PickRXFilterPathBasedOnFrequency(0xFFFFFFFF);
		BK4819_EnableFrequencyScan();
	}
	g_2000045D = 0x15;
	gCS_ScannedIndex = 0xFF;
	gCS_ScannedType = 0xFF;
	g_2000045F = 0;
	g_2000045C = 0;
	gDTMF_RequestPending = false;
	g_CxCSS_TAIL_Found = false;
	g_CDCSS_Lost = false;
	gCDCSSCodeType = 0;
	g_CTCSS_Lost = false;
	g_VOX_Lost = false;
	g_SquelchLost = false;
	gScannerEditState = 0;
	g_20000464 = 0;
}

void APP_ChangeStepDirectionMaybe(bool bFlag, int8_t Direction)
{
	RADIO_ConfigureTX();
	g_20000410 = gRxInfo->CHANNEL_SAVE;
	g_20000415 = 0;
	gStepDirection = Direction;
	if (IS_MR_CHANNEL(g_20000410)) {
		if (bFlag) {
			g_20000414 = g_20000410;
		}
		FUN_00007dd4();
	} else {
		if (bFlag) {
			g_20000418 = gRxInfo->ConfigRX.Frequency;
		}
		APP_MoreRadioStuff();
	}
	ScanPauseDelayIn10msec = 50;
	gSystickFlag9 = false;
	g_20000411 = 0;
	gScanPauseMode = 0;
	g_20000413 = 0;
}

void APP_FlipVoxSwitch(void)
{
	gEeprom.VOX_SWITCH = !gEeprom.VOX_SWITCH;
	gRequestSaveSettings = true;
	g_20000398 = 1;
	gAnotherVoiceID = VOICE_ID_VOX;
	gUpdateStatus = true;
}

void APP_CycleOutputPower(void)
{
	if (++gTxInfo->OUTPUT_POWER > OUTPUT_POWER_HIGH) {
		gTxInfo->OUTPUT_POWER = OUTPUT_POWER_LOW;
	}

	gRequestSaveChannel = 1;
	gAnotherVoiceID = VOICE_ID_POWER;
	gRequestDisplayScreen = gScreenToDisplay;
}

void APP_StartScan(bool bFlag)
{
	if (gFmRadioMode) {
		if (gCurrentFunction != FUNCTION_RECEIVE && gCurrentFunction != FUNCTION_MONITOR && gCurrentFunction != FUNCTION_TRANSMIT) {
			uint16_t Frequency;

			GUI_SelectNextDisplay(DISPLAY_FM);
			if (gFM_Step) {
				FM_Play();
				gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
				return;
			}
			if (bFlag) {
				gFM_AutoScan = true;
				gFM_ChannelPosition = 0;
				FM_EraseChannels();
				Frequency = gEeprom.FM_LowerLimit;
			} else {
				gFM_AutoScan = false;
				gFM_ChannelPosition = 0;
				Frequency = gEeprom.FM_FrequencyPlaying;
			}
			BK1080_GetFrequencyDeviation(Frequency);
			FM_Tune(Frequency, 1, bFlag);
			gAnotherVoiceID = VOICE_ID_SCANNING_BEGIN;
			return;
		}
	} else if (gScreenToDisplay != DISPLAY_SCANNER) {
		RADIO_ConfigureTX();
		if (IS_NOT_NOAA_CHANNEL(gRxInfo->CHANNEL_SAVE)) {
			GUI_SelectNextDisplay(DISPLAY_MAIN);
			if (gStepDirection) {
				FUN_0000773c();
				gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
				return;
			}
			APP_ChangeStepDirectionMaybe(true, 1);
			AUDIO_SetVoiceID(0, VOICE_ID_SCANNING_BEGIN);
			AUDIO_PlaySingleVoice(true);
		}
	}
}

void FUN_00005770(void)
{
	if (gCurrentFunction != FUNCTION_MONITOR) {
		RADIO_ConfigureTX();
		if (gRxInfo->CHANNEL_SAVE >= NOAA_CHANNEL_FIRST && gIsNoaaMode) {
			gNoaaChannel = gRxInfo->CHANNEL_SAVE - NOAA_CHANNEL_FIRST;
		}
		RADIO_SetupRegisters(true);
		APP_StartListening(FUNCTION_MONITOR);
		return;
	}
	if (gStepDirection) {
		ScanPauseDelayIn10msec = 500;
		gSystickFlag9 = false;
		gScanPauseMode = 1;
	}
	if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF && gIsNoaaMode) {
		gNOAA_Countdown = 500;
		gScheduleNOAA = false;
	}
	RADIO_SetupRegisters(true);
	if (gFmRadioMode) {
		APP_StartFM();
		gRequestDisplayScreen = DISPLAY_FM;
	} else {
		gRequestDisplayScreen = gScreenToDisplay;
	}
}

void APP_SwitchToFM(void)
{
	if (gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_MONITOR) {
		if (gFmRadioMode) {
			FM_TurnOff();
			gInputBoxIndex = 0;
			g_200003B6 = 0x50;
			g_20000398 = 1;
			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}
		RADIO_ConfigureTX();
		RADIO_SetupRegisters(true);
		APP_StartFM();
		gInputBoxIndex = 0;
		gRequestDisplayScreen = DISPLAY_FM;
	}
}

void FUN_000056a0(bool bFlag)
{
	gInputBoxIndex = 0;
	if (bFlag) {
		g_20000383 = 3;
	} else {
		g_20000383 = 1;
	}
	g_20000422 = 0;
	g_200003A0 = 1;
	gRequestDisplayScreen = DISPLAY_MAIN;
}

void APP_ProcessFlashLight(void)
{
	switch (gFlashLightState) {
	case 0:
		gFlashLightState++;
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT);
		break;
	case 1:
		gFlashLightState++;
		break;
	default:
		gFlashLightState = 0;
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT);
	}
}

void FUN_00004404(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint8_t Short;
	uint8_t Long;

	if (gScreenToDisplay == DISPLAY_MAIN && gDTMF_InputMode) {
		if (Key == KEY_SIDE1 && !bKeyHeld && bKeyPressed) {
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			if (gDTMF_InputIndex) {
				gDTMF_InputIndex--;
				gDTMF_InputBox[gDTMF_InputIndex] = '-';
				if (gDTMF_InputIndex) {
					g_20000394 = true;
					gRequestDisplayScreen = DISPLAY_MAIN;
					return;
				}
			}
			gAnotherVoiceID = VOICE_ID_CANCEL;
			gRequestDisplayScreen = DISPLAY_MAIN;
			gDTMF_InputMode = false;
		}
		g_20000394 = true;
		return;
	}

	if (Key == KEY_SIDE1) {
		Short = gEeprom.KEY_1_SHORT_PRESS_ACTION;
		Long = gEeprom.KEY_1_LONG_PRESS_ACTION;
	} else {
		Short = gEeprom.KEY_2_SHORT_PRESS_ACTION;
		Long = gEeprom.KEY_2_LONG_PRESS_ACTION;
	}
	if (!bKeyHeld && bKeyPressed) {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		return;
	}
	if (bKeyHeld || bKeyPressed) {
		if (!bKeyHeld) {
			return;
		}
		Short = Long;
		if (!bKeyPressed) {
			return;
		}
	}
	switch (Short) {
	case 1:
		APP_ProcessFlashLight();
		break;
	case 2:
		APP_CycleOutputPower();
		break;
	case 3:
		FUN_00005770();
		break;
	case 4:
		APP_StartScan(true);
		break;
	case 5:
		APP_FlipVoxSwitch();
		break;
	case 6:
		FUN_000056a0(0);
		break;
	case 7:
		APP_SwitchToFM();
		break;
	case 8:
		FUN_000056a0(1);
		break;
	}
}

static void APP_ProcessKey_MAIN(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	if (gFmRadioMode && Key != KEY_PTT && Key != KEY_EXIT) {
		if (!bKeyHeld && bKeyPressed) {
			g_20000396 = 2;
		}
		return;
	}
	if (gDTMF_InputMode && !bKeyHeld && bKeyPressed) {
		char Character = DTMF_GetCharacter(Key);
		if (Character != 0xFF) {
			g_20000396 = 1;
			DTMF_Append(Character);
			gRequestDisplayScreen = DISPLAY_MAIN;
			g_20000394 = true;
			return;
		}
	}

	// TODO: ???
	if (KEY_PTT < Key) {
		Key = KEY_SIDE2;
	}

	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		MAIN_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		MAIN_Key_MENU(bKeyPressed, bKeyHeld);
		break;
	case KEY_UP:
		MAIN_Key_UP_DOWN(bKeyPressed, bKeyHeld, 1);
		break;
	case KEY_DOWN:
		MAIN_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
		break;
	case KEY_EXIT:
		MAIN_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	case KEY_STAR:
		MAIN_Key_STAR(bKeyPressed, bKeyHeld);
		break;
	case KEY_F:
		GENERIC_Key_F(bKeyPressed, bKeyHeld);
		break;
	case KEY_PTT:
		GENERIC_Key_PTT(bKeyPressed);
		break;
	default:
		if (!bKeyHeld && bKeyPressed) {
			g_20000396 = 2;
		}
		break;
	}
}

void APP_ProcessKey_MENU(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		MENU_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		MENU_Key_MENU(bKeyPressed, bKeyHeld);
		break;
	case KEY_UP:
		MENU_Key_UP_DOWN(bKeyPressed, bKeyHeld, 1);
		break;
	case KEY_DOWN:
		MENU_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
		break;
	case KEY_EXIT:
		MENU_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	case KEY_STAR:
		MENU_Key_STAR(bKeyPressed, bKeyHeld);
		break;
	case KEY_F:
		GENERIC_Key_F(bKeyPressed, bKeyHeld);
		break;
	case KEY_PTT:
		GENERIC_Key_PTT(bKeyPressed);
		break;
	default:
		if (!bKeyHeld && bKeyPressed) {
			g_20000396 = 2;
		}
		break;
	}
	if (gScreenToDisplay == DISPLAY_MENU && gMenuCursor == MENU_VOL) {
		g_20000393 = 0x20;
	}
}

static void APP_ProcessKey_FM(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		FM_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		FM_Key_MENU(bKeyPressed, bKeyHeld);
		return;
	case KEY_UP:
		FM_Key_UP_DOWN(bKeyPressed, bKeyHeld, 1);
		break;
	case KEY_DOWN:
		FM_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
		break;;
	case KEY_EXIT:
		FM_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	case KEY_F:
		GENERIC_Key_F(bKeyPressed, bKeyHeld);
		break;
	case KEY_PTT:
		GENERIC_Key_PTT(bKeyPressed);
		break;
	default:
		if (!bKeyHeld && bKeyPressed) {
			g_20000396 = 2;
		}
		break;
	}
}

static void APP_ProcessKey_SCANNER(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		SCANNER_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		SCANNER_Key_MENU(bKeyPressed, bKeyHeld);
		break;
	case KEY_UP:
		SCANNER_Key_UP_DOWN(bKeyPressed, bKeyHeld, 1);
		break;
	case KEY_DOWN:
		SCANNER_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
		break;
	case KEY_EXIT:
		SCANNER_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	case KEY_STAR:
		SCANNER_Key_STAR(bKeyPressed, bKeyHeld);
		break;
	case KEY_PTT:
		GENERIC_Key_PTT(bKeyPressed);
		break;
	default:
		if (!bKeyHeld && bKeyPressed) {
			g_20000396 = 2;
		}
		break;
	}
}

static void APP_ProcessKey_AIRCOPY(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		AIRCOPY_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
		break;
	case KEY_MENU:
		AIRCOPY_Key_MENU(bKeyPressed, bKeyHeld);
		break;
	case KEY_EXIT:
		AIRCOPY_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	default:
		break;
	}
}

static void APP_ProcessKey(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	bool bFlag;

	if (gCurrentFunction == FUNCTION_POWER_SAVE) {
		FUNCTION_Select(FUNCTION_0);
	}
	gBatterySaveCountdown = 1000;
	if (gEeprom.AUTO_KEYPAD_LOCK) {
		gKeyLockCountdown = 30;
	}
	if (!bKeyPressed) {
		if (gFlagSaveVfo) {
			SETTINGS_SaveVfoIndices();
			gFlagSaveVfo = false;
		}
		if (gFlagSaveSettings) {
			SETTINGS_SaveSettings();
			gFlagSaveSettings = false;
		}
		if (gFlagSaveFM) {
			SETTINGS_SaveFM();
			gFlagSaveFM = false;
		}
		if (gFlagSaveChannel) {
			SETTINGS_SaveChannel(
				gTxInfo->CHANNEL_SAVE,
				gEeprom.TX_CHANNEL,
				gTxInfo,
				gFlagSaveChannel);
			gFlagSaveChannel = false;
			RADIO_ConfigureChannel(gEeprom.TX_CHANNEL, 1);
			RADIO_SetupRegisters(true);
			GUI_SelectNextDisplay(DISPLAY_MAIN);
		}
	} else {
		if (Key != KEY_PTT) {
			g_20000393 = 0x10;
		}
		BACKLIGHT_TurnOn();
		if (gDTMF_DecodeRing) {
			gDTMF_DecodeRing = false;
			AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
			if (Key != KEY_PTT) {
				g_20000394 = true;
				return;
			}
		}
	}

	if (gEeprom.KEY_LOCK && gCurrentFunction != FUNCTION_TRANSMIT && Key != KEY_PTT) {
		if (Key == KEY_F) {
			if (!bKeyHeld) {
				if (!bKeyPressed) {
					return;
				}
				if (bKeyHeld) {
					return;
				}
				AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
				gKeypadLocked = 4;
				gUpdateDisplay = true;
				return;
			}
			if (!bKeyPressed) {
				return;
			}
		} else if (Key != KEY_SIDE1 && Key != KEY_SIDE2) {
			if (!bKeyPressed) {
				return;
			}
			if (bKeyHeld) {
				return;
			}
			AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
			gKeypadLocked = 4;
			gUpdateDisplay = true;
			return;
		}
	}

	if ((gStepDirection && Key != KEY_PTT && Key != KEY_UP && Key != KEY_DOWN && Key != KEY_EXIT && Key != KEY_STAR) ||
	    (g_20000381 && Key != KEY_PTT && Key != KEY_UP && Key != KEY_DOWN && Key != KEY_EXIT && Key != KEY_STAR && Key != KEY_MENU)) {
		if (!bKeyPressed || bKeyHeld) {
			return;
		}
		AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
		return;
	}

	bFlag = false;

	if (g_20000395 == 1 && Key == KEY_PTT) {
		bFlag = bKeyHeld;
		if (!bKeyPressed) {
			bFlag = true;
			g_20000395 = 0;
		}
	}

	if (g_20000394 && Key != KEY_PTT) {
		if (bKeyHeld) {
			bFlag = true;
		}
		if (!bKeyPressed) {
			bFlag = true;
			g_20000394 = 0;
		}
	}

	if (gWasFKeyPressed && KEY_9 < Key && Key != KEY_F && Key != KEY_STAR) {
		gWasFKeyPressed = false;
		gUpdateStatus = true;
	}

	if (gF_LOCK) {
		if (Key == KEY_PTT) {
			return;
		}
		if (Key == KEY_SIDE2) {
			return;
		}
		if (Key == KEY_SIDE1) {
			return;
		}
	}

	if (!bFlag) {
		if (gCurrentFunction == FUNCTION_TRANSMIT) {
			if (g_20000383 == 0) {
				if (Key == KEY_PTT) {
					GENERIC_Key_PTT(bKeyPressed);
				} else {
					char Code;

					if (Key == KEY_SIDE2) {
						Code = 0xFE;
					} else {
						Code = DTMF_GetCharacter(Key);
						if (Code == 0xFF) {
							goto Skip;
						}
					}

					if (bKeyHeld || !bKeyPressed) {
						if (!bKeyPressed) {
							GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
							gEnableSpeaker = false;
							BK4819_ExitDTMF_TX(false);
							if (gCrossTxRadioInfo->SCRAMBLING_TYPE == 0 || !gSetting_ScrambleEnable) {
								BK4819_DisableScramble();
							} else {
								BK4819_EnableScramble(gCrossTxRadioInfo->SCRAMBLING_TYPE - 1);
							}
						}
					} else {
						if (gEeprom.DTMF_SIDE_TONE) {
							GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
							gEnableSpeaker = true;
						}
						BK4819_DisableScramble();
						if (Code == 0xFE) {
							BK4819_TransmitTone(gEeprom.DTMF_SIDE_TONE, 1750);
						} else {
							BK4819_PlayDTMFEx(gEeprom.DTMF_SIDE_TONE, Code);
						}
					}
				}
			} else if (!bKeyHeld && bKeyPressed) {
				FUN_00001150();
				if (gEeprom.REPEATER_TAIL_TONE_ELIMINATION == 0) {
					FUNCTION_Select(FUNCTION_0);
				} else {
					gRTTECountdown = gEeprom.REPEATER_TAIL_TONE_ELIMINATION * 10;
				}
				if (Key == KEY_PTT) {
					g_20000395 = 1;
				} else {
					g_20000394 = 1;
				}
			}
		} else if (Key != KEY_SIDE1 && Key != KEY_SIDE2) {
			switch (gScreenToDisplay) {
			case DISPLAY_MAIN:
				APP_ProcessKey_MAIN(Key, bKeyPressed, bKeyHeld);
				break;
			case DISPLAY_FM:
				APP_ProcessKey_FM(Key, bKeyPressed, bKeyHeld);
				break;
			case DISPLAY_MENU:
				APP_ProcessKey_MENU(Key, bKeyPressed, bKeyHeld);
				break;
			case DISPLAY_SCANNER:
				APP_ProcessKey_SCANNER(Key, bKeyPressed, bKeyHeld);
				break;
			case DISPLAY_AIRCOPY:
				APP_ProcessKey_AIRCOPY(Key, bKeyPressed, bKeyHeld);
				break;
			default:
				break;
			}
		} else if (gScreenToDisplay != DISPLAY_SCANNER && gScreenToDisplay != DISPLAY_AIRCOPY) {
			FUN_00004404(Key, bKeyPressed, bKeyHeld);
		} else if (!bKeyHeld && bKeyPressed) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
	}

Skip:
	if (gBeepToPlay) {
		AUDIO_PlayBeep(gBeepToPlay);
		gBeepToPlay = 0;
	}

	if (gFlagAcceptSetting) {
		MENU_AcceptSetting();
		gFlagRefreshSetting = true;
		gFlagAcceptSetting = false;
	}
	if (gFlagStopScan) {
		BK4819_StopScan();
		gFlagStopScan = false;
	}
	if (gRequestSaveSettings) {
		if (bKeyHeld == 0) {
			SETTINGS_SaveSettings();
		} else {
			gFlagSaveSettings = 1;
		}
		gRequestSaveSettings = false;
		gUpdateStatus = true;
	}
	if (gRequestSaveFM) {
		if (!bKeyHeld) {
			SETTINGS_SaveFM();
		} else {
			gFlagSaveFM = true;
		}
		gRequestSaveFM = false;
	}
	if (gRequestSaveVFO) {
		if (!bKeyHeld) {
			SETTINGS_SaveVfoIndices();
		} else {
			gFlagSaveVfo = true;
		}
		gRequestSaveVFO = false;
	}
	if (gRequestSaveChannel) {
		if (!bKeyHeld) {
			SETTINGS_SaveChannel(gTxInfo->CHANNEL_SAVE, gEeprom.TX_CHANNEL, gTxInfo, gRequestSaveChannel);
			if (gScreenToDisplay != DISPLAY_SCANNER) {
				gVfoConfigureMode = VFO_CONFIGURE_1;
			}
		} else {
			gFlagSaveChannel = gRequestSaveChannel;
			if (gRequestDisplayScreen == DISPLAY_INVALID) {
				gRequestDisplayScreen = DISPLAY_MAIN;
			}
		}
		gRequestSaveChannel = 0;
	}

	if (gVfoConfigureMode == VFO_CONFIGURE_0) {
		if (g_20000398 == 0) {
			goto LAB_00002aae;
		}
	} else {
		if (g_2000039B == 1) {
			RADIO_ConfigureChannel(0, gVfoConfigureMode);
			RADIO_ConfigureChannel(1, gVfoConfigureMode);
		} else {
			RADIO_ConfigureChannel(gEeprom.TX_CHANNEL, gVfoConfigureMode);
		}
		if (gRequestDisplayScreen == DISPLAY_INVALID) {
			gRequestDisplayScreen = DISPLAY_MAIN;
		}
		g_20000398 = 1;
		gVfoConfigureMode = VFO_CONFIGURE_0;
		g_2000039B = 0;
	}
	RADIO_ConfigureTX();
	RADIO_ConfigureNOAA();
	RADIO_SetupRegisters(true);
	gDTMF_AUTO_RESET_TIME = 0;
	gDTMF_CallState = DTMF_CALL_STATE_NONE;
	g_200003C3 = 0;
	g_200003BD = 0;
	gVFO_RSSI_Level[0] = 0;
	gVFO_RSSI_Level[1] = 0;
	g_20000398 = 0;

LAB_00002aae:
	if (gFlagRefreshSetting) {
		MENU_ShowCurrentSetting();
		gFlagRefreshSetting = false;
	}
	if (gFlagStartScan) {
		AUDIO_SetVoiceID(0, VOICE_ID_SCANNING_BEGIN);
		AUDIO_PlaySingleVoice(true);
		FUN_000075b0();
		gRequestDisplayScreen = DISPLAY_SCANNER;
		gFlagStartScan = false;
	}
	if (g_200003A0 == 1) {
		RADIO_SomethingWithTransmit();
		g_200003A0 = 0;
	}
	if (gAnotherVoiceID != VOICE_ID_INVALID) {
		if (gAnotherVoiceID < 76) {
			AUDIO_SetVoiceID(0, gAnotherVoiceID);
		}
		AUDIO_PlaySingleVoice(false);
		gAnotherVoiceID = VOICE_ID_INVALID;
	}
	GUI_SelectNextDisplay(gRequestDisplayScreen);
	gRequestDisplayScreen = DISPLAY_INVALID;
}

