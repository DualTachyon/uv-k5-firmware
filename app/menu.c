#include <string.h>
#include "app/menu.h"
#include "audio.h"
#include "board.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/backlight.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "frequencies.h"
#include "gui.h"
#include "misc.h"
#include "settings.h"
#include "sram-overlay.h"

static const VOICE_ID_t MenuVoices[] = {
	VOICE_ID_SQUELCH,
	VOICE_ID_FREQUENCY_STEP,
	VOICE_ID_POWER,
	VOICE_ID_DCS,
	VOICE_ID_CTCSS,
	VOICE_ID_DCS,
	VOICE_ID_CTCSS,
	VOICE_ID_FREQUENCY_DIRECTION,
	VOICE_ID_OFFSET_FREQUENCY,
	VOICE_ID_CHANNEL_BANDWIDTH,
	VOICE_ID_SCRAMBLER_ON,
	VOICE_ID_BUSY_LOCKOUT,
	VOICE_ID_MEMORY_CHANNEL,
	VOICE_ID_SAVE_MODE,
	VOICE_ID_VOX,
	VOICE_ID_INVALID,
	VOICE_ID_DUAL_STANDBY,
	VOICE_ID_INVALID,
	VOICE_ID_BEEP_PROMPT,
	VOICE_ID_TRANSMIT_OVER_TIME,
	VOICE_ID_VOICE_PROMPT,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_ANI_CODE,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_DELETE_CHANNEL,
	VOICE_ID_INITIALISATION,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
	VOICE_ID_INVALID,
};

static void FUN_000074f8(uint8_t Direction)
{
	g_20000381 = 1;
	gMenuScrollDirection = Direction;
	RADIO_ConfigureTX();
	MENU_SelectNextDCS();
	ScanPauseDelayIn10msec = 50;
	gSystickFlag9 = false;
}

int MENU_GetLimits(uint8_t Cursor, uint8_t *pMin, uint8_t *pMax)
{
	switch (Cursor) {
	case MENU_SQL:
		*pMin = 0;
		*pMax = 9;
		break;
	case MENU_STEP:
		if (gTxRadioInfo->Band == BAND2_108MHz) {
			*pMin = 0;
			*pMax = 6;
			break;
		}
		// Fallthrough
	case MENU_ABR: case MENU_F_LOCK:
		*pMin = 0;
		*pMax = 5;
		break;
	case MENU_TXP: case MENU_SFT_D:
	case MENU_TDR: case MENU_WX:
	case MENU_VOICE: case MENU_SC_REV:
	case MENU_MDF: case MENU_PONMSG:
	case MENU_ROGER:
		*pMin = 0;
		*pMax = 2;
		break;
	case MENU_R_DCS: case MENU_T_DCS:
		*pMin = 0;
		*pMax = 208;
		break;
	case MENU_R_CTCS: case MENU_T_CTCS:
		*pMin = 0;
		*pMax = 50;
		break;
	case MENU_W_N: case MENU_BCL:
	case MENU_BEEP: case MENU_AUTOLK:
	case MENU_S_ADD1: case MENU_S_ADD2:
	case MENU_STE: case MENU_AL_MOD:
	case MENU_D_ST: case MENU_D_DCD:
	case MENU_AM: case MENU_NOAA_S:
	case MENU_RESET: case MENU_350TX:
	case MENU_200TX: case MENU_500TX:
	case MENU_350EN: case MENU_SCREN:
		*pMin = 0;
		*pMax = 1;
		break;
	case MENU_SCR: case MENU_VOX:
	case MENU_TOT: case MENU_RP_STE:
		*pMin = 0;
		*pMax = 10;
		break;
	case MENU_MEM_CH: case MENU_1_CALL:
	case MENU_SLIST1: case MENU_SLIST2:
	case MENU_DEL_CH:
		*pMin = 0;
		*pMax = 199;
		break;
	case MENU_SAVE: case MENU_MIC:
		*pMin = 0;
		*pMax = 4;
		break;
	case MENU_S_LIST:
		*pMin = 1;
		*pMax = 2;
		break;
	case MENU_D_RSP: case MENU_PTT_ID:
		*pMin = 0;
		*pMax = 3;
		break;
	case MENU_D_HOLD:
		*pMin = 5;
		*pMax = 60;
		break;
	case MENU_D_PRE:
		*pMin = 3;
		*pMax = 99;
		break;
	case MENU_D_LIST:
		*pMin = 1;
		*pMax = 16;
		break;
	default:
		return -1;
	}

	return 0;
}

