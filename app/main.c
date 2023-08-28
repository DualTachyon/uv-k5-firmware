#include "app/main.h"
#include "audio.h"
#include "fm.h"
#include "frequencies.h"
#include "gui.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

extern void APP_SwitchToFM(void);
extern void FUN_0000773c(void);
extern void APP_AddStepToFrequency(VFO_Info_t *pInfo, uint8_t Step);
extern void APP_ChangeStepDirectionMaybe(bool bFlag, uint8_t Direction);
extern void APP_CycleOutputPower(void);
extern void APP_FlipVoxSwitch(void);

void MAIN_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint8_t Vfo;

	Vfo = gEeprom.TX_CHANNEL;

	if (bKeyHeld) {
		return;
	}
	if (!bKeyPressed) {
		return;
	}

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (!gWasFKeyPressed) {
		NUMBER_Append(Key);
		gRequestDisplayScreen = DISPLAY_MAIN;
		if (gTxRadioInfo->CHANNEL_SAVE < 200) {
			uint16_t Channel;

			if (gNumberOffset != 3) {
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gRequestDisplayScreen = DISPLAY_MAIN;
				return;
			}
			gNumberOffset = 0;
			Channel = ((gNumberForPrintf[0] * 100) + (gNumberForPrintf[1] * 10) + gNumberForPrintf[2]) - 1;
			if (!RADIO_CheckValidChannel(Channel, false, 0)) {
				gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
				return;
			}
			gAnotherVoiceID = (VOICE_ID_t)Key;
			gEeprom.EEPROM_0E81_0E84[Vfo] = (uint8_t)Channel;
			gEeprom.VfoChannel[Vfo] = Channel;
			gRequestSaveVFO = true;
			g_2000039A = 2;
			return;
		}
		if (gTxRadioInfo->CHANNEL_SAVE < 207) {
			uint32_t Frequency;

			if (gNumberOffset < 6) {
				gAnotherVoiceID = Key;
				return;
			}
			gNumberOffset = 0;
			NUMBER_Get(gNumberForPrintf, &Frequency);
			if (gSetting_350EN || (4999990 < (Frequency - 35000000))) {
				uint8_t i;

				for (i = 0; i < 7; i++) {
					if (Frequency <= gUpperLimitFrequencyBandTable[i] && (gLowerLimitFrequencyBandTable[i] <= Frequency)) {
						if (i < 7) {
							gAnotherVoiceID = (VOICE_ID_t)Key;
							if (gTxRadioInfo->Band != i) {
								gTxRadioInfo->Band = i;
								gEeprom.VfoChannel[Vfo] = i + 200;
								gEeprom.EEPROM_0E82_0E85[Vfo] = i + 200;
								SETTINGS_SaveVfoIndices();
								RADIO_ConfigureChannel(Vfo, 2);
							}
							Frequency += 75;
							gTxRadioInfo->DCS[0].Frequency = FREQUENCY_FloorToStep(
									Frequency,
									gTxRadioInfo->StepFrequency,
									gLowerLimitFrequencyBandTable[gTxRadioInfo->Band]
									);
							gRequestSaveChannel = 1;
							return;
						}
						break;
					}
				}
			}
		} else {
			uint8_t Channel;

			if (gNumberOffset != 2) {
				gAnotherVoiceID = Key;
				gRequestDisplayScreen = DISPLAY_MAIN;
				return;
			}
			gNumberOffset = 0;
			Channel = (gNumberForPrintf[0] * 10) + gNumberForPrintf[1];
			if ((Channel - 1) < 10) {
				Channel += 207;
				gAnotherVoiceID = (VOICE_ID_t)Key;
				gEeprom.NoaaChannel[Vfo] = Channel;
				gEeprom.VfoChannel[Vfo] = Channel;
				gRequestSaveVFO = true;
				g_2000039A = 2;
				return;
			}
		}
		gRequestDisplayScreen = DISPLAY_MAIN;
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}
	gWasFKeyPressed = false;
	g_2000036F = 1;
	switch (Key) {
	case KEY_0:
		APP_SwitchToFM();
		break;

	case KEY_2:
		if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_CHAN_A) {
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_CHAN_B;
		} else if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_CHAN_B) {
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_CHAN_A;
		} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_CHAN_A) {
			gEeprom.DUAL_WATCH = DUAL_WATCH_CHAN_B;
		} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_CHAN_B) {
			gEeprom.DUAL_WATCH = DUAL_WATCH_CHAN_A;
		} else {
			gEeprom.TX_CHANNEL = (Vfo == 0);
		}
		gRequestSaveSettings = 1;
		g_20000398 = 1;
		gRequestDisplayScreen = DISPLAY_MAIN;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		break;

	case KEY_3:
		if ((gEeprom.VFO_OPEN) && (gTxRadioInfo->CHANNEL_SAVE < 207)) {
			uint8_t Channel;

			if (gTxRadioInfo->CHANNEL_SAVE < 200) {
				gEeprom.VfoChannel[Vfo] = gEeprom.EEPROM_0E82_0E85[gEeprom.TX_CHANNEL];
				gAnotherVoiceID = VOICE_ID_FREQUENCY_MODE;
				gRequestSaveVFO = true;
				g_2000039A = 2;
				break;
			}
			Channel = RADIO_FindNextChannel(gEeprom.EEPROM_0E81_0E84[gEeprom.TX_CHANNEL], 1, false, 0);
			if (Channel != 0xFF) {
				gEeprom.VfoChannel[Vfo] = Channel;
				AUDIO_SetVoiceID(0, VOICE_ID_CHANNEL_MODE);
				AUDIO_SetDigitVoice(1, Channel + 1);
				gAnotherVoiceID = 0xFE;
				gRequestSaveVFO = true;
				g_2000039A = 2;
				break;
			}
		}
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		break;

	case KEY_4:
		gWasFKeyPressed = false;
		g_2000036F = 1;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gFlagStartScan = 1;
		g_20000458 = 0;
		g_20000459 = gEeprom.CROSS_BAND_RX_TX;
		gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
		break;

	case KEY_5:
		if (gTxRadioInfo->CHANNEL_SAVE < 207) {
			gEeprom.VfoChannel[Vfo] = gEeprom.NoaaChannel[gEeprom.TX_CHANNEL];
		} else {
			gEeprom.VfoChannel[Vfo] = gEeprom.EEPROM_0E82_0E85[gEeprom.TX_CHANNEL];
			gAnotherVoiceID = VOICE_ID_FREQUENCY_MODE;
		}
		gRequestSaveVFO = true;
		g_2000039A = 2;
		break;

	case KEY_6:
		APP_CycleOutputPower();
		break;

	case KEY_7:
		APP_FlipVoxSwitch();
		break;

	default:
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		g_2000036F = 1;
		gWasFKeyPressed = false;
	}
}

