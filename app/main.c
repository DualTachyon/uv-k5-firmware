#include "app/main.h"
#include "audio.h"
#include "gui.h"
#include "misc.h"

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

