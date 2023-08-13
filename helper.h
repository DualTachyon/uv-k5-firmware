#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

extern uint8_t gKeyReading0;
extern uint8_t gKeyReading1;
extern uint8_t g_2000042A;

uint8_t HELPER_GetKey(void);
void HELPER_CheckBootKey(uint8_t KeyType);

#endif

