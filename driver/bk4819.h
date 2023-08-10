#ifndef DRIVER_BK4819_h
#define DRIVER_BK4819_h

#include <stdint.h>
#include "driver/bk4819-regs.h"

void BK4819_Init(void);
void BK4819_WriteRegister(BK4819_REGISTER_t Register, uint16_t Data);
void BK4819_WriteU8(uint8_t Data);
void BK4819_WriteU16(uint16_t Data);

void BK4819_SetAGC(uint8_t Value);

#endif

