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

#include "aircopy.h"
#include "driver/bk4819.h"
#include "driver/crc.h"
#include "driver/eeprom.h"
#include "misc.h"
#include "radio.h"

static const uint16_t Obfuscation[8] = { 0x6C16, 0xE614, 0x912E, 0x400D, 0x3521, 0x40D5, 0x0313, 0x80E9 };

AIRCOPY_State_t gAircopyState;
uint16_t gAirCopyBlockNumber;
uint16_t gErrorsDuringAirCopy;
uint8_t gAirCopyIsSendMode;

uint16_t g_FSK_Buffer[36];

void AIRCOPY_SendMessage(void)
{
	uint8_t i;

	g_FSK_Buffer[1] = (gAirCopyBlockNumber & 0x3FF) << 6;
	EEPROM_ReadBuffer(g_FSK_Buffer[1], &g_FSK_Buffer[2], 64);
	g_FSK_Buffer[34] = CRC_Calculate(&g_FSK_Buffer[1], 2 + 64);
	for (i = 0; i < 34; i++) {
		g_FSK_Buffer[i + 1] ^= Obfuscation[i % 8];
	}
	if (++gAirCopyBlockNumber >= 0x78) {
		gAircopyState = AIRCOPY_COMPLETE;
	}
	RADIO_PrepareTransmit();
	BK4819_SendFSKData(g_FSK_Buffer);
	BK4819_SetupPowerAmplifier(0, 0);
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1, false);
	gAircopySendCountdown = 0x1e;
}

void AIRCOPY_StorePacket(void)
{
	uint16_t Status;

	if (gFSKWriteIndex < 36) {
		return;
	}

	gFSKWriteIndex = 0;
	gUpdateDisplay = true;
	Status = BK4819_GetRegister(BK4819_REG_0B);
	BK4819_PrepareFSKReceive();
	if (Status & 0x0010U && g_FSK_Buffer[0] == 0xABCD && g_FSK_Buffer[35] == 0xDCBA) {
		uint16_t CRC;
		uint8_t i;

		for (i = 0; i < 34; i++) {
			g_FSK_Buffer[i + 1] ^= Obfuscation[i % 8];
		}

		CRC = CRC_Calculate(&g_FSK_Buffer[1], 2 + 64);
		if (g_FSK_Buffer[34] == CRC) {
			const uint16_t *pData;
			uint16_t Offset;

			Offset = g_FSK_Buffer[1];
			if (Offset < 0x1E00) {
				pData = &g_FSK_Buffer[2];
				for (i = 0; i < 8; i++) {
					EEPROM_WriteBuffer(Offset, pData);
					pData += 4;
					Offset += 8;
				}
				if (Offset == 0x1E00) {
					gAircopyState = AIRCOPY_COMPLETE;
				}
				gAirCopyBlockNumber++;
				return;
			}
		}
	}
	gErrorsDuringAirCopy++;
}

