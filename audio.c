#include "audio.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"
#include "fm.h"
#include "misc.h"
#include "settings.h"

void AUDIO_PlayBeep(BEEP_Type_t Beep)
{
	uint16_t ToneConfig;
	uint16_t ToneFrequency;
	uint16_t Duration;

	if (Beep != BEEP_500HZ_60MS_DOUBLE_BEEP && Beep != BEEP_440HZ_500MS && !gEeprom.BEEP_CONTROL) {
		return;
	}

#if 0
	if (gScreenToDisplay == DISPLAY_AIRCOPY) {
		return;
	}
	if (gCurrentFunction == FUNCTION_4) {
		return;
	}
	if (gCurrentFunction == FUNCTION_2) {
		return;
	}
#endif

	ToneConfig = BK4819_GetRegister(BK4819_REG_71);

	GPIO_ClearBit(&GPIOC->DATA, 4);
#if 0
	if (gCurrentFunction == FUNCTION_5 && gThisCanEnable_BK4819_Rxon) {
		BK4819_RX_TurnOn();
	}
#endif
	if (gFmMute == true) {
		BK1080_Mute(true);
	}
	SYSTEM_DelayMs(20);
	switch (Beep) {
	case BEEP_1KHZ_60MS_OPTIONAL:
		ToneFrequency = 1000;
		break;
	case BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
	case BEEP_500HZ_60MS_DOUBLE_BEEP:
		ToneFrequency = 500;
		break;
	default:
		ToneFrequency = 440;
		break;
	}
	BK4819_PlayTone(ToneFrequency, true);
	SYSTEM_DelayMs(2);
	GPIO_SetBit(&GPIOC->DATA, 4);
	SYSTEM_DelayMs(60);

	switch (Beep) {
	case BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
	case BEEP_500HZ_60MS_DOUBLE_BEEP:
		BK4819_ExitTxMute();
		SYSTEM_DelayMs(60);
		BK4819_EnterTxMute();
		SYSTEM_DelayMs(20);
		// Fallthrough
	case BEEP_1KHZ_60MS_OPTIONAL:
		BK4819_ExitTxMute();
		Duration = 60;
		break;
	case BEEP_440HZ_500MS:
	default:
		BK4819_ExitTxMute();
		Duration = 500;
		break;
	}

	SYSTEM_DelayMs(Duration);
	BK4819_EnterTxMute();
	SYSTEM_DelayMs(20);
	GPIO_ClearBit(&GPIOC->DATA,4);
#if 0
	g_200003B6 = 0x50;
#endif
	SYSTEM_DelayMs(5);
	BK4819_TurnsOffTones_TurnsOnRX();
	SYSTEM_DelayMs(5);
	BK4819_WriteRegister(BK4819_REG_71, ToneConfig);
	if (g_2000036B == 1) {
		GPIO_SetBit(&GPIOC->DATA, 4);
	}
	if (gFmMute == true) {
		BK1080_Mute(false);
	}
#if 0
	if (gCurrentFunction == FUNCTION_5 && gThisCanEnable_BK4819_Rxon) {
		BK4819_Sleep();
	}
#endif
}

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

