#ifndef BATTERY_H
#define BATTERY_H

#include <stdbool.h>
#include <stdint.h>

extern uint16_t gBatteryCalibration[6];
extern uint16_t gBatteryBootVoltage;
extern uint16_t gBatteryCurrent;
extern uint16_t gBatteryVoltages[4];
extern uint16_t gBatteryVoltageAverage;

extern uint8_t gBatteryDisplayLevel;

extern bool gChargingWithTypeC;
extern bool gMaybeLowBatteryWarning;

void BATTERY_GetReadings(bool bDisplayBatteryLevel);

#endif

