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

const uint32_t LowerLimitFrequencyBandTable[7] = {
	 5000000,
	10800000,
	13600000,
	17400000,
	35000000,
	40000000,
	47000000,
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

const uint32_t MiddleFrequencyBandTable[7] = {
	 6500000,
	12200000,
	15000000,
	26000000,
	37000000,
	43500000,
	55000000,
};