void MENU_AcceptSetting(void)
{
	uint8_t Min, Max;
	uint8_t Sel;
	uint8_t Code;

	Sel = 0;

	if (!MENU_GetLimits(gMenuCursor, &Min, &Max)) {
		if (gSubMenuSelection < Min) {
			gSubMenuSelection = Min;
		} else if (gSubMenuSelection > Max) {
			gSubMenuSelection = Max;
		}
	}

	switch (gMenuCursor) {
	case MENU_SQL:
		gEeprom.SQUELCH_LEVEL = gSubMenuSelection;
		g_2000039E = 1;
		g_2000039A = 1;
		return;

	case MENU_STEP:
		if ((gTxRadioInfo->CHANNEL_SAVE - 200) < 7) {
			gTxRadioInfo->STEP_SETTING = gSubMenuSelection;
			g_2000039D = 1;
			return;
		}
		gSubMenuSelection = gTxRadioInfo->STEP_SETTING;
		return;

	case MENU_TXP:
		gTxRadioInfo->OUTPUT_POWER = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_T_DCS:
		Sel = 1;
		// Fallthrough
	case MENU_R_DCS:
		if (gSubMenuSelection == 0) {
			if (gTxRadioInfo->DCS[Sel].CodeType != CODE_TYPE_DIGITAL && gTxRadioInfo->DCS[Sel].CodeType != CODE_TYPE_REVERSE_DIGITAL) {
				g_2000039D = 1;
				return;
			}
			Code = 0;
			gTxRadioInfo->DCS[Sel].CodeType = CODE_TYPE_OFF;
		} else if (gSubMenuSelection < 105) {
			gTxRadioInfo->DCS[Sel].CodeType = CODE_TYPE_DIGITAL;
			Code = gSubMenuSelection - 1;
		} else {
			gTxRadioInfo->DCS[Sel].CodeType = CODE_TYPE_REVERSE_DIGITAL;
			Code = gSubMenuSelection - 105;
		}
		gTxRadioInfo->DCS[Sel].RX_TX_Code = Code;
		g_2000039D = 1;
		return;

	case MENU_T_CTCS:
		Sel = 1;
		// Fallthrough
	case MENU_R_CTCS:
		if (gSubMenuSelection == 0) {
			if (gTxRadioInfo->DCS[Sel].CodeType != CODE_TYPE_CONTINUOUS_TONE) {
				g_2000039D = 1;
				return;
			}
			Code = 0;
			gTxRadioInfo->DCS[Sel].CodeType = CODE_TYPE_OFF;
		} else {
			gTxRadioInfo->DCS[Sel].CodeType = CODE_TYPE_CONTINUOUS_TONE;
			Code = gSubMenuSelection - 1;
		}
		gTxRadioInfo->DCS[Sel].RX_TX_Code = Code;
		g_2000039D = 1;
		return;

	case MENU_SFT_D:
		gTxRadioInfo->FREQUENCY_DEVIATION_SETTING = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_OFFSET:
		gTxRadioInfo->FREQUENCY_OF_DEVIATION = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_W_N:
		gTxRadioInfo->CHANNEL_BANDWIDTH = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_SCR:
		gTxRadioInfo->SCRAMBLING_TYPE = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_BCL:
		gTxRadioInfo->BUSY_CHANNEL_LOCK = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_MEM_CH:
		gTxRadioInfo->CHANNEL_SAVE = gSubMenuSelection;
		g_2000039D = 2;
		gEeprom.EEPROM_0E81_0E84[0] = gSubMenuSelection;
		return;

	case MENU_SAVE:
		gEeprom.BATTERY_SAVE = gSubMenuSelection;
		break;

	case MENU_VOX:
		gEeprom.VOX_SWITCH = gSubMenuSelection != 0;
		if (gEeprom.VOX_SWITCH) {
			gEeprom.VOX_LEVEL = gSubMenuSelection - 1;
		}
		BOARD_EEPROM_LoadMoreSettings();
		g_20000398 = 1;
		g_2000039E = 1;
		g_2000036F = 1;
		return;

	case MENU_ABR:
		gEeprom.BACKLIGHT = gSubMenuSelection;
		if (gSubMenuSelection == 0) {
			GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
		} else {
			BACKLIGHT_TurnOn();
		}
		break;

	case MENU_TDR:
		gEeprom.DUAL_WATCH = gSubMenuSelection;
		g_20000398 = 1;
		g_2000039E = 1;
		g_2000036F = 1;
		return;

	case MENU_WX:
		if (206 < gEeprom.VfoChannel[0]) {
			return;
		}
		if (206 < gEeprom.VfoChannel[1]) {
			return;
		}
		gEeprom.CROSS_BAND_RX_TX = gSubMenuSelection;
		g_20000398 = 1;
		g_2000039E = 1;
		g_2000036F = 1;
		return;

	case MENU_BEEP:
		gEeprom.BEEP_CONTROL = gSubMenuSelection;
		break;

	case MENU_TOT:
		gEeprom.TX_TIMEOUT_TIMER = gSubMenuSelection;
		break;

	case MENU_VOICE:
		gEeprom.VOICE_PROMPT = gSubMenuSelection;
		g_2000039E = 1;
		g_2000036F = 1;
		return;

	case MENU_SC_REV:
		gEeprom.SCAN_RESUME_MODE = gSubMenuSelection;
		break;

	case MENU_MDF:
		gEeprom.CHANNEL_DISPLAY_MODE = gSubMenuSelection;
		break;

	case MENU_AUTOLK:
		gEeprom.AUTO_KEYPAD_LOCK = gSubMenuSelection;
		gKeyLockCountdown = 0x1e;
		break;

	case MENU_S_ADD1:
		gTxRadioInfo->SCANLIST1_PARTICIPATION = gSubMenuSelection;
		SETTINGS_UpdateChannel(gTxRadioInfo->CHANNEL_SAVE, gTxRadioInfo, true);
		g_2000039A = 1;
		g_2000039B = 1;
		return;

	case MENU_S_ADD2:
		gTxRadioInfo->SCANLIST2_PARTICIPATION = gSubMenuSelection;
		SETTINGS_UpdateChannel(gTxRadioInfo->CHANNEL_SAVE, gTxRadioInfo, true);
		g_2000039A = 1;
		g_2000039B = 1;
		return;

	case MENU_STE:
		gEeprom.TAIL_NOTE_ELIMINATION = gSubMenuSelection;
		break;

	case MENU_RP_STE:
		gEeprom.REPEATER_TAIL_TONE_ELIMINATION = gSubMenuSelection;
		break;

	case MENU_MIC:
		gEeprom.MIC_SENSITIVITY = gSubMenuSelection;
		BOARD_EEPROM_LoadMoreSettings();
		g_2000039E = 1;
		g_20000398 = 1;
		return;

	case MENU_1_CALL:
		gEeprom.CHAN_1_CALL = gSubMenuSelection;
		break;

	case MENU_S_LIST:
		gEeprom.SCAN_LIST_DEFAULT = gSubMenuSelection - 1;
		break;

	case MENU_AL_MOD:
		gEeprom.ALARM_MODE = gSubMenuSelection;
		break;

	case MENU_D_ST:
		gEeprom.DTMF_SIDE_TONE = gSubMenuSelection;
		break;

	case MENU_D_RSP:
		gEeprom.DTMF_DECODE_RESPONSE = gSubMenuSelection;
		break;

	case MENU_D_HOLD:
		gEeprom.DTMF_AUTO_RESET_TIME = gSubMenuSelection;
		break;

	case MENU_D_PRE:
		gEeprom.DTMF_PRELOAD_TIME = gSubMenuSelection * 10;
		break;

	case MENU_PTT_ID:
		gTxRadioInfo->DTMF_PTT_ID_TX_MODE = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_D_DCD:
		gTxRadioInfo->DTMF_DECODING_ENABLE = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_D_LIST:
		gDTMFChosenContact = gSubMenuSelection - 1;
		if (gIsDtmfContactValid) {
			GUI_SelectNextDisplay(DISPLAY_MAIN);
			g_200003BA = 1;
			g_200003BB = 3;
			memcpy(g_20000D1C, gDTMF_ID, 4);
			gRequestDisplayScreen = DISPLAY_INVALID;
		}
		return;

	case MENU_PONMSG:
		gEeprom.POWER_ON_DISPLAY_MODE = gSubMenuSelection;
		break;

	case MENU_ROGER:
		gEeprom.ROGER = gSubMenuSelection;
		break;

	case MENU_AM:
		gTxRadioInfo->AM_CHANNEL_MODE = gSubMenuSelection;
		g_2000039D = 1;
		return;

	case MENU_NOAA_S:
		gEeprom.NOAA_AUTO_SCAN = gSubMenuSelection;
		g_2000039E = 1;
		g_20000398 = 1;
		return;

	case MENU_DEL_CH:
		SETTINGS_UpdateChannel(gSubMenuSelection, NULL, false);
		g_2000039A = 2;
		g_2000039B = 1;
		return;

	case MENU_RESET:
		//EepromFactoryReset(gSubMenuSelection);
		return;

	case MENU_350TX:
		gSetting_350TX = gSubMenuSelection;
		break;

	case MENU_F_LOCK:
		gSetting_F_LOCK = gSubMenuSelection;
		break;

	case MENU_200TX:
		gSetting_200TX = gSubMenuSelection;
		break;

	case MENU_500TX:
		gSetting_500TX = gSubMenuSelection;
		break;

	case MENU_350EN:
		gSetting_350EN = gSubMenuSelection;
		g_2000039E = 1;
		g_2000039A = 2;
		g_2000039B = 1;
		return;

	case MENU_SCREN:
		gSetting_ScrambleEnable = gSubMenuSelection;
		g_2000039E = 1;
		g_20000398 = 1;
		return;

	default:
		return;
	}

	g_2000039E = 1;
}

