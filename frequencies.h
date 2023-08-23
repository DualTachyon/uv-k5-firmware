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

#ifndef FREQUENCIES_H
#define FREQUENCIES_H

#include <stdint.h>
#include "radio.h"

enum FREQUENCY_Band_t {
	BAND1_50MHz = 0,
	BAND2_108MHz,
	BAND3_136MHz,
	BAND4_174MHz,
	BAND5_350MHz,
	BAND6_400MHz,
	BAND7_470MHz,
};

typedef enum FREQUENCY_Band_t FREQUENCY_Band_t;

extern const uint32_t LowerLimitFrequencyBandTable[7];
extern const uint32_t MiddleFrequencyBandTable[7];
extern const uint32_t UpperLimitFrequencyBandTable[7];
extern const uint32_t NoaaFrequencyTable[10];
extern const uint16_t StepFrequencyTable[6];

FREQUENCY_Band_t FREQUENCY_GetBand(uint32_t Frequency);
uint32_t FREQUENCY_CalculateOutputPower(uint8_t TxpLow, uint8_t TxpMid, uint8_t TxpHigh, uint32_t LowerLimit, uint32_t Middle, uint32_t UpperLimit, uint32_t Frequency);
uint32_t FREQUENCY_FloorToStep(uint32_t Frequency, uint32_t Step, uint32_t Base);
int FREQUENCY_Check(VFO_Info_t *pInfo);

#endif

