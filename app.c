#include "audio.h"
#include "app.h"
#include "misc.h"

void APP_Update(void)
{
	if (gFlagPlayQueuedVoice) {
		AUDIO_PlayQueuedVoice();
		gFlagPlayQueuedVoice = false;
	}

	if (g_2000037E == 1) {
		return;
	}
	if (gIsFmRadioEnabled) {
		return;
	}
}

void APP_TimeSlice10ms(void)
{
}

void APP_TimeSlice500ms(void)
{
}