void MENU_SelectNextDCS(void)
{
	uint8_t UpperLimit;

	if (gMenuCursor == MENU_R_DCS) {
		UpperLimit = 208;
	} else if (gMenuCursor == MENU_R_CTCS) {
		UpperLimit = 50;
	} else {
		return;
	}

	gSubMenuSelection = NUMBER_AddWithWraparound(gSubMenuSelection, gMenuScrollDirection, 1, UpperLimit);
	if (gMenuCursor == MENU_R_DCS) {
		if (gSubMenuSelection > 105) {
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

static void MENU_ClampSelection(int8_t Direction)
{
	uint8_t Min, Max;

	if (!MENU_GetLimits(gMenuCursor, &Min, &Max)) {
		uint8_t Selection = gSubMenuSelection;

		if (Selection < Min) {
			Selection = Min;
		} else if (Selection > Max) {
			Selection = Max;
		}
		gSubMenuSelection = NUMBER_AddWithWraparound(Selection, Direction, Min, Max);
	}
}

void MENU_ShowCurrentSetting(void)
{
	switch (gMenuCursor) {
	case MENU_SQL:
		gSubMenuSelection = gEeprom.SQUELCH_LEVEL;
		break;

	case MENU_STEP:
		gSubMenuSelection = gTxRadioInfo->STEP_SETTING;
		break;

	case MENU_TXP:
		gSubMenuSelection = gTxRadioInfo->OUTPUT_POWER;
		break;

	case MENU_R_DCS:
		switch (gTxRadioInfo->DCS[0].CodeType) {
		case CODE_TYPE_DIGITAL:
			gSubMenuSelection = gTxRadioInfo->DCS[0].RX_TX_Code + 1;
			break;
		case CODE_TYPE_REVERSE_DIGITAL:
			gSubMenuSelection = gTxRadioInfo->DCS[0].RX_TX_Code + 105;
			break;
		default:
			gSubMenuSelection = 0;
			break;
		}
		break;

	case MENU_RESET:
		gSubMenuSelection = 0;
		break;

	case MENU_R_CTCS:
		if (gTxRadioInfo->DCS[0].CodeType == CODE_TYPE_CONTINUOUS_TONE) {
			gSubMenuSelection = gTxRadioInfo->DCS[0].RX_TX_Code + 1;
		} else {
			gSubMenuSelection = 0;
		}
		break;

	case MENU_T_DCS:
		switch (gTxRadioInfo->DCS[1].CodeType) {
		case CODE_TYPE_DIGITAL:
			gSubMenuSelection = gTxRadioInfo->DCS[1].RX_TX_Code + 1;
			break;
		case CODE_TYPE_REVERSE_DIGITAL:
			gSubMenuSelection = gTxRadioInfo->DCS[1].RX_TX_Code + 105;
			break;
		default:
			gSubMenuSelection = 0;
			break;
		}
		break;

	case MENU_T_CTCS:
		if (gTxRadioInfo->DCS[1].CodeType == CODE_TYPE_CONTINUOUS_TONE) {
			gSubMenuSelection = gTxRadioInfo->DCS[1].RX_TX_Code + 1;
		} else {
			gSubMenuSelection = 0;
		}
		break;

	case MENU_SFT_D:
		gSubMenuSelection = gTxRadioInfo->FREQUENCY_DEVIATION_SETTING;
		break;

	case MENU_OFFSET:
		gSubMenuSelection = gTxRadioInfo->FREQUENCY_OF_DEVIATION;
		break;

	case MENU_W_N:
		gSubMenuSelection = gTxRadioInfo->CHANNEL_BANDWIDTH;
		break;

	case MENU_SCR:
		gSubMenuSelection = gTxRadioInfo->SCRAMBLING_TYPE;
		break;

	case MENU_BCL:
		gSubMenuSelection = gTxRadioInfo->BUSY_CHANNEL_LOCK;
		break;

	case MENU_MEM_CH:
		gSubMenuSelection = gEeprom.EEPROM_0E81_0E84[0];
		break;

	case MENU_SAVE:
		gSubMenuSelection = gEeprom.BATTERY_SAVE;
		break;

	case MENU_VOX:
		if (gEeprom.VOX_SWITCH) {
			gSubMenuSelection = gEeprom.VOX_LEVEL + 1;
		} else {
			gSubMenuSelection = 0;
		}
		break;

	case MENU_ABR:
		gSubMenuSelection = gEeprom.BACKLIGHT;
		break;

	case MENU_TDR:
		gSubMenuSelection = gEeprom.DUAL_WATCH;
		break;

	case MENU_WX:
		gSubMenuSelection = gEeprom.CROSS_BAND_RX_TX;
		break;

	case MENU_BEEP:
		gSubMenuSelection = gEeprom.BEEP_CONTROL;
		break;

	case MENU_TOT:
		gSubMenuSelection = gEeprom.TX_TIMEOUT_TIMER;
		break;

	case MENU_VOICE:
		gSubMenuSelection = gEeprom.VOICE_PROMPT;
		break;

	case MENU_SC_REV:
		gSubMenuSelection = gEeprom.SCAN_RESUME_MODE;
		break;

	case MENU_MDF:
		gSubMenuSelection = gEeprom.CHANNEL_DISPLAY_MODE;
		break;

	case MENU_AUTOLK:
		gSubMenuSelection = gEeprom.AUTO_KEYPAD_LOCK;
		break;

	case MENU_S_ADD1:
		gSubMenuSelection = gTxRadioInfo->SCANLIST1_PARTICIPATION;
		break;

	case MENU_S_ADD2:
		gSubMenuSelection = gTxRadioInfo->SCANLIST2_PARTICIPATION;
		break;

	case MENU_STE:
		gSubMenuSelection = gEeprom.TAIL_NOTE_ELIMINATION;
		break;

	case MENU_RP_STE:
		gSubMenuSelection = gEeprom.REPEATER_TAIL_TONE_ELIMINATION;
		break;

	case MENU_MIC:
		gSubMenuSelection = gEeprom.MIC_SENSITIVITY;
		break;

	case MENU_1_CALL:
		gSubMenuSelection = gEeprom.CHAN_1_CALL;
		break;

	case MENU_S_LIST:
		gSubMenuSelection = gEeprom.SCAN_LIST_DEFAULT + 1;
		break;

	case MENU_SLIST1:
		gSubMenuSelection = RADIO_FindNextChannel(0, 1, true, 0);
		break;

	case MENU_SLIST2:
		gSubMenuSelection = RADIO_FindNextChannel(0, 1, true, 1);
		break;

	case MENU_AL_MOD:
		gSubMenuSelection = gEeprom.ALARM_MODE;
		break;

	case MENU_D_ST:
		gSubMenuSelection = gEeprom.DTMF_SIDE_TONE;
		break;

	case MENU_D_RSP:
		gSubMenuSelection = gEeprom.DTMF_DECODE_RESPONSE;
		break;

	case MENU_D_HOLD:
		gSubMenuSelection = gEeprom.DTMF_AUTO_RESET_TIME;
		break;

	case MENU_D_PRE:
		gSubMenuSelection = gEeprom.DTMF_PRELOAD_TIME / 10;
		break;

	case MENU_PTT_ID:
		gSubMenuSelection = gTxRadioInfo->DTMF_PTT_ID_TX_MODE;
		break;

	case MENU_D_DCD:
		gSubMenuSelection = gTxRadioInfo->DTMF_DECODING_ENABLE;
		break;

	case MENU_D_LIST:
		gSubMenuSelection = gDTMFChosenContact + 1;
		break;

	case MENU_PONMSG:
		gSubMenuSelection = gEeprom.POWER_ON_DISPLAY_MODE;
		break;

	case MENU_ROGER:
		gSubMenuSelection = gEeprom.ROGER;
		break;

	case MENU_AM:
		gSubMenuSelection = gTxRadioInfo->AM_CHANNEL_MODE;
		break;

	case MENU_NOAA_S:
		gSubMenuSelection = gEeprom.NOAA_AUTO_SCAN;
		break;

	case MENU_DEL_CH:
		gSubMenuSelection = RADIO_FindNextChannel(gEeprom.EEPROM_0E81_0E84[0], 1, false, 1);
		break;

	case MENU_350TX:
		gSubMenuSelection = gSetting_350TX;
		break;

	case MENU_F_LOCK:
		gSubMenuSelection = gSetting_F_LOCK;
		break;

	case MENU_200TX:
		gSubMenuSelection = gSetting_200TX;
		break;

	case MENU_500TX:
		gSubMenuSelection = gSetting_500TX;
		break;

	case MENU_350EN:
		gSubMenuSelection = gSetting_350EN;
		break;

	case MENU_SCREN:
		gSubMenuSelection = gSetting_ScrambleEnable;
		break;
	}
}

//

void MENU_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint16_t Value = 0;

	if (bKeyHeld) {
		return;
	}
	if (!bKeyPressed) {
		return;
	}

	g_20000396 = 1;
	NUMBER_Append(Key);
	gRequestDisplayScreen = DISPLAY_MENU;
	if (g_200003C6 == 0) {
		switch (gNumberOffset) {
		case 1:
			Value = gNumberForPrintf[0];
			if (Value && Value <= gMenuListCount) {
				gMenuCursor = Value - 1;
				g_200003A4 = 1;
				return;
			}
			break;
		case 2:
			gNumberOffset = 0;
			Value = (gNumberForPrintf[0] * 10) + gNumberForPrintf[1];
			if (Value && Value <= gMenuListCount) {
				gMenuCursor = Value - 1;
				g_200003A4 = 1;
				return;
			}
			break;
		}
		gNumberOffset = 0;
	} else {
		if (gMenuCursor == MENU_OFFSET) {
			uint32_t Frequency;

			if (gNumberOffset < 6) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				return;
			}
			gNumberOffset = 0;
			NUMBER_Get(gNumberForPrintf, &Frequency);
			Frequency += 75;
			gAnotherVoiceID = (VOICE_ID_t)Key;
			gSubMenuSelection = FREQUENCY_FloorToStep(Frequency, gTxRadioInfo->StepFrequency, 0);
			return;
		}
		if (gMenuCursor == MENU_MEM_CH || gMenuCursor == MENU_DEL_CH || gMenuCursor == MENU_1_CALL) {
			if (gNumberOffset < 3) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gRequestDisplayScreen = DISPLAY_MENU;
				return;
			}
			gNumberOffset = 0;
			Value = ((gNumberForPrintf[0] * 100) + (gNumberForPrintf[1] * 10) + gNumberForPrintf[2]) - 1;
			if (Value < 200) {
				gAnotherVoiceID = Key;
				gSubMenuSelection = Value;
				return;
			}
		} else {
			uint8_t Min, Max;

			if (!MENU_GetLimits(gMenuCursor, &Min, &Max)) {
				uint8_t Offset;

				if (Max < 100) {
					if (Max < 10) {
						Offset = 1;
					} else {
						Offset = 2;
					}
				} else {
					Offset = 3;
				}
				switch (gNumberOffset) {
				case 1:
					Value = gNumberForPrintf[0];
					break;
				case 2:
					Value = (gNumberForPrintf[0] * 10) + gNumberForPrintf[1];
					break;
				case 3:
					Value = (gNumberForPrintf[0] * 100) + (gNumberForPrintf[1] * 10) + gNumberForPrintf[2];
					break;
				}
				if (Offset == gNumberOffset) {
					gNumberOffset = 0;
				}
				if (Value <= Max) {
					gSubMenuSelection = Value;
					return;
				}
			} else {
				gNumberOffset = 0;
			}
		}
	}
	g_20000396 = 2;
}

