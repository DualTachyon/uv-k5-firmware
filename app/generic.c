#include "app/generic.h"
#include "audio.h"
#include "dtmf.h"
#include "external/printf/printf.h"
#include "fm.h"
#include "functions.h"
#include "gui.h"
#include "misc.h"
#include "settings.h"

extern void FUN_0000773c(void);
extern void PlayFMRadio(void);

void GENERIC_Key_F(bool bKeyPressed, bool bKeyHeld)
{
	if (gNumberOffset != 0) {
		if (!bKeyHeld && bKeyPressed) {
			g_20000396 = 2;
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
			if (gEeprom.KEY_LOCK) {
				gAnotherVoiceID = VOICE_ID_M_LOCK;
			} else {
				gAnotherVoiceID = VOICE_ID_LOCK;
			}
			gEeprom.KEY_LOCK = !gEeprom.KEY_LOCK;
			g_2000039E = true;
		} else {
			if ((gFmMute || gScreenToDisplay != DISPLAY_MAIN) && gScreenToDisplay != DISPLAY_FM) {
				return;
			}
			gWasFKeyPressed = !gWasFKeyPressed;
			if (!gWasFKeyPressed) {
				gAnotherVoiceID = VOICE_ID_CANCEL;
			}
			g_2000036F = true;
		}
	} else {
		if (gScreenToDisplay != DISPLAY_FM) {
			g_20000396 = 1;
			return;
		}
		if (g_20000390 == 0) {
			g_20000396 = 1;
			return;
		}
		g_20000396 = 2;
		g_20000394 = true;
	}
}

void GENERIC_Key_PTT(bool bKeyPressed)
{
	gNumberOffset = 0;
	if (!bKeyPressed) {
		if (gScreenToDisplay == DISPLAY_MAIN) {
			if (gCurrentFunction == FUNCTION_TRANSMIT) {
				if (g_200003FD == 1) {
					//TalkRelatedCode();
					if (gEeprom.REPEATER_TAIL_TONE_ELIMINATION == 0) {
						FUNCTION_Select(FUNCTION_0);
					} else {
						gRTTECountdown = gEeprom.REPEATER_TAIL_TONE_ELIMINATION * 10;
					}
				}
				g_200003FD = 0;
				g_200003B4 = 0;
			}
			RADIO_SomethingElse(0);
			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}
		gNumberOffset = 0;
		return;
	}

	if (gStepDirection != 0) {
		FUN_0000773c();
		gPttDebounceCounter = 0;
		gPttIsPressed = false;
		gRequestDisplayScreen = DISPLAY_MAIN;
		return;
	}

	if (g_20000390 == 0) {
		if (g_20000381 == 0) {
			if (gScreenToDisplay == DISPLAY_MENU) {
				gRequestDisplayScreen = DISPLAY_MAIN;
				gNumberOffset = 0;
				gPttIsPressed = false;
				gPttDebounceCounter = 0;
				return;
			}
			if (gScreenToDisplay == DISPLAY_FM) {
				gRequestDisplayScreen = DISPLAY_MAIN;
				gNumberOffset = 0;
				gPttIsPressed = false;
				gPttDebounceCounter = 0;
				return;
			}
			if (gScreenToDisplay != DISPLAY_SCANNER) {
				if (gCurrentFunction == FUNCTION_TRANSMIT && gRTTECountdown == 0) {
					gNumberOffset = 0;
					return;
				}
				g_200003A0 = 1;
				if (g_200003BA == 1) {
					if (g_200003BB || g_200003C0) {
						if (g_200003BB == 0) {
							g_200003BB = g_200003C0;
						}
						g_20000D1C[g_200003BB] = 0;
						if (g_200003BB == 3) {
							g_20000438 = DTMF_IsGroupCall(g_20000D1C, 3);
						} else {
							g_20000438 = 2;
						}
						sprintf(gDTMF_String, "%s", g_20000D1C);
						g_200003C0 = g_200003BB;
						g_200003BE = 1;
						g_CalloutAndDTMF_State = 0;
					}
					gRequestDisplayScreen = DISPLAY_MAIN;
					g_200003BA = 0;
					g_200003BB = 0;
					return;
				}
				gRequestDisplayScreen = DISPLAY_MAIN;
				g_200003A0 = 1;
				gNumberOffset = 0;
				return;
			}
			gRequestDisplayScreen = DISPLAY_MAIN;
			gEeprom.CROSS_BAND_RX_TX = g_20000459;
			g_2000036F = 1;
			g_200003A2 = 1;
			g_2000039A = 2;
			g_2000039B = 1;
		} else {
			RADIO_Whatever();
			gRequestDisplayScreen = DISPLAY_MENU;
		}
	} else {
		PlayFMRadio();
		gRequestDisplayScreen = DISPLAY_FM;
	}
	gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
	g_20000395 = 1;
}

