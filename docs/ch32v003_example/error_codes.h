#ifndef __ERROR_CODES_H__
#define __ERROR_CODES_H__

typedef enum error_code_e {
  ERR_OK = 0,
  ERR_UART = 1,
  ERR_REPLY = 2,
  ERR_MOTOR_SPINUP = 3,
  ERR_MOTOR_SPINDOWN = 4,
  ERR_MOTOR_CURRENT_ASSYMETRY = 5,
  ERR_MOTOR_OVERCURRENT = 6,
  ERR_MOTOR_UNDERCURRENT = 7,
  __ERR_CODE_LAST = 7
} error_code_type;

#endif // __ERROR_CODES_H__