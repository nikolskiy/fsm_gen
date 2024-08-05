#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#define SYSTICK_FREQ 10000

void systick_init();

extern void systick_clock_missed();
extern void systick_clock();

#endif // __SYSTICK_H__
