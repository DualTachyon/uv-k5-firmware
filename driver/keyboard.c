//
// Created by Manuel Jedinger on 8/10/23.
//

#include "keyboard.h"
#include "system.h"

#define KEYBOARD_LVLWAIT_US 10

void Keyboard_init(){
    GPIO_SetBit(&GPIOA->DIR, 10);
    GPIO_SetBit(&GPIOA->DIR, 11);
    GPIO_SetBit(&GPIOA->DIR, 12);
    GPIO_SetBit(&GPIOA->DIR, 13);

    GPIO_ClearBit(&GPIOA->DIR, 3);
    GPIO_ClearBit(&GPIOA->DIR, 4);
    GPIO_ClearBit(&GPIOA->DIR, 5);
    GPIO_ClearBit(&GPIOA->DIR, 6);
}

unsigned int PollKeyboard(){

    GPIO_SetBit(&GPIOA->DATA, 10);
    GPIO_SetBit(&GPIOA->DATA, 11);
    GPIO_SetBit(&GPIOA->DATA, 12);
    GPIO_SetBit(&GPIOA->DATA, 13);
    SYSTEM_DelayUs(KEYBOARD_LVLWAIT_US);

    //keys connected to gnd
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_FN1;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_FN2;
    if(!GPIO_CheckBit(&GPIOC->DATA, 5))
        return KEY_PPT;


    // first row
    GPIO_ClearBit(&GPIOA->DATA, 10);
    SYSTEM_DelayUs(KEYBOARD_LVLWAIT_US);
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_M;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_1;
    if(!GPIO_CheckBit(&GPIOA->DATA, 5))
        return KEY_4;
    if(!GPIO_CheckBit(&GPIOA->DATA, 6))
        return KEY_7;
    GPIO_SetBit(&GPIOA->DATA, 10);


    // second row
    GPIO_ClearBit(&GPIOA->DATA, 11);
    SYSTEM_DelayUs(KEYBOARD_LVLWAIT_US);
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_UP;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_2;
    if(!GPIO_CheckBit(&GPIOA->DATA, 5))
        return KEY_5;
    if(!GPIO_CheckBit(&GPIOA->DATA, 6))
        return KEY_8;
    GPIO_SetBit(&GPIOA->DATA, 11);

    // third row
    GPIO_ClearBit(&GPIOA->DATA, 12);
    SYSTEM_DelayUs(KEYBOARD_LVLWAIT_US);
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_DOWN;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_3;
    if(!GPIO_CheckBit(&GPIOA->DATA, 5))
        return KEY_6;
    if(!GPIO_CheckBit(&GPIOA->DATA, 6))
        return KEY_9;
    GPIO_SetBit(&GPIOA->DATA, 12);

    // fourth row
    GPIO_ClearBit(&GPIOA->DATA, 13);
    SYSTEM_DelayUs(KEYBOARD_LVLWAIT_US);
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_EXIT;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_STAR;
    if(!GPIO_CheckBit(&GPIOA->DATA, 5))
        return KEY_0;
    if(!GPIO_CheckBit(&GPIOA->DATA, 6))
        return KEY_F;
    GPIO_SetBit(&GPIOA->DATA, 13);

    return KEY_NOKEY;
}