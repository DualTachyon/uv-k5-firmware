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
#include "app/dtmf.h"
#if defined(ENABLE_FMRADIO)
#include "app/fm.h"
#endif
#include "audio.h"
#include "bsp/dp32g030/gpio.h"
#include "dcs.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "frequencies.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

VFO_Info_t *gTxVfo;
VFO_Info_t *gRxVfo;
VFO_Info_t *gCurrentVfo;

DCS_CodeType_t gCurrentCodeType;
DCS_CodeType_t gSelectedCodeType;
uint8_t gSelectedCode;

STEP_Setting_t gStepSetting;

VfoState_t VfoState[2];

bool RADIO_CheckValidChannel(uint16_t Channel, bool bCheckScanList, uint8_t VFO)
{
	uint8_t Attributes;
	uint8_t PriorityCh1;
	uint8_t PriorityCh2;

	if (!IS_MR_CHANNEL(Channel)) {
		return false;
	}

	// Check channel is valid
	Attributes = gMR_ChannelAttributes[Channel];
	if ((Attributes & MR_CH_BAND_MASK) > BAND7_470MHz) {
		return false;
	}
	
	if (bCheckScanList) {
		switch (VFO) {
		case 0:
			if ((Attributes & MR_CH_SCANLIST1) == 0) {
				return false;
			}
			PriorityCh1 = gEeprom.SCANLIST_PRIORITY_CH1[0];
			PriorityCh2 = gEeprom.SCANLIST_PRIORITY_CH2[0];
			break;
		case 1:
			if ((Attributes & MR_CH_SCANLIST2) == 0) {
				return false;
			}
			PriorityCh1 = gEeprom.SCANLIST_PRIORITY_CH1[1];
			PriorityCh2 = gEeprom.SCANLIST_PRIORITY_CH2[1];
			break;
		default:
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

uint8_t RADIO_FindNextChannel(uint8_t Channel, int8_t Direction, bool bCheckScanList, uint8_t VFO)
{
	uint8_t i;

	for (i = 0; i < 200; i++) {
		if (Channel == 0xFF) {
			Channel = MR_CHANNEL_LAST;
		} else if (Channel > MR_CHANNEL_LAST) {
			Channel = MR_CHANNEL_FIRST;
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
	pInfo->STEP_SETTING = STEP_25_0kHz;
	pInfo->StepFrequency = 2500;
	pInfo->CHANNEL_SAVE = ChannelSave;
	pInfo->FrequencyReverse = false;
	pInfo->OUTPUT_POWER = OUTPUT_POWER_HIGH;
	pInfo->ConfigRX.Frequency = Frequency;
	pInfo->ConfigTX.Frequency = Frequency;
	pInfo->pRX = &pInfo->ConfigRX;
	pInfo->pTX = &pInfo->ConfigTX;
	pInfo->FREQUENCY_OF_DEVIATION = 1000000;
	RADIO_ConfigureSquelchAndOutputPower(pInfo);
}

void RADIO_ConfigureChannel(uint8_t VFO, uint32_t Arg)
{
	VFO_Info_t *pRadio;
	uint8_t Channel;
	uint8_t Attributes;
	uint8_t Band;
	bool bParticipation2;
	uint16_t Base;
	uint8_t Data[8];
	uint8_t Tmp;
	uint32_t Frequency;

	pRadio = &gEeprom.VfoInfo[VFO];

	if (!gSetting_350EN) {
		if (gEeprom.FreqChannel[VFO] == 204) {
			gEeprom.FreqChannel[VFO] = 205;
		}
		if (gEeprom.ScreenChannel[VFO] == 204) {
			gEeprom.ScreenChannel[VFO] = 205;
		}
	}

	Channel = gEeprom.ScreenChannel[VFO];
	if (IS_VALID_CHANNEL(Channel)) {
#if defined(ENABLE_NOAA)
		if (Channel >= NOAA_CHANNEL_FIRST) {
			RADIO_InitInfo(pRadio, gEeprom.ScreenChannel[VFO], 2, NoaaFrequencyTable[Channel - NOAA_CHANNEL_FIRST]);
			if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
				return;
			}
			gUpdateStatus = true;
			gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
			return;
		}
#endif
		if (IS_MR_CHANNEL(Channel)) {
			Channel = RADIO_FindNextChannel(Channel, RADIO_CHANNEL_UP, false, VFO);
			if (Channel == 0xFF) {
				Channel = gEeprom.FreqChannel[VFO];
				gEeprom.ScreenChannel[VFO] = gEeprom.FreqChannel[VFO];
			} else {
				gEeprom.ScreenChannel[VFO] = Channel;
				gEeprom.MrChannel[VFO] = Channel;
			}
		}
	} else {
		Channel = 205;
	}

	Attributes = gMR_ChannelAttributes[Channel];
	if (Attributes == 0xFF) {
		uint8_t Index;

		if (IS_MR_CHANNEL(Channel)) {
			Channel = gEeprom.FreqChannel[VFO];
			gEeprom.ScreenChannel[VFO] = gEeprom.FreqChannel[VFO];
		}
		Index = Channel - FREQ_CHANNEL_FIRST;
		RADIO_InitInfo(pRadio, Channel, Index, gLowerLimitFrequencyBandTable[Index]);
		return;
	}

	Band = Attributes & MR_CH_BAND_MASK;
	if (Band > BAND7_470MHz) {
		Band = BAND6_400MHz;
	}

	if (IS_MR_CHANNEL(Channel)) {
		gEeprom.VfoInfo[VFO].Band = Band;
		gEeprom.VfoInfo[VFO].SCANLIST1_PARTICIPATION = !!(Attributes & MR_CH_SCANLIST1);
		bParticipation2 = !!(Attributes & MR_CH_SCANLIST2);
	} else {
		Band = Channel - FREQ_CHANNEL_FIRST;
		gEeprom.VfoInfo[VFO].Band = Band;
		bParticipation2 = true;
		gEeprom.VfoInfo[VFO].SCANLIST1_PARTICIPATION = true;
	}
	gEeprom.VfoInfo[VFO].SCANLIST2_PARTICIPATION = bParticipation2;
	gEeprom.VfoInfo[VFO].CHANNEL_SAVE = Channel;

	if (IS_MR_CHANNEL(Channel)) {
		Base = Channel * 16;
	} else {
		Base = 0x0C80 + ((Channel - FREQ_CHANNEL_FIRST) * 32) + (VFO * 16);
	}

	if (Arg == 2 || Channel >= FREQ_CHANNEL_FIRST) {
		EEPROM_ReadBuffer(Base + 8, Data, 8);

		Tmp = Data[3] & 0x0F;
		if (Tmp > 2) {
			Tmp = 0;
		}
		gEeprom.VfoInfo[VFO].FREQUENCY_DEVIATION_SETTING = Tmp;
		gEeprom.VfoInfo[VFO].AM_CHANNEL_MODE = !!(Data[3] & 0x10);

		Tmp = Data[6];
		if (Tmp > STEP_8_33kHz) {
			Tmp = STEP_25_0kHz;
		}
		gEeprom.VfoInfo[VFO].STEP_SETTING = Tmp;
		gEeprom.VfoInfo[VFO].StepFrequency = StepFrequencyTable[Tmp];

		Tmp = Data[7];
		if (Tmp > 10) {
			Tmp = 0;
		}
		gEeprom.VfoInfo[VFO].SCRAMBLING_TYPE = Tmp;
		gEeprom.VfoInfo[VFO].ConfigRX.CodeType = (Data[2] >> 0) & 0x0F;
		gEeprom.VfoInfo[VFO].ConfigTX.CodeType = (Data[2] >> 4) & 0x0F;

		Tmp = Data[0];
		switch (gEeprom.VfoInfo[VFO].ConfigRX.CodeType) {
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
			gEeprom.VfoInfo[VFO].ConfigRX.CodeType = CODE_TYPE_OFF;
			Tmp = 0;
			break;
		}
		gEeprom.VfoInfo[VFO].ConfigRX.Code = Tmp;

		Tmp = Data[1];
		switch (gEeprom.VfoInfo[VFO].ConfigTX.CodeType) {
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
			gEeprom.VfoInfo[VFO].ConfigTX.CodeType = CODE_TYPE_OFF;
			Tmp = 0;
			break;
		}
		gEeprom.VfoInfo[VFO].ConfigTX.Code = Tmp;

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

		pRadio->ConfigRX.Frequency = Info.Frequency;
		if (Info.Offset >= 100000000) {
			Info.Offset = 1000000;
		}
		gEeprom.VfoInfo[VFO].FREQUENCY_OF_DEVIATION = Info.Offset;
	}

	Frequency = pRadio->ConfigRX.Frequency;
	if (Frequency < LowerLimitFrequencyBandTable[Band]) {
		pRadio->ConfigRX.Frequency = LowerLimitFrequencyBandTable[Band];
	} else if (Frequency > UpperLimitFrequencyBandTable[Band]) {
		pRadio->ConfigRX.Frequency = UpperLimitFrequencyBandTable[Band];
	} else if (Channel >= FREQ_CHANNEL_FIRST) {
		pRadio->ConfigRX.Frequency = FREQUENCY_FloorToStep(pRadio->ConfigRX.Frequency, gEeprom.VfoInfo[VFO].StepFrequency, LowerLimitFrequencyBandTable[Band]);
	}

	if (Frequency >= 10800000 && Frequency <= 13599990) {
		gEeprom.VfoInfo[VFO].FREQUENCY_DEVIATION_SETTING = FREQUENCY_DEVIATION_OFF;
	} else if (!IS_MR_CHANNEL(Channel)) {
		Frequency = FREQUENCY_FloorToStep(gEeprom.VfoInfo[VFO].FREQUENCY_OF_DEVIATION, gEeprom.VfoInfo[VFO].StepFrequency, 0);
		gEeprom.VfoInfo[VFO].FREQUENCY_OF_DEVIATION = Frequency;
	}
	RADIO_ApplyOffset(pRadio);
	memset(gEeprom.VfoInfo[VFO].Name, 0, sizeof(gEeprom.VfoInfo[VFO].Name));
	if (IS_MR_CHANNEL(Channel)) {
		// 16 bytes allocated but only 12 used
		EEPROM_ReadBuffer(0x0F50 + (Channel * 0x10), gEeprom.VfoInfo[VFO].Name + 0, 8);
		EEPROM_ReadBuffer(0x0F58 + (Channel * 0x10), gEeprom.VfoInfo[VFO].Name + 8, 2);
	}

	if (!gEeprom.VfoInfo[VFO].FrequencyReverse) {
		gEeprom.VfoInfo[VFO].pRX = &gEeprom.VfoInfo[VFO].ConfigRX;
		gEeprom.VfoInfo[VFO].pTX = &gEeprom.VfoInfo[VFO].ConfigTX;
	} else {
		gEeprom.VfoInfo[VFO].pRX = &gEeprom.VfoInfo[VFO].ConfigTX;
		gEeprom.VfoInfo[VFO].pTX = &gEeprom.VfoInfo[VFO].ConfigRX;
	}

	if (!gSetting_350EN) {
		FREQ_Config_t *pConfig = gEeprom.VfoInfo[VFO].pRX;
		if (pConfig->Frequency >= 35000000 && pConfig->Frequency <= 39999990) {
			pConfig->Frequency = 41001250;
		}
	}

	if (gEeprom.VfoInfo[VFO].Band == BAND2_108MHz && gEeprom.VfoInfo[VFO].AM_CHANNEL_MODE) {
		gEeprom.VfoInfo[VFO].IsAM = true;
		gEeprom.VfoInfo[VFO].SCRAMBLING_TYPE = 0;
		gEeprom.VfoInfo[VFO].DTMF_DECODING_ENABLE = false;
		gEeprom.VfoInfo[VFO].ConfigRX.CodeType = CODE_TYPE_OFF;
		gEeprom.VfoInfo[VFO].ConfigTX.CodeType = CODE_TYPE_OFF;
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

	Band = FREQUENCY_GetBand(pInfo->pRX->Frequency);
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

	Band = FREQUENCY_GetBand(pInfo->pTX->Frequency);
	EEPROM_ReadBuffer(0x1ED0 + (Band * 0x10) + (pInfo->OUTPUT_POWER * 3), Txp, 3);
	pInfo->TXP_CalculatedSetting =
		FREQUENCY_CalculateOutputPower(
				Txp[0],
				Txp[1],
				Txp[2],
				LowerLimitFrequencyBandTable[Band],
				MiddleFrequencyBandTable[Band],
				UpperLimitFrequencyBandTable[Band],
				pInfo->pTX->Frequency);
}

void RADIO_ApplyOffset(VFO_Info_t *pInfo)
{
	uint32_t Frequency;

	Frequency = pInfo->ConfigRX.Frequency;
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

	pInfo->ConfigTX.Frequency = Frequency;
}

static void RADIO_SelectCurrentVfo(void)
{
	if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
		gCurrentVfo = gRxVfo;
	} else {
		gCurrentVfo = &gEeprom.VfoInfo[gEeprom.TX_CHANNEL];
	}
}

void RADIO_SelectVfos(void)
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

	gTxVfo = &gEeprom.VfoInfo[gEeprom.TX_CHANNEL];
	if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
		gEeprom.RX_CHANNEL = gEeprom.TX_CHANNEL;
	} else {
		if (gEeprom.TX_CHANNEL == 0) {
			gEeprom.RX_CHANNEL = 1;
		} else {
			gEeprom.RX_CHANNEL = 0;
		}
	}

	gRxVfo = &gEeprom.VfoInfo[gEeprom.RX_CHANNEL];
	RADIO_SelectCurrentVfo();
}

void RADIO_SetupRegisters(bool bSwitchToFunction0)
{
	BK4819_FilterBandwidth_t Bandwidth;
	uint16_t Status;
	uint16_t InterruptMask;
	uint32_t Frequency;

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
	gEnableSpeaker = false;
	BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_GREEN, false);

	Bandwidth = gRxVfo->CHANNEL_BANDWIDTH;
	if (Bandwidth != BK4819_FILTER_BW_WIDE) {
		Bandwidth = BK4819_FILTER_BW_NARROW;
	}
	BK4819_SetFilterBandwidth(Bandwidth);

	BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_RED, false);
	BK4819_SetupPowerAmplifier(0, 0);
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1, false);

	while (1) {
		Status = BK4819_ReadRegister(BK4819_REG_0C);
		if ((Status & 1U) == 0) { // INTERRUPT REQUEST
			break;
		}
		BK4819_WriteRegister(BK4819_REG_02, 0);
		SYSTEM_DelayMs(1);
	}
	BK4819_WriteRegister(BK4819_REG_3F, 0);
	BK4819_WriteRegister(BK4819_REG_7D, gEeprom.MIC_SENSITIVITY_TUNING | 0xE940);
