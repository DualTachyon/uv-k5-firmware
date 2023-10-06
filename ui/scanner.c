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

#include <stdbool.h>
#include <string.h>
#include "app/scanner.h"
#include "dcs.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "ui/helper.h"
#include "ui/scanner.h"

void UI_DisplayScanner(void)
{
	char String[16];
	bool bCentered;
	uint8_t Start;

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	memset(String, 0, sizeof(String));

	if (gScanSingleFrequency || (gScanCssState != SCAN_CSS_STATE_OFF && gScanCssState != SCAN_CSS_STATE_FAILED)) {
		sprintf(String, "FREQ:%d.%05d", gScanFrequency / 100000, gScanFrequency % 100000);
	} else {
		sprintf(String, "FREQ:**.*****");
	}
	UI_PrintString(String, 2, 127, 1, 8, false);
	memset(String, 0, sizeof(String));

	if (gScanCssState < SCAN_CSS_STATE_FOUND || !gScanUseCssResult) {
		sprintf(String, "CTC:******");
	} else if (gScanCssResultType == CODE_TYPE_CONTINUOUS_TONE) {
		sprintf(String, "CTC:%d.%dHz", CTCSS_Options[gScanCssResultCode] / 10, CTCSS_Options[gScanCssResultCode] % 10);
	} else {
		sprintf(String, "DCS:D%03oN", DCS_Options[gScanCssResultCode]);
	}
	UI_PrintString(String, 2, 127, 3, 8, false);
	memset(String, 0, sizeof(String));

	if (gScannerEditState == 2) {
		strcpy(String, "SAVE?");
		Start = 0;
		bCentered = 1;
	} else {
		if (gScannerEditState == 1) {
			strcpy(String, "SAVE:");
			UI_GenerateChannelStringEx(String + 5, gShowChPrefix, gScanChannel);
		} else if (gScanCssState < SCAN_CSS_STATE_FOUND) {
			strcpy(String, "SCAN");
			memset(String + 4, '.', (gScanProgressIndicator & 7) + 1);
		} else {
			if (gScanCssState == SCAN_CSS_STATE_FOUND) {
				strcpy(String, "SCAN CMP.");
			} else {
				strcpy(String, "SCAN FAIL.");
			}
		}
		Start = 2;
		bCentered = 0;
	}

	UI_PrintString(String, Start, 127, 5, 8, bCentered);
	ST7565_BlitFullScreen();
}

