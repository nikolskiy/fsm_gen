#include "encoder.h"
#include <ch32v003fun.h>

// Encoder and button

void exti_init(void)
{
    funGpioInitD();
    funGpioInitC();

    // D2, D3 - encoder
    funPinMode( PD3, GPIO_CFGLR_IN_PUPD ); // PullUP/PullDown

    DYN_GPIO_MOD(GPIOD, OUTDR, ODR3, 1); // PullUp

    AFIO->EXTICR = AFIO_EXTICR_EXTI3_PD; // SELECT EXTI function
    EXTI->INTENR = EXTI_INTENR_MR3; // ENABLE EXTI3
    EXTI->RTENR = EXTI_RTENR_TR3; // falling edge

    // Button C5

    funPinMode( PC5, GPIO_CFGLR_IN_PUPD );
    DYN_GPIO_MOD(GPIOC, OUTDR, ODR5, 1);
    AFIO->EXTICR |= AFIO_EXTICR_EXTI5_PC;
    EXTI->INTENR |= EXTI_INTENR_MR5;
    EXTI->RTENR |= EXTI_RTENR_TR5;
    EXTI->FTENR |= EXTI_FTENR_TR5;

    NVIC_EnableIRQ( EXTI7_0_IRQn ); // enable interrupt
}

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
    if (EXTI->INTFR & EXTI_Line3) {
        exti_encoder_rotate(funDigitalRead(PD2), SysTick->CNT);
    }
    if (EXTI->INTFR & EXTI_Line5) {
        exti_button(!funDigitalRead(PC5), SysTick->CNT);
    }
    EXTI->INTFR = EXTI_Line3 | EXTI_Line5;
}
