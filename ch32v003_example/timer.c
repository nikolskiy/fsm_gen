#include "timer.h"
#define process_event(E) (*((void(**)(void*))E))(E)
#define MAX_TIMERS 8
typedef struct timer_struct {
  void *event;
  uint32_t time;
} timer_type[MAX_TIMERS];

typedef struct ___timer____delayed_clock_params_type {
  volatile void(*___timer____delayed_event_handler)(void *);
} ___timer____delayed_clock_params_type;

typedef struct ___timer____delayed_event_queue_type {
  void* ___event_handlers[5];
  volatile int ___head;
  volatile int ___tail;
} ___timer____delayed_event_queue_type;
typedef struct ___timer____state_variables {
  timer_type timers;
  ___timer____delayed_clock_params_type ___timer____delayed_clock_params[4];
  ___timer____delayed_event_queue_type ___timer____delayed_event_queue;
  unsigned char ___timer____state_num;
} ___timer____state_variables;
static ___timer____state_variables ___timer____state = { 
  .timers={ [0 ... MAX_TIMERS-1]={.time=0}},
  .___timer____state_num=0,
  .___timer____delayed_clock_params={{.___timer____delayed_event_handler=((void(*)(void*))0)}, {.___timer____delayed_event_handler=((void(*)(void*))0)}, {.___timer____delayed_event_handler=((void(*)(void*))0)}, {.___timer____delayed_event_handler=((void(*)(void*))0)}},
  .___timer____delayed_event_queue={.___event_handlers={ (void *)0, (void *)0, (void *)0, (void *)0, (void *)0 }, .___head=0, .___tail=0}
};
#define timers ___timer____state.timers
#define delay_clock(...) (timer_clock_delayed (__VA_ARGS__))
#define enqueue_clock(...) timer_clock_enqueue (__VA_ARGS__)
void timer_init()
{
}
void timer_deinit()
{
}
static void ___timer____transition_actions_0 ()
{
  for(int i=0; i<MAX_TIMERS; ++i) {
    if (timers[i].time>0) {
      --timers[i].time;
      if (timers[i].time==0) {
        process_event(timers[i].event);
      }
    }
  }
}
static void ___timer____transition_actions_1 (uint32_t timeout, void* delayed_event)
{
  for(int i=0; i<MAX_TIMERS; ++i) {
    if (timers[i].time==0){
      timers[i].time = timeout;
      timers[i].event = delayed_event;
      return;
    }
  }
}
void timer_clock ()
{
  #define enqueue_self() timer_clock_enqueue ()
  register unsigned char ___timer____state_num = ___timer____state.___timer____state_num;
  // transition 0
  // field 'from' contains all states, so no check for state is needed
  ___timer____transition_actions_0 ();
  return;
  // NB: Unconditial return here, so next transitions are not procesed.
  #undef enqueue_self // for clock
}
void timer_set (uint32_t timeout, void* delayed_event)
{
  
  register unsigned char ___timer____state_num = ___timer____state.___timer____state_num;
  // transition 1
  // field 'from' contains all states, so no check for state is needed
  ___timer____transition_actions_1 (timeout, delayed_event);
  return;
}
static void timer_clock_process_delayed (void* ___timer____delayed_event_params)
{
  timer_clock (
  );
  ((___timer____delayed_clock_params_type*)___timer____delayed_event_params)->___timer____delayed_event_handler = (void(*)(void*))0;
}
void* timer_clock_delayed ()
{
  uint32_t ___timer____interrupts_state = ___disable_interrupts();
  for(int idx=0; idx < 4; ++idx) {
    if (((void*)___timer____state.___timer____delayed_clock_params[idx].___timer____delayed_event_handler) == (void*)0) {
      ;
      ___timer____state.___timer____delayed_clock_params[idx].___timer____delayed_event_handler=timer_clock_process_delayed;
      ___restore_interrupts(___timer____interrupts_state);
      return (void*)(&((___timer____state.___timer____delayed_clock_params)[idx]));
    }
  }
  ___restore_interrupts(___timer____interrupts_state);
  return (void *)0;
}
static int ___timer____put_delayed_event_in_queue(void *de)
{
  if (de == (void*)0) return 0;
  uint32_t ___timer____interrupts_state = ___disable_interrupts();
  int tail = ___timer____state.___timer____delayed_event_queue.___tail;
  int old_tail = tail;
  ++tail;
  if(tail >= 5) tail = 0;
  if (tail != ___timer____state.___timer____delayed_event_queue.___head) {
    ___timer____state.___timer____delayed_event_queue.___event_handlers[old_tail] = de;
    ___timer____state.___timer____delayed_event_queue.___tail = tail;
    ___restore_interrupts(___timer____interrupts_state);
    return 1;
  }
  ___restore_interrupts(___timer____interrupts_state);
  return 0;
}
void timer_clock_enqueue ()
{
  if (___timer____put_delayed_event_in_queue(timer_clock_delayed ())) return;
  // warning: ignoring unhandled event
}
void timer_process_queue()
{
  if (___timer____state.___timer____delayed_event_queue.___head != ___timer____state.___timer____delayed_event_queue.___tail) {
    int head = ___timer____state.___timer____delayed_event_queue.___head;
    int tail = ___timer____state.___timer____delayed_event_queue.___tail;
    while(head != tail) {
      timer_process_delayed_event(___timer____state.___timer____delayed_event_queue.___event_handlers[head]);
      ++head;
      if(head >= 5) head = 0;
    }
    ___timer____state.___timer____delayed_event_queue.___head = head;
  }
}
#undef timers


