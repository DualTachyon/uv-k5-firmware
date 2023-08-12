#include "audio.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"

void AUDIO_PlayVoice(uint8_t VoiceID)
{
	uint8_t i;

	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
	SYSTEM_DelayMs(7);
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
	for (i = 0; i < 8; i++) {
		if ((VoiceID & 0x80U) == 0) {
			GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_1);
		} else {
			GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_VOICE_1);
		}
		SYSTICK_DelayUs(1200);
		GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
		SYSTICK_DelayUs(1200);
		GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);
		VoiceID <<= 1;
	}
}

