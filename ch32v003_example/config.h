#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <ch32v003fun.h>
#include "system_state.h"

uint32_t ___disable_interrupts();
void ___restore_interrupts();

#endif // __CONFIG_H__