void MENU_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		g_20000396 = 1;
		if (g_20000381 == 0) {
			if (g_200003C6 == 1) {
				if (gNumberOffset == 0 || gMenuCursor != MENU_OFFSET) {
					g_200003C6 = 0;
					gNumberOffset = 0;
					g_200003A4 = 1;
					gAnotherVoiceID = VOICE_ID_CANCEL;
				} else {
					gNumberOffset -= 1;
					gNumberForPrintf[gNumberOffset] = 10;
				}
				gRequestDisplayScreen = DISPLAY_MENU;
				return;
			}
			gAnotherVoiceID = VOICE_ID_CANCEL;
			gRequestDisplayScreen = DISPLAY_MAIN;
		} else {
			RADIO_Whatever();
			gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
			gRequestDisplayScreen = DISPLAY_MENU;
		}
		g_20000394 = true;
	}
}

void MENU_Key_MENU(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		g_20000396 = 1;
		gRequestDisplayScreen = DISPLAY_MENU;
		if (g_200003C6 == 0) {
			if (gMenuCursor != MENU_SCR) {
				gAnotherVoiceID = MenuVoices[gMenuCursor];
			}
			gAskForConfirmation = 0;
			g_200003C6 = 1;
		} else {
			if (gMenuCursor == MENU_RESET || gMenuCursor == MENU_MEM_CH || gMenuCursor == MENU_DEL_CH) {
				switch (gAskForConfirmation) {
				case 0:
					gAskForConfirmation = 1;
					break;
				case 1:
					gAskForConfirmation = 2;
					GUI_DisplayMenu();
					if (gMenuCursor == MENU_RESET) {
						AUDIO_SetVoiceID(0, VOICE_ID_CONFIRM);
						AUDIO_PlaySingleVoice(true);
						MENU_AcceptSetting();
						overlay_FLASH_RebootToBootloader();
					}
					gFlagAcceptSetting = true;
					g_200003C6 = 0;
					gAskForConfirmation = 0;
				}
			} else {
				gFlagAcceptSetting = true;
				g_200003C6 = 0;
			}
			g_20000381 = 0;
			if (gMenuCursor == MENU_SCR) {
				if (gSubMenuSelection == 0) {
					gAnotherVoiceID = VOICE_ID_SCRAMBLER_OFF;
				} else {
					gAnotherVoiceID = VOICE_ID_SCRAMBLER_ON;
				}
			} else {
				gAnotherVoiceID = VOICE_ID_CONFIRM;
			}
		}
		gNumberOffset = 0;
	}
}

