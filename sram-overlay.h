#ifndef OVERLAY_H
#define OVERLAY_H

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

