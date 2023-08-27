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
#include <string.h>
#include "driver/eeprom.h"
#include "dtmf.h"
#include "misc.h"
#include "settings.h"

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

bool DTMF_GetContact(uint8_t Index, char *pContact)
{
	EEPROM_ReadBuffer(0x1C00 + (Index * 0x10), pContact, 16);
	if ((pContact[0] - ' ') >= 0x5F) {
		return false;
	}

	return true;
}

bool DTMF_FindContact(const char *pContact, char *pResult)
{
	char Contact [16];
	uint8_t i, j;

	for (i = 0; i < 16; i++) {
		if (!DTMF_GetContact(i, Contact)) {
			return false;
		}
		for (j = 0; j < 3; j++) {
			if (pContact[j] != Contact[j + 8]) {
				break;
			}
		}
		if (j == 3) {
			memcpy(pResult, Contact, 8);
			pResult[8] = 0;
			return true;
		}
	}

	return false;
}

char DTMF_GetCharacter(uint8_t Code)
{
	switch(Code) {
	case 0: case 1: case 2: case 3:
	case 4: case 5: case 6: case 7:
	case 8: case 9:
		return '0' + Code;
	case 10:
		return 'A';
	case 11:
		return 'B';
	case 12:
		return 'C';
	case 13:
		return 'D';
	case 14:
		return '*';
	case 15:
		return '#';
	}

	return -1;
}

bool DTMF_CompareMessage(const char *pDTMF, const char *pTemplate, uint8_t Size, bool bFlag)
{
	uint8_t i;

	for (i = 0; i < Size; i++) {
		if (pDTMF[i] != pTemplate[i]) {
			if (!bFlag || pDTMF[i] != gEeprom.DTMF_GROUP_CALL_CODE) {
				return false;
			}
			g_20000439 = true;
		}
	}

	return true;
}

bool DTMF_IsGroupCall(const char *pDTMF, uint32_t Size)
{
	uint32_t i;

	for (i = 0; i < Size; i++) {
		if (pDTMF[i] == gEeprom.DTMF_GROUP_CALL_CODE) {
			break;
		}
	}
	if (i != Size) {
		return true;
	}

	return false;
}

