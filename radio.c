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
#include "driver/eeprom.h"
#include "frequencies.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

RADIO_Info_t *gTxRadioInfo;
RADIO_Info_t *gRxRadioInfo;
RADIO_Info_t *gCrossTxRadioInfo;

bool RADIO_CheckValidChannel(uint8_t ChNum, bool bCheckScanList, uint8_t RadioNum)
{
	uint8_t ChParam;
	uint8_t PriorityCh1;
	uint8_t PriorityCh2;

	if (ChNum >= 200) {
		return false;
	}

	// Check channel is valid
	ChParam = gMR_ChannelParameters[ChNum];
	if ((ChParam & MR_CH_BAND_MASK) > BAND7_470MHz) {
		return false;
	}
	
	if (bCheckScanList) {
		if (RadioNum == 0) {
			if ((ChParam & MR_CH_SCANLIST1) == 0) {
				return false;
			}
			PriorityCh1 = gEeprom.SCANLIST_PRIORITY_CH1[0];
			PriorityCh2 = gEeprom.SCANLIST_PRIORITY_CH2[0];
		} else if (RadioNum == 1) {
			if ((ChParam & MR_CH_SCANLIST2) == 0) {
				return false;
			}
			PriorityCh1 = gEeprom.SCANLIST_PRIORITY_CH1[1];
			PriorityCh2 = gEeprom.SCANLIST_PRIORITY_CH2[1];
		} else {
			return true;
		}
		if (PriorityCh1 == ChNum) {
			return false;
		}
		if (PriorityCh2 == ChNum) {
			return false;
		}
	}

	return true;
}

uint8_t RADIO_FindNextChannel(uint8_t ChNum, uint8_t Direction, bool bCheckScanList, uint8_t RadioNum)
{
	uint8_t i;

	for (i = 0; i < 200; i++) {
		if (ChNum == 0xFF) {
			ChNum = 199;
		} else if (ChNum >= 200) {
			ChNum = 0;
		}
		if (RADIO_CheckValidChannel(ChNum, bCheckScanList, RadioNum)) {
			return ChNum;
		}
		ChNum += Direction;
	}

	return 0xFF;
}

void RADIO_InitInfo(RADIO_Info_t *pInfo, uint8_t ChannelSave, uint8_t Band, uint32_t Frequency)
{
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->Band = Band;
	pInfo->SCANLIST1_PARTICIPATION = true;
	pInfo->SCANLIST2_PARTICIPATION = true;
	pInfo->STEP_SETTING = 5;
	pInfo->StepFrequency = 2500;
	pInfo->CHANNEL_SAVE = ChannelSave;
	pInfo->FrequencyReverse = false;
	pInfo->OUTPUT_POWER = 2;
	pInfo->DCS[0].Frequency = Frequency;
	pInfo->DCS[1].Frequency = Frequency;
	pInfo->pNext = pInfo;
	pInfo->FREQUENCY_OF_DEVIATION = 1000000;
	pInfo->pDCS = &pInfo->DCS[1];
	RADIO_ConfigureSquelchAndOutputPower(pInfo);
}

