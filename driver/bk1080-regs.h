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

#ifndef BK1080_REGS_H
#define BK1080_REGS_H

enum BK1080_REGISTER_t {
	BK1080_REG_00                       = 0x00U,
	BK1080_REG_02_POWER_CONFIGURATION   = 0x02U,
	BK1080_REG_03_CHANNEL               = 0x03U,
	BK1080_REG_05_SYSTEM_CONFIGURATION2 = 0x03U,
	BK1080_REG_19_INTERNAL              = 0x19U,
};

typedef enum BK1080_REGISTER_t BK1080_REGISTER_t;

#endif

