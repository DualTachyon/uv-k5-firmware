#include "bsp/dp32g030/gpio.h"
#include "bk1080.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/system.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static const uint16_t BK1080_RegisterTable[] = {
	0x0008, 0x1080, 0x0201, 0x0000,
	0x40C0, 0x0A1F, 0x002E, 0x02FF,
	0x5B11, 0x0000, 0x411E, 0x0000,
	0xCE00, 0x0000, 0x0000, 0x1000,
	0x3197, 0x0000, 0x13FF, 0x9852,
	0x0000, 0x0000, 0x0008, 0x0000,
	0x51E1, 0xA8BC, 0x2645, 0x00E4,
	0x1CD8, 0x3A50, 0xEAE0, 0x3000,
	0x0200, 0x0000,
};

static bool gIsInitBK1080;

void BK1080_Init(uint16_t Channel, bool bDoScan)
{
	uint8_t i;

	if (bDoScan) {
		GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BK1080);

		if (!gIsInitBK1080) {
			for (i = 0; i < ARRAY_SIZE(BK1080_RegisterTable); i++) {
				BK1080_WriteRegister(i, BK1080_RegisterTable[i]);
			}
			SYSTEM_DelayMs(250);
			BK1080_WriteRegister(BK1080_REG_25_INTERNAL, 0xA83C);
			BK1080_WriteRegister(BK1080_REG_25_INTERNAL, 0xA8BC);
			SYSTEM_DelayMs(60);
			gIsInitBK1080 = true;
		} else {
			BK1080_WriteRegister(BK1080_REG_02_POWER_CONFIGURATION, 0x0201);
		}
		BK1080_WriteRegister(BK1080_REG_05_SYSTEM_CONFIGURATION2, 0x0A5F);
		BK1080_WriteRegister(BK1080_REG_03_CHANNEL, Channel - 760);
		SYSTEM_DelayMs(10);
		BK1080_WriteRegister(BK1080_REG_03_CHANNEL, (Channel - 760) | 0x8000);
	} else {
		BK1080_WriteRegister(BK1080_REG_02_POWER_CONFIGURATION, 0x0241);
		GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BK1080);
	}
}

void BK1080_WriteRegister(BK1080_REGISTER_t Register, uint16_t Value)
{
	I2C_Start();
	I2C_Write(0x80);
	I2C_Write(Register << 1);
	I2C_WriteBuffer(&Value, sizeof(Value));
	I2C_Stop();
}

