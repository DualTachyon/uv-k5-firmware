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
#include "audio.h"
#include "battery.h"
#include "bsp/dp32g030/gpio.h"
#include "dcs.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "fm.h"
#include "frequencies.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

VFO_Info_t *gTxRadioInfo;
VFO_Info_t *gInfoCHAN_A;
VFO_Info_t *gCrossTxRadioInfo;

DCS_CodeType_t gCodeType;
DCS_CodeType_t gCopyOfCodeType;
uint8_t gCode;

bool RADIO_CheckValidChannel(uint8_t Channel, bool bCheckScanList, uint8_t VFO)
{
	uint8_t Params;
	uint8_t PriorityCh1;
	uint8_t PriorityCh2;

	if (Channel >= 200) {
		return false;
	}

	// Check channel is valid
	Params = gMR_ChannelParameters[Channel];
	if ((Params & MR_CH_BAND_MASK) > BAND7_470MHz) {
		return false;
	}
	
	if (bCheckScanList) {
		if (VFO == 0) {
			if ((Params & MR_CH_SCANLIST1) == 0) {
				return false;
			}
			PriorityCh1 = gEeprom.SCANLIST_PRIORITY_CH1[0];
			PriorityCh2 = gEeprom.SCANLIST_PRIORITY_CH2[0];
		} else if (VFO == 1) {
			if ((Params & MR_CH_SCANLIST2) == 0) {
				return false;
			}
			PriorityCh1 = gEeprom.SCANLIST_PRIORITY_CH1[1];
			PriorityCh2 = gEeprom.SCANLIST_PRIORITY_CH2[1];
		} else {
			return true;
		}
		if (PriorityCh1 == Channel) {
			return false;
		}
		if (PriorityCh2 == Channel) {
			return false;
		}
	}

	return true;
}

uint8_t RADIO_FindNextChannel(uint8_t Channel, uint8_t Direction, bool bCheckScanList, uint8_t VFO)
{
	uint8_t i;

	for (i = 0; i < 200; i++) {
		if (Channel == 0xFF) {
			Channel = 199;
		} else if (Channel >= 200) {
			Channel = 0;
		}
		if (RADIO_CheckValidChannel(Channel, bCheckScanList, VFO)) {
			return Channel;
		}
		Channel += Direction;
	}

	return 0xFF;
}

void RADIO_InitInfo(VFO_Info_t *pInfo, uint8_t ChannelSave, uint8_t Band, uint32_t Frequency)
{
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->Band = Band;
	pInfo->SCANLIST1_PARTICIPATION = true;
	pInfo->SCANLIST2_PARTICIPATION = true;
	pInfo->STEP_SETTING = 5;
	pInfo->StepFrequency = 2500;
	pInfo->CHANNEL_SAVE = ChannelSave;
	pInfo->FrequencyReverse = false;
	pInfo->OUTPUT_POWER = OUTPUT_POWER_HIGH;
	pInfo->DCS[0].Frequency = Frequency;
	pInfo->DCS[1].Frequency = Frequency;
	pInfo->pDCS_Current = &pInfo->DCS[0];
	pInfo->FREQUENCY_OF_DEVIATION = 1000000;
	pInfo->pDCS_Reverse = &pInfo->DCS[1];
	RADIO_ConfigureSquelchAndOutputPower(pInfo);
}

