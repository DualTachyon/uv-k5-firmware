/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HARDWARE_DP32G030_SARADC_H
#define HARDWARE_DP32G030_SARADC_H

#if !defined(__ASSEMBLY__)
#include <stdint.h>
#endif

/* -------- SARADC -------- */
#define SARADC_BASE_ADDR                            0x400BA000U
#define SARADC_BASE_SIZE                            0x00000800U

#define SARADC_ADC_CALIB_OFFSET_ADDR                (SARADC_BASE_ADDR + 0x00F0U)
#define SARADC_ADC_CALIB_OFFSET                     (*(volatile uint32_t *)SARADC_ADC_CALIB_OFFSET_ADDR)
#define SARADC_ADC_CALIB_OFFSET_OFFSET_SHIFT        0
#define SARADC_ADC_CALIB_OFFSET_OFFSET_WIDTH        8
#define SARADC_ADC_CALIB_OFFSET_OFFSET_MASK         (((1U << SARADC_ADC_CALIB_OFFSET_OFFSET_WIDTH) - 1U) << SARADC_ADC_CALIB_OFFSET_OFFSET_SHIFT)
#define SARADC_ADC_CALIB_OFFSET_OFFSET_VALID_SHIFT  16
#define SARADC_ADC_CALIB_OFFSET_OFFSET_VALID_WIDTH  1
#define SARADC_ADC_CALIB_OFFSET_OFFSET_VALID_MASK   (((1U << SARADC_ADC_CALIB_OFFSET_OFFSET_VALID_WIDTH) - 1U) << SARADC_ADC_CALIB_OFFSET_OFFSET_VALID_SHIFT)

#define SARADC_ADC_CALIB_KD_ADDR                    (SARADC_BASE_ADDR + 0x00F4U)
#define SARADC_ADC_CALIB_KD                         (*(volatile uint32_t *)SARADC_ADC_CALIB_KD_ADDR)
#define SARADC_ADC_CALIB_KD_KD_SHIFT                0
#define SARADC_ADC_CALIB_KD_KD_WIDTH                8
#define SARADC_ADC_CALIB_KD_KD_MASK                 (((1U << SARADC_ADC_CALIB_KD_KD_WIDTH) - 1U) << SARADC_ADC_CALIB_KD_KD_SHIFT)
#define SARADC_ADC_CALIB_KD_KD_VALID_SHIFT          16
#define SARADC_ADC_CALIB_KD_KD_VALID_WIDTH          1
#define SARADC_ADC_CALIB_KD_KD_VALID_MASK           (((1U << SARADC_ADC_CALIB_KD_KD_VALID_WIDTH) - 1U) << SARADC_ADC_CALIB_KD_KD_VALID_SHIFT)


#endif

