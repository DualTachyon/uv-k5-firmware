#ifndef DCS_H
#define DCS_H

#include <stdint.h>

enum {
	CODE_TYPE_OFF = 0x00U,
	CODE_TYPE_CONTINUOUS_TONE = 0x01U,
	CODE_TYPE_DIGITAL = 0x02U,
	CODE_TYPE_REVERSE_DIGITAL = 0x03U,
};

extern const uint16_t CTCSS_Options[50];
extern const uint16_t DCS_Options[104];

uint32_t DCS_GetGolayCodeWord(uint8_t CodeType, uint8_t Option);

#endif