#if defined(ENABLE_NOAA)
	if (IS_NOT_NOAA_CHANNEL(gRxVfo->CHANNEL_SAVE) || !gIsNoaaMode) {
		Frequency = gRxVfo->pRX->Frequency;
	} else {
		Frequency = NoaaFrequencyTable[gNoaaChannel];
	}
#else
	Frequency = gRxVfo->pRX->Frequency;
#endif
	BK4819_SetFrequency(Frequency);
	BK4819_SetupSquelch(
			gRxVfo->SquelchOpenRSSIThresh, gRxVfo->SquelchCloseRSSIThresh,
			gRxVfo->SquelchOpenNoiseThresh, gRxVfo->SquelchCloseNoiseThresh,
			gRxVfo->SquelchCloseGlitchThresh, gRxVfo->SquelchOpenGlitchThresh);
	BK4819_PickRXFilterPathBasedOnFrequency(Frequency);
	BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2, true);
	BK4819_WriteRegister(BK4819_REG_48, 0xB3A8);

	InterruptMask = 0
		| BK4819_REG_3F_SQUELCH_FOUND
		| BK4819_REG_3F_SQUELCH_LOST
		;

	if (IS_NOT_NOAA_CHANNEL(gRxVfo->CHANNEL_SAVE)) {
		if (!gRxVfo->IsAM) {
			uint8_t CodeType;
			uint8_t Code;

			CodeType = gSelectedCodeType;
			Code = gSelectedCode;
			if (gCssScanMode == CSS_SCAN_MODE_OFF) {
				CodeType = gRxVfo->pRX->CodeType;
				Code = gRxVfo->pRX->Code;
			}
			switch (CodeType) {
			case CODE_TYPE_DIGITAL:
			case CODE_TYPE_REVERSE_DIGITAL:
				BK4819_SetCDCSSCodeWord(DCS_GetGolayCodeWord(CodeType, Code));
				InterruptMask = 0
					| BK4819_REG_3F_CxCSS_TAIL
					| BK4819_REG_3F_CDCSS_FOUND
					| BK4819_REG_3F_CDCSS_LOST
					| BK4819_REG_3F_SQUELCH_FOUND
					| BK4819_REG_3F_SQUELCH_LOST
					;
				break;
			case CODE_TYPE_CONTINUOUS_TONE:
				BK4819_SetCTCSSFrequency(CTCSS_Options[Code]);
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
				BK4819_SetCTCSSFrequency(670);
				BK4819_Set55HzTailDetection();
				InterruptMask = 0
					| BK4819_REG_3F_CxCSS_TAIL
					| BK4819_REG_3F_SQUELCH_FOUND
					| BK4819_REG_3F_SQUELCH_LOST
					;
				break;
			}
			if (gRxVfo->SCRAMBLING_TYPE == 0 || !gSetting_ScrambleEnable) {
				BK4819_DisableScramble();
			} else {
				BK4819_EnableScramble(gRxVfo->SCRAMBLING_TYPE - 1);
			}
		}
	} else {
		BK4819_SetCTCSSFrequency(2625);
		InterruptMask = 0
			| BK4819_REG_3F_CTCSS_FOUND
			| BK4819_REG_3F_CTCSS_LOST
			| BK4819_REG_3F_SQUELCH_FOUND
			| BK4819_REG_3F_SQUELCH_LOST
			;
	}

	if (gEeprom.VOX_SWITCH
#if defined(ENABLE_FMRADIO)
		&& !gFmRadioMode
#endif
		&& IS_NOT_NOAA_CHANNEL(gCurrentVfo->CHANNEL_SAVE) && !gCurrentVfo->IsAM) {
		BK4819_EnableVox(gEeprom.VOX1_THRESHOLD, gEeprom.VOX0_THRESHOLD);
		InterruptMask |= 0
			| BK4819_REG_3F_VOX_FOUND
			| BK4819_REG_3F_VOX_LOST
			;
	} else {
		BK4819_DisableVox();
	}
	if (gRxVfo->IsAM || (!gRxVfo->DTMF_DECODING_ENABLE && !gSetting_KILLED)) {
		BK4819_DisableDTMF();
	} else {
		BK4819_EnableDTMF();
		InterruptMask |= BK4819_REG_3F_DTMF_5TONE_FOUND;
	}
	BK4819_WriteRegister(BK4819_REG_3F, InterruptMask);

	FUNCTION_Init();

	if (bSwitchToFunction0) {
		FUNCTION_Select(FUNCTION_FOREGROUND);
	}
}

