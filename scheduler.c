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
#include "battery.h"
#include "functions.h"
#include "gui.h"
#include "misc.h"
#include "settings.h"

static uint32_t gGlobalSysTickCounter;

void SystickHandler(void)
{
	gGlobalSysTickCounter += 1;
	gMaybeVsync = 1;
	if ((gGlobalSysTickCounter % 50) == 0) {
		g_2000032E = 1;
		if (g_2000033E != 0) {
			g_2000033E--;
			if (g_2000033E == 0) {
				gSystickFlag0 = 1;
			}
		}
	}
	if ((gGlobalSysTickCounter & 3) == 0) {
		gSystickFlag1 = 1;
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
	if (gCurrentFunction == FUNCTION_0 && g_2000032E != 0) {
		g_2000032E--;
		if (g_2000032E == 0) {
			gSystickFlag5 = 1;
		}
	}
	if (gCurrentFunction == FUNCTION_POWER_SAVE && gBatterySave != 0) {
		gBatterySave--;
		if (gBatterySave == 0) {
			gSystickFlag6 = 1;
		}
	}
	if (g_20000380 == 0 && g_20000381 == 0 && gEeprom.DUAL_WATCH != 0) {
		if (gCurrentFunction != FUNCTION_2 && gCurrentFunction != FUNCTION_TRANSMIT) {
			if (gCurrentFunction != FUNCTION_4) {
				if (g_2000033A != 0) {
					g_2000033A--;
					if (g_2000033A == 0) {
						gSystickFlag7 = 1;
					}
				}
			}
		}
	}

	if (g_20000380 == 0 && g_20000381 == 0 && gEeprom.DUAL_WATCH == 0) {
		if (gIsNoaaMode && gCurrentFunction != FUNCTION_2 && gCurrentFunction != FUNCTION_TRANSMIT) {
			if (gCurrentFunction != FUNCTION_4) {
				if (g_20000356 != 0) {
					g_20000356--;
					if (g_20000356 == 0) {
						gSystickFlag8 = 1;
					}
				}
			}
		}
	}

	if (g_20000380 != 0 || g_20000381 == 1) {
		if (gCurrentFunction != FUNCTION_2 && gCurrentFunction != FUNCTION_TRANSMIT) {
			if (ScanPauseDelayIn10msec != 0) {
				ScanPauseDelayIn10msec--;
				if (ScanPauseDelayIn10msec == 0) {
					gSystickFlag9 = 1;
				}
			}
		}
	}

	if (g_20000342 != 0) {
		g_20000342--;
		if (g_20000342 == 0) {
			gSystickFlag10 = 1;
		}
	}

	if (gCountdownToPlayNextVoice != 0) {
		gCountdownToPlayNextVoice--;
		if (gCountdownToPlayNextVoice == 0) {
			gFlagPlayQueuedVoice = true;
		}
	}

	if (g_20000390 != 0 && gCurrentFunction != FUNCTION_2) {
		if (gCurrentFunction != FUNCTION_TRANSMIT && gCurrentFunction != FUNCTION_4) {
			if (g_2000034C != 0) {
				g_2000034C--;
				if (g_2000034C == 0) {
					gSystickFlag11 = 1;
				}
			}
		}
	}
	if (gSystickCountdown11 != 0) {
		gSystickCountdown11--;
	}
}

