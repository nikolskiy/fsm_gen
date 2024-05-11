#include "config.h"

uint32_t ___disable_interrupts()
{
    uint32_t state = NVIC_get_enabled_IRQs();
    NVIC_clear_all_IRQs_except(0);
    return state;
}

void ___restore_interrupts(uint32_t state)
{
    NVIC_restore_IRQs(state);
}