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

#ifndef HARDWARE_DP32G030_SPI_H
#define HARDWARE_DP32G030_SPI_H

#if !defined(__ASSEMBLY__)
#include <stdint.h>
#endif

/* -------- SPI0 -------- */
#define SPI0_BASE_ADDR                 0x400B8000U
#define SPI0_BASE_SIZE                 0x00000800U
#define SPI0                           ((volatile SPI_Port_t *)SPI0_BASE_ADDR)

/* -------- SPI1 -------- */
#define SPI1_BASE_ADDR                 0x400B8800U
#define SPI1_BASE_SIZE                 0x00000800U
#define SPI1                           ((volatile SPI_Port_t *)SPI1_BASE_ADDR)

/* -------- SPI -------- */

typedef struct {
	uint32_t CR;
	uint32_t WDR;
	uint32_t RDR;
	uint32_t Reserved_000C[1];
	uint32_t IE;
	uint32_t IF;
	uint32_t FIFOST;
} SPI_Port_t;

#define SPI_CR_SPR_SHIFT               0
#define SPI_CR_SPR_WIDTH               3
#define SPI_CR_SPR_MASK                (((1U << SPI_CR_SPR_WIDTH) - 1U) << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_4         (0U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_8         (1U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_16        (2U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_32        (3U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_64        (4U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_128       (5U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_256       (6U << SPI_CR_SPR_SHIFT)
#define SPI_CR_SPR_FPCLK_DIV_512       (7U << SPI_CR_SPR_SHIFT)

#define SPI_CR_SPE_SHIFT               3
#define SPI_CR_SPE_WIDTH               1
#define SPI_CR_SPE_MASK                (((1U << SPI_CR_SPE_WIDTH) - 1U) << SPI_CR_SPE_SHIFT)
#define SPI_CR_SPE_DISABLE             (0U << SPI_CR_SPE_SHIFT)
#define SPI_CR_SPE_ENABLE              (1U << SPI_CR_SPE_SHIFT)

#define SPI_CR_CPHA_SHIFT              4
#define SPI_CR_CPHA_WIDTH              1
#define SPI_CR_CPHA_MASK               (((1U << SPI_CR_CPHA_WIDTH) - 1U) << SPI_CR_CPHA_SHIFT)
#define SPI_CR_CPOL_SHIFT              5
#define SPI_CR_CPOL_WIDTH              1
#define SPI_CR_CPOL_MASK               (((1U << SPI_CR_CPOL_WIDTH) - 1U) << SPI_CR_CPOL_SHIFT)
#define SPI_CR_MSTR_SHIFT              6
#define SPI_CR_MSTR_WIDTH              1
#define SPI_CR_MSTR_MASK               (((1U << SPI_CR_MSTR_WIDTH) - 1U) << SPI_CR_MSTR_SHIFT)
#define SPI_CR_LSB_SHIFT               7
#define SPI_CR_LSB_WIDTH               1
#define SPI_CR_LSB_MASK                (((1U << SPI_CR_LSB_WIDTH) - 1U) << SPI_CR_LSB_SHIFT)
#define SPI_CR_CPHA_DATA_HOLD_S_SHIFT  8
#define SPI_CR_CPHA_DATA_HOLD_S_WIDTH  4
#define SPI_CR_CPHA_DATA_HOLD_S_MASK   (((1U << SPI_CR_CPHA_DATA_HOLD_S_WIDTH) - 1U) << SPI_CR_CPHA_DATA_HOLD_S_SHIFT)
#define SPI_CR_MSR_SSN_SHIFT           12
#define SPI_CR_MSR_SSN_WIDTH           1
#define SPI_CR_MSR_SSN_MASK            (((1U << SPI_CR_MSR_SSN_WIDTH) - 1U) << SPI_CR_MSR_SSN_SHIFT)
#define SPI_CR_MSR_SSN_DISABLE         (0U << SPI_CR_MSR_SSN_SHIFT)
#define SPI_CR_MSR_SSN_ENABLE          (1U << SPI_CR_MSR_SSN_SHIFT)