void MAIN_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		if (!gFmRadioMode) {
			if (gStepDirection == 0) {
				if (gNumberOffset == 0) {
					return;
				}
				gNumberOffset--;
				gNumberForPrintf[gNumberOffset] = 10;
				if (gNumberOffset == 0) {
					gAnotherVoiceID = VOICE_ID_CANCEL;
				}
			} else {
				FUN_0000773c();
				gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
			}
			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}
		APP_SwitchToFM();
	}
}

void MAIN_Key_MENU(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) {
		bool bFlag;

		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		bFlag = gNumberOffset == 0;
		gNumberOffset = 0;
		if (bFlag) {
			gFlagRefreshSetting = true;
			gRequestDisplayScreen = DISPLAY_MENU;
			gAnotherVoiceID = VOICE_ID_MENU;
		} else {
			gRequestDisplayScreen = DISPLAY_MAIN;
		}
	}
}

void MAIN_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction)
{
	uint8_t Channel;

	Channel = gEeprom.VfoChannel[gEeprom.TX_CHANNEL];
	if (bKeyHeld || !bKeyPressed) {
		if (gNumberOffset != 0) {
			return;
		}
		if (!bKeyPressed) {
			if (!bKeyHeld) {
				return;
			}
			if (199 < Channel) {
				return;
			}
			AUDIO_SetDigitVoice(0, gTxRadioInfo->CHANNEL_SAVE + 1);
			gAnotherVoiceID = 0xFE;
			return;
		}
	} else {
		if (gNumberOffset != 0) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			return;
		}
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
	}

	if (gStepDirection == 0) {
		if (Channel < 207) {
			uint8_t Next;

			if (199 < Channel) {
				APP_AddStepToFrequency(gTxRadioInfo, Direction);
				gRequestSaveChannel = 1;
				return;
			}
			Next = RADIO_FindNextChannel(Channel + Direction, Direction, false, 0);
			if (Next == 0xFF) {
				return;
			}
			if (Channel == Next) {
				return;
			}
			gEeprom.EEPROM_0E81_0E84[gEeprom.TX_CHANNEL] = Next;
			gEeprom.VfoChannel[gEeprom.TX_CHANNEL] = Next;
			if (!bKeyHeld) {
				AUDIO_SetDigitVoice(0, Next + 1);
				gAnotherVoiceID = 0xFE;
			}
		} else {
			Channel = 207 + NUMBER_AddWithWraparound(gEeprom.VfoChannel[gEeprom.TX_CHANNEL] + 207,Direction, 0, 9);
			gEeprom.NoaaChannel[gEeprom.TX_CHANNEL] = Channel;
			gEeprom.VfoChannel[gEeprom.TX_CHANNEL] = Channel;
		}
		gRequestSaveVFO = true;
		g_2000039A = 2;
		return;
	}
	APP_ChangeStepDirectionMaybe(false, Direction);
	g_20000394 = true;
}

