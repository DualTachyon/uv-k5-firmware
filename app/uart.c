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
#include "app/uart.h"
#include "bsp/dp32g030/dma.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/crc.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "misc.h"
#include "sram-overlay.h"

#define DMA_INDEX(x, y) (((x) + (y)) % sizeof(UART_DMA_Buffer))

typedef struct {
	uint16_t ID;
	uint16_t Size;
} Header_t;

typedef struct {
	uint8_t Padding[2];
	uint16_t ID;
} Footer_t;

static const uint8_t Obfuscation[16] = { 0x16, 0x6C, 0x14, 0xE6, 0x2E, 0x91, 0x0D, 0x40, 0x21, 0x35, 0xD5, 0x40, 0x13, 0x03, 0xE9, 0x80 };

static union {
	uint8_t Buffer[256];
	struct {
		Header_t Header;
		uint8_t Data[252];
	};
} UART_Command;

typedef struct {
	Header_t Header;
	uint32_t Timestamp;
} CMD_0514_t;

typedef struct {
	char Version[16];
	bool bHasCustomAesKey;
	bool bIsInLockScreen;
	uint8_t Padding[2];
	uint32_t Random[4];
} REPLY_0514_t;

typedef struct {
	Header_t Header;
	uint16_t Offset;
	uint8_t Size;
	uint8_t Padding;
	uint32_t Timestamp;
} CMD_051B_t;

typedef struct {
	uint16_t Offset;
	uint8_t Size;
	uint8_t Padding;
	uint8_t Data[128];
} REPLY_051B_t;

static uint16_t gUART_WriteIndex;
static bool bIsEncrypted;

static void SendReply(void *pReply, uint16_t Size)
{
	Header_t Header;
	Footer_t Footer;
	uint8_t *pBytes;
	uint16_t i;

	if (bIsEncrypted) {
		pBytes = (uint8_t *)pReply;
		for (i = 0; i < Size; i++) {
			pBytes[i] ^= Obfuscation[i % 16];
		}
	}

	Header.ID = 0xCDAB;
	Header.Size = Size;
	UART_Send(&Header, sizeof(Header));
	UART_Send(pReply, Size);
	if (bIsEncrypted) {
		Footer.Padding[0] = Obfuscation[(Size + 0) % 16] ^ 0xFF;
		Footer.Padding[1] = Obfuscation[(Size + 1) % 16] ^ 0xFF;
	} else {
		Footer.Padding[0] = 0xFF;
		Footer.Padding[1] = 0xFF;
	}
	Footer.ID = 0xBADC;

	UART_Send(&Footer, sizeof(Footer));
}

static void CMD_0514(const uint8_t *pBuffer)
{
	//const CMD_0514_t *pCmd = (const CMD_0514_t *)pBuffer;
	struct {
		Header_t Header;
		REPLY_0514_t Data;
	} Reply;

	//gCmd_Timestamp = Read_U32((byte *)&pCmd->Timestamp);
	gFmRadioCountdown = 4;
	GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);

	Reply.Header.ID = 0x0515;
	Reply.Header.Size = sizeof(Reply.Data);
	strcpy(Reply.Data.Version, "Open Edition FW");
	Reply.Data.bHasCustomAesKey = bHasCustomAesKey;
	Reply.Data.bIsInLockScreen = bIsInLockScreen;
	Reply.Data.Random[0] = 0x11111111;
	Reply.Data.Random[1] = 0x22222222;
	Reply.Data.Random[2] = 0x33333333;
	Reply.Data.Random[3] = 0x44444444;

	SendReply(&Reply, sizeof(Reply));
}

static void CMD_051B(const uint8_t *pBuffer)
{
	const CMD_051B_t *pCmd = (const CMD_051B_t *)pBuffer;
	struct {
		Header_t Header;
		REPLY_051B_t Data;
	} Reply;
	bool bLocked = false;

	// if (pCmd->Timestamp != gTimestamp) return;

	gFmRadioCountdown = 4;
	memset(&Reply, 0, sizeof(Reply));
	Reply.Header.ID = 0x051C;
	Reply.Header.Size = pCmd->Size + 4;
	Reply.Data.Offset = pCmd->Offset;
	Reply.Data.Size = pCmd->Size;

	if (bHasCustomAesKey) {
		bLocked = gIsLocked;
	}

	if (!bLocked) {
		EEPROM_ReadBuffer(pCmd->Offset, Reply.Data.Data, pCmd->Size);
	}

	SendReply(&Reply, pCmd->Size + 8);
}

