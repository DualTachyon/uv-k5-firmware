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
#include "app/fm.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "dtmf.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/ui.h"

char gDTMF_String[15];
char gDTMF_InputBox[15];
char gDTMF_Received[16];
bool gIsDtmfContactValid;
char gDTMF_ID[4];
char gDTMF_Caller[4];
char gDTMF_Callee[4];
DTMF_State_t gDTMF_State;
bool gDTMF_DecodeRing;
uint8_t gDTMF_DecodeRingCountdown;
uint8_t gDTMFChosenContact;
uint8_t gDTMF_WriteIndex;
uint8_t gDTMF_AUTO_RESET_TIME;
uint8_t gDTMF_InputIndex;
bool gDTMF_InputMode;
uint8_t gDTMF_RecvTimeout;
DTMF_CallState_t gDTMF_CallState;
DTMF_ReplyState_t gDTMF_ReplyState;
DTMF_CallMode_t gDTMF_CallMode;

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
		if ((pCode[i] < '0' || pCode[i] > '9') && (pCode[i] < 'A' || pCode[i] > 'D') && pCode[i] != '*' && pCode[i] != '#') {
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
		return '0' + (char)Code;
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

	return 0xFF;
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

void DTMF_Append(char Code)
{
	if (gDTMF_InputIndex == 0) {
		memset(gDTMF_InputBox, '-', sizeof(gDTMF_InputBox));
		gDTMF_InputBox[14] = 0;
	} else if (gDTMF_InputIndex >= sizeof(gDTMF_InputBox)) {
		return;
	}
	gDTMF_InputBox[gDTMF_InputIndex++] = Code;
}

void DTMF_HandleRequest(void)
{
	char String[20];
	uint8_t Offset;

	if (!gDTMF_RequestPending) {
		return;
	}

	gDTMF_RequestPending = false;

	if (gStepDirection || g_20000381) {
		return;
	}

	if (!gRxInfo->DTMF_DECODING_ENABLE && !gSetting_KILLED) {
		return;
	}

	if (gDTMF_WriteIndex >= 9) {
		Offset = gDTMF_WriteIndex - 9;
		sprintf(String, "%s%c%s", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE, gEeprom.KILL_CODE);
		if (DTMF_CompareMessage(gDTMF_Received + Offset, String, 9, true)) {
			if (gEeprom.PERMIT_REMOTE_KILL) {
				gSetting_KILLED = true;
				SETTINGS_SaveSettings();
				gDTMF_ReplyState = DTMF_REPLY_AB;
				if (gFmRadioMode) {
					FM_TurnOff();
					GUI_SelectNextDisplay(DISPLAY_MAIN);
				}
			} else {
				gDTMF_ReplyState = DTMF_REPLY_UP_CODE;
			}
		} else {
			sprintf(String, "%s%c%s", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE, gEeprom.REVIVE_CODE);
			if (DTMF_CompareMessage(gDTMF_Received + Offset, String, 9, true)) {
				gSetting_KILLED = false;
				SETTINGS_SaveSettings();
				gDTMF_ReplyState = DTMF_REPLY_AB;
			}
		}
		gDTMF_CallState = DTMF_CALL_STATE_NONE;
		gUpdateDisplay = true;
		gUpdateStatus = true;
		return;
	}

	if (gDTMF_WriteIndex >= 2) {
		if (DTMF_CompareMessage(gDTMF_Received + (gDTMF_WriteIndex - 2), "AB", 2, true)) {
			gDTMF_State = DTMF_STATE_TX_SUCC;
			gUpdateDisplay = true;
			return;
		}
	}
	if (gDTMF_CallState == DTMF_CALL_STATE_1 && gDTMF_CallMode == DTMF_CALL_MODE_NOT_GROUP && gDTMF_WriteIndex >= 9) {
		Offset = gDTMF_WriteIndex - 9;
		sprintf(String, "%s%c%s", gDTMF_String, gEeprom.DTMF_SEPARATE_CODE, "AAAAA");
		if (DTMF_CompareMessage(gDTMF_Received + Offset, String, 9, false)) {
			gDTMF_State = DTMF_STATE_CALL_OUT_RSP;
			gUpdateDisplay = true;
		}
	}
	if (gSetting_KILLED) {
		return;
	}
	if (gDTMF_CallState != DTMF_CALL_STATE_NONE) {
		return;
	}
	if (gDTMF_WriteIndex < 7) {
		return;
	}
	Offset = gDTMF_WriteIndex - 7;
	sprintf(String, "%s%c", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE);
	g_20000439 = false;
	if (!DTMF_CompareMessage(gDTMF_Received + Offset, String, 4, true)) {
		return;
	}
	gDTMF_CallState = DTMF_CALL_STATE_RECEIVED;
	memcpy(gDTMF_Callee, gDTMF_Received + Offset, 3);
	memcpy(gDTMF_Caller, gDTMF_Received + Offset + 4, 3);

	gUpdateDisplay = true;

	switch (gEeprom.DTMF_DECODE_RESPONSE) {
	case 3:
		gDTMF_DecodeRing = true;
		gDTMF_DecodeRingCountdown = 20;
		// Fallthrough
	case 2:
		gDTMF_ReplyState = DTMF_REPLY_AAAAA;
		break;
	case 1:
		gDTMF_DecodeRing = true;
		gDTMF_DecodeRingCountdown = 20;
		break;
	default:
		gDTMF_DecodeRing = false;
		gDTMF_ReplyState = DTMF_REPLY_UP_CODE;
		break;
	}

	if (g_20000439) {
		gDTMF_ReplyState = DTMF_REPLY_UP_CODE;
	}
}

void DTMF_Reply(void)
{
	char String[20];
	const char *pString;
	uint16_t Delay;

	switch (gDTMF_ReplyState) {
	case DTMF_REPLY_ANI:
		if (gDTMF_CallMode == DTMF_CALL_MODE_2) {
			pString = gDTMF_String;
		} else {
			sprintf(String, "%s%c%s", gDTMF_String, gEeprom.DTMF_SEPARATE_CODE, gEeprom.ANI_DTMF_ID);
			pString = String;
		}
		break;

	case DTMF_REPLY_AB:
		pString = "AB";
		break;

	case DTMF_REPLY_AAAAA:
		sprintf(String, "%s%c%s", gEeprom.ANI_DTMF_ID, gEeprom.DTMF_SEPARATE_CODE, "AAAAA");
		pString = String;
		break;

	default:
		if (gDTMF_CallState != DTMF_CALL_STATE_NONE || (gCrossTxRadioInfo->DTMF_PTT_ID_TX_MODE != PTT_ID_BOT && gCrossTxRadioInfo->DTMF_PTT_ID_TX_MODE != PTT_ID_BOTH)) {
			gDTMF_ReplyState = DTMF_REPLY_UP_CODE;
			return;
		}
		pString = gEeprom.DTMF_UP_CODE;
		break;
	}

	gDTMF_ReplyState = DTMF_REPLY_UP_CODE;
	Delay = gEeprom.DTMF_PRELOAD_TIME;
	if (gEeprom.DTMF_SIDE_TONE) {
		GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		gEnableSpeaker = true;
		Delay = gEeprom.DTMF_PRELOAD_TIME;
		if (gEeprom.DTMF_PRELOAD_TIME < 60) {
			Delay = 60;
		}
	}
	SYSTEM_DelayMs(Delay);

	BK4819_EnterDTMF_TX(gEeprom.DTMF_SIDE_TONE);

	BK4819_PlayDTMFString(
		pString,
		1,
		gEeprom.DTMF_FIRST_CODE_PERSIST_TIME,
		gEeprom.DTMF_HASH_CODE_PERSIST_TIME,
		gEeprom.DTMF_CODE_PERSIST_TIME,
		gEeprom.DTMF_CODE_INTERVAL_TIME);

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

	gEnableSpeaker = false;
	BK4819_ExitDTMF_TX(false);
}

