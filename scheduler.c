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

#include "audio.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"

static volatile uint32_t gGlobalSysTickCounter;

void SystickHandler(void);

void SystickHandler(void)
{
	gGlobalSysTickCounter++;
	gNextTimeslice = true;
	if ((gGlobalSysTickCounter % 50) == 0) {
		gNextTimeslice500ms = true;
		if (gTxTimerCountdown != 0) {
			gTxTimerCountdown--;
			if (gTxTimerCountdown == 0) {
				gTxTimeoutReached = true;
			}
		}
	}
	if ((gGlobalSysTickCounter & 3) == 0) {
		gNextTimeslice40ms = true;
	}
	if (gSystickCountdown2 != 0) {
		gSystickCountdown2--;
	}
	if (gSystickCountdown3 != 0) {
		gSystickCountdown3--;
	}
	if (gSystickCountdown4 != 0) {
		gSystickCountdown4--;
	}
	if (gCurrentFunction == FUNCTION_0 && gBatterySaveCountdown != 0) {
		gBatterySaveCountdown--;
		if (gBatterySaveCountdown == 0) {
			gSchedulePowerSave = true;
		}
	}
	if (gCurrentFunction == FUNCTION_POWER_SAVE && gBatterySave != 0) {
		gBatterySave--;
		if (gBatterySave == 0) {
			gBatterySaveCountdownExpired = true;
		}
	}
	if (gStepDirection == 0 && g_20000381 == 0 && gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
		if (gCurrentFunction != FUNCTION_MONITOR && gCurrentFunction != FUNCTION_TRANSMIT) {
			if (gCurrentFunction != FUNCTION_4) {
				if (g_2000033A != 0) {
					g_2000033A--;
					if (g_2000033A == 0) {
						gSystickFlag7 = true;
					}
				}
			}
		}
	}

	if (gStepDirection == 0 && g_20000381 == 0 && gEeprom.DUAL_WATCH == DUAL_WATCH_OFF) {
		if (gIsNoaaMode && gCurrentFunction != FUNCTION_MONITOR && gCurrentFunction != FUNCTION_TRANSMIT) {
			if (gCurrentFunction != FUNCTION_4) {
				if (g_20000356 != 0) {
					g_20000356--;
					if (g_20000356 == 0) {
						gSystickFlag8 = true;
					}
				}
			}
		}
	}

	if (gStepDirection != 0 || g_20000381 == 1) {
		if (gCurrentFunction != FUNCTION_MONITOR && gCurrentFunction != FUNCTION_TRANSMIT) {
			if (ScanPauseDelayIn10msec != 0) {
				ScanPauseDelayIn10msec--;
				if (ScanPauseDelayIn10msec == 0) {
					gSystickFlag9 = true;
				}
			}
		}
	}

	if (g_20000342 != 0) {
		g_20000342--;
		if (g_20000342 == 0) {
			gSystickFlag10 = true;
		}
	}

	if (gCountdownToPlayNextVoice != 0) {
		gCountdownToPlayNextVoice--;
		if (gCountdownToPlayNextVoice == 0) {
			gFlagPlayQueuedVoice = true;
		}
	}

	if (gFM_Step != 0 && gCurrentFunction != FUNCTION_MONITOR) {
		if (gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_4) {
			if (gFmPlayCountdown != 0) {
				gFmPlayCountdown--;
				if (gFmPlayCountdown == 0) {
					gScheduleFM = true;
				}
			}
		}
	}
	if (gSystickCountdown11 != 0) {
		gSystickCountdown11--;
	}
}