void RADIO_ConfigureChannel(uint8_t RadioNum, uint32_t Arg)
{
	RADIO_Info_t *pRadio;
	uint8_t ChNum;
	uint8_t Params;
	uint8_t Band;
	bool bParticipation2;
	uint16_t Base;
	uint8_t Data[8];
	uint8_t Tmp;

	pRadio = &gEeprom.RadioInfo[RadioNum];

	if (!gSetting_350EN) {
		if (gEeprom.EEPROM_0E82_0E85[RadioNum] == 204) {
			gEeprom.EEPROM_0E82_0E85[RadioNum] = 205;
		}
		if (gEeprom.EEPROM_0E80_0E83[RadioNum] == 204) {
			gEeprom.EEPROM_0E80_0E83[RadioNum] = 205;
		}
	}

	ChNum = gEeprom.EEPROM_0E80_0E83[RadioNum];
	if (ChNum < 217) {
		if (ChNum >= 207) {
			RADIO_InitInfo(pRadio, gEeprom.EEPROM_0E80_0E83[RadioNum], 2, NoaaFrequencyTable[ChNum - 207]);
			if (gEeprom.CROSS_BAND_RX_TX == 0) {
				return;
			}
			g_2000036F = 1;
			gEeprom.CROSS_BAND_RX_TX = 0;
			return;
		}
		if (ChNum < 200) {
			ChNum = RADIO_FindNextChannel(ChNum, RADIO_CHANNEL_UP, false, RadioNum);
			if (ChNum == 0xFF) {
				ChNum = gEeprom.EEPROM_0E82_0E85[RadioNum];
				gEeprom.EEPROM_0E80_0E83[RadioNum] = gEeprom.EEPROM_0E82_0E85[RadioNum];
			} else {
				gEeprom.EEPROM_0E80_0E83[RadioNum] = ChNum;
				gEeprom.EEPROM_0E81_0E84[RadioNum] = ChNum;
			}
		}
	} else {
		ChNum = 205;
	}

	Params = gMR_ChannelParameters[ChNum];
	if (Params == 0xFF) {
		uint8_t Index;

		if (ChNum < 200) {
			ChNum = gEeprom.EEPROM_0E82_0E85[RadioNum];
			gEeprom.EEPROM_0E80_0E83[RadioNum] = gEeprom.EEPROM_0E82_0E85[RadioNum];
		}
		Index = ChNum - 200;
		RADIO_InitInfo(pRadio, ChNum, Index, gLowerLimitFrequencyBandTable[Index]);
		return;
	}

	Band = Params & MR_CH_BAND_MASK;
	if (Band > BAND7_470MHz) {
		Band = BAND6_400MHz;
	}

	if (ChNum < 200) {
		gEeprom.RadioInfo[RadioNum].Band = Band;
		gEeprom.RadioInfo[RadioNum].SCANLIST1_PARTICIPATION = !!(Params & MR_CH_SCANLIST1);
		bParticipation2 = !!(Params & MR_CH_SCANLIST2);
	} else {
		Band = ChNum - 200;
		gEeprom.RadioInfo[RadioNum].Band = Band;
		bParticipation2 = true;
		gEeprom.RadioInfo[RadioNum].SCANLIST1_PARTICIPATION = true;
	}
	gEeprom.RadioInfo[RadioNum].SCANLIST2_PARTICIPATION = bParticipation2;
	gEeprom.RadioInfo[RadioNum].CHANNEL_SAVE = ChNum;

	if (ChNum < 200) {
		Base = ChNum << 4;
	} else {
		Base = 0x0C80 + (ChNum * 32) + (RadioNum * 16);
	}

	if (Arg == 2 || ChNum >= 200) {
		EEPROM_ReadBuffer(Base + 8, Data, 8);

		Tmp = Data[3] & 0x0F;
		if (Tmp > 2) {
			Tmp = 0;
		}
		gEeprom.RadioInfo[RadioNum].FREQUENCY_DEVIATION_SETTING = Tmp;
		gEeprom.RadioInfo[RadioNum].AM_CHANNEL_MODE = !!(Data[3] & 0x10);

		Tmp = Data[4];
		if (Tmp > 6) {
			Params = 5;
		}
		gEeprom.RadioInfo[RadioNum].STEP_SETTING = Tmp;
		gEeprom.RadioInfo[RadioNum].StepFrequency = StepFrequencyTable[Tmp];

		Tmp = Data[7];
		if (Tmp > 10) {
			Tmp = 0;
		}
		gEeprom.RadioInfo[RadioNum].SCRAMBLING_TYPE = Tmp;
		gEeprom.RadioInfo[RadioNum].DCS[0].CodeType = (Data[2] >> 0) & 0x0F;
		gEeprom.RadioInfo[RadioNum].DCS[1].CodeType = (Data[2] >> 4) & 0x0F;

		Tmp = Data[0];
		switch (gEeprom.RadioInfo[RadioNum].DCS[0].CodeType) {
		case 1: // CONTINUOUS TONE
			if (Tmp >= 50) {
				Tmp = 0;
			}
			break;
		case 2: // DIGITAL
		case 3: // REVERSE_DIGITAL
			if (Tmp >= 104) {
				Tmp = 0;
			}
			break;
		default:
			gEeprom.RadioInfo[RadioNum].DCS[0].CodeType = 0;
			break;
		}
		gEeprom.RadioInfo[RadioNum].DCS[0].RX_TX_Code = Tmp;

		Tmp = Data[1];
		switch (gEeprom.RadioInfo[RadioNum].DCS[1].CodeType) {
		case 1: // CONTINUOUS TONE
			if (Tmp >= 50) {
				Tmp = 0;
			}
			break;
		case 2: // DIGITAL
		case 3: // REVERSE_DIGITAL
			if (Tmp >= 104) {
				Tmp = 0;
			}
			break;
		default:
			gEeprom.RadioInfo[RadioNum].DCS[1].CodeType = 0;
			break;
		}
		gEeprom.RadioInfo[RadioNum].DCS[1].RX_TX_Code = Tmp;

		if (Data[4] == 0xFF) {
			gEeprom.RadioInfo[RadioNum].FrequencyReverse = false;
			gEeprom.RadioInfo[RadioNum].CHANNEL_BANDWIDTH = 0;
			gEeprom.RadioInfo[RadioNum].OUTPUT_POWER = 2;
			gEeprom.RadioInfo[RadioNum].BUSY_CHANNEL_LOCK = false;
		} else {
			gEeprom.RadioInfo[RadioNum].FrequencyReverse = !!(Data[4] & 0x01);
			gEeprom.RadioInfo[RadioNum].CHANNEL_BANDWIDTH = !!(Data[4] & 0x02);
			gEeprom.RadioInfo[RadioNum].OUTPUT_POWER = (Data[4] >> 2) & 0x03;
			gEeprom.RadioInfo[RadioNum].BUSY_CHANNEL_LOCK = !!(Data[4] & 0x10);
		}
		if (Data[5] == 0xFF) {
			gEeprom.RadioInfo[RadioNum].DTMF_DECODING_ENABLE = false;
			gEeprom.RadioInfo[RadioNum].DTMF_PTT_ID_TX_MODE = 0;
		} else {
			gEeprom.RadioInfo[RadioNum].DTMF_DECODING_ENABLE = !!(Data[5] & 1);
			gEeprom.RadioInfo[RadioNum].DTMF_PTT_ID_TX_MODE = (Data[5] >> 0x01) & 0x03;
		}

		struct {
			uint32_t Frequency;
			uint32_t Offset;
		} Info;

		EEPROM_ReadBuffer(Base, &Info, 8);

		pRadio->DCS[0].Frequency = Info.Frequency;
		if (Info.Offset >= 100000000) {
			Info.Offset = 1000000;
		}
		gEeprom.RadioInfo[RadioNum].FREQUENCY_OF_DEVIATION = Info.Offset;
	}

	uint32_t Frequency;

	// TODO: Double check this block, Ghidra has some issues
	Frequency = gLowerLimitFrequencyBandTable[Band];
	if (Frequency <= pRadio->DCS[0].Frequency && (pRadio->DCS[0].Frequency <= gUpperLimitFrequencyBandTable[Band])) {
		Frequency = pRadio->DCS[0].Frequency;
		if (ChNum >= 200) {
			Frequency = FREQUENCY_FloorToStep(pRadio->DCS[0].Frequency, gEeprom.RadioInfo[RadioNum].StepFrequency, gLowerLimitFrequencyBandTable[Band]);
		}
	}
	pRadio->DCS[0].Frequency = Frequency;

	if (Frequency - 10800000 < 2799991) {
		gEeprom.RadioInfo[RadioNum].FREQUENCY_DEVIATION_SETTING = 0;
	} else if (ChNum >= 200) {
		Frequency = FREQUENCY_FloorToStep(gEeprom.RadioInfo[RadioNum].FREQUENCY_OF_DEVIATION, gEeprom.RadioInfo[RadioNum].StepFrequency, 0);
		gEeprom.RadioInfo[RadioNum].FREQUENCY_OF_DEVIATION = Frequency;
	}
	RADIO_ApplyDeviation(pRadio);
	memset(gEeprom.RadioInfo[RadioNum].Name, 0, sizeof(gEeprom.RadioInfo[RadioNum].Name));
	if (ChNum < 200) {
		// 16 bytes allocated but only 12 used
		EEPROM_ReadBuffer(0x0F50 + (ChNum * 0x10), gEeprom.RadioInfo[RadioNum].Name + 0, 8);
		EEPROM_ReadBuffer(0x0F58 + (ChNum * 0x10), gEeprom.RadioInfo[RadioNum].Name + 8, 2);
	}

	// TODO: Double check this block, Ghidra has some issues
	/*
	if (gEeprom.RadioInfo[RadioNum].FrequencyReverse == true) {
		gEeprom.RadioInfo[RadioNum].pDCS = &pRadio->DCS[0];
		gEeprom.RadioInfo[RadioNum].pNext = &gEeprom.RadioInfo[RadioNum];
	} else {
		gEeprom.RadioInfo[RadioNum].pNext = pRadio;
		gEeprom.RadioInfo[RadioNum].pDCS = gEeprom.RadioInfo[RadioNum].;
	}
	*/

	if (gSetting_350EN == false) {
		RADIO_Info_t *pTmp = gEeprom.RadioInfo[RadioNum].pNext;
		if (pTmp->DCS[0].Frequency - 35000000 < 4999991) {
			pTmp->DCS[0].Frequency = 41001250;
		}
	}

	if (gEeprom.RadioInfo[RadioNum].Band == BAND2_108MHz && gEeprom.RadioInfo[RadioNum].AM_CHANNEL_MODE == true) {
		gEeprom.RadioInfo[RadioNum]._0x0033 = true;
		gEeprom.RadioInfo[RadioNum].SCRAMBLING_TYPE = 0;
		gEeprom.RadioInfo[RadioNum].DTMF_DECODING_ENABLE = false;
		gEeprom.RadioInfo[RadioNum].DCS[0].CodeType = 0;
		gEeprom.RadioInfo[RadioNum].DCS[1].CodeType = 0;
	} else {
		gEeprom.RadioInfo[RadioNum]._0x0033 = false;
	}

	RADIO_ConfigureSquelchAndOutputPower(pRadio);
}