#define SPI_CR_RXDMAEN_SHIFT           13
#define SPI_CR_RXDMAEN_WIDTH           1
#define SPI_CR_RXDMAEN_MASK            (((1U << SPI_CR_RXDMAEN_WIDTH) - 1U) << SPI_CR_RXDMAEN_SHIFT)
#define SPI_CR_TXDMAEN_SHIFT           14
#define SPI_CR_TXDMAEN_WIDTH           1
#define SPI_CR_TXDMAEN_MASK            (((1U << SPI_CR_TXDMAEN_WIDTH) - 1U) << SPI_CR_TXDMAEN_SHIFT)
#define SPI_CR_RF_CLR_SHIFT            15
#define SPI_CR_RF_CLR_WIDTH            1
#define SPI_CR_RF_CLR_MASK             (((1U << SPI_CR_RF_CLR_WIDTH) - 1U) << SPI_CR_RF_CLR_SHIFT)
#define SPI_CR_TF_CLR_SHIFT            16
#define SPI_CR_TF_CLR_WIDTH            1
#define SPI_CR_TF_CLR_MASK             (((1U << SPI_CR_TF_CLR_WIDTH) - 1U) << SPI_CR_TF_CLR_SHIFT)

#define SPI_IE_RXFIFO_OVF_SHIFT        0
#define SPI_IE_RXFIFO_OVF_WIDTH        1
#define SPI_IE_RXFIFO_OVF_MASK         (((1U << SPI_IE_RXFIFO_OVF_WIDTH) - 1U) << SPI_IE_RXFIFO_OVF_SHIFT)
#define SPI_IE_RXFIFO_OVF_DISABLE      (0U << SPI_IE_RXFIFO_OVF_SHIFT)
#define SPI_IE_RXFIFO_OVF_ENABLE       (1U << SPI_IE_RXFIFO_OVF_SHIFT)

#define SPI_IE_RXFIFO_FULL_SHIFT       1
#define SPI_IE_RXFIFO_FULL_WIDTH       1
#define SPI_IE_RXFIFO_FULL_MASK        (((1U << SPI_IE_RXFIFO_FULL_WIDTH) - 1U) << SPI_IE_RXFIFO_FULL_SHIFT)
#define SPI_IE_RXFIFO_FULL_DISABLE     (0U << SPI_IE_RXFIFO_FULL_SHIFT)
#define SPI_IE_RXFIFO_FULL_ENABLE      (1U << SPI_IE_RXFIFO_FULL_SHIFT)

#define SPI_IE_RXFIFO_HFULL_SHIFT      2
#define SPI_IE_RXFIFO_HFULL_WIDTH      1
#define SPI_IE_RXFIFO_HFULL_MASK       (((1U << SPI_IE_RXFIFO_HFULL_WIDTH) - 1U) << SPI_IE_RXFIFO_HFULL_SHIFT)
#define SPI_IE_RXFIFO_HFULL_DISABLE    (0U << SPI_IE_RXFIFO_HFULL_SHIFT)
#define SPI_IE_RXFIFO_HFULL_ENABLE     (1U << SPI_IE_RXFIFO_HFULL_SHIFT)

#define SPI_IE_TXFIFO_EMPTY_SHIFT      3
#define SPI_IE_TXFIFO_EMPTY_WIDTH      1
#define SPI_IE_TXFIFO_EMPTY_MASK       (((1U << SPI_IE_TXFIFO_EMPTY_WIDTH) - 1U) << SPI_IE_TXFIFO_EMPTY_SHIFT)
#define SPI_IE_TXFIFO_EMPTY_DISABLE    (0U << SPI_IE_TXFIFO_EMPTY_SHIFT)
#define SPI_IE_TXFIFO_EMPTY_ENABLE     (1U << SPI_IE_TXFIFO_EMPTY_SHIFT)

#define SPI_IE_TXFIFO_HFULL_SHIFT      4
#define SPI_IE_TXFIFO_HFULL_WIDTH      1
#define SPI_IE_TXFIFO_HFULL_MASK       (((1U << SPI_IE_TXFIFO_HFULL_WIDTH) - 1U) << SPI_IE_TXFIFO_HFULL_SHIFT)
#define SPI_IE_TXFIFO_HFULL_DISABLE    (0U << SPI_IE_TXFIFO_HFULL_SHIFT)
#define SPI_IE_TXFIFO_HFULL_ENABLE     (1U << SPI_IE_TXFIFO_HFULL_SHIFT)

#define SPI_FIFOST_RFE_SHIFT           0
#define SPI_FIFOST_RFE_WIDTH           1
#define SPI_FIFOST_RFE_MASK            (((1U << SPI_FIFOST_RFE_WIDTH) - 1U) << SPI_FIFOST_RFE_SHIFT)
#define SPI_FIFOST_RFE_NOT_EMPTY       (0U << SPI_FIFOST_RFE_SHIFT)
#define SPI_FIFOST_RFE_EMPTY           (1U << SPI_FIFOST_RFE_SHIFT)

