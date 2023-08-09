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

#ifndef SRAM_OVERLAY_H
#define SRAM_OVERLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/flash.h"

extern uint32_t overlay_FLASH_MainClock;
extern uint32_t overlay_FLASH_ClockMultiplier;
extern uint32_t overlay_0x20000478;

bool overlay_FLASH_RebootToBootloader(void);
bool overlay_FLASH_IsBusy(void);
bool overlay_FLASH_IsInitComplete(void);
void overlay_FLASH_Start(void);
void overlay_FLASH_Init(FLASH_READ_MODE ReadMode);
void overlay_FLASH_MaskLock(void);
void overlay_FLASH_SetMaskSel(FLASH_MASK_SELECTION Mask);
void overlay_FLASH_MaskUnlock(void);
void overlay_FLASH_Lock(void);
void overlay_FLASH_Unlock(void);
uint32_t overlay_FLASH_ReadByAHB(uint32_t Offset);
uint32_t overlay_FLASH_ReadByAPB(uint32_t Offset);
void overlay_FLASH_Set_NVR_SEL(FLASH_AREA Area);
void overlay_FLASH_SetReadMode(FLASH_READ_MODE Mode);
void overlay_FLASH_SetEraseTime(void);
void overlay_FLASH_WakeFromDeepSleep(void);
void overlay_FLASH_SetMode(FLASH_MODE Mode);
void overlay_FLASH_SetProgramTime(void);
void overlay_SystemReset(void);
uint32_t overlay_FLASH_ReadNvrWord(uint32_t Offset);
void overlay_FLASH_ConfigureTrimValues(void);

#endif

