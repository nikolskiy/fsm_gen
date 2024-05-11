#ifndef __SYSTEM_STATE_H__
#define __SYSTEM_STATE_H__

#include <stdint.h>

typedef enum mode_e {
  MANUAL_MODE = 0,
  CONT_MODE = 1,
  INTERVAL_MODE = 2,
} mode_type;

typedef struct system_state {
  mode_type mode;
  uint32_t interval; // in minutes
} system_state;

extern system_state global_state;

#endif // __SYSTEM_STATE_H__