#include <string.h>
#include "aircopy.h"
#include "app.h"
#include "audio.h"
#include "battery.h"
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
#include "fm.h"
#include "frequencies.h"
#include "functions.h"
#include "gui.h"
#include "helper.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

static void APP_ProcessKey(KEY_Code_t CurrentKey, bool bKeyPressed, bool bKeyHeld);
void FUN_000069f8(FUNCTION_Type_t Function);

static void FUN_00005144(void)
{
	if (!g_SquelchLost) {
		return;
	}
	if (gCurrentStep == 0) {
		if (g_20000381 != 0 && g_20000411 == 0) {
			ScanPauseDelayIn10msec = 100;
			gSystickFlag9 = false;
			g_20000411 = 1;
		}
		if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF) {
			if (gIsNoaaMode) {
				g_20000356 = 0x1e;
				gSystickFlag8 = false;
			}
			FUNCTION_Select(FUNCTION_3);
			return;
		}
		if (g_20000411 != 0) {
			FUNCTION_Select(FUNCTION_3);
			return;
		}
		g_2000033A = 100;
		gSystickFlag7 = false;
	} else {
		if (g_20000411 != 0) {
			FUNCTION_Select(FUNCTION_3);
			return;
		}
		ScanPauseDelayIn10msec = 0x1e;
		gSystickFlag9 = false;
	}
	g_20000411 = 1;
	FUNCTION_Select(FUNCTION_3);
}

void FUN_00007fd0(void)
{
	char String[20];
	uint8_t Offset;

	if (!g_200003AA) {
		return;
	}

	g_200003AA = 0;

	if (gCurrentStep || g_20000381) {
		return;
	}

	if (!gInfoCHAN_A->DTMF_DECODING_ENABLE && !gSetting_KILLED) {
		return;
	}

	if (8 < gDTMF_WriteIndex) {
		Offset = gDTMF_WriteIndex - 9;
		sprintf(String, "%s%c%s", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE, gEeprom.KILL_CODE);
		if (DTMF_CompareMessage(gDTMF_Received + Offset, String, 9, true)) {
			if (gEeprom.PERMIT_REMOTE_KILL) {
				gSetting_KILLED = true;
				//SETTINGS_SaveSettings();
				g_200003BE = 2;
				if (gFmMute == true) {
					FM_TurnOff();
					GUI_SelectNextDisplay(DISPLAY_MAIN);
				}
			} else {
				g_200003BE = 0;
			}
			g_200003BC = 0;
			gUpdateDisplay = true;
			g_2000036F = true;
			return;
		} else {
			sprintf(String, "%s%c%s", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE, gEeprom.REVIVE_CODE);
			if (DTMF_CompareMessage(gDTMF_Received + Offset, String, 9, true)) {
				gSetting_KILLED = false;
				//SETTINGS_SaveSettings();
				g_200003BE = 2;
				g_200003BC = 0;
				gUpdateDisplay = true;
				g_2000036F = true;
				return;
			}
		}
	}

	if (1 < gDTMF_WriteIndex) {
		if (DTMF_CompareMessage(gDTMF_Received + gDTMF_WriteIndex - 2, "AB", 2, true)) {
			g_CalloutAndDTMF_State = 1;
			gUpdateDisplay = true;
			return;
		}
	}
	if (g_200003BC == 1 && g_20000438 == 0 && 8 < gDTMF_WriteIndex) {
		Offset = gDTMF_WriteIndex - 9;
		sprintf(String, "%s%c%s", gDTMF_String, gEeprom.DTMF_SEPARATE_CODE, "AAAAA");
		if (DTMF_CompareMessage(gDTMF_Received + Offset, String, 9, false)) {
			g_CalloutAndDTMF_State = 2;
			gUpdateDisplay = true;
		}
	}
	if (gSetting_KILLED == true) {
		return;
	}
	if (g_200003BC != 0) {
		return;
	}
	if (gDTMF_WriteIndex < 7) {
		return;
	}
	Offset = gDTMF_WriteIndex - 7;
	sprintf(String, "%s%c", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE);
	g_20000439 = false;
	if (!DTMF_CompareMessage(gDTMF_Received + Offset, String, 4, true)) {
		return;
	}
	g_200003BC = 2;
	memcpy(gDTMF_Contact1, gDTMF_Received + Offset, 3);
	memcpy(gDTMF_Contact0, gDTMF_Received + Offset + 4, 3);

	gUpdateDisplay = true;

	switch (gEeprom.DTMF_DECODE_RESPONSE) {
	case 3:
		g_200003C1 = true;
		g_200003C4 = 20;
		// Fallthrough
	case 2:
		g_200003BE = 3;
		break;
	case 1:
		g_200003C1 = true;
		g_200003C4 = 20;
		break;
	default:
		g_200003C1 = false;
		g_200003BE = 0;
		break;
	}

	if (g_20000439) {
		g_200003BE = 0;
	}
}

