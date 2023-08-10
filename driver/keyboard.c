/* Copyright 2023 Manuel Jedinger
 * https://github.com/manujedi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */
#include "keyboard.h"

#define KEYBOARD_LVLWAIT_US 10

void Keyboard_init() {
    GPIO_SetBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_LINE1);
    GPIO_SetBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_LINE2);
    GPIO_SetBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_LINE3);
    GPIO_SetBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_LINE4);

    GPIO_ClearBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_ROW1);
    GPIO_ClearBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_ROW2);
    GPIO_ClearBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_ROW3);
    GPIO_ClearBit(&GPIOA->DIR, GPIOA_PIN_KEYBOARD_ROW4);
}

unsigned int PollKeyboardInternally() {

    //keys connected to gnd
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW1))
        return KEY_FN1;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW2))
        return KEY_FN2;


    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE1);
    SYSTICK_DelayUs(KEYBOARD_LVLWAIT_US);
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW1))
        return KEY_M;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW2))
        return KEY_1;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW3))
        return KEY_4;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW4))
        return KEY_7;
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE1);


    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE2);
    SYSTICK_DelayUs(KEYBOARD_LVLWAIT_US);
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW1))
        return KEY_UP;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW2))
        return KEY_2;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW3))
        return KEY_5;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW4))
        return KEY_8;
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE2);


    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE3);
    SYSTICK_DelayUs(KEYBOARD_LVLWAIT_US);
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW1))
        return KEY_DOWN;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW2))
        return KEY_3;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW3))
        return KEY_6;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW4))
        return KEY_9;
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE3);


    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE4);
    SYSTICK_DelayUs(KEYBOARD_LVLWAIT_US);
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW1))
        return KEY_EXIT;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW2))
        return KEY_STAR;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW3))
        return KEY_0;
    if (!GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_ROW4))
        return KEY_F;
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE4);

    return KEY_NOKEY;
}

unsigned int PollKeyboard() {
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE1);
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE2);
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE3);
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_LINE4);

    SYSTICK_DelayUs(KEYBOARD_LVLWAIT_US);

    int key = PollKeyboardInternally();

    GPIO_SetBit(&GPIOA->DATA,GPIOA_PIN_KEYBOARD_LINE1);
    GPIO_SetBit(&GPIOA->DATA,GPIOA_PIN_KEYBOARD_LINE2);
    GPIO_ClearBit(&GPIOA->DATA,GPIOA_PIN_KEYBOARD_LINE3);
    GPIO_SetBit(&GPIOA->DATA,GPIOA_PIN_KEYBOARD_LINE4);

    SYSTICK_DelayUs(KEYBOARD_LVLWAIT_US);

    return key;
}

