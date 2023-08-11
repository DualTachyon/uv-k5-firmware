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

#ifndef DRIVER_BK4819_h
#define DRIVER_BK4819_h

#include <stdbool.h>
#include <stdint.h>
#include "driver/bk4819-regs.h"

void BK4819_Init(void);
uint16_t BK4819_GetRegister(BK4819_REGISTER_t Register);
void BK4819_WriteRegister(BK4819_REGISTER_t Register, uint16_t Data);
void BK4819_WriteU8(uint8_t Data);
void BK4819_WriteU16(uint16_t Data);

void BK4819_SetAGC(uint8_t Value);

void BK4819_ToggleGpioOut(BK4819_GPIO_PIN_t Pin, bool bSet);

void BK4819_EnableCDCSS(uint32_t CodeWord);
void BK4819_EnableCTCSS(uint32_t BaudRate);
void BK4819_Set55HzTailDetection(void);
void BK4819_EnableVox(uint16_t Vox1Threshold, uint16_t Vox0Threshold);
void BK4819_SetFilterBandwidth(uint8_t Bandwidth);
void BK4819_SetupPowerAmplifier(uint16_t Bias, uint32_t Frequency);
void BK4819_SetFrequency(uint32_t Frequency);
void BK4819_SetupSquelch(uint8_t SQ0, uint8_t SQ1, uint8_t SQ2, uint8_t SQ3, uint8_t SQ4, uint8_t SQ5);
void BK4819_SetAF(uint8_t AF);
void BK4819_RX_TurnOn(void);
void BK4819_Configure_GPIO2_PIN30_GPIO3_PIN31(uint32_t Frequency);
void BK4819_DisableScramble(void);
void BK4819_EnableScramble(uint8_t Type);
void BK4819_DisableVox(void);
void BK4819_DisableDTMF_SelCall(void);
void BK4819_ConfigureDTMF_SelCall_and_UnknownRegister(void);

#endif

