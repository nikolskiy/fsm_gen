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

system_config_type system_config = {
  .parameters = {
    [MOTOR_SPINUP_TIME] = {.val=3000, .min=500, .max=7000},
    [MOTOR_CURRENT_MIN] = {.val=1000, .min=50, .max=5000},
    [MOTOR_CURRENT_MAX] = {.val=2000, .min=50, .max=5000},
    [MOTOR_CURRENT_ASSYMETRY] = {.val=10, .min=0, .max=50},
    [MOTOR_RPM_MIN] = {.val=800, .min=50, .max=5000},
    [MOTOR_RPM_MAX] = {.val=2000, .min=50, .max=5000}
  }
};

void __process_event(void* e)
{
  (*((void(**)(void*))e))(e);
}

void __free_event(void* e)
{
  if (e != ((void*)0)) {
    ((void**)e)[0] = (void*)0;
  }
}
