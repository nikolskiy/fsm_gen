#include "tm1637.h"
#define process_event(E) (*((void(**)(void*))E))(E)
#define i2c_c(d) funDigitalWrite(PA2, d ? FUN_HIGH:FUN_LOW)
#define i2c_d(d) funDigitalWrite(PA3, d ? FUN_HIGH:FUN_LOW)

typedef struct ___tm1637____delayed_clock_params_type {
  volatile void(*___tm1637____delayed_event_handler)(void *);
} ___tm1637____delayed_clock_params_type;

typedef struct ___tm1637____delayed_event_queue_type {
  void* ___event_handlers[3];
  volatile int ___head;
  volatile int ___tail;
} ___tm1637____delayed_event_queue_type;
typedef struct ___tm1637____state_variables {
  uint16_t data;
  void* callback;
  ___tm1637____delayed_clock_params_type ___tm1637____delayed_clock_params[2];
  ___tm1637____delayed_event_queue_type ___tm1637____delayed_event_queue;
  unsigned char ___tm1637____state_num;
} ___tm1637____state_variables;
static ___tm1637____state_variables ___tm1637____state = { 
  .data=0,
  .callback=((void*)0),
  .___tm1637____state_num=0,
  .___tm1637____delayed_clock_params={{.___tm1637____delayed_event_handler=((void(*)(void*))0)}, {.___tm1637____delayed_event_handler=((void(*)(void*))0)}},
  .___tm1637____delayed_event_queue={.___event_handlers={ (void *)0, (void *)0, (void *)0 }, .___head=0, .___tail=0}
};
#define data ___tm1637____state.data
#define callback ___tm1637____state.callback
#define delay_clock(...) (tm1637_clock_delayed (__VA_ARGS__))
#define enqueue_clock(...) tm1637_clock_enqueue (__VA_ARGS__)
void tm1637_init()
{
  funGpioInitAll();
  funPinMode(PA2, GPIO_CFGLR_OUT_10Mhz_OD);
  funPinMode(PA3, GPIO_CFGLR_OUT_10Mhz_OD);
}
void tm1637_deinit()
{
}
static void ___tm1637____transition_actions_0 (void* e_callback)
{
  callback = e_callback;
  data = 2;
  i2c_c(1);
  i2c_d(1);
}
static void ___tm1637____transition_actions_1 ()
{
  --data;
  i2c_d(0);
}
static void ___tm1637____transition_actions_2 ()
{
  --data;
  i2c_c(0);
}
static void ___tm1637____transition_actions_3 ()
{
  process_event(callback);
}
static void ___tm1637____transition_actions_4 (void* e_callback)
{
  callback = e_callback;
  data = 2;
  i2c_c(0);
  i2c_d(0);
}
static void ___tm1637____transition_actions_5 ()
{
  --data;
  i2c_c(0);
  i2c_d(1);
}
static void ___tm1637____transition_actions_6 ()
{
  --data;
  i2c_c(1);
  i2c_d(1);
}
static void ___tm1637____transition_actions_7 ()
{
  process_event(callback);
}
static void ___tm1637____transition_actions_8 (uint8_t e_data, void* e_callback)
{
  data = 0x100 | e_data;
  callback = e_callback;
}
static void ___tm1637____transition_actions_9 ()
{
  i2c_d(data&0x1);
  i2c_c(0);
  data >>= 1;
}
static void ___tm1637____transition_actions_10 ()
{
  i2c_c(1);
}
static void ___tm1637____transition_actions_11 ()
{
  process_event(callback);
}
void tm1637_clock ()
{
  #define enqueue_self() tm1637_clock_enqueue ()
  register unsigned char ___tm1637____state_num = ___tm1637____state.___tm1637____state_num;
  // transition 1
  if (
    (
      ___tm1637____state_num == 1
    ) &&
    (data == 2)
  ) {
    ___tm1637____transition_actions_1 ();
    return;
  }
  // transition 2
  if (
    (
      ___tm1637____state_num == 1
    ) &&
    (data == 1)
  ) {
    ___tm1637____transition_actions_2 ();
    return;
  }
  // transition 3
  if (
    (
      ___tm1637____state_num == 1
    ) &&
    (data == 0)
  ) {
    ___tm1637____transition_actions_3 ();
    ___tm1637____state.___tm1637____state_num = 0;
    return;
  }
  // transition 5
  if (
    (
      ___tm1637____state_num == 2
    ) &&
    (data == 2)
  ) {
    ___tm1637____transition_actions_5 ();
    return;
  }
  // transition 6
  if (
    (
      ___tm1637____state_num == 2
    ) &&
    (data == 1)
  ) {
    ___tm1637____transition_actions_6 ();
    return;
  }
  // transition 7
  if (
    (
      ___tm1637____state_num == 2
    ) &&
    (data == 0)
  ) {
    ___tm1637____transition_actions_7 ();
    ___tm1637____state.___tm1637____state_num = 0;
    return;
  }
  // transition 9
  if (
    (
      ___tm1637____state_num == 3
    ) &&
    (data > 0)
  ) {
    ___tm1637____transition_actions_9 ();
    ___tm1637____state.___tm1637____state_num = 4;
    return;
  }
  // transition 10
  if (
    (
      ___tm1637____state_num == 4
    )
  ) {
    ___tm1637____transition_actions_10 ();
    ___tm1637____state.___tm1637____state_num = 3;
    return;
  }
  // transition 11
  if (
    (
      ___tm1637____state_num == 3
    ) &&
    (data == 0)
  ) {
    ___tm1637____transition_actions_11 ();
    ___tm1637____state.___tm1637____state_num = 0;
    return;
  }
  // transition 12
  // field 'from' contains all states, so no check for state is needed
  // Warning: meaningless self loop transition withot 'do'
  #undef enqueue_self // for clock
}
void tm1637_send (uint8_t e_data, void* e_callback)
{
  
  register unsigned char ___tm1637____state_num = ___tm1637____state.___tm1637____state_num;
  // transition 8
  if (
    (
      ___tm1637____state_num == 0
    )
  ) {
    ___tm1637____transition_actions_8 (e_data, e_callback);
    ___tm1637____state.___tm1637____state_num = 3;
    return;
  }
  // warning: ignoring unhandled event
}
void tm1637_start (void* e_callback)
{
  
  register unsigned char ___tm1637____state_num = ___tm1637____state.___tm1637____state_num;
  // transition 0
  if (
    (
      ___tm1637____state_num == 0
    )
  ) {
    ___tm1637____transition_actions_0 (e_callback);
    ___tm1637____state.___tm1637____state_num = 1;
    return;
  }
  // warning: ignoring unhandled event
}
void tm1637_stop (void* e_callback)
{
  
  register unsigned char ___tm1637____state_num = ___tm1637____state.___tm1637____state_num;
  // transition 4
  if (
    (
      ___tm1637____state_num == 0
    )
  ) {
    ___tm1637____transition_actions_4 (e_callback);
    ___tm1637____state.___tm1637____state_num = 2;
    return;
  }
  // warning: ignoring unhandled event
}
static void tm1637_clock_process_delayed (void* ___tm1637____delayed_event_params)
{
  tm1637_clock (
  );
  ((___tm1637____delayed_clock_params_type*)___tm1637____delayed_event_params)->___tm1637____delayed_event_handler = (void(*)(void*))0;
}
void* tm1637_clock_delayed ()
{
  uint32_t ___tm1637____interrupts_state = ___disable_interrupts();
  for(int idx=0; idx < 2; ++idx) {
    if (((void*)___tm1637____state.___tm1637____delayed_clock_params[idx].___tm1637____delayed_event_handler) == (void*)0) {
      ;
      ___tm1637____state.___tm1637____delayed_clock_params[idx].___tm1637____delayed_event_handler=tm1637_clock_process_delayed;
      ___restore_interrupts(___tm1637____interrupts_state);
      return (void*)(&((___tm1637____state.___tm1637____delayed_clock_params)[idx]));
    }
  }
  ___restore_interrupts(___tm1637____interrupts_state);
  return (void *)0;
}
static int ___tm1637____put_delayed_event_in_queue(void *de)
{
  if (de == (void*)0) return 0;
  uint32_t ___tm1637____interrupts_state = ___disable_interrupts();
  int tail = ___tm1637____state.___tm1637____delayed_event_queue.___tail;
  int old_tail = tail;
  ++tail;
  if(tail >= 3) tail = 0;
  if (tail != ___tm1637____state.___tm1637____delayed_event_queue.___head) {
    ___tm1637____state.___tm1637____delayed_event_queue.___event_handlers[old_tail] = de;
    ___tm1637____state.___tm1637____delayed_event_queue.___tail = tail;
    ___restore_interrupts(___tm1637____interrupts_state);
    return 1;
  }
  ___restore_interrupts(___tm1637____interrupts_state);
  return 0;
}
void tm1637_clock_enqueue ()
{
  if (___tm1637____put_delayed_event_in_queue(tm1637_clock_delayed ())) return;
  // warning: ignoring unhandled event
}
void tm1637_process_queue()
{
  if (___tm1637____state.___tm1637____delayed_event_queue.___head != ___tm1637____state.___tm1637____delayed_event_queue.___tail) {
    int head = ___tm1637____state.___tm1637____delayed_event_queue.___head;
    int tail = ___tm1637____state.___tm1637____delayed_event_queue.___tail;
    while(head != tail) {
      tm1637_process_delayed_event(___tm1637____state.___tm1637____delayed_event_queue.___event_handlers[head]);
      ++head;
      if(head >= 3) head = 0;
    }
    ___tm1637____state.___tm1637____delayed_event_queue.___head = head;
  }
}
#undef data
#undef callback


