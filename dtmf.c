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

#include <ctype.h>
#include "dtmf.h"

char gDTMF_String[15];

bool DTMF_ValidateCodes(char *pCode, uint8_t Size)
{
	uint8_t i;

	if (pCode[0] == 0xFF || pCode[0] == 0) {
		return false;
	}

	for (i = 0; i < Size; i++) {
		if (pCode[i] == 0xFF || pCode[i] == 0) {
			pCode[i] = 0;
			break;
		}
		if (!isdigit((int)pCode[i]) && (pCode[i] < 'A' || pCode[i] > 'D') && pCode[i] != '*' && pCode[i] != '#') {
			return false;
		}
	}

	return true;
}

