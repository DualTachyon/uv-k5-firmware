#ifndef FM_H
#define FM_H

#include <stdbool.h>
#include <stdint.h>

#define FM_CHANNEL_UP	0x01
#define FM_CHANNEL_DOWN	0xFF

extern uint16_t gFM_Channels[20];

bool FM_CheckValidChannel(uint8_t Channel);
uint8_t FM_FindNextChannel(uint8_t Channel, uint8_t Direction);
int FM_ConfigureChannelState(void);

#endif

