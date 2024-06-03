#ifndef ___timer____header__
#define ___timer____header__
#include "config.h"

void timer_init();
void timer_deinit();
void timer_clock ();
void timer_set (uint32_t timeout, void* delayed_event);
void* timer_clock_delayed ();
#define timer_process_delayed_event(E) (*((void(**)(void*))E))(E)
void timer_clock_enqueue ();
void timer_process_queue();


#endif // ___timer____header__