void RADIO_ConfigureChannel(uint8_t VFO, uint32_t Arg)
{
	VFO_Info_t *pRadio;
	uint8_t ChNum;
	uint8_t Params;
	uint8_t Band;
	bool bParticipation2;
	uint16_t Base;
	uint8_t Data[8];
	uint8_t Tmp;
	uint32_t Frequency;

	pRadio = &gEeprom.VfoInfo[VFO];

	if (!gSetting_350EN) {
		if (gEeprom.EEPROM_0E82_0E85[VFO] == 204) {
			gEeprom.EEPROM_0E82_0E85[VFO] = 205;
		}
		if (gEeprom.VfoChannel[VFO] == 204) {
			gEeprom.VfoChannel[VFO] = 205;
		}
	}

	ChNum = gEeprom.VfoChannel[VFO];
	if (ChNum < 217) {
		if (ChNum >= 207) {
			RADIO_InitInfo(pRadio, gEeprom.VfoChannel[VFO], 2, NoaaFrequencyTable[ChNum - 207]);
			if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
				return;
			}
			g_2000036F = 1;
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
			return;
		}
		if (ChNum < 200) {
			ChNum = RADIO_FindNextChannel(ChNum, RADIO_CHANNEL_UP, false, VFO);
			if (ChNum == 0xFF) {
				ChNum = gEeprom.EEPROM_0E82_0E85[VFO];
				gEeprom.VfoChannel[VFO] = gEeprom.EEPROM_0E82_0E85[VFO];
			} else {
				gEeprom.VfoChannel[VFO] = ChNum;
				gEeprom.EEPROM_0E81_0E84[VFO] = ChNum;
			}
		}
	} else {
		ChNum = 205;
	}

	Params = gMR_ChannelParameters[ChNum];
	if (Params == 0xFF) {
		uint8_t Index;

		if (ChNum < 200) {
			ChNum = gEeprom.EEPROM_0E82_0E85[VFO];
			gEeprom.VfoChannel[VFO] = gEeprom.EEPROM_0E82_0E85[VFO];
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
		gEeprom.VfoInfo[VFO].Band = Band;
		gEeprom.VfoInfo[VFO].SCANLIST1_PARTICIPATION = !!(Params & MR_CH_SCANLIST1);
		bParticipation2 = !!(Params & MR_CH_SCANLIST2);
	} else {
		Band = ChNum - 200;
		gEeprom.VfoInfo[VFO].Band = Band;
		bParticipation2 = true;
		gEeprom.VfoInfo[VFO].SCANLIST1_PARTICIPATION = true;
	}
	gEeprom.VfoInfo[VFO].SCANLIST2_PARTICIPATION = bParticipation2;
	gEeprom.VfoInfo[VFO].CHANNEL_SAVE = ChNum;

	if (ChNum < 200) {
		Base = ChNum << 4;
	} else {
		Base = 0x0C80 + (ChNum * 32) + (VFO * 16);
	}

	if (Arg == 2 || ChNum >= 200) {
		EEPROM_ReadBuffer(Base + 8, Data, 8);

		Tmp = Data[3] & 0x0F;
		if (Tmp > 2) {
			Tmp = 0;
		}
		gEeprom.VfoInfo[VFO].FREQUENCY_DEVIATION_SETTING = Tmp;
		gEeprom.VfoInfo[VFO].AM_CHANNEL_MODE = !!(Data[3] & 0x10);

		Tmp = Data[4];
		if (Tmp > 6) {
			Params = 5;
		}
		gEeprom.VfoInfo[VFO].STEP_SETTING = Tmp;
		gEeprom.VfoInfo[VFO].StepFrequency = StepFrequencyTable[Tmp];

		Tmp = Data[7];
		if (Tmp > 10) {
			Tmp = 0;
		}
		gEeprom.VfoInfo[VFO].SCRAMBLING_TYPE = Tmp;
		gEeprom.VfoInfo[VFO].DCS[0].CodeType = (Data[2] >> 0) & 0x0F;
		gEeprom.VfoInfo[VFO].DCS[1].CodeType = (Data[2] >> 4) & 0x0F;

		Tmp = Data[0];
		switch (gEeprom.VfoInfo[VFO].DCS[0].CodeType) {
		case CODE_TYPE_CONTINUOUS_TONE:
			if (Tmp >= 50) {
				Tmp = 0;
			}
			break;
		case CODE_TYPE_DIGITAL:
		case CODE_TYPE_REVERSE_DIGITAL:
			if (Tmp >= 104) {
				Tmp = 0;
			}
			break;
		default:
			gEeprom.VfoInfo[VFO].DCS[0].CodeType = CODE_TYPE_OFF;
			break;
		}
		gEeprom.VfoInfo[VFO].DCS[0].RX_TX_Code = Tmp;

		Tmp = Data[1];
		switch (gEeprom.VfoInfo[VFO].DCS[1].CodeType) {
		case CODE_TYPE_CONTINUOUS_TONE:
			if (Tmp >= 50) {
				Tmp = 0;
			}
			break;
		case CODE_TYPE_DIGITAL:
		case CODE_TYPE_REVERSE_DIGITAL:
			if (Tmp >= 104) {
				Tmp = 0;
			}
			break;
		default:
			gEeprom.VfoInfo[VFO].DCS[1].CodeType = CODE_TYPE_OFF;
			break;
		}
		gEeprom.VfoInfo[VFO].DCS[1].RX_TX_Code = Tmp;

		if (Data[4] == 0xFF) {
			gEeprom.VfoInfo[VFO].FrequencyReverse = false;
			gEeprom.VfoInfo[VFO].CHANNEL_BANDWIDTH = 0;
			gEeprom.VfoInfo[VFO].OUTPUT_POWER = 2;
			gEeprom.VfoInfo[VFO].BUSY_CHANNEL_LOCK = false;
		} else {
			gEeprom.VfoInfo[VFO].FrequencyReverse = !!(Data[4] & 0x01);
			gEeprom.VfoInfo[VFO].CHANNEL_BANDWIDTH = !!(Data[4] & 0x02);
			gEeprom.VfoInfo[VFO].OUTPUT_POWER = (Data[4] >> 2) & 0x03;
			gEeprom.VfoInfo[VFO].BUSY_CHANNEL_LOCK = !!(Data[4] & 0x10);
		}
		if (Data[5] == 0xFF) {
			gEeprom.VfoInfo[VFO].DTMF_DECODING_ENABLE = false;
			gEeprom.VfoInfo[VFO].DTMF_PTT_ID_TX_MODE = 0;
		} else {
			gEeprom.VfoInfo[VFO].DTMF_DECODING_ENABLE = !!(Data[5] & 1);
			gEeprom.VfoInfo[VFO].DTMF_PTT_ID_TX_MODE = (Data[5] >> 0x01) & 0x03;
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
		gEeprom.VfoInfo[VFO].FREQUENCY_OF_DEVIATION = Info.Offset;
	}

	Frequency = pRadio->DCS[0].Frequency;
	if (Frequency < gLowerLimitFrequencyBandTable[Band]) {
		pRadio->DCS[0].Frequency = gLowerLimitFrequencyBandTable[Band];
	} else if (Frequency > gUpperLimitFrequencyBandTable[Band]) {
		pRadio->DCS[0].Frequency = gUpperLimitFrequencyBandTable[Band];
	} else if (ChNum >= 200) {
		pRadio->DCS[0].Frequency = FREQUENCY_FloorToStep(pRadio->DCS[0].Frequency, gEeprom.VfoInfo[VFO].StepFrequency, gLowerLimitFrequencyBandTable[Band]);
	}
	pRadio->DCS[0].Frequency = Frequency;

	if (Frequency - 10800000 < 2799991) {
		gEeprom.VfoInfo[VFO].FREQUENCY_DEVIATION_SETTING = FREQUENCY_DEVIATION_OFF;
	} else if (ChNum >= 200) {
		Frequency = FREQUENCY_FloorToStep(gEeprom.VfoInfo[VFO].FREQUENCY_OF_DEVIATION, gEeprom.VfoInfo[VFO].StepFrequency, 0);
		gEeprom.VfoInfo[VFO].FREQUENCY_OF_DEVIATION = Frequency;
	}
	RADIO_ApplyOffset(pRadio);
	memset(gEeprom.VfoInfo[VFO].Name, 0, sizeof(gEeprom.VfoInfo[VFO].Name));
	if (ChNum < 200) {
		// 16 bytes allocated but only 12 used
		EEPROM_ReadBuffer(0x0F50 + (ChNum * 0x10), gEeprom.VfoInfo[VFO].Name + 0, 8);
		EEPROM_ReadBuffer(0x0F58 + (ChNum * 0x10), gEeprom.VfoInfo[VFO].Name + 8, 2);
	}

	if (gEeprom.VfoInfo[VFO].FrequencyReverse == true) {
		gEeprom.VfoInfo[VFO].pDCS_Current = &gEeprom.VfoInfo[VFO].DCS[0];
		gEeprom.VfoInfo[VFO].pDCS_Reverse = &gEeprom.VfoInfo[VFO].DCS[1];
	} else {
		gEeprom.VfoInfo[VFO].pDCS_Current = &gEeprom.VfoInfo[VFO].DCS[1];
		gEeprom.VfoInfo[VFO].pDCS_Reverse = &gEeprom.VfoInfo[VFO].DCS[0];
	}

	if (gSetting_350EN == false) {
		DCS_Info_t *pDCS = gEeprom.VfoInfo[VFO].pDCS_Current;
		if (pDCS->Frequency - 35000000 < 4999991) {
			pDCS->Frequency = 41001250;
		}
	}

	if (gEeprom.VfoInfo[VFO].Band == BAND2_108MHz && gEeprom.VfoInfo[VFO].AM_CHANNEL_MODE == true) {
		gEeprom.VfoInfo[VFO].IsAM = true;
		gEeprom.VfoInfo[VFO].SCRAMBLING_TYPE = 0;
		gEeprom.VfoInfo[VFO].DTMF_DECODING_ENABLE = false;
		gEeprom.VfoInfo[VFO].DCS[0].CodeType = CODE_TYPE_OFF;
		gEeprom.VfoInfo[VFO].DCS[1].CodeType = CODE_TYPE_OFF;
	} else {
		gEeprom.VfoInfo[VFO].IsAM = false;
	}

	RADIO_ConfigureSquelchAndOutputPower(pRadio);
}

void RADIO_ConfigureSquelchAndOutputPower(VFO_Info_t *pInfo)
{
	uint8_t Txp[3];
	uint16_t Base;
	FREQUENCY_Band_t Band;

	Band = FREQUENCY_GetBand(pInfo->pDCS_Current->Frequency);
	if (Band < BAND4_174MHz) {
		Base = 0x1E60;
	} else {
		Base = 0x1E00;
	}

	if (gEeprom.SQUELCH_LEVEL == 0) {
		pInfo->SquelchOpenRSSIThresh = 0x00;
		pInfo->SquelchOpenNoiseThresh = 0x7F;
		pInfo->SquelchCloseGlitchThresh = 0xFF;
		pInfo->SquelchCloseRSSIThresh = 0x00;
		pInfo->SquelchCloseNoiseThresh = 0x7F;
		pInfo->SquelchOpenGlitchThresh = 0xFF;
	} else {
		Base += gEeprom.SQUELCH_LEVEL;
		EEPROM_ReadBuffer(Base + 0x00, &pInfo->SquelchOpenRSSIThresh, 1);
		EEPROM_ReadBuffer(Base + 0x10, &pInfo->SquelchCloseRSSIThresh, 1);
		EEPROM_ReadBuffer(Base + 0x20, &pInfo->SquelchOpenNoiseThresh, 1);
		EEPROM_ReadBuffer(Base + 0x30, &pInfo->SquelchCloseNoiseThresh, 1);
		EEPROM_ReadBuffer(Base + 0x40, &pInfo->SquelchCloseGlitchThresh, 1);
		EEPROM_ReadBuffer(Base + 0x50, &pInfo->SquelchOpenGlitchThresh, 1);
		if (pInfo->SquelchOpenNoiseThresh >= 0x80) {
			pInfo->SquelchOpenNoiseThresh = 0x7F;
		}
		if (pInfo->SquelchCloseNoiseThresh >= 0x80) {
			pInfo->SquelchCloseNoiseThresh = 0x7F;
		}
	}

	Band = FREQUENCY_GetBand(pInfo->pDCS_Reverse->Frequency);
	EEPROM_ReadBuffer(0x1ED0 + (Band * 0x10) + (pInfo->OUTPUT_POWER * 3), Txp, 3);
	pInfo->TXP_CalculatedSetting =
		FREQUENCY_CalculateOutputPower(
				Txp[0],
				Txp[1],
				Txp[2],
				LowerLimitFrequencyBandTable[Band],
				MiddleFrequencyBandTable[Band],
				UpperLimitFrequencyBandTable[Band],
				pInfo->pDCS_Reverse->Frequency);
}

void RADIO_ApplyOffset(VFO_Info_t *pInfo)
{
	uint32_t Frequency;

	Frequency = pInfo->DCS[0].Frequency;
	switch (pInfo->FREQUENCY_DEVIATION_SETTING) {
	case FREQUENCY_DEVIATION_OFF:
		break;
	case FREQUENCY_DEVIATION_ADD:
		Frequency += pInfo->FREQUENCY_OF_DEVIATION;
		break;
	case FREQUENCY_DEVIATION_SUB:
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
	if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_CHAN_B) {
		gEeprom.TX_CHANNEL = 1;
	} else if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_CHAN_A) {
		gEeprom.TX_CHANNEL = 0;
	} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_CHAN_B) {
		gEeprom.TX_CHANNEL = 1;
	} else if (gEeprom.DUAL_WATCH == DUAL_WATCH_CHAN_A) {
		gEeprom.TX_CHANNEL = 0;
	}

	gTxRadioInfo = &gEeprom.VfoInfo[gEeprom.TX_CHANNEL];
	gEeprom.RX_CHANNEL = gEeprom.TX_CHANNEL;
	if (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) {
		if (gEeprom.TX_CHANNEL == 0) {
			gEeprom.RX_CHANNEL = 1;
		} else {
			gEeprom.RX_CHANNEL = 0;
		}
	}

	gInfoCHAN_A = &gEeprom.VfoInfo[gEeprom.RX_CHANNEL];
	RADIO_ConfigureCrossTX();
}