void RADIO_ConfigureSquelchAndOutputPower(RADIO_Info_t *pInfo)
{
	uint8_t Txp[3];
	uint16_t Base;
	FREQUENCY_Band_t Band;

	Band = FREQUENCY_GetBand(pInfo->pNext->DCS[0].Frequency);
	if (Band < BAND4_174MHz) {
		Base = 0x1E60;
	} else {
		Base = 0x1E00;
	}

	if (gEeprom.SQUELCH_LEVEL == 0) {
		pInfo->SQ0 = 0x00;
		pInfo->SQ2 = 0x7F;
		pInfo->SQ4 = 0xFF;
		pInfo->SQ1 = 0x00;
		pInfo->SQ3 = 0x7F;
		pInfo->SQ5 = 0xFF;
	} else {
		Base += gEeprom.SQUELCH_LEVEL;
		EEPROM_ReadBuffer(Base + 0x00, &pInfo->SQ0, 1);
		EEPROM_ReadBuffer(Base + 0x10, &pInfo->SQ1, 1);
		EEPROM_ReadBuffer(Base + 0x20, &pInfo->SQ2, 1);
		EEPROM_ReadBuffer(Base + 0x30, &pInfo->SQ3, 1);
		EEPROM_ReadBuffer(Base + 0x40, &pInfo->SQ4, 1);
		EEPROM_ReadBuffer(Base + 0x50, &pInfo->SQ5, 1);
		if (pInfo->SQ2 >= 0x80) {
			pInfo->SQ2 = 0x7F;
		}
		if (pInfo->SQ3 >= 0x80) {
			pInfo->SQ3 = 0x7F;
		}
	}

	Band = FREQUENCY_GetBand(pInfo->pDCS->Frequency);
	EEPROM_ReadBuffer(0x1ED0 + (Band * 0x10) + (pInfo->OUTPUT_POWER * 3), Txp, 3);
	pInfo->TXP_CalculatedSetting =
		FREQUENCY_CalculateOutputPower(
				Txp[0],
				Txp[1],
				Txp[2],
				LowerLimitFrequencyBandTable[Band],
				MiddleFrequencyBandTable[Band],
				UpperLimitFrequencyBandTable[Band],
				pInfo->pDCS->Frequency);
}