#define SPI_FIFOST_RFF_SHIFT           1
#define SPI_FIFOST_RFF_WIDTH           1
#define SPI_FIFOST_RFF_MASK            (((1U << SPI_FIFOST_RFF_WIDTH) - 1U) << SPI_FIFOST_RFF_SHIFT)
#define SPI_FIFOST_RFF_NOT_FULL        (0U << SPI_FIFOST_RFF_SHIFT)
#define SPI_FIFOST_RFF_FULL            (1U << SPI_FIFOST_RFF_SHIFT)

#define SPI_FIFOST_RFHF_SHIFT          2
#define SPI_FIFOST_RFHF_WIDTH          1
#define SPI_FIFOST_RFHF_MASK           (((1U << SPI_FIFOST_RFHF_WIDTH) - 1U) << SPI_FIFOST_RFHF_SHIFT)
#define SPI_FIFOST_RFHF_NOT_HALF_FULL  (0U << SPI_FIFOST_RFHF_SHIFT)
#define SPI_FIFOST_RFHF_HALF_FULL      (1U << SPI_FIFOST_RFHF_SHIFT)

#define SPI_FIFOST_TFE_SHIFT           3
#define SPI_FIFOST_TFE_WIDTH           1
#define SPI_FIFOST_TFE_MASK            (((1U << SPI_FIFOST_TFE_WIDTH) - 1U) << SPI_FIFOST_TFE_SHIFT)
#define SPI_FIFOST_TFE_NOT_EMPTY       (0U << SPI_FIFOST_TFE_SHIFT)
#define SPI_FIFOST_TFE_EMPTY           (1U << SPI_FIFOST_TFE_SHIFT)

#define SPI_FIFOST_TFF_SHIFT           4
#define SPI_FIFOST_TFF_WIDTH           1
#define SPI_FIFOST_TFF_MASK            (((1U << SPI_FIFOST_TFF_WIDTH) - 1U) << SPI_FIFOST_TFF_SHIFT)
#define SPI_FIFOST_TFF_NOT_FULL        (0U << SPI_FIFOST_TFF_SHIFT)
#define SPI_FIFOST_TFF_FULL            (1U << SPI_FIFOST_TFF_SHIFT)

#define SPI_FIFOST_TFHF_SHIFT          5
#define SPI_FIFOST_TFHF_WIDTH          1
#define SPI_FIFOST_TFHF_MASK           (((1U << SPI_FIFOST_TFHF_WIDTH) - 1U) << SPI_FIFOST_TFHF_SHIFT)
#define SPI_FIFOST_TFHF_NOT_HALF_FULL  (0U << SPI_FIFOST_TFHF_SHIFT)
#define SPI_FIFOST_TFHF_HALF_FULL      (1U << SPI_FIFOST_TFHF_SHIFT)

#define SPI_FIFOST_RF_LEVEL_SHIFT      6
#define SPI_FIFOST_RF_LEVEL_WIDTH      3
#define SPI_FIFOST_RF_LEVEL_MASK       (((1U << SPI_FIFOST_RF_LEVEL_WIDTH) - 1U) << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_0_BYTE     (0U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_1_BYTE     (1U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_2_BYTE     (2U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_3_BYTE     (3U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_4_BYTE     (4U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_5_BYTE     (5U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_6_BYTE     (6U << SPI_FIFOST_RF_LEVEL_SHIFT)
#define SPI_FIFOST_RF_LEVEL_7_BYTE     (7U << SPI_FIFOST_RF_LEVEL_SHIFT)

#define SPI_FIFOST_TF_LEVEL_SHIFT      9
#define SPI_FIFOST_TF_LEVEL_WIDTH      3
#define SPI_FIFOST_TF_LEVEL_MASK       (((1U << SPI_FIFOST_TF_LEVEL_WIDTH) - 1U) << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_0_BYTE     (0U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_1_BYTE     (1U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_2_BYTE     (2U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_3_BYTE     (3U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_4_BYTE     (4U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_5_BYTE     (5U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_6_BYTE     (6U << SPI_FIFOST_TF_LEVEL_SHIFT)
#define SPI_FIFOST_TF_LEVEL_7_BYTE     (7U << SPI_FIFOST_TF_LEVEL_SHIFT)


#endif