void RADIO_ConfigureCrossTX(void)
{
	gCrossTxRadioInfo = gInfoCHAN_A;
	if (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) {
		gCrossTxRadioInfo = &gEeprom.VfoInfo[gEeprom.TX_CHANNEL];
	}
}

void RADIO_SetupRegisters(bool bSwitchToFunction0)
{
	BK4819_FilterBandwidth_t Bandwidth;
	uint16_t Status;
	uint16_t InterruptMask;
	uint32_t Frequency;

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	g_2000036B = 0;
	BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28, false);

	Bandwidth = gInfoCHAN_A->CHANNEL_BANDWIDTH;
	if (Bandwidth != BK4819_FILTER_BW_WIDE) {
		Bandwidth = BK4819_FILTER_BW_NARROW;
	}
	BK4819_SetFilterBandwidth(Bandwidth);

	BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29, false);
	BK4819_SetupPowerAmplifier(0, 0);
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1, false);

	while (1) {
		Status = BK4819_GetRegister(BK4819_REG_0C);
		if ((Status & 1U) == 0) { // INTERRUPT REQUEST
			break;
		}
		BK4819_WriteRegister(BK4819_REG_02, 0);
		SYSTEM_DelayMs(1);
	}
	BK4819_WriteRegister(BK4819_REG_3F, 0);
	BK4819_WriteRegister(BK4819_REG_7D, gEeprom.MIC_SENSITIVITY_TUNING | 0xE940);
	if (gInfoCHAN_A->CHANNEL_SAVE < 207 || gIsNoaaMode != false) {
		Frequency = gInfoCHAN_A->pDCS_Current->Frequency;
	} else {
		Frequency = NoaaFrequencyTable[gNoaaChannel];
	}
	BK4819_SetFrequency(Frequency);
	BK4819_SetupSquelch(
			gInfoCHAN_A->SquelchOpenRSSIThresh, gInfoCHAN_A->SquelchCloseRSSIThresh,
			gInfoCHAN_A->SquelchOpenNoiseThresh, gInfoCHAN_A->SquelchCloseNoiseThresh,
			gInfoCHAN_A->SquelchCloseGlitchThresh, gInfoCHAN_A->SquelchOpenGlitchThresh);
	BK4819_PickRXFilterPathBasedOnFrequency(Frequency);
	BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2, true);
	BK4819_WriteRegister(BK4819_REG_48, 0xB3A8);

	InterruptMask = 0
		| BK4819_REG_3F_SQUELCH_FOUND
		| BK4819_REG_3F_SQUELCH_LOST
		;

	if (gInfoCHAN_A->CHANNEL_SAVE < 207) {
		if (gInfoCHAN_A->IsAM != true) {
			uint8_t CodeType;
			uint8_t CodeWord;

			CodeType = gCodeType;
			CodeWord = gCode;
			if (g_20000381 == 0) {
				CodeType = gInfoCHAN_A->pDCS_Current->CodeType;
				CodeWord = gInfoCHAN_A->pDCS_Current->RX_TX_Code;
			}
			switch (CodeType) {
			case CODE_TYPE_DIGITAL:
			case CODE_TYPE_REVERSE_DIGITAL:
				BK4819_SetCDCSSCodeWord(DCS_GetGolayCodeWord(CodeType, CodeWord));
				InterruptMask = 0
					| BK4819_REG_3F_CxCSS_TAIL
					| BK4819_REG_3F_CDCSS_FOUND
					| BK4819_REG_3F_CDCSS_LOST
					| BK4819_REG_3F_SQUELCH_FOUND
					| BK4819_REG_3F_SQUELCH_LOST
					;
				break;
			case CODE_TYPE_CONTINUOUS_TONE:
				BK4819_SetCTCSSBaudRate(CTCSS_Options[CodeWord]);
				BK4819_Set55HzTailDetection();
				InterruptMask = 0
					| BK4819_REG_3F_CxCSS_TAIL
					| BK4819_REG_3F_CTCSS_FOUND
					| BK4819_REG_3F_CTCSS_LOST
					| BK4819_REG_3F_SQUELCH_FOUND
					| BK4819_REG_3F_SQUELCH_LOST
					;
				break;
			default:
				BK4819_SetCTCSSBaudRate(670);
				BK4819_Set55HzTailDetection();
				InterruptMask = 0
					| BK4819_REG_3F_CxCSS_TAIL
					| BK4819_REG_3F_SQUELCH_FOUND
					| BK4819_REG_3F_SQUELCH_LOST
					;
				break;
			}
			if (gInfoCHAN_A->SCRAMBLING_TYPE == 0 || gSetting_ScrambleEnable == false) {
				BK4819_DisableScramble();
			} else {
				BK4819_EnableScramble(gInfoCHAN_A->SCRAMBLING_TYPE - 1);
			}
		}
	} else {
		BK4819_SetCTCSSBaudRate(2625);
		InterruptMask = 0
			| BK4819_REG_3F_CTCSS_FOUND
			| BK4819_REG_3F_CTCSS_LOST
			| BK4819_REG_3F_SQUELCH_FOUND
			| BK4819_REG_3F_SQUELCH_LOST
			;
	}

	if (gEeprom.VOX_SWITCH == true && gFmMute != true && gCrossTxRadioInfo->CHANNEL_SAVE < 207 && gCrossTxRadioInfo->IsAM != true) {
		BK4819_EnableVox(gEeprom.VOX1_THRESHOLD, gEeprom.VOX0_THRESHOLD);
		InterruptMask |= 0
			| BK4819_REG_3F_VOX_FOUND
			| BK4819_REG_3F_VOX_LOST
			;
	} else {
		BK4819_DisableVox();
	}
	if ((gInfoCHAN_A->IsAM == true) || (gInfoCHAN_A->DTMF_DECODING_ENABLE != true && (gSetting_KILLED != true))) {
		BK4819_DisableDTMF();
	} else {
		BK4819_EnableDTMF();
		InterruptMask |= BK4819_REG_3F_DTMF_5TONE_FOUND;
	}
	BK4819_WriteRegister(BK4819_REG_3F, InterruptMask);

	FUNCTION_Init();

	if (bSwitchToFunction0 == 1) {
		FUNCTION_Select(FUNCTION_0);
	}
}

