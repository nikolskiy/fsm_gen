#include "systick.h"
#include "ch32v003fun.h"

/* some bit definitions for systick regs */
#define SYSTICK_SR_CNTIF (1<<0)
#define SYSTICK_CTLR_STE (1<<0)
#define SYSTICK_CTLR_STIE (1<<1)
#define SYSTICK_CTLR_STCLK (1<<2)
#define SYSTICK_CTLR_STRE (1<<3)
#define SYSTICK_CTLR_SWIE (1<<31)

void systick_init(void)
{
    SysTick->CTLR = 0;
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->CMP = (FUNCONF_SYSTEM_CORE_CLOCK/SYSTICK_FREQ)-1;
    SysTick->CNT = 0;
    SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STCLK;
}

volatile static uint32_t button_press_syscnt = 0;

void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
    SysTick->CMP += (FUNCONF_SYSTEM_CORE_CLOCK/10000);
    if (SysTick->CMP - SysTick->CNT > (FUNCONF_SYSTEM_CORE_CLOCK/SYSTICK_FREQ)) {
        systick_clock_missed();
        SysTick->CMP = SysTick->CNT + (FUNCONF_SYSTEM_CORE_CLOCK/SYSTICK_FREQ);
    }
    systick_clock();
    SysTick->SR = 0;
}
