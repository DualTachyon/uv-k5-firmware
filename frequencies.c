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

#include "frequencies.h"
#include "misc.h"
#include "settings.h"

const uint32_t LowerLimitFrequencyBandTable[7] = {
	 5000000,
	10800000,
	13600000,
	17400000,
	35000000,
	40000000,
	47000000,
};

const uint32_t MiddleFrequencyBandTable[7] = {
	 6500000,
	12200000,
	15000000,
	26000000,
	37000000,
	43500000,
	55000000,
};

const uint32_t UpperLimitFrequencyBandTable[7] = {
	 7600000,
	13599990,
	17399990,
	34999990,
	39999990,
	46999990,
	60000000,
};

#if defined(ENABLE_NOAA)
const uint32_t NoaaFrequencyTable[10] = {
	16255000,
	16240000,
	16247500,
	16242500,
	16245000,
	16250000,
	16252500,
	16152500,
	16177500,
	16327500,
};
#endif

const uint16_t StepFrequencyTable[7] = {
	250,
	500,
	625,
	1000,
	1250,
	2500,
	833,
};

FREQUENCY_Band_t FREQUENCY_GetBand(uint32_t Frequency)
{
	if (Frequency >=  5000000 && Frequency <=  7600000) {
		return BAND1_50MHz;
	}
	if (Frequency >= 10800000 && Frequency <= 13599990) {
		return BAND2_108MHz;
	}
	if (Frequency >= 13600000 && Frequency <= 17399990) {
		return BAND3_136MHz;
	}
	if (Frequency >= 17400000 && Frequency <= 34999990) {
		return BAND4_174MHz;
	}
	if (Frequency >= 35000000 && Frequency <= 39999990) {
		return BAND5_350MHz;
	}
	if (Frequency >= 40000000 && Frequency <= 46999990) {
		return BAND6_400MHz;
	}
	if (Frequency >= 47000000 && Frequency <= 60000000) {
		return BAND7_470MHz;
	}

	return BAND6_400MHz;
}

uint8_t FREQUENCY_CalculateOutputPower(uint8_t TxpLow, uint8_t TxpMid, uint8_t TxpHigh, int32_t LowerLimit, int32_t Middle, int32_t UpperLimit, int32_t Frequency)
{
	if (Frequency <= LowerLimit) {
		return TxpLow;
	}
	if (UpperLimit <= Frequency) {
		return TxpHigh;
	}
	if (Frequency <= Middle) {
		TxpMid += ((TxpMid - TxpLow) * (Frequency - LowerLimit)) / (Middle - LowerLimit);
		return TxpMid;
	}

	TxpMid += ((TxpHigh - TxpMid) * (Frequency - Middle)) / (UpperLimit - Middle);
	return TxpMid;
}

uint32_t FREQUENCY_FloorToStep(uint32_t Upper, uint32_t Step, uint32_t Lower)
{
	uint32_t Index;

	if (Step == 833) {
		const uint32_t Delta = Upper - Lower;
		uint32_t Base = (Delta / 2500) * 2500;
		const uint32_t Index = ((Delta - Base) % 2500) / 833;

		if (Index == 2) {
			Base++;
		}

		return Lower + Base + (Index * 833);
	}

	Index = (Upper - Lower) / Step;

	return Lower + (Step * Index);
}

int FREQUENCY_Check(VFO_Info_t *pInfo)
{
	uint32_t Frequency;

	if (pInfo->CHANNEL_SAVE >= NOAA_CHANNEL_FIRST) {
		return -1;
	}
	Frequency = pInfo->pTX->Frequency;
	switch (gSetting_F_LOCK) {
	case F_LOCK_FCC:
		if (Frequency >= 14400000 && Frequency <= 14799990) {
			return 0;
		}
		if (Frequency >= 42000000 && Frequency <= 44999990) {
			return 0;
		}
		break;

	case F_LOCK_CE:
		if (Frequency >= 14400000 && Frequency <= 14599990) {
			return 0;
		}
		break;

	case F_LOCK_GB:
		if (Frequency >= 14400000 && Frequency <= 14799990) {
			return 0;
		}
		if (Frequency >= 43000000 && Frequency <= 43999990) {
			return 0;
		}
		break;

	case F_LOCK_430:
		if (Frequency >= 13600000 && Frequency <= 17399990) {
			return 0;
		}
		if (Frequency >= 40000000 && Frequency <= 42999990) {
			return 0;
		}
		break;

	case F_LOCK_438:
		if (Frequency >= 13600000 && Frequency <= 17399990) {
			return 0;
		}
		if (Frequency >= 40000000 && Frequency <= 43799990) {
			return 0;
		}
		break;

	default:
		if (Frequency >= 13600000 && Frequency <= 17399990) {
			return 0;
		}
		if (Frequency >= 35000000 && Frequency <= 39999990) {
			if (gSetting_350TX && gSetting_350EN) {
				return 0;
			}
		}
		if (Frequency >= 40000000 && Frequency <= 46999990) {
			return 0;
		}
		if (Frequency >= 17400000 && Frequency <= 34999990) {
			if (gSetting_200TX) {
				return 0;
			}
		}

		if (Frequency >= 47000000 && Frequency <= 60000000) {
			if (gSetting_500TX) {
				return 0;
			}
		}
		break;
	}

	return -1;
}