#if defined(ENABLE_NOAA)
void RADIO_ConfigureNOAA(void)
{
	uint8_t ChanAB;

	gUpdateStatus = true;
	if (gEeprom.NOAA_AUTO_SCAN) {
		if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
			if (IS_NOT_NOAA_CHANNEL(gEeprom.ScreenChannel[0])) {
				if (IS_NOT_NOAA_CHANNEL(gEeprom.ScreenChannel[1])) {
					gIsNoaaMode = false;
					return;
				}
				ChanAB = 1;
			} else {
				ChanAB = 0;
			}
			if (!gIsNoaaMode) {
				gNoaaChannel = gEeprom.VfoInfo[ChanAB].CHANNEL_SAVE - NOAA_CHANNEL_FIRST;
			}
			gIsNoaaMode = true;
			return;
		}
		if (gRxVfo->CHANNEL_SAVE >= NOAA_CHANNEL_FIRST) {
			gIsNoaaMode = true;
			gNoaaChannel = gRxVfo->CHANNEL_SAVE - NOAA_CHANNEL_FIRST;
			gNOAA_Countdown = 50;
			gScheduleNOAA = false;
		} else {
			gIsNoaaMode = false;
		}
	} else {
		gIsNoaaMode = false;
	}
}
#endif

void RADIO_SetTxParameters(void)
{
	BK4819_FilterBandwidth_t Bandwidth;

	GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

	gEnableSpeaker = false;

	BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2, false);
	Bandwidth = gCurrentVfo->CHANNEL_BANDWIDTH;
	if (Bandwidth != BK4819_FILTER_BW_WIDE) {
		Bandwidth = BK4819_FILTER_BW_NARROW;
	}
	BK4819_SetFilterBandwidth(Bandwidth);
	BK4819_SetFrequency(gCurrentVfo->pTX->Frequency);
	BK4819_PrepareTransmit();
	SYSTEM_DelayMs(10);

	BK4819_PickRXFilterPathBasedOnFrequency(gCurrentVfo->pTX->Frequency);
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1, true);
	SYSTEM_DelayMs(5);

	BK4819_SetupPowerAmplifier(gCurrentVfo->TXP_CalculatedSetting, gCurrentVfo->pTX->Frequency);
	SYSTEM_DelayMs(10);

	switch (gCurrentVfo->pTX->CodeType) {
	case CODE_TYPE_CONTINUOUS_TONE:
		BK4819_SetCTCSSFrequency(CTCSS_Options[gCurrentVfo->pTX->Code]);
		break;
	case CODE_TYPE_DIGITAL:
	case CODE_TYPE_REVERSE_DIGITAL:
		BK4819_SetCDCSSCodeWord(
			DCS_GetGolayCodeWord(
				gCurrentVfo->pTX->CodeType,
				gCurrentVfo->pTX->Code
				)
			);
		break;
	default:
		BK4819_ExitSubAu();
		break;
	}
}