void FUN_000051e8(void)
{
	bool bFlag;

	if (!g_SquelchLost) {
		FUNCTION_Select(FUNCTION_0);
		gUpdateDisplay = true;
		return;
	}

	bFlag = (gCurrentStep == 0 || gCopyOfCodeType == CODE_TYPE_OFF);
	if (gInfoCHAN_A->CHANNEL_SAVE >= 207 && gSystickCountdown2 != 0) {
		bFlag = true;
		gSystickCountdown2 = 0;
	}
	if (g_CTCSS_Lost && gCopyOfCodeType == CODE_TYPE_CONTINUOUS_TONE) {
		bFlag = true;
		g_20000375 = 0;
	}
	if (g_CDCSS_Lost && gCDCSSCodeReceived == 0x01) {
		if (gCopyOfCodeType == CODE_TYPE_DIGITAL || gCopyOfCodeType == CODE_TYPE_REVERSE_DIGITAL) {
			g_20000376 = 0;
		}
	} else {
		if (!bFlag) {
			return;
		}
	}
	FUN_00007fd0();
	if (gCurrentStep == 0 && g_20000381 == 0) {
		if (gInfoCHAN_A->DTMF_DECODING_ENABLE == true || gSetting_KILLED == true) {
			if (g_200003BC == 0) {
				if (g_20000411 == 0x01) {
					g_2000033A = 500;
					gSystickFlag7 = false;
					g_20000411 = 2;
					return;
				}
			}
		}
	}
	FUN_000069f8(FUNCTION_4);
}