void MENU_Key_STAR(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		g_20000396 = 1;
		RADIO_ConfigureTX();
		if (gInfoCHAN_A->CHANNEL_SAVE < 207 && !gInfoCHAN_A->IsAM) {
			if (gMenuCursor == MENU_R_CTCS || gMenuCursor == MENU_R_DCS) {
				if (g_20000381 == 0) {
					FUN_000074f8(0x01);
					gRequestDisplayScreen = DISPLAY_MENU;
					AUDIO_SetVoiceID(0,VOICE_ID_SCANNING_BEGIN);
					AUDIO_PlaySingleVoice(1);
				} else {
					RADIO_Whatever();
					gRequestDisplayScreen = DISPLAY_MENU;
					gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
				}
			}
			g_20000394 = true;
			return;
		}
		g_20000396 = 2;
	}
}

void MENU_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction)
{
	uint8_t VFO;
	uint8_t Channel;
	bool bCheckScanList;

	if (!bKeyHeld) {
		if (!bKeyPressed) {
			return;
		}
		g_20000396 = 1;
		gNumberOffset = 0;
	} else if (!bKeyPressed) {
		return;
	}

	if (g_20000381 != 0) {
		FUN_000074f8(Direction);
		g_20000394 = true;
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}

	if (g_200003C6 == 0) {
		gMenuCursor = NUMBER_AddWithWraparound(gMenuCursor, -Direction, 0, gMenuListCount - 1);
		g_200003A4 = true;
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}

	if (gMenuCursor == MENU_OFFSET) {
		int32_t Offset;

		Offset = (Direction * gTxRadioInfo->StepFrequency) + gSubMenuSelection;
		if (Offset < 99999990) {
			if (Offset < 0) {
				Offset = 99999990;
			}
		} else {
			Offset = 0;
		}
		gSubMenuSelection = FREQUENCY_FloorToStep(Offset, gTxRadioInfo->StepFrequency, 0);
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}

	VFO = 0;

	switch (gMenuCursor) {
	case MENU_DEL_CH:
	case MENU_1_CALL:
		bCheckScanList = false;
		break;
	case MENU_SLIST2:
		VFO = 1;
		// Fallthrough
	case MENU_SLIST1:
		bCheckScanList = true;
		break;
	default:
		MENU_ClampSelection(Direction);
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}

	Channel = RADIO_FindNextChannel(gSubMenuSelection + Direction, Direction, bCheckScanList, VFO);
	if (Channel != 0xFF) {
		gSubMenuSelection = Channel;
	}
	gRequestDisplayScreen = DISPLAY_MENU;
}