void RADIO_SetVfoState(VfoState_t State)
{
	if (State == VFO_STATE_NORMAL) {
		VfoState[0] = VFO_STATE_NORMAL;
		VfoState[1] = VFO_STATE_NORMAL;
#if defined(ENABLE_FMRADIO)
		gFM_ResumeCountdown = 0;
#endif
	} else {
		if (State == VFO_STATE_VOL_HIGH) {
			VfoState[0] = VFO_STATE_VOL_HIGH;
			VfoState[1] = VFO_STATE_TX_DISABLE;
		} else {
			uint8_t Channel;

			if (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF) {
				Channel = gEeprom.RX_CHANNEL;
			} else {
				Channel = gEeprom.TX_CHANNEL;
			}
			VfoState[Channel] = State;
		}
#if defined(ENABLE_FMRADIO)
		gFM_ResumeCountdown = 5;
#endif
	}
	gUpdateDisplay = true;
}

void RADIO_PrepareTX(void)
{
	if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) {
		gDualWatchCountdown = 360;
		gScheduleDualWatch = false;
		if (!gRxVfoIsActive) {
			gEeprom.RX_CHANNEL = gEeprom.TX_CHANNEL;
			gRxVfo = &gEeprom.VfoInfo[gEeprom.TX_CHANNEL];
		}
		gRxVfoIsActive = true;
	}
	RADIO_SelectCurrentVfo();
