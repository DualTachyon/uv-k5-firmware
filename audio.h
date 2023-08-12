#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

enum BEEP_Type_t {
	BEEP_1KHZ_60MS_OPTIONAL = 1U,
	BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL = 2U,
	BEEP_440HZ_500MS = 3U,
	BEEP_500HZ_60MS_DOUBLE_BEEP = 4U,
};

typedef enum BEEP_Type_t BEEP_Type_t;

void AUDIO_PlayBeep(BEEP_Type_t Beep);
void AUDIO_PlayVoice(uint8_t VoiceID);

#endif

