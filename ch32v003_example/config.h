#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <ch32v003fun.h>
#include <stdint.h>
#include "system_state.h"

uint32_t ___disable_interrupts();
void ___restore_interrupts();

void __process_event(void*);
void __free_event(void*);

typedef struct parameter_val_struct {
    uint16_t val;
    uint16_t min;
    uint16_t max;
} parameter_val_type;

typedef enum parameters_e {
    MOTOR_SPINUP_TIME = 0,
    MOTOR_CURRENT_MIN,
    MOTOR_CURRENT_MAX,
    MOTOR_CURRENT_ASSYMETRY,
    MOTOR_RPM_MIN,
    MOTOR_RPM_MAX,
    __LAST_PARAMETER = MOTOR_RPM_MAX,
    __NUM_PARAMETERS
} parameter_type;

typedef struct system_config_struct {
    parameter_val_type parameters[__NUM_PARAMETERS];
} system_config_type;

extern system_config_type system_config;

#endif // __CONFIG_H__