void FUN_000052f0(void)
{
	uint8_t Value;

	Value = 0;
	if (gSystickFlag10) {
		Value = 1;
		goto LAB_0000544c;
	}

	if (gCurrentStep != 0 && (g_20000410 - 200) < 7) {
		if (g_SquelchLost) {
			return;
		}
		Value = 1;
		goto LAB_0000544c;
	}

	switch (gCopyOfCodeType) {
	case CODE_TYPE_CONTINUOUS_TONE:
		if (g_20000375 == 1) {
			if (gSystickCountdown4 == 0) {
				g_20000375 = 0;
				g_20000376 = 0;
				Value = 1;
				goto LAB_0000544c;
			}
		}
		break;
	case CODE_TYPE_DIGITAL:
	case CODE_TYPE_REVERSE_DIGITAL:
		if (g_20000376 == 1) {
			if (gSystickCountdown3 == 0) {
				g_20000375 = 0;
				g_20000376 = 0;
				Value = 1;
				goto LAB_0000544c;
			}
		}
		break;
	default:
		break;
	}
	if (g_SquelchLost) {
		if (g_20000377 == 0 && gInfoCHAN_A->CHANNEL_SAVE < 207) {
			switch (gCopyOfCodeType) {
			case CODE_TYPE_CONTINUOUS_TONE:
				if (g_CTCSS_Lost) {
					g_20000375 = 0;
				} else if (g_20000375 == 0) {
					g_20000375 = 1;
					gSystickCountdown4 = 100;
				}
				if (g_CxCSS_TAIL_Found) {
					Value = 2;
					g_CxCSS_TAIL_Found = false;
				}
				break;
			case CODE_TYPE_OFF:
				if (gEeprom.SQUELCH_LEVEL != 0) {
					if (g_CxCSS_TAIL_Found) {
						Value = 2;
						g_CxCSS_TAIL_Found = false;
					}
				}
				break;
			case CODE_TYPE_DIGITAL:
			case CODE_TYPE_REVERSE_DIGITAL:
				if (g_CDCSS_Lost && gCDCSSCodeReceived == 1) {
					g_20000376 = 0;
				} else if (g_20000376 == 0) {
					g_20000376 = 1;
					gSystickCountdown3 = 100;
				}
				if (g_CxCSS_TAIL_Found) {
					if (BK4819_GetCTCSSPhaseShift() == 1) {
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

	if (g_20000377 == 0 && Value == 0 && gNextTimeslice40ms && gEeprom.TAIL_NOTE_ELIMINATION) {
		switch (gCopyOfCodeType) {
		case CODE_TYPE_DIGITAL:
		case CODE_TYPE_REVERSE_DIGITAL:
			if (BK4819_GetCTCSSPhaseShift() == 1) {
				gNextTimeslice40ms = false;
			}
			break;
		default:
			break;
		}
	}

	gNextTimeslice40ms = false;

LAB_0000544c:
	switch (Value) {
	case 1:
		RADIO_SetupRegisters(true);
		if (206 < gInfoCHAN_A->CHANNEL_SAVE) {
			gSystickCountdown2 = 300;
		}
		gUpdateDisplay = true;
		if (gCurrentStep) {
			switch (gEeprom.SCAN_RESUME_MODE) {
			case SCAN_RESUME_CO:
				ScanPauseDelayIn10msec = 360;
				gSystickFlag9 = false;
				break;
			case SCAN_RESUME_SE:
				//FUN_0000773c();
				break;
			}
		}
		break;
	case 2:
		if (gEeprom.TAIL_NOTE_ELIMINATION != true) {
			gNextTimeslice40ms = false;
			break;
		}
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		g_20000342 = 20;
		gSystickFlag10 = false;
		g_2000036B = 0;
		g_20000377 = 1;
		break;
	default:
		break;
	}
}

void FUN_0000510c(void)
{
	switch (gCurrentFunction) {
	case FUNCTION_0:
		FUN_00005144();
		break;;
	case FUNCTION_POWER_SAVE:
		if (!gThisCanEnable_BK4819_Rxon) {
			FUN_00005144();
		}
		break;
	case FUNCTION_3:
		FUN_000051e8();
		break;
	case FUNCTION_4:
		FUN_000052f0();
		break;
	default:
		break;
	}
}

void FUN_000069f8(FUNCTION_Type_t Function)
{
	if (!gSetting_KILLED) {
		if (gFmMute) {
			BK1080_Init(0, false);
		}
		gVFO_RSSI_Level[gEeprom.RX_CHANNEL == 0] = 0;
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		g_2000036B = 1;
		BACKLIGHT_TurnOn();
		if (gCurrentStep != 0) {
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
		if (206 < gInfoCHAN_A->CHANNEL_SAVE && gIsNoaaMode) {
			gInfoCHAN_A->CHANNEL_SAVE = gNoaaChannel + 207;
			gInfoCHAN_A->pDCS_Current->Frequency = NoaaFrequencyTable[gNoaaChannel];
			gInfoCHAN_A->pDCS_Reverse->Frequency = NoaaFrequencyTable[gNoaaChannel];
			gEeprom.VfoChannel[gEeprom.RX_CHANNEL] = gInfoCHAN_A->CHANNEL_SAVE;
			g_20000356 = 500;
			gSystickFlag8 = false;
		}
		if (g_20000381 != 0) {
			g_20000381 = 2;
		}
		if (gCurrentStep == 0 && g_20000381 == 0 && gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
			g_2000041F = 1;
			g_2000033A = 360;
			gSystickFlag7 = false;
		}
		if (gInfoCHAN_A->IsAM) {
			BK4819_WriteRegister(BK4819_REG_48, 0xB3A8);
			g_20000474 = 0;
		} else {
			BK4819_WriteRegister(BK4819_REG_48, 0xB000
					| (gEeprom.VOLUME_GAIN << 4)
					| (gEeprom.DAC_GAIN << 0)
					);
		}
		if (gVoiceWriteIndex == 0) {
			if (gInfoCHAN_A->IsAM == true) {
				BK4819_SetAF(BK4819_AF_AM);
			} else {
				BK4819_SetAF(BK4819_AF_OPEN);
			}
		}
		FUNCTION_Select(Function);
		if (Function == FUNCTION_2 || gFmMute) {
			GUI_SelectNextDisplay(DISPLAY_MAIN);
			return;
		}
		gUpdateDisplay = true;
	}
}

void APP_AddStepToFrequency(VFO_Info_t *pInfo, uint8_t Step)
{
	uint32_t Frequency;

	Frequency = pInfo->DCS[0].Frequency + (Step * pInfo->StepFrequency);
	if (Frequency >= gLowerLimitFrequencyBandTable[pInfo->Band] && Frequency <= gUpperLimitFrequencyBandTable[pInfo->Band]) {
		Frequency = FREQUENCY_FloorToStep(gUpperLimitFrequencyBandTable[pInfo->Band], pInfo->StepFrequency, Frequency);
	} else {
		Frequency = gLowerLimitFrequencyBandTable[pInfo->Band];
	}
	pInfo->DCS[0].Frequency = Frequency;
}

void APP_MoreRadioStuff(void)
{
	APP_AddStepToFrequency(gInfoCHAN_A, gCurrentStep);
	RADIO_ApplyOffset(gInfoCHAN_A);
	RADIO_ConfigureSquelchAndOutputPower(gInfoCHAN_A);
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

	Ch = RADIO_FindNextChannel(g_20000410 + gCurrentStep, gCurrentStep, true, gEeprom.SCAN_LIST_DEFAULT);
	if (Ch == 0xFF) {
		return;
	}

	g_20000410 = Ch;
	if (PreviousCh != g_20000410) {
		gEeprom.EEPROM_0E81_0E84[gEeprom.RX_CHANNEL] = g_20000410;
		gEeprom.VfoChannel[gEeprom.RX_CHANNEL] = g_20000410;
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

uint8_t AddWithRollover(uint8_t Base, uint8_t Add, uint8_t LowerLimit, uint8_t UpperLimit)
{
	Base += Add;
	if (Base == 0xFF || Base < LowerLimit) {
		return UpperLimit;
	}

	if (Base > UpperLimit) {
		return LowerLimit;
	}

	return Base;
}

void NOAA_IncreaseChannel(void)
{
	gNoaaChannel++;
	if (gNoaaChannel > 9) {
		gNoaaChannel = 0;
	}
}

void APP_DCS_Related(void)
{
	uint8_t UpperLimit;

	switch (gMenuCursor) {
	case 3: // R_DCS?
		UpperLimit = 0xD0;
		break;
	case 4: // R_CTCS?
		UpperLimit = 0x32;
		break;
	default:
		return;
	}

	gSubMenuSelection = AddWithRollover(gSubMenuSelection, gMenuScrollDirection, 1, UpperLimit);
	if (gMenuCursor == 3) {
		if (gSubMenuSelection > 104) {
			gCodeType = CODE_TYPE_REVERSE_DIGITAL;
			gCode = gSubMenuSelection - 105;
		} else {
			gCodeType = CODE_TYPE_DIGITAL;
			gCode = gSubMenuSelection - 1;
		}

	} else {
		gCodeType = CODE_TYPE_CONTINUOUS_TONE;
		gCode = gSubMenuSelection - 1;
	}

	RADIO_SetupRegisters(true);

	if (gCodeType == CODE_TYPE_CONTINUOUS_TONE) {
		ScanPauseDelayIn10msec = 20;
	} else {
		ScanPauseDelayIn10msec = 30;
	}

	gUpdateDisplay = true;
}

void FUN_00007f4c(void)
{
	if (gIsNoaaMode) {
		if (gEeprom.VfoChannel[0] < 207 || gEeprom.VfoChannel[1] < 207) {
			gEeprom.RX_CHANNEL = gEeprom.RX_CHANNEL == 0;
		} else {
			gEeprom.RX_CHANNEL = 0;
		}
		gInfoCHAN_A = &gEeprom.VfoInfo[gEeprom.RX_CHANNEL];
		if (gEeprom.VfoInfo[0].CHANNEL_SAVE >= 207) {
			NOAA_IncreaseChannel();
		}
	} else {
		gEeprom.RX_CHANNEL = gEeprom.RX_CHANNEL == 0;
		gInfoCHAN_A = &gEeprom.VfoInfo[gEeprom.RX_CHANNEL];
	}
	RADIO_SetupRegisters(false);
	if (gIsNoaaMode == true) {
		g_2000033A = 7;
	} else {
		g_2000033A = 10;
	}
}

static int FM_ChecksChannelValid_and_FrequencyDeviation(uint16_t Frequency, uint16_t LowerLimit)
{
	uint16_t SNR;
	int16_t Deviation;
	uint16_t RSSI;
	int ret = -1;

	SNR = BK1080_ReadRegister(BK1080_REG_07);
	// This cast fails to extend the sign because ReadReg is guaranteed to be U16.
	Deviation = (int16_t)SNR >> 4;
	if ((SNR & 0xF) < 2) {
		goto Bail;
	}

	RSSI = BK1080_ReadRegister(BK1080_REG_10);
	if (RSSI & 0x1000 || (RSSI & 0xFF) < 10) {
		goto Bail;
	}

	if (Deviation < 280 || Deviation > 3815) {
		if ((LowerLimit < Frequency) && (Frequency - g_20000362) == 1) {
			if ((gFM_FrequencyDeviation & 0x800) != 0) {
				goto Bail;
			}
			if (gFM_FrequencyDeviation < 20) {
				goto Bail;
			}
		}
		if ((LowerLimit <= Frequency) && (g_20000362 - Frequency) == 1) {
			if ((gFM_FrequencyDeviation & 0x800) == 0) {
				goto Bail;
			}
			if (4075 < gFM_FrequencyDeviation) {
				goto Bail;
			}
		}
		ret = 0;
	}

Bail:
	gFM_FrequencyDeviation = (uint16_t)Deviation;
	g_20000362 = Frequency;

	return ret;
}

void FUN_0000752c(uint16_t Frequency, uint8_t param_2, int param_3)
{
	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	g_2000036B = 0;
	if (g_20000390 == 0) {
		g_2000034C = 0x78;
	} else {
		g_2000034C = 10;
	}
	gSystickFlag11 = false;
	g_20000390 = param_2;
	g_20000427 = 0;
	gAskToSave = false;
	gAskToDelete = false;
	gEeprom.FM_FrequencyToPlay = Frequency;
	if (param_3 != 1) {
		Frequency += param_2;
		gEeprom.FM_FrequencyToPlay = gEeprom.FM_LowerLimit;
		if (Frequency <= gEeprom.FM_UpperLimit) {
			gEeprom.FM_FrequencyToPlay = Frequency;
			if (Frequency < gEeprom.FM_LowerLimit) {
				gEeprom.FM_FrequencyToPlay = gEeprom.FM_UpperLimit;
			}
		}
	}

	BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
}

void PlayFMRadio(void)
{
	g_20000390 = 0;
	if (gIs_A_Scan) {
		gEeprom.FM_IsChannelSelected = true;
		gEeprom.FM_CurrentChannel = 0;
	}
	FM_ConfigureChannelState();
	BK1080_SetFrequency(gEeprom.FM_FrequencyToPlay);
	//StoreFMSettingsToEeprom();
	g_2000034C = 0;
	gSystickFlag11 = false;
	gAskToSave = false;
	GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	g_2000036B = 1;
}

void APP_PlayFM(void)
{
	if (!FM_ChecksChannelValid_and_FrequencyDeviation(gEeprom.FM_FrequencyToPlay, gEeprom.FM_LowerLimit)) {
		if (gIs_A_Scan != 1) {
			g_2000034C = 0;
			g_20000427 = 1;
			if (gEeprom.FM_IsChannelSelected == false) {
				gEeprom.FM_CurrentFrequency = gEeprom.FM_FrequencyToPlay;
			}
			GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
			g_2000036B = 1;
		} else {
			if (gA_Scan_Channel < 20) {
				gFM_Channels[gA_Scan_Channel++] = gEeprom.FM_FrequencyToPlay;
				if (gEeprom.FM_UpperLimit > gEeprom.FM_FrequencyToPlay) {
					FUN_0000752c(gEeprom.FM_FrequencyToPlay, g_20000390, 0);
				} else {
					PlayFMRadio();
				}
			} else {
				PlayFMRadio();
			}
		}
		PlayFMRadio();
	} else if (gIs_A_Scan) {
		if (gEeprom.FM_UpperLimit > gEeprom.FM_FrequencyToPlay) {
			FUN_0000752c(gEeprom.FM_FrequencyToPlay, g_20000390, 0);
		} else {
			PlayFMRadio();
		}
	} else {
		FUN_0000752c(gEeprom.FM_FrequencyToPlay, g_20000390, 0);
	}

	GUI_SelectNextDisplay(DISPLAY_FM);
}

void FUN_000059b4(void)
{
        gFmMute = true;
        g_20000390 = 0;
        g_2000038E = 0;
        BK1080_Init(gEeprom.FM_FrequencyToPlay, true);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
        g_2000036B = 1;
        g_2000036F = 1;
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
			g_200003AA = 1;
			g_20000442 = 5;
			if (15 < gDTMF_WriteIndex) {
				uint8_t i;
				for (i = 0; i < sizeof(gDTMF_Received) - 1; i++) {
					gDTMF_Received[i] = gDTMF_Received[i + 1];
				}
				gDTMF_WriteIndex = 15;
			}
			gDTMF_Received[gDTMF_WriteIndex++] = DTMF_GetCharacter(BK4819_GetDTMF_5TONE_Code());
			if (gCurrentFunction == FUNCTION_4) {
				//FUN_00007fd0();
			}
		}
		if (Mask & BK4819_REG_02_CxCSS_TAIL) {
			g_CxCSS_TAIL_Found = true;
		}
		if (Mask & BK4819_REG_02_CDCSS_LOST) {
			g_CDCSS_Lost = true;
			gCDCSSCodeReceived = BK4819_CheckCDCSSCodeReceived();
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
			if (gEeprom.VOX_SWITCH == true) {
				if (gCurrentFunction == FUNCTION_POWER_SAVE && gThisCanEnable_BK4819_Rxon == false) {
					gBatterySave = 20;
					gBatterySaveCountdownExpired = 0;
				}
				if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF && (gSystickFlag7 || (gSystickFlag7 == false && g_2000033A < 20))) {
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
				//SomeFSKData[gFSKWriteIndex++] = BK4819_GetRegister(BK4819_REG_5F);
			}
			//AIRCOPY_StorePacket();
		}
	}
}

void APP_Update(void)
{
	if (gFlagPlayQueuedVoice) {
		AUDIO_PlayQueuedVoice();
		gFlagPlayQueuedVoice = false;
	}

	if (gCurrentFunction == FUNCTION_TRANSMIT && gSystickFlag0) {
		gSystickFlag0 = false;
		g_200003FD = 1;
		// TODO
		//TalkRelatedCode();
		AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP);
		RADIO_SomethingElse(4);
		GUI_DisplayScreen();
	}
	if (g_2000037E == 1) {
		return;
	}
	if (gCurrentFunction != FUNCTION_TRANSMIT) {
		FUN_0000510c();
	}
	if (gFmRadioCountdown) {
		return;
	}

	if (gScreenToDisplay != DISPLAY_SCANNER && gCurrentStep != 0 && gSystickFlag9 && !gPttIsPressed && gVoiceWriteIndex == 0) {
		if (g_20000410 - 200 < 7) {
			if (gCurrentFunction == FUNCTION_3) {
				FUN_000069f8(FUNCTION_4);
			} else {
				APP_MoreRadioStuff();
			}
		} else {
			if (gCopyOfCodeType != CODE_TYPE_OFF || gCurrentFunction != FUNCTION_3) {
				FUN_00007dd4();
			} else {
				FUN_000069f8(FUNCTION_4);
			}
		}
		gScanPauseMode = 0;
		g_20000411 = 0;
		gSystickFlag9 = false;
	}

	if (g_20000381 == 1 && gSystickFlag9 && gVoiceWriteIndex == 0) {
		APP_DCS_Related();
		gSystickFlag9 = false;
	}

	if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF && gIsNoaaMode && gSystickFlag8 && gVoiceWriteIndex == 0) {
		NOAA_IncreaseChannel();
		RADIO_SetupRegisters(false);
		gSystickFlag8 = false;
		g_20000356 = 7;
	}

	if (gScreenToDisplay != DISPLAY_SCANNER && gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
		if (gSystickFlag7 && gVoiceWriteIndex == 0) {
			if (gCurrentStep == 0 && g_20000381 == 0) {
				if (!gPttIsPressed && gFmMute == false && g_200003BC == 0 && gCurrentFunction != FUNCTION_POWER_SAVE) {
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

	if (g_20000390 != 0 && gSystickFlag11 && gCurrentFunction != FUNCTION_2 && gCurrentFunction != FUNCTION_4 && gCurrentFunction != FUNCTION_TRANSMIT) {
		APP_PlayFM();
		gSystickFlag11 = false;
	}

	if (gEeprom.VOX_SWITCH == true) {
		//FUN_00008334();
	}

	if (gSystickFlag5) {
		if (gEeprom.BATTERY_SAVE == 0 || gCurrentStep != 0 || g_20000381 != 0 || gFmMute != false || gPttIsPressed || gScreenToDisplay != DISPLAY_MAIN || gKeyBeingHeld	 != 0 || g_200003BC != 0) {
			g_2000032E = 1000;
		} else {
			// TODO: Double check polarity
			if ((206 < gEeprom.VfoChannel[0] || 206 < gEeprom.VfoChannel[1]) && gIsNoaaMode) {
				g_2000032E = 1000;
			} else {
				FUNCTION_Select(FUNCTION_POWER_SAVE);
			}
		}
		gSystickFlag5 = false;
	}

	if (gBatterySaveCountdownExpired && gCurrentFunction == FUNCTION_POWER_SAVE && gVoiceWriteIndex == 0) {
		if (gThisCanEnable_BK4819_Rxon == true) {
			BK4819_Conditional_RX_TurnOn_and_GPIO6_Enable();
			if (gEeprom.VOX_SWITCH == true) {
				BK4819_EnableVox(gEeprom.VOX1_THRESHOLD, gEeprom.VOX0_THRESHOLD);
			}
			if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF && gCurrentStep == 0 && g_20000381 == 0) {
				FUN_00007f4c();
				g_20000382 = 0;
			}
			FUNCTION_Init();
			gBatterySave = 10;
			gThisCanEnable_BK4819_Rxon = false;
		} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF || gCurrentStep != 0 || g_20000381 != 0 || g_20000382 != 0) {
			gCurrentRSSI = BK4819_GetRSSI();
			GUI_DisplayRSSI(gCurrentRSSI);
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

	if (!gSetting_KILLED && (gScreenToDisplay != DISPLAY_AIRCOPY || gAircopyState == AIRCOPY_READY)) {
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
}

void APP_TimeSlice10ms(void)
{
	gFlashLightBlinkCounter++;
#if 0
	if (UART_CheckForCommand()) {
		disableIRQinterrupts();
		ProcessUartCommand();
		enableIRQinterrupts();
	}
#endif
	if (g_2000037E == 1) {
		return;
	}

	if (gCurrentFunction != FUNCTION_POWER_SAVE || gThisCanEnable_BK4819_Rxon == false) {
		APP_CheckRadioInterrupts();
	}

	if (gCurrentFunction != FUNCTION_TRANSMIT) {
		if (g_2000036F == 1) {
			GUI_DisplayStatusLine();
			g_2000036F = 0;
		}
		if (gUpdateDisplay) {
			GUI_DisplayScreen();
			gUpdateDisplay = false;
		}
	}

	// Skipping authentic device checks

	if (gFmRadioCountdown != 0) {
		return;
	}

	if (gFlashLightState == FLASHLIGHT_BLINK && (gFlashLightBlinkCounter & 15U) == 0) {
		GPIO_FlipBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT);
	}
	if (g_200003B6 != 0) {
		g_200003B6--;
	}
	if (g_200003B8 != 0) {
		g_200003B8--;
	}
	if (gCurrentFunction == FUNCTION_TRANSMIT) {
#if 0
#endif
	}
	if (gFmMute && g_2000038E != 0) {
		g_2000038E--;
		if (g_2000038E == 0) {
			FUN_000059b4();
			GUI_SelectNextDisplay(DISPLAY_FM);
		}
	}

	if (gScreenToDisplay == DISPLAY_AIRCOPY && gAircopyState == AIRCOPY_TRANSFER && gAirCopyIsSendMode == 1) {
		if (gAircopySendCountdown != 0) {
			gAircopySendCountdown--;
			if (gAircopySendCountdown == 0) {
				//FSK_SendMessage();
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
	if (g_2000037E == 1) {
		BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);
		if ((gBatteryCurrent < 0x1f5) && (gBatteryCurrentVoltage <= gBatteryCalibration[3])) {
			return;
		}
		//Command_05DD_RebootChip();
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
			GUI_DisplayRSSI(gCurrentRSSI);
			if (gCurrentFunction == FUNCTION_TRANSMIT) {
				goto LAB_00004b08;
			}
		}
		if ((g_20000390 == 0 || gAskToSave == true) && gCurrentStep == 0 && g_20000381 == 0) {
			if (gBacklightCountdown != 0) {
				gBacklightCountdown--;
				if (gBacklightCountdown == 0) {
					GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
				}
			}
			if (gScreenToDisplay != DISPLAY_AIRCOPY && (gScreenToDisplay != DISPLAY_SCANNER || (1 < gScanState))) {
				if (gEeprom.AUTO_KEYPAD_LOCK == true && gKeyLockCountdown != 0 && g_200003BA == 0) {
					gKeyLockCountdown--;
					if (gKeyLockCountdown == 0) {
						gEeprom.KEY_LOCK = true;
					}
					g_2000036F = 1;
				}
				if (g_20000393 != 0) {
					g_20000393--;
					if (g_20000393 == 0) {
						if (gNumberOffset != 0 || g_200003BA == 1 || gScreenToDisplay == DISPLAY_MENU) {
							AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
						}
						if (gScreenToDisplay == DISPLAY_SCANNER) {
							BK4819_StopScan();
							RADIO_ConfigureChannel(0, 2);
							RADIO_ConfigureChannel(1, 2);
							RADIO_SetupRegisters(true);
						}
						gWasFKeyPressed = false;
						g_2000036F = 1;
						gNumberOffset = 0;
						g_200003BA = 0;
						g_200003BB = 0;
						gAskToSave = false;
						gAskToDelete = false;
						if (gFmMute == true && gCurrentFunction != FUNCTION_4 && gCurrentFunction != FUNCTION_2 && gCurrentFunction != FUNCTION_TRANSMIT) {
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
	if (gPttIsPressed != true && g_20000373 != 0) {
		g_20000373--;
		if (g_20000373 == 0) {
			RADIO_SomethingElse(0);
			if (gCurrentFunction != FUNCTION_4 && gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_2 && gFmMute == true) {
				FUN_000059b4();
				GUI_SelectNextDisplay(DISPLAY_FM);
			}
		}
	}

	if (gLowBattery) {
		gLowBatteryBlink = ++g_20000400 & 1;
		GUI_DisplayBatteryLevel(g_20000400);
		if (gCurrentFunction != FUNCTION_TRANSMIT) {
			if (g_20000400 < 30) {
				if (g_20000400 == 29 && gChargingWithTypeC == false) {
					AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP);
				}
			} else {
				g_20000400 = 0;
				if (gChargingWithTypeC == false) {
					AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP);
					AUDIO_SetVoiceID(0, VOICE_ID_LOW_VOLTAGE);
					if (gBatteryDisplayLevel == 0) {
						AUDIO_PlaySingleVoice(true);
						g_2000037E = 1;
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

	if (gScreenToDisplay == DISPLAY_SCANNER && g_20000461 == 0 && gScanState < 2) {
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

	if (g_200003BC != 0 && gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_4) {
		if (gDTMF_AUTO_RESET_TIME != 0) {
			gDTMF_AUTO_RESET_TIME--;
			if (gDTMF_AUTO_RESET_TIME == 0) {
				g_200003BC = 0;
				gUpdateDisplay = true;
			}
		}
		if (g_200003C1 == 1 && g_200003C4 != 0) {
			g_200003C4--;
			if ((g_200003C4 % 3) == 0) {
				AUDIO_PlayBeep(BEEP_440HZ_500MS);
			}
			if (g_200003C4 == 0) {
				g_200003C1 = 0;
			}
		}
	}

	if (g_200003BD != 0 && g_200003C3 != 0) {
		g_200003C3--;
		if (g_200003C3 == 0) {
			g_200003BD = 0;
			gUpdateDisplay = true;
		}
	}

	if (g_20000442 != 0) {
		g_20000442--;
		if (g_20000442 == 0) {
			gDTMF_WriteIndex = 0;
			memset(gDTMF_Received, 0, sizeof(gDTMF_Received));
		}
	}
}

static void APP_ProcessKey(KEY_Code_t CurrentKey, bool bKeyPressed, bool bKeyHeld)
{
	if (gCurrentFunction == FUNCTION_POWER_SAVE) {
		FUNCTION_Select(FUNCTION_0);
	}
	g_2000032E = 1000;
	if (gEeprom.AUTO_KEYPAD_LOCK == true) {
		gKeyLockCountdown = 30;
	}
	if (bKeyPressed == false) {
		if (g_200003A5 != 0) {
			g_200003A5 = 0;
		}
		if (gFlagSaveSettings) {
			gFlagSaveSettings = false;
		}
		if (gFlagSaveFM) {
			gFlagSaveFM = false;
		}
		if (g_200003A7 != 0) {
			g_200003A7 = 0;
			RADIO_ConfigureChannel(gEeprom.TX_CHANNEL, 1);
			RADIO_SetupRegisters(true);
			GUI_SelectNextDisplay(DISPLAY_MAIN);
		}
	} else {
		if (CurrentKey != KEY_PTT) {
			g_20000393 = 0x10;
		}
		BACKLIGHT_TurnOn();
		if (g_200003C1 == 1) {
			g_200003C1 = 0;
			AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
			if (CurrentKey != KEY_PTT) {
				g_20000394 = true;
				return;
			}
		}
	}

	if (gF_LOCK == 1) {
		if (CurrentKey == KEY_PTT) {
			return;
		}
		if (CurrentKey == KEY_SIDE2) {
			return;
		}
		if (CurrentKey == KEY_SIDE1) {
			return;
		}
	}

}

