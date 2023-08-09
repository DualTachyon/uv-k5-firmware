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

#include <stdint.h>

extern uint8_t overlay_text_start[];
extern uint8_t overlay_text_end[];

extern uint8_t overlay_bss_start[];
extern uint8_t overlay_bss_end[];

extern uint8_t overlay_data_start[];
extern uint8_t overlay_data_end[];

extern const uint8_t sram_overlay_bin[];

void OVERLAY_Install(void)
{
	uint8_t *pStart;
	uint8_t *pEnd;
	const uint8_t *pIn;
	uint8_t *pOut;

	// Text
	pStart = overlay_text_start;
	pEnd = overlay_text_end;

	pIn = sram_overlay_bin;

	for (pOut = pStart; pOut < pEnd; ) {
		*pOut++ = *pIn++;
	}

	// Data
	pStart = overlay_data_start;
	pEnd = overlay_data_end;

	pIn = sram_overlay_bin;
	pIn += overlay_data_start - overlay_text_start;

	for (pOut = pStart; pOut < pEnd; ) {
		*pOut++ = *pIn++;
	}

	// BSS
	pStart = overlay_bss_start;
	pEnd = overlay_bss_end;

	for (pOut = pStart; pOut < pEnd; ) {
		*pOut++ = 0;
	}
}

