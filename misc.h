#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdint.h>

extern const uint32_t *gUpperLimitFrequencyBandTable;
extern const uint32_t *gLowerLimitFrequencyBandTable;

extern bool gSetting_350TX;
extern bool gSetting_KILLED;
extern bool gSetting_200TX;
extern bool gSetting_500TX;
extern bool gSetting_350EN;
extern bool gSetting_ScrambleEnable;
extern uint8_t gSetting_F_LOCK;

extern uint8_t gCustomPasswordKey[16];

extern bool bIsCheckExistingPassword;

extern uint8_t gEEPROM_1EC0_0[8];
extern uint8_t gEEPROM_1EC0_1[8];
extern uint8_t gEEPROM_1EC0_2[8];
extern uint8_t gEEPROM_1EC0_3[8];

extern uint8_t gEEPROM_1EC8_0[8];
extern uint8_t gEEPROM_1EC8_1[8];
extern uint8_t gEEPROM_1EC8_2[8];

extern uint16_t gBatteryCalibration[6];
extern uint16_t gADC_CH4_BootValue;
extern uint16_t gADC_CH4[4];
extern uint16_t gADC_CH9;

extern uint16_t gEEPROM_1F8A;
extern uint16_t gEEPROM_1F8C;
extern uint8_t gEEPROM_1F8E;

extern uint8_t gMR_ChannelParameters[207];

#endif

