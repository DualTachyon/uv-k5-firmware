//
// Created by Manuel Jedinger on 8/10/23.
//

#include "keyboard.h"

#define SLEEPTIME 100

void sleep(uint32_t time){
    for (uint32_t i = 0; i < time; ++i) {
        __asm__("nop");
    }
}

unsigned int PollKeyboard(){



    GPIO_SetBit(&GPIOA->DATA, 10);
    GPIO_SetBit(&GPIOA->DATA, 11);
    GPIO_SetBit(&GPIOA->DATA, 12);
    GPIO_SetBit(&GPIOA->DATA, 13);

    sleep(SLEEPTIME);

    //keys connected to gnd
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_FN1;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_FN2;
    if(!GPIO_CheckBit(&GPIOC->DATA, 5))
        return KEY_PPT;


    // first row
    GPIO_ClearBit(&GPIOA->DATA, 10);
    sleep(SLEEPTIME);
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
    sleep(SLEEPTIME);
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
    sleep(SLEEPTIME);
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
    sleep(SLEEPTIME);
    if(!GPIO_CheckBit(&GPIOA->DATA, 3))
        return KEY_EXIT;
    if(!GPIO_CheckBit(&GPIOA->DATA, 4))
        return KEY_STAR;
    if(!GPIO_CheckBit(&GPIOA->DATA, 5))
        return KEY_0;
    if(!GPIO_CheckBit(&GPIOA->DATA, 6))
        return KEY_F;
    GPIO_SetBit(&GPIOA->DATA, 13);

    return 24;
}