#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
	if (gAlarmState == ALARM_STATE_OFF
#if defined(ENABLE_TX1750)
		|| gAlarmState == ALARM_STATE_TX1750
#endif
#if defined(ENABLE_ALARM)
		|| (gAlarmState == ALARM_STATE_ALARM && gEeprom.ALARM_MODE == ALARM_MODE_TONE)
#endif
		) {
#else
	if (1) {
#endif
		VfoState_t State;

		if (!FREQUENCY_Check(gCurrentVfo)) {
			if (gCurrentVfo->BUSY_CHANNEL_LOCK && gCurrentFunction == FUNCTION_RECEIVE) {
				State = VFO_STATE_BUSY;
			} else if (gBatteryDisplayLevel == 0) {
				State = VFO_STATE_BAT_LOW;
			} else if (gBatteryDisplayLevel == 6) {
				State = VFO_STATE_VOL_HIGH;
			} else {
				goto Skip;
			}
		} else {
			State = VFO_STATE_TX_DISABLE;
		}
		RADIO_SetVfoState(State);
#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
		gAlarmState = ALARM_STATE_OFF;
#endif
		AUDIO_PlayBeep(BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
		gDTMF_ReplyState = DTMF_REPLY_NONE;
		return;
	}

Skip:
	if (gDTMF_ReplyState == DTMF_REPLY_ANI) {
		if (gDTMF_CallMode == DTMF_CALL_MODE_DTMF) {
			gDTMF_IsTx = true;
			gDTMF_CallState = DTMF_CALL_STATE_NONE;
			gDTMF_TxStopCountdown = 6;
		} else {
			gDTMF_CallState = DTMF_CALL_STATE_CALL_OUT;
			gDTMF_IsTx = false;
		}
	}
	FUNCTION_Select(FUNCTION_TRANSMIT);
#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
	if (gAlarmState == ALARM_STATE_OFF) {
		gTxTimerCountdown = gEeprom.TX_TIMEOUT_TIMER * 120;
	} else {
		gTxTimerCountdown = 0;
	}
#else
	gTxTimerCountdown = gEeprom.TX_TIMEOUT_TIMER * 120;
#endif
	gTxTimeoutReached = false;
	gFlagEndTransmission = false;
	gRTTECountdown = 0;
	gDTMF_ReplyState = DTMF_REPLY_NONE;
}

void RADIO_EnableCxCSS(void)
{
	switch (gCurrentVfo->pTX->CodeType) {
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

void RADIO_PrepareCssTX(void)
{
	RADIO_PrepareTX();
	SYSTEM_DelayMs(200);
	RADIO_EnableCxCSS();
	RADIO_SetupRegisters(true);
}

void RADIO_SendEndOfTransmission(void)
{
	if (gEeprom.ROGER == ROGER_MODE_ROGER) {
		BK4819_PlayRoger();
	} else if (gEeprom.ROGER == ROGER_MODE_MDC) {
		BK4819_PlayRogerMDC();
	}
	if (gDTMF_CallState == DTMF_CALL_STATE_NONE && (gCurrentVfo->DTMF_PTT_ID_TX_MODE == PTT_ID_EOT || gCurrentVfo->DTMF_PTT_ID_TX_MODE == PTT_ID_BOTH)) {
		if (gEeprom.DTMF_SIDE_TONE) {
			GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
			gEnableSpeaker = true;
			SYSTEM_DelayMs(60);
		}
		BK4819_EnterDTMF_TX(gEeprom.DTMF_SIDE_TONE);
		BK4819_PlayDTMFString(
			gEeprom.DTMF_DOWN_CODE,
			0,
			gEeprom.DTMF_FIRST_CODE_PERSIST_TIME,
			gEeprom.DTMF_HASH_CODE_PERSIST_TIME,
			gEeprom.DTMF_CODE_PERSIST_TIME,
			gEeprom.DTMF_CODE_INTERVAL_TIME
			);
		GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
		gEnableSpeaker = false;
	}
	BK4819_ExitDTMF_TX(true);
}

