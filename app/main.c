#include "app/main.h"
#include "audio.h"
#include "fm.h"
#include "gui.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

extern void APP_SwitchToFM(void);
extern void FUN_0000773c(void);
extern void APP_AddStepToFrequency(VFO_Info_t *pInfo, uint8_t Step);
extern void APP_ChangeStepDirectionMaybe(bool bFlag, uint8_t Direction);

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
			gEeprom.EEPROM_0E86_0E87[gEeprom.TX_CHANNEL] = Channel;
			gEeprom.VfoChannel[gEeprom.TX_CHANNEL] = Channel;
		}
		gRequestSaveVFO = true;
		g_2000039A = 2;
		return;
	}
	APP_ChangeStepDirectionMaybe(false, Direction);
	g_20000394 = true;
}

