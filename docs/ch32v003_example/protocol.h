#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

typedef enum CMD_e {
  CMD_MOTOR_STATUS = 0,
  CMD_MOTOR_START,
  CMD_MOTOR_STOP,
  CMD_MOTOR_RPM,
  CMD_MOTOR_CURRENT,
  CMD_SET_MOTOR_RPM_MAX,
  CMD_SET_MOTOR_RPM_MIN,
  CMD_SET_MOTOR_CURRENT_MAX,
  CMD_SET_MOTOR_CURRENT_MIN,
  CMD_SET_MOTOR_CURRENT_ASSYM,
  CMD_SET_STURTUP_TIME
} CMD_type;

typedef enum RPLY_STATUS_e {
  RPLY_STOP = 0,
  RPLY_RUN,
  RPLY_STALL,
  RPLY_ESTOP
} RPLY_STATUS_type;

typedef enum RPLY_e {
  RPLY_STATUS = 0,
  RPLY_RPM,
  RPLY_CURRENT,
  RPLY_SET_VAL
} RPLY_type;

int parse_rply(const char*, uint8_t*, uint32_t*);
void send_req(uint8_t, uint32_t);

#endif // __PROTOCOL_H__