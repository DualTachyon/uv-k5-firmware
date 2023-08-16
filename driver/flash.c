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

#include "driver/flash.h"
#include "sram-overlay.h"

__attribute__ ((section (".sram_overlay")))  void FLASH_Init(FLASH_READ_MODE ReadMode)
{
	overlay_FLASH_Init(ReadMode);
}

__attribute__ ((section (".sram_overlay"))) void FLASH_ConfigureTrimValues(void)
{
	overlay_FLASH_ConfigureTrimValues();
}

__attribute__ ((section (".sram_overlay"))) uint32_t FLASH_ReadNvrWord(uint32_t Address)
{
	return overlay_FLASH_ReadNvrWord(Address);
}

