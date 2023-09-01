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

#include "app/fm.h"
#include "app/generic.h"
#include "audio.h"
#include "driver/keyboard.h"
#include "dtmf.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "misc.h"
#include "settings.h"
#include "ui/inputbox.h"
#include "ui/ui.h"

extern void FUN_0000773c(void);
extern void PlayFMRadio(void);
extern void TalkRelatedCode(void);

void GENERIC_Key_F(bool bKeyPressed, bool bKeyHeld)
{
	if (gInputBoxIndex) {
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
				gAnotherVoiceID = VOICE_ID_UNLOCK;
			} else {
				gAnotherVoiceID = VOICE_ID_LOCK;
			}
			gEeprom.KEY_LOCK = !gEeprom.KEY_LOCK;
			gRequestSaveSettings = true;
		} else {
			if ((gFmRadioMode || gScreenToDisplay != DISPLAY_MAIN) && gScreenToDisplay != DISPLAY_FM) {
				return;
			}
			gWasFKeyPressed = !gWasFKeyPressed;
			if (!gWasFKeyPressed) {
				gAnotherVoiceID = VOICE_ID_CANCEL;
			}
			gUpdateStatus = true;
		}
	} else {
		if (gScreenToDisplay != DISPLAY_FM) {
			g_20000396 = 1;
			return;
		}
		if (gFM_Step == 0) {
			g_20000396 = 1;
			return;
		}
		g_20000396 = 2;
		g_20000394 = true;
	}
}

void GENERIC_Key_PTT(bool bKeyPressed)
{
	gInputBoxIndex = 0;
	if (!bKeyPressed) {
		if (gScreenToDisplay == DISPLAY_MAIN) {
			if (gCurrentFunction == FUNCTION_TRANSMIT) {
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
				g_200003FD = 0;
				g_200003B4 = 0;
			}
			RADIO_SomethingElse(0);
			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}
		gInputBoxIndex = 0;
		return;
	}

	if (gStepDirection) {
		FUN_0000773c();
		gPttDebounceCounter = 0;
		gPttIsPressed = false;
		gRequestDisplayScreen = DISPLAY_MAIN;
		return;
	}

	if (gFM_Step == 0) {
		if (g_20000381 == 0) {
			if (gScreenToDisplay == DISPLAY_MENU || gScreenToDisplay == DISPLAY_FM) {
				gRequestDisplayScreen = DISPLAY_MAIN;
				gInputBoxIndex = 0;
				gPttIsPressed = false;
				gPttDebounceCounter = 0;
				return;
			}
			if (gScreenToDisplay != DISPLAY_SCANNER) {
				if (gCurrentFunction == FUNCTION_TRANSMIT && gRTTECountdown == 0) {
					gInputBoxIndex = 0;
					return;
				}
				g_200003A0 = 1;
				if (gDTMF_InputMode) {
					if (gDTMF_InputIndex || g_200003C0) {
						if (gDTMF_InputIndex == 0) {
							gDTMF_InputIndex = g_200003C0;
						}
						gDTMF_InputBox[gDTMF_InputIndex] = 0;
						if (gDTMF_InputIndex == 3) {
							gDTMF_CallMode = DTMF_IsGroupCall(gDTMF_InputBox, 3);
						} else {
							gDTMF_CallMode = DTMF_CALL_MODE_2;
						}
						sprintf(gDTMF_String, "%s", gDTMF_InputBox);
						g_200003C0 = gDTMF_InputIndex;
						gDTMF_ReplyState = DTMF_REPLY_ANI;
						gDTMF_State = DTMF_STATE_0;
					}
					gRequestDisplayScreen = DISPLAY_MAIN;
					gDTMF_InputMode = false;
					gDTMF_InputIndex = 0;
					return;
				}
				gRequestDisplayScreen = DISPLAY_MAIN;
				g_200003A0 = 1;
				gInputBoxIndex = 0;
				return;
			}
			gRequestDisplayScreen = DISPLAY_MAIN;
			gEeprom.CROSS_BAND_RX_TX = gBackupCROSS_BAND_RX_TX;
			gUpdateStatus = true;
			gFlagStopScan = true;
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			g_2000039B = 1;
		} else {
			RADIO_Whatever();
			gRequestDisplayScreen = DISPLAY_MENU;
		}
	} else {
		FM_Play();
		gRequestDisplayScreen = DISPLAY_FM;
	}
	gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
	g_20000395 = 1;
}

