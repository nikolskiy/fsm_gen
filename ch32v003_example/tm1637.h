#ifndef ___tm1637____header__
#define ___tm1637____header__
#include "config.h"
#include "stdio.h"

void tm1637_init();
void tm1637_deinit();
void tm1637_clock ();
void tm1637_send (uint8_t e_data, void* e_callback);
void tm1637_start (void* e_callback);
void tm1637_stop (void* e_callback);
void* tm1637_clock_delayed ();
#define tm1637_process_delayed_event(E) (*((void(**)(void*))E))(E)
void tm1637_clock_enqueue ();
void tm1637_process_queue();


#endif // ___tm1637____header__