bool UART_IsCommandAvailable(void)
{
	uint16_t DmaLength;
	uint16_t CommandLength;
	uint16_t Index;
	uint16_t TailIndex;
	uint16_t Size;
	uint16_t CRC;
	uint16_t i;

	DmaLength = DMA_CH0->ST & 0xFFFU;
	while (1) {
		if (gUART_WriteIndex == DmaLength) {
			return false;
		}

		while (gUART_WriteIndex != DmaLength && UART_DMA_Buffer[gUART_WriteIndex] != 0xABU) {
			gUART_WriteIndex = DMA_INDEX(gUART_WriteIndex, 1);
		}

		if (gUART_WriteIndex == DmaLength) {
			return false;
		}

		if (gUART_WriteIndex < DmaLength) {
			CommandLength = DmaLength - gUART_WriteIndex;
		} else {
			CommandLength = (DmaLength + sizeof(UART_DMA_Buffer)) - gUART_WriteIndex;
		}
		if (CommandLength < 8) {
			return 0;
		}
		if (UART_DMA_Buffer[DMA_INDEX(gUART_WriteIndex, 1)] == 0xCD) {
			break;
		}
		gUART_WriteIndex = DMA_INDEX(gUART_WriteIndex, 1);
	}

	Index = DMA_INDEX(gUART_WriteIndex, 2);
	Size = (UART_DMA_Buffer[DMA_INDEX(Index, 1)] << 8) | UART_DMA_Buffer[Index];
	if (Size + 8 > sizeof(UART_DMA_Buffer)) {
		gUART_WriteIndex = DmaLength;
		return false;
	}
	if (CommandLength < Size + 8) {
		return false;
	}
	Index = DMA_INDEX(Index, 2);
	TailIndex = DMA_INDEX(Index, Size + 2);
	if (UART_DMA_Buffer[TailIndex] != 0xDC || UART_DMA_Buffer[DMA_INDEX(TailIndex, 1)] != 0xBA) {
		gUART_WriteIndex = DmaLength;
		return false;
	}
	if (TailIndex < Index) {
		uint16_t ChunkSize = sizeof(UART_DMA_Buffer) - Index;

		memcpy(UART_Command.Buffer, UART_DMA_Buffer + Index, ChunkSize);
		memcpy(UART_Command.Buffer + ChunkSize, UART_DMA_Buffer, TailIndex);
	} else {
		memcpy(UART_Command.Buffer, UART_DMA_Buffer + Index, TailIndex - Index);
	}

	TailIndex = DMA_INDEX(TailIndex, 2);
	if (TailIndex < gUART_WriteIndex) {
		memset(UART_DMA_Buffer + gUART_WriteIndex, 0, sizeof(UART_DMA_Buffer) - gUART_WriteIndex);
		memset(UART_DMA_Buffer, 0, TailIndex);
	} else {
		memset(UART_DMA_Buffer + gUART_WriteIndex, 0, TailIndex - gUART_WriteIndex);
	}

	gUART_WriteIndex = TailIndex;

	if (UART_Command.Header.ID == 0x0514) {
		bIsEncrypted = false;
	}
	if (UART_Command.Header.ID == 0x6902) {
		bIsEncrypted = true;
	}

	if (bIsEncrypted) {
		for (i = 0; i < Size + 2; i++) {
			UART_Command.Buffer[i] ^= Obfuscation[i % 16];
		}
	}

	CRC = UART_Command.Buffer[Size] | (UART_Command.Buffer[Size + 1] << 8);
	if (CRC_Calculate(UART_Command.Buffer, Size) != CRC) {
		return false;
	}

	return true;
}

void UART_HandleCommand(void)
{
	switch (UART_Command.Header.ID) {
	case 0x0527:
		break;

	case 0x051D:
		break;

	case 0x0514:
		CMD_0514(UART_Command.Buffer);
		break;

	case 0x051B:
		CMD_051B(UART_Command.Buffer);
		break;

	case 0x051F:
		break;

	case 0x0521:
		break;

	case 0x0529:
		break;

	case 0x052D:
		break;

	case 0x052F:
		break;

	case 0x05DD:
		overlay_FLASH_RebootToBootloader();
		break;
	}
}

