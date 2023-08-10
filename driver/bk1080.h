#ifndef DRIVER_BK1080_H
#define DRIVER_BK1080_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/bk1080-regs.h"

void BK1080_Init(uint16_t Frequency, bool bDoScan);
void BK1080_WriteRegister(BK1080_REGISTER_t Register, uint16_t Value);

#endif