void RADIO_ApplyDeviation(RADIO_Info_t *pInfo)
{
	uint32_t Frequency;

	Frequency = pInfo->DCS[0].Frequency;
	switch (pInfo->FREQUENCY_DEVIATION_SETTING) {
	case 0:
		break;
	case 1:
		Frequency += pInfo->FREQUENCY_OF_DEVIATION;
		break;
	case 2:
		Frequency -= pInfo->FREQUENCY_OF_DEVIATION;
		break;
	}

	if (Frequency < 5000000) {
		Frequency = 5000000;
	} else if (Frequency > 60000000) {
		Frequency = 60000000;
	}

	pInfo->DCS[1].Frequency = Frequency;
}


void RADIO_ConfigureTX(void)
{
	if (gEeprom.CROSS_BAND_RX_TX == 2) { // == CHAN B
		gEeprom.TX_CHANNEL = 1;
	} else if (gEeprom.CROSS_BAND_RX_TX == 1) { // != CHAN A
		gEeprom.TX_CHANNEL = 0;
	} else if (gEeprom.DUAL_WATCH == 2) { // == CHAN B
		gEeprom.TX_CHANNEL = 0;
	} else if (gEeprom.DUAL_WATCH == 1) { // == CHAN A
		gEeprom.TX_CHANNEL = 1;
	}

	gTxRadioInfo = &gEeprom.RadioInfo[gEeprom.TX_CHANNEL];
	gEeprom.RX_CHANNEL = gEeprom.TX_CHANNEL;
	if (gEeprom.CROSS_BAND_RX_TX != 0) { // != OFF
		if (gEeprom.TX_CHANNEL == 0) {
			gEeprom.RX_CHANNEL = 1;
		} else {
			gEeprom.RX_CHANNEL = 0;
		}
	}

	gRxRadioInfo = &gEeprom.RadioInfo[gEeprom.RX_CHANNEL];
	RADIO_ConfigureCrossTX();
}

void RADIO_ConfigureCrossTX(void)
{
	gCrossTxRadioInfo = gRxRadioInfo;
	if (gEeprom.CROSS_BAND_RX_TX != 0) { // != OFF
		gCrossTxRadioInfo = &gEeprom.RadioInfo[gEeprom.TX_CHANNEL];
	}
}

