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

#ifndef RADIO_H
#define RADIO_H

#include <stdbool.h>
#include <stdint.h>

enum {
	MR_CH_SCANLIST1 = (1U << 7),
	MR_CH_SCANLIST2 = (1U << 6),
	MR_CH_BAND_MASK = 0x0FU,
};

enum {
	RADIO_CHANNEL_UP = 0x01U,
	RADIO_CHANNEL_DOWN = 0xFFU,
};

typedef struct {
	uint32_t Frequency;
	uint8_t CodeType;
	uint8_t RX_TX_Code;
	uint8_t Padding[2];
} DCS_Info_t;

typedef struct RADIO_Info_t {
	DCS_Info_t DCS[2];
	struct RADIO_Info_t *pNext;
	DCS_Info_t *pDCS;
	uint32_t FREQUENCY_OF_DEVIATION;
	uint16_t StepFrequency;
	uint8_t CHANNEL_SAVE;
	uint8_t FREQUENCY_DEVIATION_SETTING;
	uint8_t SQ0;
	uint8_t SQ2;
	uint8_t SQ4;
	uint8_t SQ1;
	uint8_t SQ3;
	uint8_t SQ5;
	uint8_t STEP_SETTING;
	uint8_t OUTPUT_POWER;
	uint8_t TXP_CalculatedSetting;
	bool FrequencyReverse;
	uint8_t SCRAMBLING_TYPE;
	uint8_t CHANNEL_BANDWIDTH;
	uint8_t SCANLIST1_PARTICIPATION;
	uint8_t SCANLIST2_PARTICIPATION;
	uint8_t Band;
	uint8_t DTMF_DECODING_ENABLE;
	uint8_t DTMF_PTT_ID_TX_MODE;
	uint8_t BUSY_CHANNEL_LOCK;
	uint8_t AM_CHANNEL_MODE;
	bool _0x0033;
	char Name[16];
} RADIO_Info_t;

extern RADIO_Info_t *gTxRadioInfo;
extern RADIO_Info_t *gRxRadioInfo;
extern RADIO_Info_t *gCrossTxRadioInfo;

bool RADIO_CheckValidChannel(uint8_t ChNum, bool bCheckScanList, uint8_t RadioNum);
uint8_t RADIO_FindNextChannel(uint8_t ChNum, uint8_t Direction, bool bCheckScanList, uint8_t RadioNum);
void RADIO_InitInfo(RADIO_Info_t *pInfo, uint8_t ChannelSave, uint8_t ChIndex, uint32_t Frequency);
void RADIO_ConfigureChannel(uint8_t RadioNum, uint32_t Arg);
void RADIO_ConfigureSquelchAndOutputPower(RADIO_Info_t *pInfo);
void RADIO_ApplyDeviation(RADIO_Info_t *pInfo);
void RADIO_ConfigureTX(void);
void RADIO_ConfigureCrossTX(void);
void RADIO_SetupRegisters(bool bSwitchToFunction0);

#endif