void RADIO_ConfigureNOAA(void)
{
	uint8_t ChanAB;

	g_2000036F = 1;
	if (gEeprom.NOAA_AUTO_SCAN) {
		if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
			if (gEeprom.VfoChannel[0] < 207) {
				if (gEeprom.VfoChannel[1] < 207) {
					gIsNoaaMode = false;
					return;
				}
				ChanAB = 1;
			} else {
				ChanAB = 0;
			}
			if (gIsNoaaMode == false) {
				gNoaaChannel = gEeprom.VfoInfo[ChanAB].CHANNEL_SAVE - 207;
			}
			gIsNoaaMode = true;
			return;
		}
		if (gInfoCHAN_A->CHANNEL_SAVE >= 206) {
			gIsNoaaMode = true;
			gNoaaChannel = gInfoCHAN_A->CHANNEL_SAVE - 207;
			g_20000356 = 0x32;
			gSystickFlag8 = 0;
		}
	}
}

void RADIO_PrepareTransmit(void)
{
	BK4819_FilterBandwidth_t Bandwidth;

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

	g_2000036B = 0;

	BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2, false);
	Bandwidth = gCrossTxRadioInfo->CHANNEL_BANDWIDTH;
	if (Bandwidth != BK4819_FILTER_BW_WIDE) {
		Bandwidth = BK4819_FILTER_BW_NARROW;
	}
	BK4819_SetFilterBandwidth(Bandwidth);
	BK4819_SetFrequency(gCrossTxRadioInfo->pDCS_Reverse->Frequency);
	BK4819_PrepareTransmit();
	SYSTEM_DelayMs(10);

	BK4819_PickRXFilterPathBasedOnFrequency(gCrossTxRadioInfo->pDCS_Reverse->Frequency);
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1, true);
	SYSTEM_DelayMs(5);

	BK4819_SetupPowerAmplifier(gCrossTxRadioInfo->TXP_CalculatedSetting, gCrossTxRadioInfo->pDCS_Reverse->Frequency);
	SYSTEM_DelayMs(10);

	if (gCrossTxRadioInfo->pDCS_Reverse->CodeType != CODE_TYPE_CONTINUOUS_TONE) {
		if ((gCrossTxRadioInfo->pDCS_Reverse->CodeType != CODE_TYPE_DIGITAL) && (gCrossTxRadioInfo->pDCS_Reverse->CodeType != CODE_TYPE_REVERSE_DIGITAL)) {
			BK4819_ExitSubAu();
			return;
		}
		BK4819_SetCDCSSCodeWord(
			DCS_GetGolayCodeWord(
				gCrossTxRadioInfo->pDCS_Reverse->CodeType,
				gCrossTxRadioInfo->pDCS_Reverse->RX_TX_Code
				)
			);
		return;
	}

	BK4819_SetCTCSSBaudRate(CTCSS_Options[gCrossTxRadioInfo->pDCS_Reverse->RX_TX_Code + 1]);
}

