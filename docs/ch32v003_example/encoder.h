#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <stdint.h>

void exti_init();

extern void exti_encoder_rotate(int dir /*0/1*/, uint32_t /*syscntr time of edge*/);
extern void exti_button(int, uint32_t);

#endif // __ENCODER_H__
