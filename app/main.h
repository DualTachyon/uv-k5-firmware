#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/keyboard.h"

void MAIN_Key_DIGITS(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_EXIT(bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_MENU(bool bKeyPressed, bool bKeyHeld);
void MAIN_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction);

#endif