void RADIO_SomethingElse(uint8_t Arg)
{
	if (Arg == 0) {
		g_20000371[0] = 0;
		g_20000371[1] = 0;
		g_20000373 = 0;
	} else {
		if (Arg == 6) {
			g_20000371[0] = 6;
			g_20000371[1] = 3;
		} else {
			uint8_t Channel;

			Channel = gEeprom.RX_CHANNEL;
			if (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) {
				Channel = gEeprom.TX_CHANNEL;
			}
			g_20000371[Channel] = Arg;
		}
		g_20000373 = 5;
	}
	gUpdateDisplay = true;
}

void RADIO_SomethingWithTransmit(void)
{
	if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
		g_2000033A = 360;
		gSystickFlag7 = 0;
		if (g_2000041F == 0) {
			gEeprom.RX_CHANNEL = gEeprom.TX_CHANNEL;
			gInfoCHAN_A = gEeprom.VfoInfo + gEeprom.TX_CHANNEL;
		}
		g_2000041F = 1;
	}
	RADIO_ConfigureCrossTX();
	if (g_20000383 == 0 || g_20000383 == 3 || (g_20000383 == 1 && gEeprom.ALARM_MODE == 1)) {
		uint8_t Value;

		if (!FREQUENCY_Check(gCrossTxRadioInfo)) {
			if (gCrossTxRadioInfo->BUSY_CHANNEL_LOCK == true && gCurrentFunction == FUNCTION_4) {
				Value = 1;
			} else if (gBatteryDisplayLevel == 0) {
				Value = 2;
			} else {
				// TODO: Fix this goto, a bit painful to disentangle
				if (gBatteryDisplayLevel != 6) {
					goto LAB_00007c20;
				}
				Value = 6;
			}
		} else {
			Value = 3;
		}
		RADIO_SomethingElse(Value);
		g_20000383 = 0;
		AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
	} else {
LAB_00007c20:
		if (g_200003BE == 1) {
			if (g_20000438 == 2) {
				g_200003BD = 1;
				g_200003BC = 0;
				g_200003C3 = 6;
			}
			else {
				g_200003BC = 1;
				g_200003BD = 0;
			}
		}
		FUNCTION_Select(FUNCTION_TRANSMIT);
		if (g_20000383 == 0) {
			gTxTimerCountdown = gEeprom.TX_TIMEOUT_TIMER * 120;
		} else {
			gTxTimerCountdown = 0;
		}
		gSystickFlag0 = 0;
		g_200003FD = 0;
		g_2000036D = 0;
	}
	g_200003BE = 0;
}

void RADIO_EnableCxCSS(void)
{
	switch (gCrossTxRadioInfo->pDCS_Reverse->CodeType) {
	case CODE_TYPE_DIGITAL:
	case CODE_TYPE_REVERSE_DIGITAL:
		BK4819_EnableCDCSS();
		break;
	default:
		BK4819_EnableCTCSS();
		break;
	}

	SYSTEM_DelayMs(200);
}

void RADIO_Something(void)
{
	RADIO_SomethingWithTransmit();
	SYSTEM_DelayMs(200);
	RADIO_EnableCxCSS();
	RADIO_SetupRegisters(true);
}

