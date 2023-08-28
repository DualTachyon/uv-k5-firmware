#include <string.h>
#include "app/main.h"
#include "audio.h"
#include "dtmf.h"
#include "fm.h"
#include "frequencies.h"
#include "gui.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

extern void APP_SwitchToFM(void);
extern void FUN_0000773c(void);
extern void APP_SetFrequencyByStep(VFO_Info_t *pInfo, int8_t Step);
extern void APP_ChangeStepDirectionMaybe(bool bFlag, int8_t Direction);
extern void APP_CycleOutputPower(void);
extern void APP_FlipVoxSwitch(void);
extern void FUN_00005830(bool bFlag);

void MAIN_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint8_t Vfo;
	uint8_t Band;

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
			gEeprom.MrChannel[Vfo] = (uint8_t)Channel;
			gEeprom.ScreenChannel[Vfo] = (uint8_t)Channel;
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
								gEeprom.ScreenChannel[Vfo] = i + 200;
								gEeprom.FreqChannel[Vfo] = i + 200;
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
				gEeprom.ScreenChannel[Vfo] = Channel;
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

	case KEY_1:
		if (6 < (gTxRadioInfo->CHANNEL_SAVE - 200)) {
			gWasFKeyPressed = false;
			g_2000036F = 1;
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			return;
		}
		Band = gTxRadioInfo->Band + 1;
		if (gSetting_350EN || Band != BAND5_350MHz) {
			if (BAND7_470MHz < Band) {
				Band = BAND1_50MHz;
			}
		} else {
			Band = BAND6_400MHz;
		}
		gTxRadioInfo->Band = Band;
		gEeprom.ScreenChannel[Vfo] = 200 + Band;
		gEeprom.FreqChannel[Vfo] = 200 + Band;
		gRequestSaveVFO = true;
		g_2000039A = 2;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		gRequestDisplayScreen = DISPLAY_MAIN;
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
				gEeprom.ScreenChannel[Vfo] = gEeprom.FreqChannel[gEeprom.TX_CHANNEL];
				gAnotherVoiceID = VOICE_ID_FREQUENCY_MODE;
				gRequestSaveVFO = true;
				g_2000039A = 2;
				break;
			}
			Channel = RADIO_FindNextChannel(gEeprom.MrChannel[gEeprom.TX_CHANNEL], 1, false, 0);
			if (Channel != 0xFF) {
				gEeprom.ScreenChannel[Vfo] = Channel;
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
			gEeprom.ScreenChannel[Vfo] = gEeprom.NoaaChannel[gEeprom.TX_CHANNEL];
		} else {
			gEeprom.ScreenChannel[Vfo] = gEeprom.FreqChannel[gEeprom.TX_CHANNEL];
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

	case KEY_8:
		gTxRadioInfo->FrequencyReverse = gTxRadioInfo->FrequencyReverse == false;
		gRequestSaveChannel = 1;
		break;

	case KEY_9:
		if (RADIO_CheckValidChannel(gEeprom.CHAN_1_CALL, false, 0)) {
			gEeprom.MrChannel[Vfo] = gEeprom.CHAN_1_CALL;
			gEeprom.ScreenChannel[Vfo] = gEeprom.CHAN_1_CALL;
			AUDIO_SetVoiceID(0, VOICE_ID_CHANNEL_MODE);
			AUDIO_SetDigitVoice(1, gEeprom.CHAN_1_CALL + 1);
			gAnotherVoiceID = 0xFE;
			gRequestSaveVFO = true;
			g_2000039A = 2;
			break;
		}
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		break;

	default:
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		g_2000036F = 1;
		gWasFKeyPressed = false;
		break;
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

void MAIN_Key_STAR(bool bKeyPressed, bool bKeyHeld)
{
	if (gNumberOffset) {
		if (!bKeyHeld && bKeyPressed) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
		return;
	}
	if (bKeyHeld || !bKeyPressed) {
		if (bKeyHeld || bKeyPressed) {
			if (!bKeyHeld) {
				return;
			}
			if (!bKeyPressed) {
				return;
			}
			FUN_00005830(0);
			return;
		}
		if (gStepDirection == 0 && gTxRadioInfo->CHANNEL_SAVE < 207) {
			g_200003BA = 1;
			memcpy(g_20000D1C, gDTMF_String, 15);
			g_200003BB = 0;
			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}
	} else {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		if (!gWasFKeyPressed) {
			gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
			return;
		}
		gWasFKeyPressed = false;
		g_2000036F = 1;
		if (gTxRadioInfo->CHANNEL_SAVE < 207) {
			gFlagStartScan = 1;
			g_20000458 = 1;
			g_20000459 = gEeprom.CROSS_BAND_RX_TX;
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
		} else {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		}
		g_20000394 = true;
	}
}

void MAIN_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction)
{
	uint8_t Channel;

	Channel = gEeprom.ScreenChannel[gEeprom.TX_CHANNEL];
	if (bKeyHeld || !bKeyPressed) {
		if (gNumberOffset) {
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
		if (gNumberOffset) {
			gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			return;
		}
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
	}

	if (gStepDirection == 0) {
		if (Channel < 207) {
			uint8_t Next;

			if (199 < Channel) {
				APP_SetFrequencyByStep(gTxRadioInfo, Direction);
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
			gEeprom.MrChannel[gEeprom.TX_CHANNEL] = Next;
			gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] = Next;
			if (!bKeyHeld) {
				AUDIO_SetDigitVoice(0, Next + 1);
				gAnotherVoiceID = 0xFE;
			}
		} else {
			Channel = 207 + NUMBER_AddWithWraparound(gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] - 207,Direction, 0, 9);
			gEeprom.NoaaChannel[gEeprom.TX_CHANNEL] = Channel;
			gEeprom.ScreenChannel[gEeprom.TX_CHANNEL] = Channel;
		}
		gRequestSaveVFO = true;
		g_2000039A = 2;
		return;
	}
	APP_ChangeStepDirectionMaybe(false, Direction);
	g_20000394 = true;
}

