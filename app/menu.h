#ifndef APP_MENU_H
#define APP_MENU_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/keyboard.h"

int MENU_GetLimits(uint8_t Cursor, uint8_t *pMin, uint8_t *pMax);
void MENU_AcceptSetting(void);
void MENU_SelectNextDCS(void);
void MENU_ShowCurrentSetting(void);

void MENU_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void MENU_Key_EXIT(bool bKeyPressed, bool bKeyHeld);
void MENU_Key_MENU(bool bKeyPressed, bool bKeyHeld);
void MENU_Key_STAR(bool bKeyPressed, bool bKeyHeld);
void MENU_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction);

#endif

