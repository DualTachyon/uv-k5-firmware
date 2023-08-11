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

#ifndef BK4819_REGS_H
#define BK4819_REGS_H

enum BK4819_REGISTER_t {
	BK4819_REG_00                                                                 = 0x00U,
	BK4819_REG_02                                                                 = 0x02U,
	BK4819_REG_06                                                                 = 0x06U,
	BK4819_REG_07_CTC_FREQUENCY_CDCSS_BAUD_RATE                                   = 0x07U,
	BK4819_REG_08_CDCSS_CALL_SYMBOL_NUMBER_AND_COEFFICIENT                        = 0x08U,
	BK4819_REG_09                                                                 = 0x09U,
	BK4819_REG_0B_FSK_STATUS                                                      = 0x0BU,
	BK4819_REG_0C_CDCSS_CTCSS_STATUS_VOX_SQ_INTR_INDICATOR                        = 0x0CU,
	BK4819_REG_0D_FREQUENCY_SCAN_INDICATOR_FREQ_HIGH                              = 0x0DU,
	BK4819_REG_0E_FREQUENCY_SCAN_FREQ_LOW                                         = 0x0EU,
	BK4819_REG_10                                                                 = 0x10U,
	BK4819_REG_11                                                                 = 0x11U,
	BK4819_REG_12                                                                 = 0x12U,
	BK4819_REG_13                                                                 = 0x13U,
	BK4819_REG_14                                                                 = 0x14U,
	BK4819_REG_19                                                                 = 0x19U,
	BK4819_REG_1F                                                                 = 0x1FU,
	BK4819_REG_20                                                                 = 0x20U,
	BK4819_REG_21                                                                 = 0x21U,
	BK4819_REG_24_DTMF_SELCALL_CONFIG                                             = 0x24U,
	BK4819_REG_28_EXPANDER_CONFIG                                                 = 0x28U,
	BK4819_REG_29_COMPRESS_CONFIG                                                 = 0x29U,
	BK4819_REG_2B_AF_FILTER_DISABLE                                               = 0x2BU,
	BK4819_REG_30_VARIOUS_ENABLE_BITS                                             = 0x30U,
	BK4819_REG_31_ENABLE_COMPANDER_VOX_SCRAMBLE                                   = 0x31U,
	BK4819_REG_32_FREQUENCY_SCAN                                                  = 0x32U,
	BK4819_REG_33                                                                 = 0x33U,
	BK4819_REG_36_POWER_AMPLIFIER_BIAS_AND_GAIN                                   = 0x36U,
	BK4819_REG_37_VOLTAGE_LDO_AND_MISC_ENABLE                                     = 0x37U,
	BK4819_REG_38_FREQUENCY_LOW                                                   = 0x38U,
	BK4819_REG_39_FREQUENCY_HIGH                                                  = 0x39U,
	BK4819_REG_3A_RSSI_THRESHOLD_FOR_SQ1_SQ0                                      = 0x3AU,
	BK4819_REG_3B_XTAL_FREQ_LOW                                                   = 0x3BU,
	BK4819_REG_3C_XTAL_FREQ_HIGH                                                  = 0x3CU,
	BK4819_REG_3E                                                                 = 0x3EU,
	BK4819_REG_3F_INTERRUPT_ENABLE                                                = 0x3FU,
	BK4819_REG_43_RF_AFT_LPF2_FILTER_BANDWIDTH                                    = 0x43U,
	BK4819_REG_46_VOICE_AMPLITUDE_THRESHOLD_VOX1                                  = 0x46U,
	BK4819_REG_47_AF_OUTPUT_CONFIG                                                = 0x47U,
	BK4819_REG_48_AF_RF_GAIN_DAC_GAIN                                             = 0x48U,
	BK4819_REG_49                                                                 = 0x49U,
	BK4819_REG_4D_GLITCH_THR_SQ0                                                  = 0x4DU,
	BK4819_REG_4E_SQ1_DELAY_SQ0_DELAY_GLITCH_THR_SQ1                              = 0x4EU,
	BK4819_REG_4F_EXNOISE_THR_SQ0_EXNOISE_THR_SQ1                                 = 0x4FU,
	BK4819_REG_51_CTCSS_CDCSS_CONFIG                                              = 0x51U,
	BK4819_REG_58_FSK_CONFIG_BITS                                                 = 0x58U,
	BK4819_REG_59_VARIOUS_FSK_ENABLE_AND_CONFIG_BITS                              = 0x59U,
	BK4819_REG_5A_FSK_SYNC_BYTES_0_1                                              = 0x5AU,
	BK4819_REG_5B_FSK_SYNC_BYTES_2_3                                              = 0x5BU,
	BK4819_REG_5C_CRC_ENABLE_AND_UNKNOWN_BITS                                     = 0x5CU,
	BK4819_REG_5D_FSK_DATA_LENGTH                                                 = 0x5DU,
	BK4819_REG_5F_FSK_WORD_INPUT_OUTPUT                                           = 0x5FU,
	BK4819_REG_64_VOXAMP                                                          = 0x64U,
	BK4819_REG_67_RSSI                                                            = 0x67U,
	BK4819_REG_68_CTCSS_SCAN_INDICATOR_FREQ                                       = 0x68U,
	BK4819_REG_69_CDCSS_SCAN_INDICATOR_FREQ_HIGH                                  = 0x69U,
	BK4819_REG_6A_CDCSS_SCAN_FREQ_LOW                                             = 0x6AU,
	BK4819_REG_6F_AF_TX_RX_INPUT_AMPLITUDE                                        = 0x6FU,
	BK4819_REG_70_TONE1_CONFIG                                                    = 0x70U,
	BK4819_REG_71_TONE1_SCRAMBLE_FREQ_CONTROL_WORD                                = 0x71U,
	BK4819_REG_72_TONE2_FSK_FREQUENCY_CONTROL_WORD                                = 0x72U,
	BK4819_REG_78_SQUELCH_RSSI_THRESHOLD                                          = 0x78U,
	BK4819_REG_79_VOX1_DETECTION_INTERVAL_TIME_VOICE_AMPLITUDE_THRESHOLD_VOX0     = 0x79U,
	BK4819_REG_7A_VOX0_DETECTION_DELAY                                            = 0x7AU,
	BK4819_REG_7B                                                                 = 0x7BU,
	BK4819_REG_7C                                                                 = 0x7CU,
	BK4819_REG_7D_MIC_SENSITIVITY_TUNING                                          = 0x7DU,
	BK4819_REG_7E_DC_FILTER_BANDWIDTH_AND_AGC_FIX                                 = 0x7EU,
};

typedef enum BK4819_REGISTER_t BK4819_REGISTER_t;

enum BK4819_GPIO_PIN_t {
	BK4819_GPIO6_PIN2 = 0,
	BK4819_GPIO5_PIN1 = 1,
	BK4819_GPIO4_PIN32 = 2,
	BK4819_GPIO3_PIN31 = 3,
	BK4819_GPIO2_PIN30 = 4,
	BK4819_GPIO1_PIN29 = 5,
	BK4819_GPIO0_PIN28 = 6,
};

typedef enum BK4819_GPIO_PIN_t BK4819_GPIO_PIN_t;

#endif

