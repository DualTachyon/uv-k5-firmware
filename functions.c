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

#include <string.h>
#include "dcs.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"

uint8_t gCurrentFunction;

void FUNCTION_Init(void)
{
	if (gInfoCHAN_A->CHANNEL_SAVE < 207) {
		gCopyOfCodeType = gCodeType;
		if (g_20000381 == 0) {
			if (gInfoCHAN_A->_0x0033 == true) {
				gCopyOfCodeType = 0;
			} else {
				gCopyOfCodeType = gInfoCHAN_A->pDCS_Current->CodeType;
			}
		}
	} else {
		gCopyOfCodeType = CODE_TYPE_CONTINUOUS_TONE;
	}
	g_200003AA = 0;
	g_2000042F = 0;
	memset(g_20000D0C, 0, sizeof(g_20000D0C));
	g_200003AE = 0;
	g_200003AB = 0;
	g_200003AD = 0;
	g_200003AF = 0;
	g_200003B0 = 0;
	g_20000342 = 0;
	gSystickFlag10 = 0;
	g_20000375 = 0;
	g_20000376 = 0;
	gSystickCountdown4 = 0;
	gSystickCountdown3 = 0;
	g_20000377 = 0;
	gSystickCountdown2 = 0;
}

void FUNCTION_Select(uint8_t Function)
{
}

